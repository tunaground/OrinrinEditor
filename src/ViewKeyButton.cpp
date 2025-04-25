#include "stdafx.h"
#include "OrinrinEditor.h"

extern HWND		ghPrntWnd;
extern HWND		ghViewWnd;

extern INT		gdXmemory;

extern INT		gdDocXdot;
extern INT		gdDocLine;
extern INT		gdDocMozi;

extern INT		gdHideXdot;
extern INT		gdViewTopLine;
extern SIZE		gstViewArea;
extern INT		gdDispingLine;

extern BOOLEAN	gbExtract;

extern  UINT	gbUniPad;

EXTERNED BOOLEAN	gbShiftOn;
EXTERNED BOOLEAN	gbCtrlOn;
EXTERNED BOOLEAN	gbAltOn;

EXTERNED POINT	gstCursor;

EXTERNED UINT	gbBrushMode;
static TCHAR	gatBrushPtn[SUB_STRING];

static  UINT	gdSqFillCnt;

static  UINT	gbLDoubleClick;

static POINT	gstLClicken;
static  UINT	gbDragMoved;

HRESULT	ViewBrushFilling( VOID );

HRESULT	ViewScriptedLineFeed( VOID );

VOID ViewCombiKeyCheck( VOID )
{

	gbShiftOn = (0x8000 & GetKeyState(VK_SHIFT)) ? TRUE : FALSE;
	gbCtrlOn  = (0x8000 & GetKeyState(VK_CONTROL)) ? TRUE : FALSE;
	gbAltOn   = (0x8000 & GetKeyState(VK_MENU)) ? TRUE : FALSE;

	return;
}

VOID Evw_OnKey( HWND hWnd, UINT vk, BOOL fDown, INT cRepeat, UINT flags )
{
	INT		bXdirect = 0;
	UINT	dXwidth;
	INT		dDot, bCrLf, iLines, i;
	BOOLEAN	bJump = FALSE, bMemoryX = FALSE;
	BOOLEAN	bSelect = FALSE;
	UINT	bSqSel  = 0;

	ViewCombiKeyCheck(  );

	ViewSelPositionSet( NULL );

#ifdef DO_TRY_CATCH
	try{
#endif

	if( fDown )
	{
		switch( vk )
		{
			case VK_RIGHT:	bXdirect =  1;	bMemoryX = TRUE;	break;

			case VK_LEFT:	bXdirect = -1;	bMemoryX = TRUE;	break;

			case VK_DOWN:
				gdDocLine++;
				dDot = DocLineParamGet( gdDocLine, NULL, NULL );
				if( -1 == dDot ){	gdDocLine--;	}
				break;

			case VK_UP:
				if( 0 < gdDocLine ){	gdDocLine--;	}
				break;

			case VK_PRIOR:
				gdDocLine -= 10;
				if( 0 > gdDocLine ){	gdDocLine = 0;	}
				break;

			case VK_NEXT:
				gdDocLine += 10;
				iLines = DocNowFilePageLineCount(  );
				if( iLines <= gdDocLine ){	gdDocLine =  iLines - 1;	}
				break;

			case VK_END:
				if( gbCtrlOn ){	gdDocLine = DocNowFilePageLineCount(  ) - 1;	}
				gdDocXdot = DocLineParamGet( gdDocLine, &gdDocMozi, NULL );
				bMemoryX = TRUE;
				break;

			case VK_HOME:
				gdDocXdot = 0;	gdDocMozi = 0;	bMemoryX = TRUE;
				if( gbCtrlOn  ){	gdDocLine = 0;	}
				break;

			case VK_DELETE:
				bSelect = IsSelecting( &bSqSel );
				iLines = DocNowFilePageLineCount(  );
				if( bSelect )
				{
					bCrLf = DocSelectedDelete( &gdDocXdot , &gdDocLine, bSqSel, TRUE );
				}
				else
				{
					bCrLf = DocInputDelete( gdDocXdot , gdDocLine );
				}

				if( 0 <  bCrLf )
				{
					for( i = gdDocLine; iLines >= i; i++  ){	ViewRedrawSetLine(  i );	}
				}
				else{	ViewRedrawSetLine( gdDocLine  );	}
				ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
				DocPageInfoRenew( -1, 1 );
				bMemoryX = TRUE;
				return;

			case VK_PROCESSKEY:

				return;

			default:

				return;
		}

		if( bMemoryX )	gdXmemory = gdDocXdot;
		else			gdDocXdot = gdXmemory;

		DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );

		dXwidth = DocLetterShiftPos( gdDocXdot, gdDocLine, bXdirect, NULL, &bJump );

		if( 0 > bXdirect )
		{
			gdDocXdot -= dXwidth;
			if( 0 >  gdDocXdot )	gdDocXdot = 0;

			if( bJump )
			{
				if( 0 < gdDocLine ){	gdDocLine--;	}

				dDot = DocLineParamGet( gdDocLine, NULL, NULL );
				gdDocXdot = dDot;
			}
		}

		if( 0 < bXdirect )
		{
			gdDocXdot += dXwidth;
			if( bJump )
			{
				gdDocLine++;

				dDot = DocLineParamGet( gdDocLine, NULL, NULL );
				if( 0 >  dDot ){	gdDocLine--;	}
				else{	gdDocXdot = 0;	}
			}
		}

		gdDocMozi = DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );

		ViewSelMoveCheck( FALSE );

		if( bMemoryX )	gdXmemory = gdDocXdot;

		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
	}
	else
	{

	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	ETC_MSG( err.what(), 0 );	 return;	}
	catch( ... ){	ETC_MSG( ("etc error"), 0 );	 return;	}
