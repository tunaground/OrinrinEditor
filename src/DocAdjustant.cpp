#include "stdafx.h"
#include "OrinrinEditor.h"

extern FILES_ITR	gitFileIt;
extern INT		gixFocusPage;

extern  UINT	gbUniPad;

static INT		gdDiffLock;

CONST  TCHAR	gaatDotPtrnPeriod[11][9] = {
	{ TEXT("　　") },
	{ TEXT("　....") },
	{ TEXT(" 　 .") },
	{ TEXT("　　.") },
	{ TEXT("　.....") },
	{ TEXT("　　 ") },
	{ TEXT("　　..") },
	{ TEXT("　......") },
	{ TEXT("　　 .") },
	{ TEXT("　　...") },
	{ TEXT("　 　 ") }
};
#define RIGHT_WALL	TEXT('|')

CONST  TCHAR	gaatDotPtrnUnic[11][6] = {
	{ TEXT("　　") },
	{ 0x3000,0x3000,0x200A },
	{ 0x3000,0x3000,0x2009 },
	{ 0x3000,0x3000,0x2006 },
	{ 0x3000,0x3000,0x2005 },
	{ TEXT("　　 ") },
	{ 0x3000,0x0020,0x3000,0x200A },
	{ 0x3000,0x0020,0x3000,0x2009 },
	{ 0x3000,0x0020,0x3000,0x2006 },
	{ 0x3000,0x0020,0x3000,0x2005 },
	{ TEXT("　 　 ") }
};

CONST TCHAR gaatPaddingSpDot[34][9] = {
	{ TEXT("")    },
	{ TEXT(".")   },
	{ TEXT(".")   },
	{ TEXT(".")   },
	{ TEXT(".")   },
	{ TEXT(" ")   },
	{ TEXT("..")  },
	{ TEXT("..")  },
	{ TEXT(". ")  },
	{ TEXT("...") },
	{ TEXT("　")  },
	{ TEXT("　")  },
	{ TEXT("　")  },
	{ TEXT(".　") },
	{ TEXT(".　") },
	{ TEXT("　 ") },
	{ TEXT("　 ")  },
	{ TEXT(".　.") },
	{ TEXT(". 　") },
	{ TEXT(". 　") },
	{ TEXT("..　.") },
	{ TEXT(" 　 ") },
	{ TEXT("　　") },
	{ TEXT("　　")  },
	{ TEXT("　 . ") },
	{ TEXT(".　　") },
	{ TEXT("　 　") },
	{ TEXT("　 　") },
	{ TEXT(".　　.")  },
	{ TEXT(".　 　")  },
	{ TEXT(".　 　")  },
	{ TEXT(".　.　.") },
	{ TEXT("　 　 ")  },
	{ TEXT("　　　") }
};

CONST TCHAR gaatPaddingSpDotW[11][3] = {
	{ TEXT("") },
	{ 8202 },
	{ 8201 },
	{ 8198 },
	{ 8197 },
	{ TEXT(' ') },
	{ 8202, TEXT(' ') },
	{ 8201, TEXT(' ') },
	{ 8194 },
	{ 8197, TEXT(' ') },
	{ 8199 }
};

UINT	SpaceWidthAdjust( INT, PINT, PINT );
LPTSTR	SpaceStrAlloc( INT, INT );

UINT	DocSpaceDifference( UINT, PINT, INT, UINT );

HRESULT	DocRightGuideSet( INT, INT );

LPTSTR	DocPaddingSpace( INT , PINT, PINT );

UINT SpaceWidthAdjust( INT dDot, PINT pZen, PINT pHan )
{
	INT		dZen, dHan, size;

	dZen = *pZen;
	dHan = *pHan;

	do
	{
		size = (dZen * SPACE_ZEN) + (dHan * SPACE_HAN);

		if( dDot == size )
		{
			*pZen = dZen;
			*pHan = dHan;
			return 1;
		}

		dZen--;
		dHan += 2;
	}
	while(  0 <= dZen );

	return 0;
}

LPTSTR SpaceStrAlloc( INT dZen, INT dHan )
{
	INT		cchSize, i;
	LPTSTR	ptStr;

	cchSize = dZen + dHan;
	ptStr = (LPTSTR)malloc( (cchSize + 1) * sizeof(TCHAR) );
	if( !ptStr )	return NULL;
	ZeroMemory( ptStr, (cchSize + 1) * sizeof(TCHAR) );

	for( i = (cchSize - 1); 0 <= i; )
	{
		if( 0 < dHan )
		{
			ptStr[i--] = TEXT(' ');
			dHan--;
			if( 0 >  i )	break;
		}

		if( 0 < dZen )
		{
			ptStr[i--] = TEXT('　');
			dZen--;
			if( 0 >  i )	break;
		}
	}

	return ptStr;
}

