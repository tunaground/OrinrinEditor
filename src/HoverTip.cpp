#include "stdafx.h"
#include "OrinrinEditor.h"

#ifdef USE_HOVERTIP

#define HOVER_TIPS_CLASS	TEXT("HOVER_TIPS")

#define HOVER_TIME	15000
#define HOVER_TMID	1234
#define HOVER_MOVE	   8

#define HOVER_DELAY	1000

static  ATOM	gTipAtom;
static  HWND	ghTipWnd;
static HFONT	ghTipFont;

static  UINT	gdMoveVol;

static LPTSTR	gptContent;
static RECT		gstContSize;

VOID	HoverTipClose( HWND );

LRESULT	CALLBACK HoverTipProc( HWND, UINT, WPARAM, LPARAM );
VOID	Htp_OnPaint( HWND );
VOID	htp_OnTimer( HWND, UINT );
VOID	Htp_OnKillFocus( HWND, HWND );
VOID	Htp_OnLButtonUp( HWND, INT, INT, UINT );
VOID	Htp_OnMButtonUp( HWND, INT, INT, UINT );
VOID	Htp_OnRButtonUp( HWND, INT, INT, UINT );
VOID	Htp_OnMouseMove( HWND, INT, INT, UINT );

HRESULT HoverTipInitialise( HINSTANCE hInstance, HWND hPtWnd )
{
	LOGFONT	stFont;
	WNDCLASSEX	wcex;
	INT	ttSize;

	if( hInstance )
	{
		gptContent = NULL;
		gdMoveVol = 0;

		ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= HoverTipProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_INFOBK+1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= HOVER_TIPS_CLASS;
		wcex.hIconSm		= NULL;

		gTipAtom = RegisterClassEx( &wcex );

		ghTipWnd = CreateWindowEx( WS_EX_TOOLWINDOW, HOVER_TIPS_CLASS, TEXT("정보 팁"), WS_POPUP | WS_BORDER, 0, 0, 15, 15, NULL, NULL, hInstance, NULL );

		ViewingFontGet( &stFont );
		ttSize = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, FONTSZ_REDUCE );
		stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
		ghTipFont = CreateFontIndirect( &stFont );
	}
	else
	{
		DeleteFont( ghTipFont );

		DestroyWindow( ghTipWnd );

		FREE( gptContent );
	}

	return S_OK;
}

HRESULT HoverTipResist( HWND hTgtWnd )
{

	TRACKMOUSEEVENT	stTrackMsEv;

	TRACE( TEXT("마우스 호버 등록") );

	ZeroMemory( &stTrackMsEv, sizeof(TRACKMOUSEEVENT) );
	stTrackMsEv.cbSize      = sizeof(TRACKMOUSEEVENT);
	stTrackMsEv.dwFlags     = TME_HOVER | TME_LEAVE;
	stTrackMsEv.hwndTrack   = hTgtWnd;
	stTrackMsEv.dwHoverTime = HOVER_DELAY;
	TrackMouseEvent( &stTrackMsEv );

	return S_OK;
}

HRESULT HoverTipSizeChange( INT ttSize )
{
	LOGFONT	stFont;

	DeleteFont( ghTipFont );

	ViewingFontGet( &stFont );
	stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
	ghTipFont = CreateFontIndirect( &stFont );

	return S_OK;
}

