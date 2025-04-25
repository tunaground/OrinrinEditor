#include "stdafx.h"
#include "OrinrinCollector.h"

static HANDLE		ghMutex;

static HINSTANCE	ghInst;
static TCHAR		gatTitle[MAX_STRING];
static TCHAR		gatWindowClass[MAX_STRING];

static  HWND		ghToolTipWnd;
static HBRUSH		ghBrush;
static HICON		ghIcon;
static  UINT		grdTaskbarResetID;
static  HWND		ghNextViewer;
static BOOLEAN		gbClipSteal;
static BOOLEAN		gGetMsgOn;
static BOOLEAN		gIsAST;
static TCHAR		gatIniPath[MAX_PATH];
static TCHAR		gatClipFile[MAX_PATH];
static NOTIFYICONDATA	gstNtfyIcon;

EXTERNED UINT		gbUniRadixHex;

static  UINT		gbHotMod;
static  UINT		gbHotVkey;

INT APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG	msg;
	INT	msRslt;

#ifdef _DEBUG

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	ghMutex = CreateMutex( NULL, TRUE, TEXT("OrinrinCollector") );
	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		MessageBox( NULL, TEXT("已にアプリは起動してるよ！"), TEXT("お燐からのお知らせ"), MB_OK|MB_ICONINFORMATION );
		ReleaseMutex( ghMutex );
		CloseHandle( ghMutex );
		return 0;
	}

	INITCOMMONCONTROLSEX	iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC  = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	InitCommonControlsEx( &iccex );

	LoadString( hInstance, IDS_APP_TITLE, gatTitle, MAX_STRING );
	LoadString( hInstance, IDC_ORINRINCOLLECTOR, gatWindowClass, MAX_STRING );
	InitWndwClass( hInstance );

	if( !InitInstance( hInstance, nCmdShow ) )
	{
		return FALSE;
	}

	for(;;)
	{
		msRslt = GetMessage( &msg, NULL, 0, 0 );
		if( 1 != msRslt )	break;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

ATOM InitWndwClass( HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;

	ghIcon = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_ORINRINCOLLECTOR) );
	ghBrush = CreateSolidBrush( BASIC_COLOUR );

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= ghIcon;
	wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= ghBrush;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= gatWindowClass;
	wcex.hIconSm		= ghIcon;

	return RegisterClassEx( &wcex );
}

BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	HWND	hWnd;
	INT		xxx, yyy;
	RECT	rect;

	BOOL	bHotRslt;

	ghInst = hInstance;

	GetModuleFileName( hInstance, gatIniPath, MAX_PATH );
	PathRemoveFileSpec( gatIniPath );
	PathAppend( gatIniPath, INI_FILE );

	gbClipSteal   = InitParamValue( INIT_LOAD, VL_COLLECT_AON,  0 );
	gGetMsgOn     = InitParamValue( INIT_LOAD, VL_USE_BALLOON,  1 );
	gbUniRadixHex = InitParamValue( INIT_LOAD, VL_UNIRADIX_HEX, 0 );

	gbHotMod      = InitParamValue( INIT_LOAD, VL_COLHOT_MODY, (MOD_CONTROL | MOD_SHIFT) );
	gbHotVkey     = InitParamValue( INIT_LOAD, VL_COLHOT_VKEY, VK_C );

	hWnd = GetDesktopWindow(  );
	GetWindowRect( hWnd, &rect );
	xxx = ( rect.right  - WCL_WIDTH ) / 2;
	yyy = ( rect.bottom - WCL_HEIGHT ) / 2;

	hWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_APPWINDOW, gatWindowClass, gatTitle, WS_CAPTION | WS_POPUPWINDOW, xxx, yyy, WCL_WIDTH, WCL_HEIGHT, NULL, NULL, hInstance, NULL);

	if( !hWnd ){	return FALSE;	}

	grdTaskbarResetID = RegisterWindowMessage( TEXT("TaskbarCreated") );

	ghNextViewer = SetClipboardViewer( hWnd );

	FileListViewInit( hWnd );
	FileListViewGet( hWnd, 0, gatClipFile );
	FileTypeCheck( gatClipFile );

	if( gGetMsgOn ){		Button_SetCheck( GetDlgItem(hWnd,IDB_CLIP_USE_BALLOON)  , BST_CHECKED );	}
	if( gbUniRadixHex ){	Button_SetCheck( GetDlgItem(hWnd,IDB_CLIP_UNIRADIX_HEX) , BST_CHECKED );	}
	if( gbClipSteal ){		Button_SetCheck( GetDlgItem(hWnd,IDB_CLIP_STEAL_ACT_ON) , BST_CHECKED );	}

	TasktrayIconAdd( hWnd );

	bHotRslt = RegisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE, gbHotMod, gbHotVkey );

	ShowWindow( hWnd, SW_HIDE );

	return TRUE;
}