LPTSTR DocPaddingSpaceUni( INT dTgtDot, PINT pdZenSp, PINT pdHanSp, PINT pdUniSp )
{
	INT		dZen, dHan, dUni;
	INT		iCnt, iRem;
	INT		cchSize, i;
	LPTSTR	ptStr = NULL;

	if( 0 >= dTgtDot )	return NULL;

	iCnt =  dTgtDot / SPACE_ZEN;
	iRem =  dTgtDot % SPACE_ZEN;

	dZen = iCnt;

	switch( iRem )
	{
		case  1:	dUni = 1;	dHan = 0;	break;
		case  2:	dUni = 1;	dHan = 0;	break;
		case  3:	dUni = 1;	dHan = 0;	break;
		case  4:	dUni = 1;	dHan = 0;	break;
		case  5:	dUni = 0;	dHan = 1;	break;
		case  6:	dUni = 1;	dHan = 1;	break;
		case  7:	dUni = 1;	dHan = 1;	break;
		case  8:	dUni = 1;	dHan = 0;	break;
		case  9:	dUni = 1;	dHan = 1;	break;
		case 10:	dUni = 1;	dHan = 0;	break;
		default:	dUni = 0;	dHan = 0;	break;
	}

	cchSize = dZen + dHan + dUni;
	ptStr = (LPTSTR)malloc( (cchSize + 1) * sizeof(TCHAR) );
	if( !ptStr )	return NULL;
	ZeroMemory( ptStr, (cchSize + 1) * sizeof(TCHAR) );

	for( i = 0; dZen > i; i++ ){	ptStr[i] = TEXT('　');	}

	switch( iRem )
	{
		case  1:	ptStr[i++] = gaatPaddingSpDotW[1][0];	break;
		case  2:	ptStr[i++] = gaatPaddingSpDotW[2][0];	break;
		case  3:	ptStr[i++] = gaatPaddingSpDotW[3][0];	break;
		case  4:	ptStr[i++] = gaatPaddingSpDotW[4][0];	break;
		case  5:	ptStr[i++] = gaatPaddingSpDotW[5][0];	break;
		case  6:	ptStr[i++] = gaatPaddingSpDotW[6][0];	ptStr[i++] = gaatPaddingSpDotW[6][1];	break;
		case  7:	ptStr[i++] = gaatPaddingSpDotW[7][0];	ptStr[i++] = gaatPaddingSpDotW[7][1];	break;
		case  8:	ptStr[i++] = gaatPaddingSpDotW[8][0];	break;
		case  9:	ptStr[i++] = gaatPaddingSpDotW[9][0];	ptStr[i++] = gaatPaddingSpDotW[9][1];	break;
		case 10:	ptStr[i++] = gaatPaddingSpDotW[10][0];	break;
		default:	break;
	}

	if( pdZenSp )	*pdZenSp = dZen;
	if( pdHanSp )	*pdHanSp = dHan;
	if( pdUniSp )	*pdUniSp = dUni;

	return ptStr;
}

LPTSTR DocPaddingSpace( INT dTgtDot, PINT pdZenSp, PINT pdHanSp )
{
	INT		dZen, dHan;
	INT		iCnt, iRem;
	UINT	dRslt;
	LPTSTR	ptStr = NULL;

	if( 0 >= dTgtDot )	return NULL;

	iCnt =  dTgtDot / SPACE_ZEN;
	iRem =  dTgtDot % SPACE_ZEN;

	dZen = iCnt;

	if( 1 <= iRem && iRem <= 5 )
	{
		dHan = 1;
	}
	else if( 6 <= iRem && iRem <= 10 )
	{
		dHan = 0;
		dZen++;
	}
	else
	{
		dHan = 0;
	}

	dRslt = SpaceWidthAdjust( dTgtDot, &dZen, &dHan );

	if( dRslt )
	{
		if( pdZenSp )	*pdZenSp = dZen;
		if( pdHanSp )	*pdHanSp = dHan;

		ptStr = SpaceStrAlloc( dZen, dHan );

		return ptStr;
	}

	return NULL;
}

LPTSTR DocPaddingSpaceWithGap( INT dTgtDot, PINT pdZenSp, PINT pdHanSp )
{
	INT		cchSize, i;
	LPTSTR	ptStr = NULL;

	if( 16 <= dTgtDot )
	{
		i = 0;

		do
		{
			if( 22 < i )	return NULL;

			ptStr = DocPaddingSpace( dTgtDot, pdZenSp, pdHanSp );
			dTgtDot++;	i++;

		}while( !(ptStr) );

		return ptStr;
	}

	cchSize = 1;
	ptStr = (LPTSTR)malloc( (cchSize + 1) * sizeof(TCHAR) );
	if( !ptStr )	return NULL;

	if( 7 >= dTgtDot )
	{
		ptStr[0] = TEXT(' ');
		if( pdZenSp )	*pdZenSp = 0;
		if( pdHanSp )	*pdHanSp = 1;
	}
	else if( 8 <= dTgtDot && dTgtDot <= 15 )
	{
		ptStr[0] = TEXT('　');
		if( pdZenSp )	*pdZenSp = 1;
		if( pdHanSp )	*pdHanSp = 0;
	}
	ptStr[1] = 0x0000;

	return ptStr;
}

