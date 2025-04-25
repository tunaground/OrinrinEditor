#include "stdafx.h"
#include "OrinrinEditor.h"

extern FILES_ITR	gitFileIt;
extern INT			gixFocusPage;

extern  UINT		gbCrLfCode;

HRESULT DocStatisticsPage( FILES_ITR itFile, INT iPage )
{

	return S_OK;
}

BOOLEAN DocRangeIsError( FILES_ITR itFile, INT iPage, INT iLine )
{
	INT_PTR	iSize;

	if( 0 > iPage || 0 > iLine )	return TRUE;

	iSize = itFile->vcCont.size( );
	if( 0 >= iSize || iPage >= iSize )	return TRUE;

	iSize = itFile->vcCont.at( iPage ).ltPage.size( );
	if( 0 >= iSize || iLine >= iSize )	return TRUE;

	return FALSE;
}

BOOLEAN DocBadSpaceIsExist( INT rdLine )
{
	LINE_ITR	itLine;

	if( DocRangeIsError( gitFileIt, gixFocusPage, rdLine ) ){	return 0;	}

	itLine = gitFileIt->vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	return itLine->bBadSpace;
}

UINT DocBadSpaceCheck( INT rdLine )
{
	UINT_PTR	iCount, iRslt;
	BOOLEAN		bWarn;
	TCHAR		ch, chn;
	LETR_ITR	ltrItr, ltrEnd, ltrNext;
	LINE_ITR	itLine;

	if( DocRangeIsError( gitFileIt, gixFocusPage, rdLine ) )
	{
		TRACE( TEXT("범위 외 오류 발생 PAGE[%d], LINE[%d]"), gixFocusPage, rdLine );
		return 0;
	}

	itLine = gitFileIt->vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );
	if( 0 == iCount )
	{

		if( itLine->bBadSpace )
		{
			itLine->bBadSpace = 0;
			ViewRedrawSetLine( rdLine );
		}
		return 0;
	}

	iRslt = 0;

	bWarn = FALSE;
	ltrEnd = itLine->vcLine.end( );

	for( ltrItr = itLine->vcLine.begin(); ltrEnd != ltrItr; ltrItr++ )
	{
		ch = ltrItr->cchMozi;

		ltrItr->mzStyle &= ~CT_WARNING;

		if( 0xFF < ch ){	bWarn = FALSE;	continue;	}

		if( isspace( ch ) )
		{
			if( !(bWarn) )
			{

				ltrNext = ltrItr + 1;
				if( ltrNext !=  ltrEnd )
				{
					chn = ltrNext->cchMozi;
					if( 0xFF >= chn )
					{
						if( isspace( chn ) )
						{
							ltrItr->mzStyle |= CT_WARNING;
							bWarn = TRUE;
							iRslt = 1;
						}
					}
				}
			}
			else
			{
				ltrItr->mzStyle |= CT_WARNING;
			}
		}
		else{	bWarn = FALSE;	}
	}

	ltrEnd--;
	if( iswspace( ltrEnd->cchMozi ) ){	iRslt = 1;	}

	ch = itLine->vcLine.at( 0 ).cchMozi;

	if( 0xFF >= ch )
	{
		if( isspace( ch ) )
		{
			itLine->vcLine.at( 0 ).mzStyle |= CT_WARNING;
			iRslt = 1;
		}
	}

#pragma message ("DocBadSpaceCheck 내의 갱신 지령 최적화가 필요")

	if( iRslt != itLine->bBadSpace )
	{
		ViewRedrawSetLine( rdLine );
	}

	itLine->bBadSpace = iRslt;

	return iRslt;
}

UINT_PTR DocNowFilePageCount( VOID )
{
	return gitFileIt->vcCont.size( );
}

UINT_PTR DocNowFilePageLineCount( VOID )
{
	return gitFileIt->vcCont.at( gixFocusPage ).ltPage.size( );
}

