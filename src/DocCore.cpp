#include "stdafx.h"
#include "OrinrinEditor.h"

#define PAGE_LINE_MAX	80
#define LINE_MOZI_MAX	255

EXTERNED list<ONEFILE>	gltMultiFiles;

static LPARAM	gdNextNumber;

EXTERNED FILES_ITR	gitFileIt;

EXTERNED INT		gixFocusPage;

EXTERNED INT		gixDropPage;

extern  UINT		gbUniRadixHex;
extern  UINT		gbCrLfCode;

UINT	CALLBACK DocPageLoad( LPTSTR, LPCTSTR, INT );

HRESULT DocInitialise( UINT dMode )
{
	FILES_ITR	itFile;
	PAGE_ITR	itPage;

	if( dMode )
	{
		gdNextNumber = 1;
	}
	else
	{
		for( itFile = gltMultiFiles.begin( ); itFile != gltMultiFiles.end(); itFile++ )
		{
			for( itPage = itFile->vcCont.begin( ); itPage != itFile->vcCont.end(); itPage++ )
			{
				FREE( itPage->ptRawData );
			}
		}
	}

	return S_OK;
}

HRESULT DocModifyContent( UINT dMode )
{
	if( dMode )
	{
		if( (*gitFileIt).dModify )	return S_FALSE;

		MainStatusBarSetText( SB_MODIFY, MODIFY_MSG );
	}
	else
	{
		MainStatusBarSetText( SB_MODIFY, TEXT("") );
	}

	DocMultiFileModify( dMode );

	(*gitFileIt).dModify =  dMode;

	return S_OK;
}

LPARAM DocMultiFileCreate( LPTSTR ptDmyName )
{
	ONEFILE	stFile;
	FILES_ITR	itNew;

#ifdef DO_TRY_CATCH
	try{
#endif

	ZeroMemory( stFile.atFileName, sizeof(stFile.atFileName) );
	stFile.dModify   = FALSE;
	stFile.dNowPage  = 0;
	stFile.dUnique   = gdNextNumber++;
	stFile.stCaret.x = 0;
	stFile.stCaret.y = 0;

	ZeroMemory( stFile.atDummyName, sizeof(stFile.atDummyName) );
	StringCchPrintf( stFile.atDummyName, MAX_PATH, TEXT("%s%d.%s"), NAME_DUMMY_NAME, stFile.dUnique, NAME_DUMMY_EXT );

	if( ptDmyName ){	StringCchCopy( ptDmyName, MAX_PATH, stFile.atDummyName );	}

	stFile.vcCont.clear(  );

	gltMultiFiles.push_back( stFile );

	gixFocusPage = -1;

	PageListClear(  );

	itNew = gltMultiFiles.end( );
	itNew--;

	gitFileIt = itNew;

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("기타 오류"), 0 );	}
#endif

	return stFile.dUnique;
}

HRESULT DocActivateEmptyCreate( LPTSTR ptFile )
{
	INT	iNewPage;

	DocMultiFileCreate( ptFile );
	iNewPage = DocPageCreate( -1 );
	PageListInsert( iNewPage  );
	DocPageChange( iNewPage );
	MultiFileTabFirst( ptFile );
	AppTitleChange( ptFile );

	return S_OK;
}

HRESULT DocMultiFileModify( UINT dMode )
{
	TCHAR	atFile[MAX_PATH];

	StringCchCopy( atFile, MAX_PATH, (*gitFileIt).atFileName );
	if( 0 == atFile[0] ){	StringCchCopy( atFile, MAX_PATH , (*gitFileIt).atDummyName );	}

	PathStripPath( atFile );

	if( dMode ){	StringCchCat( atFile, MAX_PATH, MODIFY_MSG );	}

	MultiFileTabRename( (*gitFileIt).dUnique, atFile );

	return S_OK;
}

HRESULT DocMultiFileSelect( LPARAM uqNumber )
{
	FILES_ITR	itNow;
	POINT	stCaret;

	for( itNow = gltMultiFiles.begin(); itNow != gltMultiFiles.end(); itNow++ )
	{
		if( uqNumber == itNow->dUnique )	break;
	}
	if( itNow == gltMultiFiles.end() )	return E_OUTOFMEMORY;

	ViewSelPageAll( -1 );

	PageListClear(  );

	gitFileIt = itNow;

	PageListBuild( NULL );

	AppTitleChange( itNow->atFileName );

	gixFocusPage = itNow->dNowPage;

	DocModifyContent( itNow->dModify );

	DocCaretPosMemory( INIT_LOAD, &stCaret );

	PageListViewChange( gixFocusPage,  -1 );

	ViewPosResetCaret( stCaret.x, stCaret.y );

	return S_OK;
}

HRESULT DocMultiFileCloseAll( VOID )
{
	UINT_PTR	i, iPage, iLine;
	FILES_ITR	itNow;
	LINE_ITR	itLine;

	for( itNow = gltMultiFiles.begin( ); itNow != gltMultiFiles.end(); itNow++ )
	{
		iPage = itNow->vcCont.size( );
		for( i = 0; iPage > i; i++ )
		{
			iLine  = itNow->vcCont.at( i ).ltPage.size( );

			itLine = itNow->vcCont.at( i ).ltPage.begin();
			for( itLine = itNow->vcCont.at( i ).ltPage.begin(); itLine != itNow->vcCont.at( i ).ltPage.end(); itLine++ )
			{
				itLine->vcLine.clear( );
			}
			itNow->vcCont.at( i ).ltPage.clear( );

			SqnFreeAll( &(itNow->vcCont.at( i ).stUndoLog) );
		}
		itNow->vcCont.clear(  );
	}

	gltMultiFiles.clear(  );

	return S_OK;
}

