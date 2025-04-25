#include "stdafx.h"
#include "OrinrinEditor.h"

extern FILES_ITR	gitFileIt;
extern INT		gixFocusPage;

extern  UINT	gbUniPad;
extern  UINT	gbCrLfCode;

static INT		gdSelByte;

INT		DocLetterSelStateToggle( INT, INT, INT );
VOID	DocSelectedByteStatus( VOID );

HRESULT DocSelRangeSet( INT dTop, INT dBottom )
{
	TRACE( TEXT(" 選択レンジセット[%d - %d]"), dTop, dBottom );

	(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop    = dTop;
	(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom = dBottom;

	return S_OK;
}

HRESULT DocSelRangeGet( PINT pdTop, PINT pdBtm )
{
	if( pdTop ){	*pdTop = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;	}
	if( pdBtm ){	*pdBtm = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;	}

	return S_OK;
}

HRESULT DocSelRangeReset( PINT pdTop, PINT pdBtm )
{
	INT	iTop, iEnd, iLine;
	LINE_ITR	itLine, itLnEnd;
	LETR_ITR	itLtr;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	itLnEnd = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.end();

	iTop = -1;	iEnd = -1;
	for( iLine = 0; itLnEnd != itLine; itLine++, iLine++ )
	{
		for( itLtr = itLine->vcLine.begin(); itLine->vcLine.end() != itLtr; itLtr++ )
		{
			if( CT_SELECT & itLtr->mzStyle )
			{
				if( 0 > iTop )	iTop = iLine;
				iEnd = iLine;

				break;
			}
		}
	}

	(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop    = iTop;
	(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom = iEnd;

	if( pdTop ){	*pdTop = iTop;	}
	if( pdBtm ){	*pdBtm = iEnd;	}

	return S_OK;
}

VOID DocSelByteSet( INT iBytes )
{
	gdSelByte = iBytes;
}

UINT DocLetterSelStateGet( INT nowDot, INT rdLine )
{
	UINT	dStyle;
	INT		iLetter;
	INT_PTR	iLines, iLength;

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );
	if( iLines <= rdLine )	return 0;

	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );
	if( (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.end() == itLine ){	return 0;	}

	iLength = itLine->vcLine.size();
	if( iLength <= iLetter )	return 0;

	dStyle  = itLine->vcLine.at( iLetter ).mzStyle;

	if( dStyle & CT_SELECT )	return 1;

	return 0;
}

INT DocLetterSelStateToggle( INT nowDot, INT rdLine, INT dForce )
{
	UINT	dStyle, maeSty;
	INT		dLtrDot = 0, iLetter, dByte;
	INT_PTR	iLines, iLength;

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );
	if( iLines <= rdLine )	return 0;

	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );
	if( (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.end() == itLine ){	return 0;	}

	iLength = itLine->vcLine.size();
	if( iLength <= iLetter )	return 0;

	dLtrDot = itLine->vcLine.at( iLetter ).rdWidth;
	dByte   = itLine->vcLine.at( iLetter ).mzByte;

	dStyle  = itLine->vcLine.at( iLetter ).mzStyle;
	maeSty = dStyle;
	if( 0 == dForce ){		dStyle ^=  CT_SELECT;	}
	else if( 0 < dForce ){	dStyle |=  CT_SELECT;	}
	else if( 0 > dForce ){	dStyle &= ~CT_SELECT;	}
	itLine->vcLine.at( iLetter ).mzStyle = dStyle;

	TRACE( TEXT("L[%d] D[%d] B[%d] f[0x%X]"), rdLine, dLtrDot, dByte, dStyle );

	if( maeSty != dStyle )
	{
		if( CT_SELECT & dStyle )	gdSelByte += dByte;
		else						gdSelByte -= dByte;

		if( 0 >  gdSelByte )	gdSelByte = 0;

	}

	return dLtrDot;
}

INT DocRangeSelStateToggle( INT dBgnDot, INT dEndDot, INT rdLine, INT dForce )
{
	UINT_PTR	iLines;
	INT	dLtrDot = 0, dMaxDots, dDot;
	RECT	rect;

	iLines = DocNowFilePageLineCount( );
	if( (INT)iLines <=  rdLine )	return 0;

	dMaxDots = DocLineParamGet( rdLine, NULL, NULL );

	if( 0 > dBgnDot )	dBgnDot = 0;
	if( 0 > dEndDot )	dEndDot = dMaxDots;

	for( dDot = dBgnDot; dEndDot > dDot;  )
	{
		dDot += DocLetterSelStateToggle( dDot, rdLine, dForce );
	}

	dLtrDot = dDot - dBgnDot;

	rect.left   = dBgnDot;
	rect.top    = rdLine * LINE_HEIGHT;
	rect.right  = dEndDot;
	rect.bottom = rect.top + LINE_HEIGHT;

	ViewRedrawSetRect( &rect );

	DocSelectedByteStatus(  );

	return dLtrDot;
}

HRESULT DocReturnSelStateToggle( INT rdLine, INT dForce )
{
	UINT_PTR	iLines;
	UINT		dStyle, maeSty;
	INT			iLnDot, dByte;
	RECT		rect;

	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );
	if( (INT)iLines <=  rdLine )	return E_OUTOFMEMORY;

	iLnDot = DocLineParamGet( rdLine, NULL, NULL );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	dStyle = itLine->dStyle;
	maeSty = dStyle;
	if( 0 == dForce ){		dStyle ^=  CT_SELRTN;	}
	else if( 0 < dForce ){	dStyle |=  CT_SELRTN;	}
	else if( 0 > dForce ){	dStyle &= ~CT_SELRTN;	}
	itLine->dStyle = dStyle;
	if( maeSty != dStyle )
	{
		if( gbCrLfCode )	dByte = YY2_CRLF;
		else				dByte = STRB_CRLF;

		if( CT_SELRTN & dStyle )	gdSelByte += dByte;
		else						gdSelByte -= dByte;

		if( 0 >  gdSelByte )	gdSelByte = 0;

		DocSelectedByteStatus(  );
	}

	rect.left   = iLnDot;
	rect.top    = rdLine * LINE_HEIGHT;
	rect.right  = iLnDot + 20;
	rect.bottom = rect.top + LINE_HEIGHT;

	ViewRedrawSetRect( &rect );

	return S_OK;
}

INT DocPageSelStateToggle( INT dForce )
{
	UINT_PTR	iLines, ln, iLetters, mz;
	UINT		dStyle;
	INT			iTotal, iDot, iWid;
	RECT		inRect;

	LINE_ITR	itLine;

	if( 0 == dForce )	return 0;

	if( 0 > gixFocusPage )	return 0;

	iTotal = 0;

	iLines = DocNowFilePageLineCount( );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	for( ln = 0; iLines > ln; ln++, itLine++ )
	{
		iDot = 0;
		inRect.top    = ln * LINE_HEIGHT;
		inRect.bottom = inRect.top + LINE_HEIGHT;

		iLetters = itLine->vcLine.size( );

		for( mz = 0; iLetters > mz; mz++ )
		{

			dStyle = itLine->vcLine.at( mz ).mzStyle;
			iWid   = itLine->vcLine.at( mz ).rdWidth;

			inRect.left  = iDot;
			inRect.right = iDot + iWid;

			if( 0 < dForce )
			{
				itLine->vcLine.at( mz ).mzStyle |=  CT_SELECT;
				if( !(dStyle & CT_SELECT) ){	ViewRedrawSetRect( &inRect );	}
			}
			else
			{
				itLine->vcLine.at( mz ).mzStyle &= ~CT_SELECT;
				if( dStyle & CT_SELECT ){	ViewRedrawSetRect( &inRect );	}
			}

			iDot += iWid;
			iTotal++;
		}

		dStyle = itLine->dStyle;
		inRect.left  = iDot;
		inRect.right = iDot + 20;
		if( 0 < dForce )
		{
			if( iLines > ln+1 )
			{
				itLine->dStyle |=  CT_SELRTN;
				if( !(dStyle & CT_SELRTN) ){	ViewRedrawSetRect( &inRect );	}
			}
		}
		else
		{
			itLine->dStyle &=  ~CT_SELRTN;
			if( dStyle & CT_SELRTN ){	ViewRedrawSetRect( &inRect );	}
		}
	}

	if( 0 < dForce )
	{
		DocSelRangeSet(  0, iLines - 1 );
		DocPageParamGet( NULL, &gdSelByte );
	}
	else
	{
		DocSelRangeSet( -1, -1 );
		gdSelByte = 0;
	}
	DocSelectedByteStatus(  );

	return iTotal;
}

VOID DocSelectedByteStatus( VOID )
{
	TCHAR	atBuffer[MIN_STRING];

	if( gdSelByte )
	{
		StringCchPrintf( atBuffer, MIN_STRING, TEXT("SEL %d Bytes"), gdSelByte );
		MainStatusBarSetText( SB_SELBYTE, atBuffer );
	}
	else
	{
		MainStatusBarSetText( SB_SELBYTE, TEXT("") );
	}

	return;
}

INT DocSelectedDelete( PINT pdDot, PINT pdLine, UINT bSqSel, BOOLEAN bFirst )
{

	UINT_PTR	iMozis;
	INT			i, j, dBeginX = 0, dBeginY = 0, cbSize;
	INT			iLct, k, bCrLf;
	LPTSTR		ptText;
	LPPOINT		pstPt;

	LETR_ITR	itLtr, itEnd, itHead, itTail;
	LINE_ITR	itLine;

#ifdef DO_TRY_CATCH
	try{
#endif

	bSqSel &= D_SQUARE;

	i = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	j = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	TRACE( TEXT("範囲削除[T%d - B%d]"), i, j );
	if( 0 > i ){	return 0;	}

	iLct = j - i + 1;
	cbSize = DocSelectTextGetAlloc( D_UNI | bSqSel, (LPVOID *)(&ptText), NULL );
	pstPt = (LPPOINT)malloc( iLct * sizeof(POINT) );
	ZeroMemory( pstPt, iLct * sizeof(POINT) );
	k = iLct - 1;

	bCrLf = iLct - 1;

	dBeginY = i;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, j );

	for( ; i <= j; j--, k--, itLine-- )
	{

		iMozis = itLine->vcLine.size( );
		if( 0 < iMozis )
		{
			itLtr = itLine->vcLine.begin(  );
			itEnd = itLine->vcLine.end(  );
			itHead = itEnd;
			itTail = itEnd;

			dBeginX = 0;

			for( ; itLtr != itEnd; itLtr++ )
			{
				if( CT_SELECT & itLtr->mzStyle )
				{
					itHead =  itLtr;
					break;
				}

				dBeginX += itLtr->rdWidth;
			}

			for( ; itLtr != itEnd; itLtr++ )
			{
				if( !(CT_SELECT & itLtr->mzStyle) )
				{
					itTail =  itLtr;
					break;
				}
			}
		}

		pstPt[k].x = dBeginX;
		pstPt[k].y = j;

		if( 0 < iMozis )
		{

			itLine->vcLine.erase( itHead, itTail );
		}

		if( CT_SELRTN & itLine->dStyle ){	DocLineCombine( j );	}

		DocLineParamGet( j, NULL, NULL );

		if( D_SQUARE & bSqSel ){	DocBadSpaceCheck( j );	}

		if( (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin() == itLine )	break;

	}

	ViewSelPageAll( -1 );

	*pdDot = dBeginX;	*pdLine = dBeginY;

	if( !(D_SQUARE & bSqSel)  ){	DocBadSpaceCheck( dBeginY );	}

	if( bSqSel ){	SqnAppendSquare( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptText, pstPt, iLct , bFirst );	}
	else{		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptText, dBeginX, dBeginY, bFirst );	}

	FREE( ptText );

	FREE( pstPt );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (INT)ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return (INT)ETC_MSG( ("etc error"), 0 );	}
#endif

	return bCrLf;
}

