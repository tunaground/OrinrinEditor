#include "stdafx.h"
#include "OrinrinEditor.h"

extern FILES_ITR	gitFileIt;

extern INT		gixFocusPage;

static BOOLEAN	gbGroupUndo;

INT	SqnUndoExec( LPUNDOBUFF, PINT, PINT );
INT	SqnRedoExec( LPUNDOBUFF, PINT, PINT );

INT DocUndoExecute( PINT pxDot, PINT pyLine )
{
	INT	iRslt = 0;

	iRslt = SqnUndoExec( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), pxDot, pyLine );

	DocModifyContent( TRUE );

	return iRslt;
}

INT DocRedoExecute( PINT pxDot, PINT pyLine )
{
	INT	iRslt = 0;

	iRslt = SqnRedoExec( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), pxDot, pyLine );

	DocModifyContent( TRUE );

	return iRslt;
}

HRESULT SqnSetting( VOID )
{
	gbGroupUndo = InitParamValue( INIT_LOAD, VL_GROUP_UNDO, 1 );

	return S_OK;
}

HRESULT SqnInitialise( LPUNDOBUFF pstBuff )
{
	pstBuff->dNowSqn = 0;
	pstBuff->dTopSqn = 0;
	pstBuff->dGrpSqn = 0;

	pstBuff->vcOpeSqn.clear( );

	return S_OK;
}

HRESULT SqnFreeAll( LPUNDOBUFF pstBuff )
{
	UINT_PTR	iCount, i;

	iCount = pstBuff->vcOpeSqn.size( );

	for( i = 0; iCount > i; i++ )
	{
		free( pstBuff->vcOpeSqn.at( i ).ptText );
	}

	SqnInitialise( pstBuff );

	return S_OK;
}

HRESULT SqnNumberCheck( LPUNDOBUFF pstBuff )
{
	OPSQ_ITR	itOpe, itBuf;

	if( pstBuff->dNowSqn == pstBuff->dTopSqn )	return S_FALSE;

	if( 0 == pstBuff->dNowSqn ){	SqnFreeAll( pstBuff );	return  S_FALSE;	}

	TRACE( TEXT("UNDO BUF 오류 %d %d"), pstBuff->dNowSqn, pstBuff->dTopSqn );

	itOpe = pstBuff->vcOpeSqn.end( );
	itOpe--;

	do
	{
		if( pstBuff->dNowSqn == itOpe->ixSequence ){	break;	}

		free( itOpe->ptText );
		itBuf = itOpe;
		itBuf--;
		pstBuff->vcOpeSqn.erase( itOpe );
		itOpe = itBuf;

	}
	while( itOpe != pstBuff->vcOpeSqn.begin( ) );

	pstBuff->dTopSqn = itOpe->ixSequence;

	TRACE( TEXT("UNDO BUF 확인 %d %d"), itOpe->ixSequence, pstBuff->dTopSqn );

	return S_OK;
}

UINT SqnAppendLetter( LPUNDOBUFF pstBuff, UINT dCmd, TCHAR ch, INT xDot, INT yLine, UINT bAlone )
{
	UINT	uRslt;
	TCHAR	atBuffer[3];

	ZeroMemory( atBuffer, sizeof(atBuffer) );
	atBuffer[0] = ch;

	uRslt = SqnAppendString( pstBuff, dCmd, atBuffer, xDot, yLine, bAlone );

	DocModifyContent( TRUE );

	return uRslt;
}

UINT SqnAppendSquare( LPUNDOBUFF pstBuff, UINT dCmd, LPCTSTR ptStr, LPPOINT pstPt, INT yLine, UINT bAlone )
{
	INT	i;
	UINT_PTR	cchMozi, cchSize;
	LPCTSTR		ptCaret, ptSprt;
	OPERATELOG	stOpe;

	SqnNumberCheck( pstBuff );

	if( bAlone ){	pstBuff->dGrpSqn += 1;	}

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

	ptCaret = ptStr;

	for( i = 0; yLine > i; i++ )
	{
		if( !( *ptCaret ) ){	break;	}

		pstBuff->dTopSqn += 1;

		ZeroMemory( &stOpe, sizeof(OPERATELOG) );
		stOpe.dCommando  = dCmd;
		stOpe.ixSequence = pstBuff->dTopSqn;
		stOpe.ixGroup    = pstBuff->dGrpSqn;

		stOpe.rdXdot  = pstPt[i].x;
		stOpe.rdYline = pstPt[i].y;

		ptSprt = StrStr( ptCaret, CH_CRLFW );
		if( !(ptSprt) ){	ptSprt = ptStr + cchSize;	}

		cchMozi = ptSprt - ptCaret;

		cchMozi++;
		stOpe.cchSize = cchMozi;
		stOpe.ptText  = (LPTSTR)malloc( cchMozi * sizeof(TCHAR) );
		StringCchCopy( stOpe.ptText, cchMozi, ptCaret );

		pstBuff->vcOpeSqn.push_back( stOpe );

		ptCaret = NextLineW( ptSprt );
	}

	pstBuff->dNowSqn = pstBuff->vcOpeSqn.size( );

	DocModifyContent( TRUE );

	return pstBuff->dNowSqn;
}

