#include "stdafx.h"
#include "OrinrinEditor.h"

typedef struct tagPAGENUMINFO
{
	UINT	dStartNum;
	UINT	bInUnder;
	UINT	bOverride;
	TCHAR	atStyle[MAX_PATH];

} PAGENUMINFO, *LPPAGENUMINFO;

extern FILES_ITR	gitFileIt;

extern INT		gixFocusPage;

extern  UINT	gbUniPad;
extern  UINT	gbUniRadixHex;

extern  UINT	gdRightRuler;

HRESULT	DocInputReturn( INT, INT );
INT		DocSquareAddPreMod( INT, INT, INT, BOOLEAN );
INT		DocLetterErase( INT, INT, INT );

HRESULT	DocDelayPageNumInsert( FILES_ITR, INT, LPPAGENUMINFO, LPCTSTR );

BOOLEAN DocIsSjisTrance( TCHAR cchMozi, LPSTR pcSjis )
{
	TCHAR	atMozi[2];
	CHAR	acSjis[10];
	BOOL	bCant = FALSE;
	INT		iRslt;

	assert( pcSjis );

	atMozi[0] = cchMozi;	atMozi[1] = 0;
	acSjis[0] = 0;	acSjis[1] = 0;	acSjis[2] = 0;

	iRslt = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atMozi, 1, acSjis, 10, "?", &bCant );

	if( bCant )
	{

		if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10, ("&#x%X;"), cchMozi );	}
		else{					StringCchPrintfA( acSjis, 10, ("&#%d;"),  cchMozi );	}
	}

#ifdef SPMOZI_ENCODE
	if( IsSpMozi( cchMozi ) )
	{
		if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10, ("&#x%X;"), cchMozi );	}
		else{					StringCchPrintfA( acSjis, 10, ("&#%d;"),  cchMozi );	}

		bCant = TRUE;
	}
#endif

	StringCchCopyA( pcSjis, 10, acSjis );

	return bCant ? FALSE : TRUE;
}

INT_PTR DocLetterByteCheck( LPLETTER pstLet )
{
	pstLet->mzByte = strlen( pstLet->acSjis );

	if( pstLet->mzStyle & CT_CANTSJIS ){	pstLet->mzByte += 4;	}

	if( 1 == pstLet->mzByte )
	{

		if( 0xA1 <= (BYTE)(pstLet->acSjis[0]) && (BYTE)(pstLet->acSjis[0]) <= 0xDF ){	pstLet->mzByte =  2;	}

		else if( '"' == pstLet->acSjis[0] ){	pstLet->mzByte = strlen( ("&quot;") );	}
		else if( '<' == pstLet->acSjis[0] ){	pstLet->mzByte = strlen( ("&lt;") );	}
		else if( '>' == pstLet->acSjis[0] ){	pstLet->mzByte = strlen( ("&gt;") );	}
		else if( '&' == pstLet->acSjis[0] ){	pstLet->mzByte = strlen( ("&amp;") );	}
	}

	return pstLet->mzByte;
}

INT_PTR DocLetterDataCheck( LPLETTER pstLttr, TCHAR ch )
{
	INT_PTR	iByte;
	LETTER	stTemp;

	if( !(pstLttr) ){	pstLttr = &stTemp;	}

	ZeroMemory( pstLttr, sizeof(LETTER) );
	pstLttr->cchMozi = ch;
	pstLttr->rdWidth = ViewLetterWidthGet( ch );
	pstLttr->mzStyle = CT_NORMAL;
	if( iswspace( ch ) ){	pstLttr->mzStyle |= CT_SPACE;	}
	if( !( DocIsSjisTrance( ch, pstLttr->acSjis ) ) ){	pstLttr->mzStyle |= CT_CANTSJIS;	}

	iByte = DocLetterByteCheck( pstLttr  );

	return iByte;
}

HRESULT DocCrLfAdd( INT xDot, INT yLine, BOOLEAN bFirst )
{
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, CH_CRLFW, xDot, yLine, bFirst );

	return DocInputReturn( xDot , yLine );
}