UINT RegHotModExchange( UINT bSrc, BOOLEAN bDrct )
{
	BYTE	bDest = 0;

	if( bDrct  )
	{
		if( bSrc & MOD_SHIFT )		bDest |= HOTKEYF_SHIFT;
		if( bSrc & MOD_CONTROL )	bDest |= HOTKEYF_CONTROL;
		if( bSrc & MOD_ALT )		bDest |= HOTKEYF_ALT;
	}
	else
	{
		if( bSrc & HOTKEYF_SHIFT )		bDest |= MOD_SHIFT;
		if( bSrc & HOTKEYF_CONTROL )	bDest |= MOD_CONTROL;
		if( bSrc & HOTKEYF_ALT )		bDest |= MOD_ALT;
	}

	return bDest;
}

HRESULT ToolTipSetting( HWND hWnd, UINT itemID, LPTSTR ptText )
{
	TTTOOLINFO	stToolInfo;

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );

	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;
	stToolInfo.hwnd     = GetDlgItem( hWnd, itemID );
	stToolInfo.uId      = itemID;
	GetClientRect( stToolInfo.hwnd, &stToolInfo.rect );
	stToolInfo.lpszText = ptText;

	SendMessage( ghToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghToolTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );

	return S_OK;
}

HRESULT FileListViewInit( HWND hWnd )
{
	UINT	dItems, d;
	TCHAR	atFilePath[MAX_PATH];
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );

	ListView_DeleteAllItems( hLvWnd );

	dItems = InitParamValue( INIT_LOAD, VL_CLIPFILECNT, 1 );
	for( d = 0; dItems > d; d++ )
	{
		InitClipStealOpen( INIT_LOAD, d, atFilePath );
		FileListViewAdd( hWnd, atFilePath );
	}

	return S_OK;
}

INT FileListViewAdd( HWND hWnd, LPTSTR ptPath )
{
	INT	iCount;
	TCHAR	atName[MAX_PATH];
	LVITEM	stLvi;
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );

	iCount = ListView_GetItemCount( hLvWnd );

	if( !(ptPath) ){	return iCount;	}
	if( NULL == ptPath[0] ){	return iCount;	}

	StringCchCopy( atName, MAX_PATH, ptPath );
	PathStripPath( atName );

	ZeroMemory( &stLvi, sizeof(LVITEM) );
	stLvi.mask     = LVIF_TEXT;
	stLvi.iItem    = iCount;

	stLvi.iSubItem = 0;
	stLvi.pszText  = atName;
	ListView_InsertItem( hLvWnd, &stLvi );

	stLvi.iSubItem = 1;
	stLvi.pszText  = ptPath;
	ListView_SetItem( hLvWnd, &stLvi );

	iCount = ListView_GetItemCount( hLvWnd );

	return iCount;
}

HRESULT FileListViewGet( HWND hWnd, INT iNumber, LPTSTR ptFile )
{
	LVITEM	stLvi;
	TCHAR	atBuff[MAX_PATH];
	INT		iCount;
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );

	iCount = ListView_GetItemCount( hLvWnd );
	if( iCount <= iNumber ){	return E_OUTOFMEMORY;	}

	ZeroMemory( &stLvi, sizeof(LVITEM) );
	stLvi.mask       = LVIF_TEXT;
	stLvi.iItem      = iNumber;
	stLvi.iSubItem   = 1;
	stLvi.pszText    = atBuff;
	stLvi.cchTextMax = MAX_PATH;
	ListView_GetItem( hLvWnd, &stLvi );

	StringCchCopy( ptFile, MAX_PATH, atBuff );

	return S_OK;
}

