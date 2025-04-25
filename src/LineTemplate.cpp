#include "stdafx.h"
#include "OrinrinEditor.h"

#define LINETEMPLATE_CLASS	TEXT("LINE_TEMPLATE")
#define LT_WIDTH	240
#define LT_HEIGHT	240

#define LTP_CLICK_NEW

extern HFONT	ghAaFont;
extern HFONT	ghNameFont;

extern INT		gbTmpltDock;
extern BOOLEAN	gbDockTmplView;

extern  LONG	grdSplitPos;

static  ATOM	gTmpleAtom;
static  HWND	ghTmpleWnd;

static  HWND	ghCtgryBxWnd;
static  HWND	ghLvItemWnd;
static  HWND	ghLnLvTipWnd;

static  HWND	ghDockTabWnd;

static  UINT	gNowGroup;

static  UINT	gLnClmCnt;

static WNDPROC	gpfOrigLineCtgryProc;
static WNDPROC	gpfOrigLineItemProc;

static vector<AATEMPLATE>	gvcTmples;

LRESULT	CALLBACK LineTmpleProc( HWND, UINT, WPARAM, LPARAM );
VOID	Ltp_OnCommand( HWND , INT, HWND, UINT );
VOID	Ltp_OnSize( HWND , UINT, INT, INT );
VOID	Ltp_OnContextMenu( HWND, HWND, UINT, UINT );
#ifndef LTP_CLICK_NEW
LRESULT	Ltp_OnNotify( HWND , INT, LPNMHDR );
#endif

UINT	CALLBACK LineTmpleItemData( LPTSTR, LPCTSTR, INT );

HRESULT	LineTmpleItemListOn( UINT );
HRESULT	LineTmpleItemReload( HWND );

HRESULT	TemplateItemSplit( LPTSTR, UINT, PAGELOAD );
HRESULT	TemplateItemScatter( LPCTSTR, INT, PAGELOAD );

LRESULT	CALLBACK gpfLineCtgryProc( HWND, UINT, WPARAM, LPARAM );
LRESULT	CALLBACK gpfLineItemProc(  HWND, UINT, WPARAM, LPARAM );
LRESULT	Ltl_OnNotify( HWND , INT, LPNMHDR );
#ifdef LTP_CLICK_NEW
VOID	Ltl_OnMouseButtonUp( HWND, UINT, INT, INT, UINT );
#endif

HWND	DockingTabCreate( HINSTANCE, HWND, LPRECT );

