#include "stdafx.h"
#include "OrinrinEditor.h"

static CONST TCHAR	gcatLicense[] = {
TEXT("このプログラムはフリーソフトウェアです。あなたはこれを、フリーソフトウェア財団によって発行されたGNU一般公衆利用許諾書(バージョン3か、それ以降のバージョンのうちどれか)が定める条件の下で再頒布または改変することができます。\r\n\r\n")
TEXT("このプログラムは有用であることを願って頒布されますが、*全くの無保証*です。商業可能性の保証や特定目的への適合性は、言外に示されたものも含め、全く存在しません。\r\n\r\n")
TEXT("詳しくはGNU一般公衆利用許諾書をご覧ください。\r\n\r\n")
TEXT("あなたはこのプログラムと共に、GNU一般公衆利用許諾書のコピーを一部受け取っているはずです。\r\n\r\n")
TEXT("もし受け取っていなければ、<http://www.gnu.org/licenses/> をご覧ください。\r\n\r\n")
};

static LOGFONT	gstBaseFont = {
	FONTSZ_NORMAL,
	0,
	0,
	0,
	FW_NORMAL,
	FALSE,
	FALSE,
	FALSE,
	DEFAULT_CHARSET,
	OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS,
	PROOF_QUALITY,
	VARIABLE_PITCH,
	TEXT("ＭＳ Ｐゴシック")
};

static  HINSTANCE	ghInst;
static  TCHAR		gszTitle[MAX_STRING];
static  TCHAR		gszWindowClass[MAX_STRING];

static  HMENU		ghMenu;

static	HACCEL		ghAccelTable;

static  HWND		ghFileTabWnd;
static  HWND		ghFileTabTip;
static WNDPROC		gpfOriginFileTabProc;

static  HWND		ghMainWnd;
static  HWND		ghStsBarWnd;
static  HBRUSH		ghStsRedBrush;

static HANDLE		ghMutex;

static INT			giResizeWide;

#ifdef WORK_LOG_OUT
static HANDLE		ghLogFlie;
#endif

EXTERNED HFONT		ghNameFont;

EXTERNED HWND		ghMaaWnd;
EXTERNED HWND		ghPgVwWnd;
EXTERNED HWND		ghLnTmplWnd;
EXTERNED HWND		ghBrTmplWnd;
EXTERNED BOOLEAN	gbDockTmplView;

EXTERNED HWND		ghMainSplitWnd;
EXTERNED LONG		grdSplitPos;

EXTERNED UINT		gbUniPad;
EXTERNED UINT		gbUniRadixHex;

static   UINT		gdBUInterval;
EXTERNED UINT		gbAutoBUmsg;
EXTERNED UINT		gbCrLfCode;

EXTERNED UINT		gbSaveMsgOn;

EXTERNED UINT		gdPageByteMax;

static TCHAR		gatExePath[MAX_PATH];
static TCHAR		gatIniPath[MAX_PATH];

EXTERNED INT		gbTmpltDock;

static list<OPENHIST>	gltOpenHist;
EXTERNED HMENU		ghHistyMenu;

static BOOLEAN		gbDummy;

#ifdef BIG_TEXT_SEPARATE
static BOOLEAN		gbSeparateMod;
#endif

#ifdef FIND_STRINGS
extern  HWND		ghFindDlg;
#endif
extern  HWND		ghMoziWnd;

extern  HWND		ghMaaFindDlg;

extern  UINT		 gdClickDrt;
extern  UINT		gdSubClickDrt;

extern  HWND		ghViewWnd;

extern  UINT		 gdGridXpos;
extern  UINT		 gdGridYpos;
extern  UINT		gdRightRuler;
extern  UINT		gdUnderRuler;

#ifdef SPMOZI_ENCODE
extern  UINT		gbSpMoziEnc;
#endif

#ifdef PLUGIN_ENABLE

plugin::PLUGIN_FILE_LIST	gPluginList;
#include "Plugin/PluginCtrlAsync.h"

#endif

#define SB_ITEMS	8
CONST INT	gadStsBarSize[] = { 50, 200, 350, 500, 700, 800, 900, -1 };

VOID	Cls_OnActivate( HWND, UINT, HWND, BOOL );
BOOLEAN	Cls_OnCreate( HWND, LPCREATESTRUCT );
VOID	Cls_OnCommand( HWND , INT, HWND, UINT );
VOID	Cls_OnPaint( HWND );
VOID	Cls_OnSize( HWND , UINT, INT, INT );
VOID	Cls_OnMove( HWND, INT, INT );
VOID	Cls_OnDestroy( HWND );
LRESULT	Cls_OnNotify( HWND , INT, LPNMHDR );
VOID	Cls_OnTimer( HWND, UINT );
VOID	Cls_OnDropFiles( HWND , HDROP );
VOID	Cls_OnContextMenu(HWND,HWND,UINT,UINT );
VOID	Cls_OnHotKey(HWND, INT, UINT, UINT );
VOID	Cls_OnDrawItem( HWND, CONST DRAWITEMSTRUCT * );

BOOL	Cls_OnWindowPosChanging( HWND, LPWINDOWPOS );

#ifdef MULTIACT_RELAY
void	Cls_OnCopyData( HWND, HWND, PCOPYDATASTRUCT );
#endif

INT_PTR	CALLBACK OptionDlgProc( HWND, UINT, WPARAM, LPARAM );

LRESULT	CALLBACK gpfFileTabProc( HWND, UINT, WPARAM, LPARAM );
VOID	Ftb_OnMButtonUp( HWND, INT, INT, UINT );

HRESULT	ViewingFontNameLoad( VOID );

#ifdef USE_NOTIFYICON
VOID	TaskTrayIconEvent( HWND, UINT, UINT );
#endif

INT APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	INT		msRslt;
	MSG		msg;

	INT		iArgc;
	TCHAR	atArgv[MAX_PATH];
	LPTSTR	*pptArgs;

	INT		iCode, dMultiEna;

	LPACCEL	pstAccel;
	INT	iEntry;

#ifdef WORK_LOG_OUT
	SYSTEMTIME	stTime;
#endif
#ifdef MULTIACT_RELAY
	HWND	hWndActed;
	COPYDATASTRUCT	stCopyData;
#endif

#ifdef _DEBUG

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );

#endif

	INITCOMMONCONTROLSEX	iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC  = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES | ICC_COOL_CLASSES;
	InitCommonControlsEx( &iccex );

	LoadString(hInstance, IDS_APP_TITLE, gszTitle, MAX_STRING);
	LoadString(hInstance, IDC_ORINRINEDITOR, gszWindowClass, MAX_STRING);

	ZeroMemory( atArgv, sizeof(atArgv) );
	pptArgs = CommandLineToArgvW( GetCommandLine( ), &iArgc );

	if( 2 <= iArgc ){	StringCchCopy( atArgv, MAX_PATH, pptArgs[1] );	}
	LocalFree( pptArgs );

	ghMutex = NULL;

	GetModuleFileName( hInstance, gatExePath, MAX_PATH );
	PathRemoveFileSpec( gatExePath );

	StringCchCopy( gatIniPath, MAX_PATH, gatExePath );
	PathAppend( gatIniPath, INI_FILE );

#ifdef MULTIACT_RELAY
	dMultiEna = InitParamValue( INIT_LOAD, VL_MULTI_ACT_E, 0 );
	if( !(dMultiEna) )
	{
#endif

		SECURITY_DESCRIPTOR	stSeqDes;
		SECURITY_ATTRIBUTES	secAttribute;
		InitializeSecurityDescriptor( &stSeqDes, SECURITY_DESCRIPTOR_REVISION );
		SetSecurityDescriptorDacl( &stSeqDes, TRUE, 0, FALSE );
		secAttribute.nLength				= sizeof( secAttribute );
		secAttribute.lpSecurityDescriptor	= &stSeqDes;
		secAttribute.bInheritHandle			= TRUE;

		ghMutex = CreateMutex( &secAttribute, TRUE, TEXT("OrinrinEditor") );
		if( !(ghMutex) )
		{

		}
		else if( GetLastError() == ERROR_ALREADY_EXISTS )
		{
	#ifdef MULTIACT_RELAY
			hWndActed = FindWindow( gszWindowClass, NULL );
			SetForegroundWindow( hWndActed );

			if( 0 != atArgv[0] )
			{
				stCopyData.dwData = 1;
				stCopyData.cbData = sizeof(atArgv);
				stCopyData.lpData = atArgv;

				SendMessage( hWndActed, WM_COPYDATA, NULL, (LPARAM)(&stCopyData) );
			}
	#else
			MessageBox( NULL, TEXT("已にアプリは起動してるよ！"), TEXT("お燐からのお知らせ"), MB_OK|MB_ICONINFORMATION );
	#endif
			ReleaseMutex( ghMutex );
			CloseHandle( ghMutex );
			return 0;
		}
#ifdef MULTIACT_RELAY
	}
#endif

	InitWndwClass( hInstance );

	DocBackupDirectoryInit( gatExePath );
	FrameInitialise( gatExePath, hInstance );
	MoziInitialise( gatExePath, hInstance );

	AacBackupDirectoryInit( gatExePath );

	gbUniPad = 0;

	gbDockTmplView = TRUE;

	gbDummy = 0;

#ifdef BIG_TEXT_SEPARATE
	gbSeparateMod = FALSE;
#endif