#endif

	return;
}

VOID Evw_OnChar( HWND hWnd, TCHAR ch, INT cRepeat )
{
	BOOLEAN	bSelect, bFirst;
	UINT	bSqSel = 0;
	INT		isctrl, bCrLf, iLines, i;

	TCHAR	atCh[2];

	ViewCombiKeyCheck(  );

	bSelect = IsSelecting( &bSqSel );

	isctrl = iswcntrl( ch );

	if( isctrl )
	{
		TRACE( TEXT("制御文字[%04X]"), ch );

		if( VK_RETURN == ch )
		{
			TRACE( TEXT("Enter Shift[%d]"), gbShiftOn );
			if( gbShiftOn )
			{
				ViewScriptedLineFeed(  );
			}
			else
			{
				bFirst = TRUE;
				if( bSelect )
				{
					bCrLf = DocSelectedDelete( &gdDocXdot , &gdDocLine, bSqSel, bFirst );	bFirst = FALSE;
				}

				DocCrLfAdd( gdDocXdot , gdDocLine, bFirst );
				ViewRedrawSetLine( gdDocLine );

				gdDocXdot = 0;	gdDocMozi = 0;	gdDocLine++;
				ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
				gdXmemory = gdDocXdot;

				iLines = DocPageParamGet( NULL, NULL );
				for( i = gdDocLine; iLines >= i; i++ ){	ViewRedrawSetLine(  i );	}
			}
		}

		if( VK_BACK == ch )
		{
			TRACE( TEXT("BACKSP [%d][%d:%d]"), bSelect, gdDocXdot, gdDocLine );
			iLines = DocNowFilePageLineCount(  );
			if( bSelect )
			{
				bCrLf = DocSelectedDelete( &gdDocXdot , &gdDocLine, bSqSel, TRUE );
			}
			else
			{
				bCrLf = DocInputBkSpace( &gdDocXdot , &gdDocLine );
			}
			if( bCrLf  )
			{
				for( i = gdDocLine; iLines >= i; i++  ){	ViewRedrawSetLine(  i );	}
			}
			else{	ViewRedrawSetLine( gdDocLine  );	}

			ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
			gdXmemory = gdDocXdot;
		}

		return;
	}

	TRACE( TEXT("入力文字[%c]"), ch );

	if( 0 < gdSqFillCnt )
	{
		gdSqFillCnt--;
		TRACE( TEXT("キャンセル[%u]"), gdSqFillCnt );
		return;
	}

	iLines = DocPageParamGet( NULL, NULL );
	bCrLf = 0;
	if( bSelect )
	{
		if( bSqSel )
		{
			atCh[0] = ch;	atCh[1] = NULL;
			DocSelectedBrushFilling( atCh, &gdDocXdot ,&gdDocLine );
			ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
			DocPageInfoRenew( -1, 1 );
			return;
		}
		else{	bCrLf = DocSelectedDelete( &gdDocXdot , &gdDocLine, 0, TRUE );	}

	}

	DocInsertLetter( &gdDocXdot, gdDocLine, ch );

	gdDocMozi = DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );
	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );

	gdXmemory = gdDocXdot;

	if( bCrLf  )
	{
		for( i = gdDocLine; iLines > i; i++ ){	ViewRedrawSetLine(  i );	}
	}
	else{	ViewRedrawSetLine( gdDocLine  );	}

	DocPageInfoRenew( -1, 1 );

	return;
}

