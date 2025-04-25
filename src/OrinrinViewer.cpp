#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"

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

static  UINT	gdUseMode;
static  UINT	gdUseSubMode;

static  HWND	ghMaaWnd;
static TCHAR	gatIniPath[MAX_PATH];

extern  HWND	ghMaaFindDlg;

extern HFONT	ghAaFont;

extern  UINT	gdClickDrt;

extern HMENU	ghProfHisMenu;

BOOLEAN	SelectFolderDlg( HWND, LPTSTR, UINT_PTR );

HRESULT	ViewingFontNameLoad( VOID );

INT APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG		msg;
	HACCEL	hAccelTable;
	INT		msRslt;

#ifdef _DEBUG

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	INITCOMMONCONTROLSEX	iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC  = ICC_WIN95_CLASSES;
	InitCommonControlsEx( &iccex );

	GetCurrentDirectory( MAX_PATH, gatIniPath );
	PathAppend( gatIniPath, INI_FILE );

	SplitBarClass( hInstance );

	gdUseMode    = InitParamValue( INIT_LOAD, VL_MAA_LCLICK, MAA_SJISCLIP );
	gdUseSubMode = InitParamValue( INIT_LOAD, VL_MAA_MCLICK, MAA_SJISCLIP );

	ViewingFontNameLoad(  );

	ghMaaWnd = MaaTmpltInitialise( hInstance, GetDesktopWindow(), NULL );
	if( !(ghMaaWnd) )	return (-1);

#ifdef USE_HOVERTIP
	HoverTipInitialise( hInstance, ghMaaWnd );
#endif

	DraughtInitialise( hInstance, ghMaaWnd );
	gdClickDrt = gdUseMode;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ORINRINVIEWER));

	for(;;)
	{
		msRslt = GetMessage( &msg, NULL, 0, 0 );
		if( 1 != msRslt )	break;

		if( ghMaaFindDlg )
		{
			if( ghMaaFindDlg == GetForegroundWindow(  ) )
			{
				if( TranslateAccelerator( ghMaaFindDlg, hAccelTable, &msg ) )	continue;
				if( IsDialogMessage( ghMaaFindDlg, &msg ) )	continue;
			}
		}

		if( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
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

#pragma region ("設定内容読書")

INT InitParamValue( UINT dMode, UINT dStyle, INT nValue )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	INT	dBuff = 0;

	switch( dStyle )
	{
		case  VL_MAA_SPLIT:		StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSplit") );		break;
		case  VL_MAA_LCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaMethod") );		break;
		case  VL_MAATIP_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaToolTip")  );	break;
		case  VL_MAATIP_SIZE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaToolTipSize") );	break;
		case  VL_MAA_TOPMOST:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaTopMost")  );	break;
		case  VL_MAASEP_STYLE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSepLine")  );	break;
		case  VL_MAA_MCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSubMethod") );	break;
		case  VL_MAA_BKCOLOUR:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaBkColour") );	break;
		case  VL_THUMB_HORIZ:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ThumbHoriz")  );	break;
		case  VL_THUMB_VERTI:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ThumbVerti")  );	break;
		case  VL_MAATAB_SNGL:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaTabSingle") );	break;
		default:	return nValue;
	}

	if( dMode  )
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );
		GetPrivateProfileString( TEXT("General"), atKeyName, atBuff, atBuff, MIN_STRING, gatIniPath );
		dBuff = StrToInt( atBuff );
	}
	else
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );
		WritePrivateProfileString( TEXT("General"), atKeyName, atBuff, gatIniPath );
	}

	return dBuff;
}

HRESULT InitParamString( UINT dMode, UINT dStyle, LPTSTR ptFile )
{
	TCHAR	atKeyName[MIN_STRING], atDefault[MAX_PATH];

	if(  !(ptFile) )	return E_INVALIDARG;

	switch( dStyle )
	{
		case VS_PROFILE_NAME:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ProfilePath") );	break;
		case VS_PAGE_FORMAT:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageFormat")  );	break;
		case VS_FONT_NAME:		StringCchCopy( atKeyName, SUB_STRING, TEXT("FontName") );		break;
		default:	return E_INVALIDARG;
	}

	if( dMode )
	{
		StringCchCopy( atDefault, MAX_PATH, ptFile );
		GetPrivateProfileString( TEXT("General"), atKeyName, atDefault, ptFile, MAX_PATH, gatIniPath );
	}
	else
	{
		WritePrivateProfileString( TEXT("General"), atKeyName, ptFile, gatIniPath );
	}

	return S_OK;
}

