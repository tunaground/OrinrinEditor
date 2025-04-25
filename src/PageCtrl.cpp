#include "stdafx.h"
#include "OrinrinEditor.h"

#define PAGELIST_CLASS	TEXT("PAGE_LIST")
#define PL_WIDTH	110
#define PL_HEIGHT	300

extern FILES_ITR	gitFileIt;

extern INT		gixFocusPage;
extern INT		gixDropPage;

static HINSTANCE	ghInst;

static  ATOM	gPageAtom;
static  HWND	ghPageWnd;

static  HWND	ghToolWnd;
static  HWND	ghPageListWnd;

#ifdef PGL_TOOLTIP
static  HWND	ghPageTipWnd;
static HFONT	ghPgTipFont;
static LPTSTR	gptPgTipBuf;
#endif
static BOOLEAN	gbPgTipView;

static INT		gixPreviSel;

static INT		gixMouseSel;
#ifdef PGL_TOOLTIP
static INT	gixPreSel;
#endif

static BOOLEAN	gbPgRetFocus;

static WNDPROC	gpfOrigPageViewProc;
static WNDPROC	gpfOrigPageToolProc;

static HIMAGELIST	ghPgLstImgLst;

extern INT	gbTmpltDock;
extern BOOLEAN	gbDockTmplView;

extern  UINT	gdPageByteMax;

extern  LONG	grdSplitPos;

#define PGTB_ITEMS	9
static TBBUTTON gstPgTlBarInfo[] = {
{  0,	IDM_PAGEL_ADD,		TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },
{  1,	IDM_PAGEL_INSERT,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },
{  2,	IDM_PAGEL_DUPLICATE,TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },
{  3,	IDM_PAGEL_DELETE,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },
{  4,	IDM_PAGEL_COMBINE,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },
{  5,	IDM_PAGEL_UPFLOW,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },
{  6,	IDM_PAGEL_DOWNSINK,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },
{  7,	IDM_PAGEL_RENAME,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },
{  8,	IDM_PAGEL_DETAIL,	TBSTATE_WRAP,					BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  }
};

LRESULT	CALLBACK PageListProc( HWND, UINT, WPARAM, LPARAM );
VOID	Plt_OnCommand( HWND, INT, HWND, UINT );
VOID	Plt_OnSize( HWND, UINT, INT, INT );
LRESULT	Plt_OnNotify( HWND, INT, LPNMHDR );
VOID	Plt_OnContextMenu( HWND, HWND, UINT, UINT );

LRESULT	PageListNotify( HWND, LPNMLISTVIEW );
HRESULT	PageListNameChange( INT );
HRESULT	PageListSpinning( HWND, INT, INT );
HRESULT	PageListDuplicate( HWND, INT );
HRESULT PageListCombine( HWND, INT );

HRESULT	PageListJump( INT );

LRESULT	CALLBACK gpfPageViewProc( HWND, UINT, WPARAM, LPARAM );
VOID	Plv_OnMouseMove( HWND, INT, INT, UINT );
#ifdef PGL_TOOLTIP
LRESULT	Plv_OnNotify( HWND , INT, LPNMHDR );
#endif

LRESULT	CALLBACK gpfPageToolProc( HWND, UINT, WPARAM, LPARAM );

INT_PTR	CALLBACK PageNameDlgProc( HWND, UINT, WPARAM, LPARAM );

#ifdef USE_HOVERTIP
LPTSTR	CALLBACK PageListHoverTipInfo( LPVOID );
#endif

