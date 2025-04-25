#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"

#ifdef _ORRVW
#define  MAATMPLT_CLASS_NAME	TEXT("ORINRINVIEWER")
#else
#define  MAATMPLT_CLASS_NAME	TEXT("MULTILINEAA_CLASS")
#endif
#define MA_WIDTH	320
#define MA_HEIGHT	320

#ifdef MAA_TEXT_FIND
#define TXTFIND_BARHEI	25
#endif

static HINSTANCE	ghInst;

static  HWND	ghMainWnd;

static  HWND	ghMaaWnd;

static  HWND	ghStsBarWnd;

EXTERNED HWND	ghSplitaWnd;

EXTERNED UINT	gbAAtipView;

EXTERNED HWND	ghMaaFindDlg;

static TCHAR	gatProfilePath[MAX_PATH];

static TCHAR	gatTemplatePath[MAX_PATH];

static list<OPENHIST>	gltProfHist;
EXTERNED HMENU	ghProfHisMenu;

static CONST INT	giStbRoom[] = { 150 , 350 , -1 };

LRESULT	CALLBACK MaaTmpltWndProc( HWND, UINT, WPARAM, LPARAM );
BOOLEAN	Maa_OnCreate( HWND, LPCREATESTRUCT );
VOID	Maa_OnPaint( HWND );
VOID	Maa_OnDestroy( HWND );
LRESULT	Maa_OnNotify( HWND , INT, LPNMHDR );
VOID	Maa_OnDrawItem( HWND, CONST DRAWITEMSTRUCT * );
VOID	Maa_OnMeasureItem( HWND, MEASUREITEMSTRUCT * );

#ifndef _ORRVW
VOID	Maa_OnActivate( HWND, UINT, HWND, BOOL );
VOID	Maa_OnShowWindow( HWND, BOOL, UINT );
#endif

#define TREEPROF_AUTOCHECK

INT_PTR	CALLBACK TreeProfileDlgProc( HWND, UINT, WPARAM, LPARAM );
HRESULT	TreeProfListUp( HWND, HWND, LPTSTR, HTREEITEM, UINT, INT );
UINT	TreeLoadNodeProc( HWND, HWND, HTREEITEM, UINT );
VOID	TreeProfCheckState( HWND, HTREEITEM, UINT );
#ifdef TREEPROF_AUTOCHECK
UINT	TreeProfCheckExistent( HWND, LPTSTR, HWND, HTREEITEM, UINT );
#endif

HWND MaaTmpltInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame )
{
	WNDCLASSEX	wcex;
	RECT	wdRect, rect;
#ifndef _ORRVW
	RECT	sbRect;
#endif
	INT		bMode = 0;

#ifdef _ORRVW
	INT	bTopMost;
#endif

	WIN32_FIND_DATA	stFindData;
	HANDLE	hFind;

	ghMainWnd  = hParentWnd;
	ghInst = hInstance;

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MaaTmpltWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= MAATMPLT_CLASS_NAME;
#ifdef _ORRVW
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ORINRINEDITOR));
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ORINRINVIEWER);
#else
	wcex.hIcon			= NULL;
	wcex.hIconSm		= NULL;
	wcex.lpszMenuName	= NULL;
#endif

	RegisterClassEx( &wcex );

	ghMaaFindDlg = NULL;

#ifdef _ORRVW
	SplitBarClass( hInstance );
#endif

	InitWindowPos( INIT_LOAD, WDP_MAATPL, &rect );
	if( 0 == rect.right || 0 == rect.bottom )
	{
		GetWindowRect( hParentWnd, &wdRect );
#ifdef _ORRVW
		rect.left = ( wdRect.right  - MA_WIDTH ) / 2;
		rect.top  = ( wdRect.bottom - MA_HEIGHT ) / 2;
#else
		rect.left   = wdRect.right + 64;
		rect.top    = wdRect.top + 64;
#endif
		rect.right  = MA_WIDTH;
		rect.bottom = MA_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_MAATPL, &rect );
	}

	ghMaaWnd = CreateWindowEx(
#ifdef _ORRVW
		0, MAATMPLT_CLASS_NAME, TEXT("Orinrin Viewer"),
		WS_OVERLAPPEDWINDOW,
#else
		WS_EX_TOOLWINDOW, MAATMPLT_CLASS_NAME, TEXT("Multi Line AA Template"),
		WS_POPUP | WS_THICKFRAME | WS_BORDER | WS_CAPTION,
#endif
		rect.left, rect.top, rect.right, rect.bottom,
		NULL, NULL, hInstance, NULL);

	if( !(ghMaaWnd) )	return NULL;

#ifdef _ORRVW
	bTopMost = InitParamValue( INIT_LOAD, VL_MAA_TOPMOST, 1 );
	if( bTopMost )
	{
		SetWindowPos( ghMaaWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
		CheckMenuItem( GetMenu(ghMaaWnd), IDM_TOPMOST_TOGGLE, MF_CHECKED );
	}

	ShowWindow( ghMaaWnd, SW_SHOW );
	UpdateWindow( ghMaaWnd );
#else
	if( InitParamValue( INIT_LOAD, VL_MAA_TOPMOST, 1 ) )
	{
		if( InitWindowTopMost( INIT_LOAD, WDP_MAATPL , 0 ) )
		{	SetWindowPos( ghMaaWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );	}

		ShowWindow( ghMaaWnd, SW_SHOW );
		UpdateWindow( ghMaaWnd );
	}
#endif

	OpenProfileInitialise( ghMaaWnd );

	ZeroMemory( gatTemplatePath, sizeof(gatTemplatePath) );

	ZeroMemory( gatProfilePath, sizeof(gatProfilePath) );
	InitParamString( INIT_LOAD, VS_PROFILE_NAME, gatProfilePath );

	hFind = FindFirstFile( gatProfilePath, &stFindData );
	if( INVALID_HANDLE_VALUE != hFind ){	FindClose( hFind  );	}
	else{		ZeroMemory( gatProfilePath, sizeof(gatProfilePath) );	};

	if( NULL != gatProfilePath[0] )
	{
		bMode = TreeProfileMake( ghMaaWnd, gatProfilePath );
		if( 0 > bMode ){	bMode = 0;	}
	}

	TreeConstruct( ghMaaWnd, gatTemplatePath, TRUE );

	gbAAtipView = InitParamValue( INIT_LOAD, VL_MAATIP_VIEW, 1 );

#ifndef _ORRVW

	MaaTabBarSizeGet( pstFrame );
	pstFrame->top = pstFrame->bottom;
	GetClientRect( ghStsBarWnd, &sbRect );
	pstFrame->bottom = sbRect.bottom;
#endif

	return ghMaaWnd;
}

HRESULT MaaTmpltPositionReset( HWND hMainWnd )
{
	RECT	wdRect, rect;

	GetWindowRect( hMainWnd, &wdRect );
	rect.left   = wdRect.right + 64;
	rect.top    = wdRect.top + 64;
	rect.right  = MA_WIDTH;
	rect.bottom = MA_HEIGHT;

	SetWindowPos( ghMaaWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW | SWP_NOZORDER );

	return S_OK;
}

#ifndef _ORRVW