INT DocLineStateCheckWithDot( INT dDot, INT rdLine, PINT pLeft, PINT pRight, PINT pStCnt, PINT pCount, PBOOLEAN pIsSp )
{
	UINT_PTR	iCount;
	INT		bgnDot, endDot;
	INT		iBgnCnt, iRngCnt;
	TCHAR	ch, chb;
	UINT	dMozis;
	INT		bSpace;
	LETR_ITR	itMozi, itHead, itTail, itTemp;

	LINE_ITR	itLine;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	if( !(pLeft) || !(pRight) || !(pIsSp) ){	return 0;	}

	itMozi = itLine->vcLine.begin( );
	iCount = itLine->vcLine.size( );

	if( 0 >= iCount ){	*pIsSp =  FALSE;	*pLeft =  0;	*pRight = 0;	return 0;	}

	dMozis = DocLetterPosGetAdjust( &dDot , rdLine, 0 );

	if( 1 <= dMozis ){	itMozi += (dMozis-1);	}

	ch = itMozi->cchMozi;
	bSpace = iswspace( ch );

	itHead = itLine->vcLine.begin( );

	for( ; itHead != itMozi; itMozi-- )
	{
		chb = itMozi->cchMozi;
		if( iswspace( chb ) != bSpace ){	itMozi++;	break;	}
	}

	if( itHead == itMozi )
	{
		chb = itMozi->cchMozi;
		if( iswspace( chb ) != bSpace ){	itMozi++;	}
	}

	bgnDot = 0;
	iBgnCnt = 0;
	for( itTemp = itHead; itTemp != itMozi; itTemp++ )
	{
		bgnDot += itTemp->rdWidth;
		iBgnCnt++;
	}

	itTail = itLine->vcLine.end( );

	endDot = bgnDot;
	iRngCnt = 0;
	for( ; itTemp != itTail; itTemp++ )
	{
		chb = itTemp->cchMozi;
		if( iswspace( chb ) != bSpace ){	break;	}

		endDot += itTemp->rdWidth;
		iRngCnt++;
	}

	*pLeft  = bgnDot;
	*pRight = endDot;
	*pIsSp  = bSpace ? TRUE : FALSE;

	if( pCount )	*pCount = iRngCnt;
	if( pStCnt )	*pStCnt = iBgnCnt;

	return (endDot - bgnDot);
}

UINT DocSpaceDifference( UINT vk, PINT pXdot, INT dLine, UINT dFirst )
{
	INT			dTgtDot, dNowDot;
	INT			dBgnDot, dEndDot;
	INT			dBgnCnt, dRngCnt;
	UINT_PTR	cchSize;
	BOOLEAN		bIsSpace;
	LPTSTR		ptSpace;
	INT			dZenSp, dHanSp, dUniSp;
	INT			iDots, iBytes;

	wstring		wsBuffer;
	LETR_ITR	vcLtrBgn, vcLtrEnd, vcItr;

	LINE_ITR	itLine;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, dLine );

	dNowDot = *pXdot;

	if( 0 == dNowDot )
	{
		dNowDot = itLine->vcLine.at( 0 ).rdWidth;
	}

	dTgtDot = DocLineStateCheckWithDot( dNowDot, dLine, &dBgnDot, &dEndDot, &dBgnCnt, &dRngCnt, &bIsSpace );
	if( !(bIsSpace) )	return 0;

	if( VK_RIGHT == vk )		dTgtDot++;
	else if( VK_LEFT == vk )	dTgtDot--;
	else	return 0;

	ptSpace = DocPaddingSpace( dTgtDot, &dZenSp, &dHanSp );
	if( gbUniPad )
	{

		if( !(ptSpace) || (dZenSp < dHanSp) )
		{
			FREE(ptSpace);
			ptSpace = DocPaddingSpaceUni( dTgtDot, &dZenSp, &dHanSp, &dUniSp );
		}
	}

	if( !(ptSpace) )	return 0;

	StringCchLength( ptSpace, STRSAFE_MAX_CCH, &cchSize );

	vcLtrBgn  = itLine->vcLine.begin( );
	vcLtrBgn += dBgnCnt;
	vcLtrEnd  = vcLtrBgn;
	vcLtrEnd += dRngCnt;

	iDots = 0;	iBytes = 0;
	wsBuffer.clear();
	for( vcItr = vcLtrBgn; vcLtrEnd != vcItr; vcItr++ )
	{
		wsBuffer += vcItr->cchMozi;
		iDots    += vcItr->rdWidth;
		iBytes   += vcItr->mzByte;
	}

	itLine->vcLine.erase( vcLtrBgn, vcLtrEnd );
	itLine->iByteSz -= iBytes;	if( 0 > itLine->iByteSz ){	itLine->iByteSz = 0;	}
	itLine->iDotCnt -= iDots;	if( 0 > itLine->iDotCnt ){	itLine->iDotCnt = 0;	}

	dNowDot = dBgnDot;
	DocStringAdd( &dNowDot, &dLine, ptSpace, cchSize );

	*pXdot = dNowDot;

	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, wsBuffer.c_str( ), dBgnDot, dLine, dFirst );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptSpace, dBgnDot, dLine, FALSE );

	FREE( ptSpace );

	return dTgtDot;
}

INT DocSpaceShiftProc( UINT vk, PINT pXdot, INT dLine )
{
	INT		dDot, dMozi, dPreByte;

	dDot = DocLineParamGet( dLine, &dMozi, &dPreByte );
	if( 0 >= dMozi )	return 0;

	dDot = DocSpaceDifference( vk, pXdot, dLine, TRUE );

	DocLetterPosGetAdjust( pXdot, dLine, 0 );

	ViewRedrawSetLine( dLine );

	ViewDrawCaret( *pXdot, dLine, 1 );

	return dDot;
}