HRESULT DocInputReturn( INT nowDot, INT rdLine )
{
	INT_PTR	iLetter, iLines, iCount;
	ONELINE	stLine;

	LETR_ITR	vcLtrItr, vcLtrEnd;

	LINE_ITR	itLine, ltLineItr;

#ifdef DO_TRY_CATCH
	try{
#endif

	iLines = DocNowFilePageLineCount( );

	if( iLines <= rdLine )	return E_OUTOFMEMORY;

	ZeroONELINE( &stLine );

	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );

	if( iLetter < iCount )
	{
		ltLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin( );
		std::advance( ltLineItr, (rdLine+1) );

		(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.insert( ltLineItr, stLine );

		ltLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin( );
		std::advance( ltLineItr, (rdLine+1) );

		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, rdLine );

		vcLtrItr  = itLine->vcLine.begin( );
		vcLtrItr += iLetter;
		vcLtrEnd  = itLine->vcLine.end( );

		std::copy( vcLtrItr, vcLtrEnd, back_inserter(ltLineItr->vcLine) );

		itLine->vcLine.erase( vcLtrItr, vcLtrEnd );

		DocLineParamGet( rdLine,   NULL, NULL );
		DocLineParamGet( rdLine+1, NULL, NULL );
	}
	else
	{
		if( (iLines - 1) == rdLine )
		{
			(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.push_back( stLine );
		}
		else
		{
			ltLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin( );
			std::advance( ltLineItr, (rdLine+1) );

			(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.insert( ltLineItr, stLine );
		}
	}

	DocBadSpaceCheck( rdLine   );
	DocBadSpaceCheck( rdLine+1 );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

INT DocInputBkSpace( PINT pdDot, PINT pdLine )
{
	INT_PTR	iLines;
	INT		iLetter, width = 0, neDot, bCrLf = 0;
	INT		dLine = *pdLine;
	TCHAR	ch;

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );

	if( iLines <=  dLine )	return 0;

	iLetter = DocLetterPosGetAdjust( pdDot, dLine, 0 );
	neDot = *pdDot;

	if( 0 == iLetter && 0 == dLine )	return 0;

	if( 0 != iLetter )
	{
		iLetter--;
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, dLine );

		width = itLine->vcLine.at( iLetter ).rdWidth;
		ch    = itLine->vcLine.at( iLetter ).cchMozi;

		*pdDot = neDot - width;
		bCrLf = 0;

		SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, *pdDot, dLine, TRUE );
	}
	else
	{
		dLine--;	*pdLine = dLine;
		neDot = DocLineParamGet( dLine, &iLetter, NULL );
		*pdDot = neDot;
		bCrLf = 1;

		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, CH_CRLFW, *pdDot, dLine, TRUE );
	}

	DocLetterErase( *pdDot, dLine, iLetter );
	DocBadSpaceCheck( dLine );

	return bCrLf;
}

INT DocInputDelete( INT xDot, INT yLine )
{
	INT_PTR	iLines;
	INT		iCount, iLetter, iCrLf;
	TCHAR	ch;

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );
	if( iLines <= yLine )	return 0;

	iLetter = DocLetterPosGetAdjust( &xDot, yLine, 0 );

	DocLineParamGet( yLine, &iCount, NULL );

	if( iCount <= iLetter )
	{
		if( iLines <= (yLine+1) )	return 0;
		ch = CC_LF;
	}
	else
	{
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, yLine );

		ch = itLine->vcLine.at( iLetter ).cchMozi;
	}

	iCrLf = DocLetterErase( xDot, yLine, iLetter );
	if( 0 > iCrLf ){	return -1;	}

	DocBadSpaceCheck( yLine );

	if( 0 < iCrLf )
	{
		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, CH_CRLFW, xDot, yLine, TRUE );
	}
	else
	{
		SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, xDot, yLine, TRUE );
	}

	return iCrLf;
}

INT DocLetterErase( INT xDot, INT yLine, INT iLetter )
{
	INT		iCount, iRslt;

	LETR_ITR	vcLtrItr;
	LINE_ITR	itLine;

	iRslt = DocLineParamGet( yLine, &iCount, NULL );
	if( 0 > iRslt ){	return -1;	}

	if( iCount > iLetter )
	{
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, yLine );

		vcLtrItr  = itLine->vcLine.begin( );
		vcLtrItr += iLetter;

		DocIterateDelete( vcLtrItr, yLine );
		return 0;
	}
	else
	{
		DocLineCombine( yLine );
		return 1;
	}

}