VOID Evw_OnLButtonDown( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	INT		dX, dY;
	INT		dDot, dMaxDot, dLine, iMaxLine;
	UINT	dRslt;

	SetFocus( hWnd );

	dX = x;
	dY = y;
	ViewPositionTransform( &dX, &dY, 0 );

	ViewCombiKeyCheck(  );

	if( 0 > dX )	dX = 0;
	if( 0 > dY )	dY = 0;

	dDot  = dX;
	dLine = dY / LINE_HEIGHT;

	if( fDoubleClick )
	{
		TRACE( TEXT("マウス左ダブルクルック[%d / %d]%d:%d:%d"), dDot, dLine, gbShiftOn, gbCtrlOn, gbAltOn );

		ViewSelAreaSelect( NULL );

		gbLDoubleClick = TRUE;
		return;
	}

	TRACE( TEXT("マウス左ダウン[%d / %d]%d:%d:%d"), dDot, dLine, gbShiftOn, gbCtrlOn, gbAltOn );

	SetCapture( hWnd  );

	iMaxLine = DocNowFilePageLineCount(  );
	if( iMaxLine <=dLine )	dLine = iMaxLine - 1;

	dMaxDot = DocLineParamGet( dLine, NULL, NULL );
	if( dMaxDot <=dDot )	dDot = dMaxDot;

	gdDocMozi = DocLetterPosGetAdjust( &dDot, dLine, 0 );
	gdDocXdot = dDot;
	gdDocLine = dLine;

	gstLClicken.x = gdDocXdot;
	gstLClicken.y = gdDocLine;

	gdXmemory = gdDocXdot;
	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );

	dRslt = DocLetterSelStateGet( gdDocXdot, gdDocLine );

	if( dRslt ){	gbDragMoved = TRUE;	}
	else{	ViewSelMoveCheck( FALSE );	}

	ViewSelPositionSet( NULL );

	return;
}

VOID Evw_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TCHAR	atString[SUB_STRING];
	INT		dX, dY;
	INT		dDot, dMaxDot, dLine, iMaxLine;

	if( gbLDoubleClick ){	 return;	}

	dX = x;
	dY = y;
	ViewPositionTransform( &dX, &dY, 0 );

	ViewCombiKeyCheck(  );

	if( 0 > dY )	dY = 0;

	dLine = dY / LINE_HEIGHT;

	if( 0 > dX )
	{
		dX = 0;
		if( (keyFlags & MK_LBUTTON) )
		{

			dX = DocLineParamGet( dLine, NULL, NULL );

			if( ViewSelBackCheck( dLine ) ){	dX =  0;	}
		}
	}

	dDot  = dX;

	gstCursor.x = dDot;
	gstCursor.y = dLine;

	if( (keyFlags & MK_LBUTTON) )
	{

		iMaxLine = DocNowFilePageLineCount(  );
		if( iMaxLine <= dLine ){	dLine = iMaxLine - 1;	}

		dMaxDot = DocLineParamGet( dLine, NULL, NULL );
		if( dMaxDot <=dDot )	dDot = dMaxDot;

		gdDocMozi = DocLetterPosGetAdjust( &dDot, dLine, 0 );
		gdDocXdot = dDot;
		gdDocLine = dLine;

		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );

		if( !(gbDragMoved) ){	ViewSelMoveCheck( TRUE );	}

		ViewSelPositionSet( NULL );
	}

	StringCchPrintf( atString, SUB_STRING, TEXT("MOUSE %d[dot] %d[line]"), gstCursor.x, gstCursor.y );
	MainStatusBarSetText( SB_MOUSEPOS, atString );

	return;
}