HRESULT	DocRightGuideline( LPVOID pVoid )
{
	INT	iTop, iBottom, i;

	TRACE( TEXT("오른쪽 정렬선") );

	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;

	DocRightGuideSet( iTop, iBottom );

	ViewSelPageAll( -1 );

	if( 0 > iTop || 0 > iBottom ){	ViewRedrawSetLine( -1 );	}
	else{	for( i =  iTop; iBottom >= i; i++ ){	ViewRedrawSetLine(  i );	}	}

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

HRESULT DocRightGuideSet( INT dTop, INT dBottom )
{

	UINT_PTR	iLines, cchSize;
	INT			baseDot, i, j, iMz, nDot, sDot, lDot, iUnt, iPadot;
	TCHAR		ch, atBuffer[MAX_PATH];
	LPTSTR		ptBuffer;
	BOOLEAN		bFirst;
	wstring		wsBuffer;

	iLines = DocNowFilePageLineCount( );
	if( 0 > dTop )		dTop = 0;
	if( 0 > dBottom )	dBottom = iLines - 1;

	ZeroMemory( atBuffer, sizeof(atBuffer) );
	atBuffer[0] = RIGHT_WALL;
	InitParamString( INIT_LOAD, VS_RGUIDE_MOZI, atBuffer );

	baseDot = DocPageMaxDotGet( dTop, dBottom );

	bFirst = TRUE;

	for( i = dTop; dBottom >= i; i++ )
	{
		nDot = DocLineParamGet( i , NULL, NULL );
		sDot = baseDot - nDot;
		iUnt = sDot / SPACE_ZEN;
		sDot = sDot % SPACE_ZEN;

		iPadot = nDot;
		wsBuffer.clear( );

		for( j = 0; iUnt > j; j++ )
		{
			ch = TEXT('　');
			wsBuffer += ch;
			lDot  = DocInputLetter( nDot, i, ch );
			nDot += lDot;
		}

		if( gbUniPad  ){	iMz = lstrlen( gaatDotPtrnUnic[sDot]  );	}
		else{				iMz = lstrlen( gaatDotPtrnPeriod[sDot] );	}

		for( j = 0; iMz > j; j++ )
		{
			if( gbUniPad  ){	ch = gaatDotPtrnUnic[sDot][j];	}
			else{			ch = gaatDotPtrnPeriod[sDot][j];	}

			wsBuffer += ch;
			lDot  = DocInputLetter( nDot, i, ch );
			nDot += lDot;
		}

		wsBuffer += atBuffer[0];
		lDot  = DocInputLetter( nDot, i, atBuffer[0] );
		nDot += lDot;

		DocBadSpaceCheck( i );

		cchSize = wsBuffer.size( ) + 1;
		ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
		StringCchCopy( ptBuffer, cchSize, wsBuffer.c_str(  ) );

		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptBuffer, iPadot, i, bFirst );
		bFirst = FALSE;

		FREE( ptBuffer );
	}

	return S_OK;
}

INT DocDiffAdjBaseSet( INT yLine )
{
	INT	dDot = 0;
	TCHAR	atMessage[MAX_STRING];

	TRACE( TEXT("자동 조정 기준점 고정") );

	dDot = DocLineParamGet( yLine, NULL, NULL );

	gdDiffLock = dDot;

	StringCchPrintf( atMessage, MAX_STRING, TEXT("조정 기준 위치를 %d 도트로 설정했습니다"), dDot );
	NotifyBalloonExist( atMessage, TEXT("고정"), NIIF_INFO );

	return dDot;
}

INT DocDiffAdjExec( PINT pxDot, INT yLine )
{
	INT			dMotoDot = 0;
	INT			dBgnDot, dEndDot, dBgnCnt, dRngCnt, iSabun, dTgtDot, nDot;
	UINT_PTR	cchSize, cchPlus;
	BOOLEAN		bIsSpace;
	LPTSTR		ptPlus, ptBuffer;

	wstring		wsDelBuf, wsAddBuf;
	LETR_ITR	vcLtrBgn, vcLtrEnd, vcItr;

	LINE_ITR	itLine;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, yLine );

	dTgtDot = DocLineStateCheckWithDot( *pxDot, yLine, &dBgnDot, &dEndDot, &dBgnCnt, &dRngCnt, &bIsSpace );
	if( !(bIsSpace) )
	{
		NotifyBalloonExist( TEXT("연속된 공백 부분에 커서를 맞추세요"), TEXT("조정 불가능"), NIIF_ERROR );
		return 0;
	}

	dMotoDot = DocLineParamGet( yLine, NULL, NULL );
	iSabun = gdDiffLock - dMotoDot;

	dTgtDot += iSabun;

	if( 41 > dTgtDot )
	{
		NotifyBalloonExist( TEXT("조금 더 폭이 있어야 조정할 수 있습니다"), TEXT("너무 좁음"), NIIF_ERROR );
		return 0;
	}

	ptPlus = DocPaddingSpaceWithPeriod( dTgtDot, NULL, NULL, NULL, FALSE );

	if( !(ptPlus) )
	{
		NotifyBalloonExist( TEXT("조정할 수 없었습니다"), TEXT("자동 조정 실패"), NIIF_ERROR );
		return 0;
	}

	StringCchLength( ptPlus, STRSAFE_MAX_CCH, &cchPlus );

	vcLtrBgn  = itLine->vcLine.begin( );
	vcLtrBgn += dBgnCnt;
	vcLtrEnd  = vcLtrBgn;
	vcLtrEnd += dRngCnt;

	wsDelBuf.clear();
	for( vcItr = vcLtrBgn; vcLtrEnd != vcItr; vcItr++ ){	wsDelBuf +=  vcItr->cchMozi;	}

	itLine->vcLine.erase( vcLtrBgn, vcLtrEnd );
	nDot = dBgnDot;

	cchSize = wsDelBuf.size( ) + 1;
	ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	StringCchCopy( ptBuffer, cchSize, wsDelBuf.c_str( ) );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, dBgnDot, yLine, TRUE );
	FREE( ptBuffer );

	DocStringAdd( &nDot, &yLine, ptPlus, cchPlus );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptPlus, dBgnDot, yLine, FALSE );
	FREE(ptPlus);

	*pxDot = nDot;

	DocLetterPosGetAdjust( pxDot, yLine, 0 );

	ViewRedrawSetLine( yLine );

	ViewDrawCaret( *pxDot, yLine, 1 );

	return iSabun;
}