BOOLEAN MaaViewToggle( UINT bSet )
{
	BOOL	bStyle;

	if( !(ghMaaWnd) )	return FALSE;

	bStyle = IsWindowVisible( ghMaaWnd );

	if( bSet )
	{
		if( bStyle )	ShowWindow( ghMaaWnd, SW_HIDE );
		else			ShowWindow( ghMaaWnd, SW_SHOW );

		bStyle = !(bStyle);
	}

	return bStyle;
}

#endif

LRESULT CALLBACK MaaTmpltWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	UINT	uRslt;
#ifdef MAA_TEXT_FIND
	INT		itemID;
	HDC		hdc;
	HWND	hWndChild;
#endif

	switch( message )
	{
		HANDLE_MSG( hWnd, WM_CREATE,      Maa_OnCreate );
		HANDLE_MSG( hWnd, WM_PAINT,       Maa_OnPaint );
		HANDLE_MSG( hWnd, WM_COMMAND,     Maa_OnCommand );
		HANDLE_MSG( hWnd, WM_DESTROY,     Maa_OnDestroy );
		HANDLE_MSG( hWnd, WM_NOTIFY,      Maa_OnNotify );
		HANDLE_MSG( hWnd, WM_SIZE,        Maa_OnSize  );
		HANDLE_MSG( hWnd, WM_CHAR,        Maa_OnChar  );
		HANDLE_MSG( hWnd, WM_DRAWITEM,    Maa_OnDrawItem  );
		HANDLE_MSG( hWnd, WM_MEASUREITEM, Maa_OnMeasureItem );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Maa_OnContextMenu );
#ifndef _ORRVW
		HANDLE_MSG( hWnd, WM_ACTIVATE,    Maa_OnActivate  );
		HANDLE_MSG( hWnd, WM_SHOWWINDOW,  Maa_OnShowWindow );
#endif
		HANDLE_MSG( hWnd, WM_KEYDOWN,     Aai_OnKey );
		HANDLE_MSG( hWnd, WM_KEYUP,       Aai_OnKey );

		HANDLE_MSG( hWnd, WM_VSCROLL,     Aai_OnVScroll );

#ifdef MAA_TEXT_FIND
		case  WM_CTLCOLORSTATIC:
			hdc = (HDC)(wParam);
			hWndChild = (HWND)(lParam);

			itemID = GetDlgCtrlID( hWndChild );

			if( IDS_MAA_TXTFIND_FIND   == itemID || IDS_MAA_TXTFIND_MSGBOX == itemID ||
				IDB_MAA_TXTFIND_TOP_GO == itemID || IDB_MAA_TXTFIND_NOW_GO == itemID )
			{
				SetBkColor( hdc, GetSysColor( COLOR_WINDOW ) );
				return (LRESULT)GetSysColorBrush( COLOR_WINDOW );
			}
			break;
#endif
		case WM_MOUSEWHEEL:
			uRslt = Maa_OnMouseWheel( hWnd, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (INT)(SHORT)HIWORD(wParam), (UINT)(SHORT)LOWORD(wParam) );
			break;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

#ifndef _ORRVW

VOID Maa_OnShowWindow( HWND hWnd, BOOL fShow, UINT status )
{
	INT	rslt;

	TRACE( TEXT("MAA SHOWWINDOW fShow[%u] status[%u]"), fShow, status );

	if( fShow )
	{
		rslt = InitParamValue( INIT_LOAD, VL_MAA_TOPMOST, 1 );
		if( !(rslt) )
		{
			ShowWindow( ghMaaWnd, SW_HIDE );
		}
	}

	return;
}

VOID Maa_OnActivate( HWND hWnd, UINT state, HWND hWndActDeact, BOOL fMinimized )
{

	BOOL		bStyle;

	bStyle = IsWindowVisible( ghMaaWnd );
	TRACE( TEXT("MAA ACTIVATE[%d] STATE[%u] HWND[%X][%X] MIN[%u]"), bStyle, state, hWndActDeact, ghMainWnd, fMinimized );

	return;
}

#endif

BOOLEAN Maa_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	HINSTANCE lcInst = lpCreateStruct->hInstance;
	RECT	rect, sbRect, tbRect;

	INT	iTfTop;
	INT	spPos;

	GetClientRect( hWnd, &rect );

	ghStsBarWnd = CreateStatusWindow( WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, TEXT(""), hWnd, IDSB_STATUSBAR );
	GetClientRect( ghStsBarWnd, &sbRect );
	rect.bottom -= sbRect.bottom;
	SendMessage( ghStsBarWnd, SB_SETPARTS, 3, (LPARAM)giStbRoom );

#ifdef MAA_TEXT_FIND

	rect.bottom -= TXTFIND_BARHEI;
	iTfTop = rect.bottom + 1;

	CreateWindowEx( 0, WC_STATIC, TEXT("検索"), WS_VISIBLE | WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, 1, iTfTop, 40, 23, hWnd, (HMENU)IDS_MAA_TXTFIND_FIND, lcInst, NULL );

	CreateWindowEx( 0, WC_EDIT, TEXT(""), WS_VISIBLE | WS_CHILD | WS_BORDER, 42, iTfTop, 139, 23, hWnd, (HMENU)IDE_MAA_TXTFIND_TEXT, lcInst, NULL );

	CreateWindowEx( 0, WC_BUTTON, TEXT("先頭から"), WS_VISIBLE | WS_CHILD | WS_GROUP | BS_AUTORADIOBUTTON | BS_VCENTER, 183, iTfTop, 80, 23, hWnd, (HMENU)IDB_MAA_TXTFIND_TOP_GO, lcInst, NULL );
	CreateWindowEx( 0, WC_BUTTON, TEXT("次頁から"), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_VCENTER, 265, iTfTop, 80, 23, hWnd, (HMENU)IDB_MAA_TXTFIND_NOW_GO, lcInst, NULL );
	CheckDlgButton( hWnd, IDB_MAA_TXTFIND_TOP_GO, BST_CHECKED );

	CreateWindowEx( 0, WC_BUTTON, TEXT("↓検索"), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 347, iTfTop, 65, 23, hWnd, (HMENU)IDB_MAA_TXTFIND_NEXT, lcInst, NULL );

	CreateWindowEx( 0, WC_STATIC, TEXT(""), WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE, 414, iTfTop, 120, 23, hWnd, (HMENU)IDS_MAA_TXTFIND_MSGBOX, lcInst, NULL );
#endif

	TreeInitialise( hWnd, lcInst, &rect );

	MaaTabBarSizeGet( &tbRect );

	spPos = InitParamValue( INIT_LOAD, VL_MAA_SPLIT, TMPL_DOCK );
	ghSplitaWnd = SplitBarCreate( lcInst, hWnd, spPos, tbRect.bottom, rect.bottom - tbRect.bottom );

	AaItemsInitialise( hWnd, lcInst, &rect );

	return TRUE;
}

