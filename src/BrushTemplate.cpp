#include "stdafx.h"
#include "OrinrinEditor.h"

#define BRUSHTEMPLATE_CLASS	TEXT("BRUSH_TEMPLATE")
#define BT_WIDTH	240
#define BT_HEIGHT	240

#define BTV_R_MARGIN	18

#define TB_ITEMS	1
static  TBBUTTON	gstBrTBInfo[] = {
	{ 0,	IDM_BRUSH_ON_OFF,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0 }
};

extern HFONT	ghAaFont;

extern INT		gbTmpltDock;
extern BOOLEAN	gbDockTmplView;

extern  LONG	grdSplitPos;

static HIMAGELIST	ghBrushImgLst;

static  UINT	gbBrushMode;

static  ATOM	gBrTmplAtom;
static  HWND	ghBrTmplWnd;
static  HWND	ghBrTlBarWnd;
static  HWND	ghCtgryBxWnd;
static  HWND	ghLvItemWnd;
static  HWND	ghBrLvTipWnd;

static  HWND	ghMainWnd;

static  UINT	gNowGroup;

static WNDPROC	gpfOrigBrushCtgryProc;
static WNDPROC	gpfOrigBrushItemProc;

static  UINT	gBrhClmCnt;

static WNDPROC	gpfOrigTBProc;

static vector<AATEMPLATE>	gvcBrTmpls;

LRESULT	CALLBACK BrushTmpleProc( HWND, UINT, WPARAM, LPARAM );
VOID	Btp_OnCommand( HWND, INT, HWND, UINT );
VOID	Btp_OnSize( HWND, UINT, INT, INT );
LRESULT	Btp_OnNotify( HWND, INT, LPNMHDR );
VOID	Btp_OnContextMenu( HWND, HWND, UINT, UINT );

UINT	CALLBACK BrushTmpleItemData( LPTSTR, LPCTSTR, INT );

UINT	BrushTmpleItemListOn( UINT );
HRESULT	BrushTmpleItemReload( HWND );

LRESULT	CALLBACK gpfBrushCtgryProc( HWND, UINT, WPARAM, LPARAM );
LRESULT	CALLBACK gpfBrushItemProc(  HWND, UINT, WPARAM, LPARAM );
LRESULT	Blv_OnNotify( HWND, INT, LPNMHDR );

static LRESULT	CALLBACK gpfToolbarProc( HWND, UINT, WPARAM, LPARAM );