LPARAM DocMultiFileClose( HWND hWnd, LPARAM uqNumber )
{
	INT			iRslt;
	UINT_PTR	i, iPage, iLine;
	UINT_PTR	iCount;
	LPARAM	dNowNum, dPrevi;
	FILES_ITR	itNow;
	LINE_ITR	itLine;
	TCHAR		atBuffer[MAX_PATH];

	iCount = gltMultiFiles.size();
	if( 1 >= iCount )	return 0;

	dNowNum = gitFileIt->dUnique;

	itNow = gltMultiFiles.begin( );
	itNow++;
	dPrevi = itNow->dUnique;

	for( itNow = gltMultiFiles.begin( ); itNow != gltMultiFiles.end(); itNow++ )
	{
		if( uqNumber == itNow->dUnique )	break;
		dPrevi = itNow->dUnique;
	}
	if( itNow == gltMultiFiles.end() )	return 0;

	if( dNowNum != uqNumber )
	{
		gixFocusPage = -1;
		DocMultiFileSelect( uqNumber  );
		dPrevi = dNowNum;
	}

	if( gitFileIt->dModify )
	{
		StringCchPrintf( atBuffer, MAX_PATH, TEXT("잠깐만! [%s]는 변경된 상태야. 여기서 저장하고 닫을까?"), PathFindFileName( gitFileIt->atFileName ) );
		iRslt = MessageBox( hWnd, atBuffer, TEXT("오린의 확인"), MB_YESNOCANCEL | MB_ICONQUESTION );
		if( IDCANCEL == iRslt ){	return 0;	}

		if( IDYES == iRslt ){	DocFileSave( hWnd, D_SJIS );	}
	}

	iPage = itNow->vcCont.size( );
	for( i = 0; iPage > i; i++ )
	{
		iLine = itNow->vcCont.at( i ).ltPage.size( );

		for( itLine = itNow->vcCont.at( i ).ltPage.begin(); itLine != itNow->vcCont.at( i ).ltPage.end(); itLine++ )
		{
			itLine->vcLine.clear( );
		}
		itNow->vcCont.at( i ).ltPage.clear( );

		FREE( itNow->vcCont.at( i ).ptRawData );

		SqnFreeAll( &(itNow->vcCont.at( i ).stUndoLog) );
	}
	itNow->vcCont.clear(  );

	gltMultiFiles.erase( itNow );

	gixFocusPage = -1;
	DocMultiFileSelect( dPrevi );

	return dPrevi;
}

INT DocMultiFileFetch( INT iTgt, LPTSTR ptFile, LPTSTR ptIniPath )
{
	TCHAR	atKeyName[MIN_STRING];
	INT		iCount;

	assert( ptIniPath );

	iCount = GetPrivateProfileInt( TEXT("MultiOpen"), TEXT("Count"), 0, ptIniPath );
	if( 0 > iTgt )	return iCount;

	assert( ptFile );

	if( iCount <= iTgt ){	ptFile[0] = NULL;	return iCount;	}

	StringCchPrintf( atKeyName, MIN_STRING, TEXT("Item%u"), iTgt );

	GetPrivateProfileString( TEXT("MultiOpen"), atKeyName, TEXT(""), ptFile, MAX_PATH, ptIniPath );

	return iCount;
}

HRESULT DocMultiFileStore( LPTSTR ptIniPath )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	UINT	i;
	FILES_ITR	itNow;

	assert( ptIniPath );

	ZeroMemory( atBuff, sizeof(atBuff) );
	WritePrivateProfileSection( TEXT("MultiOpen"), atBuff, ptIniPath );

	i = 0;
	for( itNow = gltMultiFiles.begin( ); itNow != gltMultiFiles.end(); itNow++ )
	{
		if( NULL != itNow->atFileName[0] )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Item%u"), i );
			WritePrivateProfileString( TEXT("MultiOpen"), atKeyName, itNow->atFileName, ptIniPath );
			i++;
		}
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), i );
	WritePrivateProfileString( TEXT("MultiOpen"), TEXT("Count"), atBuff, ptIniPath );

	return S_OK;
}

LPTSTR DocMultiFileNameGet( INT tabNum )
{
	INT	i;
	FILES_ITR	itNow;

	for( i = 0, itNow = gltMultiFiles.begin(); itNow != gltMultiFiles.end(); i++, itNow++ )
	{
		if( tabNum == i )	break;
	}
	if( itNow == gltMultiFiles.end() )	return NULL;

	if( NULL == itNow->atFileName[ 0] ){	return itNow->atDummyName;	}

	return itNow->atFileName;
}

VOID DocCaretPosMemory( UINT dMode, LPPOINT pstPos )
{
	if( dMode )
	{
		pstPos->x = gitFileIt->stCaret.x;
		pstPos->y = gitFileIt->stCaret.y;
	}
	else
	{
		gitFileIt->stCaret.x = pstPos->x;
		gitFileIt->stCaret.y = pstPos->y;
	}

	return;
}

HRESULT DocOpenFromNull( HWND hWnd )
{
	LPARAM	dNumber;

	TCHAR	atDummyName[MAX_PATH];

	dNumber = DocMultiFileCreate( atDummyName );

	MultiFileTabAppend( dNumber, (*gitFileIt).atDummyName );

	AppTitleChange( atDummyName );

	gixFocusPage = DocPageCreate( -1 );
	PageListInsert( gixFocusPage  );
	DocPageChange( 0 );

	ViewRedrawSetLine( -1 );

	return S_OK;
}