#ifdef WORK_LOG_OUT
	iCode = InitParamValue( INIT_LOAD, VL_WORKLOG, 0 );
	if( iCode )
	{
		ghLogFlie = CreateFile( TEXT("LogFile.txt"), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( INVALID_HANDLE_VALUE != ghLogFlie )
		{
			SetFilePointer( ghLogFlie, 0, NULL, FILE_END );

			GetLocalTime( &stTime );
			TRACE( TEXT("LOG START [%u/%02u/%02u %02u:%02u:%02u]"), stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond );
		}
	}
	else
	{
		ghLogFlie = INVALID_HANDLE_VALUE;
	}
#endif

	iCode = InitParamValue( INIT_LOAD, VL_CLASHCOVER, 0 );
	if( iCode )
	{
		TRACE( TEXT("異常終了の可能性アリ") );
		iCode = MessageBox( NULL, TEXT("エディタが正しく終了出来なかった気配があるよ。\r\nバックアップが残っているかもしれないから、先に確認してみて！\r\nこのまま起動してもいいかい？　「いいえ」を選ぶと、ここで終了するよ。"), TEXT("ごめんね"), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2 );
		if( IDNO == iCode ){	return 0;	}
	}
	InitParamValue( INIT_SAVE, VL_CLASHCOVER, 1 );

	ViewingFontNameLoad(  );

	if( !InitInstance( hInstance, nCmdShow , atArgv ) ){	return FALSE;	}

	CntxEditInitialise( gatExePath, hInstance );

	VertInitialise( gatExePath, hInstance );

	RegisterHotKey( ghMainWnd, IDHK_THREAD_DROP, MOD_CONTROL | MOD_SHIFT, VK_D );

	pstAccel = AccelKeyTableLoadAlloc( &iEntry );
	AccelKeyTableCreate( pstAccel, iEntry );
	ToolBarInfoChange( pstAccel, iEntry );
	FREE( pstAccel );

#ifdef TODAY_HINT_STYLE
	TodayHintPopup( ghMainWnd, ghInst, gatExePath );
#endif

#ifdef PLUGIN_ENABLE

	plugin::PluginInputParam	param;
	param.hMainWnd    = ghMainWnd;
	param.hMlutAAWnd  = ghMaaWnd;
	param.hFileTabWnd = ghFileTabWnd;
	param.hViewWnd    = ghViewWnd;

	param.hMainMenu   = ghMenu;

	plugin::InitializePlugin( param );
	plugin::InitializePluginFileListAsync( ghMainWnd );

#endif

	for(;;)
	{
		msRslt = GetMessage( &msg, NULL, 0, 0 );
		if( 1 != msRslt )	break;

#ifdef FIND_STRINGS
		if( ghFindDlg )
		{
			if( ghFindDlg == GetForegroundWindow(  ) )
			{
				if( TranslateAccelerator( ghFindDlg, ghAccelTable, &msg ) )	continue;
				if( IsDialogMessage( ghFindDlg, &msg ) )	continue;
			}
		}
#endif

		if( ghMaaFindDlg )
		{
			if( ghMaaFindDlg == GetForegroundWindow(  ) )
			{
				if( TranslateAccelerator( ghMaaFindDlg, ghAccelTable, &msg ) )	continue;
				if( IsDialogMessage( ghMaaFindDlg, &msg ) )	continue;
			}
		}

		if( !TranslateAccelerator( msg.hwnd, ghAccelTable, &msg ) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#ifdef PLUGIN_ENABLE

	plugin::FinalizePluginList( gPluginList );
	plugin::FinalizePlugin();

#endif

	InitParamValue( INIT_SAVE, VL_CLASHCOVER, 0 );

	UnregisterHotKey( ghMainWnd, IDHK_THREAD_DROP );

	if( ghMutex )
	{
		ReleaseMutex( ghMutex );
		CloseHandle( ghMutex );
	}

	TRACE( TEXT("Program Terminate") );

#ifdef WORK_LOG_OUT
	if( INVALID_HANDLE_VALUE != ghLogFlie )
	{
		SetEndOfFile( ghLogFlie );
		CloseHandle( ghLogFlie );
	}
#endif

	return (int)msg.wParam;
}

LPTSTR ExePathGet( VOID )
{
	return gatExePath;
}

ATOM InitWndwClass( HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ORINRINEDITOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ORINRINEDITOR);
	wcex.lpszClassName	= gszWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance( HINSTANCE hInstance, INT nCmdShow, LPTSTR ptArgv )
{
	HWND	hWnd;
	RECT	rect, wnRect;
	INT		isMaxim = 0, sptBuf = PLIST_DOCK;

	HMENU	hSubMenu;

	ghInst = hInstance;

	SplitBarClass( hInstance );

	isMaxim = InitParamValue( INIT_LOAD, VL_MAXIMISED, 0 );

	InitWindowPos( INIT_LOAD, WDP_MVIEW, &rect );
	if( 0 == rect.right || 0 == rect.bottom )
	{
		hWnd = GetDesktopWindow( );
		GetWindowRect( hWnd, &rect );
		rect.left   = ( rect.right  - W_WIDTH ) / 3;
		rect.top    = ( rect.bottom - W_HEIGHT ) / 2;
		rect.right  = W_WIDTH;
		rect.bottom = W_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_MVIEW, &rect );
	}

	hWnd = CreateWindowEx( 0, gszWindowClass, gszTitle, WS_OVERLAPPEDWINDOW, rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, hInstance, NULL);

	if( !hWnd ){	return FALSE;	}

	gbUniPad      = InitParamValue( INIT_LOAD, VL_USE_UNICODE,  1 );

	gbUniRadixHex = InitParamValue( INIT_LOAD, VL_UNIRADIX_HEX, 1 );

	gdBUInterval  = InitParamValue( INIT_LOAD, VL_BACKUP_INTVL, 3 );
	gbAutoBUmsg   = InitParamValue( INIT_LOAD, VL_BACKUP_MSGON, 1 );
	gbCrLfCode    = InitParamValue( INIT_LOAD, VL_CRLF_CODE, 0 );

	gbSaveMsgOn   = InitParamValue( INIT_LOAD, VL_SAVE_MSGON, 1 );

	gbTmpltDock   = InitParamValue( INIT_LOAD, VL_PLS_LN_DOCK,  1 );

	gdPageByteMax = InitParamValue( INIT_LOAD, VL_PAGEBYTE_MAX, PAGE_BYTE_MAX );

#ifdef SPMOZI_ENCODE
	gbSpMoziEnc   = InitParamValue( INIT_LOAD, VL_SPMOZI_ENC, 0 );
#endif

	ghMainWnd = hWnd;

	hSubMenu = GetSystemMenu( hWnd, FALSE );
	InsertMenu( hSubMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0 );
	InsertMenu( hSubMenu, 0, MF_BYPOSITION, IDM_POSITION_RESET, TEXT("窓配置リセット") );

	ghMenu = GetMenu( hWnd );

	hSubMenu = GetSubMenu( ghMenu , 0 );
	DeleteMenu( hSubMenu, IDM_TREE_RECONSTRUCT, MF_BYCOMMAND );
#ifndef ACCELERATOR_EDIT
	DeleteMenu( hSubMenu, IDM_ACCELKEY_EDIT_DLG_OPEN, MF_BYCOMMAND );
#endif

	hSubMenu = GetSubMenu( ghMenu , 1 );
#ifndef SEARCH_HIGHLIGHT
	DeleteMenu( hSubMenu, IDM_FIND_HIGHLIGHT_OFF, MF_BYCOMMAND );
#endif
#ifndef FIND_STRINGS
	DeleteMenu( hSubMenu, IDM_FIND_DLG_OPEN, MF_BYCOMMAND );

#endif

	hSubMenu = GetSubMenu( ghMenu , 4 );
	if( gbTmpltDock )
	{
		DeleteMenu( hSubMenu, IDM_PAGELIST_VIEW, MF_BYCOMMAND );
		DeleteMenu( hSubMenu, IDM_LINE_TEMPLATE, MF_BYCOMMAND );
		DeleteMenu( hSubMenu, IDM_BRUSH_PALETTE, MF_BYCOMMAND );
	}

	AaFontCreate( 1 );

	AppClientAreaCalc( &rect  );

	SqnSetting(  );

	if( gbTmpltDock )
	{

		grdSplitPos = InitParamValue( INIT_LOAD, VL_MAIN_SPLIT, PLIST_DOCK );
		if( grdSplitPos < SPLITBAR_WIDTH || rect.right <= grdSplitPos ){	grdSplitPos = PLIST_DOCK;	}
		sptBuf = grdSplitPos;

		ghMainSplitWnd = SplitBarCreate( hInstance, hWnd, rect.right - grdSplitPos, rect.top, rect.bottom );
	}
	else{	ghMainSplitWnd = NULL;	grdSplitPos = 0;	}

	ghPgVwWnd = PageListInitialise( hInstance, hWnd, &rect );

	ghLnTmplWnd = LineTmpleInitialise( hInstance, hWnd, &rect );

	ViewInitialise( hInstance, hWnd, &rect, ptArgv );
	ViewMaaItemsModeSet( InitParamValue( INIT_LOAD, VL_MAA_LCLICK, MAA_INSERT ), InitParamValue( INIT_LOAD, VL_MAA_MCLICK, MAA_INSERT ) );

	LayerBoxInitialise( hInstance, &rect );
	LayerBoxAlphaSet( InitParamValue( INIT_LOAD, VL_LAYER_TRANS, 192 ) );

	UniDlgInitialise( hWnd, TRUE );

	UserDefInitialise( hWnd, TRUE );

	PreviewInitialise( hInstance, hWnd );

	FrameNameModifyMenu( hWnd );

	TraceInitialise( hWnd, TRUE );

	OpenHistoryInitialise( hWnd );

	OpenProfileInitialise( hWnd );

	DocInverseInit( TRUE );

#ifdef FIND_STRINGS
	FindDialogueOpen( NULL, NULL );
#endif

#ifdef USE_HOVERTIP
	HoverTipInitialise( hInstance, hWnd );
#endif

	SetFocus( ghViewWnd );

	if( 1 <= gdBUInterval )
	{
		SetTimer( hWnd, IDT_BACKUP_TIMER, (gdBUInterval * 60000), NULL );
	}

	ghBrTmplWnd = BrushTmpleInitialise( hInstance, hWnd, &rect, ghMaaWnd );

	ghMaaWnd = MaaTmpltInitialise( hInstance, hWnd, &wnRect );

	MenuItemCheckOnOff( IDM_MAATMPLE_VIEW, MaaViewToggle( 0 ) );

	MenuItemCheckOnOff( IDM_UNICODE_TOGGLE , gbUniPad );

	DraughtInitialise( hInstance, hWnd );

	if( isMaxim )
	{
		ShowWindow( hWnd, SW_MAXIMIZE );
		AppClientAreaCalc( &rect  );
		if( ghMainSplitWnd )
		{
			grdSplitPos = sptBuf;
			SetWindowPos( ghMainSplitWnd, HWND_TOP, rect.right - grdSplitPos, rect.top, 0, 0, SWP_NOSIZE );
		}
		Cls_OnSize( hWnd, SIZE_MAXIMIZED, rect.right, rect.top );
	}
	else{	ShowWindow( hWnd, nCmdShow );	}

	UpdateWindow( hWnd );

	return TRUE;
}

#ifdef ACCELERATOR_EDIT

LPACCEL AccelKeyTableGetAlloc( LPINT piEntry )
{
	INT	iItems;
	LPACCEL	pstAccel = NULL;

	if( !(piEntry) )	return NULL;
	*piEntry = 0;

	iItems = CopyAcceleratorTable( ghAccelTable, NULL, 0 );
	if( 0 >= iItems )	return NULL;

	pstAccel = (LPACCEL)malloc( iItems * sizeof(ACCEL) );
	if( !(pstAccel) )	return NULL;

	iItems = CopyAcceleratorTable( ghAccelTable, pstAccel, iItems );

	*piEntry = iItems;

	return pstAccel;
}

HACCEL AccelKeyTableCreate( LPACCEL pstAccel, INT iEntry )
{
	HACCEL	hAccel;

	if( pstAccel )
	{
		DestroyAcceleratorTable( ghAccelTable );

		hAccel = CreateAcceleratorTable( pstAccel, iEntry );
	}
	else
	{
		hAccel = LoadAccelerators( ghInst, MAKEINTRESOURCE(IDC_ORINRINEDITOR) );
	}

	ghAccelTable = hAccel;

	AccelKeyMenuRewrite( ghMainWnd, pstAccel, iEntry );

	return hAccel;
}

#endif

HRESULT AppTitleChange( LPTSTR ptText )
{
	LPTSTR	ptName;
	TCHAR	atBuff[MAX_PATH];

	if( ptText )
	{
		if( NULL != ptText[0] ){	ptName = PathFindFileName( ptText );	}
		else{						ptName = NAMELESS_DUMMY;	}
		StringCchPrintf( atBuff, MAX_PATH, TEXT("%s - %s"), gszTitle, ptName );
	}
	else
	{
		StringCchCopy( atBuff, MAX_PATH, gszTitle );
	}

	SetWindowText( ghMainWnd, atBuff );

	return S_OK;
}

HRESULT AppTitleTrace( UINT bMode )
{
	static	TCHAR	atOrig[MAX_PATH];
	TCHAR	atBuff[MAX_PATH];

	if( bMode )
	{
		GetWindowText( ghMainWnd, atOrig, MAX_PATH );
		StringCchPrintf( atBuff, MAX_PATH, TEXT("%s [トレスモード]"), atOrig );
		SetWindowText( ghMainWnd, atBuff );
	}
	else
	{
		if( 0 != atOrig[0] ){	SetWindowText( ghMainWnd , atOrig );	}
		else{	SetWindowText( ghMainWnd, gszTitle );	}
	}

	return S_OK;
}

INT_PTR CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_INITDIALOG:
			SetDlgItemText( hDlg, IDE_ABOUT_DISP, gcatLicense );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	HMENU	hSubMenu;
#ifdef NDEBUG
	INT_PTR	iRslt;
#endif

	switch( message )
	{
		HANDLE_MSG( hWnd, WM_CREATE,		Cls_OnCreate );
		HANDLE_MSG( hWnd, WM_PAINT,			Cls_OnPaint );
		HANDLE_MSG( hWnd, WM_COMMAND,		Cls_OnCommand );
		HANDLE_MSG( hWnd, WM_DESTROY,		Cls_OnDestroy );
		HANDLE_MSG( hWnd, WM_SIZE,			Cls_OnSize );
		HANDLE_MSG( hWnd, WM_MOVE,			Cls_OnMove );
		HANDLE_MSG( hWnd, WM_DROPFILES,		Cls_OnDropFiles );
		HANDLE_MSG( hWnd, WM_ACTIVATE,		Cls_OnActivate );
		HANDLE_MSG( hWnd, WM_NOTIFY,		Cls_OnNotify  );
		HANDLE_MSG( hWnd, WM_TIMER,			Cls_OnTimer );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU,	Cls_OnContextMenu );
		HANDLE_MSG( hWnd, WM_HOTKEY,		Cls_OnHotKey  );

		HANDLE_MSG( hWnd, WM_DRAWITEM,		Cls_OnDrawItem );
#ifdef MULTIACT_RELAY
		HANDLE_MSG( hWnd, WM_COPYDATA,		Cls_OnCopyData );
#endif
		HANDLE_MSG( hWnd, WM_KEYDOWN,		Evw_OnKey );
		HANDLE_MSG( hWnd, WM_KEYUP,			Evw_OnKey );
		HANDLE_MSG( hWnd, WM_CHAR,			Evw_OnChar );
		HANDLE_MSG( hWnd, WM_MOUSEWHEEL,	Evw_OnMouseWheel );

		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING,	Cls_OnWindowPosChanging );

		case WM_MBUTTONUP:	TRACE( TEXT("MIDDLE  UP") );	break;

		case WM_SYSCOMMAND:
			if( IDM_POSITION_RESET == LOWORD(wParam) )
			{
				WindowPositionReset( hWnd );
				return 0;
			}
			break;

#ifdef NDEBUG
		case WM_CLOSE:
			iRslt = DocFileCloseCheck( hWnd, TRUE );
			if( !(iRslt)  ){	return FALSE;	}
			break;
#endif

#ifdef USE_NOTIFYICON

		case WMP_TRAYNOTIFYICON:
			TaskTrayIconEvent( hWnd, (UINT)wParam, (UINT)lParam );
			return 0;