HWND LineTmpleInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame )
{

	WNDCLASSEX	wcex;
	RECT		wdRect, clRect, rect, cbxRect;
	UINT_PTR	dItems, i;
	DWORD		dwExStyle, dwStyle;
	HWND		hPrWnd;
	INT			spPos;

	TTTOOLINFO	stToolInfo;
	LVCOLUMN	stLvColm;

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= LineTmpleProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= LINETEMPLATE_CLASS;
	wcex.hIconSm		= NULL;

	gTmpleAtom = RegisterClassEx( &wcex );

	TemplateItemLoad( AA_LIST_FILE, LineTmpleItemData );

	InitWindowPos( INIT_LOAD, WDP_LNTMPL, &rect );
	if( 0 == rect.right || 0 == rect.bottom )
	{
		GetWindowRect( hParentWnd, &wdRect );
		rect.left   = wdRect.right;
		rect.top    = wdRect.top;
		rect.right  = LT_WIDTH;
		rect.bottom = LT_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_LNTMPL, &rect );
	}

	gLnClmCnt = InitParamValue( INIT_LOAD, VL_LINETMP_CLM, 4 );

	if( gbTmpltDock )
	{
		spPos = grdSplitPos - SPLITBAR_WIDTH;

		hPrWnd    = hParentWnd;
		dwExStyle = 0;
		dwStyle   = WS_CHILD | WS_VISIBLE;

		rect = *pstFrame;
		rect.left  = rect.right - spPos;
		rect.right = PLIST_DOCK;
		rect.bottom >>= 1;
		rect.top    += rect.bottom;

		ghDockTabWnd = DockingTabCreate( hInstance, hPrWnd, &rect );
	}
	else
	{

		dwExStyle = WS_EX_TOOLWINDOW;
		if( InitWindowTopMost( INIT_LOAD, WDP_LNTMPL, 0 ) ){	dwExStyle |=  WS_EX_TOPMOST;	}
		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
		hPrWnd = NULL;
	}

	ghTmpleWnd = CreateWindowEx( dwExStyle, LINETEMPLATE_CLASS, TEXT("라인 템플릿"),
		dwStyle, rect.left, rect.top, rect.right, rect.bottom, hPrWnd, NULL, hInstance, NULL);

	GetClientRect( ghTmpleWnd, &clRect );

	ghCtgryBxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT("카테고리"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST,
		0, 0, clRect.right, 127, ghTmpleWnd,
		(HMENU)IDCB_LT_CATEGORY, hInstance, NULL );

	gpfOrigLineCtgryProc = SubclassWindow( ghCtgryBxWnd, gpfLineCtgryProc );

	dItems = gvcTmples.size( );
	for( i = 0; dItems > i; i++ )
	{
		ComboBox_AddString( ghCtgryBxWnd, gvcTmples.at( i ).atCtgryName );
	}
	ComboBox_SetCurSel( ghCtgryBxWnd, 0 );
	gNowGroup = 0;

	GetClientRect( ghCtgryBxWnd, &cbxRect );

	ghLvItemWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("라인아이템"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LVS_REPORT | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER,
		0, cbxRect.bottom, clRect.right, clRect.bottom - cbxRect.bottom,
		ghTmpleWnd, (HMENU)IDLV_LT_ITEMVIEW, hInstance, NULL );
	ListView_SetExtendedListViewStyle( ghLvItemWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP | LVS_EX_ONECLICKACTIVATE );

	SetWindowFont( ghLvItemWnd, ghAaFont, TRUE );

	gpfOrigLineItemProc = SubclassWindow( ghLvItemWnd, gpfLineItemProc );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt      = LVCFMT_LEFT;
	stLvColm.pszText  = TEXT("아이템");
	stLvColm.cx       = 10;
	for( i = 0; gLnClmCnt > i; i++ )
	{
		stLvColm.iSubItem = i;
		ListView_InsertColumn( ghLvItemWnd, i, &stLvColm );
	}

	LineTmpleItemListOn( 0 );

	ghLnLvTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghTmpleWnd, NULL, hInstance, NULL );

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;
	stToolInfo.hwnd     = ghLvItemWnd;
	stToolInfo.uId      = IDLV_LT_ITEMVIEW;
	GetClientRect( ghLvItemWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage( ghLnLvTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghLnLvTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );

	ShowWindow( ghTmpleWnd, SW_SHOW );
	UpdateWindow( ghTmpleWnd );

	return ghTmpleWnd;
}

HWND DockingTabCreate( HINSTANCE hInst, HWND hPrWnd, LPRECT pstRect )
{
	HWND	hWorkWnd;
	RECT	itRect;
	TCITEM	stTcItem;

	hWorkWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("도크선택탭"),
		WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_TABS | TCS_SINGLELINE,
		pstRect->left, pstRect->top, pstRect->right, 10, hPrWnd, (HMENU)IDTB_DOCK_TAB, hInst, NULL );
	SetWindowFont( hWorkWnd, ghNameFont, FALSE );

	ZeroMemory( &stTcItem, sizeof(stTcItem) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = TEXT("한 줄");	TabCtrl_InsertItem( hWorkWnd, 0, &stTcItem );
	stTcItem.pszText = TEXT("브러시");	TabCtrl_InsertItem( hWorkWnd, 1, &stTcItem );

	TabCtrl_GetItemRect( hWorkWnd, 1, &itRect );
	itRect.bottom  += itRect.top;
	MoveWindow( hWorkWnd, pstRect->left, pstRect->top, pstRect->right, itRect.bottom, TRUE );

	pstRect->top    += itRect.bottom;
	pstRect->bottom -= itRect.bottom;

	return hWorkWnd;
}