VOID Maa_OnCommand( HWND hWnd, INT id, HWND hwndCtl, UINT codeNotify )
{
	LONG_PTR	rdExStyle;

#ifdef _ORRVW
	if( IDM_OPEN_HIS_FIRST <= id && id <= IDM_OPEN_HIS_LAST )
	{
		OpenProfileLoad( hWnd, id );
		return;
	}
	else if( IDM_OPEN_HIS_CLEAR ==  id )
	{
		OpenProfileLogging( hWnd, NULL );
		return;
	}
#endif

	switch( id )
	{

		case IDSO_AAITEMS:	TRACE( TEXT("static") );	break;

		case IDM_DRAUGHT_OPEN:	DraughtWindowCreate( GetModuleHandle(NULL), ghMaaWnd, 0 );	break;

		case IDM_MAA_THUMBNAIL_OPEN:	DraughtWindowCreate( GetModuleHandle(NULL), ghMaaWnd, 1 );	break;

		case IDLB_FAVLIST:	FavListSelected( hWnd, codeNotify );	break;

		case IDM_TOPMOST_TOGGLE:
			rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )
			{
				SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
#ifdef _ORRVW
				CheckMenuItem( GetMenu(hWnd), IDM_TOPMOST_TOGGLE, MF_UNCHECKED );
				InitParamValue( INIT_SAVE, VL_MAA_TOPMOST, 0 );
#else
				InitWindowTopMost( INIT_SAVE, WDP_MAATPL, 0 );
#endif
			}
			else
			{
				SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
#ifdef _ORRVW
				CheckMenuItem( GetMenu(hWnd), IDM_TOPMOST_TOGGLE, MF_CHECKED );
				InitParamValue( INIT_SAVE, VL_MAA_TOPMOST, 1 );
#else
				InitWindowTopMost( INIT_SAVE, WDP_MAATPL, 1 );
#endif
			}
#ifdef _ORRVW
			DrawMenuBar( hWnd );
#endif
			break;

		case IDCB_AAITEMTITLE:	AaTitleSelect( hWnd, codeNotify );	break;

		case IDM_FILE_CLOSE:	TabMultipleSelDelete( hWnd );	break;

#ifdef _ORRVW
		case IDM_MAAITEM_BKCOLOUR:	MaaBackColourChoose( hWnd );	break;

		case IDM_ABOUT:	DialogBox( ghInst , MAKEINTRESOURCE(IDD_ORRVWR_ABOUTBOX), hWnd, About );	break;
		case IDM_EXIT:	DestroyWindow( hWnd );	break;

		case IDM_GENERAL_OPTION:
			DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_ORRVWR_OPTION_DLG), hWnd, OptionDlgProc, NULL );
			break;

		case IDM_MAA_PROFILE_MAKE:	TreeProfileOpen( hWnd );	break;
		case IDM_TREE_RECONSTRUCT:	TreeProfileRebuild( hWnd  );	break;

#else
		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_MAAT,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_MAAT, -1 );	break;
#endif

		case IDM_FINDMAA_DLG_OPEN:	TreeMaaFileFind( hWnd );	break;

		case IDM_TMPLT_GROUP_NEXT:
		case IDM_TMPLT_GROUP_PREV:
			TRACE( TEXT("Ctrl＋↑↓") );
			break;

#ifdef MAA_TEXT_FIND
		case IDM_FIND_JUMP_NEXT:	AacFindTextEntry( hWnd, 0 );	break;

		case IDB_MAA_TXTFIND_NEXT:	AacFindTextEntry( hWnd, 1 );	break;
#endif
		default:	break;
	}

	return;
}

VOID Maa_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	EndPaint( hWnd, &ps );

	return;
}

VOID Maa_OnDestroy( HWND hWnd )
{
	RECT	rect;
	DWORD	dwStyle;

	dwStyle = GetWindowStyle( hWnd );
	if( !(dwStyle & WS_MINIMIZE) )
	{

		SplitBarPosGet( ghSplitaWnd, &rect );
		InitParamValue( INIT_SAVE, VL_MAA_SPLIT, rect.left );

		GetWindowRect( hWnd, &rect );
		rect.right  -= rect.left;
		rect.bottom -= rect.top;
		InitWindowPos( INIT_SAVE, WDP_MAATPL, &rect );
	}

	AaItemsInitialise( NULL, NULL, NULL );
	AacMatrixClear(   );

	TreeInitialise( NULL, NULL, NULL );

	SqlDatabaseOpenClose( M_DESTROY, NULL );

#ifdef _ORRVW

#ifdef USE_HOVERTIP
	HoverTipInitialise( NULL, NULL );
#endif
	DraughtInitialise( NULL, NULL );

	OpenProfileInitialise( NULL );

	PostQuitMessage( 0 );
#endif

	return;
}

VOID Maa_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	RECT	rect, sbRect, tbRect;
	INT	iTfTop;

	GetClientRect( hWnd, &rect );

	MoveWindow( ghStsBarWnd, 0, 0, 0, 0, TRUE );
	GetClientRect( ghStsBarWnd, &sbRect );
	rect.bottom -= sbRect.bottom;