#endif

		case WMP_BRUSH_TOGGLE:
			hSubMenu = GetSubMenu( ghMenu, 1 );
			CheckMenuItem( hSubMenu, IDM_BRUSH_STYLE, wParam ? MF_CHECKED : MF_UNCHECKED );
			return 0;

		case WMP_PREVIEW_CLOSE:
			DestroyCaret(  );
			ViewFocusSet(  );
			ViewShowCaret(  );
			return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Cls_OnActivate( HWND hWnd, UINT state, HWND hWndActDeact, BOOL fMinimized )
{
	LONG_PTR	rdExStyle;
	HWND		hWorkWnd;

	TRACE( TEXT("MAIN ACTIVATE STATE[%u] HWND[%X] MIN[%u]"), state, hWndActDeact, fMinimized );

	if( WA_INACTIVE == state )
	{
	}
	else
	{

		rdExStyle = GetWindowLongPtr( ghPgVwWnd, GWL_EXSTYLE );
		if( WS_EX_TOPMOST & rdExStyle )	hWorkWnd = HWND_TOPMOST;
		else							hWorkWnd = hWnd;
		SetWindowPos( ghPgVwWnd, hWorkWnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

		if( InitParamValue( INIT_LOAD, VL_MAA_TOPMOST, 1 ) )
		{
			rdExStyle = GetWindowLongPtr( ghMaaWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )	hWorkWnd = HWND_TOPMOST;
			else							hWorkWnd = hWnd;
			SetWindowPos( ghMaaWnd, hWorkWnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
		}
		else
		{
			ShowWindow( ghMaaWnd, SW_HIDE );
		}

		if( IsWindowVisible(ghLnTmplWnd ) )
		{
			rdExStyle = GetWindowLongPtr( ghLnTmplWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )	hWorkWnd = HWND_TOPMOST;
			else							hWorkWnd = hWnd;
			SetWindowPos( ghLnTmplWnd, hWorkWnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
		}

		if( IsWindowVisible(ghBrTmplWnd ) )
		{
			rdExStyle = GetWindowLongPtr( ghBrTmplWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )	hWorkWnd = HWND_TOPMOST;
			else							hWorkWnd = hWnd;
			SetWindowPos( ghBrTmplWnd, hWorkWnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
		}

		ViewFocusSet(  );
	}

	return;
}

BOOLEAN Cls_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	RECT	clRect, tbRect;

	RECT	tiRect;
	TCITEM	stTcItem;
	TTTOOLINFO	stToolInfo;

	HINSTANCE lcInst = lpCreateStruct->hInstance;

	DragAcceptFiles( hWnd, TRUE );

	GetClientRect( hWnd, &clRect );

#ifdef USE_NOTIFYICON
	NOTIFYICONDATA	nid;

	ZeroMemory( &nid, sizeof(NOTIFYICONDATA) );
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd   = hWnd;
	nid.uID    = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WMP_TRAYNOTIFYICON;
	nid.hIcon  = LoadIcon( lcInst, MAKEINTRESOURCE(IDI_ORINRINEDITOR) );
	StringCchCopy( nid.szTip, 128, gszTitle );
	Shell_NotifyIcon( NIM_ADD, &nid );
#endif

	ToolBarCreate( hWnd, lcInst );

	ghNameFont = CreateFont( 12, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, TEXT("MS UI Gothic") );

	ghFileTabWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("filetab"), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_SINGLELINE, 0, 0, clRect.right, 0, hWnd, (HMENU)IDTB_MULTIFILE, lcInst, NULL );
	SetWindowFont( ghFileTabWnd, ghNameFont, FALSE );

	gpfOriginFileTabProc = SubclassWindow( ghFileTabWnd, gpfFileTabProc );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask    = TCIF_TEXT;
	stTcItem.pszText = NAMELESS_DUMMY;
	TabCtrl_InsertItem( ghFileTabWnd, 0, &stTcItem );

	ToolBarSizeGet( &tbRect );
	TabCtrl_GetItemRect( ghFileTabWnd, 1, &tiRect );
	tiRect.bottom  += tiRect.top;
	MoveWindow( ghFileTabWnd, 0, tbRect.bottom, clRect.right, tiRect.bottom, TRUE );

	ghFileTabTip = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghFileTabWnd, NULL, lcInst, NULL );

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	GetClientRect( ghFileTabWnd, &stToolInfo.rect );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;
	stToolInfo.hwnd     = ghFileTabWnd;
	stToolInfo.uId      = IDTT_TILETAB_TIP;
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage( ghFileTabTip, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghFileTabTip, TTM_SETMAXTIPWIDTH, 0, 0 );

	ghStsRedBrush = CreateSolidBrush( 0xFF );

	ghStsBarWnd = CreateStatusWindow( WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, TEXT(""), hWnd, IDSB_VIEW_STATUS_BAR );
	SendMessage( ghStsBarWnd, SB_SETPARTS, (WPARAM)SB_ITEMS, (LPARAM)(LPINT)gadStsBarSize );

	StatusBar_SetText( ghStsBarWnd, 1 , TEXT("") );

	return TRUE;
}

VOID Cls_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
#ifdef NDEBUG
	 INT	iRslt;
#endif

	switch( id )
	{
		case  IDM_ABOUT:	DialogBox( ghInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About );	return;

		case IDM_EXIT:
#ifdef NDEBUG
			iRslt = DocFileCloseCheck( hWnd, TRUE );
			if( iRslt ){	DestroyWindow( hWnd );	}
#else
			DestroyWindow( hWnd );
#endif
			return;
	}

	OperationOnCommand( hWnd, id, hWndCtl, codeNotify );

	return;
}

VOID Cls_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	EndPaint( hWnd, &ps );

	return;
}

VOID Cls_OnTimer( HWND hWnd, UINT id )
{
	if( IDT_BACKUP_TIMER != id )	return;

	DocFileBackup( hWnd );

	return;
}

BOOL Cls_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
{
	RECT	winPos;

	GetWindowRect( hWnd, &winPos );
	winPos.right  = winPos.right - winPos.left;
	winPos.bottom = winPos.bottom - winPos.top;

	TRACE( TEXT("MAIN WM_WINDOWPOSCHANGING [%d %d %d %d][%d %d %d %d]"), winPos.left, winPos.top, winPos.right, winPos.bottom, pstWpos->x, pstWpos->y, pstWpos->cx, pstWpos->cy );

	giResizeWide = pstWpos->cx - winPos.right;

	return TRUE;
}

VOID Cls_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	static  UINT	ccState;

	RECT	rect;

	if( SIZE_MINIMIZED == state )
	{
		ShowWindow( ghMaaWnd, SW_HIDE );
		if( !(gbTmpltDock) )
		{
			ShowWindow( ghPgVwWnd, SW_HIDE );
			ShowWindow( ghLnTmplWnd, SW_HIDE );
			ShowWindow( ghBrTmplWnd, SW_HIDE );
		}
		ccState = SIZE_MINIMIZED;

		return;
	}

	if( SIZE_MINIMIZED == ccState &&  ccState != state )
	{
		ShowWindow( ghMaaWnd, SW_SHOW );
		if( !(gbTmpltDock) )
		{
			ShowWindow( ghPgVwWnd,   SW_SHOW );
			ShowWindow( ghLnTmplWnd, SW_SHOW );
			ShowWindow( ghBrTmplWnd, SW_SHOW );
		}
		ccState = SIZE_RESTORED;
	}

	if( SIZE_MAXIMIZED == state )
	{
#ifndef SPLIT_BAR_POS_FIX
		if( ghMainSplitWnd )
		{
			AppClientAreaCalc( &rect  );
			SetWindowPos( ghMainSplitWnd, HWND_TOP, rect.right - grdSplitPos, rect.top, 0, 0, SWP_NOSIZE );
		}
#endif
		ccState = SIZE_MAXIMIZED;
	}

	if( SIZE_RESTORED == state &&  SIZE_MAXIMIZED == ccState )
	{
		if( !(IsZoomed( hWnd ) ) )
		{
#ifndef SPLIT_BAR_POS_FIX
			if( ghMainSplitWnd )
			{
				AppClientAreaCalc( &rect  );
				SetWindowPos( ghMainSplitWnd, HWND_TOP, rect.right - grdSplitPos, rect.top, 0, 0, SWP_NOSIZE );
			}
#endif
			ccState = SIZE_RESTORED;
		}
	}

	MoveWindow( ghStsBarWnd, 0, 0, 0, 0, TRUE );

	ToolBarOnSize( hWnd, state, cx, cy );

	AppClientAreaCalc( &rect  );

#ifdef SPLIT_BAR_POS_FIX
	if( ghMainSplitWnd && (SIZE_SPLITBAR_MOVED != state) )
	{
		SetWindowPos( ghMainSplitWnd, HWND_TOP, rect.right - grdSplitPos, rect.top, 0, 0, SWP_NOSIZE );
	}
#endif
	ViewSizeMove( hWnd, &rect );

	return;
}

VOID Cls_OnMove( HWND hWnd, INT x, INT y )
{
	DWORD	dwStyle;

	dwStyle = GetWindowStyle( hWnd );
	if( dwStyle & WS_MINIMIZE )
	{
		LayerMoveFromView( hWnd, SIZE_MINIMIZED );
		FrameMoveFromView( hWnd, SIZE_MINIMIZED );
		MoziMoveFromView( hWnd, SIZE_MINIMIZED );
		VertMoveFromView( hWnd, SIZE_MINIMIZED );
	}
	else
	{
		LayerMoveFromView( hWnd, SIZE_RESTORED );
		FrameMoveFromView( hWnd, SIZE_RESTORED );
		MoziMoveFromView( hWnd, SIZE_RESTORED );
		VertMoveFromView( hWnd, SIZE_RESTORED );
	}

	return;
}

VOID Cls_OnDestroy( HWND hWnd )
{
	RECT	rect;
	DWORD	dwStyle;

#ifdef USE_NOTIFYICON
	NOTIFYICONDATA	nid;

	ZeroMemory( &nid, sizeof(NOTIFYICONDATA) );
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd   = hWnd;
	Shell_NotifyIcon( NIM_DELETE, &nid );
#endif

	DocInverseInit( FALSE );

#ifdef USE_HOVERTIP
	HoverTipInitialise( NULL, NULL );
#endif

	ToolBarBandInfoGet( NULL );

	PreviewInitialise( NULL, NULL );

	TraceInitialise( hWnd, FALSE );

	OpenHistoryInitialise( NULL );

	OpenProfileInitialise( NULL );

	BrushTmpleInitialise( NULL, NULL, NULL, NULL );

	LayerBoxInitialise( NULL, NULL );

	FrameInitialise( NULL, NULL );

	MoziInitialise( NULL, NULL );

	UniDlgInitialise( NULL, FALSE );

	VertInitialise( NULL, NULL );

	DraughtInitialise( NULL, NULL );

	CntxEditInitialise( NULL, NULL );

	dwStyle = GetWindowStyle( hWnd );
	if( !(dwStyle & WS_MINIMIZE) )
	{
		if( dwStyle & WS_MAXIMIZE ){	InitParamValue( INIT_SAVE, VL_MAXIMISED, 1 );	}
		else
		{

			GetWindowRect( ghMainWnd, &rect );
			rect.right  = rect.right  - rect.left;
			rect.bottom = rect.bottom - rect.top;
			InitWindowPos( INIT_SAVE, WDP_MVIEW, &rect );

			InitParamValue( INIT_SAVE, VL_MAXIMISED, 0 );
		}

		if( !(gbTmpltDock) )
		{
			GetWindowRect( ghPgVwWnd, &rect );
			rect.right  = rect.right  - rect.left;
			rect.bottom = rect.bottom - rect.top;
			InitWindowPos( INIT_SAVE, WDP_PLIST, &rect );

			GetWindowRect( ghLnTmplWnd, &rect );
			rect.right  = rect.right  - rect.left;
			rect.bottom = rect.bottom - rect.top;
			InitWindowPos( INIT_SAVE, WDP_LNTMPL, &rect );

			GetWindowRect( ghBrTmplWnd, &rect );
			rect.right  = rect.right  - rect.left;
			rect.bottom = rect.bottom - rect.top;
			InitWindowPos( INIT_SAVE, WDP_BRTMPL, &rect );
		}
	}

	DestroyWindow( ghMaaWnd );

	DestroyWindow( ghPgVwWnd );

	InitMultiFileTabOpen( INIT_SAVE, 0, NULL );

	DocInitialise( FALSE );

	SetWindowFont( ghFileTabWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );

	DeleteFont( ghNameFont );

	DeleteBrush( ghStsRedBrush );

	ToolBarDestroy(  );

	DestroyWindow( ghViewWnd );
	AaFontCreate( 0 );

	PostQuitMessage( 0 );

	return;
}

