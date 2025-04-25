#include "stdafx.h"
#include "OrinrinEditor.h"

#ifdef MINI_TEMPLATE

#define MINITEMPLATE_CLASS	TEXT("MINI_TEMPLATE")
#define MT_WIDTH	240
#define MT_HEIGHT	240

extern HFONT	ghAaFont;

extern INT		gbTmpltDock;
extern BOOLEAN	gbDockTmplView;

extern  HWND	ghMainSplitWnd;
extern  LONG	grdSplitPos;

static  ATOM	gMnTmplAtom;
static  HWND	ghMnTmplWnd;
static  HWND	ghTitleBxWnd;
static  HWND	ghItemStcWnd;

static  HWND	ghMainWnd;

static WNDPROC	gpfOrigMmaaTitleProc;
static WNDPROC	gpfOrigMmaaItemProc;

static  vector<AAMATRIX>	gvcMmaaTmpls;

LRESULT	CALLBACK MmaaTmpleProc( HWND, UINT, WPARAM, LPARAM );
VOID	Mma_OnCommand( HWND, INT, HWND, UINT );
VOID	Mma_OnSize( HWND, UINT, INT, INT );
LRESULT	Mma_OnNotify( HWND, INT, LPNMHDR );
VOID	Mma_OnContextMenu( HWND, HWND, UINT, UINT );

UINT	CALLBACK MmaaTmpleItemData( LPTSTR, LPCTSTR, INT );

UINT	MmaaTmpleItemListOn( UINT );
HRESULT	MmaaTmpleItemReload( HWND );

LRESULT	CALLBACK gpfMmaaTitleProc( HWND, UINT, WPARAM, LPARAM );
LRESULT	CALLBACK gpfMmaaItemProc(  HWND, UINT, WPARAM, LPARAM );
LRESULT	Mlv_OnNotify( HWND, INT, LPNMHDR );

HWND BrushTmpleInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame, HWND hMaaWnd )
{
	DWORD		dwExStyle, dwStyle;
	HWND		hPrWnd;
	UINT_PTR	dItems, i;
	TCHAR		atBuffer[MAX_STRING];

	INT		spPos;

	WNDCLASSEX	wcex;
	RECT		wdRect, clRect, rect, cbxRect, tbRect, mtbRect;
	LVCOLUMN	stLvColm;

	if( !(hInstance) && !(hParentWnd) )
	{

		return NULL;
	}

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MmaaTmpleProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= MINITEMPLATE_CLASS;
	wcex.hIconSm		= NULL;

	gMnTmplAtom = RegisterClassEx( &wcex );

	ghMainWnd = hParentWnd;

	InitWindowPos( INIT_LOAD, WDP_MMAATPL, &rect );
	if( 0 == rect.right || 0 == rect.bottom )
	{
		GetWindowRect( hParentWnd, &wdRect );
		rect.left   = wdRect.right + 64;
		rect.top    = wdRect.top + 64;
		rect.right  = MT_WIDTH;
		rect.bottom = MT_HEIGHT;
		InitWindowPos( INIT_SAVE , WDP_MMAATPL, &rect );
	}

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
		if( InitWindowTopMost( INIT_LOAD, WDP_MMAATPL, 0 ) ){	dwExStyle |=  WS_EX_TOPMOST;	}
		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
	}

	ghMnTmplWnd = CreateWindowEx( dwExStyle, MINITEMPLATE_CLASS, TEXT("미미 마 템플릿"),
		dwStyle, rect.left, rect.top, rect.right, rect.bottom, hPrWnd, NULL, hInstance, NULL);

	GetClientRect( ghMnTmplWnd, &clRect );

	ghTitleBxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT("마아 항목"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST,
		0, 0, clRect.right, 127, ghMnTmplWnd,
		(HMENU)IDCB_MT_CATEGORY, hInstance, NULL );

	gpfOrigMmaaTitleProc = SubclassWindow( ghTitleBxWnd, gpfMmaaTitleProc );

	GetClientRect( ghTitleBxWnd, &cbxRect );

	ghItemStcWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_STATIC, TEXT(""),
		WS_VISIBLE | WS_CHILD | SS_OWNERDRAW | SS_NOTIFY,
		0, cbxRect.bottom, clRect.right, clRect.bottom - cbxRect.bottom,
		ghMnTmplWnd, (HMENU)IDLV_MT_ITEMSTATIC, hInstance, NULL );
	SetWindowFont( ghItemStcWnd, ghAaFont, TRUE );

	gpfOrigMmaaItemProc = SubclassWindow( ghItemStcWnd, gpfMmaaItemProc );

	BrushTmpleItemListOn( 0 );

	if( !(gbTmpltDock) )
	{
		ShowWindow( ghMnTmplWnd, SW_SHOW );
		UpdateWindow( ghMnTmplWnd );
	}

	return ghMnTmplWnd;
}

VOID MmaaTmpleResize( HWND hPrntWnd, LPRECT pstFrame )
{
	RECT	rect, tbRect;

	if( !(ghMnTmplWnd) )	return;

	if( !(gbDockTmplView) )	return;

	rect = *pstFrame;
	rect.left    = rect.right - (grdSplitPos - SPLITBAR_WIDTH);
	rect.right   = (grdSplitPos - SPLITBAR_WIDTH);
	rect.bottom >>= 1;
	rect.top    += rect.bottom;

	DockingTabSizeGet( &tbRect );
	rect.top    += tbRect.bottom;
	rect.bottom -= tbRect.bottom;

	SetWindowPos( ghMnTmplWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER );

	return;
}

LRESULT CALLBACK MmaaTmpleProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_SIZE,        Mma_OnSize );
		HANDLE_MSG( hWnd, WM_COMMAND,     Mma_OnCommand );
		HANDLE_MSG( hWnd, WM_NOTIFY,      Mma_OnNotify );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Mma_OnContextMenu );

		case WM_MOUSEWHEEL:	SendMessage( ghItemStcWnd, WM_MOUSEWHEEL, wParam, lParam );	return 0;

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
#error 작업 중
	switch( id )
	{
		case IDCB_MT_CATEGORY:
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

		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_MMAA,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_MMAA, -1 );	break;

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
			TRACE( TEXT("브러시 템플릿[%d x %d]"), iItem, iSubItem );

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

LRESULT CALLBACK gpfMmaaItemProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{

		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Mai_OnMouseMove );
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Mai_OnLButtonUp );

		HANDLE_MSG( hWnd, WM_DROPFILES,   Mai_OnDropFiles );

#ifdef USE_HOVERTIP
		case WM_MOUSEHOVER:
			HoverTipOnMouseHover( hWnd, wParam, lParam, MmaaItemsHoverTipInfo );
			return 0;

		case WM_MOUSELEAVE:
			HoverTipOnMouseLeave( hWnd );
			gixNowToolTip = -1;
			return 0;
#endif

		default:	break;
	}

	return CallWindowProc( gpfOrigMmaaItemProc, hWnd, msg, wParam, lParam );
}

#endif