HWND BrushTmpleInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame, HWND hMaaWnd )
{
	DWORD		dwExStyle, dwStyle;
	HWND		hPrWnd;
	UINT_PTR	dItems, i;
	TCHAR		atBuffer[MAX_STRING];

	HBITMAP	hImg, hMsq;
	INT		spPos;

	WNDCLASSEX	wcex;
	RECT		wdRect, clRect, rect, cbxRect, tbRect, mtbRect;
	LVCOLUMN	stLvColm;

	TTTOOLINFO	stToolInfo;

	if( !(hInstance) && !(hParentWnd) )
	{
		ImageList_Destroy( ghBrushImgLst  );
		return NULL;
	}

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= BrushTmpleProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= BRUSHTEMPLATE_CLASS;
	wcex.hIconSm		= NULL;

	gBrTmplAtom = RegisterClassEx( &wcex );

	ghMainWnd = hParentWnd;

	gbBrushMode = FALSE;

	TemplateItemLoad( AA_BRUSH_FILE, BrushTmpleItemData );

	InitWindowPos( INIT_LOAD, WDP_BRTMPL, &rect );
	if( 0 == rect.right || 0 == rect.bottom )
	{
		GetWindowRect( hParentWnd, &wdRect );
		rect.left   = wdRect.right + 32;
		rect.top    = wdRect.top + 32;
		rect.right  = BT_WIDTH;
		rect.bottom = BT_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_BRTMPL, &rect );
	}

	gBrhClmCnt = InitParamValue( INIT_LOAD, VL_BRUSHTMP_CLM, 4 );

	if( gbTmpltDock )
	{
		spPos = grdSplitPos - SPLITBAR_WIDTH;

		hPrWnd    = hParentWnd;
		dwExStyle = 0;
		dwStyle   = WS_CHILD;

		rect = *pstFrame;
		rect.left  = rect.right - spPos;
		rect.right = PLIST_DOCK;
		rect.bottom >>= 1;
		rect.top    += rect.bottom;

		DockingTabSizeGet( &mtbRect );
		rect.top    += mtbRect.bottom;
		rect.bottom -= mtbRect.bottom;
	}
	else
	{
		hPrWnd = NULL;

		dwExStyle = WS_EX_TOOLWINDOW;
		if( InitWindowTopMost( INIT_LOAD, WDP_BRTMPL, 0 ) ){	dwExStyle |=  WS_EX_TOPMOST;	}
		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
	}

	ghBrTmplWnd = CreateWindowEx( dwExStyle, BRUSHTEMPLATE_CLASS, TEXT("브러시 템플릿"),
		dwStyle, rect.left, rect.top, rect.right, rect.bottom, hPrWnd, NULL, hInstance, NULL);

	ghBrTlBarWnd = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("brtoolbar"), WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, ghBrTmplWnd, (HMENU)IDW_BRUSH_TOOL_BAR, hInstance, NULL );

	SendMessage( ghBrTlBarWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	ghBrushImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 1, 1 );
	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_BRUSH_MODE) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_BRUSH_MODE) ) );
	ImageList_Add( ghBrushImgLst , hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	SendMessage( ghBrTlBarWnd, TB_SETIMAGELIST, 0, (LPARAM)ghBrushImgLst );

	SendMessage( ghBrTlBarWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuffer, MAX_STRING, TEXT("브러시 모드 ON/OFF") );
	gstBrTBInfo[0].iString = SendMessage( ghBrTlBarWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghBrTlBarWnd , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstBrTBInfo );

	SendMessage( ghBrTlBarWnd , TB_AUTOSIZE, 0, 0 );
	InvalidateRect( ghBrTlBarWnd , NULL, TRUE );

	gpfOrigTBProc = SubclassWindow( ghBrTlBarWnd, gpfToolbarProc );

	GetClientRect( ghBrTlBarWnd, &tbRect );

	GetClientRect( ghBrTmplWnd, &clRect );

	ghCtgryBxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT("BrCategory"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST,
		0, tbRect.bottom, clRect.right, 127, ghBrTmplWnd,
		(HMENU)IDCB_BT_CATEGORY, hInstance, NULL );

	gpfOrigBrushCtgryProc = SubclassWindow( ghCtgryBxWnd, gpfBrushCtgryProc );

	dItems = gvcBrTmpls.size( );
	for( i = 0; dItems > i; i++ )
	{
		ComboBox_AddString( ghCtgryBxWnd, gvcBrTmpls.at( i ).atCtgryName );
	}
	ComboBox_SetCurSel( ghCtgryBxWnd, 0 );
	gNowGroup = 0;

	GetClientRect( ghCtgryBxWnd, &cbxRect );

	ghLvItemWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("brushitem"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LVS_REPORT | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER | LVS_SINGLESEL,
		0, tbRect.bottom + cbxRect.bottom, clRect.right, clRect.bottom - (cbxRect.bottom + tbRect.bottom),
		ghBrTmplWnd, (HMENU)IDLV_BT_ITEMVIEW, hInstance, NULL );
	ListView_SetExtendedListViewStyle( ghLvItemWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	SetWindowFont( ghLvItemWnd, ghAaFont, TRUE );

	gpfOrigBrushItemProc = SubclassWindow( ghLvItemWnd, gpfBrushItemProc );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt      = LVCFMT_LEFT;
	stLvColm.pszText  = TEXT("Brush");
	stLvColm.cx       = 10;
	stLvColm.iSubItem = 0;

	for( i = 0; gBrhClmCnt > i; i++ )
	{
		stLvColm.iSubItem = i;
		ListView_InsertColumn( ghLvItemWnd, i, &stLvColm );
	}

	BrushTmpleItemListOn( 0 );

	ghBrLvTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghBrTmplWnd, NULL, hInstance, NULL );

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;
	stToolInfo.hwnd     = ghLvItemWnd;
	stToolInfo.uId      = IDLV_BT_ITEMVIEW;
	GetClientRect( ghLvItemWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage( ghBrLvTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghBrLvTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );

	if( !(gbTmpltDock) )
	{
		ShowWindow( ghBrTmplWnd, SW_SHOW );
		UpdateWindow( ghBrTmplWnd );
	}

	return ghBrTmplWnd;
}

HRESULT BrushTmplePositionReset( HWND hMainWnd )
{
	RECT	wdRect, rect;

	GetWindowRect( hMainWnd, &wdRect );
	rect.left   = wdRect.right + 32;
	rect.top    = wdRect.top + 32;
	rect.right  = BT_WIDTH;
	rect.bottom = BT_HEIGHT;

	SetWindowPos( ghBrTmplWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW | SWP_NOZORDER );

	return S_OK;
}

VOID BrushTmpleResize( HWND hPrntWnd, LPRECT pstFrame )
{
	RECT	rect, tbRect;

	if( !(ghBrTmplWnd) )	return;

	if( !(gbDockTmplView) )	return;

	rect = *pstFrame;
	rect.left    = rect.right - (grdSplitPos - SPLITBAR_WIDTH);
	rect.right   = (grdSplitPos - SPLITBAR_WIDTH);
	rect.bottom >>= 1;
	rect.top    += rect.bottom;

	DockingTabSizeGet( &tbRect );
	rect.top    += tbRect.bottom;
	rect.bottom -= tbRect.bottom;

	SetWindowPos( ghBrTmplWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER );

	return;
}

LRESULT CALLBACK gpfToolbarProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if( SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0 ){	ReleaseCapture(   );	}
			return 0;
	}

	return CallWindowProc( gpfOrigTBProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK BrushTmpleProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_SIZE,        Btp_OnSize );
		HANDLE_MSG( hWnd, WM_COMMAND,     Btp_OnCommand );
		HANDLE_MSG( hWnd, WM_NOTIFY,      Btp_OnNotify );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Btp_OnContextMenu );

		case WM_MOUSEWHEEL:	SendMessage( ghLvItemWnd, WM_MOUSEWHEEL, wParam, lParam );	return 0;

		case WM_CLOSE:	ShowWindow( ghBrTmplWnd , SW_HIDE );	return 0;

		case WMP_BRUSH_TOGGLE:
			if( gbBrushMode )
			{
				SendMessage( ghBrTlBarWnd, TB_SETSTATE, IDM_BRUSH_ON_OFF, TBSTATE_ENABLED );
				gbBrushMode = FALSE;
			}
			else
			{
				SendMessage( ghBrTlBarWnd, TB_SETSTATE, IDM_BRUSH_ON_OFF, (TBSTATE_CHECKED | TBSTATE_ENABLED) );
				gbBrushMode = TRUE;
			}
			ViewBrushStyleSetting( gbBrushMode, NULL );
			return gbBrushMode;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Btp_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	INT			rslt;
	UINT		dClm;
	LRESULT		lRslt;
	LONG_PTR	rdExStyle;
	TCHAR		atItem[SUB_STRING];

	ZeroMemory( atItem, sizeof(atItem) );

	switch( id )
	{
		case IDCB_BT_CATEGORY:
			if( 0 < gvcBrTmpls.size() )
			{
				if( CBN_SELCHANGE == codeNotify )
				{
					rslt = ComboBox_GetCurSel( ghCtgryBxWnd );
					gNowGroup = rslt;

					BrushTmpleItemListOn( rslt );

					gbBrushMode = FALSE;
					SendMessage( ghBrTlBarWnd, TB_SETSTATE, IDM_BRUSH_ON_OFF, TBSTATE_ENABLED );
					StringCchCopy( atItem, SUB_STRING, gvcBrTmpls.at( gNowGroup ).vcItems.at( 0 ).c_str( ) );
					ViewBrushStyleSetting( gbBrushMode, atItem );
				}
			}
			break;

		case IDM_BRUSH_ON_OFF:
			lRslt = SendMessage( ghBrTlBarWnd, TB_GETSTATE, IDM_BRUSH_ON_OFF, 0 );
			gbBrushMode = (lRslt & TBSTATE_CHECKED) ? TRUE : FALSE;
			ViewBrushStyleSetting( gbBrushMode, NULL );
			break;

		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_BRUSH,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_BRUSH, -1 );	break;

		case IDM_TMPL_GRID_INCREASE:
		case IDM_TMPL_GRID_DECREASE:
			dClm = TemplateGridFluctuate( ghLvItemWnd, ((IDM_TMPL_GRID_INCREASE == id) ? 1 : -1) );
			if( dClm )
			{
				gBrhClmCnt = dClm;
				BrushTmpleItemListOn( gNowGroup );
				InitParamValue( INIT_SAVE, VL_BRUSHTMP_CLM, gBrhClmCnt );
			}
			break;

		case IDM_TMPLT_GROUP_PREV:
			if( 0 < gNowGroup )
			{
				gNowGroup--;
				ComboBox_SetCurSel( ghCtgryBxWnd, gNowGroup );
				BrushTmpleItemListOn( gNowGroup );
			}
			break;

		case IDM_TMPLT_GROUP_NEXT:
			if( (gNowGroup + 1) < gvcBrTmpls.size() )
			{
				gNowGroup++;
				ComboBox_SetCurSel( ghCtgryBxWnd, gNowGroup );
				BrushTmpleItemListOn( gNowGroup );
			}
			break;

		case IDM_TOPMOST_TOGGLE:
			rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )
			{
				SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_BRTMPL, 0 );
			}
			else
			{
				SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_BRTMPL, 1 );
			}
			break;

		case IDM_TMPLT_RELOAD:	BrushTmpleItemReload( hWnd );	break;

		default:	break;
	}

	return;
}

