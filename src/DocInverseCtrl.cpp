#include "stdafx.h"
#include "OrinrinEditor.h"

#define IV_MIRROR	1
#define IV_UPSET	0

typedef struct tagINVERSEPARTS
{
	TCHAR	atSrcStr[MIN_STRING];
	TCHAR	atDestStr[MIN_STRING];

} INVERSEPARTS, *LPINVERSEPARTS;

extern list<ONEFILE>	gltMultiFiles;
extern FILES_ITR	gitFileIt;
extern INT			gixFocusPage;

static  vector<INVERSEPARTS>	gvcMirrorParts;
static  vector<INVERSEPARTS>	gvcUpsetParts;
typedef vector<INVERSEPARTS>::iterator	PARTS_ITR;

HRESULT	DocMirrorTranceLine( INT, INT );
HRESULT	DocMirrorTranceBox( INT, INT );

HRESULT	DocUpsetTranceLine( INT, INT );
HRESULT	DocUpsetTranceBox( INT, INT );

LPTSTR	SeledTextAlloc( LINE_ITR, PINT, PINT );

HRESULT	InversePartsLoad( UINT );
UINT	InversePartsCheck( UINT, LPCTSTR, LPTSTR, UINT_PTR );

HRESULT DocInverseInit( UINT dMode )
{

	if( dMode )
	{

	}
	else
	{
		gvcMirrorParts.clear( );
		gvcUpsetParts.clear(  );
	}

	return S_OK;
}

#define INV_ITEMS	3

HRESULT InversePartsLoad( UINT dMode )
{
	CONST WCHAR rtHead = 0xFEFF;
	WCHAR	rtUniBuf;

	HANDLE	hFile;
	DWORD	readed;

	LPVOID	pBuffer;
	INT		iByteSize;

	LPTSTR	ptString;
	LPSTR	pcText;
	UINT	cchSize, cchLen;
	TCHAR	atFileName[MAX_PATH];

	TCHAR	atBuff[INV_ITEMS][MIN_STRING];
	INT		nYct, nXct;
	UINT	caret, dItem;

	INVERSEPARTS	stData;

	UINT_PTR	loop;
	list<INVERSEPARTS>	ltParts;
	list<INVERSEPARTS>::iterator	itParts, itPtPos;

	StringCchCopy( atFileName, MAX_PATH, ExePathGet( ) );
	PathAppend( atFileName, TEMPLATE_DIR );

	if( dMode  )
	{
		if( 1 <= gvcMirrorParts.size( ) )	return S_FALSE;

		PathAppend( atFileName, AA_MIRROR_FILE );
	}
	else
	{
		if( 1 <= gvcUpsetParts.size(  ) )	return S_FALSE;

		PathAppend( atFileName, AA_UPSET_FILE );
	}

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

	ZeroMemory( atBuff, sizeof(atBuff) );

	dItem = 0;
	for( nYct = 0, nXct = 0, caret = 0; caret <= cchSize; caret++, nXct++ )
	{
		if( MIN_STRING <= nXct ){	nXct = MIN_STRING - 1;	}
		if( INV_ITEMS  <= nYct ){	nYct = INV_ITEMS - 1;	}

		atBuff[nYct][nXct] = ptString[caret];

		if( (TEXT('\r') == ptString[caret] && TEXT('\n') == ptString[caret + 1]) || 0x0000 == ptString[caret] )
		{

			atBuff[nYct][nXct] = 0x0000;
			nXct = -1;
			nYct = 0;
			caret++;

			if( (0x0000 != atBuff[0][0]) && (0 != dItem) )
			{

				ZeroMemory( &stData, sizeof(INVERSEPARTS) );
				StringCchCopy( stData.atSrcStr,  MIN_STRING, atBuff[0] );
				StringCchCopy( stData.atDestStr, MIN_STRING, atBuff[1] );
				ltParts.push_back( stData );

				ZeroMemory( &stData, sizeof(INVERSEPARTS) );
				StringCchCopy( stData.atSrcStr,  MIN_STRING, atBuff[1] );
				StringCchCopy( stData.atDestStr, MIN_STRING, atBuff[0] );
				ltParts.push_back( stData );

			}

			dItem++;
			ZeroMemory( atBuff , sizeof(atBuff) );
		}

		if( TEXT('　') == ptString[caret] )
		{
			atBuff[nYct][nXct] = 0x0000;
			nXct = -1;
			nYct++;
		}
	}

	FREE( pBuffer );

	cchSize = 0;

	loop = ltParts.size();
	while( loop )
	{
		itParts = ltParts.begin();
		StringCchLength( itParts->atSrcStr, MIN_STRING , &cchSize );

		for( itPtPos = ltParts.begin(); ltParts.end() != itPtPos; itPtPos++ )
		{
			StringCchLength( itPtPos->atSrcStr, MIN_STRING, &cchLen );
			if( cchSize <  cchLen ){	itParts = itPtPos;	}
		}

		ZeroMemory( &stData, sizeof(INVERSEPARTS) );
		StringCchCopy( stData.atSrcStr,  MIN_STRING, itParts->atSrcStr );
		StringCchCopy( stData.atDestStr, MIN_STRING, itParts->atDestStr );
		if( dMode ){	gvcMirrorParts.push_back( stData );	}
		else{			gvcUpsetParts.push_back( stData );	}

		ltParts.erase( itParts );

		loop = ltParts.size();
	}

	return S_OK;
}

