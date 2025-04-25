#include "stdafx.h"
#include "OrinrinEditor.h"

extern  HWND	ghPrntWnd;
extern  HWND	ghViewWnd;

extern INT		gdDocXdot;
extern INT		gdDocLine;

extern INT		gdHideXdot;
extern INT		gdViewTopLine;
extern SIZE		gstViewArea;
extern INT		gdDispingLine;

extern BOOLEAN	gbShiftOn;
extern BOOLEAN	gbCtrlOn;
extern BOOLEAN	gbAltOn;

extern BOOLEAN	gbExtract;

extern POINT	gstCursor;

static POINT	gstSqSelBegin;
static POINT	gstSqSelEnd;

static POINT	gstSelBgnOrig;
static POINT	gstSelEndOrig;

static POINT	gstPrePos;

static BOOLEAN	gbSelecting;
EXTERNED UINT	gbSqSelect;

HRESULT	ViewSelStateChange( UINT );
HRESULT	ViewSqSelAdjust( INT );

BOOLEAN IsSelecting( PUINT pSqSel )
{
	if( pSqSel )	*pSqSel = gbSqSelect;

	return gbSelecting;
}

HRESULT ViewSelPositionSet( LPVOID pVoid )
{
	INT	iBgn, iEnd, iDmy = 0;

	if( (gstPrePos.x != gdDocXdot) )
	{
		iBgn =  gstPrePos.x;	if( 0 > iBgn )	iBgn = 0;
		iEnd =  gstPrePos.x + 1;
		ViewPositionTransform( &iBgn, &iDmy, 1 );
		ViewPositionTransform( &iEnd, &iDmy, 1 );
		ViewRulerRedraw( iBgn, iEnd );

		iBgn =  gdDocXdot;	if( 0 > iBgn )	iBgn = 0;
		iEnd =  gdDocXdot + 1;
		ViewPositionTransform( &iBgn, &iDmy, 1 );
		ViewPositionTransform( &iEnd, &iDmy, 1 );
		ViewRulerRedraw( iBgn, iEnd );
	}

	gstPrePos.x = gdDocXdot;
	gstPrePos.y = gdDocLine;

	return S_OK;
}

UINT ViewSqSelModeToggle( UINT bMode, LPVOID pVoid )
{
	POINT	point;

	TRACE( TEXT("사각형 선택 ON/OFF") );

	if( gbSelecting )	return gbSqSelect;

	if( bMode )
	{
		gbSqSelect ^= D_SQUARE;
	}
	else
	{

		if( gbAltOn ){	gbSqSelect |=  D_SQUARE;	}

	}

	gstSqSelBegin.x = -1;
	gstSqSelBegin.y = -1;
	gstSqSelEnd.x   = -1;
	gstSqSelEnd.y   = -1;

	MenuItemCheckOnOff( IDM_SQSELECT , gbSqSelect );

	OperationOnStatusBar(  );

	if( D_SQUARE & gbSqSelect )
	{
		SetClassLongPtr( ghViewWnd, GCLP_HCURSOR, (LONG_PTR)(LoadCursor( NULL, IDC_CROSS ) ) );
	}
	else
	{
		SetClassLongPtr( ghViewWnd, GCLP_HCURSOR, (LONG_PTR)(LoadCursor( NULL, IDC_IBEAM ) ) );
	}
	GetCursorPos( &point );
	SetCursorPos( point.x, point.y );

	return gbSqSelect;
}

UINT ViewSelRangeCheck( UINT dMode )
{

	if( gstSelBgnOrig.x == gstSelEndOrig.x && gstSelBgnOrig.y == gstSelEndOrig.y )
	{
		TRACE( TEXT("범위 소멸로 인한 선택 해제") );
		if( IsSelecting( NULL ) )
		{
			TRACE( TEXT("선택 중이었다면 범위 해제") );
			ViewSelPageAll( -1 );
		}

		gstSqSelBegin.x = -1;
		gstSqSelBegin.y = -1;
		gstSqSelEnd.x   = -1;
		gstSqSelEnd.y   = -1;

		gstSelBgnOrig.x = -1;
		gstSelBgnOrig.y = -1;
		gstSelEndOrig.x = -1;
		gstSelEndOrig.y = -1;

		return 0;
	}

	return 1;
}