HWND PageListInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame )
{

	LVCOLUMN	stLvColm;
	RECT		tbRect;
	DWORD		dwExStyle, dwStyle;
	TCHAR		atBuff[MAX_STRING];
	HWND		hPrWnd;

	UINT		ici, resnum;
	HBITMAP		hImg, hMsq;
	INT			spPos;

#ifdef PGL_TOOLTIP
	TTTOOLINFO	stToolInfo;
	LOGFONT	stFont;
#endif
	WNDCLASSEX	wcex;
	RECT	wdRect, clRect, rect;

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= PageListProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= PAGELIST_CLASS;
	wcex.hIconSm		= NULL;

	gPageAtom = RegisterClassEx( &wcex );

	ghInst = hInstance;

	gixMouseSel = -1;
#ifdef PGL_TOOLTIP
	gixPreSel = -1;
#endif
	gixPreviSel = -1;

	gbPgTipView = InitParamValue( INIT_LOAD, VL_PAGETIP_VIEW, 1 );

	gbPgRetFocus = InitParamValue( INIT_LOAD, VL_PGL_RETFCS, 0 );

	InitWindowPos( INIT_LOAD, WDP_PLIST, &rect );
	if( 0 == rect.right || 0 == rect.bottom )
	{
		GetWindowRect( hParentWnd, &wdRect );
		rect.left   = wdRect.left - PL_WIDTH;
		rect.top    = wdRect.top;
		rect.right  = PL_WIDTH;
		rect.bottom = PL_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_PLIST, &rect );
	}

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
	}
	else
	{
		dwExStyle = WS_EX_TOOLWINDOW;
		if( InitWindowTopMost( INIT_LOAD, WDP_PLIST , 0 ) ){	dwExStyle |=  WS_EX_TOPMOST;	}
		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
		hPrWnd = NULL;
	}
	ghPageWnd = CreateWindowEx( dwExStyle, PAGELIST_CLASS, TEXT("Page List"), dwStyle,
		rect.left, rect.top, rect.right, rect.bottom, hPrWnd, NULL, hInstance, NULL);

	GetClientRect( ghPageWnd, &clRect );

	ghToolWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("pagetoolbar"),
		WS_CHILD | WS_VISIBLE | CCS_NORESIZE | CCS_LEFT | CCS_NODIVIDER | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,
		0, 0, 0, 0, ghPageWnd, (HMENU)IDTB_PAGE_TOOLBAR, hInstance, NULL);

	SendMessage( ghToolWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	ghPgLstImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 9, 1 );
	resnum = IDBMPQ_PAGE_TB_FIRST;
	for( ici = 0; 9 > ici; ici++ )
	{
		hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghPgLstImgLst, hImg, hMsq );
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	SendMessage( ghToolWnd, TB_SETIMAGELIST, 0, (LPARAM)ghPgLstImgLst );

	SendMessage( ghToolWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuff, MAX_STRING, TEXT("末尾に新規作成\r\nAlt + Shift + I ") );	gstPgTlBarInfo[ 0].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("選択頁の次に挿入\r\nAlt + I") );			gstPgTlBarInfo[ 1].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("選択頁を複製\r\nAlt + C") );				gstPgTlBarInfo[ 2].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("選択頁を削除\r\nAlt + D") );				gstPgTlBarInfo[ 3].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("次の頁と統合\r\nAlt + G") );				gstPgTlBarInfo[ 4].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("頁を上へ移動\r\nAlt + U") );				gstPgTlBarInfo[ 5].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("頁を下へ移動\r\nAlt + J") );				gstPgTlBarInfo[ 6].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("頁名称の変更\r\nAlt + N") );				gstPgTlBarInfo[ 7].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("最新の情報に更新") );						gstPgTlBarInfo[ 8].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );

	SendMessage( ghToolWnd, TB_SETROWS, MAKEWPARAM(PGTB_ITEMS,TRUE), (LPARAM)(&tbRect) );

	SendMessage( ghToolWnd, TB_ADDBUTTONS, (WPARAM)PGTB_ITEMS, (LPARAM)&gstPgTlBarInfo );

	SendMessage( ghToolWnd, TB_GETITEMRECT, 0, (LPARAM)(&tbRect) );
	MoveWindow( ghToolWnd, 0, 0, tbRect.right, rect.bottom, TRUE );
	InvalidateRect( ghToolWnd , NULL, TRUE );

	gpfOrigPageToolProc = SubclassWindow( ghToolWnd, gpfPageToolProc );

	tbRect.bottom  = rect.bottom;
	tbRect.left    = 0;
	tbRect.top     = 0;

	ghPageListWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("pagelist"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
#ifdef PAGE_MULTISELECT
		LVS_REPORT | LVS_NOSORTHEADER,
#else
		LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL,
#endif
		tbRect.right, clRect.top, clRect.right - tbRect.right, clRect.bottom, ghPageWnd,
		(HMENU)IDLV_PAGELISTVIEW, hInstance, NULL );
	ListView_SetExtendedListViewStyle( ghPageListWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	gpfOrigPageViewProc = SubclassWindow( ghPageListWnd, gpfPageViewProc );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt = LVCFMT_LEFT;
	stLvColm.pszText = TEXT("No");		stLvColm.cx =  28;	stLvColm.iSubItem = 0;	ListView_InsertColumn( ghPageListWnd, 0, &stLvColm );
	stLvColm.pszText = TEXT("名");		stLvColm.cx =  67;	stLvColm.iSubItem = 1;	ListView_InsertColumn( ghPageListWnd, 1, &stLvColm );
	stLvColm.pszText = TEXT("Byte");	stLvColm.cx =  45;	stLvColm.iSubItem = 2;	ListView_InsertColumn( ghPageListWnd, 2, &stLvColm );
	stLvColm.pszText = TEXT("Line");	stLvColm.cx =  45;	stLvColm.iSubItem = 3;	ListView_InsertColumn( ghPageListWnd, 3, &stLvColm );

#ifdef PGL_TOOLTIP
	ghPageTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghPageWnd, NULL, hInstance, NULL );

	ViewingFontGet( &stFont );
	stFont.lfHeight = FONTSZ_REDUCE;
	ghPgTipFont = CreateFontIndirect( &stFont );
	SetWindowFont( ghPageTipWnd, ghPgTipFont, TRUE );

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;
	stToolInfo.hwnd     = ghPageListWnd;
	stToolInfo.uId      = IDLV_PAGELISTVIEW;
	GetClientRect( ghPageListWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage( ghPageTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghPageTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );

#endif

	ShowWindow( ghPageWnd, SW_SHOW );
	UpdateWindow( ghPageWnd );

	return ghPageWnd;
}