VOID Cls_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		curSel;
	INT		iCount;
	POINT	stPost;
	TCHITTESTINFO	stTcHitInfo;

	MENUITEMINFO	stMenuItemInfo;

	TCITEM	stTcItem;
	TCHAR	atText[MAX_PATH];
	UINT	cchSize;

	stPost.x = (SHORT)xPos;
	stPost.y = (SHORT)yPos;

	TRACE( TEXT("MAIN CONTEXT[%d x %d]"), stPost.x, stPost.y );

	if( ToolBarOnContextMenu( hWnd, hWndContext, stPost.x, stPost.y ) ){	 return;	}

	if( SUCCEEDED( DockingTabContextMenu( hWnd, hWndContext, stPost.x, stPost.y ) ) ){	 return;	}

	if( ghFileTabWnd != hWndContext ){	 return;	}

	iCount = TabCtrl_GetItemCount( ghFileTabWnd );

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_MULTIFILE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	stTcHitInfo.pt = stPost;
	ScreenToClient( ghFileTabWnd, &(stTcHitInfo.pt) );
	curSel = TabCtrl_HitTest( ghFileTabWnd, &stTcHitInfo );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask       = TCIF_TEXT | TCIF_PARAM;
	stTcItem.pszText    = atText;
	stTcItem.cchTextMax = MAX_PATH;
	TabCtrl_GetItem( ghFileTabWnd, curSel, &stTcItem );

	TabCtrl_SetCurSel( ghFileTabWnd, curSel );
	DocMultiFileSelect( stTcItem.lParam );

	StringCchCat( atText, MAX_PATH, TEXT(" を閉じる(&Q)") );
	StringCchLength( atText, MAX_PATH, &cchSize );

	ZeroMemory( &stMenuItemInfo, sizeof(MENUITEMINFO) );
	stMenuItemInfo.cbSize     = sizeof(MENUITEMINFO);
	stMenuItemInfo.fMask      = MIIM_TYPE;
	stMenuItemInfo.fType      = MFT_STRING;
	stMenuItemInfo.cch        = cchSize;
	stMenuItemInfo.dwTypeData = atText;

	if( 1 >= iCount )
	{
		stMenuItemInfo.fMask |= MIIM_STATE;
		stMenuItemInfo.fState = MFS_GRAYED;

	}

	SetMenuItemInfo( hSubMenu, IDM_FILE_CLOSE, FALSE, &stMenuItemInfo );

	dRslt = TrackPopupMenu( hSubMenu, 0, stPost.x, stPost.y, 0, hWnd, NULL );

	DestroyMenu( hMenu );

	return;
}

LRESULT Cls_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	INT	curSel;
	TCHAR	atText[MAX_PATH];
	TCITEM	stTcItem;
	LPNMTTDISPINFO	pstDispInfo;
	TCHITTESTINFO	stTcHitTest;

	ToolBarOnNotify( hWnd, idFrom, pstNmhdr );

	if( IDTB_DOCK_TAB == idFrom )
	{
		if( TCN_SELCHANGE == pstNmhdr->code )
		{
			curSel = TabCtrl_GetCurSel( pstNmhdr->hwndFrom );

			TRACE( TEXT("TMPL TAB sel [%d]"), curSel );

			if( gbDockTmplView )
			{

				switch( curSel )
				{
					case  0:	ShowWindow( ghLnTmplWnd , SW_SHOW );	ShowWindow( ghBrTmplWnd , SW_HIDE );	break;
					case  1:	ShowWindow( ghLnTmplWnd , SW_HIDE );	ShowWindow( ghBrTmplWnd , SW_SHOW );	break;
					default:	break;
				}
			}
		}
	}

	if( IDTB_MULTIFILE == idFrom )
	{
		if( TCN_SELCHANGE == pstNmhdr->code )
		{
			curSel = TabCtrl_GetCurSel( ghFileTabWnd );

			TRACE( TEXT("FILE TAB sel [%d]"), curSel );

			ZeroMemory( &stTcItem, sizeof(TCITEM) );
			stTcItem.mask       = TCIF_TEXT | TCIF_PARAM;
			stTcItem.pszText    = atText;
			stTcItem.cchTextMax = MAX_PATH;
			TabCtrl_GetItem( ghFileTabWnd, curSel, &stTcItem );

			TRACE( TEXT("FILE [%s] param[%d]"), atText, stTcItem.lParam );

			DocMultiFileSelect( stTcItem.lParam );
		}
	}

	if( IDTT_TILETAB_TIP == idFrom )
	{
		if( TTN_GETDISPINFO ==  pstNmhdr->code )
		{
			GetCursorPos( &(stTcHitTest.pt) );
			ScreenToClient( ghFileTabWnd, &(stTcHitTest.pt) );

			curSel = TabCtrl_HitTest( ghFileTabWnd, &stTcHitTest );
			TRACE( TEXT("FILE TAB under [%d]"), curSel );

			pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );

			pstDispInfo->lpszText = DocMultiFileNameGet( curSel );
			if( !(pstDispInfo->lpszText)  )
			{
				StringCchCopy( pstDispInfo->szText, 80, TEXT(" ") );
				pstDispInfo->lpszText = pstDispInfo->szText;
			}
		}
	}

	return 0;
}

#ifdef MULTIACT_RELAY

void Cls_OnCopyData( HWND hWnd, HWND hWndFrom, PCOPYDATASTRUCT pstCopyData )
{
	TCHAR	atBuff[MAX_PATH];

	StringCchCopy( atBuff, MAX_PATH, (LPTSTR)(pstCopyData->lpData) );

	TRACE( TEXT("COPYDATA[%s]"), atBuff );

	DocDoOpenFile( hWnd, atBuff );

	return;
}

#endif

VOID Cls_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	TRACE( TEXT("DROP[%s]"), atFileName );

	DocDoOpenFile( hWnd, atFileName );

	return;
}

VOID Cls_OnHotKey(HWND hWnd, INT idHotKey, UINT fuModifiers, UINT vk )
{
	if( VK_D == vk )
	{
		TRACE( TEXT("Hotkey Incoming!!") );
		DocThreadDropCopy(  );
	}

	return;
}

VOID Cls_OnDrawItem( HWND hWnd, CONST DRAWITEMSTRUCT *pstDrawItem )
{
	UINT	dBytes;
	TCHAR	atBuff[SUB_STRING];
	RECT	rect;

	if( IDSB_VIEW_STATUS_BAR != pstDrawItem->CtlID )	return;

	if( SB_BYTECNT == pstDrawItem->itemID )
	{
		SetBkMode( pstDrawItem->hDC, TRANSPARENT );
		rect = pstDrawItem->rcItem;

		dBytes =  pstDrawItem->itemData;
		StringCchPrintf( atBuff, SUB_STRING, TEXT("%d Bytes"), dBytes );

		if( gdPageByteMax < dBytes )	FillRect( pstDrawItem->hDC, &(pstDrawItem->rcItem), ghStsRedBrush );

		DrawText( pstDrawItem->hDC, atBuff, -1, &(rect), DT_LEFT | DT_VCENTER | DT_SINGLELINE );
	}

	return;
}

HRESULT MainStatusBarSetText( INT room, LPCTSTR ptText )
{
	StatusBar_SetText( ghStsBarWnd, room , ptText );

	return S_OK;
}

HRESULT MainSttBarSetByteCount( UINT dByte )
{
	SendMessage( ghStsBarWnd, SB_SETTEXT, (WPARAM)(SB_BYTECNT | SBT_OWNERDRAW), (LPARAM)dByte );

	return S_OK;
}

HRESULT MenuItemCheckOnOff( UINT itemID, UINT bCheck )
{
	CheckMenuItem( ghMenu, itemID, bCheck ? MF_CHECKED : MF_UNCHECKED );

	ToolBarCheckOnOff( itemID, bCheck );

	return S_OK;
}

HRESULT BrushModeToggle( VOID )
{
	LRESULT	rlst;
	HMENU	hSubMenu;

	rlst = SendMessage( ghBrTmplWnd, WMP_BRUSH_TOGGLE, 0, 0 );
	hSubMenu = GetSubMenu( ghMenu, 1 );
	CheckMenuItem( hSubMenu, IDM_BRUSH_STYLE, rlst ? MF_CHECKED : MF_UNCHECKED );

	return S_OK;
}

UINT AppClientAreaCalc( LPRECT pstRect )
{
	RECT	rect, sbRect, tbRect;
	RECT	ftRect;

	if( !(pstRect) )	return 0;

	if( !(ghMainWnd)  ){	SetRect( pstRect , 0, 0, 0, 0 );	return 0;	}

	GetClientRect( ghMainWnd, &rect );

	ToolBarSizeGet( &tbRect );
	rect.top      += tbRect.bottom;
	rect.bottom   -= tbRect.bottom;

	GetClientRect( ghStsBarWnd, &sbRect );
	rect.bottom   -= sbRect.bottom;

	GetWindowRect( ghFileTabWnd, &ftRect );
	ftRect.bottom -= ftRect.top;
	SetWindowPos( ghFileTabWnd, HWND_TOP, 0, tbRect.bottom, rect.right, ftRect.bottom, SWP_NOZORDER );
	rect.top      += ftRect.bottom;
	rect.bottom   -= ftRect.bottom;

	SetRect( pstRect, rect.left, rect.top, rect.right, rect.bottom );

	return 1;
}

INT InitTraceValue( UINT dMode, LPTRACEPARAM pstInfo )
{
	TCHAR	atBuff[MIN_STRING];
	INT	iBuff = 0;

	if( dMode )
	{
		iBuff = GetPrivateProfileInt( TEXT("Trace"), TEXT("Turning"), -1, gatIniPath );
		if( 0 > iBuff )	return 0;

		pstInfo->dTurning     = iBuff;
		pstInfo->dZooming     = GetPrivateProfileInt( TEXT("Trace"), TEXT("Zooming"), 0, gatIniPath );
		pstInfo->dGrayMoph    = GetPrivateProfileInt( TEXT("Trace"), TEXT("GrayMoph"), 0, gatIniPath );
		pstInfo->dGamma       = GetPrivateProfileInt( TEXT("Trace"), TEXT("Gamma"), 0, gatIniPath );
		pstInfo->dContrast    = GetPrivateProfileInt( TEXT("Trace"), TEXT("Contrast"), 0, gatIniPath );
		pstInfo->stOffsetPt.y = GetPrivateProfileInt( TEXT("Trace"), TEXT("OffsetY"), 0, gatIniPath );
		pstInfo->stOffsetPt.x = GetPrivateProfileInt( TEXT("Trace"), TEXT("OffsetX"), 0, gatIniPath );
		pstInfo->bUpset       = GetPrivateProfileInt( TEXT("Trace"), TEXT("Upset"), 0, gatIniPath );
		pstInfo->bMirror      = GetPrivateProfileInt( TEXT("Trace"), TEXT("Mirror"), 0, gatIniPath );
		pstInfo->dMoziColour  = GetPrivateProfileInt( TEXT("Trace"), TEXT("MoziColour"), 0, gatIniPath );
	}
	else
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->stOffsetPt.x );
		WritePrivateProfileString( TEXT("Trace"), TEXT("OffsetX"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->stOffsetPt.y );
		WritePrivateProfileString( TEXT("Trace"), TEXT("OffsetY"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dContrast );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Contrast"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dGamma );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Gamma"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dGrayMoph );
		WritePrivateProfileString( TEXT("Trace"), TEXT("GrayMoph"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dZooming );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Zooming"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dTurning );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Turning"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo->bUpset );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Upset"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo->bMirror );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Mirror"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo->dMoziColour );
		WritePrivateProfileString( TEXT("Trace"), TEXT("MoziColour"), atBuff, gatIniPath );
	}

	return 1;
}

COLORREF InitColourValue( UINT dMode, UINT dStyle, COLORREF nColour )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	LPTSTR	ptEnd;

	switch( dStyle )
	{
		case CLRV_BASICPEN:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BasicPen") );	break;
		case CLRV_BASICBK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BasicBack") );	break;
		case CLRV_GRIDLINE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("GridLine") );	break;
		case CLRV_CRLFMARK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CrLfMark") );	break;
		case CLRV_CANTSJIS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CantSjis") );	break;
		default:	return nColour;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%08X"), nColour );

	if( dMode )
	{
		GetPrivateProfileString( TEXT("Colour"), atKeyName, atBuff, atBuff, MIN_STRING, gatIniPath );
		nColour = (COLORREF)_tcstoul( atBuff, &ptEnd, 16 );
	}
	else
	{
		WritePrivateProfileString( TEXT("Colour"), atKeyName, atBuff, gatIniPath );
	}

	return nColour;
}

HRESULT InitToolBarLayout( UINT dMode, INT items, LPREBARLAYOUTINFO pstInfo )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	UINT	dValue;
	INT	i;

	if( dMode )
	{

		dValue = GetPrivateProfileInt( TEXT("ReBar"), TEXT("IDX0_ID"), 0, gatIniPath );
		if( 0 == dValue ){	return E_NOTIMPL;	}

		for( i = 0; items > i; i++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_ID"), i );
			pstInfo[i].wID = GetPrivateProfileInt( TEXT("ReBar"), atKeyName, pstInfo[i].wID, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_CX"), i );
			pstInfo[i].cx = GetPrivateProfileInt( TEXT("ReBar"), atKeyName, pstInfo[i].cx, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_STYLE"), i );
			pstInfo[i].fStyle = GetPrivateProfileInt( TEXT("ReBar"), atKeyName, pstInfo[i].fStyle, gatIniPath );
		}
	}
	else
	{
		WritePrivateProfileSection( TEXT("ReBar"), NULL, gatIniPath );

		for( i = 0; items > i; i++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_ID"), i );
			StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo[i].wID );
			WritePrivateProfileString( TEXT("ReBar"), atKeyName, atBuff, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_CX"), i );
			StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo[i].cx );
			WritePrivateProfileString( TEXT("ReBar"), atKeyName, atBuff, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_STYLE"), i );
			StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo[i].fStyle );
			WritePrivateProfileString( TEXT("ReBar"), atKeyName, atBuff, gatIniPath );
		}
	}

	return S_OK;
}