HRESULT ViewSelMoveCheck( UINT dMode )
{

	if( gbExtract && dMode )
	{
		ViewSelStateChange( FALSE );

		return S_OK;
	}

	if( gbSelecting )
	{
		if( gbShiftOn || dMode )
		{
			ViewSelStateChange( FALSE );

			ViewSelRangeCheck( dMode );
		}
		else
		{
			TRACE( TEXT("다른 작업으로 인한 선택 해제") );
			ViewSelPageAll( -1 );

			gstSqSelBegin.x = -1;
			gstSqSelBegin.y = -1;
			gstSqSelEnd.x   = -1;
			gstSqSelEnd.y   = -1;

			gstSelBgnOrig.x = -1;
			gstSelBgnOrig.y = -1;
			gstSelEndOrig.x = -1;
			gstSelEndOrig.y = -1;
		}
	}
	else
	{

		if( gbShiftOn || dMode )
		{

				ViewSqSelModeToggle( 0, NULL );

			TRACE( TEXT("STATE[%d %d %d]"), gbShiftOn, gbAltOn, dMode );

			gstSqSelBegin = gstPrePos;
			gstSqSelEnd.x = gdDocXdot;
			gstSqSelEnd.y = gdDocLine;

			gstSelBgnOrig = gstSqSelBegin;
			gstSelEndOrig = gstSqSelEnd;

			gbSelecting   = TRUE;
			TRACE( TEXT("선택 처리 시작[%d:%d]"), gstSqSelBegin.x, gstSqSelBegin.y );

			ViewSelStateChange( TRUE );
		}

	}

	return S_OK;
}

INT ViewSelPageAll( INT dForce )
{
	TRACE( TEXT("전체 선택[%d]"), dForce );

	if( 0 < dForce )		gbSelecting =  TRUE;
	else if( 0 > dForce )	gbSelecting = FALSE;
	else					return 0;

	return DocPageSelStateToggle( dForce );
}

HRESULT ViewSelStateChange( UINT dFirst )
{

	INT		dBeginDot, dEndDot, dStep = 0;
	INT		dBaseLine, dJpLn;

	if( gstPrePos.y != gdDocLine )
	{
		dStep = gdDocLine - gstPrePos.y;

		TRACE( TEXT("선택으로 인해 줄 넘김 발생：D[%d] L[%d] St[%d]"), gdDocXdot, gdDocLine, dStep );

		if( 0 <  dStep )
		{
			dBeginDot =  gstPrePos.x;
			dEndDot   =  -1;
		}
		else
		{
			dBeginDot = 0;
			dEndDot   = gstPrePos.x;
		}

		gstSqSelEnd.x =  gdDocXdot;

		dBaseLine = gstPrePos.y;
	}
	else
	{
		if( gstPrePos.x < gdDocXdot )
		{
			dBeginDot = gstPrePos.x;
			dEndDot   = gdDocXdot;
		}
		else
		{
			dBeginDot = gdDocXdot;
			dEndDot   = gstPrePos.x;
		}

		dBaseLine = gdDocLine;
	}

	gstSelEndOrig.x = gdDocXdot;
	gstSelEndOrig.y = gdDocLine;

	if( gstSelBgnOrig.y >  gstSelEndOrig.y )
	{
		gstSqSelBegin.y = gdDocLine;
		gstSqSelEnd.y   = gstSelBgnOrig.y;
	}
	else
	{
		gstSqSelBegin.y = gstSelBgnOrig.y;
		gstSqSelEnd.y   = gdDocLine;
	}

	if( gstSelBgnOrig.x >  gstSelEndOrig.x )
	{
		gstSqSelBegin.x = gdDocXdot;
		gstSqSelEnd.x   = gstSelBgnOrig.x;
	}
	else
	{
		gstSqSelBegin.x = gstSelBgnOrig.x;
		gstSqSelEnd.x   = gdDocXdot;
	}

	TRACE( TEXT("[%d:%d][%d:%d]"), gstSqSelBegin.x, gstSqSelBegin.y, gstSqSelEnd.x, gstSqSelEnd.y );

	DocSelRangeSet( gstSqSelBegin.y, gstSqSelEnd.y );

	if( gbSqSelect )
	{
		ViewSqSelAdjust( dBaseLine );
	}
	else
	{

		DocRangeSelStateToggle( dBeginDot, dEndDot, dBaseLine, 0 );

		if( 1 <= dStep )
		{
			DocReturnSelStateToggle( dBaseLine, 0 );

			for( dJpLn = (dBaseLine + 1); gdDocLine > dJpLn; dJpLn++ )
			{
				DocReturnSelStateToggle( dJpLn, 0 );
				DocRangeSelStateToggle( 0, -1, dJpLn, 0 );
			}

			DocRangeSelStateToggle( 0, gdDocXdot, gdDocLine, 0 );
		}

		if( -1 >= dStep )
		{

			for( dJpLn = (gdDocLine + 1); dBaseLine > dJpLn; dJpLn++ )
			{
				DocReturnSelStateToggle( dJpLn, 0 );
				DocRangeSelStateToggle( 0, -1, dJpLn, 0 );
			}

			DocReturnSelStateToggle( gdDocLine, 0 );
			DocRangeSelStateToggle( gdDocXdot, -1, gdDocLine, 0 );
		}
	}

	return S_OK;
}