INT DocSelectedBrushFilling( LPTSTR ptBrush, PINT pdDot, PINT pdLine )
{
	UINT_PTR	iMozis;
	UINT_PTR	cchSize;
	INT			i, j, dBeginX = 0, dBeginY = 0;
	INT			iLct, dTgtDot, dBgnDot, dNowDot;
	BOOLEAN		bFirst;

	LPTSTR		ptReplc = NULL, ptDeled;

	wstring		wsBuffer;
	LETR_ITR	itLtr, itEnd, itHead, itTail;

	LINE_ITR	itLine;

	bFirst = TRUE;

	i = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	j = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	TRACE( TEXT("範囲確認[T%d - B%d]"), i, j );
	if( 0 > i ){	return 0;	}

	dBeginY = i;
	dBeginX = 0;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, i );

	for( iLct = i; j >= iLct; iLct++, itLine++ )
	{

		iMozis = itLine->vcLine.size( );
		if( 0 < iMozis )
		{
			itLtr = itLine->vcLine.begin(  );
			itEnd = itLine->vcLine.end(  );
			itHead = itEnd;
			itTail = itEnd;

			dBgnDot = 0;
			dTgtDot = 0;

			for( ; itLtr != itEnd; itLtr++ )
			{
				if( CT_SELECT & itLtr->mzStyle )
				{
					itHead =  itLtr;
					dTgtDot = itLtr->rdWidth;
					itLtr++;
					break;
				}

				dBgnDot += itLtr->rdWidth;
			}
			if( iLct == i ){	dBeginX = dBgnDot;	}

			for( ; itLtr != itEnd; itLtr++ )
			{
				if( !(CT_SELECT & itLtr->mzStyle) )
				{
					itTail =  itLtr;
					break;
				}
				dTgtDot += itLtr->rdWidth;
			}

			if( ptBrush )
			{
				ptReplc = BrushStringMake( dTgtDot, ptBrush );
			}
			else
			{
				ptReplc = DocPaddingSpaceMake( dTgtDot );
			}

			if( !(ptReplc) )	continue;

			wsBuffer.clear();
			for( itLtr = itHead; itLtr != itTail; itLtr++ )
			{
				wsBuffer += itLtr->cchMozi;
			}

			cchSize = wsBuffer.size( ) + 1;
			ptDeled = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
			StringCchCopy( ptDeled, cchSize, wsBuffer.c_str( ) );

			itLine->vcLine.erase( itHead, itTail );

			StringCchLength( ptReplc, STRSAFE_MAX_CCH, &cchSize );
			dNowDot = dBgnDot;
			DocStringAdd( &dNowDot, &iLct, ptReplc, cchSize );

			SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptDeled, dBgnDot, iLct, bFirst );	bFirst = FALSE;
			SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptReplc, dBgnDot, iLct, bFirst );

			FREE( ptDeled );

			FREE( ptReplc );

			ViewRedrawSetLine( iLct );
		}

	}

	ViewSelPageAll( -1 );

	*pdDot = dBeginX;	*pdLine = dBeginY;

	return 1;
}