INT DocFileCloseCheck( HWND hWnd, UINT dMode )
{
	INT		rslt, ret;

	TCHAR	atMessage[BIG_STRING];
	BOOLEAN	bMod = FALSE;
	FILES_ITR	itFiles;

	for( itFiles = gltMultiFiles.begin(); itFiles != gltMultiFiles.end(); itFiles++ )
	{
		if( itFiles->dModify )
		{
			StringCchPrintf( atMessage, BIG_STRING, TEXT("잠깐만! %s는 저장되지 않았어. 여기서 저장할까?"), itFiles->atFileName[0] ? PathFindFileName( itFiles->atFileName ) : itFiles->atDummyName );
			rslt = MessageBox( hWnd, atMessage, TEXT("오린의 확인"), MB_YESNOCANCEL | MB_ICONQUESTION );
			if( IDCANCEL ==  rslt ){	return 0;	}
			if( IDYES == rslt ){	DocFileSave( hWnd, D_SJIS );	}

			bMod = TRUE;
		}
	}

	if( !(bMod) )
	{
		rslt = MessageBox( hWnd, TEXT("이제 끝낼까?"), TEXT("오린의 확인"), MB_YESNO | MB_ICONQUESTION );
		if( IDYES == rslt ){	ret = 1;	}
		else{					ret = 0;	}
	}

	return ret;
}

#ifdef BIG_TEXT_SEPARATE

UINT DocFileHugeCheck( LPTSTR ptStr, UINT_PTR cchSize )
{
	LPTSTR		ptBuff;

	UINT_PTR	dCount;
	UINT		dRslt;

	if( 0 == StrCmpN( AST_SEPARATERW , ptStr, 4 ) ){	return 0;	}

	ptBuff = StrStr( ptStr, MLT_SEPARATERW );
	if( ptBuff ){	return 0;	}

	dCount = 0;

	do{
		ptBuff = StrStr( ptStr , TEXT("\r\n") );
		if( ptBuff ){	dCount++;	}else{	break;	}

		ptStr = ptBuff+2;
	}while( ptBuff );

	if( 0 == dCount && 1000 <= cchSize )
	{
		DocHugeFileTreatment( 2 );
		return 2;
	}

	if( 100 >= dCount ){	return 0;	}

	dRslt = DocHugeFileTreatment( 1 );

	return dRslt;
}

LPTSTR DocFileHugeSeparate( LPTSTR ptSource, UINT_PTR cchSource )
{
	LPTSTR		ptDest, ptBuff;
	UINT_PTR	dd, ds, dLineCnt, dEmptyCnt, cchDest;

	cchDest = cchSource;
	ptDest = (LPTSTR)malloc( sizeof(TCHAR) * cchDest );
	if( !(ptDest) ){	return NULL;	}
	ZeroMemory( ptDest, sizeof(TCHAR) * cchDest );

	dd = 0;
	dLineCnt = 0;
	dEmptyCnt = 0;
	for( ds = 0; cchSource > ds; ds++ )
	{
		if( NULL == ptSource[ds] )	break;

		ptDest[dd] = ptSource[ds];
		dd++;
		ptDest[dd] = NULL;

		if( 0xD == ptSource[ds] && 0xA == ptSource[ds+1] )
		{
			ds++;
			ptDest[dd] = ptSource[ds];
			dd++;
			ptDest[dd] = NULL;

			if( cchDest <= (dd+12) )
			{
				ptBuff = (LPTSTR)realloc( ptDest, sizeof(TCHAR) * (cchDest + 0x800) );
				if( !(ptBuff)  )
				{
					free( ptDest );
					return NULL;
				}
				ptDest = ptBuff;
				cchDest += 0x800;
			}

			dLineCnt++;
			if( 40 <= dLineCnt ){	dEmptyCnt++;	}

			if( (5 <= dEmptyCnt) || (100 <= dLineCnt && 2 <= dEmptyCnt) || (256 <= dLineCnt) )
			{
				StringCchCat( ptDest, cchDest, MLT_SEPARATERW );
				StringCchCat( ptDest, cchDest, TEXT("\r\n") );
				StringCchLength( ptDest, cchDest, &dd );

				dLineCnt = 0;
				dEmptyCnt = 0;
			}
		}
		else
		{
			dEmptyCnt = 0;
		}

		if( cchDest <= (dd+12) )
		{
			ptBuff = (LPTSTR)realloc( ptDest, sizeof(TCHAR) * (cchDest + 0x800) );
			if( !(ptBuff)  )
			{
				free( ptDest );
				return NULL;
			}
			ptDest = ptBuff;
			cchDest += 0x800;
		}

	}

	return ptDest;
}

#endif