VOID Evw_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	UINT	dRslt, bSqSel;

	INT		iCrLf;
	LPTSTR	ptString = NULL;
	UINT	cbSize;

	INT		xPos, yPos;

	TRACE( TEXT("マウス左アップ[%d / %d]"), x, y );

	if( gbLDoubleClick ){	gbLDoubleClick =  FALSE;	 return;	}

	ViewSelRangeCheck( FALSE  );

	ReleaseCapture(   );

	if( !(gbExtract) )
	{
		ViewBrushFilling(  );

		if( (gstLClicken.x == gdDocXdot) && (gstLClicken.y == gdDocLine) )
		{
			ViewSelMoveCheck( FALSE );
			gbDragMoved = FALSE;
		}

		if( gbDragMoved )
		{

			dRslt = DocLetterSelStateGet( gdDocXdot, gdDocLine );
			if( !(dRslt)  )
			{
				IsSelecting( &bSqSel );

				cbSize = DocSelectTextGetAlloc( D_UNI | bSqSel, (LPVOID *)(&ptString), NULL );
				iCrLf = DocInsertString( &gdDocXdot, &gdDocLine, NULL, ptString, bSqSel, TRUE );
				xPos = gdDocXdot;	yPos = gdDocLine;
				FREE( ptString );

				DocSelRangeReset( NULL , NULL );
				iCrLf = DocSelectedDelete( &gdDocXdot, &gdDocLine, bSqSel, FALSE );

				ViewRedrawSetLine( -1 );

				ViewPosResetCaret( xPos, yPos );
			}
			gbDragMoved = FALSE;
		}
	}

	return;
}

VOID Evw_OnRButtonDown( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{

	INT		dX, dY;
	INT		dDot, dMaxDot, dLine, iMaxLine;

	SetFocus( hWnd );

	if( IsSelecting( NULL ) )
	{
		TRACE( TEXT("[%X]マウス右ダウン　%d:%d　選択中"), hWnd, x, y );
		return;
	}

	dX = x;
	dY = y;
	ViewPositionTransform( &dX, &dY, 0 );

	dDot  = dX;
	dLine = dY / LINE_HEIGHT;

	TRACE( TEXT("[%X]マウス右ダウン[%d:%d[%d] / %d:%d:%d]"), hWnd, dX, dY, dLine, gbShiftOn, gbCtrlOn, gbAltOn );

	if( 0 <= dX || 0 <= dY )
	{

		iMaxLine = DocNowFilePageLineCount(  );
		if( iMaxLine <=dLine )	dLine = iMaxLine - 1;

		dMaxDot = DocLineParamGet( dLine, NULL, NULL );
		if( dMaxDot <=dDot )	dDot = dMaxDot;

		gdDocMozi = DocLetterPosGetAdjust( &dDot, dLine, 0 );
		gdDocXdot = dDot;
		gdDocLine = dLine;

		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );

		ViewSelMoveCheck( FALSE );

		ViewSelPositionSet( NULL );

	}

	return;
}

VOID Evw_OnMouseWheel( HWND hWnd, INT xPos, INT yPos, INT zDelta, UINT fwKeys )
{
	UINT	dCode;

	HWND	hChdWnd;
	POINT	stPoint;

	TRACE( TEXT("POS[%d x %d] DELTA[%d] K[%X]"), xPos, yPos, zDelta, fwKeys );

	stPoint.x = xPos;
	stPoint.y = yPos;
	ScreenToClient( ghPrntWnd, &stPoint );
	hChdWnd = ChildWindowFromPointEx( ghPrntWnd, stPoint, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT );

	if( ghViewWnd != hChdWnd )	return;

	if( 0 < zDelta )		dCode = SB_LINEUP;
	else if( 0 > zDelta )	dCode = SB_LINEDOWN;
	else					dCode = SB_ENDSCROLL;

	FORWARD_WM_VSCROLL( ghViewWnd, NULL, dCode, 1, PostMessage );

	return;
}

INT ViewInsertUniSpace( UINT dCommando )
{
	INT	width;
	TCHAR	ch;

	TRACE( TEXT("挿入：ユニコード空白") );

	switch( dCommando )
	{
		case IDM_IN_01SPACE:	ch  = (TCHAR)0x200A;	break;
		case IDM_IN_02SPACE:	ch  = (TCHAR)0x2009;	break;
		case IDM_IN_03SPACE:	ch  = (TCHAR)0x2006;	break;
		case IDM_IN_04SPACE:	ch  = (TCHAR)0x2005;	break;
		case IDM_IN_05SPACE:	ch  = (TCHAR)0x2004;	break;
		case IDM_IN_08SPACE:	ch  = (TCHAR)0x2002;	break;
		case IDM_IN_10SPACE:	ch  = (TCHAR)0x2007;	break;
		case IDM_IN_16SPACE:	ch  = (TCHAR)0x2003;	break;
		default:	return 0;
	}

	width = DocInsertLetter( &gdDocXdot, gdDocLine, ch );

	gdDocMozi = DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );
	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
	ViewRedrawSetLine( gdDocLine  );

	DocPageInfoRenew( -1, 1 );

	return width;
}