#pragma message ("指定行選択範囲確保、必要になったら作る")
#if 0

INT DocSelectLineSelTextAlloc( LINE_ITR itLine, UINT bStyle, LPVOID *pText, PINT piDot, PINT piMozi )
{

	return 0;
}

#endif

INT DocSelectTextGetAlloc( UINT bStyle, LPVOID *pText, LPPOINT *pstPt )
{

	UINT_PTR	iLines, i, j, iLetters;
	INT_PTR		iSize, cchSz;
	INT			d, k, m, iLn;
	BOOLEAN		bNoSel;
	LPPOINT		pstPoint = NULL;

	string	srString;
	wstring	wsString;

	LINE_ITR	itLine;

	srString.clear( );
	wsString.clear( );

	iLines = DocNowFilePageLineCount( );

	d = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	k = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	TRACE( TEXT("選択内容確保[%d - %d]"), d, k );
	if( 0 > d ){	d = 0;	}
	if( 0 > k ){	k = iLines -  1;	}

	if( pstPt )
	{
		iLn = k - d + 1;
		if( 0 < iLn ){	pstPoint = (LPPOINT)malloc( iLn * sizeof(POINT) );	}
		*pstPt = pstPoint;
	}

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, d );

	for( m = 0, i = d; iLines > i; i++, m++, itLine++ )
	{
		if( pstPoint  ){	pstPoint[m].x = 0;	pstPoint[m].y = i;	}

		iLetters = itLine->vcLine.size( );

		bNoSel = TRUE;
		for( j = 0; iLetters > j; j++ )
		{

			if( CT_SELECT & itLine->vcLine.at( j ).mzStyle )
			{
				bNoSel = FALSE;

				if( bStyle & D_UNI )	wsString += itLine->vcLine.at( j ).cchMozi;
				else	srString +=  string( itLine->vcLine.at( j ).acSjis );
			}

			if( bNoSel && pstPt )	pstPoint[m].x += itLine->vcLine.at( j ).rdWidth;
		}

		if( bStyle & D_SQUARE  )
		{
			if( bStyle & D_UNI )	wsString += wstring( CH_CRLFW );
			else					srString +=  string( CH_CRLFA );
		}
		else
		{

			if( CT_SELRTN & itLine->dStyle )
			{
				if( bStyle & D_UNI )	wsString += wstring( CH_CRLFW );
				else					srString +=  string( CH_CRLFA );
			}
		}

		if( (INT)i == (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom )	break;
	}

	if( bStyle & D_UNI )
	{
		cchSz = wsString.size( ) + 1;
		iSize = cchSz * sizeof(TCHAR);

		if( pText )
		{
			*pText = (LPTSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopy( (LPTSTR)(*pText), cchSz, wsString.c_str( ) );
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

	return iSize;
}

HRESULT DocExtractExecute( HINSTANCE hInst )
{
	INT	dOffDot, dCount;
	BOOLEAN	bLnFirst, bMzFirst, bIsVoid;
	LPTSTR	ptSpace, ptString;
	UINT_PTR	cch;

	LINE_ITR	itLnFirst, itLnLast, itLnErate, itLnEnd;
	LETR_ITR	itMozi, itMzEnd;

	wstring	wsBuffer;

	if( 0 >= DocNowFilePageCount( ) )	return S_FALSE;

	itLnErate = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	itLnEnd   = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.end();
	itLnFirst = itLnErate;
	itLnLast  = itLnEnd;

	dOffDot = DocPageMaxDotGet( -1, -1 );

	bLnFirst = TRUE;

	for( ; itLnEnd != itLnErate; itLnErate++ )
	{
		itMozi  = itLnErate->vcLine.begin();
		itMzEnd = itLnErate->vcLine.end();

		dCount = 0;

		for( ; itMzEnd != itMozi; itMozi++ )
		{
			if( CT_SELECT & itMozi->mzStyle )
			{
				if( bLnFirst )
				{
					itLnFirst = itLnErate;
					bLnFirst = FALSE;
				}
				itLnLast  = itLnErate;

				if( dOffDot > dCount )	dOffDot = dCount;

				break;
			}

			dCount += itMozi->rdWidth;

		}
	}
	if( itLnLast != itLnEnd )	 itLnLast++;

	if( bLnFirst )	return  S_FALSE;

	wsBuffer.clear();

	for( itLnErate = itLnFirst; itLnLast != itLnErate; itLnErate++ )
	{
		itMozi  = itLnErate->vcLine.begin();
		itMzEnd = itLnErate->vcLine.end();

		bMzFirst = TRUE;
		bIsVoid  = FALSE;
		dCount   = 0;

		for( ; itMzEnd != itMozi; itMozi++ )
		{
			if( CT_SELECT & itMozi->mzStyle )
			{
				if( bIsVoid )
				{
					if( bMzFirst )
					{
						dCount -= dOffDot;
						if( 0 > dCount )	dCount = 0;
						bMzFirst = FALSE;
					}

					ptSpace = DocPaddingSpaceMake( dCount );
					if( ptSpace )
					{
						wsBuffer += ptSpace;

						FREE(ptSpace);
					}
				}

				wsBuffer += itMozi->cchMozi;
				dCount =  0;
				bIsVoid = FALSE;
			}
			else
			{
				dCount += itMozi->rdWidth;
				bIsVoid = TRUE;
			}
		}

		wsBuffer += CH_CRLFW;
	}

	cch = wsBuffer.size( ) + 1;
	ptString = (LPTSTR)malloc( cch * sizeof(TCHAR) );
	StringCchCopy( ptString, cch, wsBuffer.c_str( ) );

	if( hInst )
	{
		LayerBoxVisibalise( hInst, ptString, 0x00 );
	}
	else
	{
		DocClipboardDataSet( ptString, cch * sizeof(TCHAR), D_UNI );
	}

	FREE(ptString);

	return S_OK;
}