#ifdef MAA_TEXT_FIND

	rect.bottom -= TXTFIND_BARHEI;
	iTfTop = rect.bottom + 1;

	SetWindowPos( GetDlgItem(hWnd,IDS_MAA_TXTFIND_FIND),   HWND_TOP,   1, iTfTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	SetWindowPos( GetDlgItem(hWnd,IDE_MAA_TXTFIND_TEXT),   HWND_TOP,  42, iTfTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	SetWindowPos( GetDlgItem(hWnd,IDB_MAA_TXTFIND_TOP_GO), HWND_TOP, 183, iTfTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	SetWindowPos( GetDlgItem(hWnd,IDB_MAA_TXTFIND_NOW_GO), HWND_TOP, 265, iTfTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	SetWindowPos( GetDlgItem(hWnd,IDB_MAA_TXTFIND_NEXT),   HWND_TOP, 347, iTfTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	SetWindowPos( GetDlgItem(hWnd,IDS_MAA_TXTFIND_MSGBOX), HWND_TOP, 414, iTfTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
#endif

	TabBarResize( hWnd, &rect );
	MaaTabBarSizeGet( &tbRect );
	rect.top = tbRect.bottom;
	rect.bottom -= tbRect.bottom;

	SplitBarResize( ghSplitaWnd, &rect );

	TreeResize( hWnd, &rect );
	AaItemsResize( hWnd, &rect );

	return;
}

LRESULT Maa_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{

	if( IDTV_ITEMTREE == idFrom ){	TreeNotify( hWnd , (LPNMTREEVIEW)pstNmhdr );	}

	if( IDTB_TREESEL == idFrom ){	TabBarNotify( hWnd , pstNmhdr );	}

	return 0;
}

VOID Maa_OnDrawItem( HWND hWnd, CONST DRAWITEMSTRUCT *pstDrawItem )
{
	if( IDSO_AAITEMS == pstDrawItem->CtlID )
	{
		AaItemsDrawItem( hWnd, pstDrawItem );
	}

	return;
}

VOID Maa_OnMeasureItem( HWND hWnd, MEASUREITEMSTRUCT *pstMeasureItem )
{
#if 0
	if( IDSO_AAITEMS == pstMeasureItem->CtlID )
	{
		AaItemsMeasureItem( hWnd, pstMeasureItem );
	}
#endif
	return;
}

UINT Maa_OnMouseWheel( HWND hWnd, INT xPos, INT yPos, INT zDelta, UINT fwKeys )
{
	HWND	hChdWnd;
	POINT	stPoint;

	stPoint.x = xPos;
	stPoint.y = yPos;
	ScreenToClient( ghMaaWnd, &stPoint );
	hChdWnd = ChildWindowFromPointEx( ghMaaWnd, stPoint, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT );

	if( hChdWnd == hWnd )	return 0;

	if( AaItemsIsUnderCursor( ghMaaWnd, hChdWnd , zDelta ) )	return 1;

	TreeFavIsUnderCursor( ghMaaWnd, hChdWnd, xPos, yPos, zDelta, fwKeys );

	return 1;
}

VOID StatusBarMsgSet( UINT room, LPTSTR ptMsg )
{

	SendMessage( ghStsBarWnd, SB_SETTEXT, room, (LPARAM)ptMsg );

	UpdateWindow( ghStsBarWnd );

	return;
}

INT TreeProfileMake( HWND hWnd, LPTSTR ptProf )
{
	OPENFILENAME	stOpenFile;
	BOOLEAN	bOpened;
	UINT	iCount;
	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING], atBuffer[MAX_PATH];
	TCHAR	atFolder[MAX_PATH];
	INT_PTR	iRslt;

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );

	if( ptProf )
	{
		StringCchCopy( atFilePath, MAX_PATH, ptProf );
	}
	else
	{
		ZeroMemory( &stOpenFile, sizeof(OPENFILENAME) );
		stOpenFile.lStructSize     = sizeof(OPENFILENAME);
		stOpenFile.hwndOwner       = ghMaaWnd;
		stOpenFile.lpstrFilter     = TEXT("プロファイル(*.qor)\0*.qor\0全てのファイル(*.*)\0*.*\0\0");

		stOpenFile.lpstrFile       = atFilePath;
		stOpenFile.nMaxFile        = MAX_PATH;
		stOpenFile.lpstrFileTitle  = atFileName;
		stOpenFile.nMaxFileTitle   = MAX_STRING;

		stOpenFile.lpstrTitle      = TEXT("ファイル名を指定するか、作成したいファイル名を入力してね");
		stOpenFile.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
		stOpenFile.lpstrDefExt     = TEXT("qor");

		bOpened = GetOpenFileName( &stOpenFile );

		if( !(bOpened) ){	return -1;	}

		StringCchCopy( gatProfilePath, MAX_PATH, atFilePath );
		InitParamString( INIT_SAVE, VS_PROFILE_NAME, atFilePath );
	}

	TabMultipleStore( hWnd );
	TabMultipleDeleteAll( hWnd );

	SqlDatabaseOpenClose( M_DESTROY, NULL );

	SqlDatabaseOpenClose( M_CREATE, atFilePath );

	StringCchCopy( atBuffer, MAX_PATH, atFilePath );
	PathStripPath( atBuffer );
	StatusBarMsgSet( SBMAA_PROFNAME, atBuffer );

	ZeroMemory( atFolder,  sizeof(atFolder) );

	OpenProfileLogging( hWnd, atFilePath );

	ZeroMemory( gatTemplatePath,  sizeof(gatTemplatePath) );
	SqlTreeProfSelect( NULL, 0, gatTemplatePath, MAX_PATH );

	if( NULL == gatTemplatePath[0] )
	{

		iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_PROFILE_TREESEL_DLG), hWnd, TreeProfileDlgProc, (LPARAM)atFolder );
		if( IDCANCEL == iRslt ){	return -1;	}

		StringCchCopy( gatTemplatePath, MAX_PATH, atFolder );
	}

	iCount = SqlTreeCount( 1, NULL );
	if( 0 >= iCount )	return 0;

	return 1;
}

INT TreeProfileOpen( HWND hWnd )
{
	INT	iRslt;

	iRslt = TreeProfileMake( hWnd, NULL );
	if( 0 > iRslt ){	return 0;	}

	TreeConstruct( ghMaaWnd, gatTemplatePath, TRUE );

	return 1;
}

INT TreeProfileRebuild( HWND hWnd )
{
	TCHAR	atFolder[MAX_PATH];
	INT_PTR	iRslt;

	StringCchCopy( atFolder, MAX_PATH, gatTemplatePath );

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_PROFILE_TREESEL_DLG), hWnd, TreeProfileDlgProc, (LPARAM)atFolder );
	if( IDCANCEL == iRslt ){	return (-1);	}

	if( NULL != atFolder[0] )
	{
		StringCchCopy( gatTemplatePath, MAX_PATH, atFolder );

		TreeConstruct( ghMaaWnd, gatTemplatePath, FALSE );
	}

	return iRslt;
}

