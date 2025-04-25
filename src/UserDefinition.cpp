#include "stdafx.h"
#include "OrinrinEditor.h"

typedef struct tagUSERITEMS
{
	TCHAR	atItemName[MAX_STRING];

	vector<ONELINE>	vcUnits;

} USERITEMS, *LPUSERITEMS;

extern INT		gdDocLine;

static TCHAR	gatUsDfPath[MAX_PATH];

static  UINT	gdItemCnt;

static USERITEMS	gstUserItem[USER_ITEM_MAX];

UINT	CALLBACK UserDefItemLoad( LPTSTR, LPCTSTR, INT );

HRESULT	UserDefAppendMenu( HWND );

HRESULT UserDefObliterate( HWND hWnd )
{
	UINT_PTR	i, iLine;
	UINT_PTR	j;

	for( i = 0; USER_ITEM_MAX > i; i++ )
	{
		iLine = gstUserItem[i].vcUnits.size( );
		for( j = 0; iLine > j; j++ )
		{
			gstUserItem[i].vcUnits.at( j ).vcLine.clear(   );
		}
		gstUserItem[i].vcUnits.clear(  );

		ZeroMemory( gstUserItem[i].atItemName, sizeof(gstUserItem[i].atItemName) );
	}

	gdItemCnt = 0;

	return S_OK;
}

INT UserDefInitialise( HWND hWnd, UINT bFirst )
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

	if( bFirst )
	{
		ZeroMemory( gatUsDfPath, sizeof(gatUsDfPath) );
		StringCchCopy( gatUsDfPath, MAX_PATH, ExePathGet() );
		PathAppend( gatUsDfPath, TEMPLATE_DIR );
		PathAppend( gatUsDfPath, USER_ITEM_FILE );
	}

	hFile = CreateFile( gatUsDfPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return 0;	}

	UserDefObliterate( hWnd );

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

	if( !( StrCmpN( AST_SEPARATERW, ptString, 4 ) ) )
	{
		DocStringSplitAST( ptString , cchSize, UserDefItemLoad );
	}

	FREE( pBuffer );

	UserDefAppendMenu( hWnd );

	return 1;
}

UINT CALLBACK UserDefItemLoad( LPTSTR ptName, LPCTSTR ptCont, INT cchSize )
{

	if( USER_ITEM_MAX <= gdItemCnt )	return 0;

	if( ptName )
	{
		StringCchCopy( gstUserItem[gdItemCnt].atItemName, MAX_STRING, ptName );
	}
	else
	{
		StringCchPrintf( gstUserItem[gdItemCnt].atItemName, MAX_STRING, TEXT("(ユーザアイテム No.%d"), gdItemCnt + 1 );
	}

	if( 0 < cchSize )
	{
		UserDefSetString( &(gstUserItem[gdItemCnt].vcUnits), ptCont, cchSize );
	}

	gdItemCnt++;

	return 1;
}

HRESULT UserDefSetString( vector<ONELINE> *pvcUnits, LPCTSTR ptText, UINT cchSize )
{
	UINT	i;
	INT		yLine;
	ONELINE	stLine;
	LETTER	stLetter;

	ZeroONELINE( &stLine );

	pvcUnits->push_back( stLine );

	yLine = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )
		{
			pvcUnits->push_back( stLine );

			i++;
			yLine++;
		}
		else if( CC_TAB == ptText[i] )
		{

		}
		else
		{
			DocLetterDataCheck( &stLetter, ptText[i] );

			pvcUnits->at( yLine ).vcLine.push_back( stLetter );

			pvcUnits->at( yLine ).iDotCnt += stLetter.rdWidth;
			pvcUnits->at( yLine ).iByteSz += stLetter.mzByte;
		}
	}

	return S_OK;
}

HRESULT UserDefAppendMenu( HWND hWnd )
{
	HMENU	hMenu, hSubMenu;

#pragma message("メニュー構造変わったらユーザ定義の位置であるここも変更")
	hMenu = GetMenu( hWnd );
	hSubMenu = GetSubMenu( hMenu, 2 );
	hMenu = hSubMenu;
	hSubMenu = GetSubMenu( hMenu, 10 );

	UserDefMenuWrite( hSubMenu, 1 );

	DeleteMenu( hSubMenu, IDM_USERINS_NA, MF_BYCOMMAND );

	DrawMenuBar( hWnd );

	return S_OK;
}

HRESULT UserDefMenuWrite( HMENU hMenu, UINT bMode )
{
	UINT	i;
	TCHAR	atBuffer[MAX_PATH];

	for( i = 0; gdItemCnt > i; i++ )
	{
		if( bMode )
		{
			StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s(&%c)"), gstUserItem[i].atItemName, i+'A' );
		}
		else
		{
			StringCchCopy( atBuffer, MAX_PATH, gstUserItem[i].atItemName );
		}
		AppendMenu( hMenu, MF_STRING, (IDM_USERINS_ITEM_FIRST + i), atBuffer );
	}
	return S_OK;
}

HRESULT UserDefItemNameget( UINT dNumber, LPTSTR ptNamed, UINT_PTR cchSize )
{

	if( gdItemCnt <= dNumber )	return E_OUTOFMEMORY;

	StringCchCopy( ptNamed, cchSize, gstUserItem[dNumber].atItemName );

	return S_OK;
}

LPTSTR UserDefTextLineAlloc( UINT idNum, INT uLine )
{
	INT_PTR	iLines, iLetters, i, cchSz;
	LPTSTR	ptText;
	LETR_ITR	itLetter;

	if( gdItemCnt <= idNum )	return NULL;

	iLines = gstUserItem[idNum].vcUnits.size( );
	if( iLines <= uLine )	return NULL;

	iLetters = gstUserItem[idNum].vcUnits.at( uLine ).vcLine.size( );

	cchSz = iLetters + 1;

	ptText = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
	ZeroMemory( ptText, cchSz * sizeof(TCHAR) );

	for( itLetter = gstUserItem[idNum].vcUnits.at( uLine ).vcLine.begin(), i = 0;
		itLetter != gstUserItem[idNum].vcUnits.at( uLine ).vcLine.end(); i++, itLetter++ )
	{	ptText[i] = itLetter->cchMozi;	}

	ptText[i] = NULL;

	return ptText;
}

HRESULT UserDefItemInsert( HWND hWnd, UINT idNum )
{
	INT		iLines, yLine, iMinus, i, dmyDot;
	INT_PTR	dNeedLine;
	LPTSTR	ptText;
	BOOLEAN	bFirst = TRUE;

	if( gdItemCnt <= idNum )	return E_OUTOFMEMORY;

	yLine = gdDocLine;

	dNeedLine = gstUserItem[idNum].vcUnits.size( );

	iLines = DocPageParamGet( NULL , NULL );

	if( iLines < (dNeedLine + yLine) )
	{
		iMinus = (dNeedLine + yLine) - iLines;

		DocAdditionalLine( iMinus, &bFirst );

		iLines = DocPageParamGet( NULL , NULL );
	}

	for( i = 0; dNeedLine > i; i++, yLine++ )
	{
		ptText = UserDefTextLineAlloc( idNum, i );

		dmyDot = 0;
		DocInsertString( &dmyDot, &yLine, NULL, ptText, 0, bFirst );	bFirst = FALSE;

		FREE(ptText);
	}

	return S_OK;
}