UINT DocRawDataParamGet( LPCTSTR ptRaw, PINT piMozi, PINT piByte )
{
	UINT_PTR	cchSize, d;
	INT			iMozis, iLines, iBytes, iBy;

	StringCchLength( ptRaw, STRSAFE_MAX_CCH, &cchSize );

	iBytes = 0;
	iMozis = 0;
	iLines = 1;

	for( d = 0; cchSize > d; d++ )
	{
		if( TEXT('\r') == ptRaw[d] && TEXT('\n') == ptRaw[d+1] )
		{
			iLines++;

			if( gbCrLfCode )	iBytes += YY2_CRLF;
			else				iBytes += STRB_CRLF;

			d++;
			continue;
		}

		iBy = DocLetterDataCheck( NULL , ptRaw[d] );

		iBytes += iBy;
		iMozis++;
	}

	if( piMozi )	*piMozi = iMozis;
	if( piByte )	*piByte = iBytes;

	return iLines;
}

UINT DocPageParamGet( PINT piMozi, PINT piByte )
{
	INT_PTR	iLines, i, dMozis = 0;
	INT		dBytes = 0;

	LINE_ITR	itLine;

	if( gitFileIt->vcCont.at( gixFocusPage ).ptRawData )
	{
		iLines = DocRawDataParamGet( gitFileIt->vcCont.at( gixFocusPage ).ptRawData, &dMozis, &dBytes );
		gitFileIt->vcCont.at( gixFocusPage ).iLineCnt = iLines;
		gitFileIt->vcCont.at( gixFocusPage ).iMoziCnt = dMozis;
	}
	else
	{
		iLines = DocNowFilePageLineCount( );

		itLine = gitFileIt->vcCont.at( gixFocusPage ).ltPage.begin();
		for( i = 0; iLines > i; i++, itLine++ )
		{

			if( 1 <= i )
			{
				if( gbCrLfCode )	dBytes += YY2_CRLF;
				else				dBytes += STRB_CRLF;
			}

			dMozis += itLine->vcLine.size( );
			dBytes += itLine->iByteSz;
		}
	}

	if( piMozi )	*piMozi = dMozis;
	if( piByte )	*piByte = dBytes;

	gitFileIt->vcCont.at( gixFocusPage ).dByteSz = dBytes;

	DocPageInfoRenew( -1, 1 );

	return iLines;
}

UINT DocPageByteCount( FILES_ITR itFile, INT dPage, PINT pMozi, PINT pByte )
{
	INT		iBytes, iMozis, i, iLnBy, iDots;
	UINT	dLines;
	LINE_ITR	itLine, endLine;
	LETR_ITR	itMozi, endMozi;

	if( 0 > dPage ){	dPage = gixFocusPage;	}

	if( itFile->vcCont.at( dPage ).ptRawData )
	{
		MessageBox( NULL, TEXT("DocPageByteCount"), TEXT("DELAY_LOAD"), MB_OK );
	}

	iBytes = 0;
	iMozis = 0;

	dLines = DocNowFilePageLineCount( );

	itLine  = itFile->vcCont.at( dPage ).ltPage.begin();
	endLine = itFile->vcCont.at( dPage ).ltPage.end();

	for( i = 0; itLine != endLine; itLine++, i++ )
	{

		itMozi  = itLine->vcLine.begin();
		endMozi = itLine->vcLine.end();

		iLnBy = 0;	iDots = 0;
		for( ; itMozi != endMozi; itMozi++ )
		{
			iDots += itMozi->rdWidth;
			iLnBy += itMozi->mzByte;
			iMozis++;
		}
		itLine->iByteSz = iLnBy;
		itLine->iDotCnt = iDots;

		iBytes += iLnBy;

		if( 1 <= i )
		{
			if( gbCrLfCode )	iBytes += YY2_CRLF;
			else				iBytes += STRB_CRLF;
		}
	}

	itFile->vcCont.at( dPage ).dByteSz = iBytes;

	if( pMozi ){	*pMozi = iMozis;	}
	if( pByte ){	*pByte = iBytes;	}

	return dLines;
}