VOID DockingTabSizeGet( LPRECT pstRect )
{
	ZeroMemory( pstRect, sizeof(RECT) );

	if( ghDockTabWnd )
	{
		GetWindowRect( ghDockTabWnd, pstRect );
		pstRect->right -= pstRect->left;
		pstRect->bottom -= pstRect->top;
	}

	return;
}

HRESULT DockingTabContextMenu( HWND hWnd, HWND hWndContext, LONG xPos, LONG yPos )
{
	HMENU	hPopupMenu = NULL;

	if( hWndContext != ghDockTabWnd ){	return  E_ABORT;	}

	hPopupMenu = CreatePopupMenu(  );

	if( gbDockTmplView )	AppendMenu( hPopupMenu, MF_STRING, IDM_LINE_BRUSH_TMPL_VIEW, TEXT("템플릿 숨기기") );
	else					AppendMenu( hPopupMenu, MF_STRING, IDM_LINE_BRUSH_TMPL_VIEW, TEXT("템플릿 표시") );

	TrackPopupMenu( hPopupMenu, 0, xPos, yPos, 0, hWnd, NULL );
	DestroyMenu( hPopupMenu );

	return S_OK;
}

HWND DockingTabGet( VOID )
{
	if( gbTmpltDock )	return ghDockTabWnd;

	return NULL;
}

HRESULT LineTmplePositionReset( HWND hMainWnd )
{
	RECT	wdRect, rect;

	GetWindowRect( hMainWnd, &wdRect );
	rect.left   = wdRect.right;
	rect.top    = wdRect.top;
	rect.right  = LT_WIDTH;
	rect.bottom = LT_HEIGHT;

	SetWindowPos( ghTmpleWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW | SWP_NOZORDER );

	return S_OK;
}

VOID LineTmpleResize( HWND hPrntWnd, LPRECT pstFrame )
{
	RECT	rect, tbRect;

	rect = *pstFrame;
	rect.left    = rect.right - (grdSplitPos - SPLITBAR_WIDTH);
	rect.right   = (grdSplitPos - SPLITBAR_WIDTH);

	if( gbDockTmplView )
	{
		rect.bottom >>= 1;
		rect.top    += rect.bottom;

		GetWindowRect( ghDockTabWnd, &tbRect );

		tbRect.left    = rect.left;
		tbRect.right   = (grdSplitPos - SPLITBAR_WIDTH);
		tbRect.bottom -= tbRect.top;
		tbRect.top     = rect.top;
		MoveWindow( ghDockTabWnd, tbRect.left, tbRect.top, tbRect.right, tbRect.bottom, TRUE );
	}
	else
	{
		DockingTabSizeGet( &tbRect );

		tbRect.left    = rect.left;
		tbRect.right   = (grdSplitPos - SPLITBAR_WIDTH);

		tbRect.top     = rect.top + (rect.bottom - tbRect.bottom);

		MoveWindow( ghDockTabWnd, tbRect.left, tbRect.top, tbRect.right, tbRect.bottom, TRUE );
		return;

	}

	rect.top    += tbRect.bottom;
	rect.bottom -= tbRect.bottom;
	SetWindowPos( ghTmpleWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER );

	return;
}

LRESULT CALLBACK LineTmpleProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_SIZE,        Ltp_OnSize );
		HANDLE_MSG( hWnd, WM_COMMAND,     Ltp_OnCommand );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Ltp_OnContextMenu );
#ifndef LTP_CLICK_NEW
		HANDLE_MSG( hWnd, WM_NOTIFY,      Ltp_OnNotify );