HRESULT ViewScriptedLineFeed( VOID )
{

	INT		dLines, iTgtDot, iLastDot, iLineDot, iPadDot;
	INT		iPrvDot, iChkDot;
	BOOLEAN	bIsSp, bFirst = TRUE, bJump;
	UINT	dStyle = 0;
	LPTSTR	ptSpace;

	iChkDot = gdDocXdot;
	iTgtDot = 0;

	while( iChkDot )
	{

		DocLineStateCheckWithDot( iChkDot, gdDocLine, &iTgtDot, &iLastDot, NULL, NULL, &bIsSp );

		if(  0 == iTgtDot && !(bIsSp) ){	break;	}

		if( bIsSp ){	iTgtDot = iChkDot;	break;	}

		DocLetterShiftPos( iTgtDot, gdDocLine, -1, &iPrvDot, &bJump );

		DocLineStateCheckWithDot( iPrvDot, gdDocLine, &iChkDot, &iLastDot, NULL, NULL, &bIsSp );
		if( bIsSp ){	break;	}

		iChkDot = iPrvDot;
	}

	TRACE( TEXT("TEXT START D[%d] L[%d]"), iTgtDot, gdDocLine );

	dLines = DocNowFilePageLineCount(  );
	if( (dLines - 1) <= gdDocLine )
	{
		DocAdditionalLine( 1, &bFirst );
		ViewRedrawSetLine( gdDocLine );
	}

	gdDocLine++;

	iLineDot = DocLineParamGet( gdDocLine, NULL, NULL );
	if( iTgtDot <= iLineDot )
	{
		gdDocXdot = iTgtDot;
		DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );
		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
	}
	else
	{
		iPadDot = iTgtDot - iLineDot;
		ptSpace = DocPaddingSpaceMake( iPadDot );
		gdDocXdot = iLineDot;

		DocInsertString( &gdDocXdot, &gdDocLine, NULL, ptSpace, dStyle, bFirst );	bFirst = FALSE;

		FREE(ptSpace);
	}

	ViewRedrawSetLine( gdDocLine  );

	return S_OK;
}

INT ViewInsertColourTag( UINT dCommando )
{
	UINT	dCrLf;
	INT		dDot;
	TCHAR	atString[MAX_STRING];

	switch( dCommando )
	{
		case IDM_INSTAG_WHITE:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_WHITE );	break;
		case IDM_INSTAG_BLUE:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_BLUE  );	break;
		case IDM_INSTAG_BLACK:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_BLACK );	break;
		case IDM_INSTAG_RED:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_RED   );	break;
		case IDM_INSTAG_GREEN:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_GREEN );	break;
		default:	return 0;
	}

	dDot = gdDocXdot;

	dCrLf = DocInsertString( &gdDocXdot, &gdDocLine, &gdDocMozi, atString, 0, TRUE );

	dDot = gdDocXdot- dDot;

	DocPageInfoRenew( -1, 1 );

	return dDot;
}

INT ViewInsertTmpleString( LPCTSTR ptText )
{
	UINT	dCrLf;
	INT		dDot;

	dDot = gdDocXdot;

	dCrLf = DocInsertString( &gdDocXdot, &gdDocLine, &gdDocMozi, ptText, 0, TRUE );

	dDot = gdDocXdot- dDot;

	DocPageInfoRenew( -1, 1 );

	return dDot;
}

HRESULT ViewBrushStyleSetting( UINT bBrushOn, LPTSTR ptPattern )
{
	gbBrushMode = bBrushOn;

	SendMessage( ghPrntWnd, WMP_BRUSH_TOGGLE, (WPARAM)bBrushOn, (LPARAM)IDM_BRUSH_STYLE );

	if( ptPattern ){	StringCchCopy( gatBrushPtn, SUB_STRING, ptPattern );	}

	OperationOnStatusBar(  );

	return S_OK;
}