HRESULT DocLineErase( INT yLine, PBOOLEAN pFirst )
{
	INT		dLines, iMozis, i;
	INT_PTR	cbSize, cchSize;
	LPTSTR	ptBuffer;
	wstring	wsString;
	LINE_ITR	itLine;

	wsString.clear( );

	dLines = DocNowFilePageLineCount(  );
	if( dLines <= yLine )	return E_OUTOFMEMORY;

	DocLineParamGet( yLine, &iMozis, NULL );

	if( 0 >= iMozis )	return  E_ABORT;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, yLine );

	for( i = 0; iMozis > i; i++ )
	{
		wsString += itLine->vcLine.at( i ).cchMozi;
	}

	cchSize = wsString.size(  ) + 1;
	cbSize = cchSize * sizeof(TCHAR);

	ptBuffer = (LPTSTR)malloc( cbSize );
	ZeroMemory( ptBuffer, cbSize );
	StringCchCopy( ptBuffer, cchSize, wsString.c_str(  ) );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, 0, yLine, *pFirst );
	*pFirst = FALSE;

	itLine->vcLine.clear();

	DocLineParamGet( yLine, NULL, NULL );
	DocPageParamGet( NULL, NULL );

	DocBadSpaceCheck( yLine );
	ViewRedrawSetLine( yLine );

	return S_OK;
}

INT DocIterateDelete( LETR_ITR itLtr, INT dBsLine )
{
	INT	width = 0, bySz;
	LINE_ITR	itLine;

	width = itLtr->rdWidth;
	bySz  = itLtr->mzByte;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, dBsLine );

	itLine->vcLine.erase( itLtr );

	itLine->iDotCnt -= width;
	itLine->iByteSz -= bySz;

	(*gitFileIt).vcCont.at( gixFocusPage ).dByteSz -= bySz;

	return width;
}

HRESULT DocLineCombine( INT dBsLine )
{
	LETR_ITR	vcLtrItr, vcLtrNxItr, vcLtrNxEnd;

	LINE_ITR	itLine, itLineNx, ltLineItr;

	itLineNx = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLineNx, dBsLine+1 );

	if( itLineNx == (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.end() )	return E_ACCESSDENIED;

	vcLtrNxItr = itLineNx->vcLine.begin(  );
	vcLtrNxEnd = itLineNx->vcLine.end(  );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, dBsLine );
	std::copy( vcLtrNxItr, vcLtrNxEnd, back_inserter( itLine->vcLine ) );

	DocLineParamGet( dBsLine , NULL, NULL );

	ltLineItr  = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin(  );
	std::advance( ltLineItr, dBsLine+1 );

	(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.erase( ltLineItr );

	DocBadSpaceCheck( dBsLine );

	return S_OK;
}

INT DocInsertLetter( PINT pxDot, INT yLine, TCHAR ch )
{
	INT	width;

	SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ch, *pxDot, yLine, TRUE );

	width = DocInputLetter( *pxDot, yLine, ch );
	*pxDot += width;

	DocBadSpaceCheck( yLine );

	return width;
}

INT DocInputLetter( INT nowDot, INT rdLine, TCHAR ch )
{
	INT_PTR	iLetter, iCount, iLines;
	LETTER	stLetter;
	LETR_ITR	vcItr;
	LINE_ITR	itLine;

#ifdef DO_TRY_CATCH
	try{
#endif

	if( 0 == ch )
	{
		TRACE( TEXT("NULL文字が入った") );
		return 0;
	}

	iLines = DocNowFilePageLineCount( );

	if( iLines <= rdLine )
	{
		TRACE( TEXT("OutOfRange 指定[%d] 行数[%d]"), rdLine, iLines );
		return 0;
	}

	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );

	DocLetterDataCheck( &stLetter, ch );

	if( iLetter >=  iCount )
	{
		itLine->vcLine.push_back( stLetter );
	}
	else
	{
		vcItr = itLine->vcLine.begin( );
		vcItr += iLetter;
		itLine->vcLine.insert( vcItr, stLetter );
	}

	itLine->iDotCnt += stLetter.rdWidth;
	itLine->iByteSz += stLetter.mzByte;

	(*gitFileIt).vcCont.at( gixFocusPage ).dByteSz += stLetter.mzByte;

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return stLetter.rdWidth;
}