INT_PTR CALLBACK TreeProfileDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static BOOLEAN	cbAct;
	static LPTSTR	ptFolder;
	static  HWND	chTvWnd;

	static HTREEITEM	chTreeRoot;
	static TVINSERTSTRUCT	cstRootIns;

	HWND	hWorkWnd;
	TCHAR	atTgtDir[MAX_PATH];
	INT		id;

	DWORD	mPos;
	UINT	bCheck, count;
	INT		idFrom;
	LPNMHDR	pstNmhdr;

	LPNMTREEVIEW	pstNmTrVw;
	TVHITTESTINFO	stTreeHit;
	HTREEITEM		hItem;

	SHFILEINFO		stShFileInfo;

	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:
			ptFolder = (LPTSTR)lParam;

			cbAct = FALSE;

			hWorkWnd = GetDlgItem( hDlg, IDPB_PRTREE_PROGRESS );
			SendMessage( hWorkWnd, PBM_SETRANGE32, 0, 0xFF );
			SendMessage( hWorkWnd, PBM_SETPOS, 0, 0 );
			SendMessage( hWorkWnd, PBM_SETSTEP, 1, 0 );
			ShowWindow( hWorkWnd, SW_HIDE );

			Edit_SetText( GetDlgItem(hDlg,IDE_PRTREE_DIR), ptFolder );

			chTvWnd = GetDlgItem( hDlg, IDTV_PRTREE_DIR_TVIEW );

			TreeView_DeleteAllItems( chTvWnd );

			ZeroMemory( &cstRootIns, sizeof(TVINSERTSTRUCT) );
			cstRootIns.hParent      = TVI_ROOT;
			cstRootIns.hInsertAfter = TVI_SORT;
			cstRootIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			cstRootIns.item.pszText = TEXT("ROOT");
			cstRootIns.item.lParam  = 0;

			SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
			cstRootIns.item.iImage = stShFileInfo.iIcon;
			SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
			cstRootIns.item.iSelectedImage = stShFileInfo.iIcon;

			chTreeRoot = TreeView_InsertItem( chTvWnd, &cstRootIns );

			SqlTreeCacheOpenClose( M_CREATE );

			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case  IDB_PRTREE_DIR_REF:
					if( SelectDirectoryDlg( hDlg, atTgtDir, MAX_PATH ) )
					{
						Edit_SetText( GetDlgItem(hDlg,IDE_PRTREE_DIR), atTgtDir );
					}
					return (INT_PTR)TRUE;

				case  IDB_PRTREE_LISTUP:
					Edit_GetText( GetDlgItem(hDlg,IDE_PRTREE_DIR), atTgtDir, MAX_PATH );
					if( NULL != atTgtDir[0] )
					{
						TreeView_DeleteAllItems( chTvWnd  );
						SqlTreeNodeAllDelete( 0 );
						chTreeRoot = TreeView_InsertItem( chTvWnd, &cstRootIns );
#ifndef TREEPROF_AUTOCHECK
						TreeView_SetCheckState( chTvWnd , chTreeRoot, TRUE );
#endif
						UpdateWindow( chTvWnd );

						StringCchCopy( ptFolder, MAX_PATH, atTgtDir );

						hWorkWnd = GetDlgItem( hDlg, IDPB_PRTREE_PROGRESS );
						SendMessage( hWorkWnd, PBM_SETPOS, 0, 0 );
						ShowWindow( hWorkWnd, SW_SHOW );
						TreeProfListUp( hDlg, chTvWnd, atTgtDir, chTreeRoot, 0, 1 );
						TreeView_Expand( chTvWnd, chTreeRoot, TVE_EXPAND );
#ifdef TREEPROF_AUTOCHECK

						TreeProfCheckExistent( hDlg, atTgtDir, chTvWnd, chTreeRoot, 0 );
#endif
						ShowWindow( hWorkWnd, SW_HIDE );
					}
					return (INT_PTR)TRUE;

				case IDOK:
					count = TreeView_GetCount( chTvWnd );
					if( 1 >= count )
					{
						MessageBox( hDlg, TEXT("リストアップ出来ていないみたい。\r\nこのままじゃツリーが作れないよ。"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
						return (INT_PTR)TRUE;
					}

					if( SUCCEEDED( TreeLoadDirCheck( hDlg, chTvWnd ) ) )
					{
						SqlTreeCacheOpenClose( M_DESTROY );
						EndDialog(hDlg, IDOK );
					}
					return (INT_PTR)TRUE;

				case IDCANCEL:
					SqlTreeCacheOpenClose( M_DESTROY );
					EndDialog(hDlg, IDCANCEL );
					return (INT_PTR)TRUE;
			}
			break;

		case WM_NOTIFY:
			idFrom   = (INT)wParam;
			pstNmhdr = (LPNMHDR)lParam;
			if( IDTV_PRTREE_DIR_TVIEW == idFrom )
			{
				pstNmTrVw = (LPNMTREEVIEW)pstNmhdr;
				if( NM_CLICK == pstNmhdr->code )
				{
					ZeroMemory( &stTreeHit, sizeof(TVHITTESTINFO) );
					mPos = GetMessagePos( );
					stTreeHit.pt.x = GET_X_LPARAM( mPos );
					stTreeHit.pt.y = GET_Y_LPARAM( mPos );
					MapWindowPoints( HWND_DESKTOP, chTvWnd, &stTreeHit.pt, 1 );
					TreeView_HitTest( chTvWnd, &stTreeHit );

					if ( TVHT_ONITEMSTATEICON & stTreeHit.flags )
					{

						bCheck = TreeView_GetCheckState( chTvWnd, stTreeHit.hItem );

						hItem = TreeView_GetChild( chTvWnd, stTreeHit.hItem );
						TreeProfCheckState( chTvWnd, hItem, bCheck );

						if( !(bCheck)  )
						{
							hItem = TreeView_GetParent( chTvWnd, stTreeHit.hItem );
							while( hItem  )
							{
								TreeView_SetCheckState( chTvWnd, hItem, TRUE );
								hItem = TreeView_GetParent( chTvWnd, hItem );
							}
						}
					}

					SetWindowLong( hDlg, DWL_MSGRESULT, 0 );
					return (INT_PTR)TRUE;
				}

			}
			break;
	}

	return (INT_PTR)FALSE;
}

VOID TreeProfProgressUp( HWND hDlg )
{
	HWND	hProgWnd;
	UINT	pos;

	hProgWnd = GetDlgItem( hDlg, IDPB_PRTREE_PROGRESS );

	pos = SendMessage( hProgWnd, PBM_GETPOS, 0, 0 );
	pos++;
	pos &= 0xFF;
	SendMessage( hProgWnd, PBM_SETPOS, pos, 0 );
	UpdateWindow( hProgWnd );

	return;
}

#ifdef TREEPROF_AUTOCHECK

UINT TreeProfCheckExistent( HWND hDlg, LPTSTR ptTgDir, HWND hTvWnd, HTREEITEM hNode, UINT sqlID )
{
	UINT		checked = 0, tgtID;
	TCHAR		atProfRoot[MAX_PATH];
	TCHAR		atName[MAX_PATH];
	HTREEITEM	hItem, hRoot;
	TVITEM		stItem;

	if( ptTgDir )
	{
		ZeroMemory( atProfRoot, sizeof(atProfRoot) );
		SqlTreeProfSelect( NULL, 0, atProfRoot, MAX_PATH );

		if( StrCmp( atProfRoot, ptTgDir ) )	return 0;

		hRoot = hNode;
		hItem = TreeView_GetChild( hTvWnd , hNode );
		hNode = hItem;
		if( !(hNode) )	return 0;
		sqlID = 0;
	}

	do{

		ZeroMemory( &stItem, sizeof(TVITEM) );
		stItem.mask       = TVIF_HANDLE | TVIF_TEXT;
		stItem.hItem      = hNode;
		stItem.pszText    = atName;
		stItem.cchTextMax = MAX_PATH;
		TreeView_GetItem( hTvWnd, &stItem );
		tgtID = SqlTreeFileGetOnParent( atName, sqlID );

		if( tgtID )
		{
			checked++;
			TreeView_SetCheckState( hTvWnd, hNode, TRUE );

			hItem = TreeView_GetChild( hTvWnd , hNode );

			if( hItem ){	checked += TreeProfCheckExistent( hDlg, NULL , hTvWnd, hItem, tgtID );	}
		}

		hItem = TreeView_GetNextSibling( hTvWnd, hNode );
		hNode = hItem;

		TreeProfProgressUp( hDlg );
	}
	while( hNode );

	if( ptTgDir )
	{
		if( checked ){	TreeView_SetCheckState( hTvWnd, hRoot, TRUE );	}
	}

	return checked;
}

#endif

VOID TreeProfCheckState( HWND hTvWnd, HTREEITEM hNode, UINT bCheck )
{
	HTREEITEM	hItem;

	if( !(hNode) )	return;

	do
	{
		TreeView_SetCheckState( hTvWnd, hNode, bCheck ? FALSE : TRUE );

		hItem = TreeView_GetChild( hTvWnd , hNode );

		if( hItem ){	TreeProfCheckState( hTvWnd, hItem, bCheck );	}

		hItem = TreeView_GetNextSibling( hTvWnd, hNode );
		hNode = hItem;
	}
	while( hNode );

	return;
}