#endif

		case WM_MOUSEWHEEL:	SendMessage( ghLvItemWnd, WM_MOUSEWHEEL, wParam, lParam );	return 0;

		case WM_CLOSE:	ShowWindow( ghTmpleWnd, SW_HIDE );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Ltp_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	INT		rslt;
	UINT	dClm;
	LONG_PTR	rdExStyle;

	switch( id )
	{
		case IDCB_LT_CATEGORY:
			if( CBN_SELCHANGE == codeNotify )
			{
				rslt = ComboBox_GetCurSel( ghCtgryBxWnd );
				gNowGroup = rslt;

				LineTmpleItemListOn( rslt );
			}
			break;

		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_LINE,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_LINE, -1 );	break;

		case IDM_TMPL_GRID_INCREASE:
		case IDM_TMPL_GRID_DECREASE:
			dClm = TemplateGridFluctuate( ghLvItemWnd, ((IDM_TMPL_GRID_INCREASE == id) ? 1 : -1) );
			if( dClm )
			{
				gLnClmCnt = dClm;
				LineTmpleItemListOn( gNowGroup );
				InitParamValue( INIT_SAVE, VL_LINETMP_CLM, gLnClmCnt );
			}
			break;

		case IDM_TMPLT_GROUP_PREV:
			if( 0 < gNowGroup )
			{
				gNowGroup--;
				ComboBox_SetCurSel( ghCtgryBxWnd, gNowGroup );
				LineTmpleItemListOn( gNowGroup );
			}
			break;

		case IDM_TMPLT_GROUP_NEXT:
			if( (gNowGroup + 1) < gvcTmples.size() )
			{
				gNowGroup++;
				ComboBox_SetCurSel( ghCtgryBxWnd, gNowGroup );
				LineTmpleItemListOn( gNowGroup );
			}
			break;

		case IDM_TOPMOST_TOGGLE:
			rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )
			{
				SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_LNTMPL, 0 );
			}
			else
			{
				SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_LNTMPL, 1 );
			}
			break;

		case IDM_TMPLGROUPSTYLE_TGL:
			break;

		case IDM_TMPLT_RELOAD:	LineTmpleItemReload( hWnd );	break;

		default:	break;
	}

	return;
}

VOID Ltp_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	LONG	width;
	UINT	i;
	RECT	cbxRect, rect;

	MoveWindow( ghCtgryBxWnd, 0, 0, cx, 127, TRUE );
	GetClientRect( ghCtgryBxWnd, &cbxRect );

	MoveWindow( ghLvItemWnd, 0, cbxRect.bottom, cx, cy - cbxRect.bottom, TRUE );

	GetClientRect( ghLvItemWnd, &rect );
	width  = rect.right / gLnClmCnt;

	for( i = 0; gLnClmCnt > i; i++ )
	{
		ListView_SetColumnWidth( ghLvItemWnd, i, width );
	}

	return;
}

#ifndef LTP_CLICK_NEW