LRESULT CALLBACK gpfPageToolProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if( SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0 ){	ReleaseCapture(   );	}
			return 0;
	}

	return CallWindowProc( gpfOrigPageToolProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK PageListProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_COMMAND,     Plt_OnCommand );
		HANDLE_MSG( hWnd, WM_SIZE,        Plt_OnSize );
		HANDLE_MSG( hWnd, WM_NOTIFY,      Plt_OnNotify );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Plt_OnContextMenu );

		case WM_DESTROY:
#ifdef PGL_TOOLTIP
			SetWindowFont( ghPageTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
			DeleteFont( ghPgTipFont );
			FREE( gptPgTipBuf );
#endif
			ImageList_Destroy( ghPgLstImgLst );
			return 0;

		case WM_CLOSE:	ShowWindow( ghPageWnd, SW_HIDE );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Plt_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	INT	iPage, iItem, mRslt, iDiff;
	LONG_PTR	rdExStyle;
#ifdef PAGE_MULTISELECT
	INT	iNxItem = 0, iCount, i;
#endif

	switch( id )
	{
		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_PAGE,  1 );	return;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_PAGE, -1 );	return;

		case IDM_PAGEL_ADD:
			iPage = DocPageCreate( -1 );
			PageListInsert( iPage );
			DocPageChange( iPage );
			DocModifyContent( TRUE );
			DocFileBackup( hWnd );
			ViewFocusSet(  );
			return;

		case IDM_TOPMOST_TOGGLE:
			rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )
			{
				SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_PLIST, 0 );
			}
			else
			{
				SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_PLIST, 1 );
			}
			return;

		case IDM_PAGEL_AATIP_TOGGLE:
			gbPgTipView = gbPgTipView ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_PAGETIP_VIEW, gbPgTipView );
			return;

		case IDM_PAGEL_RETURN_FOCUS:
			gbPgRetFocus = gbPgRetFocus ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_PGL_RETFCS, gbPgRetFocus );
			return;

		default:	break;
	}

	iItem = ListView_GetNextItem( ghPageListWnd, -1, LVNI_ALL | LVNI_SELECTED );
#ifdef PAGE_MULTISELECT
	iCount = ListView_GetItemCount( ghPageListWnd );

	if( 0 <= iItem )
	{
		iNxItem = ListView_GetNextItem( ghPageListWnd, iItem, LVNI_ALL | LVNI_SELECTED );
	}
#endif

	if( 0 >  iItem ){	iItem = gixFocusPage;	}

	switch( id )
	{
		case IDM_PAGE_PREV:
			iDiff = iItem - 1;
			PageListJump( iDiff );
			return;

		case IDM_PAGE_NEXT:
			iDiff = iItem + 1;
			PageListJump( iDiff );
			return;

		case IDM_PAGEL_INSERT:
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}
#endif
			iPage = DocPageCreate( iItem );
			PageListInsert( iPage );
			DocPageChange( iPage );
			DocFileBackup( hWnd );
			break;

		case IDM_PAGEL_RENAME:
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}
#endif
			if( FAILED( PageListNameChange( iItem ) ) ){	 return;	}
			break;

		case IDM_PAGEL_DELETE:
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem )
			{
				mRslt = MessageBox( hWnd, TEXT("複数の頁を削除しようとしてるよ。\r\n本当に削除していいのかい？"), TEXT("お燐からの確認"), MB_YESNO | MB_DEFBUTTON2 );
				if( IDYES == mRslt )
				{
					for( i = 0; iCount > i; i++ )
					{
						iItem = ListView_GetNextItem( ghPageListWnd, -1, LVNI_ALL | LVNI_SELECTED );
						if( 0 > iItem ){	break;	}
						DocPageDelete( iItem , -1 );
					}
				}
			}
			else
			{
#endif
				mRslt = MessageBoxCheckBox( hWnd, ghInst, 2 );
				if( IDYES == mRslt ){	DocPageDelete( iItem , -1 );	}
#ifdef PAGE_MULTISELECT
			}
