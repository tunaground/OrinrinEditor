#include "stdafx.h"
#include "SplitBar.h"

LRESULT	CALLBACK SplitProc( HWND, UINT, WPARAM, LPARAM );

VOID	Spt_OnPaint( HWND );
VOID	Spt_OnLButtonDown( HWND, BOOL, INT, INT, UINT );
VOID	Spt_OnMouseMove( HWND, INT, INT, UINT );
VOID	Spt_OnLButtonUp( HWND, INT, INT, UINT );

ATOM SplitBarClass( HINSTANCE hInst )
{
	WNDCLASSEX	wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= SplitProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor( NULL, IDC_SIZEWE );
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= SPLITBAR_CLASS;
	wcex.hIconSm		= NULL;

	return RegisterClassEx( &wcex );
}

HWND SplitBarCreate( HINSTANCE hInst, HWND hPrWnd, INT x, INT y, INT dHeight )
{
	HWND	hWorkWnd;

	hWorkWnd = CreateWindowEx( WS_EX_WINDOWEDGE, SPLITBAR_CLASS, TEXT("분할바"),
		WS_CHILD | WS_VISIBLE, x, y, SPLITBAR_WIDTH, dHeight,
		hPrWnd, NULL, hInst, NULL );
	SetWindowLongPtr( hWorkWnd, GWLP_USERDATA, 0 );

	return hWorkWnd;
}

LRESULT CALLBACK SplitProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT,       Spt_OnPaint );
		HANDLE_MSG( hWnd, WM_LBUTTONDOWN, Spt_OnLButtonDown );
		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Spt_OnMouseMove );
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Spt_OnLButtonUp );

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Spt_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	EndPaint( hWnd, &ps );

	return;
}

VOID Spt_OnLButtonDown( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	if( fDoubleClick )	return;

	SetWindowLongPtr( hWnd, GWLP_USERDATA, 1 );

	SetCapture( hWnd );

	return;
}

VOID Spt_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	HWND	hPrWnd;
	RECT	stRect;

	LONG	wWidth = 0;
	LONG	wLeft = 0;
	LONG	bSpliting;

	bSpliting = GetWindowLongPtr( hWnd, GWLP_USERDATA );

	if( bSpliting )
	{
		hPrWnd = GetParent( hWnd );
		GetClientRect( hPrWnd, &stRect );
		wWidth = stRect.right;

		SplitBarPosGet( hWnd, &stRect );
		wLeft = stRect.left + x;

		if( wLeft < SPLITBAR_LEFTLIMIT){	wLeft =  SPLITBAR_LEFTLIMIT;	}
		else if( wLeft >  wWidth - SPLITBAR_LEFTLIMIT ){	wLeft = wWidth - SPLITBAR_LEFTLIMIT;	}

		SetWindowPos( hWnd, HWND_TOP, wLeft, stRect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	}

	return;
}

VOID Spt_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	HWND	hPrWnd;
	RECT	stRect;

	LONG	bSpliting;

	bSpliting = GetWindowLongPtr( hWnd, GWLP_USERDATA );

	if( bSpliting )
	{
		hPrWnd = GetParent( hWnd );

		ReleaseCapture(   );
		SetWindowPos( hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
		GetClientRect( hPrWnd, &stRect );

#ifdef SPLIT_BAR_POS_FIX
		FORWARD_WM_SIZE( hPrWnd, SIZE_SPLITBAR_MOVED, x, y, PostMessage );
#else
		FORWARD_WM_SIZE( hPrWnd, SIZE_RESTORED, x, y, PostMessage );
#endif
	}

	SetWindowLongPtr( hWnd, GWLP_USERDATA, 0 );

	return;
}

VOID SplitBarPosGet( HWND hSplitWnd, LPRECT ptRect )
{
	HWND	hPrWnd = GetParent( hSplitWnd );
	RECT	clRect;
	POINT	point;
	LONG	wHeight;

	GetWindowRect( hSplitWnd, &clRect );
	wHeight = clRect.bottom - clRect.top;
	point.x = clRect.left;
	point.y = clRect.top;

	ScreenToClient( hPrWnd, &point );

	SetRect( ptRect, point.x, point.y, SPLITBAR_WIDTH, wHeight );

	return;
}

LONG SplitBarResize( HWND hSplitWnd, LPRECT ptRect )
{
	RECT	rect;

	SplitBarPosGet( hSplitWnd, &rect );

	SetWindowPos( hSplitWnd, HWND_TOP, rect.left, ptRect->top, SPLITBAR_WIDTH, ptRect->bottom, 0 );

	InvalidateRect( hSplitWnd, NULL, TRUE );
	UpdateWindow( hSplitWnd );

	return rect.left;
}