INT InitParamValue( UINT dMode, UINT dStyle, INT nValue )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	INT	iBuff = 0;

	switch( dStyle )
	{
		case  VL_CLASHCOVER:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ClashCover") );		break;
		case  VL_GROUP_UNDO:	StringCchCopy( atKeyName, SUB_STRING, TEXT("GroupUndo") );		break;
		case  VL_USE_UNICODE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UseUnicode")  );	break;
		case  VL_LAYER_TRANS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("LayerTrans")  );	break;
		case  VL_RIGHT_SLIDE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("RightSlide")  );	break;
		case  VL_MAA_SPLIT:		StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSplit") );		break;
		case  VL_MAA_LCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaMethod") );		break;
		case  VL_UNILISTLAST:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UniListLast") );	break;
		case  VL_MAATIP_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaToolTip")  );	break;
		case  VL_MAATIP_SIZE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaToolTipSize") );	break;
		case  VL_LINETMP_CLM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("LineTmplClm") );	break;
		case  VL_BRUSHTMP_CLM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BrushTmplClm") );	break;
		case  VL_UNIRADIX_HEX:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UniRadixHex") );	break;
		case  VL_BACKUP_INTVL:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BackUpIntvl") );	break;
		case  VL_BACKUP_MSGON:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BackUpMsgOn") );	break;
		case  VL_CRLF_CODE:		StringCchCopy( atKeyName, SUB_STRING, TEXT("CrLfCode") );		break;
		case  VL_SPACE_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("SpaceView") );		break;
		case  VL_GRID_X_POS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("GridXpos") );		break;
		case  VL_GRID_Y_POS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("GridYpos") );		break;
		case  VL_GRID_VIEW:		StringCchCopy( atKeyName, SUB_STRING, TEXT("GridView") );		break;
		case  VL_MAA_TOPMOST:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaViewOn") );		break;
		case  VL_R_RULER_POS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("RightRuler")  );	break;
		case  VL_R_RULER_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("RitRulerView") );	break;
		case  VL_PAGETIP_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageToolTip") );	break;
		case  VL_PCOMBINE_NM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageCombineNM") );	break;
		case  VL_PDIVIDE_NM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageDivideNM") );	break;
		case  VL_PDELETE_NM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageDeleteNM") );	break;
		case  VL_MAASEP_STYLE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSepLine")  );	break;
		case  VL_PLS_LN_DOCK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PLstLineDock") );	break;

		case  VL_SWAP_COPY:		StringCchCopy( atKeyName, SUB_STRING, TEXT("CopyModeSwap") );	break;
		case  VL_MAIN_SPLIT:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MainSplit") );		break;
		case  VL_MAXIMISED:		StringCchCopy( atKeyName, SUB_STRING, TEXT("Maximised") );		break;
		case  VL_DRT_LCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("DraughtDefault") );	break;
		case  VL_FIRST_READED:	StringCchCopy( atKeyName, SUB_STRING, TEXT("FirstStep") );		break;
		case  VL_LAST_OPEN:		StringCchCopy( atKeyName, SUB_STRING, TEXT("LastOpenStyle") );	break;
		case  VL_MAA_MCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSubMethod") );	break;
		case  VL_DRT_MCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("DraughtSubDef") );	break;
		case  VL_WORKLOG:		StringCchCopy( atKeyName, SUB_STRING, TEXT("WorkLogDebug") );	break;
		case  VL_PAGE_UNDER:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageNumUnder") );	break;
		case  VL_PAGE_OVWRITE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageNumOvWrite") );	break;
		case  VL_MAA_RETFCS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaRetFocus") );	break;
		case  VL_PGL_RETFCS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageRetFocus") );	break;
		case  VL_U_RULER_POS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UnderRuler")  );	break;
		case  VL_U_RULER_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UndRulerView") );	break;
		case  VL_PAGEBYTE_MAX:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageByteMax") );	break;
		case  VL_MAA_BKCOLOUR:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaBkColour") );	break;
		case  VL_THUMB_HORIZ:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ThumbHoriz")  );	break;
		case  VL_THUMB_VERTI:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ThumbVerti")  );	break;
		case  VL_MULTI_ACT_E:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MultiAct") );		break;
		case  VL_SAVE_MSGON:	StringCchCopy( atKeyName, SUB_STRING, TEXT("SaveMsgOn") );		break;
		case  VL_MAATAB_SNGL:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaTabSingle") );	break;
		case  VL_HINT_ENABLE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("HintPopup") );		break;
		case  VL_SPMOZI_ENC:	StringCchCopy( atKeyName, SUB_STRING, TEXT("SpMoziEnc") );		break;
		default:	return nValue;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );

	if( dMode )
	{
		GetPrivateProfileString( TEXT("General"), atKeyName, atBuff, atBuff, MIN_STRING, gatIniPath );
		iBuff = StrToInt( atBuff );
	}
	else
	{
		WritePrivateProfileString( TEXT("General"), atKeyName, atBuff, gatIniPath );
	}

	return iBuff;
}

HRESULT InitParamString( UINT dMode, UINT dStyle, LPTSTR ptStr )
{
	TCHAR	atKeyName[MIN_STRING], atDefault[MAX_PATH];

	if( !(ptStr) )	return E_INVALIDARG;

	switch( dStyle )
	{
		case VS_PROFILE_NAME:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ProfilePath") );	break;
		case VS_PAGE_FORMAT:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageFormat")  );	break;
		case VS_FONT_NAME:		StringCchCopy( atKeyName, SUB_STRING, TEXT("FontName") );		break;
		case VS_UNI_USE_LOG:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UniUseLog") );		break;
		case VS_RGUIDE_MOZI:	StringCchCopy( atKeyName, SUB_STRING, TEXT("RightGuideMozi") );	break;
		case VS_EXT_M2H_PATH:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ExtM2HPath") );		break;
		default:	return E_INVALIDARG;
	}

	if( dMode )
	{
		StringCchCopy( atDefault, MAX_PATH, ptStr );
		GetPrivateProfileString( TEXT("General"), atKeyName, atDefault, ptStr, MAX_PATH, gatIniPath );
	}
	else
	{
		WritePrivateProfileString( TEXT("General"), atKeyName, ptStr, gatIniPath );
	}

	return S_OK;
}

INT InitWindowTopMost( UINT dMode, UINT dStyle, INT nValue )
{
	TCHAR	atAppName[MIN_STRING], atBuff[MIN_STRING];
	INT	iBuff = 0;

	switch( dStyle )
	{
		case  WDP_PLIST:	StringCchCopy( atAppName, SUB_STRING, TEXT("PageList") );	break;
		case  WDP_LNTMPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("LineTmple") );	break;
		case  WDP_BRTMPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("BrushTmple") );	break;
		case  WDP_MAATPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("MaaTmple") );	break;
		default:	return 0;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );

	if( dMode )
	{
		GetPrivateProfileString( atAppName, TEXT("TopMost"), atBuff, atBuff, MIN_STRING, gatIniPath );
		iBuff = StrToInt( atBuff );
	}
	else
	{
		WritePrivateProfileString( atAppName, TEXT("TopMost"), atBuff, gatIniPath );
	}

	return iBuff;
}