#endif
			break;

		case IDM_PAGEL_DIVIDE:
			break;

		case IDM_PAGEL_COMBINE:
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}
#endif

			mRslt = MessageBoxCheckBox( hWnd, ghInst, 0 );
			if( IDYES == mRslt ){	PageListCombine( hWnd , iItem );	}
			break;

		case IDM_PAGEL_UPFLOW:
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}
#endif
			PageListSpinning( hWnd, iItem, 1 );
			break;

		case IDM_PAGEL_DOWNSINK:
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}
#endif
			PageListSpinning( hWnd, iItem, -1 );
			break;

		case IDM_PAGEL_DUPLICATE:
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}
#endif
			PageListDuplicate( hWnd, iItem );
			break;

		case IDM_PAGEL_DETAIL:
			TRACE( TEXT("最新の情報に更新") );
			return;

		default:	TRACE( TEXT("未実装[%d]"), id );	return;
	}

	DocModifyContent( TRUE );

	ViewFocusSet(  );

	return;
}

VOID Plt_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	RECT	tbRect;
#ifdef PGL_TOOLTIP
	TTTOOLINFO	stToolInfo;
#endif

	tbRect.right = 0;
	if( ghToolWnd )
	{
		SendMessage( ghToolWnd, TB_GETITEMRECT, 0, (LPARAM)(&tbRect) );
		MoveWindow( ghToolWnd, 0, 0, tbRect.right, cy, TRUE );
	}

	MoveWindow( ghPageListWnd, tbRect.right, 0, cx - tbRect.right, cy, TRUE );

#ifdef PGL_TOOLTIP

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize = sizeof(TTTOOLINFO);
	stToolInfo.hwnd   = ghPageListWnd;
	stToolInfo.uId    = IDLV_PAGELISTVIEW;
	GetClientRect( ghPageListWnd, &stToolInfo.rect );
	SendMessage( ghPageTipWnd, TTM_NEWTOOLRECT, 0, (LPARAM)&stToolInfo );
#endif
	return;
}

LRESULT Plt_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{

	if( IDLV_PAGELISTVIEW == idFrom ){	return PageListNotify( hWnd, (LPNMLISTVIEW)pstNmhdr );	}

	return 0;
}

VOID Plt_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		iCount, iItem;
	BOOLEAN	bSel;
	LONG_PTR	rdExStyle;

	POINT	stPoint;

	stPoint.x = (SHORT)xPos;
	stPoint.y = (SHORT)yPos;

	bSel = FALSE;
	iCount  = ListView_GetItemCount( ghPageListWnd );
	iItem = ListView_GetNextItem( ghPageListWnd, -1, LVNI_ALL | LVNI_SELECTED);
	if( 0 <= iItem )	bSel = TRUE;

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_PGLVPOPUPMENU) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	if( 1 >= iCount ){	EnableMenuItem( hSubMenu, IDM_PAGEL_DELETE, MF_GRAYED );	}

	if( gbTmpltDock )
	{
		EnableMenuItem( hSubMenu, IDM_TOPMOST_TOGGLE, MF_GRAYED );
	}
	else
	{

		rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
		if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}
	}

	if( gbPgTipView ){	CheckMenuItem( hSubMenu, IDM_PAGEL_AATIP_TOGGLE, MF_CHECKED );	}

	if( gbPgRetFocus ){	CheckMenuItem( hSubMenu, IDM_PAGEL_RETURN_FOCUS, MF_CHECKED );	}

	if( !(bSel) )
	{
		EnableMenuItem( hSubMenu, IDM_PAGEL_INSERT, MF_GRAYED );
		EnableMenuItem( hSubMenu, IDM_PAGEL_DELETE, MF_GRAYED );
		EnableMenuItem( hSubMenu, IDM_PAGEL_COMBINE, MF_GRAYED );
		EnableMenuItem( hSubMenu, IDM_PAGEL_UPFLOW, MF_GRAYED );
		EnableMenuItem( hSubMenu, IDM_PAGEL_DOWNSINK, MF_GRAYED );
		EnableMenuItem( hSubMenu, IDM_PAGEL_DUPLICATE, MF_GRAYED );
		EnableMenuItem( hSubMenu, IDM_PAGEL_RENAME, MF_GRAYED );
	}

	dRslt = TrackPopupMenu( hSubMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );

	DestroyMenu( hMenu );

	return;
}

HRESULT PageListPositionReset( HWND hMainWnd )
{
	RECT	wdRect, rect;

	GetWindowRect( hMainWnd, &wdRect );
	rect.left   = wdRect.left - PL_WIDTH;
	rect.top    = wdRect.top;
	rect.right  = PL_WIDTH;
	rect.bottom = PL_HEIGHT;

	SetWindowPos( ghPageWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW | SWP_NOZORDER );

	return S_OK;
}

VOID PageListResize( HWND hPrntWnd, LPRECT pstFrame )
{
	RECT	rect, tbRect;

	rect = *pstFrame;
	rect.left  = rect.right - (grdSplitPos - SPLITBAR_WIDTH);
	rect.right = (grdSplitPos - SPLITBAR_WIDTH);
	if( gbDockTmplView )
	{
		rect.bottom >>= 1;
	}
	else
	{
		DockingTabSizeGet( &tbRect );
		rect.bottom -= tbRect.bottom;
	}

	SetWindowPos( ghPageWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW );

	return;
}