LPTSTR DocPaddingSpaceWithPeriod( INT dTgtDot, PINT pdZen, PINT pdHan, PINT pdPrd, BOOLEAN bFull )
{
	INT	dZenSp, dHanSp, dPrdSp, m, dPre;
	LPTSTR	ptSpace = NULL, ptPlus = NULL;
	UINT	cchSize, cchPlus;

	dPre = dTgtDot;
	dPrdSp = 0;

	do{
		dZenSp =  0;	dHanSp =  0;
		ptSpace = DocPaddingSpace( dTgtDot, &dZenSp, &dHanSp );

		if( !(ptSpace) || (dZenSp < dHanSp) )
		{
			FREE(ptSpace);
			if( gbUniPad )
			{
				ptSpace = DocPaddingSpaceUni( dTgtDot, &dZenSp, &dHanSp, NULL );
				break;
			}
			else
			{
				dPrdSp++;	dTgtDot -= 3;
			}
		}
		else
		{
			break;
		}

	}while( dTgtDot >= 19 );

	if( !(ptSpace) && bFull )
	{
		dPrdSp = 0;
		dTgtDot = dPre;

		StringCchLength( gaatPaddingSpDot[dTgtDot], STRSAFE_MAX_CCH, &cchSize );

		cchSize += 1;
		ptSpace = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
		ZeroMemory( ptSpace, cchSize * sizeof(TCHAR) );

		StringCchCopy( ptSpace, cchSize, gaatPaddingSpDot[dTgtDot] );
	}

	if( ptSpace )
	{
		StringCchLength( ptSpace, STRSAFE_MAX_CCH, &cchSize );

		cchPlus = cchSize + dPrdSp + 1;
		ptPlus = (LPTSTR)malloc( cchPlus * sizeof(TCHAR) );
		ZeroMemory( ptPlus, cchPlus * sizeof(TCHAR) );

		StringCchCopy( ptPlus, cchPlus, ptSpace );
		FREE(ptSpace);

		for( m = 0; dPrdSp > m; m++ ){	StringCchCat( ptPlus , cchPlus, TEXT(".") );	}
	}

	if( pdZen  )	*pdZen = dZenSp;
	if( pdHan  )	*pdHan = dHanSp;
	if( pdPrd  )	*pdPrd = dPrdSp;

	return ptPlus;
}

HRESULT DocTopLetterInsert( TCHAR ch, PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i, xDot = 0;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE;

	TRACE( TEXT("행 머리 공백 추가") );

	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	for( i = iTop; iBottom >= i; i++ )
	{

		xDot = DocInputLetter( 0, i, ch );

		SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ch, 0, i, bFirst );
		bFirst = FALSE;

		if( bSeled )
		{
			DocRangeSelStateToggle( -1, -1, i, 1 );
			DocReturnSelStateToggle( i, 1 );
		}

		DocBadSpaceCheck( i );
		ViewRedrawSetLine( i );
	}

	*pXdot += xDot;
	DocLetterPosGetAdjust( pXdot, dLine, 0 );
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

HRESULT DocTopSpaceErase( PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE;
	TCHAR		ch;

	LETR_ITR	vcLtrItr;

	LINE_ITR	itLine;

	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	TRACE( TEXT("행 머리 공백 삭제") );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{

		if( 0 != itLine->vcLine.size(  ) )
		{
			vcLtrItr = itLine->vcLine.begin( );
			ch = vcLtrItr->cchMozi;

			if( ( iswspace( ch ) && TEXT(' ') != ch ) )
			{
				SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, 0, i, bFirst );
				bFirst = FALSE;

				DocIterateDelete( vcLtrItr, i );
			}
		}

		if( bSeled )
		{
			DocRangeSelStateToggle( -1, -1, i, 1 );
			DocReturnSelStateToggle( i, 1 );
		}

		DocBadSpaceCheck( i );
		ViewRedrawSetLine( i );
	}

	*pXdot = 0;
	DocLetterPosGetAdjust( pXdot, dLine, 0 );
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