LPARAM DocFileInflate( LPTSTR ptFileName )
{
	CONST WCHAR rtHead = 0xFEFF;
	WCHAR	rtUniBuf;

	HANDLE	hFile;
	DWORD	readed;

	LPVOID	pBuffer;
	INT		iByteSize;

	LPTSTR	ptString;
	LPSTR	pcText;
	UINT_PTR	cchSize;

	LPARAM	dNumber;

#ifdef BIG_TEXT_SEPARATE
	UINT	dSepRslt;
	LPTSTR	ptSepBuff;
#endif

#ifdef _DEBUG
	DWORD	dTcStart, dTcEnd;
#endif

#ifdef _DEBUG
	dTcStart = GetTickCount(  );
#endif
	assert( ptFileName );

	if( NULL == ptFileName[0] ){	return 0;	}

	hFile = CreateFile( ptFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return 0;	}

	iByteSize = GetFileSize( hFile, NULL );
	pBuffer = malloc( iByteSize + 4 );
	ZeroMemory( pBuffer, iByteSize + 4 );

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

#ifdef BIG_TEXT_SEPARATE
	dSepRslt = DocFileHugeCheck( ptString, cchSize );
	if(  1 == dSepRslt )
	{
		ptSepBuff = DocFileHugeSeparate( ptString, cchSize );
		if( !(ptSepBuff)  ){	return 0;	}

		FREE( pBuffer );
		pBuffer = ptSepBuff;
		ptString = ptSepBuff;

		StringCchLength( ptString, STRSAFE_MAX_CCH , &cchSize );
	}
	else if( 2 == dSepRslt )
	{
		FREE( pBuffer );
		return 0;
	}

#endif

	dNumber = DocMultiFileCreate( NULL );
	if( 0 >= dNumber )	return 0;

	StringCchCopy( (*gitFileIt).atFileName, MAX_PATH, ptFileName );

	if( StrCmpN( AST_SEPARATERW, ptString, 4 ) )
	{
		DocStringSplitMLT( ptString , cchSize, DocPageLoad );
	}
	else
	{
		DocStringSplitAST( ptString , cchSize, DocPageLoad );
	}

	ViewEditReset(  );

	FREE( pBuffer );

	DocPageChange( 0  );
	PageListViewChange( -1, -1 );

	AppTitleChange( ptFileName );

#ifdef _DEBUG
	dTcEnd = GetTickCount(  );
	TRACE( TEXT("LOAD START[%u]  END[%u]    ELAPSE[%u]"), dTcStart, dTcEnd, (dTcEnd - dTcStart) );
#endif

	return dNumber;
}