INT DocStringErase( INT xDot, INT yLine, LPTSTR ptDummy, INT cchSize )
{
	INT	i, iCrLf, iLetter, rdCnt;

	iLetter = DocLetterPosGetAdjust( &xDot, yLine, 0 );

	rdCnt = 0;
	for( i = 0; cchSize > i; i++ )
	{
		iCrLf = DocLetterErase( xDot, yLine, iLetter );
		if( 0 >  iCrLf )	break;
		if( iCrLf ){	i++;	rdCnt++;	}
	}

	DocBadSpaceCheck( yLine );

	return rdCnt;
}

INT DocStringAdd( PINT pNowDot, PINT pdLine, LPCTSTR ptStr, INT cchSize )
{
	INT	i, insDot, dLn, dCrLf;

	assert( ptStr );

	dCrLf  = 0;
	dLn    = *pdLine;
	insDot = *pNowDot;

#ifdef DO_TRY_CATCH
	try{
#endif
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptStr[i] && CC_LF == ptStr[i+1] )
		{
			DocInputReturn( insDot, *pdLine );
			i++;
			(*pdLine)++;
			insDot =  0;
			dCrLf++;
		}
		else if( CC_TAB == ptStr[i] )
		{

		}
		else
		{
			insDot += DocInputLetter( insDot, *pdLine, ptStr[i] );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (INT)ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return (INT)ETC_MSG( ("etc error"), 0 );	}
#endif

#ifdef DO_TRY_CATCH
	try{
#endif

	for( i = dLn; *pdLine >= i; i++ )
	{
		DocBadSpaceCheck( i );
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (INT)ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return (INT)ETC_MSG( ("etc error"), 0 );	}
#endif

	*pNowDot = insDot;

	return dCrLf;
}

INT DocSquareAdd( PINT pNowDot, PINT pdLine, LPCTSTR ptStr, INT cchSize, LPPOINT *ppstPt )
{
	LPCTSTR		ptCaret, ptSprt;
	UINT_PTR	cchMozi;
	INT			dCrLf;
	INT			dBaseDot, dBaseLine;

	LPPOINT	pstBuf;

	dCrLf = 0;

	ptCaret = ptStr;
	dBaseLine = *pdLine;

	do
	{
		dBaseDot  = *pNowDot;
		DocLetterPosGetAdjust( &dBaseDot, dBaseLine, 0 );

		ptSprt = StrStr( ptCaret, CH_CRLFW );
		if( !(ptSprt) ){	ptSprt = ptStr + cchSize;	}

		cchMozi = ptSprt - ptCaret;

		pstBuf = (LPPOINT)realloc( *ppstPt, ( sizeof(POINT) * (dCrLf+1) ) );

		if( pstBuf ){	*ppstPt = pstBuf;	}
		else{	TRACE( TEXT("fatal mem error") );	return 0;	}
		pstBuf += dCrLf;

		pstBuf->x = dBaseDot;
		pstBuf->y = dBaseLine;
		DocStringAdd( &dBaseDot, &dBaseLine, ptCaret, cchMozi );

		ptCaret = NextLineW( ptSprt );
		if( *ptCaret  ){	dBaseLine++;	}

		dCrLf++;

	}while( *ptCaret  );

	*pdLine  = dBaseLine;
	*pNowDot = dBaseDot;

	return dCrLf;
}

INT DocAdditionalLine( INT addLine, PBOOLEAN pFirst )
{
	UINT_PTR	iLines;
	INT			cbSize, cchMozi, i;
	INT			dBaseDot, dBaseLine;
	LPTSTR		ptBuffer = NULL;

	iLines = DocNowFilePageLineCount( );

	dBaseLine = iLines - 1;

	cchMozi = CH_CRLF_CCH * addLine;
	cbSize  = (cchMozi + 1) * sizeof(TCHAR);
	ptBuffer = (LPTSTR)malloc( cbSize );

	ZeroMemory( ptBuffer, cbSize );
	for( i = 0; addLine >  i; i++ )
	{
		StringCchCat( ptBuffer, cchMozi + 1, CH_CRLFW );
		ViewRedrawSetLine( dBaseLine + i  );
	}

	dBaseDot = DocLineParamGet( dBaseLine, NULL, NULL );
	SqnAppendString( &(gitFileIt->vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptBuffer, dBaseDot, dBaseLine, *pFirst );
	DocStringAdd( &dBaseDot, &dBaseLine, ptBuffer, cchMozi );

	FREE( ptBuffer );

	*pFirst = FALSE;

	return iLines;
}

INT DocSquareAddPreMod( INT xDot, INT yLine, INT dNeedLine, BOOLEAN bFirst )
{

	INT_PTR	iLines;
	INT		iBaseDot, iBaseLine, iMinus, i;
	UINT	cchBuf;
	LPTSTR	ptBuffer = NULL;

	iLines = DocNowFilePageLineCount( );

	if( iLines < (dNeedLine + yLine) )
	{
		iMinus = (dNeedLine + yLine) - iLines;

		DocAdditionalLine( iMinus, &bFirst );

		iLines = DocNowFilePageLineCount( );
	}

	for( i = 0; dNeedLine > i; i++ )
	{
		iBaseLine = yLine + i;
		iBaseDot  = DocLineParamGet( iBaseLine, NULL, NULL );

		iMinus    = xDot - iBaseDot;

		if( gbUniPad  ){	if( 0 >= iMinus )	continue;	}
		else{	if( 3 >= iMinus )	continue;	}

		ptBuffer = DocPaddingSpaceWithPeriod( iMinus, NULL, NULL, NULL, FALSE );
		if( !ptBuffer )
		{	ptBuffer = DocPaddingSpaceWithGap( iMinus, NULL, NULL );	}
		if( !ptBuffer ){	continue;	}
		StringCchLength( ptBuffer, STRSAFE_MAX_CCH, &cchBuf );

		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptBuffer, iBaseDot, iBaseLine, bFirst );
		bFirst = FALSE;
		DocStringAdd( &iBaseDot, &iBaseLine, ptBuffer, cchBuf );

		FREE( ptBuffer );
	}

	return bFirst;
}

INT DocInsertString( PINT pNowDot, PINT pdLine, PINT pdMozi, LPCTSTR ptText, UINT dStyle, BOOLEAN bFirst )
{
	INT		dBaseDot, dBaseLine, dNeedLine;
	INT		dCrLf, i, dLastLine;
	UINT_PTR	cchSize;
	LPPOINT	pstPoint;

	dBaseDot  = *pNowDot;
	dBaseLine = *pdLine;
	dLastLine = *pdLine;

	if( !(ptText)  )	return 0;

	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

	if( dStyle &  D_SQUARE )
	{

		dNeedLine = DocStringInfoCount( ptText, cchSize, NULL, NULL );

		bFirst = DocSquareAddPreMod( *pNowDot, *pdLine, dNeedLine, bFirst );

		pstPoint = NULL;
		dCrLf = DocSquareAdd( pNowDot, pdLine, ptText, cchSize, &pstPoint );

		SqnAppendSquare( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptText, pstPoint, dCrLf, bFirst );
		bFirst = FALSE;

		FREE( pstPoint );

		dLastLine = *pdLine;
	}
	else
	{

		dCrLf = DocStringAdd( pNowDot, pdLine, ptText, cchSize );

		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptText, dBaseDot, dBaseLine, bFirst );
		bFirst = FALSE;

		dLastLine = DocPageParamGet( NULL, NULL );
	}

	if( dCrLf )
	{
		for( i = dBaseLine; dLastLine >= i; i++ ){	ViewRedrawSetLine(  i );	}
	}
	else
	{
		ViewRedrawSetLine( *pdLine );
	}

	if( pdMozi ){	*pdMozi = DocLetterPosGetAdjust( pNowDot, *pdLine , 0 );	}

	if( !(D_INVISI & dStyle) )	ViewDrawCaret( *pNowDot, *pdLine, TRUE );

	return dCrLf;
}