HRESULT InitWindowPos( UINT dMode, UINT dStyle, LPRECT pstRect )
{
	TCHAR	atAppName[MIN_STRING], atBuff[MIN_STRING];

	if( !pstRect )	return E_INVALIDARG;

	switch( dStyle )
	{
		case  WDP_MAATPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("MaaTmple") );	break;
		default:	SetRect( pstRect , 0, 0, 0, 0 );	return E_INVALIDARG;
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

HRESULT OpenProfMenuModify( HWND hWnd )
{
	HMENU	hMenu, hSubMenu;

	hMenu = GetMenu( hWnd );
	hSubMenu = GetSubMenu( hMenu, 0 );

	ModifyMenu( hSubMenu, 2, MF_BYPOSITION | MF_POPUP, (UINT_PTR)ghProfHisMenu, TEXT("ファイル使用履歴(&H)") );

	DrawMenuBar( hWnd );

	return S_OK;
}

#pragma endregion

#pragma region ("クリップする処理")

HRESULT DocClipboardDataSet( LPVOID pDatum, INT cbSize, UINT dStyle )
{
	HGLOBAL	hGlobal;
	HANDLE	hClip;
	LPVOID	pBuffer;
	HRESULT	hRslt;
	UINT	ixFormat, ixSqrFmt;

	ixFormat = RegisterClipboardFormat( CLIP_FORMAT );
	ixSqrFmt = RegisterClipboardFormat( CLIP_SQUARE );

	hGlobal = GlobalAlloc( GHND, cbSize );
	pBuffer = GlobalLock( hGlobal );
	CopyMemory( pBuffer, pDatum, cbSize );
	GlobalUnlock( hGlobal );

	OpenClipboard( NULL );

	EmptyClipboard(  );

	if( dStyle & D_UNI )	hClip = SetClipboardData( CF_UNICODETEXT, hGlobal );
	else					hClip = SetClipboardData( CF_TEXT, hGlobal );

	if( hClip )
	{

		SetClipboardData( ixFormat, hGlobal );
		hRslt = S_OK;
	}
	else
	{

		GlobalFree( hGlobal );
		hRslt = E_OUTOFMEMORY;
	}

	CloseClipboard(  );

	TRACE( TEXT("COPY DONE") );

	return hRslt;
}

UINT ViewMaaMaterialise( HWND hWnd, LPSTR pcCont, UINT cbSize, UINT dMode )
{
	LPTSTR		ptString;
	UINT_PTR	cchSize;
	UINT		uRslt = TRUE;

	if( MAA_DEFAULT ==  dMode ){	dMode = gdUseMode;	}
	if( MAA_SUBDEFAULT== dMode ){	dMode = gdUseSubMode;	}

	if( MAA_UNICLIP == dMode )
	{
		ptString = SjisDecodeAlloc( pcCont );
		StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

		DocClipboardDataSet( ptString, (cchSize + 1) * 2, D_UNI );

		FREE(ptString);
	}
	else if( MAA_DRAUGHT == dMode ){	DraughtItemAdding( hWnd, pcCont );	}
	else{	DocClipboardDataSet( pcCont, (cbSize + 1), D_SJIS );	}

	return uRslt;
}

#pragma endregion

#pragma region ("設定ダイヤログ")

INT_PTR CALLBACK OptionDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	UINT	id;
	INT		dValue, dBuff;

	switch( message )
	{
		case WM_INITDIALOG:

			dValue = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, 16 );
			if( FONTSZ_REDUCE == dValue )	CheckRadioButton( hDlg, IDRB_POPUP_NOMAL, IDRB_POPUP_REDUCE, IDRB_POPUP_REDUCE );
			else							CheckRadioButton( hDlg, IDRB_POPUP_NOMAL, IDRB_POPUP_REDUCE, IDRB_POPUP_NOMAL );

			dValue = InitParamValue( INIT_LOAD, VL_MAATIP_VIEW, 1 );
			CheckDlgButton( hDlg, IDCB_POPUP_VISIBLE, dValue ? BST_CHECKED : BST_UNCHECKED );

			dValue = InitParamValue( INIT_LOAD, VL_MAA_LCLICK, MAA_SJISCLIP );
			switch( dValue )
			{
				case MAA_UNICLIP:	id = IDRB_SEL_CLIP_UNI;		break;
				default:
				case MAA_SJISCLIP:	id = IDRB_SEL_CLIP_SJIS;	break;
				case MAA_DRAUGHT:	id = IDRB_SEL_DRAUGHT;		break;
			}
			CheckRadioButton( hDlg, IDRB_SEL_INS_EDIT, IDRB_SEL_DRAUGHT, id );

			dValue = InitParamValue( INIT_LOAD, VL_MAA_MCLICK, MAA_SJISCLIP );
			switch( dValue )
			{
				case MAA_UNICLIP:	id = IDRB_SELSUB_CLIP_UNI;	break;
				default:
				case MAA_SJISCLIP:	id = IDRB_SELSUB_CLIP_SJIS;	break;
				case MAA_DRAUGHT:	id = IDRB_SELSUB_DRAUGHT;	break;
			}
			CheckRadioButton( hDlg, IDRB_SELSUB_INS_EDIT, IDRB_SELSUB_DRAUGHT, id );

			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{

				case IDB_APPLY:
				case IDOK:

					dValue = FONTSZ_NORMAL;
					if( IsDlgButtonChecked( hDlg, IDRB_POPUP_REDUCE ) ){	dValue =  FONTSZ_REDUCE;	}
					InitParamValue( INIT_SAVE, VL_MAATIP_SIZE, dValue );
					dBuff = IsDlgButtonChecked( hDlg, IDCB_POPUP_VISIBLE );
					AaItemsTipSizeChange( dValue, dBuff );
					InitParamValue( INIT_SAVE, VL_MAATIP_VIEW, dBuff );

					if(      IsDlgButtonChecked( hDlg, IDRB_SEL_CLIP_UNI ) ){	dValue = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_DRAUGHT ) ){	dValue = MAA_DRAUGHT;	}
					else{	dValue = MAA_SJISCLIP;	}
					InitParamValue( INIT_SAVE, VL_MAA_LCLICK, dValue );
					gdUseMode = dValue;
					gdClickDrt = gdUseMode;

					if(      IsDlgButtonChecked( hDlg, IDRB_SELSUB_CLIP_UNI ) ){	dValue = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_DRAUGHT )  ){	dValue = MAA_DRAUGHT;	}
					else{	dValue = MAA_SJISCLIP;	}
					InitParamValue( INIT_SAVE, VL_MAA_MCLICK, dValue );
					gdUseSubMode = dValue;

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