UINT CALLBACK DocPageLoad( LPTSTR ptName, LPCTSTR ptCont, INT cchSize )
{
	gixFocusPage = DocPageCreate(  -1 );
	PageListInsert( gixFocusPage  );

	if( ptName ){	DocPageNameSet( ptName );	}
	(*gitFileIt).vcCont.at( gixFocusPage ).ptRawData = (LPTSTR)malloc( (cchSize+2) * sizeof(TCHAR) );
	ZeroMemory( (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData, (cchSize+2) * sizeof(TCHAR) );

	StringCchCopy( (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData, (cchSize+2), ptCont );

	DocPageParamGet( NULL, NULL );

	return 1;
}

#ifdef FILE_PRELOAD

UINT DocPreloadMLT( LPTSTR ptString, INT cchSize )
{
	LPTSTR	ptCaret;
	LPTSTR	ptEnd;
	INT		iLines, iDots, iMozis;
	UINT	dPage;
	UINT_PTR	cchItem;
	BOOLEAN	bLast = FALSE;

	ptCaret = ptString;

	dPage = 0;

	do
	{
		ptEnd = StrStr( ptCaret, MLT_SEPARATERW );
		if( !ptEnd )
		{
			ptEnd = ptString + cchSize;
			bLast = TRUE;
		}
		cchItem = ptEnd - ptCaret;

		if( !(bLast) && 0 < cchItem ){	cchItem -=  CH_CRLF_CCH;	}

		dPage++;

		iLines = DocStringInfoCount( ptCaret, cchItem, &iDots, &iMozis );

		ptCaret = NextLineW( ptEnd );

	}while( *ptCaret  );

	return dPage;
}

#endif

UINT DocStringSplitMLT( LPTSTR ptStr, INT cchSize, PAGELOAD pfPageLoad )
{
	LPTSTR	ptCaret;
	LPTSTR	ptEnd;
	UINT	iNumber;
#ifdef FILE_PRELOAD
	UINT	dPage;
#endif
	UINT	cchItem;

	BOOLEAN	bLast = FALSE;

	ptCaret = ptStr;

	iNumber = 0;

#ifdef FILE_PRELOAD
	dPage = DocPreloadMLT( ptStr, cchSize );
#endif

	do
	{
		ptEnd = StrStr( ptCaret, MLT_SEPARATERW );
		if( !ptEnd )
		{
			ptEnd = ptStr + cchSize;
			bLast = TRUE;
		}
		cchItem = ptEnd - ptCaret;

		if( !(bLast) && 0 < cchItem )
		{
			cchItem -=  CH_CRLF_CCH;
			ptCaret[cchItem] = 0;
		}

		pfPageLoad( NULL, ptCaret, cchItem );

		iNumber++;

		ptCaret = NextLineW( ptEnd );

	}while( *ptCaret );

	return iNumber;
}

UINT DocStringSplitAST( LPTSTR ptStr, INT cchSize, PAGELOAD pfPageLoad )
{
	LPTSTR	ptCaret;
	LPTSTR	ptStart;
	LPTSTR	ptEnd;
	UINT	iNumber;
	UINT	cchItem;
	BOOLEAN	bLast;
	TCHAR	atName[MAX_PATH];

	ptCaret = ptStr;

	iNumber = 0;

	bLast = FALSE;

	do
	{
		ptStart = NextLineW( ptCaret );

		ptCaret += 5;
		cchItem = ptStart - ptCaret;
		cchItem -= 3;

		ZeroMemory( atName, sizeof(atName) );
		if( 0 < cchItem )	StringCchCopyN( atName, MAX_PATH, ptCaret, cchItem );

		ptCaret = ptStart;

		ptEnd = StrStr( ptCaret, AST_SEPARATERW );

		if( !ptEnd )
		{
			ptEnd = ptStr + cchSize;
			bLast = TRUE;
		}
		cchItem = ptEnd - ptCaret;

		if( !(bLast) && 0 < cchItem )
		{
			cchItem -= CH_CRLF_CCH;
			ptCaret[cchItem] = 0;
		}

		pfPageLoad( atName, ptCaret, cchItem );

		iNumber++;

		ptCaret = ptEnd;

	}while( *ptCaret );

	return iNumber;
}

UINT DocImportSplitASD( LPSTR pcStr, INT cbSize, PAGELOAD pfPageLoad )
{

	LPSTR	pcCaret;
	LPSTR	pcEnd, pcDesc;
	UINT	iNumber;
	UINT	cbItem, d;
	BOOLEAN	bLast;

	LPTSTR		ptName, ptCont;
	UINT_PTR	cchItem;

	pcCaret = pcStr;

	iNumber = 0;

	bLast = FALSE;

	do
	{
		pcEnd = NextLineA( pcCaret );

		cbItem  = pcEnd - pcCaret;

		pcDesc = NULL;
		ptName = NULL;
		ptCont = NULL;

		for( d = 0; cbItem > d; d++ )
		{
			if( (0x0D == pcCaret[d]) && (0x0A == pcCaret[d+1]) )
			{
				pcCaret[d]   = 0x00;
				pcCaret[d+1] = 0x00;

				if( pcDesc ){	ptName =  SjisDecodeAlloc( pcDesc );	}

				break;
			}

			if( (0x01 == pcCaret[d]) && (0x01 == pcCaret[d+1]) )
			{
				pcCaret[d]   = 0x0D;
				pcCaret[d+1] = 0x0A;
				d++;
			}

			if( (0x02 == pcCaret[d]) && (0x02 == pcCaret[d+1]) )
			{
				pcDesc = &(pcCaret[d+2]);

				pcCaret[d]   = 0x00;
				pcCaret[d+1] = 0x00;
				d++;
			}
		}

		ptCont = SjisDecodeAlloc( pcCaret );
		StringCchLength( ptCont, STRSAFE_MAX_CCH, &cchItem );

		pfPageLoad( ptName, ptCont, cchItem );

		iNumber++;

		FREE( ptCont );
		FREE( ptName );

		pcCaret = pcEnd;

	}while( *pcCaret  );

	return iNumber;
}

HRESULT DocPageNameSet( LPTSTR ptName )
{
	StringCchCopy( (*gitFileIt).vcCont.at( gixFocusPage ).atPageName, SUB_STRING, ptName );

	PageListNameSet( gixFocusPage, ptName );

	return S_OK;
}

INT DocPageCreate( INT iAdding )
{
	INT_PTR		iTotal, iNext;
	UINT_PTR	iAddPage = 0;
	INT		i;

	ONELINE	stLine;
	ONEPAGE	stPage;

	PAGE_ITR	itPage;

#ifdef DO_TRY_CATCH
	try{
#endif

	ZeroONELINE( &stLine  );

	ZeroMemory( stPage.atPageName, sizeof(stPage.atPageName) );

	stPage.dByteSz = 0;
	stPage.ltPage.clear(  );
	stPage.ltPage.push_back( stLine );
	stPage.dSelLineTop    =  -1;
	stPage.dSelLineBottom =  -1;
	stPage.ptRawData = NULL;
	SqnInitialise( &(stPage.stUndoLog) );

	iTotal = DocNowFilePageCount(  );

	if( 0 <= iAdding )
	{
		iNext = iAdding + 1;
		if( iTotal <= iNext ){	iNext =  -1;	}
	}
	else
	{
		iNext = -1;
	}

	if( 0 >  iNext )
	{
		(*gitFileIt).vcCont.push_back( stPage  );

		iAddPage = DocNowFilePageCount( );
		iAddPage--;
	}
	else
	{
		itPage = (*gitFileIt).vcCont.begin(  );
		for( i = 0; iNext >  i; i++ ){	itPage++;	}
		(*gitFileIt).vcCont.insert( itPage, stPage );

		iAddPage = iNext;
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("기타 오류"), 0 );	}
#endif

	return iAddPage;
}

HRESULT DocPageDelete( INT iPage, INT iBack )
{
	INT	i, iNew;
	PAGE_ITR	itPage;

	if( 1 >= DocNowFilePageCount( ) )	return E_ACCESSDENIED;

#ifdef DO_TRY_CATCH
	try{
#endif

	itPage = (*gitFileIt).vcCont.begin(  );
	for( i = 0; iPage > i; i++ ){	itPage++;	}

	FREE( itPage->ptRawData );

	SqnFreeAll( &(itPage->stUndoLog)  );
	(*gitFileIt).vcCont.erase( itPage  );
	gixFocusPage = -1;

	PageListDelete( iPage );

	if( 0 <= iBack )
	{
		iNew = iBack;
	}
	else
	{
		iNew = iPage - 1;
		if( 0 > iNew )	iNew = 0;
	}

	DocPageChange( iNew );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), E_FAIL );	}
	catch( ... ){	return  ETC_MSG( ("기타 오류"), E_FAIL );	}
#endif

	return S_OK;
}

UINT DocDelayPageLoad( FILES_ITR itFile, INT iPage )
{
	INT	dmyX = 0, dmyY = 0;
	UINT_PTR	cchSize;

	if( itFile->vcCont.at( iPage ).ptRawData )
	{
		TRACE( TEXT("페이지 지연 로드 [%d]"), iPage );

		StringCchLength( itFile->vcCont.at( iPage ).ptRawData, STRSAFE_MAX_CCH, &cchSize );

		if( 0 < cchSize )
		{
			DocStringAdd( &dmyX, &dmyY, itFile->vcCont.at( iPage ).ptRawData, cchSize );

		}

		FREE( itFile->vcCont.at( iPage ).ptRawData  );

#ifdef FIND_STRINGS
#ifdef SEARCH_HIGHLIGHT

		FindDelayPageReSearch( iPage );
#endif
#endif

	}
	else
	{
		return 0;
	}

	return 1;
}