INT DocInputFromClipboard( PINT pNowDot, PINT pdLine, PINT pdMozi, UINT bSqMode )
{
	LPTSTR	ptString = NULL;
	UINT	cchSize, dStyle = 0, i, j;
	INT		dCrLf, dTop, dBtm;
	BOOLEAN	bSelect;
	UINT	dSqSel, iLines;

	ptString = DocClipboardDataGet( &dStyle );
	if( !(ptString) )
	{
		NotifyBalloonExist( TEXT("テキストじゃないみたい。\t\n貼り付けられないよ。"), TEXT("お燐からのお知らせ"), NIIF_INFO );
		return 0;
	}

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	for( i = 0; cchSize > i; )
	{
		if( CC_TAB == ptString[i] )
		{
			for( j = i; cchSize > j; j++ )
			{
				ptString[j] = ptString[j+1];
			}
			cchSize--;
			continue;
		}
		i++;
	}

	bSelect = IsSelecting( &dSqSel );
	if( bSelect )
	{
		DocSelRangeGet( &dTop, &dBtm );
		dCrLf = DocSelectedDelete( pNowDot, pdLine, dSqSel, TRUE );
		if( dCrLf  )
		{
			iLines = DocPageParamGet( NULL, NULL );
			for( i = *pdLine; iLines >= i; i++ ){	ViewRedrawSetLine(  i );	}
		}
		else{	ViewRedrawSetLine( *pdLine );	}

	}

	if( bSqMode )	dStyle |= D_SQUARE;
	dCrLf = DocInsertString( pNowDot, pdLine, pdMozi, ptString, dStyle, TRUE );

	FREE( ptString );

	DocPageInfoRenew( -1, 1 );

	return dCrLf;
}