HRESULT InitWindowPos( UINT dMode, UINT dStyle, LPRECT pstRect )
{
	TCHAR	atAppName[MIN_STRING], atBuff[MIN_STRING];

	if( !pstRect )	return E_INVALIDARG;

	switch( dStyle )
	{
		case  WDP_MVIEW:	StringCchCopy( atAppName, SUB_STRING, TEXT("MainView") );	break;
		case  WDP_PLIST:	StringCchCopy( atAppName, SUB_STRING, TEXT("PageList") );	break;
		case  WDP_LNTMPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("LineTmple") );	break;
		case  WDP_BRTMPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("BrushTmple") );	break;
		case  WDP_MAATPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("MaaTmple") );	break;
		case  WDP_PREVIEW:	StringCchCopy( atAppName, SUB_STRING, TEXT("Preview") );	break;
		case  WDP_MMAATPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("MnMaaTmple") );	break;
		default:	return E_INVALIDARG;
	}

	if( dMode )
	{
		GetPrivateProfileString( atAppName, TEXT("LEFT"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->left   = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("TOP"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->top    = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("RIGHT"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->right  = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("BOTTOM"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->bottom = StrToInt( atBuff );
	}
	else
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->left );
		WritePrivateProfileString( atAppName, TEXT("LEFT"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->top );
		WritePrivateProfileString( atAppName, TEXT("TOP"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->right );
		WritePrivateProfileString( atAppName, TEXT("RIGHT"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->bottom );
		WritePrivateProfileString( atAppName, TEXT("BOTTOM"), atBuff, gatIniPath );
	}

	return S_OK;
}

#if 0

HRESULT InitLastOpen( UINT dMode, LPTSTR ptFile )
{
	if(  !(ptFile) )	return E_INVALIDARG;
	if( dMode ){	GetPrivateProfileString( TEXT("History"), TEXT("LastOpen"), TEXT(""), ptFile, MAX_PATH, gatIniPath );	}
	else{			WritePrivateProfileString( TEXT("History") , TEXT("LastOpen"), ptFile, gatIniPath );	}

	return S_OK;
}

#endif

HRESULT WindowFocusChange( INT nowWnd, INT iDir )
{
	INT	nextWnd;

	if( gbTmpltDock )
	{
		if( WND_MAIN == nowWnd )	nextWnd = WND_MAAT;
		else						nextWnd = WND_MAIN;
	}
	else
	{
		nextWnd = nowWnd + iDir;
		if( 0 >= nextWnd )			nextWnd = WND_BRUSH;
		if( WND_BRUSH < nextWnd )	nextWnd = WND_MAIN;

	}

	switch( nextWnd )
	{
		default:
		case WND_MAIN:	SetForegroundWindow( ghMainWnd );	break;
		case WND_MAAT:	SetForegroundWindow( ghMaaWnd );	break;
		case WND_PAGE:	SetForegroundWindow( ghPgVwWnd );	break;
		case WND_LINE:	SetForegroundWindow( ghLnTmplWnd );	break;
		case WND_BRUSH:	SetForegroundWindow( ghBrTmplWnd );	break;
	}

	return S_OK;
}

HRESULT InitProfHistory( UINT dMode, UINT dNumber, LPTSTR ptFile )
{
	TCHAR	atKeyName[MIN_STRING], atDefault[MAX_PATH];

	if( dMode  )
	{
		ZeroMemory( ptFile, sizeof(TCHAR) * MAX_PATH );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("Hist%X"), dNumber );
		GetPrivateProfileString( TEXT("ProfHistory"), atKeyName, TEXT(""), atDefault, MAX_PATH, gatIniPath );

		if( NULL == atDefault[0] )	return E_NOTIMPL;

		StringCchCopy( ptFile, MAX_PATH, atDefault );
	}
	else
	{
		if( ptFile )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Hist%X"), dNumber );
			WritePrivateProfileString( TEXT("ProfHistory"), atKeyName, ptFile, gatIniPath );
		}
		else
		{
			WritePrivateProfileSection( TEXT("ProfHistory"), NULL, gatIniPath );
		}
	}

	return S_OK;
}

HRESULT WindowPositionReset( HWND hWnd )
{
	HWND	hWorkWnd;
	RECT	rect;

	TRACE(TEXT("★位置情報リセット") );

	hWorkWnd = GetDesktopWindow( );
	GetWindowRect( hWorkWnd, &rect );
	rect.left   = ( rect.right  - W_WIDTH ) / 3;
	rect.top    = ( rect.bottom - W_HEIGHT ) / 2;
	rect.right  = W_WIDTH;
	rect.bottom = W_HEIGHT;

	SetWindowPos( ghMainWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW );

	if( ghMainSplitWnd )
	{
		AppClientAreaCalc( &rect  );
		SetWindowPos( ghMainSplitWnd, HWND_TOP, rect.right - PLIST_DOCK, rect.top, 0, 0, SWP_NOSIZE );

		ViewSizeMove( hWnd, &rect );
	}
	else
	{
		PageListPositionReset( ghMainWnd );
		LineTmplePositionReset( ghMainWnd );
		BrushTmplePositionReset( ghMainWnd );
	}

	MaaTmpltPositionReset( ghMainWnd );

	return S_OK;
}

HRESULT OpenHistoryInitialise( HWND hWnd )
{
	HMENU	hSubMenu;
	TCHAR	atKeyName[MIN_STRING], atString[MAX_PATH+10];
	UINT	d;
	UINT_PTR	dItems;
	OPENHIST	stOpenHist;
	OPHIS_ITR	itHist;

	if( hWnd )
	{
		gltOpenHist.clear( );

		for( d = 0; OPENHIST_MAX > d; d++ )
		{
			ZeroMemory( &stOpenHist, sizeof(OPENHIST) );
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Hist%X"), d );
			GetPrivateProfileString( TEXT("OpenHistory"), atKeyName, TEXT(""), stOpenHist.atFile, MAX_PATH, gatIniPath );
			if( NULL == stOpenHist.atFile[0] )	break;

			gltOpenHist.push_back( stOpenHist );
		}

		if( ghHistyMenu )	DestroyMenu( ghHistyMenu );

		ghHistyMenu = CreatePopupMenu(  );
		AppendMenu( ghHistyMenu, MF_SEPARATOR, 0, NULL );
		AppendMenu( ghHistyMenu, MF_STRING, IDM_OPEN_HIS_CLEAR, TEXT("履歴クリヤ") );

		dItems = gltOpenHist.size( );
		if( 0 == dItems )
		{

			InsertMenu( ghHistyMenu, 0, MF_STRING | MF_BYPOSITION | MF_GRAYED, IDM_OPEN_HIS_FIRST, TEXT("(无)") );
		}
		else
		{

			for( itHist = gltOpenHist.begin(), d = dItems-1; gltOpenHist.end() != itHist; itHist++, d-- )
			{
				StringCchPrintf( atString, MAX_PATH+10, TEXT("(&%X) %s"), d, itHist->atFile );
				InsertMenu( ghHistyMenu, 0, MF_STRING | MF_BYPOSITION, (IDM_OPEN_HIS_FIRST + d), atString );
				itHist->dMenuNumber = (IDM_OPEN_HIS_FIRST + d);
			}
		}

#pragma message ("ファイルオーポン履歴の基点メニュー、位置に注意")

		hSubMenu = GetSubMenu( ghMenu, 0 );
		ModifyMenu( hSubMenu, 2, MF_BYPOSITION | MF_POPUP, (UINT_PTR)ghHistyMenu, TEXT("ファイル使用履歴(&H)") );

		DrawMenuBar( hWnd );
	}
	else
	{
		if( ghHistyMenu )	DestroyMenu( ghHistyMenu );

		WritePrivateProfileSection( TEXT("OpenHistory"), NULL, gatIniPath );

		for( itHist = gltOpenHist.begin(), d = 0; gltOpenHist.end() != itHist; itHist++, d++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Hist%X"), d );
			WritePrivateProfileString( TEXT("OpenHistory"), atKeyName, itHist->atFile, gatIniPath );
		}

	}

	return S_OK;
}

HRESULT OpenHistoryLoad( HWND hWnd, INT id )
{
	UINT_PTR	dNumber, dItems;
	LPARAM		lUnique;
	OPHIS_ITR	itHist;

	dNumber = id - IDM_OPEN_HIS_FIRST;

	TRACE( TEXT("履歴 -> %d"), dNumber );
	if( OPENHIST_MAX <= dNumber ){	return E_OUTOFMEMORY;	}

	dItems = gltOpenHist.size();
	dNumber = (dItems-1) - dNumber;

	itHist = gltOpenHist.begin();
	std::advance( itHist , dNumber );

	lUnique = DocOpendFileCheck( itHist->atFile );
	if( 1 <= lUnique )
	{
		if( SUCCEEDED( MultiFileTabSelect( dNumber ) ) )
		{
			DocMultiFileSelect( lUnique );
		}
	}
	else
	{
		DocDoOpenFile( hWnd, itHist->atFile );
	}

	return S_OK;
}

HRESULT OpenHistoryLogging( HWND hWnd, LPTSTR ptFile )
{
	UINT_PTR	dItems;
	OPENHIST	stOpenHist;
	OPHIS_ITR	itHist;

	if( ptFile )
	{
		ZeroMemory( &stOpenHist, sizeof(OPENHIST) );

		StringCchCopy( stOpenHist.atFile, MAX_PATH, ptFile );

		for( itHist = gltOpenHist.begin(); gltOpenHist.end() != itHist; itHist++ )
		{
			if( !StrCmp( itHist->atFile, stOpenHist.atFile ) )
			{
				gltOpenHist.erase( itHist );
				break;
			}
		}

		gltOpenHist.push_back( stOpenHist );

		dItems = gltOpenHist.size( );
		if( OPENHIST_MAX <  dItems )
		{
			gltOpenHist.pop_front(  );
		}
	}
	else
	{
		gltOpenHist.clear();
	}

	OpenHistoryInitialise( NULL );
	OpenHistoryInitialise( hWnd );

	return S_OK;
}

HRESULT OptionDialogueOpen( VOID )
{
	UINT	bURHbuff, bABUIbuff;
	TCHAR	atFontName[LF_FACESIZE];

	ZeroMemory( atFontName, sizeof(atFontName) );

	bURHbuff  = gbUniRadixHex;
	bABUIbuff = gdBUInterval;

	DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_GENERAL_OPTION_DLG), ghMainWnd, OptionDlgProc, (LPARAM)atFontName );

	if( bURHbuff != gbUniRadixHex ){	UnicodeRadixExchange( NULL );	};
	if( bABUIbuff != gdBUInterval )
	{
		if( 1 <= bABUIbuff )	KillTimer( ghMainWnd, IDT_BACKUP_TIMER );
		if( 1 <= gdBUInterval )	SetTimer(  ghMainWnd, IDT_BACKUP_TIMER, (gdBUInterval * 60000), NULL );
	}

	MenuItemCheckOnOff( IDM_UNICODE_TOGGLE , gbUniPad );

	SqnSetting(   );

	if( NULL != atFontName[ 0] )
	{
		InitParamString( INIT_SAVE, VS_FONT_NAME, atFontName );

	}

	return S_OK;
}

VOID OptionExtMlt2HtmlPath( HWND hDlg )
{
	OPENFILENAME	stOpenFile;

	 BOOLEAN	bOpened;
	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];

	ZeroMemory( &stOpenFile, sizeof(OPENFILENAME) );

	StringCchCopy( atFilePath, MAX_PATH, TEXT("MLT2HTML.exe") );
	ZeroMemory( atFileName,  sizeof(atFileName) );

	stOpenFile.lStructSize     = sizeof(OPENFILENAME);
	stOpenFile.hwndOwner       = hDlg;
	stOpenFile.lpstrFilter     = TEXT("実行ファイル\0*.exe\0\0");
	stOpenFile.lpstrFile       = atFilePath;
	stOpenFile.nMaxFile        = MAX_PATH;
	stOpenFile.lpstrFileTitle  = atFileName;
	stOpenFile.nMaxFileTitle   = MAX_STRING;
	stOpenFile.lpstrTitle      = TEXT("実行ファイルを指定しておくれ");
	stOpenFile.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
	stOpenFile.lpstrDefExt     = TEXT("exe");

	bOpened = GetOpenFileName( &stOpenFile );

	if( !(bOpened) ){	 return;	}

	SetDlgItemText( hDlg, IDE_EXTERNAL_M2H_PATH, atFilePath );

	return;
}

INT_PTR CALLBACK OptionDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static  UINT	cdGrXp, cdGrYp, cdRtRr, cdUdRr;

	UINT	id;
	INT		dValue, iBuff;
	TCHAR	atBuff[SUB_STRING];
	TCHAR	atPath[MAX_PATH];

	static LPTSTR	ptFontName;
	CHOOSEFONT	stChooseFont;
	LOGFONT		stLogFont;

	switch( message )
	{
		case WM_INITDIALOG:
			ptFontName = (LPTSTR)lParam;

			SendDlgItemMessage( hDlg, IDSL_LAYERBOX_TRANCED, TBM_SETRANGE, TRUE, MAKELPARAM( 0, 0xE0 ) );

			dValue = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, 16 );
			if( FONTSZ_REDUCE == dValue )	CheckRadioButton( hDlg, IDRB_POPUP_NOMAL, IDRB_POPUP_REDUCE, IDRB_POPUP_REDUCE );
			else							CheckRadioButton( hDlg, IDRB_POPUP_NOMAL, IDRB_POPUP_REDUCE, IDRB_POPUP_NOMAL );
			dValue = InitParamValue( INIT_LOAD, VL_MAATIP_VIEW, 1 );
			CheckDlgButton( hDlg, IDCB_POPUP_VISIBLE, dValue ? BST_CHECKED : BST_UNCHECKED );

			dValue = InitParamValue( INIT_LOAD, VL_RIGHT_SLIDE, 790 );
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), dValue );
			Edit_SetText( GetDlgItem(hDlg,IDE_RIGHTSLIDE_DOT), atBuff );

			cdGrXp = gdGridXpos;
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), gdGridXpos );
			Edit_SetText( GetDlgItem(hDlg,IDE_GRID_X_POS), atBuff );

			cdGrYp = gdGridYpos;
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), gdGridYpos );
			Edit_SetText( GetDlgItem(hDlg,IDE_GRID_Y_POS), atBuff );

			cdRtRr = gdRightRuler;
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), gdRightRuler );
			Edit_SetText( GetDlgItem(hDlg,IDE_RIGHT_RULER_POS), atBuff );

			cdUdRr = gdUnderRuler;
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), gdUnderRuler );
			Edit_SetText( GetDlgItem(hDlg,IDE_UNDER_RULER_POS), atBuff );

			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), gdPageByteMax );
			Edit_SetText( GetDlgItem(hDlg,IDE_PAGE_BYTE_MAX), atBuff );

			dValue = InitParamValue( INIT_LOAD, VL_BACKUP_INTVL, 3 );
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), dValue );
			Edit_SetText( GetDlgItem(hDlg,IDE_AUTO_BU_INTVL), atBuff );

			CheckDlgButton( hDlg, IDCB_AUTOBU_MSG_ON, gbAutoBUmsg ? BST_CHECKED : BST_UNCHECKED );

			CheckDlgButton( hDlg, IDCB_SAVE_MSG_ON, gbSaveMsgOn ? BST_CHECKED : BST_UNCHECKED );

			CheckRadioButton( hDlg, IDRB_CRLF_STRB, IDRB_CRLF_2CH_YY, gbCrLfCode ? IDRB_CRLF_2CH_YY : IDRB_CRLF_STRB );

			CheckDlgButton( hDlg, IDCB_USE_UNISPACE_SET, gbUniPad ? BST_CHECKED : BST_UNCHECKED );

			CheckDlgButton( hDlg, IDCB_UNIRADIX_HEX, gbUniRadixHex ? BST_CHECKED : BST_UNCHECKED );

			dValue = InitParamValue( INIT_LOAD, VL_GROUP_UNDO, 1 );
			CheckDlgButton( hDlg, IDCB_GROUPUNDO_SET, dValue ? BST_CHECKED : BST_UNCHECKED );

			dValue = InitParamValue( INIT_LOAD, VL_MULTI_ACT_E, 0 );
			CheckDlgButton( hDlg, IDCB_MULTIACT_ENA, dValue ? BST_CHECKED : BST_UNCHECKED );

			dValue = InitParamValue( INIT_LOAD, VL_PLS_LN_DOCK, 1 );
			CheckDlgButton( hDlg, IDCB_DOCKING_STYLE, dValue ? BST_CHECKED : BST_UNCHECKED );

			dValue = InitParamValue( INIT_LOAD, VL_LAST_OPEN, LASTOPEN_DO );
			CheckRadioButton( hDlg, IDRB_LASTOPEN_DO, IDRB_LASTOPEN_ASK, (IDRB_LASTOPEN_DO + dValue) );

#ifdef TODAY_HINT_STYLE

			dValue = InitParamValue( INIT_LOAD, VL_HINT_ENABLE, 1 );
			CheckDlgButton( hDlg, IDCB_POPHINT_VIEW, dValue ? BST_CHECKED : BST_UNCHECKED );
#endif
#ifdef SPMOZI_ENCODE

			dValue = InitParamValue( INIT_LOAD, VL_SPMOZI_ENC, 0 );
			CheckDlgButton( hDlg, IDCB_SPMOZI_ENCODE, dValue ? BST_CHECKED : BST_UNCHECKED );