LRESULT PageListNotify( HWND hWnd, LPNMLISTVIEW pstLv )
{
	HWND	hLvWnd;
	INT		iCount, iItem, nmCode;

	INT		iSel;

	DWORD	lvClmn;
	INT		lvLine;
	LPNMLVCUSTOMDRAW	pstDraw;

	PAGEINFOS	stInfo;

	hLvWnd = pstLv->hdr.hwndFrom;
	nmCode = pstLv->hdr.code;

	iCount = ListView_GetItemCount( hLvWnd );
	iItem  = pstLv->iItem;

	if( NM_CLICK == nmCode )
	{
		if( 0 <= iItem )
		{
			TRACE( TEXT("ページ選択[%d]"), iItem );
			DocPageChange( iItem );

			if( gbPgRetFocus  ){	ViewFocusSet(  );	}
		}
	}

	if( NM_DBLCLK == nmCode )
	{
		if( 0 <= iItem )
		{
			if( SUCCEEDED( PageListNameChange( iItem ) ) )
			{
				DocModifyContent( TRUE );
			}
		}
	}

	if( NM_RETURN == nmCode )
	{

		iSel = ListView_GetNextItem( hLvWnd, -1, LVNI_ALL | LVNI_SELECTED );
		if( 0 > iSel )	return 0;

		TRACE( TEXT("NM_RETURN[%d]"), iSel );
	}

	if( NM_CUSTOMDRAW == nmCode )
	{
		pstDraw = (LPNMLVCUSTOMDRAW)pstLv;

		if( pstDraw->nmcd.dwDrawStage == CDDS_PREPAINT ){		return CDRF_NOTIFYSUBITEMDRAW;	}
		if( pstDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT ){	return CDRF_NOTIFYSUBITEMDRAW;	}

		if( pstDraw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT|CDDS_SUBITEM) )
		{
			lvLine = pstDraw->nmcd.dwItemSpec;
			lvClmn = pstDraw->iSubItem;

			if( gixFocusPage == lvLine )
			{
				pstDraw->clrTextBk = 0x00FF8080;
			}
			else if( gixPreviSel == lvLine )
			{
				pstDraw->clrTextBk = 0x00CCCCDD;
			}
			else
			{
				pstDraw->clrTextBk = 0xFF000000;
			}

			if( 0 == lvClmn )
			{
				if( NowPageInfoGet( lvLine, NULL ) )
				{
					pstDraw->clrTextBk = 0x00C0C0C0;
				}

			}

			if( 2 == lvClmn )
			{
				stInfo.dMasqus = PI_BYTES;
				NowPageInfoGet( lvLine, &stInfo );

				if( gdPageByteMax <  (UINT)(stInfo.iBytes) )	pstDraw->clrTextBk = 0x000000FF;

			}

			return CDRF_NEWFONT;
		}
	}

	return 0;
}

HRESULT PageListClear( VOID )
{
	ListView_DeleteAllItems( ghPageListWnd );

	return S_OK;
}

HRESULT PageListViewChange( INT iPage, INT iPrePage )
{

	LONG	iItem;

	gixPreviSel = iPrePage;

	InvalidateRect( ghPageListWnd, NULL, TRUE );

	iItem = ListView_GetItemCount( ghPageListWnd );
	if( iItem <= iPage || 0 > iPage )	return E_OUTOFMEMORY;

	ListView_SetItemState( ghPageListWnd, iPage, LVIS_SELECTED, LVIS_SELECTED );

	ViewEditReset(  );

	gixDropPage = iPage;

	return S_OK;
}

HRESULT PageListInsert( INT iBefore )
{

	UINT	iItem, i;
	TCHAR	atBuffer[MIN_STRING];
	LVITEM	stLvi;

	iItem = ListView_GetItemCount( ghPageListWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask  = LVIF_TEXT;

	ZeroMemory( atBuffer, sizeof(atBuffer) );

	if( 0 > iBefore )
	{
		stLvi.iItem = iItem;

		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), iItem + 1 );
		stLvi.pszText  = atBuffer;
	}
	else
	{
		stLvi.iItem = iBefore;
		stLvi.pszText  = TEXT("");
	}

	stLvi.iSubItem = 0;
	ListView_InsertItem( ghPageListWnd, &stLvi );

	stLvi.pszText  = TEXT("");
	stLvi.iSubItem =  1;
	ListView_SetItem( ghPageListWnd, &stLvi );

	stLvi.pszText = TEXT("0");
	stLvi.iSubItem =  2;
	ListView_SetItem( ghPageListWnd, &stLvi );

	stLvi.pszText = TEXT("1");
	stLvi.iSubItem =  3;
	ListView_SetItem( ghPageListWnd, &stLvi );

	if( 0 <= iBefore )
	{

		iItem = ListView_GetItemCount( ghPageListWnd );
		for( i = iBefore; iItem > i; i++ )
		{
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), i + 1 );
			ListView_SetItemText( ghPageListWnd, i, 0, atBuffer );
		}
	}

	return S_OK;
}