LRESULT Ltp_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	HWND	hLvWnd;
	INT		iPos, iItem, nmCode, iSubItem;
	INT_PTR	items;
	TCHAR	atItem[SUB_STRING];
	LPNMLISTVIEW	pstLv;
	LVHITTESTINFO	stHitTestInfo;

	if( IDLV_LT_ITEMVIEW == idFrom )
	{
		pstLv = (LPNMLISTVIEW)pstNmhdr;

		hLvWnd = pstLv->hdr.hwndFrom;
		nmCode = pstLv->hdr.code;

		if( NM_CLICK == nmCode )
		{
			stHitTestInfo.pt = pstLv->ptAction;
			ListView_SubItemHitTest( hLvWnd, &stHitTestInfo );

			iItem = stHitTestInfo.iItem;
			iSubItem = stHitTestInfo.iSubItem;
			iPos = iItem * gLnClmCnt + iSubItem;

			if( 0 < gvcTmples.size() )
			{
				items = gvcTmples.at( gNowGroup ).vcItems.size( );

				TRACE( TEXT("라인 템플릿[%d x %d]"), iItem, iSubItem );

				if( 0 <= iPos && iPos <  items )
				{
					StringCchCopy( atItem, SUB_STRING, gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );
					ViewInsertTmpleString( atItem );

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

#endif

VOID Ltp_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
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

UINT CALLBACK LineTmpleItemData( LPTSTR ptName, LPCTSTR ptLine, INT cchSize )
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
		gvcTmples.push_back( cstItem );
	}

	return 1;
}

HRESULT LineTmpleItemListOn( UINT listNum )
{
	INT			width;
	UINT_PTR	i, items;
	TCHAR		atItem[SUB_STRING];
	LVITEM		stLvi;
	RECT		rect;

	ZeroMemory( atItem, sizeof(atItem) );

	if( 0 >= gvcTmples.size() ){	return E_OUTOFMEMORY;	}

	items = gvcTmples.at( listNum ).vcItems.size( );

	TRACE( TEXT("라인 열기 번호[%u] 아이템[%u] 그리드[%d]"), listNum, items, gLnClmCnt );

	ListView_DeleteAllItems( ghLvItemWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask    = LVIF_TEXT;
	stLvi.pszText = atItem;

	for( i = 0; items > i; i++ )
	{
		StringCchCopy( atItem, SUB_STRING, gvcTmples.at( listNum ).vcItems.at( i ).c_str( ) );

		stLvi.iItem     = i / gLnClmCnt;
		stLvi.iSubItem  = i % gLnClmCnt;
		if( 0 == stLvi.iSubItem )	ListView_InsertItem( ghLvItemWnd, &stLvi );
		else						ListView_SetItem( ghLvItemWnd, &stLvi );
	}

	GetClientRect( ghLvItemWnd, &rect );
	width  = rect.right / gLnClmCnt;
	for( i = 0; gLnClmCnt > i; i++ ){	ListView_SetColumnWidth( ghLvItemWnd, i, width );	}

	return S_OK;
}

HRESULT LineTmpleItemReload( HWND hWnd )
{
	TEMPL_ITR	itTmpl;

	gNowGroup = 0;

	for( itTmpl = gvcTmples.begin( ); gvcTmples.end( ) != itTmpl; itTmpl++ ){	itTmpl->vcItems.clear();	}
	gvcTmples.clear(  );

	while( ComboBox_GetCount( ghCtgryBxWnd )  ){	ComboBox_DeleteString( ghCtgryBxWnd, 0 );	}

	TemplateItemLoad( AA_LIST_FILE, LineTmpleItemData );

	for( itTmpl = gvcTmples.begin( ); gvcTmples.end( ) != itTmpl; itTmpl++ )
	{
		ComboBox_AddString( ghCtgryBxWnd, itTmpl->atCtgryName );
	}
	ComboBox_SetCurSel( ghCtgryBxWnd, 0 );

	LineTmpleItemListOn( 0 );

	return S_OK;
}

HRESULT TemplateItemLoad( LPTSTR ptFileName, PAGELOAD pfCalling )
{
	CONST WCHAR rtHead = 0xFEFF;
	WCHAR	rtUniBuf;

	HANDLE	hFile;
	DWORD	readed;

	LPVOID	pBuffer;
	INT		iByteSize;

	LPTSTR	ptString;
	LPSTR	pcText;
	UINT	cchSize;

	TCHAR	atFileName[MAX_PATH];

	StringCchCopy( atFileName, MAX_PATH, ExePathGet() );
	PathAppend( atFileName, TEMPLATE_DIR );
	PathAppend( atFileName, ptFileName );

	hFile = CreateFile( atFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return E_HANDLE;	}

	iByteSize = GetFileSize( hFile, NULL );
	pBuffer = malloc( iByteSize + 2 );
	ZeroMemory( pBuffer, iByteSize + 2 );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pBuffer, iByteSize, &readed, NULL );
	CloseHandle( hFile );

	CopyMemory( &rtUniBuf, pBuffer, 2 );
	if( rtHead == rtUniBuf )
	{
		ptString = (LPTSTR)pBuffer;
		ptString++;
	}
	else
	{
		pcText = (LPSTR)pBuffer;
		ptString = SjisDecodeAlloc( pcText );

		FREE( pBuffer );
		pBuffer = ptString;
	}

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	TemplateItemSplit( ptString, cchSize, pfCalling );

	FREE( pBuffer );

	return S_OK;
}

HRESULT TemplateItemSplit( LPTSTR ptStr, UINT cchSize, PAGELOAD pfCalling )
{
	LPCTSTR	ptCaret;
	LPCTSTR	ptStart;
	LPTSTR	ptEnd;
	UINT	iNumber;
	UINT	cchItem;

	BOOLEAN	bLast;
	TCHAR	atName[MAX_PATH];

	ptCaret = ptStr;

	iNumber = 0;

	bLast = FALSE;

	ptEnd = StrStr( ptCaret, TMPLE_BEGINW );
	if( !ptEnd )	return E_INVALIDARG;

	ptCaret = ptEnd;

	do
	{
		ptStart = NextLineW( ptCaret );
		if( !ptStart )	return  S_FALSE;

		ptEnd = StrStr( ptCaret, TEXT("=") );
		ptCaret = ptEnd + 1;
		cchItem = ptStart - ptCaret;
		cchItem -= 3;

		ZeroMemory( atName, sizeof(atName) );
		if( 0 < cchItem ){	StringCchCopyN( atName, MAX_PATH, ptCaret, cchItem );	}
		else{	StringCchPrintf( atName, MAX_PATH, TEXT("이름없음%d"), iNumber );	}

		StringCchLength( atName, MAX_PATH, &cchItem );
		pfCalling( atName, NULL, cchItem );

		ptCaret = ptStart;

		ptEnd = StrStr( ptCaret, TMPLE_ENDW );

		if( !ptEnd ){	return  S_FALSE;	}

		cchItem = ptEnd - ptCaret;

		TemplateItemScatter( ptCaret, cchItem, pfCalling );

		pfCalling( NULL, NULL, 0 );

		iNumber++;

		ptCaret = NextLineW( ptEnd );

	}while( *ptCaret );

	return S_OK;
}

HRESULT TemplateItemScatter( LPCTSTR ptCont, INT cchSize, PAGELOAD pfCalling )
{

	INT	nowCaret, nYct, nXct, rtcnt;

	TCHAR	hdBuf[MAX_STRING];

	ZeroMemory( hdBuf, sizeof(hdBuf) );

	nowCaret = 0;
	rtcnt = 0;

	for( nYct = 0, nXct = 0; nowCaret <= cchSize; nowCaret++, nXct++ )
	{
		if( nXct >= MAX_STRING )	nXct = MAX_STRING - 1;

		hdBuf[nXct] = ptCont[nowCaret];

		if( ( TEXT('\r') == ptCont[nowCaret] && TEXT('\n') == ptCont[nowCaret + 1]) || 0x0000 == ptCont[nowCaret] )
		{
			if( 0 == nXct )	continue;
			hdBuf[nXct] = 0x0000;

			pfCalling( NULL, hdBuf, nXct );

			nXct = -1;
			nYct = 0;
			nowCaret++;

			rtcnt++;
			ZeroMemory( hdBuf, sizeof(hdBuf) );
		}
	}

	return 1;
}

UINT TemplateGridFluctuate( HWND hLvWnd, INT dFluct )
{
	INT	clmCount, clmNew, i;
	LVCOLUMN	stLvColm;

	if( 0 == dFluct )	return 0;

	clmCount = Header_GetItemCount( ListView_GetHeader(hLvWnd) );

	TRACE( TEXT("컬럼 증감[%u][%d]"), clmCount, dFluct );

	if( 0 > dFluct && 1 >= clmCount )	return 0;

	clmNew =  clmCount + dFluct;

	if( 0 > dFluct )
	{
		for( i = clmCount; clmNew < i; i-- )
		{
			ListView_DeleteColumn( hLvWnd, (i-1) );
		}
	}

	if( 0 < dFluct )
	{
		ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
		stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		stLvColm.fmt      = LVCFMT_LEFT;
		stLvColm.pszText  = TEXT("아이템");
		stLvColm.cx       = 10;
		for( i = clmCount; clmNew > i; i++ )
		{
			stLvColm.iSubItem = i;
			ListView_InsertColumn( hLvWnd, i, &stLvColm );
		}
	}

	return clmNew;
}

LRESULT CALLBACK gpfLineCtgryProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
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
					SendMessage( ghTmpleWnd, WM_COMMAND, wParam, lParam );
					return 0;
			}
			break;

		case WM_MOUSEWHEEL:
			SendMessage( ghLvItemWnd, WM_MOUSEWHEEL, wParam, lParam );
			return 0;

	}

	return CallWindowProc( gpfOrigLineCtgryProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK gpfLineItemProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT	id;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_NOTIFY, Ltl_OnNotify );