HRESULT DocPageChange( INT dPageNum )
{
	INT	iPrePage;

#ifdef DO_TRY_CATCH
	try{
#endif

	ViewSelPageAll( -1 );

	iPrePage = gixFocusPage;
	gixFocusPage = dPageNum;

	(*gitFileIt).dNowPage = dPageNum;

	DocDelayPageLoad( gitFileIt, dPageNum );

	PageListViewChange( dPageNum, iPrePage );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("기타 오류") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

HRESULT DocPageInfoRenew( INT dPage, UINT bMode )
{
	UINT_PTR	dLines;
	UINT		dBytes;

	if( 0 > dPage ){	dPage = gixFocusPage;	}

	dBytes = gitFileIt->vcCont.at( dPage ).dByteSz;

	if( bMode )
	{
		MainSttBarSetByteCount( dBytes );
	}

	if( gitFileIt->vcCont.at( dPage ).ptRawData )
	{
		dLines = gitFileIt->vcCont.at( dPage ).iLineCnt;
	}
	else
	{
		dLines = gitFileIt->vcCont.at( dPage ).ltPage.size( );
	}

	PageListInfoSet( dPage, dBytes, dLines );

	return S_OK;
}

INT DocLineDataGetAlloc( INT rdLine, INT iStart, LPLETTER *pstTexts, PINT pchLen, PUINT pdFlag )
{
	INT		iSize, i = 0, j, dotCnt;
	INT_PTR	iCount, iLines;

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );
	if( iLines <=  rdLine )	return -1;

	itLine = gitFileIt->vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );
	*pdFlag = 0;

	if( 0 == iCount )
	{
		*pchLen = 0;
		dotCnt  = 0;
	}
	else
	{
		if( iStart >= iCount )	return 0;

		iSize = iCount - iStart;

		*pchLen = iSize;
		iSize++;

		if( !pstTexts )	return 0;

		*pstTexts = (LPLETTER)malloc( iSize * sizeof(LETTER) );
		if( !( *pstTexts ) ){	TRACE( TEXT("malloc 오류") );	return 0;	}

		ZeroMemory( *pstTexts, iSize * sizeof(LETTER) );

		dotCnt = 0;
		for( i = iStart, j = 0; iCount > i; i++, j++ )
		{
			(*pstTexts)[j].cchMozi = itLine->vcLine.at( i ).cchMozi;
			(*pstTexts)[j].rdWidth = itLine->vcLine.at( i ).rdWidth;
			(*pstTexts)[j].mzStyle = itLine->vcLine.at( i ).mzStyle;

			dotCnt += itLine->vcLine.at( i ).rdWidth;
		}

		if( iswspace( itLine->vcLine.at( iCount-1 ).cchMozi ) )
		{	*pdFlag |= CT_LASTSP;	}
	}

	if( iLines - 1 >  rdLine )	*pdFlag |= CT_RETURN;
	else						*pdFlag |= CT_EOF;

	*pdFlag |= itLine->dStyle;

	return dotCnt;
}

INT DocPageGetAlloc( UINT bStyle, LPVOID *pText )
{
	return DocPageTextGetAlloc( gitFileIt, gixFocusPage, bStyle, pText, FALSE );
}