HRESULT DocLastLetterErase( PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i, xDot = 0;
	TCHAR		ch;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE;
	RECT		rect;

	LETR_ITR	vcLtrItr;

	LINE_ITR	itLine;

	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	TRACE( TEXT("행 끝 문자 삭제") );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{

		if( 0 != itLine->vcLine.size( ) )
		{
			vcLtrItr = itLine->vcLine.end( );
			vcLtrItr--;
			ch = vcLtrItr->cchMozi;

			rect.top    = i * LINE_HEIGHT;
			rect.bottom = rect.top + LINE_HEIGHT;

			if( !( iswspace( ch ) ) )
			{
				xDot  = DocLineParamGet( i, NULL, NULL );

				xDot -= vcLtrItr->rdWidth;

				SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, xDot, i, bFirst );
				bFirst = FALSE;

				DocIterateDelete( vcLtrItr, i );

				rect.left  = xDot;
				rect.right = xDot + 40;

				ViewRedrawSetRect( &rect );

				DocBadSpaceCheck( i );
			}
		}

		if( bSeled )
		{
			DocRangeSelStateToggle( -1, -1, i , 1 );
			DocReturnSelStateToggle( i, 1 );
		}
	}

	*pXdot = 0;
	DocLetterPosGetAdjust( pXdot, dLine, 0 );
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

HRESULT DocLastSpaceErase( PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i, xDelDot, xMotoDot;
	BOOLEAN		bFirst = TRUE;
	LPTSTR		ptBuffer = NULL;
	RECT		rect;

	LINE_ITR	itLine;

	TRACE( TEXT("행 끝 공백 삭제") );

	iLines = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	ViewSelPageAll( -1 );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{
		xMotoDot = itLine->iDotCnt;
		ptBuffer = DocLastSpDel( &(itLine->vcLine) );
		xDelDot  = DocLineParamGet( i, NULL, NULL );

		if( ptBuffer  )
		{
			SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, xDelDot, i , bFirst );
			bFirst = FALSE;
		}

		FREE( ptBuffer );

		DocBadSpaceCheck( i );

		rect.top    = i * LINE_HEIGHT;
		rect.bottom = rect.top + LINE_HEIGHT;
		rect.left   = xDelDot;
		rect.right  = xMotoDot + 20;
		ViewRedrawSetRect( &rect );

	}

	DocLetterPosGetAdjust( pXdot, dLine, 0 );
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

LPTSTR DocLastSpDel( vector<LETTER> *vcTgLine )
{
	UINT_PTR	cchSize;
	LPTSTR		ptBuffer = NULL;
	wstring		wsDelBuf;
	LETR_ITR	itLtr, itDel;

	if( 0 >= vcTgLine->size( ) )	return NULL;

	itLtr = vcTgLine->end( );
	itLtr--;

	for( ; itLtr != vcTgLine->begin(); itLtr-- )
	{
		if( !( iswspace( itLtr->cchMozi ) ) )
		{
			itLtr++;
			break;
		}
	}
	if( itLtr == vcTgLine->begin() )
	{

		if( !( iswspace( itLtr->cchMozi ) ) ){	itLtr++;	}
	}

	if( itLtr == vcTgLine->end( ) ){	return NULL;	}

	wsDelBuf.clear();
	for( itDel = itLtr; vcTgLine->end( ) != itDel; itDel++ ){	wsDelBuf +=  itDel->cchMozi;	}

	cchSize = wsDelBuf.size( ) + 1;
	ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	StringCchCopy( ptBuffer, cchSize, wsDelBuf.c_str( ) );

	vcTgLine->erase( itLtr, vcTgLine->end( ) );

	return ptBuffer;
}

UINT DocRangeDeleteByMozi( INT xDot, INT yLine, INT dBgnMozi, INT dEndMozi, PBOOLEAN pFirst )
{
	UINT_PTR	cchSize;
	INT			iBytes;
	LPTSTR		ptBuffer;
	LETR_ITR	vcLtrBgn, vcLtrEnd, vcItr;
	wstring		wsDelBuf;

	LINE_ITR	itLine;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, yLine );

	vcLtrBgn  = itLine->vcLine.begin( );
	vcLtrEnd  = itLine->vcLine.begin( );
	vcLtrBgn += dBgnMozi;
	vcLtrEnd += dEndMozi;

	wsDelBuf.clear();
	iBytes = 0;
	for( vcItr = vcLtrBgn; vcLtrEnd != vcItr; vcItr++ )
	{
		wsDelBuf += vcItr->cchMozi;
		iBytes   += vcItr->mzByte;
	}

	itLine->vcLine.erase( vcLtrBgn, vcLtrEnd );
	itLine->iByteSz -= iBytes;

	cchSize = wsDelBuf.size( ) + 1;
	ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	StringCchCopy( ptBuffer, cchSize, wsDelBuf.c_str( ) );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, xDot, yLine, *pFirst );
	FREE( ptBuffer );	*pFirst = FALSE;

	return (UINT)(cchSize - 1);
}