UINT ViewSelBackCheck( INT line )
{

	TRACE( TEXT("LINE[%d] ST[%d]"), line, gstSelBgnOrig.y );

	if( gstSelBgnOrig.y >= line ){	return TRUE;	}

	return FALSE;
}

HRESULT ViewSqSelAdjust( INT dBaseLine )
{
	INT	i, xDotBegin, xDotEnd, xDotLast;

#pragma message ("여기서, 선택 범위 전체의 처리가 여러 번 수행되어 무겁다")

	if( dBaseLine < gstSqSelBegin.y )
	{
		for( i = dBaseLine; gstSqSelBegin.y > i; i++ )
		{
			DocRangeSelStateToggle( 0, -1, i, -1 );
		}
	}

	if( gstSqSelEnd.y < dBaseLine )
	{
		for( i = gstSqSelEnd.y + 1; dBaseLine >= i; i++ )
		{
			DocRangeSelStateToggle( 0, -1, i, -1 );
		}
	}

	for( i = gstSqSelBegin.y; gstSqSelEnd.y >= i; i++ )
	{
		xDotBegin = gstSqSelBegin.x;
		DocLetterPosGetAdjust( &xDotBegin, i, 0 );

		xDotEnd = (gstSqSelEnd.x < gstCursor.x) ? gstCursor.x : gstSqSelEnd.x;
		DocLetterPosGetAdjust( &xDotEnd, i, 0 );

		xDotLast = DocLineParamGet( i, NULL, NULL );

		if( 0 < xDotBegin )
		{
			DocRangeSelStateToggle( 0, xDotBegin, i, -1 );
		}

		DocRangeSelStateToggle( xDotBegin, xDotEnd, i, 1 );

		if( xDotEnd < xDotLast )
		{
			DocRangeSelStateToggle( xDotEnd, -1, i, -1 );
		}
	}

	return S_OK;
}

HRESULT ViewSelAreaSelect( LPVOID pVoid )
{
	INT		iBeginDot, iEndDot, iStCnt, iCount;
	INT		iRangeDot;
	BOOLEAN	bIsSpase;

	DocPageSelStateToggle(  FALSE );

	iRangeDot = DocLineStateCheckWithDot( gdDocXdot, gdDocLine, &iBeginDot, &iEndDot, &iStCnt, &iCount, &bIsSpase );

	gdDocXdot = iBeginDot;
	ViewSelMoveCheck( FALSE );
	ViewSelPositionSet( NULL );

	gdDocXdot = iEndDot;

	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );

	ViewSelMoveCheck( TRUE );
	ViewSelPositionSet( NULL );

	return S_OK;
}
