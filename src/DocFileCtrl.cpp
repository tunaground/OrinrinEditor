#include "stdafx.h"
#include "OrinrinEditor.h"

extern list<ONEFILE>	gltMultiFiles;
extern FILES_ITR	gitFileIt;
extern INT		gixFocusPage;

extern  UINT	gbAutoBUmsg;

extern  UINT	gbSaveMsgOn;

static TCHAR	gatBackUpDirty[MAX_PATH];

INT	DocAstSeparatorGetAlloc( FILES_ITR, INT, UINT, LPVOID * );

INT	DocUnicode2UTF8( LPVOID * );

LPARAM DocOpendFileCheck( LPTSTR ptFile )
{
	FILES_ITR	itFile;

	for( itFile = gltMultiFiles.begin(); gltMultiFiles.end() != itFile; itFile++ )
	{

		if( !( StrCmp( itFile->atFileName, ptFile ) ) ){	return  itFile->dUnique;	}
	}

	return -1;
}

HRESULT DocFileOpen( HWND hWnd )
{
	OPENFILENAME	stOpenFile;
	BOOLEAN	bOpened;

	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];

	ZeroMemory( &stOpenFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );

	stOpenFile.lStructSize     = sizeof(OPENFILENAME);
	stOpenFile.hwndOwner       = hWnd;
	stOpenFile.lpstrFilter     = TEXT("アスキーアートファイル ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0全ての形式(*.*)\0*.*\0\0");
	stOpenFile.nFilterIndex    = 1;
	stOpenFile.lpstrFile       = atFilePath;
	stOpenFile.nMaxFile        = MAX_PATH;
	stOpenFile.lpstrFileTitle  = atFileName;
	stOpenFile.nMaxFileTitle   = MAX_STRING;

	stOpenFile.lpstrTitle      = TEXT("開くファイルを指定しておくれ");
	stOpenFile.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
	stOpenFile.lpstrDefExt     = TEXT("mlt");

	bOpened = GetOpenFileName( &stOpenFile );

	ViewFocusSet(  );

	if( !(bOpened) ){	return  E_ABORT;	}

	DocDoOpenFile( hWnd, atFilePath );

	return S_OK;
}