INT DocPageMaxDotGet( INT dTop, INT dBottom )
{
	INT		maxDot = 0, thisDot, i;
	UINT_PTR	iLines;

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );

	if( 0 > dTop )		dTop = 0;
	if( 0 > dBottom )	dBottom = iLines - 1;

	itLine = gitFileIt->vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, dTop );

	for( i = dTop; dBottom >= i; i++, itLine++ )
	{
		thisDot = itLine->iDotCnt;
		if( maxDot < thisDot )	maxDot = thisDot;
	}

	return maxDot;
}

INT DocLineParamGet( INT rdLine, PINT pdMozi, PINT pdByte )
{
	INT_PTR	iCount, i, iLines;
	INT		dDotCnt, dByteCnt;

#ifdef DO_TRY_CATCH
	try{
#endif

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );
	if( iLines <= rdLine )	return -1;

	itLine = gitFileIt->vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );

	if( pdMozi )	*pdMozi = iCount;

	dDotCnt = 0;
	dByteCnt = 0;
	for( i = 0; iCount > i; i++ )
	{
		dDotCnt  += itLine->vcLine.at( i ).rdWidth;
		dByteCnt += itLine->vcLine.at( i ).mzByte;
	}

	if( pdByte )	*pdByte = dByteCnt;

	itLine->iDotCnt = dDotCnt;
	itLine->iByteSz = dByteCnt;

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), -1 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), -1 );	}
#endif

	return dDotCnt;
}

INT DocLetterPosGetAdjust( PINT pNowDot, INT rdLine, INT round )
{
	INT	i, iCount, iLines;
	INT	iLetter;
	INT	iMaxLine;
	INT	dDotCnt = 0, dPrvCnt = 0, rdWidth = 0;

	LINE_ITR	itLine;

#ifdef DO_TRY_CATCH
	try{
#endif

	iMaxLine = DocNowFilePageLineCount(  );
	if( iMaxLine <= rdLine )	rdLine = iMaxLine - 1;

	itLine = gitFileIt->vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	assert( pNowDot );

	iLines = DocLineParamGet( rdLine, &iCount, NULL );
	if( 0 > iLines )	return 0;
#pragma message ("행 수 확인만 한다면 DocLineParamGet에서 재계산에 걸리는 것이 아닌가")

	for( i = 0, iLetter = 0; iCount > i; i++, iLetter++ )
	{
		if( dDotCnt >= *pNowDot ){	break;	}

		dPrvCnt = dDotCnt;
		rdWidth = itLine->vcLine.at( i ).rdWidth;

		dDotCnt += rdWidth;
	}

	if( dDotCnt != *pNowDot )
	{
		if( 0 <  round )
		{
			*pNowDot = dDotCnt;
		}
		else if( 0 > round )
		{
			*pNowDot = dPrvCnt;
			iLetter--;
		}
		else
		{

			if( (*pNowDot - dPrvCnt) < (dDotCnt - *pNowDot ) )
			{
				*pNowDot = dPrvCnt;
				iLetter--;
			}
			else
			{
				*pNowDot = dDotCnt;
			}
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return iLetter;
}

INT DocLetterShiftPos( INT nowDot, INT rdLine, INT bDirect, PINT pdAbsDot, PBOOLEAN pbJump )
{
	INT_PTR	iCount, iLetter, iLines;
	INT		dLtrDot = 0;

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );
	if( iLines <=  rdLine ){	return -1;	}

	if( 0 == bDirect )
	{
		if( pdAbsDot ){	*pdAbsDot = nowDot;	}
		if( pbJump ){	*pbJump = FALSE;	}
		return nowDot;
	}

	itLine = gitFileIt->vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );

	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	if( ((0 == iLetter) && (0 > bDirect)) || ((iCount <= iLetter) && (0 < bDirect)) )
	{
		if((0 > bDirect) && (0 == rdLine))	return 0;

		if( pbJump ){	*pbJump =  TRUE;	}

		return 0;
	}

	if( 0 > bDirect )
	{
		dLtrDot = itLine->vcLine.at( iLetter-1 ).rdWidth;

		nowDot -= dLtrDot;
	}

	if( 0 < bDirect )
	{
		dLtrDot = itLine->vcLine.at( iLetter ).rdWidth;

		nowDot += dLtrDot;
	}

	if( pdAbsDot )	*pdAbsDot = nowDot;

	return dLtrDot;
}

INT DocStringInfoCount( LPCTSTR ptStr, UINT_PTR cchSize, PINT pMaxDot, PINT pMaxLtr )
{
	INT	iDot, iLine, iMax, iMozi, e;
	UINT_PTR	d;
	wstring	wsBuffer;

	if(  0 == cchSize ){	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );	}

	wsBuffer.clear();
	iDot = 0;	iLine = 0;	iMax = 0;	iMozi = 0;

	for( d = 0, e = 0; cchSize >= d; d++ )
	{
		if( cchSize <= d || 0x000D == ptStr[d] )
		{
			iLine++;
			iDot = ViewStringWidthGet( wsBuffer.c_str() );
			if( iMax  < iDot ){	iMax  = iDot;	}
			if( iMozi < e ){	iMozi = e;		}

			if( cchSize <= d )	break;

			d++;

			e = 0;
			wsBuffer.clear();
		}
		else
		{
			wsBuffer.push_back( ptStr[d] );
		}
	}

	if( pMaxDot ){	*pMaxDot = iMax;	}
	if( pMaxLtr ){	*pMaxLtr = iMozi;	}

	return iLine;
}