#ifdef LTP_CLICK_NEW
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
			TRACE( TEXT("LTL_MOUSenAN") );
			return 0;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
			Ltl_OnMouseButtonUp( hWnd, msg, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(wParam) );
			return 0;
#endif
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
					SendMessage( ghTmpleWnd, WM_COMMAND, wParam, lParam );
					return 0;
			}
			break;
	}

	return CallWindowProc( gpfOrigLineItemProc, hWnd, msg, wParam, lParam );
}

#ifdef LTP_CLICK_NEW

VOID Ltl_OnMouseButtonUp( HWND hWnd, UINT msg, INT x, INT y, UINT keyFlags )
{
	INT		iPos, iItem, iSubItem;
	INT_PTR	items;

	LVHITTESTINFO	stHitTestInfo;

	TRACE( TEXT("LTL_MOUSEB %d x %d"), x, y );

	ZeroMemory( &stHitTestInfo, sizeof(LVHITTESTINFO) );
	stHitTestInfo.pt.x = x;
	stHitTestInfo.pt.y = y;
	ListView_SubItemHitTest( hWnd, &stHitTestInfo );

	iItem = stHitTestInfo.iItem;
	iSubItem = stHitTestInfo.iSubItem;
	iPos = iItem * gLnClmCnt + iSubItem;
	TRACE( TEXT("라인 템플릿[%d x %d][%d]"), iItem, iSubItem, iPos );

	if( 0 < gvcTmples.size() )
	{
		items = gvcTmples.at( gNowGroup ).vcItems.size( );

		if( 0 <= iPos && iPos <  items )
		{

			if( WM_LBUTTONUP == msg )
			{
				ViewInsertTmpleString( gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str(  ) );
				ViewFocusSet(  );
			}
			else if( WM_MBUTTONUP == msg )
			{
				LayerBoxVisibalise( GetModuleHandle(NULL), gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str( ), 0x00 );
			}
		}
	}
	else
	{
		ViewFocusSet(  );
	}

	return;
}