HRESULT TreeProfListUp( HWND hDlg, HWND hTvWnd, LPTSTR ptRoot, HTREEITEM hTreePr, UINT dPrntID, INT fCheck )
{
	HANDLE	hFind;
	TCHAR	atPath[MAX_PATH], atNewTop[MAX_PATH], atTarget[MAX_PATH];
	BOOL	bRslt;
	UINT	dPnID = 0;

	WIN32_FIND_DATA	stFindData;

	HTREEITEM	hNewParent, hLastDir = TVI_FIRST;
	TVINSERTSTRUCT	stTreeIns;
	SHFILEINFO	stShFileInfo;

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = hTreePr;
	stTreeIns.hInsertAfter = TVI_LAST;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	ZeroMemory( atTarget, sizeof(atTarget) );
	StringCchCopy( atTarget, MAX_PATH, ptRoot );
	PathAppend( atTarget, TEXT("*") );

	hFind = FindFirstFile( atTarget, &stFindData );
	do{
		if( lstrcmp( stFindData.cFileName, TEXT("..") ) && lstrcmp( stFindData.cFileName, TEXT(".") ) )
		{
			TreeProfProgressUp( hDlg );

			StringCchCopy( atPath, MAX_PATH, ptRoot );
			PathAppend( atPath, stFindData.cFileName );

			SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
			stTreeIns.item.iImage = stShFileInfo.iIcon;
			SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
			stTreeIns.item.iSelectedImage = stShFileInfo.iIcon;

			stTreeIns.item.pszText = stFindData.cFileName;

			if( stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				dPnID = SqlTreeCacheInsert( FILE_ATTRIBUTE_DIRECTORY, dPrntID, stFindData.cFileName );

				stTreeIns.item.lParam  = dPnID;
				stTreeIns.hInsertAfter = hLastDir;
				hNewParent = TreeView_InsertItem( hTvWnd, &stTreeIns );
				hLastDir = hNewParent;

#ifndef TREEPROF_AUTOCHECK
				TreeView_SetCheckState( hTvWnd , hNewParent, TRUE );
#endif
				StringCchCopy( atNewTop, MAX_PATH, ptRoot );
				PathAppend( atNewTop, stFindData.cFileName );

				TreeProfListUp( hDlg, hTvWnd, atNewTop, hNewParent, dPnID, fCheck );

			}
			else
			{
				bRslt  = PathMatchSpec( stFindData.cFileName, TEXT("*.mlt") );
				bRslt |= PathMatchSpec( stFindData.cFileName, TEXT("*.ast") );
				bRslt |= PathMatchSpec( stFindData.cFileName, TEXT("*.txt") );
				if( bRslt )
				{
					dPnID = SqlTreeCacheInsert( FILE_ATTRIBUTE_NORMAL, dPrntID, stFindData.cFileName );

					stTreeIns.item.lParam  = dPnID;
					stTreeIns.hInsertAfter = TVI_LAST;
					hNewParent = TreeView_InsertItem( hTvWnd, &stTreeIns );

#ifndef TREEPROF_AUTOCHECK
					TreeView_SetCheckState( hTvWnd , hNewParent, TRUE );
#endif
				}
			}
		}

	}while( FindNextFile( hFind, &stFindData ) );

	FindClose( hFind );

	return S_OK;
}