VOID Btp_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	LONG	width;
	UINT	i;
	RECT	cbxRect, tbrRect, rect;

	if( !(ghBrTlBarWnd) )	return;
	MoveWindow( ghBrTlBarWnd, 0, 0, 0, 0, TRUE );
	GetClientRect( ghBrTlBarWnd, &tbrRect );

	if( !(ghCtgryBxWnd) )	return;
	MoveWindow( ghCtgryBxWnd, 0, tbrRect.bottom, cx, 127, TRUE );
	GetClientRect( ghCtgryBxWnd, &cbxRect );

	if( !(ghLvItemWnd) )	return;
	MoveWindow( ghLvItemWnd, 0, (cbxRect.bottom + tbrRect.bottom), cx, cy - (cbxRect.bottom + tbrRect.bottom), TRUE );

	GetClientRect( ghLvItemWnd, &rect );
	width = rect.right / gBrhClmCnt;

	for( i = 0; gBrhClmCnt > i; i++ )
	{
		ListView_SetColumnWidth( ghLvItemWnd, i, width );
	}

	return;
}

LRESULT Btp_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	HWND	hLvWnd;
	INT		iPos, iItem, nmCode, iSubItem;
	INT_PTR	items;
	TCHAR	atItem[SUB_STRING];
	LPNMLISTVIEW	pstLv;
	LVHITTESTINFO	stHitTestInfo;

	if( IDLV_BT_ITEMVIEW == idFrom )
	{
		pstLv = (LPNMLISTVIEW)pstNmhdr;

		hLvWnd = pstLv->hdr.hwndFrom;
		nmCode = pstLv->hdr.code;

		stHitTestInfo.pt = pstLv->ptAction;
		ListView_SubItemHitTest( hLvWnd, &stHitTestInfo );

		iItem = stHitTestInfo.iItem;
		iSubItem = stHitTestInfo.iSubItem;
		iPos = iItem * gBrhClmCnt + iSubItem;

		if( NM_CLICK == nmCode )
		{
			TRACE( TEXT("BRUSH TMPL[%d x %d]"), iItem, iSubItem );

			if( 0 < gvcBrTmpls.size() )
			{
				items = gvcBrTmpls.at( gNowGroup ).vcItems.size( );

				if( 0 <= iPos && iPos <  items )
				{
					StringCchCopy( atItem, SUB_STRING, gvcBrTmpls.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );

					gbBrushMode = TRUE;
					SendMessage( ghBrTlBarWnd, TB_SETSTATE, IDM_BRUSH_ON_OFF, (TBSTATE_CHECKED | TBSTATE_ENABLED) );

					ViewBrushStyleSetting( gbBrushMode, atItem );

					ViewFocusSet(  );
				}
			}
			else
			{
				ViewFocusSet(  );
			}
		}
	}

	return 0;
}