BOOLEAN SelectDirectoryDlg( HWND hWnd, LPTSTR ptSelFolder, UINT_PTR cchLen )
{
	BROWSEINFO		stBrowseInfo;
	LPITEMIDLIST	pstItemIDList;
	TCHAR	atDisplayName[MAX_PATH];

	if( !(ptSelFolder) )	return FALSE;
	ZeroMemory( ptSelFolder, sizeof(TCHAR) * cchLen );

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

		StringCchCopy( ptSelFolder, cchLen, atDisplayName );

		CoTaskMemFree( pstItemIDList );
	}

	return TRUE;
}

#pragma endregion

INT ViewLetterWidthGet( TCHAR ch )
{
	SIZE	stSize;
	HDC		hdc= GetDC( ghMaaWnd );
	HFONT	hFtOld;

	hFtOld = SelectFont( hdc, ghAaFont );

	GetTextExtentPoint32( hdc, &ch, 1, &stSize );

	SelectFont( hdc, hFtOld );

	ReleaseDC( ghMaaWnd, hdc );

	return stSize.cx;
}

INT ViewStringWidthGet( LPCTSTR ptStr )
{
	SIZE	stSize;
	UINT	cchSize;
	HDC		hdc= GetDC( ghMaaWnd );
	HFONT	hFtOld;

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

	if( 0 >= cchSize )	return 0;

	hFtOld = SelectFont( hdc, ghAaFont );

	GetTextExtentPoint32( hdc, ptStr, cchSize, &stSize );

	SelectFont( hdc, hFtOld );

	ReleaseDC( ghMaaWnd, hdc );

	return stSize.cx;
}

UINT ViewMaaItemsModeGet( PUINT pdSubMode )
{
	if( pdSubMode ){	*pdSubMode = gdUseSubMode;	}

	return gdUseMode;
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