HRESULT ViewBrushFilling( VOID )
{
	INT			dTgDot;
	INT			dLeft, dRight, iBgnMozi, iCntMozi, rslt;
	BOOLEAN		bSpace, bFirst = TRUE;
	LPTSTR		ptBuff;
	wstring		wsBuff;

	if( !(gbBrushMode) )	return S_FALSE;

	rslt = DocSelectedBrushFilling( gatBrushPtn, &gdDocXdot, &gdDocLine );
	if( rslt )	return S_OK;

	dTgDot = DocLineStateCheckWithDot( gdDocXdot, gdDocLine, &dLeft, &dRight, &iBgnMozi, &iCntMozi, &bSpace );
	if( !(bSpace)  )	return S_FALSE;

	ptBuff = BrushStringMake( dTgDot, gatBrushPtn );
	if( !(ptBuff) )
	{
		NotifyBalloonExist( TEXT("ブラシを選んでおいてね"), TEXT("操作ミス"), NIIF_INFO );
		return E_OUTOFMEMORY;
	}

	DocRangeDeleteByMozi( dLeft, gdDocLine, iBgnMozi, (iBgnMozi + iCntMozi), &bFirst );

	DocInsertString( &dLeft, &gdDocLine, NULL, ptBuff, 0, bFirst );	bFirst = FALSE;

	gdDocMozi = DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );

	FREE(ptBuff);

	return S_OK;
}

LPTSTR BrushStringMake( INT dDotLen, LPTSTR ptPattern )
{
	INT			dPtnDot, dCnt, dAmr, i, wid;
#if 0
	INT			chk;
#endif
	UINT_PTR	cchSize;
	LPTSTR		ptBuff;
	wstring		wsBuff;

	wsBuff.clear( );

	dPtnDot = ViewStringWidthGet( ptPattern );
	if( 0 >= dPtnDot ||  0 >= dDotLen ){	return NULL;	}

	dCnt = dDotLen / dPtnDot;
	dAmr = dDotLen - (dCnt * dPtnDot);

	for( i = 0; dCnt > i; i++ ){	wsBuff += ptPattern;	}

	i = 0;
	while( 0 < dAmr )
	{
		if(  0 == ptPattern[i] )	break;

		wid = ViewLetterWidthGet( ptPattern[i] );
#if 0
		if( gbUniPad )
		{
			chk = dAmr - wid;
			if( 0 > chk )	break;
		}
#endif
		wsBuff += ptPattern[i];
		dAmr -= wid;
		i++;
	}

	cchSize = wsBuff.size( ) + 8;
	ptBuff = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	ZeroMemory( ptBuff, cchSize * sizeof(TCHAR) );
	StringCchCopy( ptBuff, cchSize, wsBuff.c_str( ) );
#if 0
	if( gbUniPad )
	{
		ptPadd = DocPaddingSpaceUni( dAmr, NULL, NULL, NULL );
		if( ptPadd )
		{
			StringCchCat( ptBuff, cchSize, ptPadd );
			FREE(ptPadd);
		}
	}
#endif

	return ptBuff;
}

VOID Evw_OnImeComposition( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	HIMC	hImc;
	LONG	cbSize;
	LPTSTR	ptBuffer;
	UINT_PTR	cchSize;

	BOOLEAN	bSelect = FALSE;
	UINT	bSqSel  = 0;

	TRACE( TEXT("WM_IME_COMPOSITION[0x%X][0x%X]"), wParam, lParam );

	bSelect = IsSelecting( &bSqSel );
	gdSqFillCnt = 0;

	if( (GCS_RESULTSTR & lParam) && bSelect && bSqSel )
	{
		hImc = ImmGetContext( ghViewWnd );

		cbSize = ImmGetCompositionString( hImc, GCS_RESULTSTR, NULL, 0 );

		cbSize += 2;
		ptBuffer = (LPTSTR)malloc( cbSize );
		ZeroMemory( ptBuffer, cbSize );
		ImmGetCompositionString( hImc, GCS_RESULTSTR, ptBuffer, cbSize );
		TRACE( TEXT("COMPOSITION MOZI[%d][%s]"), cbSize, ptBuffer );
		ImmReleaseContext( ghViewWnd , hImc );

		DocSelectedBrushFilling( ptBuffer, &gdDocXdot ,&gdDocLine );
		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
		DocPageInfoRenew( -1, 1 );

		StringCchLength( ptBuffer, cbSize, &cchSize );
		gdSqFillCnt = cchSize;

		FREE(ptBuffer);
	}

	return;
}