VOID Btp_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	LONG_PTR	rdExStyle;

	POINT	stPoint;

	stPoint.x = (SHORT)xPos;
	stPoint.y = (SHORT)yPos;

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_TEMPLATE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	if( gbTmpltDock ){	DeleteMenu( hSubMenu, IDM_TOPMOST_TOGGLE, MF_BYCOMMAND );	}

	rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
	if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}

	dRslt = TrackPopupMenu( hSubMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );

	DestroyMenu( hMenu );

	return;
}

UINT CALLBACK BrushTmpleItemData( LPTSTR ptName, LPCTSTR ptLine, INT cchSize )
{

	static AATEMPLATE	cstItem;

	if( ptName )
	{
		StringCchCopy( cstItem.atCtgryName, SUB_STRING, ptName );
		cstItem.vcItems.clear(  );
	}
	else if( ptLine )
	{
		cstItem.vcItems.push_back( wstring( ptLine ) );
	}
	else
	{
		gvcBrTmpls.push_back( cstItem );
	}

	return 1;
}

UINT BrushTmpleItemListOn( UINT listNum )
{
	INT			width;
	UINT_PTR	i, items;
	TCHAR		atItem[SUB_STRING];
	LVITEM		stLvi;
	RECT		rect;

	if( 0 >= gvcBrTmpls.size() ){	return 0;	}

	ZeroMemory( atItem, sizeof(atItem) );

	items = gvcBrTmpls.at( listNum ).vcItems.size( );

	TRACE( TEXT("BRUSH open NUM[%u] ITEM[%u] GRID[%d]"), listNum, items, gBrhClmCnt );

	ListView_DeleteAllItems( ghLvItemWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask    = LVIF_TEXT;
	stLvi.pszText = atItem;

	for( i = 0; items > i; i++ )
	{
		StringCchCopy( atItem, SUB_STRING, gvcBrTmpls.at( listNum ).vcItems.at( i ).c_str( ) );

		stLvi.iItem    = i / gBrhClmCnt;
		stLvi.iSubItem = i % gBrhClmCnt;
		if( 0 == stLvi.iSubItem )	ListView_InsertItem( ghLvItemWnd, &stLvi );
		else						ListView_SetItem( ghLvItemWnd, &stLvi );
	}

	GetClientRect( ghLvItemWnd, &rect );
	width = rect.right / gBrhClmCnt;
	for( i = 0; gBrhClmCnt > i; i++ ){	ListView_SetColumnWidth( ghLvItemWnd, i, width );	}

	return items;
}

HRESULT BrushTmpleItemReload( HWND hWnd )
{
	TEMPL_ITR	itTmpl;

	gNowGroup = 0;

	gbBrushMode = 0;
	ViewBrushStyleSetting( gbBrushMode, TEXT("") );

	for( itTmpl = gvcBrTmpls.begin( );  gvcBrTmpls.end( ) != itTmpl; itTmpl++ ){	itTmpl->vcItems.clear();	}
	gvcBrTmpls.clear(  );

	while( ComboBox_GetCount( ghCtgryBxWnd )  ){	ComboBox_DeleteString( ghCtgryBxWnd, 0 );	}

	TemplateItemLoad( AA_BRUSH_FILE, BrushTmpleItemData );

	for( itTmpl = gvcBrTmpls.begin( );  gvcBrTmpls.end( ) != itTmpl; itTmpl++ )
	{
		ComboBox_AddString( ghCtgryBxWnd, itTmpl->atCtgryName );
	}
	ComboBox_SetCurSel( ghCtgryBxWnd, 0 );

	BrushTmpleItemListOn( 0 );

	return S_OK;
}

LRESULT CALLBACK gpfBrushCtgryProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT	id;

	switch( msg )
	{
		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDM_WINDOW_CHANGE:
				case IDM_WINDOW_CHG_RVRS:
				case IDM_TMPL_GRID_INCREASE:
				case IDM_TMPL_GRID_DECREASE:
				case IDM_TMPLT_GROUP_PREV:
				case IDM_TMPLT_GROUP_NEXT:
					SendMessage( ghBrTmplWnd, WM_COMMAND, wParam, lParam );
					return 0;
			}
			break;

		case WM_MOUSEWHEEL:
			SendMessage( ghLvItemWnd, WM_MOUSEWHEEL, wParam, lParam );
			return 0;
	}

	return CallWindowProc( gpfOrigBrushCtgryProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK gpfBrushItemProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT	id;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_NOTIFY, Blv_OnNotify );

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDM_WINDOW_CHANGE:
				case IDM_WINDOW_CHG_RVRS:
				case IDM_TMPL_GRID_INCREASE:
				case IDM_TMPL_GRID_DECREASE:
				case IDM_TMPLT_GROUP_PREV:
				case IDM_TMPLT_GROUP_NEXT:
					SendMessage( ghBrTmplWnd, WM_COMMAND, wParam, lParam );
					return 0;
			}
			break;
	}

	return CallWindowProc( gpfOrigBrushItemProc, hWnd, msg, wParam, lParam );
}