#endif

LRESULT Ltl_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
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
		TRACE( TEXT("LT_TOOL %u"), idFrom );
		if( IDLV_LT_ITEMVIEW == idFrom )
		{
			ZeroMemory( &stHitTestInfo, sizeof(LVHITTESTINFO) );
			GetCursorPos( &(stHitTestInfo.pt) );
			ScreenToClient( hLvWnd, &(stHitTestInfo.pt) );
			ListView_SubItemHitTest( hLvWnd, &stHitTestInfo );

			iItem = stHitTestInfo.iItem;
			iSubItem = stHitTestInfo.iSubItem;
			iPos = iItem * gLnClmCnt + iSubItem;

			TRACE( TEXT("LLvTT[%d]"), iPos );

			pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );
			pstDispInfo->lpszText = pstDispInfo->szText;

			if( 0 < gvcTmples.size( ) )
			{
				items = gvcTmples.at( gNowGroup ).vcItems.size( );

				if( 0 <= iPos && iPos <  items )
				{
					StringCchCopy( atItem, SUB_STRING, gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );
					iDot = ViewStringWidthGet( atItem );

					StringCchPrintf( pstDispInfo->szText, 80, TEXT("%s [%d 점]"), atItem, iDot );
				}
			}

			return 0;
		}
	}

	return CallWindowProc( gpfOrigLineItemProc, hWnd, WM_NOTIFY, (WPARAM)idFrom, (LPARAM)pstNmhdr );
}
