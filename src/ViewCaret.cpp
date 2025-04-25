#include "stdafx.h"
#include "OrinrinEditor.h"

#define CARET_WIDTH	2

extern HWND		ghPrntWnd;
extern HWND		ghViewWnd;

extern INT		gdDocXdot;
extern INT		gdDocLine;
extern INT		gdDocMozi;

extern INT		gdHideXdot;
extern INT		gdViewTopLine;
extern SIZE		gstViewArea;
extern INT		gdDispingLine;

static HBITMAP	ghbmpCaret;

static BOOLEAN	gbCaretShow;

HRESULT	ImeInputBoxPosSet( VOID );

HRESULT	ViewCaretFrameOutCheck( INT, INT, UINT );

HRESULT ViewCaretCreate( HWND hWnd, COLORREF clrMain, COLORREF clrBack )
{
	HDC		hdcMem, hdc = GetDC( hWnd );
	HBITMAP	hBmpOld;
	HBRUSH	hBrushCaret, hBrushBack, hBrushOld;

	clrBack  = ~(clrMain);
	clrBack &= 0x00FFFFFF;

	ghbmpCaret  = CreateCompatibleBitmap( hdc, CARET_WIDTH, LINE_HEIGHT );
	hdcMem      = CreateCompatibleDC( hdc );
	hBmpOld     = SelectBitmap( hdcMem, ghbmpCaret );

	hBrushCaret = CreateSolidBrush( clrMain );
	hBrushBack  = CreateSolidBrush( clrBack );

	hBrushOld   = SelectBrush( hdcMem, hBrushCaret );
	PatBlt( hdcMem, 0, 0, CARET_WIDTH, LINE_HEIGHT, PATCOPY );
	SelectBrush( hdcMem, hBrushBack );
	PatBlt( hdcMem, 0, 0, CARET_WIDTH, LINE_HEIGHT, PATINVERT );

	SelectBrush( hdcMem, hBrushOld );
	SelectBitmap( hdcMem, hBmpOld );

	DeleteBrush( hBrushCaret );
	DeleteBrush( hBrushBack );
	DeleteDC( hdcMem );

	ReleaseDC( hWnd, hdc );

	CreateCaret( hWnd, ghbmpCaret, CARET_WIDTH, LINE_HEIGHT );

	gbCaretShow = FALSE;

	return S_OK;
}

HRESULT ViewCaretDelete( VOID )
{
	if( ghbmpCaret )
	{
		DestroyCaret(  );
		DeleteObject( ghbmpCaret );
	}
	ghbmpCaret = NULL;

	return S_OK;
}

BOOL ViewShowCaret( VOID )
{
	BOOL	bRslt;

	bRslt = ShowCaret( ghViewWnd  );

	if( !(bRslt) )
	{
		bRslt = CreateCaret( ghViewWnd, ghbmpCaret, CARET_WIDTH, LINE_HEIGHT );
		TRACE( TEXT("CARET reset %u"), bRslt );
		gbCaretShow = FALSE;
		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
	}

	return bRslt;
}

VOID ViewHideCaret( VOID )
{
	HideCaret( ghViewWnd  );
	gbCaretShow = FALSE;

	return;
}

HRESULT ViewCaretReColour( COLORREF crtColour )
{

	ViewCaretDelete(  );

	ViewCaretCreate( ghViewWnd, crtColour, 0 );

	ViewShowCaret(  );

	return S_OK;
}

BOOLEAN ViewPosResetCaret( INT xDot, INT yLine )
{
	if( 0 > xDot )	xDot  = 0;
	if( 0 > yLine )	yLine = 0;

	gdDocLine = yLine;
	gdDocMozi = DocLetterPosGetAdjust( &xDot, yLine, 0 );
	gdDocXdot = xDot;

	return ViewDrawCaret( xDot, yLine, TRUE );
}

BOOLEAN ViewDrawCaret( INT rdXdot, INT rdLine, BOOLEAN bOnScr )
{
	INT		dX, dY, loop;
	BOOLEAN	bRslt, fRslt, cRslt;
	POINT	stCaret;

	dX = rdXdot;
	dY = rdLine * LINE_HEIGHT;

	stCaret.x = rdXdot;
	stCaret.y = rdLine;
	DocCaretPosMemory( INIT_SAVE, &stCaret );

	if( bOnScr )	ViewCaretFrameOutCheck( dX, dY, 1 );

	ViewPositionTransform( &dX, &dY, 1 );

	fRslt = ViewIsPosOnFrame( dX , dY );
	if( fRslt )
	{
		bRslt = SetCaretPos( dX, dY );

		if( !(gbCaretShow) )
		{
			for( loop = 0; 10 > loop; loop++ )
			{
				cRslt = ShowCaret( ghViewWnd  );
				if( cRslt )	break;
			}
		}
		gbCaretShow = TRUE;
	}
	else
	{
		if( gbCaretShow )
		{
			for( loop = 0; 10 > loop; loop++ )
			{
				cRslt = HideCaret( ghViewWnd  );
				if( cRslt )	break;
			}
		}
		gbCaretShow = FALSE;
	}

	ViewNowPosStatus(  );

	ImeInputBoxPosSet(  );

	return fRslt;
}

HRESULT ViewCaretFrameOutCheck( INT dDotX, INT dDotY, UINT dummy )
{
	BOOLEAN	bRedraw = FALSE;
	INT	opX, opY;
	INT	bkWid;

	if( gdHideXdot > dDotX )
	{
		gdHideXdot = dDotX;

		bRedraw = TRUE;
	}

	opX = dDotX - gdHideXdot;
	if( gstViewArea.cx < (opX + EOF_WIDTH) )
	{
		bkWid = (opX + EOF_WIDTH) - gstViewArea.cx;

		gdHideXdot += bkWid;

		bRedraw = TRUE;
	}

	if( gdViewTopLine > gdDocLine )
	{
		assert( gdViewTopLine );
		gdViewTopLine = gdDocLine;

		bRedraw = TRUE;
	}

	opY = gdDocLine - gdViewTopLine;
	if( gdDispingLine <= opY )
	{
		gdViewTopLine = gdDocLine - gdDispingLine + 1;

		bRedraw = TRUE;
	}

	if( bRedraw )	ViewRedrawSetLine( -1 );

	return S_OK;
}

HRESULT ImeInputBoxPosSet( VOID )
{
	COMPOSITIONFORM	stCompForm;
	HIMC	hImc;
	POINT	stPoint;

	hImc = ImmGetContext( ghViewWnd );

	if( hImc )
	{
		GetCaretPos( &stPoint );
		stCompForm.dwStyle = CFS_POINT;
		stCompForm.ptCurrentPos.x = stPoint.x;
		stCompForm.ptCurrentPos.y = stPoint.y;

		ImmSetCompositionWindow( hImc, &stCompForm );

		ImmReleaseContext( ghViewWnd , hImc );
	}

	return S_OK;
}

INT ViewCaretPosGet( PINT pXdot, PINT pYline )
{
	if( pXdot ){	*pXdot  = gdDocXdot;	}
	if( pYline ){	*pYline = gdDocLine;	}

	return gdDocMozi;
}