HRESULT PageListBuild( LPVOID pVoid )
{
	INT			i, iLastPage;
	PAGE_ITR	itPage;
	LVITEM		stLvi;
	TCHAR		atBuffer[MIN_STRING];

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask  = LVIF_TEXT;

#pragma message ("頁一覧再構成・項目注意")

	i = 0;
	for( itPage = (*gitFileIt).vcCont.begin(); itPage != (*gitFileIt).vcCont.end(); itPage++ )
	{
		stLvi.iItem    = i;
		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), i + 1 );
		stLvi.pszText  = atBuffer;
		stLvi.iSubItem = 0;
		ListView_InsertItem( ghPageListWnd, &stLvi );

		stLvi.pszText  = itPage->atPageName;
		stLvi.iSubItem =  1;
		ListView_SetItem( ghPageListWnd, &stLvi );

		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), itPage->dByteSz );
		stLvi.pszText  = atBuffer;
		stLvi.iSubItem =  2;
		ListView_SetItem( ghPageListWnd, &stLvi );

		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), itPage->ltPage.size() );
		stLvi.iSubItem =  3;
		ListView_SetItem( ghPageListWnd, &stLvi );

		i++;
	}

	iLastPage = (*gitFileIt).dNowPage;
	ListView_EnsureVisible( ghPageListWnd, iLastPage, FALSE );

	return S_OK;
}

HRESULT PageListSpinning( HWND hWnd, INT iPage, INT bDir )
{
	INT	iItem, i = 0;
	PAGE_ITR	itPage, itSwap;

	iItem = ListView_GetItemCount( ghPageListWnd );

	if( 0 == bDir ){	return E_INVALIDARG;	}

	if( (0 == iPage) && (0 < bDir) ){	return  E_ABORT;	}

	if( (iItem <= (iPage+1)) && (0 > bDir) ){	return  E_ABORT;	}

	TRACE( TEXT("頁移動処理[%d]"), iPage );

	itPage = (*gitFileIt).vcCont.begin(  );
	std::advance( itPage, iPage );

	if( 0 <  bDir ){	itSwap = itPage - 1;	i = iPage - 1;	}
	if( 0 >  bDir ){	itSwap = itPage + 1;	i = iPage + 1;	}

	iter_swap( itPage, itSwap );

	PageListViewRewrite( iPage );
	PageListViewRewrite( i );

	ListView_SetItemState( ghPageListWnd, i, LVIS_SELECTED, LVIS_SELECTED );

	DocPageChange( i );

	return S_OK;
}

HRESULT PageListDelete( INT iPage )
{
	UINT	iItem, i;
	TCHAR	atBuffer[MIN_STRING];

	ListView_DeleteItem( ghPageListWnd, iPage );

	iItem = ListView_GetItemCount( ghPageListWnd );
	for( i = 0; iItem > i; i++ )
	{
		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), i + 1 );
		ListView_SetItemText( ghPageListWnd, i, 0, atBuffer );
	}

	return S_OK;
}

HRESULT PageListInfoSet( INT iPage, INT dByte, INT dLine )
{
	INT		iPageCnt;
	TCHAR	atBuffer[MIN_STRING];

	iPageCnt = ListView_GetItemCount( ghPageListWnd );
	if( iPageCnt <= iPage ){	return E_OUTOFMEMORY;	}

	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), dByte );
	ListView_SetItemText( ghPageListWnd, iPage, 2, atBuffer );

	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), dLine );
	ListView_SetItemText( ghPageListWnd, iPage, 3, atBuffer );

	return S_OK;
}

HRESULT PageListViewRewrite( INT iPage )
{
#pragma message ("頁一覧再描画・項目注意")
	UINT_PTR	dLines;
	UINT		dBytes;
	INT			iPageCount, i;
	TCHAR	atBuffer[MIN_STRING];

	iPageCount = ListView_GetItemCount( ghPageListWnd );
	if( iPageCount <= iPage )	return E_OUTOFMEMORY;

	if( 0 >  iPage )
	{
		for( i = 0; iPageCount > i; i++ )
		{
			PageListViewRewrite( i );
		}

		return S_OK;
	}

	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), iPage + 1 );
	ListView_SetItemText( ghPageListWnd, iPage, 0, atBuffer );

	ListView_SetItemText( ghPageListWnd, iPage, 1, (*gitFileIt).vcCont.at( iPage ).atPageName );

	dBytes = (*gitFileIt).vcCont.at( iPage ).dByteSz;
	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), dBytes );
	ListView_SetItemText( ghPageListWnd, iPage, 2, atBuffer );

	dLines = (*gitFileIt).vcCont.at( iPage ).ltPage.size( );
	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), dLines );
	ListView_SetItemText( ghPageListWnd, iPage, 3, atBuffer );

	return S_OK;
}