LRESULT HoverTipOnMouseHover( HWND hEvWnd, WPARAM wParam, LPARAM lParam, HOVERTIPDISP pfInfoGet )
{
	INT		x, y;
	UINT	keyFlags;
	LPTSTR	ptText;

	HDC		hdc;
	HFONT	hOldFnt;

	RECT	deskRect;
	POINT	point;
	INT		xSub, ySub;

	TRACE( TEXT("마우스 호버 발생") );

	keyFlags = (UINT)wParam;

	x = (INT)(SHORT)LOWORD(lParam);
	y = (INT)(SHORT)HIWORD(lParam);

	point.x = x;
	point.y = y;
	ClientToScreen( hEvWnd, &point );

	FREE( gptContent );

	ptText = pfInfoGet( NULL );
	if( !(ptText) )
	{
		return 0;
	}

	gptContent = ptText;

	hdc = GetDC( ghTipWnd );

	SetRect( &gstContSize, 0, 0, 2222, 100 );

	hOldFnt = SelectFont( hdc , ghTipFont );

	DrawText( hdc, gptContent, -1, &gstContSize, DT_LEFT | DT_CALCRECT | DT_NOPREFIX );
	TRACE( TEXT("호버 크기[ %d x %d, %d : %d"), gstContSize.left, gstContSize.top, gstContSize.right, gstContSize.bottom );

	SelectFont( hdc, hOldFnt );

	gstContSize.right  += 4;
	gstContSize.bottom += 4;

	GetWindowRect( GetDesktopWindow(), &deskRect );

	if( gstContSize.right  >  deskRect.right  ){	gstContSize.right  = deskRect.right;	}
	if( gstContSize.bottom >  deskRect.bottom ){	gstContSize.bottom = deskRect.bottom;	}

	xSub = (point.x + gstContSize.right) - deskRect.right;
	if( 0 <  xSub ){	point.x -= xSub;	}
	ySub = (point.y + gstContSize.bottom) - deskRect.bottom;
	if( 0 <  ySub ){	point.y -= ySub;	}

	gdMoveVol = 0;

	SetWindowPos( ghTipWnd, HWND_TOPMOST, (point.x + 1), (point.y + 1), gstContSize.right, gstContSize.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE );
	SetTimer( ghTipWnd , HOVER_TMID, HOVER_TIME, NULL );

	ReleaseDC( ghTipWnd, hdc );

	return 0;
}

LRESULT HoverTipOnMouseLeave( HWND hEvWnd )
{
	TRACE( TEXT("마우스 떠남 발생") );

	return 0;
}

VOID HoverTipClose( HWND hWnd )
{
	KillTimer( hWnd, HOVER_TMID );
	ShowWindow( hWnd, SW_HIDE );

	return;
}

LRESULT CALLBACK HoverTipProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Htp_OnLButtonUp );
		HANDLE_MSG( hWnd, WM_MBUTTONUP,   Htp_OnMButtonUp );
		HANDLE_MSG( hWnd, WM_RBUTTONUP,   Htp_OnRButtonUp );
		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Htp_OnMouseMove );
		HANDLE_MSG( hWnd, WM_KILLFOCUS,   Htp_OnKillFocus );
		HANDLE_MSG( hWnd, WM_PAINT,       Htp_OnPaint );
		HANDLE_MSG( hWnd, WM_TIMER,       htp_OnTimer );

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Htp_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;
	HFONT		hOldFnt;
	RECT	rect;

	hdc = BeginPaint( hWnd, &ps );

	hOldFnt = SelectFont( hdc, ghTipFont );
	SetTextColor( hdc, GetSysColor( COLOR_INFOTEXT ) );
	SetBkMode( hdc, TRANSPARENT );

	rect = gstContSize;
	rect.left += 2;
	rect.top  += 2;

	DrawText( hdc, gptContent, -1, &rect, DT_LEFT | DT_NOPREFIX );

	SelectFont( hdc, hOldFnt );

	EndPaint( hWnd, &ps );

	return;
}

VOID htp_OnTimer( HWND hWnd, UINT id )
{

	if( HOVER_TMID != id )	return;

	HoverTipClose( hWnd );

	return;
}

VOID Htp_OnKillFocus( HWND hWnd, HWND hwndNewFocus )
{

	return;
}

VOID Htp_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TRACE( TEXT("HTP 왼쪽 버튼 업 %d x %d"), x , y );

	HoverTipClose( hWnd );

	return;
}

VOID Htp_OnMButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TRACE( TEXT("HTP 중간 버튼 업 %d x %d"), x , y );

	HoverTipClose( hWnd );

	return;
}

VOID Htp_OnRButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TRACE( TEXT("HTP 오른쪽 버튼 업 %d x %d"), x , y );

	HoverTipClose( hWnd );

	return;
}

VOID Htp_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	gdMoveVol++;
	if( HOVER_MOVE < gdMoveVol )
	{
		HoverTipClose( hWnd );
	}

	return;
}

#endif