LRESULT Blv_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	HWND	hLvWnd;
	INT		iPos, iItem, nmCode, iSubItem, iDot;
	INT_PTR	items;
	TCHAR	atItem[SUB_STRING];
	LVHITTESTINFO	stHitTestInfo;
	LPNMLISTVIEW	pstLv;
	LPNMTTDISPINFO	pstDispInfo;

	pstLv = (LPNMLISTVIEW)pstNmhdr;

	hLvWnd = hWnd;
	nmCode = pstLv->hdr.code;

	if( TTN_GETDISPINFO == nmCode )
	{
		if( IDLV_BT_ITEMVIEW == idFrom )
		{
			ZeroMemory( &stHitTestInfo, sizeof(LVHITTESTINFO) );
			GetCursorPos( &(stHitTestInfo.pt) );
			ScreenToClient( hLvWnd, &(stHitTestInfo.pt) );
			ListView_SubItemHitTest( hLvWnd, &stHitTestInfo );

			iItem = stHitTestInfo.iItem;
			iSubItem = stHitTestInfo.iSubItem;
			iPos = iItem * gBrhClmCnt + iSubItem;

			TRACE( TEXT("BLvTT[%d]"), iPos );

			pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );
			pstDispInfo->lpszText = pstDispInfo->szText;

			if( 0 < gvcBrTmpls.size( ) )
			{
				items = gvcBrTmpls.at( gNowGroup ).vcItems.size( );

				if( 0 <= iPos && iPos <  items )
				{
					StringCchCopy( atItem, SUB_STRING, gvcBrTmpls.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );
					iDot = ViewStringWidthGet( atItem );

					StringCchPrintf( pstDispInfo->szText, 80, TEXT("%s [%d Dot]"), atItem, iDot );
				}
			}

			return 0;
		}
	}

	return CallWindowProc( gpfOrigBrushItemProc, hWnd, WM_NOTIFY, (WPARAM)idFrom, (LPARAM)pstNmhdr );
}