HRESULT PageListJump( INT iDiff )
{
	INT	iItem;

	if( 0 >  iDiff )	return E_OUTOFMEMORY;

	iItem = ListView_GetItemCount( ghPageListWnd );
	if( iItem <= iDiff )	return E_OUTOFMEMORY;

	ListView_EnsureVisible( ghPageListWnd, iDiff, FALSE );

	DocPageChange( iDiff );

	ViewFocusSet(  );

	return S_OK;
}

INT_PTR CALLBACK PageNameDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static INT cdPage;

	switch( message )
	{
		case WM_INITDIALOG:
			cdPage = lParam;
			Edit_SetText( GetDlgItem(hDlg,IDE_PAGENAME), (*gitFileIt).vcCont.at( cdPage ).atPageName );
			SetFocus( GetDlgItem(hDlg,IDE_PAGENAME) );
			return (INT_PTR)FALSE;

		case WM_COMMAND:
			if( IDOK == LOWORD(wParam) )
			{
				Edit_GetText( GetDlgItem(hDlg,IDE_PAGENAME), (*gitFileIt).vcCont.at( cdPage ).atPageName, SUB_STRING );
				EndDialog( hDlg, IDOK );
				return (INT_PTR)TRUE;
			}

			if( IDCANCEL == LOWORD(wParam) )
			{
				EndDialog( hDlg, IDCANCEL );
				return (INT_PTR)TRUE;
			}

			break;
	}
	return (INT_PTR)FALSE;
}

HRESULT PageListNameChange( INT dPage )
{
	INT_PTR	iRslt;

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_PAGE_NAME_DLG), ghPageWnd, PageNameDlgProc, dPage );
	if( IDOK == iRslt )
	{
		PageListNameSet( dPage, (*gitFileIt).vcCont.at( dPage ).atPageName );
		return S_OK;
	}

	return E_ABORT;
}

HRESULT PageListNameSet( INT dPage, LPTSTR ptName )
{
	INT		iPageCount;
	LVITEM	stLvi;

	iPageCount = ListView_GetItemCount( ghPageListWnd );
	if( iPageCount <= dPage )	return E_OUTOFMEMORY;

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask     = LVIF_TEXT;
	stLvi.iItem    = dPage;
	stLvi.pszText  = ptName;
	stLvi.iSubItem =  1;
	ListView_SetItem( ghPageListWnd, &stLvi );

	return S_OK;
}

HRESULT PageListNameRewrite( LPTSTR ptName )
{
	UINT_PTR	cchSize;

	StringCchLength( ptName, STRSAFE_MAX_CCH, &cchSize );

	if( SUB_STRING <= cchSize ){	ptName[(SUB_STRING-1)] = NULL;	}

	StringCchCopy( (*gitFileIt).vcCont.at( gixFocusPage ).atPageName, SUB_STRING, ptName );

	PageListNameSet( gixFocusPage, ptName );

	return S_OK;
}

INT PageListIsNamed( FILES_ITR itFile )
{
	UINT_PTR	iPageCount, i;

	iPageCount = itFile->vcCont.size(  );
	for( i = 0; iPageCount > i; i++ )
	{
		if( 0 != itFile->vcCont.at( i ).atPageName[0] ){	return TRUE;	}
	}

	return FALSE;
}

HRESULT PageListDuplicate( HWND hWnd, INT iNowPage )
{
	INT		iNewPage;

	LINE_ITR	itLine;

	TRACE( TEXT("頁複製") );

	iNewPage = DocPageCreate( iNowPage );
	PageListInsert( iNewPage  );

	(*gitFileIt).vcCont.at( iNewPage ).ltPage.clear(  );

	std::copy(	(*gitFileIt).vcCont.at( iNowPage ).ltPage.begin(),
				(*gitFileIt).vcCont.at( iNowPage ).ltPage.end(),
				back_inserter( (*gitFileIt).vcCont.at( iNewPage ).ltPage ) );

#pragma message ("PageListDuplicate 作った頁の内容の再計算いるか？")

	return S_OK;
}