LPTSTR SeledTextAlloc( LINE_ITR itLine, PINT piDot, PINT piMozi )
{
	UINT_PTR	j, dLetters;
	INT			iDot, iMozi;
	INT			iSelDot;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz;

	wstring	wsSrcBuff;

	dLetters = itLine->vcLine.size( );

	wsSrcBuff.clear();
	iSelDot = -1;
	iDot = 0;
	iMozi = 0;

	for( j = 0; dLetters > j; j++ )
	{

		if( CT_SELECT & itLine->vcLine.at( j ).mzStyle )
		{
			wsSrcBuff += itLine->vcLine.at( j ).cchMozi;

			iMozi++;
			if( 0 > iSelDot )	iSelDot = iDot;
		}

		iDot +=  itLine->vcLine.at( j ).rdWidth;
	}

	cchSz = wsSrcBuff.size();
	ptString = (LPTSTR)malloc( (cchSz+2) * sizeof(TCHAR) );
	if( ptString )	StringCchCopy( ptString, (cchSz+2), wsSrcBuff.c_str() );

	if( 0 > iSelDot )	iSelDot = 0;

	if( piDot  )	*piDot  = iSelDot;
	if( piMozi )	*piMozi = iMozi;

	return ptString;
}

HRESULT DocInverseTransform( UINT dStyle, UINT dMode, PINT pXdot, INT dLine )
{
	INT_PTR	iLines;
	INT		iTop, iBtm, iInX;

#ifdef DO_TRY_CATCH
	try{
#endif

	iLines = DocNowFilePageLineCount( );

	iTop = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBtm = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 >  iTop ){	iTop = 0;	}
	if( 0 >  iBtm ){	iBtm = iLines - 1;	}

	iInX = DocLineParamGet( iBtm, NULL, NULL );
	if( 0 == iInX ){	 iBtm--;	}

	if( iLines <= iTop || iLines <= iBtm )	return E_OUTOFMEMORY;

	if( dStyle & D_SQUARE )
	{
		if( dMode ){	DocMirrorTranceBox( iTop, iBtm );	}
		else{			DocUpsetTranceBox( iTop, iBtm );	}
	}
	else
	{
		if( dMode ){	DocMirrorTranceLine( iTop, iBtm );	}
		else{			DocUpsetTranceLine( iTop, iBtm );	}
	}
	ViewSelPageAll( -1 );

	DocLetterPosGetAdjust( pXdot, dLine, 0 );
	ViewDrawCaret( *pXdot, dLine, 1 );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

HRESULT DocMirrorTranceLine( INT iTop, INT iBtm )
{
	INT_PTR		iLns;
	INT			iPadd, baseDot, iBytes;
	INT			iDot, iGyou, iMzDot;
	LPTSTR		ptPadd;
	LPTSTR		ptInvStr;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz;
	UINT		d;
	TCHAR		atBuff[MIN_STRING];

	BOOLEAN		bFirst = TRUE;

	wstring	wsInvBuff;

	LINE_ITR	itLine;

#ifdef DO_TRY_CATCH
	try{
#endif

	InversePartsLoad( IV_MIRROR );

	baseDot = DocPageMaxDotGet( iTop, iBtm );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	for( iLns = iTop; iBtm >= iLns; iLns++ )
	{
		wsInvBuff.clear(  );

		iMzDot = DocLineParamGet( iLns, NULL, NULL );
		if( 0 >= iMzDot )	continue;

		iPadd = baseDot - iMzDot;
		ptPadd = DocPaddingSpaceMake( iPadd );

		iBytes = DocLineTextGetAlloc( gitFileIt, gixFocusPage, D_UNI, iLns, (LPVOID *)(&ptString) );
		if( 0 < iBytes )
		{
			StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSz );
			for( d = 0; cchSz > d; )
			{
				d += InversePartsCheck( IV_MIRROR, &(ptString[d]), atBuff, MIN_STRING );
				wsInvBuff.insert( 0, atBuff );
			}
		}
		FREE( ptString );

		cchSz = wsInvBuff.size() + 2;
		ptInvStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
		StringCchCopy( ptInvStr, cchSz, wsInvBuff.c_str() );

		DocLineErase( iLns, &bFirst );
		iDot = 0;	iGyou = iLns;
		if( ptPadd ){	DocInsertString( &iDot, &iGyou, NULL, ptPadd, 0, bFirst );	bFirst  = FALSE;	}
		DocInsertString( &iDot, &iGyou, NULL, ptInvStr, 0, bFirst );	bFirst  = FALSE;

		FREE( ptPadd );
		FREE( ptInvStr );
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

HRESULT DocMirrorTranceBox( INT iTop, INT iBtm )
{
	INT_PTR		iLns;
	INT			iGyou;
	INT			iSelDot, iMozi;
	LPTSTR		ptInvStr;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz;
	UINT		d;
	TCHAR		atBuff[MIN_STRING];

	BOOLEAN		bFirst = TRUE;

	wstring	wsInvBuff, wsSrcBuff;

	LINE_ITR	itLine;

#ifdef DO_TRY_CATCH
	try{
#endif

	InversePartsLoad( IV_MIRROR );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	for( iLns = iTop; iBtm >= iLns; iLns++ )
	{
		wsInvBuff.clear(  );

		ptString = SeledTextAlloc( itLine, &iSelDot, &iMozi );	itLine++;
		StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSz );

		if( 0 <  cchSz )
		{
			for( d = 0; cchSz > d; )
			{
				d += InversePartsCheck( IV_MIRROR, &(ptString[d]), atBuff, MIN_STRING );
				wsInvBuff.insert( 0, atBuff );
			}

			DocStringErase( iSelDot, iLns, NULL, iMozi );
			SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog) , DO_DELETE, ptString, iSelDot, iLns, bFirst );	bFirst = FALSE;
			FREE( ptString );

			cchSz = wsInvBuff.size() + 2;
			ptInvStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
			StringCchCopy( ptInvStr, cchSz, wsInvBuff.c_str() );

			iGyou = iLns;
			DocInsertString( &iSelDot, &iGyou, NULL, ptInvStr, 0, FALSE );

			FREE( ptInvStr );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

HRESULT DocUpsetTranceLine( INT iTop, INT iBtm )
{

	INT_PTR		iLns;
	INT			iBytes;
	INT			iDot, iGyou;
	LPTSTR		ptInvStr;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz, d, dL;
	TCHAR		atBuff[MIN_STRING];

	BOOLEAN		bFirst = TRUE;

	LINE_ITR	itLine;

	wstring	wsInvBuff;
	vector<wstring>	vcUpset;

#ifdef DO_TRY_CATCH
	try{
#endif

	InversePartsLoad( IV_UPSET );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	iLns = (iBtm - iTop) + 1;
	vcUpset.resize( iLns  );

	for( iLns = iTop, dL = 0; iBtm >= iLns; iLns++, dL++ )
	{
		vcUpset.at( dL ).clear( );

		iBytes = DocLineTextGetAlloc( gitFileIt, gixFocusPage, D_UNI, iLns, (LPVOID *)(&ptString) );
		if( 0 < iBytes )
		{

			StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSz );
			for( d = 0; cchSz > d; )
			{
				d += InversePartsCheck( IV_UPSET, &(ptString[d]), atBuff, MIN_STRING );
				vcUpset.at( dL ) += wstring(atBuff);

			}
		}
		FREE( ptString );
	}

	for( iLns = iTop, dL = vcUpset.size()-1; iBtm >= iLns; iLns++, dL-- )
	{
		DocLineErase( iLns, &bFirst );

		cchSz = vcUpset.at( dL ).size( );
		if( 0 < cchSz )
		{
			cchSz += 2;
			ptInvStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
			StringCchCopy( ptInvStr, cchSz, vcUpset.at( dL ).c_str( ) );

			iDot = 0;	iGyou = iLns;
			DocInsertString( &iDot, &iGyou, NULL, ptInvStr, 0, bFirst );	bFirst  = FALSE;

			FREE( ptInvStr );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

HRESULT DocUpsetTranceBox( INT iTop, INT iBtm )
{
	INT_PTR		iLns;
	INT			iGyou;
	INT			iSelDot, iMozi;
	LPTSTR		ptInvStr;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz, d, dL;
	TCHAR		atBuff[MIN_STRING];

	BOOLEAN		bFirst = TRUE;

	LINE_ITR	itLine, itStart;

	vector<wstring>	vcUpset;

#ifdef DO_TRY_CATCH
	try{
#endif
	InversePartsLoad( IV_UPSET );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );
	itStart = itLine;

	iLns = (iBtm - iTop) + 1;
	vcUpset.resize( iLns  );

	for( iLns = iTop, dL = 0; iBtm >= iLns; iLns++, dL++ )
	{
		vcUpset.at( dL ).clear( );

		ptString = SeledTextAlloc( itLine, &iSelDot, &iMozi );	itLine++;
		StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSz );

		if( 0 <  cchSz )
		{

			for( d = 0; cchSz > d; )
			{
				d += InversePartsCheck( IV_UPSET, &(ptString[d]), atBuff, MIN_STRING );
				vcUpset.at( dL ) += wstring(atBuff);

			}
		}
		FREE( ptString );
	}

	itLine = itStart;
	for( iLns = iTop, dL = vcUpset.size()-1; iBtm >= iLns; iLns++, dL-- )
	{

		ptString = SeledTextAlloc( itLine, &iSelDot, &iMozi );	itLine++;
		if( 0 != iMozi )
		{

			DocStringErase( iSelDot, iLns, NULL, iMozi );
			SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog) , DO_DELETE, ptString, iSelDot, iLns, bFirst );	bFirst = FALSE;
			FREE( ptString );
		}

		cchSz = vcUpset.at( dL ).size() + 2;
		ptInvStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
		StringCchCopy( ptInvStr, cchSz, vcUpset.at( dL ).c_str() );

		if( 0 == iMozi ){	iSelDot = DocLineParamGet( iLns, NULL, NULL );	}

		iGyou = iLns;
		DocInsertString( &iSelDot, &iGyou, NULL, ptInvStr, 0 , bFirst );	bFirst = FALSE;

		FREE( ptInvStr );
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

UINT InversePartsCheck( UINT dMode, LPCTSTR ptSource, LPTSTR ptOutput, UINT_PTR cchSz )
{
	UINT_PTR	dParts;
	UINT_PTR	cchPrt = 1;
	PARTS_ITR	itParts, itBegin, itEnd;

	if( dMode )
	{
		dParts  = gvcMirrorParts.size();
		itBegin = gvcMirrorParts.begin();
		itEnd   = gvcMirrorParts.end();
	}
	else
	{
		dParts  = gvcUpsetParts.size();
		itBegin = gvcUpsetParts.begin();
		itEnd   = gvcUpsetParts.end();
	}

	ZeroMemory( ptOutput, cchSz * sizeof(TCHAR) );

	ptOutput[0] = ptSource[0];

	if( 0 == dParts ){	return 1;	}

	for( itParts = itBegin; itEnd != itParts; itParts++ )
	{
		StringCchLength( itParts->atSrcStr, MIN_STRING, &cchPrt );

		if( 0 == StrCmpN( ptSource, itParts->atSrcStr, cchPrt ) )
		{
			StringCchCopy( ptOutput, cchSz, itParts->atDestStr );
			break;
		}

	}

	return cchPrt;
}