#endif

			dValue = InitParamValue( INIT_LOAD, VL_LAYER_TRANS, 192 );
			SendDlgItemMessage( hDlg, IDSL_LAYERBOX_TRANCED, TBM_SETPOS, TRUE, (dValue - 0x1F) );

			dValue = InitParamValue( INIT_LOAD, VL_MAA_LCLICK, MAA_INSERT );
			switch( dValue )
			{
				default:
				case MAA_INSERT:	id =  IDRB_SEL_INS_EDIT;	break;
				case MAA_INTERRUPT:	id =  IDRB_SEL_INTRPT_EDIT;	break;
				case MAA_LAYERED:	id =  IDRB_SEL_SET_LAYER;	break;
				case MAA_UNICLIP:	id =  IDRB_SEL_CLIP_UNI;	break;
				case MAA_SJISCLIP:	id =  IDRB_SEL_CLIP_SJIS;	break;
				case MAA_DRAUGHT:	id =  IDRB_SEL_DRAUGHT;		break;
			}
			CheckRadioButton( hDlg, IDRB_SEL_INS_EDIT, IDRB_SEL_DRAUGHT, id );

			dValue = InitParamValue( INIT_LOAD, VL_MAA_MCLICK, MAA_INSERT );
			switch( dValue )
			{
				default:
				case MAA_INSERT:	id =  IDRB_SELSUB_INS_EDIT;		break;
				case MAA_INTERRUPT:	id =  IDRB_SELSUB_INTRPT_EDIT;	break;
				case MAA_LAYERED:	id =  IDRB_SELSUB_SET_LAYER;	break;
				case MAA_UNICLIP:	id =  IDRB_SELSUB_CLIP_UNI;		break;
				case MAA_SJISCLIP:	id =  IDRB_SELSUB_CLIP_SJIS;	break;
				case MAA_DRAUGHT:	id =  IDRB_SELSUB_DRAUGHT;		break;
			}
			CheckRadioButton( hDlg, IDRB_SELSUB_INS_EDIT, IDRB_SELSUB_DRAUGHT, id );

			switch( gdClickDrt )
			{
				case MAA_INSERT:	id =  IDRB_DRT_INS_EDIT;	break;
				case MAA_INTERRUPT:	id =  IDRB_DRT_INTRPT_EDIT;	break;
				case MAA_LAYERED:	id =  IDRB_DRT_SET_LAYER;	break;
				case MAA_UNICLIP:	id =  IDRB_DRT_CLIP_UNI;	break;
				case MAA_SJISCLIP:	id =  IDRB_DRT_CLIP_SJIS;	break;
			}
			CheckRadioButton( hDlg, IDRB_DRT_INS_EDIT, IDRB_DRT_CLIP_SJIS, id );

			switch( gdSubClickDrt )
			{
				case MAA_INSERT:	id =  IDRB_DRTSUB_INS_EDIT;		break;
				case MAA_INTERRUPT:	id =  IDRB_DRTSUB_INTRPT_EDIT;	break;
				case MAA_LAYERED:	id =  IDRB_DRTSUB_SET_LAYER;	break;
				case MAA_UNICLIP:	id =  IDRB_DRTSUB_CLIP_UNI;		break;
				case MAA_SJISCLIP:	id =  IDRB_DRTSUB_CLIP_SJIS;	break;
			}
			CheckRadioButton( hDlg, IDRB_DRTSUB_INS_EDIT, IDRB_DRTSUB_CLIP_SJIS, id );

			ZeroMemory( atPath, sizeof(atPath) );
			InitParamString( INIT_LOAD, VS_EXT_M2H_PATH, atPath );
			SetDlgItemText( hDlg, IDE_EXTERNAL_M2H_PATH, atPath );

			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDB_CHOOSEFONT:
					ViewingFontGet( &stLogFont );
					ZeroMemory( &stChooseFont, sizeof(CHOOSEFONT) );
					stChooseFont.lStructSize = sizeof(CHOOSEFONT);
					stChooseFont.hwndOwner = hDlg;
					stChooseFont.lpLogFont = &stLogFont;
					stChooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
					if( ChooseFont(&stChooseFont) )
					{
						StringCchCopy( ptFontName, LF_FACESIZE, stLogFont.lfFaceName );
					}
					break;

				case IDB_EXTERNAL_M2H_SEL:
					OptionExtMlt2HtmlPath( hDlg );
					break;

				case IDB_APPLY:
				case IDOK:

					dValue = FONTSZ_NORMAL;
					if( IsDlgButtonChecked( hDlg, IDRB_POPUP_REDUCE ) ){	dValue =  FONTSZ_REDUCE;	}
					InitParamValue( INIT_SAVE, VL_MAATIP_SIZE, dValue );
#ifdef USE_HOVERTIP
					HoverTipSizeChange( dValue );
#endif

					iBuff = IsDlgButtonChecked( hDlg, IDCB_POPUP_VISIBLE );
					AaItemsTipSizeChange( dValue, iBuff );
					InitParamValue( INIT_SAVE, VL_MAATIP_VIEW, iBuff );

					Edit_GetText( GetDlgItem(hDlg,IDE_RIGHTSLIDE_DOT), atBuff, SUB_STRING );
					dValue = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_RIGHT_SLIDE, dValue );

					Edit_GetText( GetDlgItem(hDlg,IDE_GRID_X_POS), atBuff, SUB_STRING );
					gdGridXpos = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_GRID_X_POS, gdGridXpos );
					if( cdGrXp != gdGridXpos )	ViewRedrawSetLine( -1 );

					Edit_GetText( GetDlgItem(hDlg,IDE_GRID_Y_POS), atBuff, SUB_STRING );
					gdGridYpos = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_GRID_Y_POS, gdGridYpos );
					if( cdGrYp != gdGridYpos )	ViewRedrawSetLine( -1 );

					Edit_GetText( GetDlgItem(hDlg,IDE_RIGHT_RULER_POS), atBuff, SUB_STRING );
					gdRightRuler = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_R_RULER_POS, gdRightRuler );
					if( cdRtRr != gdRightRuler )	ViewRedrawSetLine( -1 );

					Edit_GetText( GetDlgItem(hDlg,IDE_UNDER_RULER_POS), atBuff, SUB_STRING );
					gdUnderRuler = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_U_RULER_POS, gdUnderRuler );
					if( cdUdRr != gdUnderRuler )	ViewRedrawSetLine( -1 );

					Edit_GetText( GetDlgItem(hDlg,IDE_PAGE_BYTE_MAX), atBuff, SUB_STRING );
					gdPageByteMax = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_PAGEBYTE_MAX, gdPageByteMax );

					Edit_GetText( GetDlgItem(hDlg,IDE_AUTO_BU_INTVL), atBuff, SUB_STRING );
					gdBUInterval = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_BACKUP_INTVL, gdBUInterval );

					dValue = IsDlgButtonChecked( hDlg, IDCB_AUTOBU_MSG_ON );
					gbAutoBUmsg = dValue ? 1 : 0;
					InitParamValue( INIT_SAVE, VL_BACKUP_MSGON, gbAutoBUmsg );

					dValue = IsDlgButtonChecked( hDlg, IDCB_SAVE_MSG_ON );
					gbSaveMsgOn = dValue ? 1 : 0;
					InitParamValue( INIT_SAVE, VL_SAVE_MSGON, gbSaveMsgOn );

					gbCrLfCode = 0;
					if( IsDlgButtonChecked( hDlg , IDRB_CRLF_2CH_YY ) ){	gbCrLfCode =  1;	}
					InitParamValue( INIT_SAVE, VL_CRLF_CODE, gbCrLfCode );

					dValue = IsDlgButtonChecked( hDlg, IDCB_USE_UNISPACE_SET );
					gbUniPad = dValue ? 1 : 0;
					InitParamValue( INIT_SAVE, VL_USE_UNICODE, gbUniPad );

					dValue = IsDlgButtonChecked( hDlg, IDCB_UNIRADIX_HEX );
					gbUniRadixHex = dValue ? 1 : 0;
					InitParamValue( INIT_SAVE, VL_UNIRADIX_HEX, gbUniRadixHex );

					dValue = IsDlgButtonChecked( hDlg, IDCB_GROUPUNDO_SET );
					InitParamValue( INIT_SAVE, VL_GROUP_UNDO, dValue ? 1 : 0 );

					dValue = IsDlgButtonChecked( hDlg, IDCB_MULTIACT_ENA );
					InitParamValue( INIT_SAVE, VL_MULTI_ACT_E, dValue ? 1 : 0 );

					dValue = IsDlgButtonChecked( hDlg, IDCB_DOCKING_STYLE );
					InitParamValue( INIT_SAVE, VL_PLS_LN_DOCK, dValue ? 1 : 0 );

#ifdef TODAY_HINT_STYLE

					dValue = IsDlgButtonChecked( hDlg, IDCB_POPHINT_VIEW );
					InitParamValue( INIT_SAVE, VL_HINT_ENABLE, dValue );
#endif
#ifdef SPMOZI_ENCODE

					dValue = IsDlgButtonChecked( hDlg, IDCB_SPMOZI_ENCODE );
					gbSpMoziEnc = dValue ? 1 : 0;
					InitParamValue( INIT_SAVE, VL_SPMOZI_ENC, gbSpMoziEnc );
#endif

					if( IsDlgButtonChecked( hDlg, IDRB_LASTOPEN_NON ) ){	dValue = LASTOPEN_NON;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_LASTOPEN_ASK ) ){	dValue = LASTOPEN_ASK;	}
					else{	dValue = LASTOPEN_DO;	}
					InitParamValue( INIT_SAVE, VL_LAST_OPEN, dValue );

					dValue = SendDlgItemMessage( hDlg, IDSL_LAYERBOX_TRANCED, TBM_GETPOS, 0, 0 );
					dValue += 0x1F;
					InitParamValue( INIT_SAVE, VL_LAYER_TRANS, dValue );
					LayerBoxAlphaSet( dValue );

					if( IsDlgButtonChecked( hDlg, IDRB_SEL_INTRPT_EDIT )  ){	dValue = MAA_INTERRUPT;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_SET_LAYER ) ){	dValue = MAA_LAYERED;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_CLIP_UNI ) ){	dValue = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_CLIP_SJIS ) ){	dValue = MAA_SJISCLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_DRAUGHT ) ){	dValue = MAA_DRAUGHT;	}
					else{	dValue = MAA_INSERT;	}
					InitParamValue( INIT_SAVE, VL_MAA_LCLICK, dValue );

					if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_INTRPT_EDIT )   ){	iBuff = MAA_INTERRUPT;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_SET_LAYER ) ){	iBuff = MAA_LAYERED;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_CLIP_UNI ) ){	iBuff = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_CLIP_SJIS ) ){	iBuff = MAA_SJISCLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_DRAUGHT )  ){	iBuff = MAA_DRAUGHT;	}
					else{	iBuff =  MAA_INSERT;	}
					InitParamValue( INIT_SAVE, VL_MAA_MCLICK, iBuff );

					ViewMaaItemsModeSet( dValue, iBuff );

					if( IsDlgButtonChecked( hDlg, IDRB_DRT_INTRPT_EDIT )  ){	gdClickDrt = MAA_INTERRUPT;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRT_SET_LAYER ) ){	gdClickDrt = MAA_LAYERED;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRT_CLIP_UNI ) ){	gdClickDrt = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRT_CLIP_SJIS ) ){	gdClickDrt = MAA_SJISCLIP;	}
					else{	gdClickDrt = MAA_INSERT;	}
					InitParamValue( INIT_SAVE, VL_DRT_LCLICK, gdClickDrt );

					if( IsDlgButtonChecked( hDlg, IDRB_DRTSUB_INTRPT_EDIT )   ){	gdSubClickDrt = MAA_INTERRUPT;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRTSUB_SET_LAYER ) ){	gdSubClickDrt = MAA_LAYERED;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRTSUB_CLIP_UNI ) ){	gdSubClickDrt = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRTSUB_CLIP_SJIS ) ){	gdSubClickDrt = MAA_SJISCLIP;	}
					else{	gdSubClickDrt = MAA_INSERT;	}
					InitParamValue( INIT_SAVE, VL_DRT_MCLICK, gdSubClickDrt );

					GetDlgItemText( hDlg, IDE_EXTERNAL_M2H_PATH, atPath, MAX_PATH );
					InitParamString( INIT_SAVE, VS_EXT_M2H_PATH, atPath );

					if( IDOK == id ){	EndDialog( hDlg, IDOK );	}
					return (INT_PTR)TRUE;

				case IDCANCEL:
					EndDialog( hDlg, IDCANCEL );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		default:	break;
	}

	return (INT_PTR)FALSE;
}

UINT UnicodeUseToggle( LPVOID pVoid )
{
	gbUniPad =  !(gbUniPad);
	InitParamValue( INIT_SAVE, VL_USE_UNICODE, gbUniPad );
	MenuItemCheckOnOff( IDM_UNICODE_TOGGLE, gbUniPad );

	return gbUniPad;
}