HRESULT DocDoOpenFile( HWND hWnd, LPTSTR ptFile )
{
	LPARAM	dNumber;

	dNumber = DocOpendFileCheck( ptFile );
	if( 1 <= dNumber )
	{
		if( SUCCEEDED( MultiFileTabSelect( dNumber ) ) )
		{
			DocMultiFileSelect( dNumber );
			return S_OK;
		}
	}

	dNumber = DocFileInflate( ptFile  );
	if( !(dNumber) )
	{
		MessageBox( hWnd, TEXT("ファイルを開けなかったよ"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
		return E_HANDLE;
	}
	else
	{
		MultiFileTabAppend( dNumber, ptFile );
		OpenHistoryLogging( hWnd , ptFile );
	}

	return S_OK;
}

VOID DocBackupDirectoryInit( LPTSTR ptCurrent )
{
	StringCchCopy( gatBackUpDirty, MAX_PATH, ptCurrent );
	PathAppend( gatBackUpDirty, BACKUP_DIR );
	CreateDirectory( gatBackUpDirty, NULL );

	return;
}

HRESULT DocFileBackup( HWND hWnd )
{
	CONST  TCHAR	aatExte[3][5] = { {TEXT(".ast")}, {TEXT(".mlt")}, {TEXT(".txt")} };

	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];
	TCHAR	atBuffer[MAX_PATH];

	HANDLE	hFile;
	DWORD	wrote;

	LPTSTR	ptExten;
	TCHAR	atExBuf[10];

	LPVOID	pBuffer;
	INT		iByteSize, iNullTmt, iCrLf;

	LPVOID	pbSplit;
	UINT	cbSplSz;

	INT		isAST, isMLT, idExten;

	UINT_PTR	iPages, i;

	FILES_ITR	itFile;

	ZeroMemory( atFilePath, sizeof(atFilePath) );
	ZeroMemory( atFileName, sizeof(atFileName) );
	ZeroMemory( atBuffer,   sizeof(atBuffer) );

	for( itFile = gltMultiFiles.begin(); itFile != gltMultiFiles.end(); itFile++ )
	{
		iPages = itFile->vcCont.size( );

		if( 1 >= iPages )	isMLT = FALSE;
		else				isMLT = TRUE;

		isAST = PageListIsNamed( itFile );

		if( isAST ){		idExten = 0;	}
		else if( isMLT ){	idExten = 1;	}
		else{				idExten = 2;	}

		StringCchCopy( atBuffer, MAX_PATH, itFile->atFileName );

		if( atBuffer[0] == NULL )
		{
			StringCchCopy( atFileName, MAX_STRING, itFile->atDummyName );
		}
		else
		{
			PathStripPath( atBuffer );
			StringCchCopy( atFileName, MAX_STRING, atBuffer );
		}

		ptExten = PathFindExtension( atFileName );
		if( 0 == *ptExten )
		{

			StringCchCopy( ptExten, 5, aatExte[idExten] );
		}
		else
		{
			StringCchCopy( atExBuf, 10, ptExten );
			CharLower( atExBuf );

			if( isAST )
			{
				if( StrCmp( atExBuf , aatExte[0] ) )
				{
					StringCchCopy( ptExten, 5, aatExte[0] );
				}
			}
			else if( isMLT )
			{
				if( StrCmp( atExBuf , aatExte[1] ) )
				{
					StringCchCopy( ptExten, 5, aatExte[1] );
				}
			}

		}

		StringCchCopy( atFilePath, MAX_PATH, gatBackUpDirty );
		PathAppend( atFilePath, atFileName );

		hFile = CreateFile( atFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( INVALID_HANDLE_VALUE == hFile )
		{
			NotifyBalloonExist( TEXT("バックアップが出来なかったよ・・・"), TEXT("異常発生"), NIIF_ERROR );

			return E_HANDLE;
		}

		iNullTmt = 1;
		iCrLf = CH_CRLF_CCH;
		SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

		if( isAST )
		{
			pbSplit = NULL;
		}
		else
		{
			pbSplit = malloc( 30 );
			ZeroMemory( pbSplit, 30 );

			cbSplSz = MLT_SPRT_CCH + CH_CRLF_CCH;
			StringCchPrintfA( (LPSTR)pbSplit, 30, ("%s%s"), MLT_SEPARATERA, CH_CRLFA );
		}

		for( i = 0; iPages > i; i++ )
		{
			if( isAST )
			{
				cbSplSz = DocAstSeparatorGetAlloc( itFile, i, D_SJIS, &pbSplit );

				WriteFile( hFile , pbSplit, (cbSplSz- iNullTmt), &wrote, NULL );
				FREE(pbSplit);
			}
			else
			{
				if( 1 <= i ){	WriteFile( hFile , pbSplit, cbSplSz, &wrote, NULL );	}
			}

			iByteSize = DocPageTextGetAlloc( itFile, i, D_SJIS, &pBuffer, TRUE );

			if( (i+1) == iPages ){	iByteSize -=  iCrLf;	}

			WriteFile( hFile, pBuffer, iByteSize - iNullTmt, &wrote, NULL );

			FREE( pBuffer );
		}

		SetEndOfFile( hFile );
		CloseHandle( hFile );

		FREE( pbSplit );
	}

	if( gbAutoBUmsg ){	NotifyBalloonExist( TEXT("作業中のファイルをバックアップ保存したよ。"), TEXT("お燐からのお知らせ"), NIIF_INFO );	}

	return S_OK;
}

HRESULT DocFileSave( HWND hWnd, UINT bStyle )
{
	CONST  TCHAR	aatExte[3][5] = { {TEXT(".ast")}, {TEXT(".mlt")}, {TEXT(".txt")} };
	CONST  WCHAR	rtHead = 0xFEFF;

	SYSTEMTIME		stSysTile;
	OPENFILENAME	stSaveFile;

	BOOLEAN	bOpened;

	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];
	TCHAR	atBuffer[MAX_STRING];

	HANDLE	hFile;
	DWORD	wrote;

	LPTSTR	ptExten;
	TCHAR	atExBuf[10];

	LPVOID	pBuffer;
	INT		iByteSize, iNullTmt, iCrLf;

	LPVOID	pbSplit;
	UINT	cbSplSz;

	INT		isAST, isMLT, idExten, mbRslt;
	BOOLEAN	bExtChg =FALSE, bLastChg = FALSE;
	BOOLEAN	bForceMLT = FALSE;
	BOOLEAN	bNoName = FALSE;

	BOOLEAN	bUtf8 = FALSE;
	BOOLEAN	bUnic = FALSE;

	UINT_PTR	iPages, i;

	ZeroMemory( &stSaveFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );
	ZeroMemory( atBuffer,  sizeof(atBuffer) );

	iPages = DocNowFilePageCount( );
	if( 1 >= iPages )	isMLT = FALSE;
	else				isMLT = TRUE;

	isAST = PageListIsNamed( gitFileIt );

	if( isAST ){	idExten = 0;	}
	else{			idExten = 1;	}

	GetLocalTime( &stSysTile );

	StringCchCopy( atFilePath, MAX_PATH, (*gitFileIt).atFileName );

	if( NULL == (*gitFileIt).atFileName[0] )	bNoName = TRUE;

	if( (bStyle & D_RENAME) || bNoName )
	{

		stSaveFile.lStructSize     = sizeof(OPENFILENAME);
		stSaveFile.hwndOwner       = hWnd;
		stSaveFile.lpstrFilter     = TEXT("[ShiftJIS]アスキーアートファイル ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0[UTF8]アスキーアートファイル ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0\0");
		stSaveFile.nFilterIndex    = 1;
		stSaveFile.lpstrFile       = atFilePath;
		stSaveFile.nMaxFile        = MAX_PATH;
		stSaveFile.lpstrFileTitle  = atFileName;
		stSaveFile.nMaxFileTitle   = MAX_STRING;

		stSaveFile.lpstrTitle      = TEXT("保存するファイル名を指定してね");
		stSaveFile.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

		bOpened = GetSaveFileName( &stSaveFile );

		ViewFocusSet(  );

		if( !(bOpened) ){	return  E_ABORT;	}

		if(  2 == stSaveFile.nFilterIndex ){	bUtf8 = TRUE;	}

		if( bUnic || bUtf8 )
		{
			if( NULL == (*gitFileIt).atFileName[0] )
			{
				MessageBox( hWnd, TEXT("先に通常の保存をしてからエクスポートしてね。"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONINFORMATION );
				return E_FAIL;
			}
		}
		else{	bLastChg = TRUE;	}

	}

	ptExten = PathFindExtension( atFilePath );
	if( 0 == *ptExten )
	{

		StringCchCopy( ptExten, 5, aatExte[idExten] );
		bExtChg = TRUE;
	}
	else
	{
		StringCchCopy( atExBuf, 10, ptExten );
		CharLower( atExBuf );

		if( !( StrCmp( atExBuf, aatExte[0] ) ) )
		{

			isAST = TRUE;	isMLT = FALSE;	idExten = 0;
		}

		if( !( StrCmp( atExBuf, aatExte[1] ) ) )
		{
			if( isAST && (bStyle & D_RENAME) )
			{
				mbRslt = MessageBox( hWnd, TEXT("MLTで保存すると頁名称がなくなっちゃうよ。\r\nそれでも良いかい？"), TEXT("お燐からの確認"), MB_OKCANCEL | MB_ICONQUESTION );
				if( IDOK != mbRslt )	return E_ABORT;

				isMLT = TRUE;	isAST = FALSE;	idExten = 1;
				bForceMLT = TRUE;
			}
		}

		if( isAST )
		{
			if( StrCmp( atExBuf , aatExte[0] ) )
			{
				StringCchCopy( ptExten, 5, aatExte[0] );
				bExtChg = TRUE;
			}
		}
		else if( isMLT )
		{
			if( StrCmp( atExBuf , aatExte[1] ) )
			{
				StringCchCopy( ptExten, 5, aatExte[1] );
				bExtChg = TRUE;
			}
		}

	}

	if( !(bUnic) &&  !(bUtf8) ){	StringCchCopy( (*gitFileIt).atFileName, MAX_PATH, atFilePath );	}

	hFile = CreateFile( atFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		MessageBox( hWnd, TEXT("ファイルを開けなかったよ"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
		return E_HANDLE;
	}

	iNullTmt = 1;
	iCrLf = CH_CRLF_CCH;
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	if( bUnic )
	{
		WriteFile( hFile, &rtHead, 2, &wrote, NULL );
		iNullTmt = 2;
		iCrLf *= 2;
	}

	if( isAST )
	{
		pbSplit = NULL;
	}
	else
	{
		pbSplit = malloc( 30 );
		ZeroMemory( pbSplit, 30 );

		if( bUnic )
		{
			cbSplSz = (MLT_SPRT_CCH + CH_CRLF_CCH) * sizeof(TCHAR);
			StringCchPrintfW( (LPTSTR)pbSplit, 15, TEXT("%s%s"), MLT_SEPARATERW, CH_CRLFW );
		}
		else
		{
			cbSplSz = MLT_SPRT_CCH + CH_CRLF_CCH;
			StringCchPrintfA( (LPSTR)pbSplit, 30, ("%s%s"), MLT_SEPARATERA, CH_CRLFA );
		}
	}

	if( bUnic || bUtf8 ){	bStyle |= D_UNI;	}

	for( i = 0; iPages > i; i++ )
	{
		if( isAST )
		{

			cbSplSz = DocAstSeparatorGetAlloc( gitFileIt, i, bStyle, &pbSplit );

			if( bUtf8 ){	cbSplSz = DocUnicode2UTF8( &pbSplit );	}

			WriteFile( hFile , pbSplit, (cbSplSz- iNullTmt), &wrote, NULL );
			FREE(pbSplit);
		}
		else
		{
			if( 1 <= i ){	WriteFile( hFile , pbSplit, cbSplSz, &wrote, NULL );	}
			if( bForceMLT ){	DocAstSeparatorGetAlloc( gitFileIt, i, 0, NULL );	}
		}

		iByteSize = DocPageTextGetAlloc( gitFileIt, i, bStyle, &pBuffer, TRUE );

		if( bUtf8 ){	iByteSize = DocUnicode2UTF8( &pBuffer );	}

		if( (i+1) == iPages ){	iByteSize -=  iCrLf;	}
		WriteFile( hFile, pBuffer, iByteSize - iNullTmt, &wrote, NULL );

		FREE( pBuffer );
	}

	SetEndOfFile( hFile );
	CloseHandle( hFile );

	FREE( pbSplit );

	if( !(bUnic) &&  !(bUtf8) ){	DocModifyContent( FALSE );	}

	if( bExtChg )
	{

		MultiFileTabRename( (*gitFileIt).dUnique, atFilePath );
		AppTitleChange( atFilePath );
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("拡張子を %s にして保存したよ。"), aatExte[idExten] );
		NotifyBalloonExist( atBuffer, TEXT("お燐からのお知らせ"), NIIF_INFO );

		OpenHistoryLogging( hWnd , atFilePath );
	}
	else
	{

		if( bLastChg )
		{

			MultiFileTabRename( (*gitFileIt).dUnique, atFilePath );
			AppTitleChange( atFilePath );

			OpenHistoryLogging( hWnd , atFilePath );
		}

		if( bUnic || bUtf8 )
		{
			NotifyBalloonExist( TEXT("ファイルのエクスポートしたよ。"), TEXT("お燐からのお知らせ"), NIIF_INFO );
		}
		else
		{
			if( gbSaveMsgOn ){	NotifyBalloonExist( TEXT("ファイルを保存したよ。"), TEXT("お燐からのお知らせ"), NIIF_INFO );	}
		}
	}

	if( bForceMLT ){	PageListViewRewrite( -1 );	}

	return S_OK;
}

INT DocUnicode2UTF8( LPVOID *pText )
{
	UINT_PTR	cchSz;
	INT	cbSize, rslt;
	LPVOID		pUtf8;

	StringCchLength( (LPTSTR)(*pText), STRSAFE_MAX_CCH, &cchSz );

	cbSize = WideCharToMultiByte( CP_UTF8, 0, (LPTSTR)(*pText), -1, NULL, 0, NULL, NULL );
	TRACE( TEXT("cbSize[%d]"), cbSize );
	pUtf8 = (LPSTR)malloc( cbSize );
	ZeroMemory( pUtf8, cbSize );
	rslt = WideCharToMultiByte( CP_UTF8, 0, (LPTSTR)(*pText), -1, (LPSTR)(pUtf8), cbSize, NULL, NULL );
	TRACE( TEXT("rslt[%d]"), rslt );

	FREE( *pText );

	*pText = pUtf8;

	return cbSize;
}

INT DocAstSeparatorGetAlloc( FILES_ITR itFile, INT dPage, UINT bStyle, LPVOID *pText )
{
	UINT	cchSize, cbSize;
	TCHAR	atBuffer[MAX_STRING];

	StringCchPrintf( atBuffer, MAX_STRING, TEXT("[AA][%s]\r\n"), itFile->vcCont.at( dPage ).atPageName );
	StringCchLength( atBuffer, MAX_STRING, &cchSize );

	if( !(pText) )
	{
		ZeroMemory( itFile->vcCont.at( dPage ).atPageName, SUB_STRING * sizeof(TCHAR) );
		return 0;
	}

	if( bStyle & D_UNI )
	{
		cbSize = (cchSize + 1) *  sizeof(TCHAR);

		*pText = (LPTSTR)malloc( cbSize );
		ZeroMemory( *pText, cbSize );
		StringCchCopy( (LPTSTR)(*pText), (cchSize + 1), atBuffer );
	}
	else
	{
		cbSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atBuffer, cchSize, NULL, 0, NULL, NULL );
		cbSize++;
		*pText = (LPSTR)malloc( cbSize );
		ZeroMemory( *pText, cbSize );
		WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atBuffer, cchSize, (LPSTR)(*pText), cbSize, NULL, NULL );
	}

	return cbSize;
}

HRESULT DocImageSave( HWND hWnd, UINT bStyle, HFONT hFont )
{

	LPVOID	pBuffer;
	LPTSTR	ptText;
	UINT	dLines;
	INT		iDotX, iDotY, iByteSize, bType;
	UINT_PTR	cchSize;
	RECT	rect;

	INT	iLine;
	UINT_PTR	cchLen, start, caret = 0;

	BOOL	bOpened;
	OPENFILENAME	stSaveFile;

	TCHAR	atOutName[MAX_PATH], atFileName[MAX_STRING];
	TCHAR	atPart[MIN_STRING];

	HDC		hdc, hMemDC;
	HBITMAP	hBitmap,hOldBmp;
	HFONT	hOldFont;

	StringCchCopy( atOutName, MAX_PATH, gitFileIt->atFileName );

	PathRemoveExtension( atOutName );

	StringCchPrintf( atPart, MIN_STRING, TEXT("_Page%d"), gixFocusPage );
	StringCchCat( atOutName, MAX_PATH, atPart );

	ZeroMemory( &stSaveFile, sizeof(OPENFILENAME) );
	stSaveFile.lStructSize     = sizeof(OPENFILENAME);
	stSaveFile.hwndOwner       = hWnd;
	stSaveFile.lpstrFilter     = TEXT("BMP ファイル ( *.bmp )\0*.bmp\0PNG ファイル ( *.png )\0*.png\0\0");
	stSaveFile.nFilterIndex    = 1;
	stSaveFile.lpstrFile       = atOutName;
	stSaveFile.nMaxFile        = MAX_PATH;
	stSaveFile.lpstrFileTitle  = atFileName;
	stSaveFile.nMaxFileTitle   = MAX_STRING;

	stSaveFile.lpstrTitle      = TEXT("保存するファイル名と形式を指定しておくれ");
	stSaveFile.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	bOpened = GetSaveFileName( &stSaveFile );

	ViewFocusSet(  );

	if( !(bOpened) ){	return  E_ABORT;	}

	switch( stSaveFile.nFilterIndex )
	{
		default:	bType = ISAVE_BMP;	break;
		case  2:	bType = ISAVE_PNG;	break;
	}

	dLines = DocNowFilePageLineCount(  );
	iDotX  = DocPageMaxDotGet( -1, -1 );
	iDotY  = dLines * LINE_HEIGHT;

	iDotX += 8;
	iDotY += 8;

	SetRect( &rect, 4, 4, iDotX - 4, iDotY- 4 );

	TRACE( TEXT("サイズ %d x %d"), iDotX, iDotY );

	iByteSize = DocPageTextGetAlloc( gitFileIt, gixFocusPage, D_UNI, &pBuffer, TRUE );
	ptText = (LPTSTR)pBuffer;
	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

	hdc = GetDC( hWnd );

	hBitmap = CreateCompatibleBitmap( hdc, iDotX, iDotY );
	hMemDC  = CreateCompatibleDC( hdc );

	hOldBmp  = SelectBitmap( hMemDC, hBitmap );
	hOldFont = SelectFont( hMemDC, hFont );

	PatBlt( hMemDC, 0, 0, iDotX, iDotY, WHITENESS );

	ReleaseDC( hWnd, hdc );

	iLine  = 0;
	cchLen = 0;
	start  = 0;

	for( caret = 0; cchSize > caret; )
	{
		if( TEXT('\r') == ptText[caret] )
		{
			TextOut( hMemDC, 0, iLine, &(ptText[start]), cchLen );
			cchLen = 0;
			caret += 2;
			start = caret;

			iLine += LINE_HEIGHT;
		}
		else
		{
			cchLen++;
			caret++;
		}
	}

	TextOut( hMemDC, 0, iLine, &(ptText[start]), cchLen );

	FREE(pBuffer);

	if( SUCCEEDED( ImageFileSaveDC( hMemDC, atOutName, bType ) ) )
	{

		TRACE( TEXT("保存 %s"), atOutName );
	}
	else
	{

		TRACE( TEXT("失敗 %s"), atOutName );
	}

	SelectBitmap( hMemDC, hOldBmp );
	DeleteBitmap( hBitmap );

	SelectFont( hMemDC, hOldFont );

	DeleteDC( hMemDC );

	return S_OK;
}

HRESULT DocHtmlExport( HWND hWnd )
{
	TCHAR	atFilePath[MAX_PATH], atCommandLine[BIG_STRING + 10];
	TCHAR	atExePath[MAX_PATH];

	 PROCESS_INFORMATION	stProInfo;
	 STARTUPINFO	stStartInfo;

	ZeroMemory( atFilePath,  sizeof(atFilePath) );

	StringCchCopy( atFilePath, MAX_PATH, (*gitFileIt).atFileName );

	if( gitFileIt->dModify || ( NULL == atFilePath[0] ) )
	{
		MessageBox( hWnd, TEXT("先にファイルを保存してからにしてね。"), TEXT("ファイルが保存されてないよ"), MB_OK | MB_ICONERROR );
		return E_ABORT;
	}
	PathQuoteSpaces( atFilePath );

	ZeroMemory( atExePath, sizeof(atExePath) );
	InitParamString( INIT_LOAD, VS_EXT_M2H_PATH, atExePath );
	if( NULL == atExePath[0] )
	{
		MessageBox( hWnd, TEXT("MLT2HTML.exe を設定しておいてね。"), TEXT("外部ツールが無いよ"), MB_OK | MB_ICONERROR );
		return E_ABORT;
	}
	PathQuoteSpaces( atExePath );

	ZeroMemory( atCommandLine,  sizeof(atCommandLine) );

	StringCchPrintf( atCommandLine, BIG_STRING + 10, TEXT("%s %s"), atExePath, atFilePath );

	ZeroMemory( &stProInfo, sizeof(PROCESS_INFORMATION) );

	ZeroMemory( &stStartInfo, sizeof(STARTUPINFO) );
	stStartInfo.cb = sizeof(STARTUPINFO);

	CreateProcess( NULL, atCommandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &stStartInfo, &stProInfo );

	CloseHandle( stProInfo.hThread );

	WaitForSingleObject( stProInfo.hProcess, INFINITE );

	CloseHandle( stProInfo.hProcess );

	return S_OK;
}