INT DocExClipSelect( UINT bStyle )
{
	INT	cbSize;
	LPVOID	pString = NULL;

	cbSize = DocSelectTextGetAlloc( bStyle, &pString, NULL );

	TRACE( TEXT("BYTE:%d"), cbSize );

	DocClipboardDataSet( pString, cbSize, bStyle );

	FREE( pString );

	return cbSize;
}

LPTSTR DocClipboardDataGet( PUINT pdStyle )
{
	LPTSTR	ptString = NULL, ptClipTxt;
	LPSTR	pcStr, pcClipTp;
	DWORD	cbSize;
	UINT	ixSqrFmt, dEnumFmt;
	INT		ixCount, iC;
	HANDLE	hClipData;

	ixSqrFmt = RegisterClipboardFormat( CLIP_SQUARE );

	if( IsClipboardFormatAvailable( CF_UNICODETEXT ) )
	{
		if( pdStyle )
		{
			if( IsClipboardFormatAvailable( ixSqrFmt ) ){	*pdStyle = D_SQUARE;	}
		}

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

		if( dStyle & D_SQUARE  )
		{
			SetClipboardData( ixSqrFmt, hGlobal );
		}

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

HRESULT DocClipLetter( TCHAR ch )
{
	TCHAR	atBuff[3];

	ZeroMemory( atBuff, sizeof(atBuff) );
	atBuff[0] = ch;

	DocClipboardDataSet( atBuff, 4, D_UNI );

	return S_OK;
}

HRESULT DocPageAllCopy( UINT bStyle )
{
	INT	cbSize;
	LPVOID	pString = NULL;

	cbSize = DocPageGetAlloc( bStyle, &pString );

	TRACE( TEXT("BYTE:%d"), cbSize );

	DocClipboardDataSet( pString, cbSize, bStyle );

	FREE( pString );

	return S_OK;
}

HRESULT DocScreenFill( LPTSTR ptFill )
{
	UINT_PTR	dLines, dRiDot, cchSize;
	BOOLEAN		bSel = TRUE, bFirst;
	INT			iTop, iBottom, i, iUnt, j, remain;
	INT			nDot, sDot, mDot;
	LPTSTR		ptBuffer;
	wstring		wsBuffer;

	dLines = DocNowFilePageLineCount( );
	dRiDot = gdRightRuler;

	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 > iTop || 0 > iBottom ){	iTop = 0;	iBottom = dLines - 1;	bSel = FALSE;	}

	ViewSelPageAll( -1 );

	mDot = ViewStringWidthGet( ptFill );

	bFirst = TRUE;

	for( i = iTop; iBottom >= i; i++ )
	{
		nDot = DocLineParamGet( i , NULL, NULL );
		sDot = dRiDot - nDot;
		if( 0 >= sDot ){	continue;	}

		iUnt = (sDot / mDot) + 1;

		wsBuffer.clear( );
		for( j = 0; iUnt > j; j++ ){	wsBuffer += wstring( ptFill );	}

		cchSize = wsBuffer.size( ) + 1;
		ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
		StringCchCopy( ptBuffer, cchSize, wsBuffer.c_str( ) );

		DocInsertString( &nDot, &i, NULL, ptBuffer, 0, bFirst );	bFirst = FALSE;
		FREE(ptBuffer);

		DocBadSpaceCheck( i );
	}

	if( !(bSel) )
	{
		remain = 40 - dLines;
		if( 0 < remain )
		{
			DocAdditionalLine( remain, &bFirst );
			dLines = DocNowFilePageLineCount( );
			iUnt  = (dRiDot / mDot) + 1;

			wsBuffer.clear( );
			for( j = 0; iUnt > j; j++ ){	wsBuffer += wstring( ptFill );	}
			cchSize = wsBuffer.size( ) + 1;
			ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
			StringCchCopy( ptBuffer, cchSize, wsBuffer.c_str( ) );

			iTop    = iBottom + 1;
			iBottom = dLines - 1;

			for( i = iTop; iBottom >= i; i++ )
			{

				nDot = DocLineParamGet( i , NULL, NULL );
				DocInsertString( &nDot, &i, NULL, ptBuffer, 0, bFirst );	bFirst = FALSE;

				DocBadSpaceCheck( i );
			}

			FREE(ptBuffer);
		}
	}

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

INT_PTR CALLBACK PageNumDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPPAGENUMINFO	pstInfo;
	INT		id;
	HWND	hWndCtl;
	UINT	codeNotify;

	switch( message )
	{
		case WM_INITDIALOG:
			pstInfo = (LPPAGENUMINFO)lParam;
			SetDlgItemInt(  hDlg, IDE_PN_START, pstInfo->dStartNum, FALSE );
			SetDlgItemText( hDlg, IDE_PN_STYLE, pstInfo->atStyle );
			CheckDlgButton( hDlg, IDCB_PN_UNDER,    pstInfo->bInUnder );
			CheckDlgButton( hDlg, IDCB_PN_OVERRIDE, pstInfo->bOverride );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			hWndCtl = (HWND)lParam;
			codeNotify = HIWORD(wParam);

			if( IDOK == id )
			{
				pstInfo->dStartNum = GetDlgItemInt( hDlg, IDE_PN_START, NULL, FALSE );
				GetDlgItemText( hDlg, IDE_PN_STYLE, pstInfo->atStyle, MIN_STRING );
				pstInfo->bInUnder  = IsDlgButtonChecked( hDlg, IDCB_PN_UNDER );
				pstInfo->bOverride = IsDlgButtonChecked( hDlg, IDCB_PN_OVERRIDE );
				EndDialog(hDlg, IDOK );
				return (INT_PTR)TRUE;
			}
			if( IDCANCEL == id )
			{
				EndDialog(hDlg, IDCANCEL );
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

HRESULT DocPageNumInsert( HINSTANCE hInst, HWND hWnd )
{
	INT			dNowPageBuffer;
	INT			iLine, iDot;
	INT_PTR		iRslt, maxPage, iNow;
	UINT		ixNumber;
	BOOLEAN		bFirst = TRUE;
	TCHAR		atText[MAX_PATH];
	PAGENUMINFO	stInfo;

	dNowPageBuffer = gixFocusPage;

	maxPage = DocNowFilePageCount(  );

	ZeroMemory( &stInfo, sizeof(PAGENUMINFO) );
	stInfo.dStartNum = 1;

	stInfo.bInUnder  = InitParamValue( INIT_LOAD, VL_PAGE_UNDER,   BST_UNCHECKED );
	stInfo.bOverride = InitParamValue( INIT_LOAD, VL_PAGE_OVWRITE, BST_UNCHECKED );

	StringCchCopy( stInfo.atStyle, MAX_PATH, TEXT("%u") );
	InitParamString( INIT_LOAD, VS_PAGE_FORMAT, stInfo.atStyle );

	iRslt = DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_PAGENUMBER_DLG), hWnd, PageNumDlgProc, (LPARAM)(&stInfo) );
	if( IDOK == iRslt )
	{
#pragma message("ディレイロードしたら、頁番号挿入がおかしくなるはず")
		ixNumber = stInfo.dStartNum;

		InitParamString( INIT_SAVE, VS_PAGE_FORMAT, stInfo.atStyle );

		InitParamValue( INIT_SAVE, VL_PAGE_UNDER,   stInfo.bInUnder );
		InitParamValue( INIT_SAVE, VL_PAGE_OVWRITE, stInfo.bOverride );

		for( iNow = 0; maxPage > iNow; iNow++, ixNumber++ )
		{
			StringCchPrintf( atText, MAX_PATH, stInfo.atStyle, ixNumber );

			if( NowPageInfoGet( iNow, NULL ) )
			{

				DocDelayPageNumInsert( gitFileIt, iNow, &stInfo, atText );

			}
			else
			{
				gixFocusPage = iNow;
				if( stInfo.bInUnder )
				{
					if( stInfo.bOverride )
					{
						iLine = DocPageParamGet( NULL, NULL );
						iLine--;	if( 0 > iLine ){	iLine = 0;	}
						DocLineErase( iLine , &bFirst );
					}
					else
					{
						iLine = DocAdditionalLine( 1, &bFirst );
					}
				}
				else
				{
					iDot = 0;	iLine = 0;
					if( stInfo.bOverride )
					{
						DocLineErase( 0 , &bFirst );
					}
					else
					{
						DocInsertString( &iDot, &iLine, NULL, CH_CRLFW, 0, bFirst );	bFirst = FALSE;
					}
					iLine = 0;
				}
				iDot = 0;

				DocInsertString( &iDot, &iLine, NULL, atText, 0, bFirst );	bFirst = FALSE;
			}
		}

		gixFocusPage = dNowPageBuffer;

		ViewRedrawSetLine( -1 );
	}

	return S_OK;
}

HRESULT DocDelayPageNumInsert( FILES_ITR itFile, INT iPage, LPPAGENUMINFO pstInfo, LPCTSTR ptPageText )
{
	UINT_PTR	cchPgTx;
	UINT_PTR	cchSrc, cchSize;
	LPTSTR	ptWork, ptMoto, ptNext, ptCaret, ptPrev;

	if( !(itFile->vcCont.at( iPage ).ptRawData) )	return E_NOTIMPL;

	ptMoto = itFile->vcCont.at( iPage ).ptRawData;

	StringCchLength( ptPageText, MAX_PATH, &cchPgTx );

	StringCchLength( ptMoto, STRSAFE_MAX_CCH, &cchSrc );

	cchSize = cchSrc + cchPgTx + 4;
	ptWork = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	ZeroMemory( ptWork, cchSize * sizeof(TCHAR) );

	if( pstInfo->bInUnder )
	{
		StringCchCopy( ptWork, cchSize, ptMoto );

		if( pstInfo->bOverride )
		{
			ptNext = ptMoto;
			ptCaret = ptMoto;
			do{
				ptPrev = ptNext;
				ptNext = NextLineW( ptCaret );
				ptCaret = ptNext;

			}while( ptNext );

			StringCchCopy( ptPrev, (cchPgTx + 4), ptPageText );

		}
		else
		{
			StringCchCat( ptWork, cchSize, TEXT("\r\n") );
			StringCchCat( ptWork, cchSize, ptPageText );
		}
	}
	else
	{
		StringCchCopy( ptWork, cchSize, ptPageText );
		StringCchCat( ptWork, cchSize, TEXT("\r\n") );

		if( pstInfo->bOverride )
		{

			ptNext = NextLineW( ptMoto );
			if( ptNext )	StringCchCat( ptWork, cchSize, ptNext );

		}
		else
		{

			StringCchCat( ptWork, cchSize, ptMoto );
		}
	}

	FREE( itFile->vcCont.at( iPage ).ptRawData );
	itFile->vcCont.at( iPage ).ptRawData = ptWork;

	return S_OK;
}