INT FileListViewDelete( HWND hWnd )
{
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );
	INT		iItem, iCount;
	TCHAR	atPath[MAX_PATH];

	iItem = ListView_GetNextItem( hLvWnd, -1, LVNI_ALL | LVNI_SELECTED );

	if( 0 <= iItem )
	{

		FileListViewGet( hWnd, iItem, atPath );
		if( 0 == StrCmp( gatClipFile, atPath ) )
		{
			MessageBox( hWnd, TEXT("そのファイルは使用中だよ。\r\n削除できないよ。"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
		}
		else
		{
			ListView_DeleteItem( hLvWnd, iItem );
		}
	}

	iCount = ListView_GetItemCount( hLvWnd );

	return iCount;
}

HRESULT InitSettingSave( HWND hWnd, UINT bActOn )
{
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );
	INT		iCount, i;
	TCHAR	atBuff[MAX_PATH];

	iCount = ListView_GetItemCount( hLvWnd );

	ZeroMemory( atBuff, sizeof(atBuff) );
	WritePrivateProfileSection( TEXT("Collector"), atBuff, gatIniPath );

	InitParamValue( INIT_SAVE, VL_USE_BALLOON,  gGetMsgOn );
	InitParamValue( INIT_SAVE, VL_UNIRADIX_HEX, gbUniRadixHex );
	InitParamValue( INIT_SAVE, VL_CLIPFILECNT,  iCount );
	InitParamValue( INIT_SAVE, VL_COLLECT_AON,  bActOn );
	InitParamValue( INIT_SAVE, VL_COLHOT_MODY,  gbHotMod );
	InitParamValue( INIT_SAVE, VL_COLHOT_VKEY,  gbHotVkey );

	for( i = 0; iCount > i; i++ )
	{
		FileListViewGet( hWnd, i, atBuff );
		InitClipStealOpen( INIT_SAVE, i, atBuff );
	}

	return S_OK;
}

HRESULT InitClipStealOpen( UINT dMode, UINT dNumber, LPTSTR ptFile )
{
	TCHAR	atKey[MIN_STRING];

	if(  !(ptFile) )	return 0;

	ZeroMemory( atKey, sizeof(atKey) );
	if( 0 == dNumber )	StringCchCopy( atKey, MIN_STRING, TEXT("CopySaveFile") );
	else	StringCchPrintf( atKey, MIN_STRING, TEXT("CopySaveFile%d"), dNumber );

	if( dMode ){	GetPrivateProfileString( TEXT("Collector"), atKey, TEXT(""), ptFile, MAX_PATH, gatIniPath );	}
	else{			WritePrivateProfileString( TEXT("Collector"), atKey, ptFile, gatIniPath );	}

	return S_OK;
}

BOOLEAN FileTypeCheck( LPTSTR ptFile )
{
	LPTSTR		ptExten;
	TCHAR		atExBuf[10];

	ptExten = PathFindExtension( ptFile );
	StringCchCopy( atExBuf, 10, ptExten );
	CharLower( atExBuf );

	if( !( StrCmp( atExBuf , TEXT(".ast") ) ) ){	gIsAST = TRUE;	}
	else	gIsAST =  FALSE;

	return gIsAST;
}

INT InitParamValue( UINT dMode, UINT dStyle, INT nValue )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	INT	dBuff = 0;

	switch( dStyle )
	{
		case VL_UNIRADIX_HEX:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UniRadixHex") );	break;
		case VL_USE_BALLOON:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UseBalloon")  );	break;
		case VL_CLIPFILECNT:	StringCchCopy( atKeyName, SUB_STRING, TEXT("FileCount") );		break;
		case VL_COLLECT_AON:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CollectActOn") );	break;
		case VL_COLHOT_MODY:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CollectHotMod") );	break;
		case VL_COLHOT_VKEY:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CollectHotVkey") );	break;

		default:	return nValue;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );

	if( dMode )
	{
		GetPrivateProfileString( TEXT("Collector"), atKeyName, atBuff, atBuff, MIN_STRING, gatIniPath );
		dBuff = StrToInt( atBuff );
	}
	else
	{
		WritePrivateProfileString( TEXT("Collector"), atKeyName, atBuff, gatIniPath );
	}

	return dBuff;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_CREATE,         Cls_OnCreate );
		HANDLE_MSG( hWnd, WM_PAINT,          Cls_OnPaint  );
		HANDLE_MSG( hWnd, WM_COMMAND,        Cls_OnCommand );
		HANDLE_MSG( hWnd, WM_DESTROY,        Cls_OnDestroy );
		HANDLE_MSG( hWnd, WM_HOTKEY,         Cls_OnHotKey );
		HANDLE_MSG( hWnd, WM_CTLCOLORSTATIC, Cls_OnCtlColor );
		HANDLE_MSG( hWnd, WM_DRAWCLIPBOARD,  Cls_OnDrawClipboard );
		HANDLE_MSG( hWnd, WM_CHANGECBCHAIN,  Cls_OnChangeCBChain );

		case WM_CLOSE:
			RegisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE, gbHotMod, gbHotVkey );
			ShowWindow( hWnd, SW_HIDE );
			return 0;

		case WMP_TRAYNOTIFYICON:
			TaskTrayIconEvent( hWnd, (UINT)wParam, (UINT)lParam );
			return 0;

		default:
			if( grdTaskbarResetID == message )
			{
				TasktrayIconAdd( hWnd );
				return 0;
			}
			break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