HRESULT PageListCombine( HWND hWnd, INT iNowPage )
{
	 INT	iLastLine, iLastDot;
	 INT	iNext;
	INT_PTR	iTotal;
	ONELINE	stLine;
	LINE_ITR	itLine;

	ZeroONELINE( &stLine );

	TRACE( TEXT("頁統合") );

	iTotal = DocNowFilePageCount(  );

	if( 1 >= iTotal )	return E_ACCESSDENIED;

	iNext = iNowPage + 1;
	if( iTotal <= iNext ){	return E_OUTOFMEMORY;	}

	(*gitFileIt).vcCont.at( iNowPage ).ltPage.push_back( stLine );

	if( (*gitFileIt).vcCont.at( iNext ).ptRawData )
	{

		iLastLine = DocNowFilePageLineCount(  ) - 1;
		iLastDot  = DocLineParamGet( iLastLine, NULL, NULL );

		DocInsertString( &iLastDot, &iLastLine, NULL, (*gitFileIt).vcCont.at( iNext ).ptRawData, 0, TRUE );
	}
	else
	{

		std::copy(	(*gitFileIt).vcCont.at( iNext ).ltPage.begin(),
					(*gitFileIt).vcCont.at( iNext ).ltPage.end(),
					back_inserter( (*gitFileIt).vcCont.at( iNowPage ).ltPage ) );
	}

	SqnFreeAll( &((*gitFileIt).vcCont.at( iNowPage ).stUndoLog) );

	DocPageDelete( iNext , -1 );

	return S_OK;
}

LRESULT CALLBACK gpfPageViewProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_MOUSEMOVE, Plv_OnMouseMove );
		HANDLE_MSG( hWnd, WM_COMMAND,   Plt_OnCommand );
#ifdef PGL_TOOLTIP
		HANDLE_MSG( hWnd, WM_NOTIFY,    Plv_OnNotify  );
#endif

#ifdef USE_HOVERTIP
		case WM_MOUSEHOVER:
			HoverTipOnMouseHover( hWnd, wParam, lParam, PageListHoverTipInfo );
			return 0;

		case WM_MOUSELEAVE:
			HoverTipOnMouseLeave( hWnd );
			gixMouseSel = -1;
			return 0;
#endif
	}

	return CallWindowProc( gpfOrigPageViewProc, hWnd, msg, wParam, lParam );
}

VOID Plv_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	LVHITTESTINFO	stHitInfo;
	INT	iItem;
	BOOLEAN	bReDraw = FALSE;

	ZeroMemory( &stHitInfo, sizeof(LVHITTESTINFO) );
	stHitInfo.pt.x = 10;
	stHitInfo.pt.y = y;

	iItem = ListView_HitTest( hWnd, &stHitInfo );
	if( gixMouseSel != iItem )	bReDraw = TRUE;
	gixMouseSel = iItem;

#ifdef USE_HOVERTIP
	if( bReDraw ){	HoverTipResist( ghPageListWnd );	}
#endif

#ifdef PGL_TOOLTIP
	if( bReDraw ){	SendMessage( ghPageTipWnd, TTM_UPDATE, 0, 0 );	}
#endif

	return;
}

#ifdef PGL_TOOLTIP

LRESULT Plv_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	INT				dBytes;
	UINT_PTR		rdLength;
	LPNMTTDISPINFO	pstDispInfo;

	if( TTN_GETDISPINFO == pstNmhdr->code )
	{
		if( IDLV_PAGELISTVIEW == idFrom )
		{
			pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

			if( gixPreSel != gixMouseSel )
			{

				ZeroMemory( pstDispInfo->szText, sizeof(pstDispInfo->szText) );
				pstDispInfo->lpszText = NULL;

				FREE( gptPgTipBuf );

				if( !(gbPgTipView) ){	return 0;	}
				if( 0 > gixMouseSel ){	return 0;	}

				TRACE( TEXT("1 TTN_GETDISPINFO %d  %X"), gixMouseSel, pstDispInfo->uFlags );
				gixPreSel = gixMouseSel;

				dBytes = DocPageTextGetAlloc( gitFileIt, gixMouseSel, D_UNI, (LPVOID *)(&gptPgTipBuf), FALSE );

				if( gptPgTipBuf )
				{
					StringCchLength( gptPgTipBuf, STRSAFE_MAX_CCH, &rdLength );

					pstDispInfo->lpszText = gptPgTipBuf;
				}
			}
			else
			{
				TRACE( TEXT("2 TTN_GETDISPINFO %d  %X"), gixMouseSel, pstDispInfo->uFlags );

				ZeroMemory( pstDispInfo->szText, sizeof(pstDispInfo->szText) );
				pstDispInfo->lpszText = gptPgTipBuf;
			}

			return 0;
		}
	}

	return CallWindowProc( gpfOrigPageViewProc, hWnd, WM_NOTIFY, (WPARAM)idFrom, (LPARAM)pstNmhdr );

}

#endif

#ifdef USE_HOVERTIP

LPTSTR CALLBACK PageListHoverTipInfo( LPVOID pVoid )
{
	INT		dBytes;
	LPTSTR	ptBuffer = NULL;

	if( !(gbPgTipView) ){	return NULL;	}
	if( 0 > gixMouseSel ){	return NULL;	}

	dBytes = DocPageTextGetAlloc( gitFileIt, gixMouseSel, D_UNI, (LPVOID *)(&ptBuffer), FALSE );
	TRACE( TEXT("HOVER CALL %d, by[%d]"), gixMouseSel, dBytes );

	return ptBuffer;
}

#endif