INT DocPageTextGetAlloc( FILES_ITR itFile, INT dPage, UINT bStyle, LPVOID *pText, BOOLEAN bCrLf )
{
	UINT_PTR	iLines, iLetters, j, iSize;
	UINT_PTR	i;
	UINT_PTR	cchSize;

	LPSTR		pcStr;

	string	srString;
	wstring	wsString;

	LINE_ITR	itLines;

#ifdef DO_TRY_CATCH
	try{
#endif

	srString.clear( );
	wsString.clear( );

	if( 0 > dPage ){	dPage = gixFocusPage;	}

	if( itFile->vcCont.at( dPage ).ptRawData )
	{
		if( bStyle & D_UNI )
		{
			wsString = wstring( itFile->vcCont.at( dPage ).ptRawData );
			if( bCrLf ){	wsString += wstring( CH_CRLFW );	}
		}
		else
		{
			pcStr = SjisEncodeAlloc( itFile->vcCont.at( dPage ).ptRawData );
			if( pcStr )
			{
				srString = string( pcStr );
				if( bCrLf ){	srString +=  string( CH_CRLFA );	}

				FREE( pcStr );
			}
		}
	}
	else
	{

		iLines = itFile->vcCont.at( dPage ).ltPage.size( );

		for( itLines = itFile->vcCont.at( dPage ).ltPage.begin(), i = 0;
		itLines != itFile->vcCont.at( dPage ).ltPage.end();
		itLines++, i++ )
		{
			iLetters = itLines->vcLine.size( );

			for( j = 0; iLetters > j; j++ )
			{
				srString +=  string( itLines->vcLine.at( j ).acSjis );
				wsString += itLines->vcLine.at( j ).cchMozi;
			}

			if( !(1 == iLines && 0 == iLetters) )
			{
				if( iLines > (i+1) )
				{
					srString +=  string( CH_CRLFA );
					wsString += wstring( CH_CRLFW );
				}
			}
		}

		if( bCrLf )
		{
			srString +=  string( CH_CRLFA );
			wsString += wstring( CH_CRLFW );
		}
	}

	if( bStyle & D_UNI )
	{
		cchSize = wsString.size(  ) + 1;
		iSize = cchSize * sizeof(TCHAR);

		if( pText )
		{
			*pText = (LPTSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopy( (LPTSTR)(*pText), cchSize, wsString.c_str( ) );
		}
	}
	else
	{
		iSize = srString.size( ) + 1;

		if( pText )
		{
			*pText = (LPSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopyA( (LPSTR)(*pText), iSize, srString.c_str( ) );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (INT)ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return (INT)ETC_MSG( ("기타 오류") , 0 );	}
#endif

	return (INT)iSize;
}

INT DocLineTextGetAlloc( FILES_ITR itFile, INT dPage, UINT bStyle, UINT dTarget, LPVOID *pText )
{
	UINT_PTR	dLines;
	UINT_PTR	dLetters, j, iSize;
	UINT_PTR	cchSize;

	string	srString;
	wstring	wsString;

	LINE_ITR	itLines;

	dLines = itFile->vcCont.at( dPage ).ltPage.size( );
	if( dLines <= dTarget ){	return 0;	}

	itLines = itFile->vcCont.at( dPage ).ltPage.begin();
	std::advance( itLines, dTarget );

	dLetters = itLines->vcLine.size( );
	for( j = 0; dLetters > j; j++ )
	{
		srString +=  string( itLines->vcLine.at( j ).acSjis );
		wsString +=  itLines->vcLine.at( j ).cchMozi;
	}

	if( bStyle & D_UNI )
	{
		cchSize = wsString.size(  ) + 1;
		iSize = cchSize * sizeof(TCHAR);

		if( pText )
		{
			*pText = (LPTSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopy( (LPTSTR)(*pText), cchSize, wsString.c_str( ) );
		}
	}
	else
	{
		iSize = srString.size( ) + 1;

		if( pText )
		{
			*pText = (LPSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopyA( (LPSTR)(*pText), iSize, srString.c_str( ) );
		}
	}
	return (INT)iSize;
}

#if 0

INT DocPageTextAllGetAlloc( UINT bStyle, LPVOID *pText )
{

	UINT_PTR	iLines, i, iLetters, j;
	UINT_PTR	cchSize;
	INT_PTR		iSize;

	LPTSTR		ptData;
	LPSTR		pcStr;

	string	srString;
	wstring	wsString;

	LINE_ITR	itLine;

	srString.clear( );
	wsString.clear( );

	if( gitFileIt->vcCont.at( gixFocusPage ).ptRawData )
	{
		ptData = (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData;
		StringCchLength( ptData, STRSAFE_MAX_CCH, &cchSize );

		if( bStyle & D_UNI )
		{
			iSize = (cchSize+1) * sizeof(TCHAR);

			if( pText )
			{
				*pText = (LPTSTR)malloc( iSize );
				ZeroMemory( *pText, iSize );
				StringCchCopy( (LPTSTR)(*pText), cchSize, ptData );
			}
		}
		else
		{
			pcStr = SjisEncodeAlloc( ptData );
			if( pcStr )
			{
				StringCchLengthA( pcStr, STRSAFE_MAX_CCH, &cchSize );
				iSize = cchSize + 1;

				if( pText ){	*pText =  pcStr;	}
				else{	FREE( pcStr );	}
			}
		}
	}
	else
	{

		iLines = DocNowFilePageLineCount( );

		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();

		for( i = 0; iLines > i; i++, itLine++ )
		{

			iLetters = itLine->vcLine.size( );

			if( bStyle & D_UNI )
			{
				for( j = 0; iLetters > j; j++ )
				{
					wsString += itLine->vcLine.at( j ).cchMozi;
				}

				if( iLines > (i+1) )	wsString += wstring( CH_CRLFW );
			}
			else
			{
				for( j = 0; iLetters > j; j++ )
				{
					srString +=  string( itLine->vcLine.at( j ).acSjis );
				}

				if( iLines > (i+1) )	srString +=  string( CH_CRLFA );
			}
		}

		if( bStyle & D_UNI )
		{
			cchSize = wsString.size(  ) + 1;
			iSize = cchSize * sizeof(TCHAR);

			if( pText )
			{
				*pText = (LPTSTR)malloc( iSize );
				ZeroMemory( *pText, iSize );
				StringCchCopy( (LPTSTR)(*pText), cchSize, wsString.c_str( ) );
			}
		}
		else
		{
			iSize = srString.size( ) + 1;

			if( pText )
			{
				*pText = (LPSTR)malloc( iSize );
				ZeroMemory( *pText, iSize );
				StringCchCopyA( (LPSTR)(*pText), iSize, srString.c_str( ) );
			}
		}
	}

	return iSize;
}

#endif

LPSTR DocPageTextPreviewAlloc( INT iPage, PINT pdBytes )
{

	UINT_PTR	iLines, i, iLetters;
	INT_PTR		iSize;
	LPSTR	pcText = NULL;
	CHAR	acEntity[10];

	string	srString;
	LINE_ITR	itLine, itLineEnd;
	LETR_ITR	itLtr;

	TCHAR	atEntity[10];
	wstring	widString;
	LPTSTR	ptCaret;

	srString.clear( );

	if( pdBytes )	*pdBytes = 0;

	if( DocRangeIsError( gitFileIt, iPage, 0 ) )	return NULL;

	widString.clear();

	if(  (*gitFileIt).vcCont.at( iPage ).ptRawData )
	{
		ptCaret = (*gitFileIt).vcCont.at( iPage ).ptRawData;
		StringCchLength( ptCaret, STRSAFE_MAX_CCH, &iLetters );

		for( i = 0; iLetters > i; i++ )
		{
			if( HtmlEntityCheckW( ptCaret[i], atEntity, 10 ) )
			{
				widString += wstring( atEntity );
			}
			else if( TEXT('\r') == ptCaret[i] )
			{
				widString += wstring( TEXT("<br>") );
				i++;
			}
			else
			{
				widString += ptCaret[i];
			}
		}

		widString += wstring( TEXT("<br>") );

		pcText = SjisEncodeAlloc( widString.c_str() );
		iSize = strlen( pcText );
	}
	else
	{

		iLines    = (*gitFileIt).vcCont.at( iPage ).ltPage.size( );

		itLine    = (*gitFileIt).vcCont.at( iPage ).ltPage.begin( );
		itLineEnd = (*gitFileIt).vcCont.at( iPage ).ltPage.end( );

		for( i = 0; itLine != itLineEnd; i++, itLine++ )
		{

			iLetters = itLine->vcLine.size( );

			for( itLtr = itLine->vcLine.begin(); itLtr != itLine->vcLine.end(); itLtr++ )
			{

				if( HtmlEntityCheckA( itLtr->cchMozi, acEntity, 10 ) )
				{
					srString +=  string( acEntity );
				}
				else
				{
					srString +=  string( itLtr->acSjis );
				}
			}

			srString +=  string( "<br>" );
		}

		iSize = srString.size( ) + 1;

		pcText = (LPSTR)malloc( iSize );
		ZeroMemory( pcText, iSize );
		StringCchCopyA( pcText, iSize, srString.c_str( ) );
	}

	if( pdBytes )	*pdBytes = iSize;

	return pcText;
}

HRESULT UnicodeRadixExchange( LPVOID pVoid )
{
	INT_PTR	iPage, iLine, iMozi, dP, dL, dM;
	TCHAR	cchMozi;
	CHAR	acSjis[10];

	LINE_ITR	itLine;

	iPage = DocNowFilePageCount(  );

	for( dP = 0; iPage >  dP; dP++ )
	{
		iLine = (*gitFileIt).vcCont.at( dP ).ltPage.size(  );

		itLine = (*gitFileIt).vcCont.at( dP ).ltPage.begin();
		for( dL = 0; iLine >  dL; dL++, itLine++ )
		{
			iMozi = itLine->vcLine.size(  );

			for( dM = 0; iMozi >  dM; dM++ )
			{
				if( itLine->vcLine.at( dM ).mzStyle & CT_CANTSJIS )
				{
					cchMozi = itLine->vcLine.at( dM ).cchMozi;
					if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10, ("&#x%X;"), cchMozi );	}
					else{					StringCchPrintfA( acSjis, 10, ("&#%d;"),  cchMozi );	}

					StringCchCopyA( itLine->vcLine.at( dM ).acSjis, 10, acSjis );

				}
			}
		}
	}

	return S_OK;
}

HRESULT DocPageDivide( HWND hWnd, HINSTANCE hInst, INT iNow )
{
	INT	iDivLine = iNow + 1;
	INT	iLines, mRslt, iNewPage;

	ONELINE	stLine;
	LINE_ITR	itLine, itEnd;

	ZeroONELINE( &stLine );

	mRslt = MessageBoxCheckBox( hWnd, hInst, 1 );
	if( IDNO == mRslt ){	return  E_ABORT;	}

	iLines = DocNowFilePageLineCount(  );

	if( iLines <= iDivLine )	return E_OUTOFMEMORY;

	iNewPage = DocPageCreate( gixFocusPage );
	PageListInsert( iNewPage  );

	(*gitFileIt).vcCont.at( iNewPage ).ltPage.clear(  );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin( );
	std::advance( itLine, iDivLine );

	itEnd  = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.end( );

	std::copy( itLine, itEnd, back_inserter( (*gitFileIt).vcCont.at( iNewPage ).ltPage ) );

	(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.erase( itLine, itEnd );

	SqnFreeAll( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog) );

	DocPageByteCount( gitFileIt, gixFocusPage, NULL, NULL );
	DocPageInfoRenew( gixFocusPage, TRUE );

	DocPageByteCount( gitFileIt, iNewPage, NULL, NULL );
	DocPageInfoRenew( iNewPage, FALSE );

	ViewRedrawSetLine( -1 );

	return S_OK;
}

HRESULT DocThreadDropCopy( VOID )
{
	CHAR	acBuf[260];
	TCHAR	atTitle[64], atInfo[256];
	INT	cbSize, maxPage;
	LPVOID	pcString = NULL;

	cbSize = DocPageTextGetAlloc( gitFileIt, gixDropPage, D_SJIS, &pcString, FALSE );

	TRACE( TEXT("%d 페이지를 복사"), gixDropPage );

	DocClipboardDataSet( pcString, cbSize, D_SJIS );

	ZeroMemory( acBuf, sizeof(acBuf) );
	StringCchCopyNA( acBuf, 260, (LPCSTR)pcString, 250 );
	ZeroMemory( atInfo, sizeof(atInfo) );
	MultiByteToWideChar( CP_ACP, 0, acBuf, (INT)strlen(acBuf), atInfo, 256 );

	StringCchPrintf( atTitle, 64, TEXT("%d 페이지를 복사했어"), gixDropPage + 1 );

	NotifyBalloonExist( atInfo, atTitle, NIIF_INFO );

	FREE( pcString );

	gixDropPage++;

	maxPage = DocNowFilePageCount(  );
	if( maxPage <= gixDropPage )	gixDropPage = 0;

	return S_OK;
}

HRESULT DocSelText2PageName( VOID )
{

	INT	cbSize;
	LPVOID	pString = NULL;
	LPTSTR	ptText;
	UINT_PTR	cchSize, d;

	if( !( IsSelecting( NULL ) ) )	return  E_ABORT;

	cbSize = DocSelectTextGetAlloc( D_UNI, &pString, NULL );
	TRACE( TEXT("BYTE:%d"), cbSize );

	ptText = (LPTSTR)pString;
	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

	for( d = 0; cchSize > d; d++ )
	{
		if( 0x0D == ptText[d] )
		{
			ptText[d] = NULL;
			break;
		}
	}

	PageListNameRewrite( ptText );

	FREE( pString );

	return S_OK;
}