UINT SqnAppendString( LPUNDOBUFF pstBuff, UINT dCmd, LPCTSTR ptStr, INT xDot, INT yLine, UINT bAlone )
{
	UINT_PTR	cchSize;
	OPERATELOG	stOpe;

	SqnNumberCheck( pstBuff );

	pstBuff->dTopSqn += 1;

	stOpe.dCommando  = dCmd;
	stOpe.ixSequence = pstBuff->dTopSqn;

	if( bAlone ){	pstBuff->dGrpSqn += 1;	}
	stOpe.ixGroup    = pstBuff->dGrpSqn;

	stOpe.rdXdot  = xDot;
	stOpe.rdYline = yLine;

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );
	stOpe.cchSize = cchSize;
	cchSize++;
	stOpe.ptText  = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	StringCchCopy( stOpe.ptText, cchSize, ptStr );

	pstBuff->vcOpeSqn.push_back( stOpe );

	pstBuff->dNowSqn = pstBuff->vcOpeSqn.size( );

	DocModifyContent( TRUE );

	return pstBuff->dNowSqn;
}

INT SqnUndoExec( LPUNDOBUFF pstBuff, PINT pxDot, PINT pyLine )
{
	OPSQ_ITR	itSqn;
	INT		xDot, yLine, iRslt = 0, dCrLf = 0, yPreLine = 0;
	UINT	dCmd, dGrp, dNow, cchSize;
	UINT	dPreGroup = 0;
	LPTSTR	ptStr;

	if( !(pstBuff) )	return 0;

#ifdef DO_TRY_CATCH
	try{
#endif

	do
	{
		dNow  = pstBuff->dNowSqn;

		if( 0 >= dNow ){	return dCrLf;	}

		dNow--;
		TRACE( TEXT("UNDO SQNUM:%u"), dNow );

		dCmd  = pstBuff->vcOpeSqn.at( dNow ).dCommando;
		dGrp  = pstBuff->vcOpeSqn.at( dNow ).ixGroup;
		xDot  = pstBuff->vcOpeSqn.at( dNow ).rdXdot;
		yLine = pstBuff->vcOpeSqn.at( dNow ).rdYline;

		if( 0 == dPreGroup )
		{
			dPreGroup = dGrp;
			yPreLine = yLine;
		}
		else
		{

			if( dPreGroup != dGrp ){	break;	}

			if( yPreLine != yLine && 0 == dCrLf ){	dCrLf = 1;	}
		}

		TRACE( TEXT("UNDO CMD:%u GRP:%u  D:%d, L:%d"), dCmd, dGrp, xDot, yLine );

		ptStr = pstBuff->vcOpeSqn.at( dNow ).ptText;

		switch( dCmd )
		{
			case  DO_INSERT:
				StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );
				iRslt = DocStringErase( xDot, yLine, NULL, cchSize );
				break;

			case  DO_DELETE:
				iRslt = DocStringAdd( &xDot, &yLine, pstBuff->vcOpeSqn.at( dNow ).ptText, pstBuff->vcOpeSqn.at( dNow ).cchSize );
				break;

			default:	TRACE( TEXT("Undo 오류! [%u]알 수 없는 코드"), dCmd );	return dCrLf;
		}

		*pxDot  = xDot;
		*pyLine = yLine;

		if( dCrLf < iRslt ){	dCrLf = iRslt;	}

		pstBuff->dNowSqn -= 1;

		if( !(gbGroupUndo) ){	break;	}

	}while( pstBuff->dNowSqn );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("기타 오류"), 0 );	}
#endif

	return dCrLf;
}

INT SqnRedoExec( LPUNDOBUFF pstBuff, PINT pxDot, PINT pyLine )
{
	OPSQ_ITR	itSqn;
	INT		xDot, yLine, iRslt = 0, dCrLf = 0, yPreLine = 0;
	UINT	dCmd, dGrp, dNow, cchSize;
	UINT	dPreGroup = 0;
	LPTSTR	ptStr;

#ifdef DO_TRY_CATCH
	try{
#endif

	do
	{
		dNow  = pstBuff->dNowSqn;
		if( dNow == pstBuff->vcOpeSqn.size( ) ){	return 0;	}

		TRACE( TEXT("REDO SQNUM:%u"), dNow );

		dCmd  = pstBuff->vcOpeSqn.at( dNow ).dCommando;
		dGrp  = pstBuff->vcOpeSqn.at( dNow ).ixGroup;
		xDot  = pstBuff->vcOpeSqn.at( dNow ).rdXdot;
		yLine = pstBuff->vcOpeSqn.at( dNow ).rdYline;

		if( 0 == dPreGroup )
		{
			dPreGroup = dGrp;
			yPreLine = yLine;
		}
		else
		{

			if( dPreGroup != dGrp ){	break;	}

			if( yPreLine != yLine && 0 == dCrLf ){	dCrLf = 1;	}
		}

		TRACE( TEXT("REDO CMD:%u GRP:%u  D:%d, L:%d"), dCmd, dGrp, xDot, yLine );

		ptStr = pstBuff->vcOpeSqn.at( dNow ).ptText;

		switch( dCmd )
		{
			case  DO_INSERT:
				iRslt = DocStringAdd( &xDot, &yLine, pstBuff->vcOpeSqn.at( dNow ).ptText, pstBuff->vcOpeSqn.at( dNow ).cchSize );
				break;

			case  DO_DELETE:
				StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );
				iRslt = DocStringErase( xDot, yLine, NULL, cchSize );
				break;

			default:	TRACE( TEXT("Redo 오류! [%u]알 수 없는 코드"), dCmd );	return dCrLf;
		}

		*pxDot  = xDot;
		*pyLine = yLine;

		if( dCrLf < iRslt ){	dCrLf = iRslt;	}

		pstBuff->dNowSqn += 1;

		if( !(gbGroupUndo) ){	break;	}

	}while( pstBuff->dNowSqn != pstBuff->vcOpeSqn.size( ) );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("기타 오류"), 0 );	}
#endif

	return dCrLf;
}