HRESULT DocRightSlide( PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i;
	INT			dSliDot, dRitDot, dPaDot, dInBgn;
	INT			dMozi, dLefDot, dAdDot;
	BOOLEAN		bFirst = TRUE;
	LPTSTR		ptBuffer = NULL;

	LINE_ITR	itLine;

	TRACE( TEXT("오른쪽 정렬") );

	dSliDot = InitParamValue( INIT_LOAD, VL_RIGHT_SLIDE, 790 );

	iLines = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	ViewSelPageAll( -1 );

	dRitDot = DocPageMaxDotGet( iTop, iBottom );

	dPaDot = dSliDot - dRitDot;
	if( 0 > dPaDot )
	{
		NotifyBalloonExist( TEXT("넘쳤습니다"), TEXT("실패"), NIIF_ERROR );
		return E_FAIL;
	}

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{
		dAdDot = dPaDot;

		dLefDot = LayerHeadSpaceCheck( &(itLine->vcLine), &dMozi );
		if( 0 < dLefDot )
		{
			dAdDot += dLefDot;

			DocRangeDeleteByMozi( 0, i, 0, dMozi, &bFirst );	bFirst = FALSE;
		}

		dInBgn = 0;
		ptBuffer = DocPaddingSpaceWithPeriod( dAdDot, NULL, NULL, NULL, TRUE );
		DocInsertString( &dInBgn, &i, NULL, ptBuffer, 0, bFirst );	bFirst = FALSE;
		FREE(ptBuffer);

		ViewRedrawSetLine( i );
	}

	*pXdot = 0;
	DocLetterPosGetAdjust( pXdot, dLine, 0 );
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

LPTSTR DocPaddingSpaceMake( INT dTgtDot )
{
	LPTSTR	ptReplc = NULL;
	INT		dZenSp, dHanSp, dUniSp;

	if( 0 >= dTgtDot )	return NULL;

	if( gbUniPad )
	{
		ptReplc = DocPaddingSpace( dTgtDot, &dZenSp, &dHanSp );

		if( !(ptReplc) || (dZenSp < dHanSp) )
		{
			FREE(ptReplc);
			ptReplc = DocPaddingSpaceUni( dTgtDot, &dZenSp, &dHanSp, &dUniSp );
		}
	}
	else
	{
		ptReplc = DocPaddingSpaceWithGap( dTgtDot, &dZenSp, &dHanSp );
	}

	return ptReplc;
}

HRESULT DocPositionShift( UINT vk, PINT pXdot, INT dLine )
{
	UINT_PTR	iLines, cchSz;
	INT			iTop, iBottom, i;
	INT			wid, iDot, iLin, iMzCnt;
	INT			iTgtWid, iLefDot, iRitDot;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE, bDone = FALSE;
	BOOLEAN		bRight;
	BOOLEAN		bIsSp;
	LPTSTR		ptRepl;
	TCHAR		ch, chOneSp;

	LPUNDOBUFF	pstUndoBuff;

	LETR_ITR	vcLtrItr;
	LINE_ITR	itLine;

	chOneSp = gaatPaddingSpDotW[1][0];

	pstUndoBuff = &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog);

	TRACE( TEXT("전체 이동") );

	if( VK_RIGHT == vk )		bRight = TRUE;
	else if( VK_LEFT == vk )	bRight = FALSE;
	else	return E_INVALIDARG;

	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	if( bSeled ){	DocPageSelStateToggle( -1 );	}

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{

		if( 0 != itLine->vcLine.size(  ) )
		{

			vcLtrItr = itLine->vcLine.begin( );
			ch  = vcLtrItr->cchMozi;
			wid = vcLtrItr->rdWidth;

			bDone = FALSE;

			if( !(iswspace(ch)) )
			{
				if( bRight )
				{

					DocInputLetter( 0, i, chOneSp );
					SqnAppendLetter( pstUndoBuff, DO_INSERT, chOneSp, 0, i, bFirst );	bFirst = FALSE;
					bDone = TRUE;
				}
				else
				{
					ptRepl = DocPaddingSpaceMake( wid );
					StringCchLength( ptRepl, STRSAFE_MAX_CCH, &cchSz );

					SqnAppendLetter( pstUndoBuff, DO_DELETE, ch, 0, i, bFirst );	bFirst = FALSE;
					DocIterateDelete( vcLtrItr, i );

					iDot = 0;	iLin = i;
					DocStringAdd( &iDot, &iLin, ptRepl, cchSz );
					SqnAppendString( pstUndoBuff, DO_INSERT, ptRepl, 0, i, bFirst );	bFirst = FALSE;
					FREE(ptRepl);
				}
			}

			if( !(bDone) )
			{

				iTgtWid = DocLineStateCheckWithDot( 0, i, &iLefDot, &iRitDot, NULL, &iMzCnt, &bIsSp );
				if( bRight )	iTgtWid++;
				else			iTgtWid--;
				if( 0 > iTgtWid )	iTgtWid = 0;

				ptRepl = DocPaddingSpaceMake( iTgtWid );

				DocRangeDeleteByMozi( 0, i, 0, iMzCnt, &bFirst );

				if( ptRepl )
				{
					StringCchLength( ptRepl, STRSAFE_MAX_CCH, &cchSz );
					iDot = 0;	iLin = i;
					DocStringAdd( &iDot, &iLin, ptRepl, cchSz );
					SqnAppendString( pstUndoBuff, DO_INSERT, ptRepl, 0, i, bFirst );	bFirst = FALSE;
					FREE(ptRepl);
				}
			}

			if( bSeled )
			{
				DocRangeSelStateToggle( -1, -1, i , 1 );

				if( iBottom > i )	DocReturnSelStateToggle( i, 1 );
			}

			DocBadSpaceCheck( i );
			ViewRedrawSetLine( i );
		}
	}

	if( bSeled )
	{
		(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop    = iTop;
		(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom = iBottom;
	}

	iDot = 0;
	DocLetterPosGetAdjust( &iDot, dLine, 0 );
	ViewDrawCaret( iDot, dLine, 1 );

	DocPageByteCount( gitFileIt, gixFocusPage, NULL, NULL );
	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

HRESULT DocHeadHalfSpaceExchange( HWND hWnd )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i;
	INT			xDot;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE;
	TCHAR		ch;

	LETR_ITR	vcLtrItr;
	LINE_ITR	itLine;

	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	TRACE( TEXT("행 머리 반각을 유니코드로") );

	ViewSelPageAll( -1 );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{

		if( 0 != itLine->vcLine.size(  ) )
		{
			vcLtrItr = itLine->vcLine.begin( );
			ch = vcLtrItr->cchMozi;

			if( TEXT(' ') == ch )
			{

				SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, 0, i, bFirst );
				bFirst = FALSE;
				DocIterateDelete( vcLtrItr, i );

				ch  = (TCHAR)0x2004;
				xDot = DocInputLetter( 0, i, ch );
				SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ch, 0, i, bFirst );
			}

			DocBadSpaceCheck( i );
			ViewRedrawSetLine( i );
		}
	}

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

#ifdef DOT_SPLIT_MODE

HRESULT DocCentreWidthShift( UINT vk, PINT pXdot, INT dLine )
{
	UINT_PTR	iLines, cchSz;
	UINT		dRslt, dFirst;
	INT			iBaseDot, iTop, iBottom, iBufDot;
	INT			wid, iDot, iLin, iMzCnt;
	INT			iFnlDot;
	BOOLEAN		bSeled = FALSE;
	BOOLEAN		bRight;
	LPTSTR		ptRepl;
	TCHAR		ch, chOneSp;

	LPUNDOBUFF	pstUndoBuff;

	LETR_ITR	vcLtrItr;
	LINE_ITR	itLine;

	chOneSp = gaatPaddingSpDotW[1][0];

	pstUndoBuff = &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog);

	iBaseDot = *pXdot;
	TRACE( TEXT("중간 이동 %dDOT"), iBaseDot );

	iFnlDot = iBaseDot;

	if( VK_RIGHT == vk )		bRight = TRUE;
	else if( VK_LEFT == vk )	bRight = FALSE;
	else	return E_INVALIDARG;

	if(  0 == iBaseDot )
	{
		return DocPositionShift( vk, pXdot, dLine );
	}

	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	if( bSeled ){	DocPageSelStateToggle( -1 );	}

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	dFirst = TRUE;

	for( iLin = iTop; iBottom >= iLin; iLin++, itLine++ )
	{
		iDot = itLine->iDotCnt;
		if( iBaseDot >=  iDot ){	continue;	}

		iDot = iBaseDot;
		iMzCnt = DocLetterPosGetAdjust( &iDot, iLin, -1 );

		iBufDot = iDot;
		dRslt = DocSpaceDifference( vk, &iBufDot, iLin, dFirst );
		if( dRslt  )
		{
			if( iLin == dLine ){	iFnlDot =  iBaseDot;	}
			dFirst = FALSE;
		}
		else
		{
			vcLtrItr = itLine->vcLine.begin( );
			std::advance( vcLtrItr, iMzCnt );
			ch  = vcLtrItr->cchMozi;
			wid = vcLtrItr->rdWidth;

			if( bRight )
			{
				if( iswspace( ch ) )
				{
					iBufDot = iDot + wid;
					DocSpaceDifference( vk, &iBufDot, iLin, dFirst );
					if( iLin == dLine ){	iFnlDot =  iBaseDot;	}
				}
				else
				{
					SqnAppendLetter( pstUndoBuff, DO_INSERT, chOneSp, iDot, iLin, dFirst );
					DocInputLetter( iDot, iLin, chOneSp );
					if( iLin == dLine ){	iFnlDot = iDot +  1;	}
				}
				dFirst = FALSE;
			}
			else
			{
				if( iLin == dLine ){	iFnlDot =  iDot;	}

				SqnAppendLetter( pstUndoBuff, DO_DELETE, ch, iDot, iLin, dFirst );	dFirst = FALSE;
				DocIterateDelete( vcLtrItr , iLin );
				if( 2 <= wid )
				{
					ptRepl = DocPaddingSpaceMake( wid-1 );
					StringCchLength( ptRepl , STRSAFE_MAX_CCH, &cchSz );
					SqnAppendString( pstUndoBuff, DO_INSERT, ptRepl, iDot, iLin, dFirst );	dFirst = FALSE;
					DocStringAdd( &iDot, &iLin, ptRepl, cchSz );
					FREE(ptRepl);
				}
			}
		}

		if( bSeled )
		{
			if( iLin == iTop )
			{
				iDot = iBaseDot;
				DocLetterPosGetAdjust( &iDot, iLin, 0 );
				DocRangeSelStateToggle( iDot, -1, iLin , 1 );
			}
			else
			{
				DocRangeSelStateToggle( -1, -1, iLin , 1 );
			}

			if( iBottom > iLin )	DocReturnSelStateToggle( iLin, 1 );
		}

		DocBadSpaceCheck( iLin );
		ViewRedrawSetLine( iLin );
	}

	if( bSeled )
	{
		(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop    = iTop;
		(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom = iBottom;
	}

	DocLetterPosGetAdjust( &iFnlDot, dLine, 0 );
	*pXdot = iFnlDot;
	ViewDrawCaret( iFnlDot, dLine, 1 );

	DocPageByteCount( gitFileIt, gixFocusPage, NULL, NULL );
	DocPageInfoRenew( -1, 1 );

	return S_OK;
}

#endif