BOOLEAN Cls_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	HINSTANCE	lcInst = lpCreateStruct->hInstance;
	HWND	hWorkWnd;
	RECT	rect;
	LVCOLUMN	stLvColm;

	UINT	bCtrlMod;

	GetClientRect( hWnd, &rect );

	ghToolTipWnd = CreateWindowEx( 0, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, lcInst, NULL );

	hWorkWnd = CreateWindowEx( 0, WC_STATIC, TEXT(""), WS_CHILD | WS_VISIBLE | SS_ICON, 8, 8, 32, 32, hWnd, (HMENU)IDC_MYICON, lcInst, NULL );
	SendMessage( hWorkWnd, STM_SETICON, (WPARAM)ghIcon, 0 );

	CreateWindowEx( 0, WC_STATIC, TEXT("OrinrinCollector, Version 1.2 (2012.510.2200.920)"), WS_CHILD | WS_VISIBLE, 44, 8, 370, 23, hWnd, (HMENU)IDC_STATIC, lcInst, NULL );

	CreateWindowEx( 0, WC_STATIC, TEXT("頂戴したクリップ内容を保存するファイル名"), WS_CHILD | WS_VISIBLE, 8, 48, 370, 23, hWnd, (HMENU)IDC_STATIC, lcInst, NULL );

	CreateWindowEx( 0, WC_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 8, 70, rect.right-16-160, 23, hWnd, (HMENU)IDE_CLIPSTEAL_FILE, lcInst, NULL );
	ToolTipSetting( hWnd, IDE_CLIPSTEAL_FILE, TEXT("ファイル名を入力してね。MLTとASTが使えるよ。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("参照"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect.right-8-160, 70, 50, 23, hWnd, (HMENU)IDB_CLIPSTEAL_REF, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIPSTEAL_REF, TEXT("ファイル選択ダイヤログを開いて、ファイルを指定できるよ。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("追加"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect.right-8-100, 70, 50, 23, hWnd, (HMENU)IDB_CLIPSTEAL_FILEADD, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIPSTEAL_FILEADD, TEXT("指定したファイルを、使用リストに追加するよ。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("削除"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect.right-8-50, 70, 50, 23, hWnd, (HMENU)IDB_CLIPSTEAL_FILEDEL, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIPSTEAL_FILEDEL, TEXT("使用リストで選択したファイルを、リストから削除するよ。\r\nファイル自体を削除するわけじゃないよ。") );

	hWorkWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("filelv"), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL, 8, 95, rect.right-16, 105, hWnd, (HMENU)IDLV_CLIPSTEAL_FILELISTVW, lcInst, NULL );
	ListView_SetExtendedListViewStyle( hWorkWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	ToolTipSetting( hWnd, IDLV_CLIPSTEAL_FILELISTVW, TEXT("使用するファイルの一覧だよ。") );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt = LVCFMT_LEFT;
	stLvColm.pszText = TEXT("ファイル名");	stLvColm.cx = 120;	stLvColm.iSubItem = 0;	ListView_InsertColumn( hWorkWnd, 0, &stLvColm );
	stLvColm.pszText = TEXT("フルパス");	stLvColm.cx = 300;	stLvColm.iSubItem = 1;	ListView_InsertColumn( hWorkWnd, 1, &stLvColm );

	CreateWindowEx( 0, WC_BUTTON, TEXT("保存したらバルーンメッセージを表示する"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 207, rect.right-16, 23, hWnd, (HMENU)IDB_CLIP_USE_BALLOON, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIP_USE_BALLOON, TEXT("ファイルに取り込んだら、バルーンメッセージでお知らせするよ。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("ユニコード数値参照は１６進数型（非チェックで１０進数）"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 234, rect.right-16, 23, hWnd, (HMENU)IDB_CLIP_UNIRADIX_HEX, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIP_UNIRADIX_HEX, TEXT("頂戴した内容にユニコードが含まれていたら、どういう形式で保存するのかを決めてね。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("起動したときに、コピペ保存をＯＮにする"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 261, rect.right-16, 23, hWnd, (HMENU)IDB_CLIP_STEAL_ACT_ON, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIP_STEAL_ACT_ON, TEXT("起動したときに、コピペ保存機能をＯＮにしておくよ。") );

	CreateWindowEx( 0, WC_STATIC, TEXT("メニューポップアップホットキー"), WS_CHILD | WS_VISIBLE, 8, 290, 260, 23, hWnd, (HMENU)IDC_STATIC, lcInst, NULL );
	hWorkWnd = CreateWindowEx( 0, HOTKEY_CLASS, TEXT(""), WS_CHILD | WS_VISIBLE, 270, 290, 200, 23, hWnd, (HMENU)IDHK_CLIP_POPUP_KEYBIND, lcInst, NULL );
	ToolTipSetting( hWnd, IDHK_CLIP_POPUP_KEYBIND, TEXT("ここをクリックして、キーの組み合わせを設定してね。") );

	bCtrlMod = RegHotModExchange( gbHotMod, 1 );
	SendMessage( hWorkWnd , HKM_SETHOTKEY, MAKEWORD(gbHotVkey, bCtrlMod), 0 );

	CreateWindowEx( 0, WC_BUTTON, TEXT("保存して閉じる"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect.right-8-150, rect.bottom-30, 150, 23, hWnd, (HMENU)IDB_CLIP_SAVE_AND_EXIT, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIP_SAVE_AND_EXIT, TEXT("変更内容を保存して、この窓を閉じるよ。") );

	return TRUE;
}

VOID Cls_OnCommand( HWND hWnd, INT id, HWND hwndCtl, UINT codeNotify )
{
	TCHAR	atPath[MAX_PATH], atBuff[MAX_PATH];
	INT		iRslt, iTgt;

	UINT	bActOn;
	BOOL	bHotRslt;

	LRESULT	lRslt;
	UINT	bMod, bVkey;

	switch( id )
	{
		case IDM_EXIT:	DestroyWindow( hWnd );	break;

		case IDM_CLIPSTEAL_OPTION:
			ShowWindow( hWnd, SW_SHOW );
			UnregisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE );
			break;

		case IDM_CLIPSTEAL_TOGGLE:
			gbClipSteal = !(gbClipSteal);
			TaskTrayIconCaptionChange( hWnd );
			break;

		case IDB_CLIPSTEAL_REF:
			if( SelectFileDlg( hWnd, atPath, MAX_PATH ) )
			{
				Edit_SetText( GetDlgItem(hWnd,IDE_CLIPSTEAL_FILE), atPath );
			}
			break;

		case IDB_CLIPSTEAL_FILEADD:
			Edit_GetText( GetDlgItem(hWnd,IDE_CLIPSTEAL_FILE), atPath, MAX_PATH );
			iRslt = FileListViewAdd( hWnd, atPath );
			if( 1 == iRslt )
			{
				StringCchCopy( gatClipFile, MAX_PATH, atPath );
				FileTypeCheck( gatClipFile );
			}
			break;

		case IDB_CLIPSTEAL_FILEDEL:
			FileListViewDelete( hWnd );
			break;

		case IDB_CLIP_SAVE_AND_EXIT:

			lRslt = SendDlgItemMessage( hWnd, IDHK_CLIP_POPUP_KEYBIND, HKM_GETHOTKEY, 0, 0 );
			bVkey = LOBYTE( lRslt );
			bMod  = RegHotModExchange( HIBYTE( lRslt ), 0 );

			bHotRslt = RegisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE, bMod, bVkey );
			if( bHotRslt )
			{
				gbHotVkey = bVkey;
				gbHotMod  = bMod;
			}
			else
			{
				MessageBox( hWnd, TEXT("ホットキーが登録出来なかったよ。\r\n已に使われてるみたい。\r\n他の組み合わせを試してみて！"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
				break;
			}

			if( BST_CHECKED == IsDlgButtonChecked( hWnd, IDB_CLIP_USE_BALLOON ) ){	gGetMsgOn = TRUE;	}
			else{	gGetMsgOn = FALSE;	}

			if( BST_CHECKED == IsDlgButtonChecked( hWnd, IDB_CLIP_UNIRADIX_HEX ) ){	gbUniRadixHex = TRUE;	}
			else{	gbUniRadixHex = FALSE;	}

			bActOn = IsDlgButtonChecked( hWnd, IDB_CLIP_STEAL_ACT_ON );

			InitSettingSave( hWnd, bActOn );

			ShowWindow( hWnd, SW_HIDE );
			break;

		default:	break;
	}

	if( IDM_CLIPSTEAL_SELECT <= id )
	{
		iTgt = id - IDM_CLIPSTEAL_SELECT;
		FileListViewGet( hWnd, iTgt, atPath );
		StringCchCopy( gatClipFile, MAX_PATH, atPath );
		FileTypeCheck( gatClipFile );

		PathStripPath( atPath );
		StringCchPrintf( atBuff, MAX_PATH, TEXT("%s に保存していくよ"), atPath );
		TaskTrayIconBalloon( hWnd, atBuff, TEXT("保存先を変更したよ"), NIIF_INFO );
	}

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

VOID Cls_OnDestroy( HWND hWnd )
{
	UnregisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE );

	ChangeClipboardChain( hWnd , ghNextViewer );

	gstNtfyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	Shell_NotifyIcon( NIM_DELETE, &gstNtfyIcon );

	DeleteBrush( ghBrush );

	PostQuitMessage( 0 );

	return;
}

HBRUSH Cls_OnCtlColor( HWND hWnd, HDC hdc, HWND hWndChild, INT type )
{
	SetBkColor( hdc, BASIC_COLOUR );

	return ghBrush;
}

VOID Cls_OnHotKey( HWND hWnd, INT idHotKey, UINT fuModifiers, UINT vk )
{
	POINT	stPoint;
	HMENU	hMenu;

	if( IDHK_CLIPSTEAL_FILECHANGE == idHotKey )
	{
		GetCursorPos( &stPoint );

		SetForegroundWindow( hWnd );

		hMenu = CreateFileSelMenu( hWnd, 1 );
		TrackPopupMenu( hMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
	}

	return;
}

BOOLEAN SelectFileDlg( HWND hWnd, LPTSTR ptSelFile, UINT_PTR cchLen )
{
	OPENFILENAME	stOpenFile;
	BOOLEAN	bOpened;
	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];

	if( !(ptSelFile) )	return FALSE;

	ZeroMemory( &stOpenFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );

	stOpenFile.lStructSize     = sizeof(OPENFILENAME);
	stOpenFile.hwndOwner       = hWnd;
	stOpenFile.lpstrFilter     = TEXT("アスキーアートファイル ( mlt, ast )\0*.mlt;*.ast\0全ての形式(*.*)\0*.*\0\0");
	stOpenFile.nFilterIndex    = 1;
	stOpenFile.lpstrFile       = atFilePath;
	stOpenFile.nMaxFile        = MAX_PATH;
	stOpenFile.lpstrFileTitle  = atFileName;
	stOpenFile.nMaxFileTitle   = MAX_STRING;

	stOpenFile.lpstrTitle      = TEXT("ため込むファイルを指定するのです");
	stOpenFile.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
	stOpenFile.lpstrDefExt     = TEXT("mlt");

	bOpened = GetOpenFileName( &stOpenFile );

	if( !(bOpened) ){	return FALSE;	}

	StringCchCopy( ptSelFile, cchLen, atFilePath );

	return TRUE;
}

HRESULT TasktrayIconAdd( HWND hWnd )
{
	INT	crdCount = 0;

	ZeroMemory( &gstNtfyIcon, sizeof(gstNtfyIcon) );
	gstNtfyIcon.cbSize = sizeof(NOTIFYICONDATA);
	gstNtfyIcon.hWnd   = hWnd;
	gstNtfyIcon.uID    = IDC_ORINRINCOLLECTOR;
	gstNtfyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	gstNtfyIcon.uCallbackMessage = WMP_TRAYNOTIFYICON;
	gstNtfyIcon.hIcon  = ghIcon;
	StringCchPrintf( gstNtfyIcon.szTip, 128, TEXT("OrinrinCollector ー %s"), gbClipSteal ? TEXT("＜●>　<●＞") : TEXT("ＺＺＺ．．．") );

	for( ; ; )
	{
		if( Shell_NotifyIcon(NIM_ADD, &gstNtfyIcon) )
		{
			break;
		}
		else
		{
			if ( ERROR_TIMEOUT != GetLastError() || 10 <= crdCount )
			{
				MessageBox( hWnd, TEXT("タスクトレイにアイコンを登録できなかったのです。\r\n終了するのです。"), NULL, MB_OK | MB_ICONERROR );
				SendMessage( hWnd, WM_DESTROY, 0, 0 );
				break;
			}

			if( Shell_NotifyIcon( NIM_MODIFY, &gstNtfyIcon) )
			{
				return S_OK;
			}
			else
			{
				Sleep( 500 );
			}

			crdCount++;
		}
	}

	return S_OK;
}

VOID TaskTrayIconEvent( HWND hWnd, UINT uID, UINT message )
{
	POINT	stPoint;
	HMENU	hMenu, hSubMenu;

	GetCursorPos( &stPoint );

	if( WM_RBUTTONUP == message )
	{
		SetForegroundWindow( hWnd );

		hMenu = LoadMenu( ghInst, MAKEINTRESOURCE(IDC_ORINRINCOLLECTOR) );
		hSubMenu = GetSubMenu( hMenu, 0 );

		if( gbClipSteal )	CheckMenuItem( hSubMenu, IDM_CLIPSTEAL_TOGGLE, MF_CHECKED );

		TrackPopupMenu( hSubMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
	}

	if( WM_LBUTTONUP == message )
	{
		SetForegroundWindow( hWnd );

		hMenu = CreateFileSelMenu( hWnd, 0 );
		TrackPopupMenu( hMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
	}

	return;
}

HMENU CreateFileSelMenu( HWND hWnd, UINT bStyle )
{
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );
	INT		iCount, i;
	HMENU	hMenu;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH];

	iCount = ListView_GetItemCount( hLvWnd );

	hMenu = CreatePopupMenu(  );
	if( 0 == iCount )
	{
		AppendMenu( hMenu, MF_STRING, IDM_CLIPSTEAL_SELECT, TEXT("（无）") );
		EnableMenuItem( hMenu, IDM_CLIPSTEAL_SELECT, MF_GRAYED );
	}
	else
	{
		if( bStyle )
		{
			AppendMenu( hMenu, MF_STRING, IDM_CLIPSTEAL_TOGGLE, TEXT("コピー内容を頂戴する") );
			AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );

			if( gbClipSteal )	CheckMenuItem( hMenu, IDM_CLIPSTEAL_TOGGLE, MF_CHECKED );
		}

		for( i = 0; iCount > i; i++ )
		{
			FileListViewGet( hWnd, i, atPath );
			StringCchCopy( atName, MAX_PATH, atPath );
			PathStripPath( atName );
			AppendMenu( hMenu, MF_STRING, IDM_CLIPSTEAL_SELECT+i, atName );

			if( 0 == StrCmp( gatClipFile, atPath ) )
			{
				CheckMenuItem( hMenu, IDM_CLIPSTEAL_SELECT+i, MF_CHECKED );
			}
		}
	}

	return hMenu;
}

HRESULT TaskTrayIconCaptionChange( HWND hWnd )
{
	gstNtfyIcon.uFlags = NIF_TIP;

	StringCchPrintf( gstNtfyIcon.szTip, 128, TEXT("OrinrinCollector ー %s"), gbClipSteal ? TEXT("＜●>　<●＞") : TEXT("ＺＺＺ．．．") );

	Shell_NotifyIcon( NIM_MODIFY, &gstNtfyIcon );

	return S_OK;
}

HRESULT TaskTrayIconBalloon( HWND hWnd, LPTSTR ptInfo, LPTSTR ptTitle, DWORD dIconTy )
{
	gstNtfyIcon.uFlags       = NIF_INFO;
	StringCchCopy( gstNtfyIcon.szInfoTitle, 64, ptTitle );
	StringCchCopy( gstNtfyIcon.szInfo, 256, ptInfo );
	gstNtfyIcon.uTimeout     = 11111;
	gstNtfyIcon.dwInfoFlags  = dIconTy;
	Shell_NotifyIcon( NIM_MODIFY, &gstNtfyIcon );

	return S_OK;
}

VOID Cls_OnChangeCBChain( HWND hWnd, HWND hWndRemove, HWND hWndNext )
{

	if( hWndRemove == ghNextViewer ){	ghNextViewer = hWndNext;	}
	else if( hWndNext ){	SendMessage( hWndNext, WM_CHANGECBCHAIN, (WPARAM)hWndRemove, (LPARAM)hWndNext );	}

	return;
}

VOID Cls_OnDrawClipboard( HWND hWnd )
{
	OutputDebugString( TEXT("クリップボード内容確認\r\n") );

	if( ghNextViewer )	SendMessage( ghNextViewer, WM_DRAWCLIPBOARD, 0, 0 );

	ClipStealDoing( hWnd );

	return;
}

HRESULT ClipStealDoing( HWND hWnd )
{
	LPTSTR		ptTexts;
	LPSTR		pcStrs;
	CHAR		acBuffer[MAX_STRING];
	TCHAR		atMsg[MAX_STRING];

	UINT_PTR	cbSize, cbSplSz;
	DWORD		wrote;

	HANDLE	hFile;

	SYSTEMTIME	stTime;

	if( NULL == gatClipFile[0] ){	return E_NOTIMPL;	}

	if( !(gbClipSteal) ){	return  S_FALSE;	}

	ptTexts = ClipboardDataGet( NULL );
	if( !(ptTexts) ){	return E_ACCESSDENIED;	}

	pcStrs = SjisEncodeAlloc( ptTexts );
	StringCchLengthA( pcStrs, STRSAFE_MAX_CCH, &cbSize );

	hFile = CreateFile( gatClipFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE != hFile )
	{
		SetFilePointer( hFile, 0, NULL, FILE_END );

		if( gIsAST )
		{
			GetLocalTime( &stTime );
			StringCchPrintfA( acBuffer, MAX_STRING, ("[AA][%04u/%02u/%02u %02u:%02u:%02u]\r\n"),
				stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond );
			StringCchLengthA( acBuffer, MAX_STRING, &cbSplSz );
			WriteFile( hFile, acBuffer, cbSplSz, &wrote, NULL );
		}

		WriteFile( hFile, pcStrs, cbSize, &wrote, NULL );

		if( gIsAST ){	StringCchCopyA( acBuffer, MAX_STRING, ("\r\n") );	}
		else{	StringCchCopyA( acBuffer, MAX_STRING, ("\r\n[SPLIT]\r\n") );	}
		StringCchLengthA( acBuffer, MAX_STRING, &cbSplSz );
		WriteFile( hFile, acBuffer, cbSplSz, &wrote, NULL );

		SetEndOfFile( hFile );
		CloseHandle( hFile );
	}

	FREE( ptTexts );
	FREE( pcStrs );

	if( gGetMsgOn )
	{
		StringCchPrintf( atMsg, MAX_STRING, TEXT("%u Byte 取得"), cbSize );
		TaskTrayIconBalloon( hWnd, TEXT("コピーされた文字列を保存したよ。"), atMsg, 1 );
	}

	return S_OK;
}

LPTSTR ClipboardDataGet( LPVOID pVoid )
{
	LPTSTR	ptString = NULL, ptClipTxt;
	LPSTR	pcStr, pcClipTp;
	DWORD	cbSize;
	UINT	dEnumFmt;
	INT		ixCount, iC;
	HANDLE	hClipData;

	if( IsClipboardFormatAvailable( CF_UNICODETEXT ) )
	{
		OpenClipboard( NULL );

		dEnumFmt = 0;
		ixCount = CountClipboardFormats(  );
		for( iC = 0; ixCount > iC; iC++ )
		{
			dEnumFmt = EnumClipboardFormats( dEnumFmt );
			if( CF_UNICODETEXT == dEnumFmt || CF_TEXT == dEnumFmt ){	break;	}
		}
		if( 0 == dEnumFmt ){	return NULL;	}

		hClipData = GetClipboardData( dEnumFmt );

		if( CF_UNICODETEXT == dEnumFmt )
		{

			ptClipTxt = (LPTSTR)GlobalLock( hClipData );
			cbSize    = GlobalSize( (HGLOBAL)hClipData );

			if( 0 < cbSize )
			{
				ptString = (LPTSTR)malloc( cbSize );
				StringCchCopy( ptString, (cbSize / 2), ptClipTxt );
			}
		}
		else
		{
			pcClipTp = (LPSTR)GlobalLock( hClipData );
			cbSize   = GlobalSize( (HGLOBAL)hClipData );

			if( 0 < cbSize )
			{
				pcStr = (LPSTR)malloc( cbSize );
				StringCchCopyA( pcStr, cbSize, pcClipTp );

				ptString = SjisDecodeAlloc( pcStr );
				free( pcStr );
			}
		}

		GlobalUnlock( hClipData );
		CloseClipboard(  );
	}

	return ptString;
}