BOOLEAN SelectDirectoryDlg( HWND hWnd, LPTSTR ptSelDir, UINT_PTR cchLen )
{
	BROWSEINFO		stBrowseInfo;
	LPITEMIDLIST	pstItemIDList;
	TCHAR	atDisplayName[MAX_PATH];

	if( !(ptSelDir) )	return FALSE;
	ZeroMemory( ptSelDir, sizeof(TCHAR) * cchLen );

	stBrowseInfo.hwndOwner		 = hWnd;
	stBrowseInfo.pidlRoot		 = NULL;
	stBrowseInfo.pszDisplayName	 = atDisplayName;
	stBrowseInfo.lpszTitle		 = TEXT("ＡＡの入ってるディレクトリを選択するのー！");
	stBrowseInfo.ulFlags		 = BIF_RETURNONLYFSDIRS;
	stBrowseInfo.lpfn			 = NULL;
	stBrowseInfo.lParam			 = (LPARAM)0;

	pstItemIDList = SHBrowseForFolder( &stBrowseInfo );
	if( !(pstItemIDList) )
	{

		return FALSE;
	}
	else
	{

		if( !SHGetPathFromIDList( pstItemIDList, atDisplayName ) )
		{

			return FALSE;
		}

		StringCchCopy( ptSelDir, cchLen, atDisplayName );

		CoTaskMemFree( pstItemIDList );
	}

	return TRUE;
}

LRESULT	CALLBACK gpfFileTabProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_MBUTTONUP, Ftb_OnMButtonUp );

		default:	break;
	}

	return CallWindowProc( gpfOriginFileTabProc, hWnd, msg, wParam, lParam );
}

VOID Ftb_OnMButtonUp( HWND hWnd, INT x, INT y, UINT flags )
{
	INT	curSel;
	TCHITTESTINFO	stTcHitInfo;

	stTcHitInfo.pt.x = x;
	stTcHitInfo.pt.y = y;
	curSel = TabCtrl_HitTest( ghFileTabWnd, &stTcHitInfo );

	TRACE( TEXT("FTAB start TAB [%d] [%d x %d]"), curSel, x, y );

	MultiFileTabClose( curSel );

	return;
}

HRESULT MultiFileTabFirst( LPTSTR ptName )
{
	TCHAR	atName[MAX_PATH];
	TCITEM		stTcItem;

	StringCchCopy( atName, MAX_PATH, ptName );
	PathStripPath( atName );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;
	stTcItem.pszText = atName;
	stTcItem.lParam  = 1;
	TabCtrl_InsertItem( ghFileTabWnd, 1, &stTcItem );

	TabCtrl_DeleteItem( ghFileTabWnd, 0 );

	TabCtrl_SetCurSel( ghFileTabWnd, 0 );

#ifdef BIG_TEXT_SEPARATE
	if( gbSeparateMod )
	{
		DocModifyContent( 1 );
		gbSeparateMod = FALSE;
	}

#endif

	return S_OK;
}

HRESULT MultiFileTabAppend( LPARAM dNumber, LPTSTR ptName )
{
	INT		iCount;
	TCHAR	atName[MAX_PATH];
	TCITEM		stTcItem;

	StringCchCopy( atName, MAX_PATH, ptName );
	PathStripPath( atName );

	iCount = TabCtrl_GetItemCount( ghFileTabWnd );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;
	stTcItem.pszText = atName;
	stTcItem.lParam  = dNumber;
	TabCtrl_InsertItem( ghFileTabWnd, iCount, &stTcItem );

	TabCtrl_SetCurSel( ghFileTabWnd, iCount );

#ifdef BIG_TEXT_SEPARATE
	if( gbSeparateMod )
	{
		DocModifyContent( 1 );
		gbSeparateMod = FALSE;
	}

#endif

	return S_OK;
}

INT MultiFileTabSearch( LPARAM dNumber )
{
	INT		iCount, i;
	TCITEM	stTcItem;

	iCount = TabCtrl_GetItemCount( ghFileTabWnd );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_PARAM;

	for( i = 0; iCount > i; i++ )
	{
		TabCtrl_GetItem( ghFileTabWnd, i, &stTcItem );

		if( dNumber == stTcItem.lParam )	return i;
	}

	return -1;
}

HRESULT MultiFileTabSelect( LPARAM dNumber )
{
	INT	iRslt;

	iRslt = MultiFileTabSearch( dNumber );

	if( 0 <= iRslt )
	{
		TabCtrl_SetCurSel( ghFileTabWnd, iRslt );
		return S_OK;
	}

	return E_OUTOFMEMORY;
}

HRESULT MultiFileTabSlide( INT xDir )
{
	INT		curSel, iCount, iTarget;
	LPARAM	dSele;
	TCITEM	stTcItem;

	if( 0 == xDir ){	return  S_FALSE;	}

	iCount = TabCtrl_GetItemCount( ghFileTabWnd );

	curSel = TabCtrl_GetCurSel( ghFileTabWnd );

	if( 0 < xDir )
	{
		iTarget = curSel + 1;
		if( iCount <= iTarget ){	iTarget = 0;	}
	}
	else
	{
		iTarget = curSel - 1;
		if( 0 > iTarget ){	iTarget = iCount - 1;	}
	}

	TabCtrl_SetCurSel( ghFileTabWnd, iTarget );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_PARAM;
	TabCtrl_GetItem( ghFileTabWnd, iTarget, &stTcItem );
	dSele = stTcItem.lParam;

	DocMultiFileSelect( dSele );

	return S_OK;
}

HRESULT MultiFileTabRename( LPARAM dNumber, LPTSTR ptName )
{
	INT	iRslt;
	TCHAR	atName[MAX_PATH];
	TCITEM		stTcItem;

	iRslt = MultiFileTabSearch( dNumber );
	if( 0 > iRslt ){	return E_OUTOFMEMORY;	}

	StringCchCopy( atName, MAX_PATH, ptName );
	PathStripPath( atName );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = atName;
	TabCtrl_SetItem( ghFileTabWnd, iRslt, &stTcItem );

	return S_OK;
}

INT InitMultiFileTabOpen( UINT dMode, INT iTgt, LPTSTR ptFile )
{
	if( dMode )
	{
		return DocMultiFileFetch( iTgt, ptFile, gatIniPath );
	}
	else
	{
		DocMultiFileStore( gatIniPath );
	}

	return 0;
}

HRESULT MultiFileTabClose( INT iSelTab )
{
	INT	curSel, nowSel;
	LPARAM	dSele;
	TCITEM	stTcItem;

	nowSel = TabCtrl_GetCurSel( ghFileTabWnd );

	if( 0 > iSelTab ){	curSel = nowSel;	}
	else{				curSel = iSelTab;	}

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_PARAM;
	TabCtrl_GetItem( ghFileTabWnd, curSel, &stTcItem );

	dSele = DocMultiFileClose( ghMainWnd, stTcItem.lParam );
	if( dSele )
	{
		TabCtrl_DeleteItem( ghFileTabWnd, curSel );

		if( curSel ==  nowSel ){	MultiFileTabSelect( dSele );	}
	}

	return S_OK;
}

HRESULT ViewingFontNameLoad( VOID )
{
	TCHAR	atName[LF_FACESIZE];

	ZeroMemory( atName, sizeof(atName) );
	StringCchCopy( atName, LF_FACESIZE, TEXT("ＭＳ Ｐゴシック") );

	InitParamString( INIT_LOAD, VS_FONT_NAME, atName );

	StringCchCopy( gstBaseFont.lfFaceName, LF_FACESIZE, atName );

	return S_OK;
}

HRESULT ViewingFontGet( LPLOGFONT pstLogFont )
{
	ZeroMemory( pstLogFont, sizeof(LOGFONT) );

	*pstLogFont = gstBaseFont;

	return S_OK;
}

#ifdef BIG_TEXT_SEPARATE

UINT DocHugeFileTreatment( UINT dStyle )
{
	 INT	iRslt = IDNO;
	UINT	dMode;
	BOOL	bVisible;

	bVisible = IsWindowVisible( ghMainWnd );

	if( 1 == dStyle )
	{
		iRslt = MessageBox( bVisible ? ghMainWnd : NULL, TEXT("大きなファイルを開こうとしてるよ。適当に分割して読み込むかい？\r\n\r\n　　はい：分割読込（すぐ保存してね）\t\n　　いいえ：そのまま読込（時間かかるかも）\r\n　　キャンセル：読込中止"), TEXT("一枚板のテキストっぽいよ"), MB_YESNOCANCEL | MB_ICONQUESTION );

		switch( iRslt )
		{
			case IDYES:	dMode = 1;	gbSeparateMod = TRUE;	break;
			case IDNO:	dMode = 0;	break;
			default:	dMode = 2;	break;
		}
	}
	else if( 2 == dStyle )
	{
		MessageBox( bVisible ? ghMainWnd : NULL, TEXT("ファイルの中身がなんかおかしいよ。読込を中止するよ。"), TEXT("改行がないかも"), MB_OK | MB_ICONERROR );
		dMode = 2;
	}
	else
	{
		dMode = 2;
	}

	return dMode;
}

#endif

#ifdef USE_NOTIFYICON

VOID TaskTrayIconEvent( HWND hWnd, UINT uID, UINT message )
{

	return;
}

#endif

HRESULT NotifyBalloonExist( LPTSTR ptInfo, LPTSTR ptTitle, DWORD dIconTy )
{
#ifdef USE_NOTIFYICON
	NOTIFYICONDATA	nid;

	ZeroMemory( &nid, sizeof(NOTIFYICONDATA) );
	nid.cbSize      = sizeof(NOTIFYICONDATA);
	nid.uFlags      = NIF_INFO;
	nid.hWnd        = ghMainWnd;
	StringCchCopy( nid.szInfoTitle, 64, ptTitle );
	StringCchCopy( nid.szInfo, 256, ptInfo );
	nid.uTimeout    = 15000;
	nid.dwInfoFlags = dIconTy;
	Shell_NotifyIcon( NIM_MODIFY, &nid );

#endif
	return S_OK;
}

HRESULT DockingTmplViewToggle( UINT bMode )
{
	HWND	hDockWnd;
	RECT	clntRect, tabRect, pageRect, sbRect;
	RECT	rect;
	INT		iHeight, curSel;

	hDockWnd = DockingTabGet( );

	if( !(hDockWnd) )	return E_ABORT;

	GetClientRect( ghMainWnd, &clntRect );
	GetClientRect( hDockWnd,  &tabRect );
	GetClientRect( ghPgVwWnd, &pageRect );

	GetClientRect( ghStsBarWnd, &sbRect );

	AppClientAreaCalc( &rect );

	if( gbDockTmplView )
	{
		gbDockTmplView = FALSE;
		ShowWindow( ghLnTmplWnd, SW_HIDE );
		ShowWindow( ghBrTmplWnd, SW_HIDE );

		iHeight = clntRect.bottom - tabRect.bottom - sbRect.bottom;

		SetWindowPos( hDockWnd, HWND_TOP, rect.right - tabRect.right, iHeight, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

		SetWindowPos( ghPgVwWnd, HWND_TOP, 0, 0, pageRect.right, iHeight - rect.top, SWP_NOMOVE | SWP_NOZORDER );
	}
	else
	{
		gbDockTmplView = TRUE;

		ViewSizeMove( ghMainWnd, &rect );

		curSel = TabCtrl_GetCurSel( hDockWnd );
		switch( curSel )
		{
			case  0:	ShowWindow( ghLnTmplWnd , SW_SHOW );	break;
			case  1:	ShowWindow( ghBrTmplWnd , SW_SHOW );	break;
			default:	break;
		}

	}

	return S_OK;
}

#if defined(_DEBUG) || defined(WORK_LOG_OUT)
VOID OutputDebugStringPlus( DWORD rixError, LPTSTR ptFile, INT rdLine, LPCSTR ptFunc, LPTSTR ptFormat, ... )

{
	va_list	argp;
	TCHAR	atBuf[MAX_PATH], atOut[MAX_PATH], atFiFu[MAX_PATH], atErrMsg[MAX_PATH];

#ifdef WORK_LOG_OUT
	UINT_PTR	cchLen;
	DWORD	wrote;
#endif

	StringCchCopy( atFiFu, MAX_PATH, ptFile );
	PathStripPath( atFiFu );

	va_start(argp, ptFormat);
	StringCchVPrintf( atBuf, MAX_PATH, ptFormat, argp );
	va_end( argp );

	StringCchPrintf( atOut, MAX_PATH, TEXT("%s @ %s %d %s\r\n"), atBuf, atFiFu, rdLine, ptFunc );

#ifdef _DEBUG
	OutputDebugString( atOut );
#endif

#ifdef WORK_LOG_OUT
	if( INVALID_HANDLE_VALUE != ghLogFlie )
	{
		StringCchLength( atOut, MAX_PATH, &cchLen );
		WriteFile( ghLogFlie, atOut, cchLen * sizeof(TCHAR), &wrote, NULL );
	}
#endif

	if( rixError )
	{
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, rixError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), atErrMsg, MAX_PATH, NULL );

		StringCchPrintf( atBuf, MAX_PATH, TEXT("[%d]%s"), rixError, atErrMsg );

#ifdef _DEBUG
		OutputDebugString( atBuf );
#endif
#ifdef WORK_LOG_OUT
		if( INVALID_HANDLE_VALUE != ghLogFlie )
		{
			StringCchLength( atBuf, MAX_PATH, &cchLen );
			WriteFile( ghLogFlie, atBuf, cchLen * sizeof(TCHAR), &wrote, NULL );
		}
#endif

		SetLastError( 0 );
	}
}

#endif