HRESULT TreeLoadDirCheck( HWND hDlg, HWND hTvWnd )
{
	TCHAR	atTgtDir[MAX_PATH];
	HTREEITEM	hTreeRoot, hItem;
	UINT	dCacheMax, dCacheCnt, m, count;
	UINT	dType, dPrnt, index, logoa;
	TCHAR	atName[MAX_PATH];
	HWND	hWorkWnd;

	hTreeRoot = TreeView_GetRoot( hTvWnd  );

	hItem = TreeView_GetChild( hTvWnd, hTreeRoot );

	hWorkWnd = GetDlgItem( hDlg, IDPB_PRTREE_PROGRESS );
	SendMessage( hWorkWnd, PBM_SETPOS, 0, 0 );
	ShowWindow( hWorkWnd, SW_SHOW );

	count = TreeLoadNodeProc( hDlg, hTvWnd, hItem, 0 );
	TRACE( TEXT("%u"), count );
	if( 0 == count )
	{
		ShowWindow( hWorkWnd, SW_HIDE );
		MessageBox( hDlg, TEXT("ファイルが一つも選択されてないよ。\r\nこのままだと使えないよ。"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
		return E_ABORT;
	}

	TreeLoadNodeProc( hDlg, hTvWnd, hItem, 1 );

	Edit_GetText( GetDlgItem(hDlg,IDE_PRTREE_DIR), atTgtDir, MAX_PATH );

	SqlTransactionOnOff( TRUE );

	SqlTreeProfUpdate( NULL, atTgtDir );
	SqlTreeNodeAllDelete( 1 );

	dCacheCnt = SqlTreeCount( 3, &dCacheMax );
	index = 0;
	for( m = 0; dCacheMax > m; m++ )
	{
		ZeroMemory( atName, sizeof(atName) );
		index = SqlTreeNodePickUpID( index, &dType, &dPrnt, atName, 0x00 );
		TRACE( TEXT("[%4u]%4u\t%4u\t%4u\t%s"), m, index, dType, dPrnt, atName );
		if( 0 >= index )	break;

		logoa = SqlTreeNodeInsert( index, dType, dPrnt, atName );

		TreeProfProgressUp( hDlg );
	}

	ShowWindow( hWorkWnd, SW_HIDE );

	SqlTransactionOnOff( FALSE );

	return S_OK;
}

UINT TreeLoadNodeProc( HWND hDlg, HWND hTvWnd, HTREEITEM hNode, UINT bFixe )
{
	TCHAR		atName[MAX_PATH];
	INT			param;
	UINT		dRslt, dType, dPrID, count = 0;
	HTREEITEM	hItem;
	TVITEM		stItem;

	do
	{

		ZeroMemory( &stItem, sizeof(TVITEM) );
		stItem.mask  = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
		stItem.hItem      = hNode;
		stItem.pszText    = atName;
		stItem.cchTextMax = MAX_PATH;
		TreeView_GetItem( hTvWnd, &stItem );
		param = stItem.lParam;

		dRslt = TreeView_GetCheckState( hTvWnd, hNode );
		TRACE( TEXT("Node:[%d][%d]%s"), dRslt, param, atName );

		if( bFixe )
		{

			if( !(dRslt)  ){	SqlTreeCacheDelID( param  );	}
		}
		else
		{
			SqlTreeNodePickUpID( param-1, &dType, &dPrID, atName, 0x00 );

			if( dRslt && (FILE_ATTRIBUTE_NORMAL==dType) ){	count++;	}
		}

		hItem = TreeView_GetChild( hTvWnd , hNode );

		if( hItem ){	count += TreeLoadNodeProc( hDlg, hTvWnd, hItem, bFixe );	}

		hItem = TreeView_GetNextSibling( hTvWnd, hNode );
		hNode = hItem;

		TreeProfProgressUp( hDlg );
	}
	while( hNode );

	return count;
}

LPTSTR PathSplitFirstPath( LPTSTR ptSource, LPTSTR ptSplits )
{
	UINT	d;

	if( !(ptSource) )	return NULL;
	if( NULL ==  ptSource[0] )	return NULL;

	for( d = 0; MAX_PATH > d; d++ )
	{
		if( TEXT('\\') == ptSource[d] ){	ptSplits[d] =  NULL;	d++;	break;	}
		else if( NULL ==  ptSource[d] ){	ptSplits[d] =  NULL;	break;	}
		else{	ptSplits[d] = ptSource[d];	}
	}

	return &(ptSource[d]);
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列の次の改行までを取る
	@param[in]	ptSource	元文字列
	@param[in]	*ptNextLn	次の行の先頭
	@return		確保した文字列・freeセヨ
*/
LPTSTR StringLineGet( LPCTSTR ptSource, LPCTSTR *ptNextLn )
{
	UINT		t = 0;
	UINT_PTR	len;
	wstring		wStr;
	LPTSTR		ptDest;

	while( ptSource[t] )
	{
		if( TEXT('\r') == ptSource[t] || TEXT('\n') == ptSource[t] )	break;

		wStr += ptSource[t];
		t++;
	}

	if( TEXT('\r') == ptSource[t] || TEXT('\n') == ptSource[t] )	t++;
	if( TEXT('\r') == ptSource[t] || TEXT('\n') == ptSource[t] )	t++;
	*ptNextLn = &(ptSource[t]);

	len = wStr.size() + 1;
	ptDest = (LPTSTR)malloc( len * sizeof(TCHAR) );
	ZeroMemory( ptDest, len * sizeof(TCHAR) );
	StringCchCopy( ptDest, len, wStr.c_str() );

	return ptDest;
}
//-------------------------------------------------------------------------------------------------

/*
ツリー内からサーチ
同じ親番号を持つ者内でサーチ・同じディレクトリ内には、単一名しか入らないから、
それが同じモノだと判断できるはず

*/

/*!
	検索してリストビューに入れる
	@param[in]	hDlg	ダイヤログハンドル
*/
HRESULT MaaFindExecute( HWND hDlg )
{
	UINT	dCnt, dMax, d;
	UINT	dItem, dType, dPrntID, dOwnID;
	UINT	dDmyType, dDmyID;
	TCHAR	atPattern[MAX_PATH];
	TCHAR	atFileName[MAX_PATH], atPrntName[MAX_PATH];
	HWND	hLvWnd, hEdWnd;
	LVITEM	stLvi;

	hEdWnd = GetDlgItem( hDlg, IDE_MAA_FIND_NAME );
	hLvWnd = GetDlgItem( hDlg, IDLV_MAA_FINDED_FILE );

	ListView_DeleteAllItems( hLvWnd );

	ZeroMemory( atPattern, sizeof(atPattern) );
	GetDlgItemText( hDlg, IDE_MAA_FIND_NAME, atPattern, MAX_PATH );
	//	空文字列なら検索しない
	if( NULL == atPattern[0] )	return  E_ABORT;

	dCnt = SqlTreeCount( 1, &dMax );

	dOwnID = 0;
	for( d = 0; dMax > d; d++ )
	{
		dOwnID = SqlTreeFileSearch( atPattern, dOwnID );	//	ヒットを確認
		if( 0 == dOwnID )	break;	//	それ以上無いようなら終わり

		ZeroMemory( atFileName, sizeof(atFileName) );
		dType   = 0;
		dPrntID = 0;

		//	該当ＩＤの内容を確認
		SqlTreeNodePickUpID( dOwnID, &dType, &dPrntID, atFileName, 0x11 );
		if( FILE_ATTRIBUTE_NORMAL == dType )
		{
			//	引っ張った内容ファイル名をリストビューに表示
			dItem = ListView_GetItemCount( hLvWnd );

			ZeroMemory( &stLvi, sizeof(stLvi) );
			stLvi.iItem = dItem;

			stLvi.mask     = LVIF_TEXT | LVIF_PARAM;
			stLvi.pszText  = atFileName;
			stLvi.lParam   = dOwnID;
			stLvi.iSubItem = 0;
			ListView_InsertItem( hLvWnd, &stLvi );

			SqlTreeNodePickUpID( dPrntID, &dDmyType, &dDmyID, atPrntName, 0x11 );

			stLvi.mask     = LVIF_TEXT;
			stLvi.pszText  = atPrntName;
			stLvi.iSubItem = 1;
			ListView_SetItem( hLvWnd, &stLvi );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	MAA検索ダイヤログのノーティファイメッセージの処理
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
INT_PTR MaaFindOnNotify( HWND hDlg, INT idFrom, LPNMHDR pstNmhdr )
{
	HWND	hLvWnd;
	INT		iItem, nmCode;
	LPNMLISTVIEW	pstNmLv;
	LVITEM			stLvi;
	HTREEITEM		hTgtItem;

	if( IDLV_MAA_FINDED_FILE == idFrom )
	{
		pstNmLv = (LPNMLISTVIEW)pstNmhdr;

		hLvWnd = pstNmLv->hdr.hwndFrom;
		nmCode = pstNmLv->hdr.code;

		//	選択されてる項目を確保
		iItem = ListView_GetNextItem( hLvWnd, -1, LVNI_ALL | LVNI_SELECTED );

		if( 0 >  iItem )	return FALSE;	//	未選択状態なら何もしない

		//	ダブルクルックであった場合
		if( NM_DBLCLK == nmCode )
		{
			ZeroMemory( &stLvi, sizeof(stLvi) );
			stLvi.mask     = LVIF_PARAM;
			stLvi.iItem    = iItem;
			stLvi.iSubItem = 0;
			ListView_GetItem( hLvWnd, &stLvi );

			hTgtItem = MaaSelectIDfile( hDlg, stLvi.lParam );	//	SqlID渡して開くようにする
			//	ツリーのを選択状態にしている

			if( hTgtItem )
			{
				SetForegroundWindow( ghMaaWnd );

				//	ここで、タブ選択からチェインさせればいい
				TabMultipleCtrlFromFind( ghMaaWnd );
				//AaTitleClear(  );	//	ここでクルヤーせないかん
				//TreeSelItemProc( ghMaaWnd, hTgtItem, 0 );	//	渡すハンドル、MAA窓のハンドルにしておかないとまずい？
			}
		}
	}

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	検索ダイヤログのプロシージャ
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK TreeMaaFindDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	HWND	hWorkWnd;
	UINT	id;
//	HWND	hWndChild;
	LVCOLUMN	stLvColm;

	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:
			hWorkWnd = GetDlgItem( hDlg, IDLV_MAA_FINDED_FILE );
			ListView_SetExtendedListViewStyle( hWorkWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
			ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
			stLvColm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			stLvColm.fmt = LVCFMT_LEFT;
			stLvColm.pszText = TEXT("ファイル名");	stLvColm.cx = 250;	stLvColm.iSubItem = 0;	ListView_InsertColumn( hWorkWnd, 0, &stLvColm );
			stLvColm.pszText = TEXT("所属");		stLvColm.cx = 250;	stLvColm.iSubItem = 1;	ListView_InsertColumn( hWorkWnd, 1, &stLvColm );
			SetFocus( GetDlgItem(hDlg,IDE_MAA_FIND_NAME) );
			return (INT_PTR)FALSE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			hWorkWnd = GetDlgItem( hDlg, IDE_FIND_TEXT );
			switch( id )
			{
				case IDCANCEL:	DestroyWindow( hDlg );	ghMaaFindDlg = NULL;	return (INT_PTR)TRUE;

				case IDOK:		MaaFindExecute( hDlg );	return (INT_PTR)TRUE;	//	検索する

				case IDM_PASTE:	SendMessage( hWorkWnd, WM_PASTE, 0, 0 );	return (INT_PTR)TRUE;
				case IDM_COPY:	SendMessage( hWorkWnd, WM_COPY,  0, 0 );	return (INT_PTR)TRUE;
				case IDM_CUT:	SendMessage( hWorkWnd, WM_CUT,   0, 0 );	return (INT_PTR)TRUE;
				case IDM_UNDO:	SendMessage( hWorkWnd, WM_UNDO,  0, 0 );	return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		case WM_CLOSE:		DestroyWindow( hDlg );	ghMaaFindDlg = NULL;	return (INT_PTR)TRUE;

		case WM_DESTROY:	ghMaaFindDlg = NULL;	return (INT_PTR)TRUE;

		case WM_NOTIFY:		MaaFindOnNotify( hDlg, (INT)(wParam), (LPNMHDR)(lParam) );	return (INT_PTR)TRUE;

	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------　λ...

/*!
	MAAファイル検索窓の処理
	@param[in]	hWnd	ウインドウハンドル
	@return	HRESULT	終了状態コード
*/
HRESULT TreeMaaFileFind( HWND hWnd )
{
	HINSTANCE	hInst;

	hInst = GetModuleHandle( NULL );

	if( ghMaaFindDlg )
	{
		SetForegroundWindow( ghMaaFindDlg );
		return S_OK;
	}

	ghMaaFindDlg = CreateDialogParam( hInst, MAKEINTRESOURCE(IDD_FIND_MAA_DLG), hWnd, TreeMaaFindDlgProc, 0 );

	ShowWindow( ghMaaFindDlg, SW_SHOW );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイルからプロフ履歴取り込んだり書き込んだり
	@param[in]	hWnd	ウインドウハンドル・NULLならデストロイ
	@return	HRESULT	終了状態コード
*/
HRESULT OpenProfileInitialise( HWND hWnd )
{
	TCHAR	atString[MAX_PATH+10];
	UINT	d;
	UINT_PTR	dItems;
	OPENHIST	stProfHist;
	OPHIS_ITR	itHist;

	if( hWnd )	//	ロード
	{
		gltProfHist.clear( );	//	とりあえず全削除

		for( d = 0; OPENHIST_MAX > d; d++ )
		{
			ZeroMemory( &stProfHist, sizeof(OPENHIST) );

			if( SUCCEEDED( InitProfHistory( INIT_LOAD, d, stProfHist.atFile ) ) )
			{
				gltProfHist.push_back( stProfHist );
			}
			else{	break;	}	//	中身があったら記録・無かったら終わり
		}

		if( ghProfHisMenu ){	DestroyMenu( ghProfHisMenu );	}
		//	メニュー作成
		ghProfHisMenu = CreatePopupMenu(  );
		AppendMenu( ghProfHisMenu, MF_SEPARATOR, 0 , NULL );	//	セッパレター
		AppendMenu( ghProfHisMenu, MF_STRING, IDM_OPEN_HIS_CLEAR, TEXT("履歴クリヤ") );
		//	コマンドＩＤがファイルと同じであることに注意・受取は別だから問題無い

		dItems = gltProfHist.size( );
		if( 0 == dItems )
		{
			//	オーポン履歴が無い場合
			InsertMenu( ghProfHisMenu, 0, MF_STRING | MF_BYPOSITION | MF_GRAYED, IDM_OPEN_HIS_FIRST, TEXT("(无)") );
		}
		else
		{
			//	オーポン履歴を並べる
			for( itHist = gltProfHist.begin(), d = dItems-1; gltProfHist.end() != itHist; itHist++, d-- )
			{
				StringCchPrintf( atString, MAX_PATH+10, TEXT("(&%X) %s"), d, itHist->atFile );
				InsertMenu( ghProfHisMenu, 0, MF_STRING | MF_BYPOSITION, (IDM_OPEN_HIS_FIRST + d), atString );
				itHist->dMenuNumber = (IDM_OPEN_HIS_FIRST + d);
			}
		}

		//	コンテキストメニューは必要に応じてロードするので、ここではイジらない
#ifdef _ORRVW
		OpenProfMenuModify( hWnd );
#endif
	}
	else	//	終了時
	{
		if( ghProfHisMenu ){	DestroyMenu( ghProfHisMenu );	}

		InitProfHistory( INIT_SAVE, 0, NULL );	//	一旦全削除

		//	中身を保存
		for( itHist = gltProfHist.begin(), d = 0; gltProfHist.end() != itHist; itHist++, d++ )
		{
			InitProfHistory( INIT_SAVE, d, itHist->atFile );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いたプロフ履歴を番号指定して読み込む
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	id		履歴指定メッセージ・メニューＩＤである
	@return	HRESULT	終了状態コード
*/
HRESULT OpenProfileLoad( HWND hWnd, INT id )
{
	UINT_PTR	dNumber, dItems;
	TCHAR		atFilePath[MAX_PATH];
	OPHIS_ITR	itHist;

	WIN32_FIND_DATA	stFindData;
	HANDLE	hFind;

	dNumber = id - IDM_OPEN_HIS_FIRST;

	TRACE( TEXT("プロフ -> %d"), dNumber );
	if( OPENHIST_MAX <= dNumber ){	return E_OUTOFMEMORY;	}

	dItems = gltProfHist.size();
	dNumber = (dItems-1) - dNumber;

	itHist = gltProfHist.begin();
	std::advance( itHist , dNumber );	//	個数分進める

	//	選択したプロフを開く
	StringCchCopy( atFilePath, MAX_PATH, itHist->atFile );

	//	そのファイルは存在するか？
	hFind = FindFirstFile( atFilePath, &stFindData );	//	TEXT("*")
	if( INVALID_HANDLE_VALUE != hFind ){	FindClose( hFind  );	}
	else{	ZeroMemory( atFilePath, sizeof(atFilePath) );	};

	if( NULL != atFilePath[0]  )	//	無ければ何もしない
	{
		StringCchCopy( gatProfilePath, MAX_PATH, atFilePath );
		TreeProfileMake( ghMaaWnd, gatProfilePath );

		InitParamString( INIT_SAVE, VS_PROFILE_NAME, gatProfilePath );

		TreeConstruct( ghMaaWnd, gatTemplatePath, TRUE );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いたPROFILEを記録
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	ptProf	開いたプロフファイル名
	@return	HRESULT	終了状態コード
*/
HRESULT OpenProfileLogging( HWND hWnd, LPCTSTR ptProf )
{
	UINT_PTR	dItems;
	OPENHIST	stProfHist;
	OPHIS_ITR	itHist;

	if( ptProf )
	{
		ZeroMemory( &stProfHist, sizeof(OPENHIST) );

		StringCchCopy( stProfHist.atFile, MAX_PATH, ptProf );
		//既存の内容なら最新に入れ替えるので、検索しておく
		for( itHist = gltProfHist.begin(); gltProfHist.end() != itHist; itHist++ )
		{
			if( !StrCmp( itHist->atFile, stProfHist.atFile ) )	//	同じものがあったら削除する
			{
				gltProfHist.erase( itHist );
				break;
			}
		}

		gltProfHist.push_back( stProfHist );	//	リスト末尾ほど新しい

		//	もしはみ出すようなら古いのを削除する
		dItems = gltProfHist.size( );
		if( OPENHIST_MAX <  dItems )
		{
			gltProfHist.pop_front(  );
		}
	}
	else	//	文字列指定無い場合は全クリ
	{
		gltProfHist.clear();
	}

	OpenProfileInitialise( NULL );	//	古いの破壊して
	OpenProfileInitialise( hWnd );	//	最新の内容で作り直し

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