BOOLEAN NowPageInfoGet( UINT iTgtPage, LPPAGEINFOS pstInfo )
{
	LINE_ITR	itLine;
	INT_PTR		iMozis;
	UINT		dMasqus;

	if( gitFileIt->vcCont.size(  ) <= iTgtPage )	return 0;

	if( pstInfo )
	{
		dMasqus = pstInfo->dMasqus;
		ZeroMemory( pstInfo, sizeof(PAGEINFOS) );

		if( PI_RECALC & dMasqus )
		{
			DocPageByteCount( gitFileIt, iTgtPage, NULL, NULL );
		}

		if( PI_LINES & dMasqus )
		{
			pstInfo->iLines = gitFileIt->vcCont.at( iTgtPage ).ltPage.size( );
		}

		if( PI_BYTES & dMasqus )
		{
			pstInfo->iBytes = gitFileIt->vcCont.at( iTgtPage ).dByteSz;
		}

		if( PI_MOZIS & dMasqus )
		{
			iMozis = 0;
			for( itLine = gitFileIt->vcCont.at( iTgtPage ).ltPage.begin(); gitFileIt->vcCont.at( iTgtPage ).ltPage.end() != itLine; itLine++ )
			{
				iMozis += itLine->vcLine.size();
			}
			pstInfo->iMozis = iMozis;
		}

		if( PI_NAME & dMasqus )
		{
			StringCchCopy( pstInfo->atPageName, SUB_STRING, gitFileIt->vcCont.at( iTgtPage ).atPageName );
		}
	}

	return (gitFileIt->vcCont.at( iTgtPage ).ptRawData ? TRUE : FALSE);
}

BOOLEAN PageIsDelayed( FILES_ITR itFile, UINT dPage )
{
	return (itFile->vcCont.at( dPage ).ptRawData ? TRUE : FALSE);
}

VOID ZeroONELINE( LPONELINE pstLine )
{

	pstLine->iDotCnt    = 0;
	pstLine->iByteSz    = 0;
	pstLine->dStyle     = 0;
	pstLine->bBadSpace  = FALSE;
	pstLine->vcLine.clear(  );
	pstLine->dFrtSpDot  = 0;
	pstLine->dFrtSpMozi = 0;

	return;
}
