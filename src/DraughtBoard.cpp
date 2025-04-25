#include "stdafx.h"
#include "OrinrinEditor.h"

#include "MaaTemplate.h"

#define DRAUGHT_BOARD_CLASS	TEXT("DRAUGHT_BOARD")

#define TTMSG_NO_ITEM	TEXT("아이템 없음")

#define DTHMSZ_ULTRALIGHT	 80
#define DTHMSZ_REGULAR		128
#define DTHMSZ_DEMIBOLD		160
#define DTHMSZ_ULTRABOLD	192

#define TPNL_HORIZ	5
#define TPNL_VERTI	3

extern HFONT	ghAaFont;

#ifdef MAA_TOOLTIP
extern HFONT	ghTipFont;

static  HWND	ghDrghtTipWnd;
static LPTSTR	gptTipBuffer;
#endif

extern  UINT	gbAAtipView;

static  HWND	ghPtWnd;

static  ATOM	gDraughtAtom;
static  HWND	ghDraughtWnd;

EXTERNED UINT	gdClickDrt;
EXTERNED UINT	gdSubClickDrt;

static HDC		ghNonItemDC;
static HBITMAP	ghNonItemBMP, ghOldBmp;
static HPEN		ghLinePen;

static HFONT	ghAreaFont;

static INT		giItemSel;
static INT		giTarget;

static  UINT	gbThumb;
static  LONG	gdVwTop;
static  HWND	ghScrBarWnd;

static INT		giItemWidth;
static INT		giItemHeight;

static POINT	gstViewLsPt;

static vector<AAMATRIX>	gvcDrtItems;

INT		DraughtTargetItemSet( LPPOINT );
DOUBLE	DraughtAspectKeeping( LPSIZE, UINT );
INT		DraughtItemDelete( CONST INT  );
HRESULT	DraughtItemUse( HWND, INT );
HRESULT	DraughtItemExport( HWND, LPTSTR );
VOID	DraughtButtonUp( HWND, INT, INT, UINT, UINT );
HRESULT	DraughtFrameResize( HWND, INT, INT );

LRESULT CALLBACK DraughtProc( HWND, UINT, WPARAM, LPARAM );
VOID	Drt_OnCommand( HWND , INT, HWND, UINT );
VOID	Drt_OnPaint( HWND );

VOID	Drt_OnMouseMove( HWND, INT, INT, UINT );
VOID	Drt_OnLButtonUp( HWND, INT, INT, UINT );
VOID	Drt_OnMButtonUp( HWND, INT, INT, UINT );
VOID	Drt_OnContextMenu( HWND, HWND, UINT, UINT );
VOID	Drt_OnDestroy( HWND );
VOID	Drt_OnKillFocus( HWND, HWND );
VOID	Drt_OnVScroll( HWND , HWND, UINT, INT );
VOID	Drt_OnMouseWheel( HWND, INT, INT, INT, UINT );
#ifdef MAA_TOOLTIP
LRESULT	Drt_OnNotify( HWND , INT, LPNMHDR );
#endif

#ifdef USE_HOVERTIP
LPTSTR	CALLBACK DraughtHoverTipInfo( LPVOID  );
#endif

HRESULT DraughtInitialise( HINSTANCE hInstance, HWND hPtWnd )
{
	WNDCLASSEX	wcex;

	if( hInstance )
	{
		ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= DraughtProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= DRAUGHT_BOARD_CLASS;
		wcex.hIconSm		= NULL;

		gDraughtAtom = RegisterClassEx( &wcex );

		ghNonItemDC = NULL;

		giItemWidth  = InitParamValue( INIT_LOAD, VL_THUMB_HORIZ, DTHMSZ_REGULAR );
		giItemHeight = InitParamValue( INIT_LOAD, VL_THUMB_VERTI, DTHMSZ_REGULAR );

		ghAreaFont = CreateFont( FONTSZ_REDUCE, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, TEXT("MS UI Gothic") );

		ghPtWnd = hPtWnd;

		gstViewLsPt.x = -1;

		giItemSel = -1;

#ifndef _ORRVW

		gdClickDrt    = InitParamValue( INIT_LOAD, VL_DRT_LCLICK, MAA_INSERT );
		gdSubClickDrt = InitParamValue( INIT_LOAD, VL_DRT_MCLICK, MAA_INSERT );

#endif
	}
	else
	{
		if( ghNonItemDC )
		{
			SelectBitmap( ghNonItemDC, ghOldBmp );
			SelectPen( ghNonItemDC, GetStockPen(NULL_PEN) );
			DeleteObject( ghNonItemDC );
		}
		if( ghNonItemBMP  ){	DeleteBitmap( ghNonItemBMP );	}
		if( ghLinePen ){	DeletePen( ghLinePen  );	}
		if( ghAreaFont ){	DeleteFont( ghAreaFont );	}

#ifdef MAA_TOOLTIP
		FREE( gptTipBuffer );
#endif
		DraughtItemDelete( -1 );
	}

	return S_OK;
}

HWND DraughtWindowCreate( HINSTANCE hInstance, HWND hPtWnd, UINT bThumb )
{
	INT_PTR	iItems;
	INT		iRslt, iScWid = 0, iScHei;
	HDC		hdc;

	INT		iBrdrWid = 0;
	TCHAR	atCaption[SUB_STRING];

	RECT	wdRect, rect;
#ifdef MAA_TOOLTIP
	TTTOOLINFO	stToolInfo;
#endif

	INT		iCapHei, iXfrm, iYfrm;
	INT		iLines, iStep = 0;
	LONG	rigOffs = 0;
	SCROLLINFO	stScrollInfo;

	if( !(hPtWnd ) )
	{
		return NULL;
	}

	if( ghDraughtWnd  ){	UpdateWindow( ghDraughtWnd );	return ghDraughtWnd;	}

	gbThumb = bThumb;

	iItems = gvcDrtItems.size( );

	iCapHei = GetSystemMetrics( SM_CYSMCAPTION );
	iXfrm   = GetSystemMetrics( SM_CXFIXEDFRAME );
	iYfrm   = GetSystemMetrics( SM_CYFIXEDFRAME );

	if( 0 >  gstViewLsPt.x )
	{
		GetWindowRect( hPtWnd, &wdRect );
		rect.left   = wdRect.left + 32;
		rect.top    = wdRect.top  + 32;

		gstViewLsPt.x = rect.left;
		gstViewLsPt.y = rect.top;
	}
	else
	{
		rect.left   = gstViewLsPt.x;
		rect.top    = gstViewLsPt.y;
	}
	rect.right  = (giItemWidth  * TPNL_HORIZ) + (iXfrm * 2);
	rect.bottom = (giItemHeight * TPNL_VERTI);
	iScHei = rect.bottom;
	rect.bottom += ((iYfrm * 2) + iCapHei);

	if( gbThumb )
	{
		gdVwTop = 0;

		iItems = AacItemCount( 0  );
		if( 0 >= iItems )	return NULL;

		iLines = (iItems + (TPNL_HORIZ-1)) / TPNL_HORIZ;

		iStep = iLines - TPNL_VERTI;
		if( 0 > iStep ){	iStep = 0;	}

		rigOffs = rect.right;

		iScWid = GetSystemMetrics( SM_CXVSCROLL );
		rect.right += iScWid;

		iBrdrWid = GetSystemMetrics( SM_CXFIXEDFRAME );
		rect.right += (iBrdrWid*2);

		StringCchCopy( atCaption, SUB_STRING, TEXT("MAA 썸네일") );
	}
	else
	{
		StringCchCopy( atCaption, SUB_STRING, TEXT("드래프트 보드") );
	}

	ghDraughtWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		DRAUGHT_BOARD_CLASS, atCaption, WS_POPUP | WS_VISIBLE | WS_CAPTION,
		rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, hInstance, NULL );

#ifdef MAA_TOOLTIP
	FREE( gptTipBuffer );

	ghDrghtTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghDraughtWnd, NULL, hInstance, NULL );
	SetWindowFont( ghDrghtTipWnd, ghTipFont, TRUE );

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	GetClientRect( ghDraughtWnd, &stToolInfo.rect );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;
	stToolInfo.hwnd     = ghDraughtWnd;
	stToolInfo.uId      = IDTT_DRT_TOOLTIP;
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage( ghDrghtTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghDrghtTipWnd, TTM_SETMAXTIPWIDTH, 0 , 0 );
#endif

	if( gbThumb )
	{

		ghScrBarWnd = CreateWindowEx( 0, WC_SCROLLBAR, TEXT("scroll"), WS_VISIBLE | WS_CHILD | SBS_VERT,
			rigOffs, 0, iScWid, iScHei, ghDraughtWnd, (HMENU)IDSB_DRT_THUM_SCROLL, hInstance, NULL );

		ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
		stScrollInfo.cbSize = sizeof(SCROLLINFO);
		stScrollInfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		stScrollInfo.nMax  = iStep;
		stScrollInfo.nPos  = 0;
		stScrollInfo.nPage = 1;
		stScrollInfo.nTrackPos = 0;
		SetScrollInfo( ghScrBarWnd, SB_CTL, &stScrollInfo, TRUE );
	}

	if( !(ghNonItemDC) )
	{
		hdc = GetDC( ghDraughtWnd );

		ghNonItemDC  = CreateCompatibleDC( hdc );
		ghNonItemBMP = CreateCompatibleBitmap( hdc, giItemWidth, giItemHeight );

		ghOldBmp = SelectBitmap( ghNonItemDC, ghNonItemBMP );
		SelectFont( ghNonItemDC, ghAaFont );

		iRslt = PatBlt( ghNonItemDC, 0, 0, giItemWidth, giItemHeight, WHITENESS );

		ReleaseDC( ghDraughtWnd, hdc );

		SetRect( &rect, 0, 0, giItemWidth, giItemHeight );
		iRslt = DrawText( ghNonItemDC, TEXT("아이템 없음"), 7, &rect, DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP | DT_SINGLELINE );

		SelectFont( ghNonItemDC, GetStockFont(DEFAULT_GUI_FONT) );

		ghLinePen = CreatePen( PS_SOLID, 1, 0 );
		SelectPen( ghNonItemDC, ghLinePen );
	}

	UpdateWindow( ghDraughtWnd );

	return ghDraughtWnd;
}

HRESULT DraughtFrameResize( HWND hWnd, INT iWidth, INT iHeight )
{
	INT		iBrdrWid = 0;
	INT		iScWid = 0, iScHei;
	INT		iCapHei, iXfrm, iYfrm;
	LONG	rigOffs = 0;
	RECT	rect;

	giItemWidth  = iWidth;
	giItemHeight = iHeight;

	iCapHei = GetSystemMetrics( SM_CYSMCAPTION );
	iXfrm   = GetSystemMetrics( SM_CXFIXEDFRAME );
	iYfrm   = GetSystemMetrics( SM_CYFIXEDFRAME );

	rect.left   = gstViewLsPt.x;
	rect.top    = gstViewLsPt.y;
	rect.right  = (giItemWidth  * TPNL_HORIZ) + (iXfrm * 2);
	rect.bottom = (giItemHeight * TPNL_VERTI);
	iScHei      = rect.bottom;
	rect.bottom += ((iYfrm * 2) + iCapHei);

	if( gbThumb )
	{
		rigOffs = rect.right;

		iScWid = GetSystemMetrics( SM_CXVSCROLL );
		rect.right += iScWid;

		iBrdrWid = GetSystemMetrics( SM_CXFIXEDFRAME );
		rect.right += (iBrdrWid*2);
	}

	SetWindowPos( ghDraughtWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOMOVE );

	if( gbThumb )
	{
		SetWindowPos( ghScrBarWnd, HWND_TOP, rigOffs, 0, iScWid, iScHei, SWP_NOZORDER );
	}

	InvalidateRect( ghDraughtWnd, NULL, TRUE );

	InitParamValue( INIT_SAVE, VL_THUMB_HORIZ, giItemWidth );
	InitParamValue( INIT_SAVE, VL_THUMB_VERTI, giItemHeight );

	return S_OK;
}

LRESULT CALLBACK DraughtProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{

		HANDLE_MSG( hWnd, WM_COMMAND,     Drt_OnCommand );
		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Drt_OnMouseMove );
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Drt_OnLButtonUp );
		HANDLE_MSG( hWnd, WM_MBUTTONUP,   Drt_OnMButtonUp );
		HANDLE_MSG( hWnd, WM_PAINT,       Drt_OnPaint );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Drt_OnContextMenu );
		HANDLE_MSG( hWnd, WM_DESTROY,     Drt_OnDestroy );
		HANDLE_MSG( hWnd, WM_KILLFOCUS,   Drt_OnKillFocus );
		HANDLE_MSG( hWnd, WM_VSCROLL,     Drt_OnVScroll );
		HANDLE_MSG( hWnd, WM_MOUSEWHEEL,  Drt_OnMouseWheel );
#ifdef MAA_TOOLTIP
		HANDLE_MSG( hWnd, WM_NOTIFY,      Drt_OnNotify );
#endif

#ifdef USE_HOVERTIP
		case WM_MOUSEHOVER:
			HoverTipOnMouseHover( hWnd, wParam, lParam, DraughtHoverTipInfo );
			return 0;

		case WM_MOUSELEAVE:
			HoverTipOnMouseLeave( hWnd );
			giItemSel = -1;
			return 0;
#endif

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Drt_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{

	switch( id )
	{
#ifndef _ORRVW
		case IDM_DRAUGHT_INSERTEDIT:
		case IDM_DRAUGHT_INTERRUPTEDIT:
		case IDM_DRAUGHT_LAYERBOX:
#endif
		case IDM_DRAUGHT_UNICLIP:
		case IDM_DRAUGHT_SJISCLIP:		DraughtItemUse( hWnd , id );	DestroyWindow( hWnd );	break;

		case IDM_THUMB_DRAUGHT_ADD:		DraughtItemUse( hWnd , id );	break;

		case IDM_DRAUGHT_DELETE:		DraughtItemDelete( giTarget );	InvalidateRect( hWnd , NULL, TRUE );	break;
		case IDM_DRAUGHT_ALLDELETE:		DraughtItemDelete( -1 );	DestroyWindow( hWnd );	break;

		case IDM_DB_THUMB_ULTRALIGHT:	DraughtFrameResize( hWnd, DTHMSZ_ULTRALIGHT, DTHMSZ_ULTRALIGHT );	break;
		case IDM_DB_THUMB_REGULAR:		DraughtFrameResize( hWnd, DTHMSZ_REGULAR,    DTHMSZ_REGULAR );	break;
		case IDM_DB_THUMB_DEMIBOLD:		DraughtFrameResize( hWnd, DTHMSZ_DEMIBOLD,   DTHMSZ_DEMIBOLD );	break;
		case IDM_DB_THUMB_ULTRABOLD:	DraughtFrameResize( hWnd, DTHMSZ_ULTRABOLD,  DTHMSZ_ULTRABOLD );	break;

		case IDM_DRAUGHT_EXPORT:		DraughtItemExport( hWnd, NULL );	break;
		case IDM_DRAUGHT_CLOSE:			DestroyWindow( hWnd );	break;

		default:	break;
	}

	return;
}

VOID Drt_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc, hAaDC;
	HBITMAP		hOldBmp, hBmp;
	HFONT		hOldFnt;
	UINT		x = 0, y = 0;
	INT_PTR		iItems;
	UINT_PTR	cchLen;
	TCHAR		atArea[MIN_STRING];
	SIZE		stSize, stOrgSize, stArea;

	MAAM_ITR	itItem;

	hdc = BeginPaint( hWnd, &ps );

	hOldFnt = SelectFont( hdc, ghAreaFont );

	SetStretchBltMode( hdc, HALFTONE );

	if( gbThumb )
	{
		iItems = gdVwTop * TPNL_HORIZ;

		for( y = 0; TPNL_VERTI > y; y++ )
		{
			for( x = 0; TPNL_HORIZ > x; x++ )
			{
				ZeroMemory( atArea, sizeof(atArea) );

				hBmp = AacArtImageGet( hWnd, iItems, &stSize, &stArea );
				if( hBmp )
				{
					stOrgSize = stSize;
					DraughtAspectKeeping( &stSize, TRUE );

					hAaDC = CreateCompatibleDC( hdc );
					hOldBmp = SelectBitmap( hAaDC, hBmp );

					StretchBlt( hdc, (x * giItemWidth), (y * giItemHeight), stSize.cx, stSize.cy,
						hAaDC, 0, 0, stOrgSize.cx, stOrgSize.cy,
						SRCCOPY );

					SelectBitmap( hAaDC, hOldBmp );

					StringCchPrintf( atArea, MIN_STRING, TEXT("%dDOT x %dLINE"), stArea.cx, stArea.cy );
					StringCchLength( atArea, MIN_STRING, &cchLen );
					ExtTextOut( hdc, (x * giItemWidth)+1, ((y+1) * giItemHeight)-12, 0, NULL, atArea, cchLen, NULL );

					DeleteDC( hAaDC );

					iItems++;
				}
				else
				{
					BitBlt( hdc, (x * giItemWidth), (y * giItemHeight), giItemWidth, giItemHeight, ghNonItemDC, 0, 0, SRCCOPY );
				}
			}
		}
	}
	else
	{
		itItem = gvcDrtItems.begin();

		for( y = 0; TPNL_VERTI > y; y++ )
		{
			for( x = 0; TPNL_HORIZ > x; x++ )
			{
				if( itItem != gvcDrtItems.end() )
				{
					ZeroMemory( atArea, sizeof(atArea) );

					stSize = itItem->stSize;
					DraughtAspectKeeping( &stSize, TRUE );

					hAaDC = CreateCompatibleDC( hdc );
					hOldBmp = SelectBitmap( hAaDC, itItem->hThumbBmp );

					StretchBlt( hdc, (x * giItemWidth), (y * giItemHeight), stSize.cx, stSize.cy,
						hAaDC, 0, 0, itItem->stSize.cx, itItem->stSize.cy,
						SRCCOPY );

					SelectBitmap( hAaDC, hOldBmp );

					StringCchPrintf( atArea, MIN_STRING, TEXT("%dDOT x %dLINE"), itItem->iMaxDot, itItem->iLines );
					StringCchLength( atArea, MIN_STRING, &cchLen );
					ExtTextOut( hdc, (x * giItemWidth)+1, ((y+1) * giItemHeight)-12, 0, NULL, atArea, cchLen, NULL );

					DeleteDC( hAaDC );

					itItem++;
				}
				else
				{
					BitBlt( hdc, (x * giItemWidth), (y * giItemHeight), giItemWidth, giItemHeight, ghNonItemDC, 0, 0, SRCCOPY );
				}
			}
		}
	}

	SelectFont( hdc, hOldFnt );

	for( y = 1; TPNL_HORIZ > y; y++ )
	{
		MoveToEx( hdc, (y * giItemWidth), 0, NULL );
		LineTo( hdc, (y * giItemWidth), (giItemHeight * TPNL_VERTI) );
	}

	for( x = 1; TPNL_VERTI > x; x++ )
	{
		MoveToEx( hdc, 0, (x * giItemHeight), NULL );
		LineTo(   hdc, (giItemWidth * TPNL_HORIZ), (x * giItemHeight) );
	}

	EndPaint( hWnd, &ps );

	return;
}

VOID Drt_OnKillFocus( HWND hWnd, HWND hwndNewFocus )
{
	RECT	rect;

	GetWindowRect( hWnd, &rect );
	gstViewLsPt.x = rect.left;
	gstViewLsPt.y = rect.top;

	DestroyWindow( hWnd );

	return;
}

#ifdef MAA_TOOLTIP

LRESULT Drt_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	INT		iTarget, iOffset, i;
	INT_PTR	iItems;
	LPSTR	pcConts = NULL;
	POINT	stMosPos;
	LPNMTTDISPINFO	pstDispInfo;

	MAAM_ITR	itItem;

	if( TTN_GETDISPINFO ==  pstNmhdr->code )
	{
		GetCursorPos( &stMosPos );
		ScreenToClient( hWnd, &stMosPos );

		pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

		ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );
		pstDispInfo->lpszText = NULL;

		if( !(gbAAtipView) ){	return 0;	}

		FREE( gptTipBuffer );

		iTarget = DraughtTargetItemSet( &stMosPos );
		TRACE( TEXT("TARGET %d"), iTarget );

		if( gbThumb )
		{
			iOffset = gdVwTop * TPNL_HORIZ;
			iTarget = iOffset + iTarget;
			pcConts = AacAsciiArtGet( iTarget );

			gptTipBuffer = SjisDecodeAlloc( pcConts );
			FREE( pcConts );
		}
		else
		{
			iItems = gvcDrtItems.size( );
			if( iItems > iTarget )
			{
				for( i = 0, itItem = gvcDrtItems.begin(); gvcDrtItems.end() != itItem; i++, itItem++ )
				{
					if( iTarget == i )
					{
						gptTipBuffer = SjisDecodeAlloc( itItem->pcItem );
						break;
					}
				}
			}
		}

		if( gptTipBuffer  ){	pstDispInfo->lpszText = gptTipBuffer;	}
		else{					pstDispInfo->lpszText = TTMSG_NO_ITEM;	}
	}

	return 0;
}

#endif

VOID Drt_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	INT		iTarget;
	POINT	point;
	BOOLEAN		bReDraw = FALSE;

	point.x = x;
	point.y = y;

	iTarget = DraughtTargetItemSet( &point );
	if( giItemSel !=  iTarget ){	bReDraw =  TRUE;	}
	giItemSel = iTarget;

#ifdef USE_HOVERTIP

	if( bReDraw && gbAAtipView ){	HoverTipResist( ghDraughtWnd  );	}
#endif

	return;
}

VOID Drt_OnMButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TRACE( TEXT("MUP %d x %d"), x , y );

	DraughtButtonUp( hWnd, x, y, keyFlags, WM_MBUTTONUP );

	return;
}

VOID Drt_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TRACE( TEXT("LUP %d x %d"), x , y );

	DraughtButtonUp( hWnd, x, y, keyFlags, WM_LBUTTONUP );

	return;
}

VOID DraughtButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags, UINT message )
{
	POINT	stPos;
	INT		id;
	UINT	dMode, dSubMode, dStyle;

	stPos.x = x;
	stPos.y = y;

	giTarget = DraughtTargetItemSet( &stPos );
	TRACE( TEXT("TARGET %d"), giTarget );

	if( gbThumb )
	{
		dMode = ViewMaaItemsModeGet( &dSubMode );

		if( WM_MBUTTONUP == message )	dMode = dSubMode;

		switch( dMode )
		{
	#ifndef _ORRVW
			case  0:	id = IDM_DRAUGHT_INSERTEDIT;	break;
			case  1:	id = IDM_DRAUGHT_INTERRUPTEDIT;	break;
			case  2:	id = IDM_DRAUGHT_LAYERBOX;	break;
	#endif
			case  3:	id = IDM_DRAUGHT_UNICLIP;	break;
			default:
			case  4:	id = IDM_DRAUGHT_SJISCLIP;	break;
			case  5:	id = IDM_THUMB_DRAUGHT_ADD;	break;
		}
	}
	else
	{
		if( WM_MBUTTONUP == message ){	dStyle = gdSubClickDrt;	}
		else{							dStyle = gdClickDrt;	}

		switch( dStyle )
		{
	#ifndef _ORRVW
			case  0:	id = IDM_DRAUGHT_INSERTEDIT;	break;
			case  1:	id = IDM_DRAUGHT_INTERRUPTEDIT;	break;
			case  2:	id = IDM_DRAUGHT_LAYERBOX;	break;
	#endif
			case  3:	id = IDM_DRAUGHT_UNICLIP;	break;
			default:
			case  4:	id = IDM_DRAUGHT_SJISCLIP;	break;
		}
	}

	FORWARD_WM_COMMAND( hWnd, id, ghDraughtWnd, 0, SendMessage );

	return;
}

VOID Drt_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;

	POINT	stPoint, stPos;

	stPoint.x = (SHORT)xPos;
	stPoint.y = (SHORT)yPos;

	TRACE( TEXT("CTX %d x %d"), stPoint.x, stPoint.y );

	stPos = stPoint;
	ScreenToClient( hWnd, &stPos );
	giTarget = DraughtTargetItemSet( &stPos );
	TRACE( TEXT("TARGET %d"), giTarget );

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_DRAUGHT_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	if( gbThumb )
	{
		DeleteMenu( hSubMenu, IDM_DRAUGHT_ALLDELETE, MF_BYCOMMAND );
		DeleteMenu( hSubMenu, IDM_DRAUGHT_EXPORT,    MF_BYCOMMAND );

		ModifyMenu( hSubMenu, IDM_DRAUGHT_CLOSE,     MF_BYCOMMAND | MFT_STRING, IDM_DRAUGHT_CLOSE, TEXT("썸네일 닫기(&Q)") );
		ModifyMenu( hSubMenu, IDM_DRAUGHT_DELETE,    MF_BYCOMMAND | MFT_STRING, IDM_THUMB_DRAUGHT_ADD, TEXT("드래프트 보드에 추가(&D)") );
	}

	if( giItemWidth == giItemHeight )
	{
		switch( giItemWidth )
		{
			case DTHMSZ_ULTRALIGHT:	CheckMenuItem( hSubMenu, IDM_DB_THUMB_ULTRALIGHT, MF_CHECKED );	break;
			case DTHMSZ_REGULAR:	CheckMenuItem( hSubMenu, IDM_DB_THUMB_REGULAR, MF_CHECKED );	break;
			case DTHMSZ_DEMIBOLD:	CheckMenuItem( hSubMenu, IDM_DB_THUMB_DEMIBOLD, MF_CHECKED );	break;
			case DTHMSZ_ULTRABOLD:	CheckMenuItem( hSubMenu, IDM_DB_THUMB_ULTRABOLD, MF_CHECKED );	break;
			default:	break;
		}
	}

	dRslt = TrackPopupMenu( hSubMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );

	DestroyMenu( hMenu );

	return;
}

VOID Drt_OnDestroy( HWND hWnd )
{
	ghDraughtWnd = NULL;
	ghScrBarWnd  = NULL;

#ifdef MAA_TOOLTIP
	FREE( gptTipBuffer );
#endif
	return;
}

VOID Drt_OnMouseWheel( HWND hWnd, INT xPos, INT yPos, INT zDelta, UINT fwKeys )
{
	UINT	dCode;

	TRACE( TEXT("POS[%d x %d] DELTA[%d] K[%X]"), xPos, yPos, zDelta, fwKeys );

	if( 0 < zDelta )		dCode = SB_LINEUP;
	else if( 0 > zDelta )	dCode = SB_LINEDOWN;
	else					dCode = SB_ENDSCROLL;

	Drt_OnVScroll( hWnd, ghScrBarWnd, dCode, 1 );

	return;
}

VOID Drt_OnVScroll( HWND hWnd, HWND hwndCtl, UINT code, INT pos )
{
	INT	maePos;
	SCROLLINFO	stScrollInfo;

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( ghScrBarWnd, SB_CTL, &stScrollInfo );

	maePos = gdVwTop;

	switch( code )
	{
		case SB_TOP:
			gdVwTop = 0;
			break;

		case SB_LINEUP:
			gdVwTop--;
			if( 0 > gdVwTop )	gdVwTop = 0;
			break;

		case SB_PAGEUP:
			gdVwTop -= 3;
			if( 0 > gdVwTop )	gdVwTop = 0;
			break;

		case SB_LINEDOWN:
			gdVwTop++;
			if( stScrollInfo.nMax <=  gdVwTop ){	gdVwTop = stScrollInfo.nMax;	}
			break;

		case SB_PAGEDOWN:
			gdVwTop += 3;
			if( stScrollInfo.nMax <=  gdVwTop ){	gdVwTop = stScrollInfo.nMax;	}
			break;

		case SB_BOTTOM:
			gdVwTop = stScrollInfo.nMax;
			break;

		case SB_THUMBTRACK:
			gdVwTop = stScrollInfo.nTrackPos;
			break;

		default:	return;
	}

	if( maePos == gdVwTop  )	return;

	InvalidateRect( ghDraughtWnd, NULL, TRUE );
	UpdateWindow( ghDraughtWnd );

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = gdVwTop;
	SetScrollInfo( ghScrBarWnd, SB_CTL, &stScrollInfo, TRUE );

	return;
}

#ifdef USE_HOVERTIP

LPTSTR CALLBACK DraughtHoverTipInfo( LPVOID pVoid )
{
	INT		iTarget, iOffset, i;
	INT_PTR	iItems;
	LPSTR	pcConts = NULL;
	LPTSTR	ptBuffer = NULL;

	MAAM_ITR	itItem;

	if( !(gbAAtipView) ){	return NULL;	}
	if( 0 > giItemSel ){	return NULL;	}

	if( gbThumb )
	{
		iOffset = gdVwTop * TPNL_HORIZ;
		iTarget = iOffset + giItemSel;
		pcConts = AacAsciiArtGet( iTarget );

		ptBuffer = SjisDecodeAlloc( pcConts );
		FREE( pcConts );
	}
	else
	{
		iTarget = giItemSel;
		iItems  = gvcDrtItems.size( );
		if( iItems > iTarget )
		{
			for( i = 0, itItem = gvcDrtItems.begin(); gvcDrtItems.end() != itItem; i++, itItem++ )
			{
				if( iTarget == i )
				{
					ptBuffer = SjisDecodeAlloc( itItem->pcItem );
					break;
				}
			}
		}
	}

	return ptBuffer;
}

#endif

#ifndef _ORRVW

UINT DraughtItemAddFromSelect( HWND hWnd, UINT bSqSel )
{
	LPTSTR	ptString = NULL;
	UINT	cchSize, cbSize;
	LPPOINT	pstPos = NULL;

	UINT_PTR	i, j, iTexts;
	LONG	dMin = 0;
	INT		insDot, yLine, iLines = 0, dOffset;
	LPTSTR	ptSpace = NULL;

	LPSTR	pcArts;
	wstring	wsString;

	cbSize = DocSelectTextGetAlloc( D_UNI | bSqSel, (LPVOID *)(&ptString), (bSqSel & D_SQUARE) ? &pstPos : NULL );

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	if( 0 >= cchSize )	return 0;

	if( pstPos )
	{
		dMin = pstPos[0].x;

		yLine = 0;
		for( i = 0; cchSize > i; i++ )
		{
			if( CC_CR == ptString[i] && CC_LF == ptString[i+1] )
			{

				if( dMin > pstPos[yLine].x ){	dMin =  pstPos[yLine].x;	}

				i++;
				yLine++;
			}
		}

		iLines = yLine;

		insDot = 0;
		dOffset = pstPos[0].x - dMin;
		ptSpace = DocPaddingSpaceUni( dOffset, NULL, NULL, NULL );

		StringCchLength( ptSpace, STRSAFE_MAX_CCH, &iTexts );
		for( j = 0; iTexts > j; j++ ){	wsString +=  ptSpace[j];	}
		FREE(ptSpace);
	}

	yLine = 0;	insDot = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptString[i] && CC_LF == ptString[i+1] )
		{
			wsString +=  wstring( TEXT("\r\n") );

			i++;
			yLine++;

			if( pstPos && (iLines > yLine) )
			{
				dOffset = pstPos[yLine].x - dMin;
				ptSpace = DocPaddingSpaceUni( dOffset, NULL, NULL, NULL );

				StringCchLength( ptSpace, STRSAFE_MAX_CCH, &iTexts );
				for( j = 0; iTexts > j; j++ ){	wsString +=  ptSpace[j];	}
				FREE(ptSpace);
			}
		}
		else if( CC_TAB ==  ptString[i] ){		}
		else{	wsString += ptString[i];	}
	}

	FREE(ptString);
	FREE(pstPos);

	pcArts =  SjisEncodeAlloc( wsString.c_str() );

	DraughtItemAdding( hWnd, pcArts );

	FREE(pcArts);

	return yLine;
}

#endif

UINT DraughtItemAdding( HWND hWnd, LPSTR pcArts )
{
	UINT_PTR	cbSize;
	AAMATRIX	stItem;

	INT_PTR	iItems;

	StringCchLengthA( pcArts, STRSAFE_MAX_CCH, &cbSize );

	stItem.cbItem = cbSize;
	stItem.pcItem = (LPSTR)malloc( (cbSize + 1) );
	ZeroMemory( stItem.pcItem, (cbSize + 1) );
	StringCchCopyA( stItem.pcItem, (cbSize + 1), pcArts );

	DraughtAaImageing( hWnd, &stItem );

	gvcDrtItems.push_back( stItem );

	do
	{
		iItems = gvcDrtItems.size( );
		if( (TPNL_HORIZ * TPNL_VERTI) < iItems ){	DraughtItemDelete(  0 );	}

	}while( (TPNL_HORIZ * TPNL_VERTI) < iItems );

	return iItems;
}

UINT DraughtAaImageing( HWND hWnd, LPAAMATRIX pstItem )
{
	UINT_PTR	cchSize;
	LPTSTR		ptTextaa;
	INT		iRslt, iYdot, iXdot, iLine;
	HDC		hdc, hMemDC;
	HBITMAP	hOldBmp;
	RECT	rect;

	ptTextaa = SjisDecodeAlloc( pstItem->pcItem );
	StringCchLength( ptTextaa, STRSAFE_MAX_CCH, &cchSize );

	iXdot = TextViewSizeGet( ptTextaa, &iLine );
	iYdot = iLine * LINE_HEIGHT;

	pstItem->iMaxDot = iXdot;
	pstItem->iLines  = iLine;

	if( giItemWidth >  iXdot )	iXdot = giItemWidth;
	if( giItemHeight > iYdot )	iYdot = giItemHeight;

	pstItem->stSize.cx = iXdot;
	pstItem->stSize.cy = iYdot;

	SetRect( &rect, 0, 0, iXdot, iYdot );

	hdc = GetDC( hWnd );

	hMemDC = CreateCompatibleDC( hdc );
	pstItem->hThumbBmp = CreateCompatibleBitmap( hdc, rect.right, rect.bottom );
	hOldBmp = SelectBitmap( hMemDC, pstItem->hThumbBmp );
	SelectFont( hMemDC, ghAaFont );

	ReleaseDC( ghDraughtWnd, hdc );

	iRslt = PatBlt( hMemDC, 0, 0, rect.right, rect.bottom, WHITENESS );

	iRslt = DrawText( hMemDC, ptTextaa, cchSize, &rect, DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK );

	SelectFont( hMemDC, GetStockFont(DEFAULT_GUI_FONT) );
	SelectBitmap( hMemDC, hOldBmp );
	DeleteDC( hMemDC );

	FREE( ptTextaa );

	return 0;
}

HRESULT DraughtItemUse( HWND hWnd, INT id )
{
	LPSTR		pcAaItem;
	INT_PTR		iItems, i, iOffset, iTarget;
	UINT_PTR	cbSize;
	UINT		dMode;
	MAAM_ITR	itItem;

	if( gbThumb )
	{
		iOffset = gdVwTop * TPNL_HORIZ;
		iTarget = iOffset + giTarget;

		pcAaItem = AacAsciiArtGet( iTarget );
		if( !(pcAaItem) )	return E_OUTOFMEMORY;

		switch( id )
		{
			case IDM_DRAUGHT_INSERTEDIT:	dMode = MAA_INSERT;		break;
			case IDM_DRAUGHT_INTERRUPTEDIT:	dMode = MAA_INTERRUPT;	break;
			case IDM_DRAUGHT_LAYERBOX:		dMode = MAA_LAYERED;	break;
			default:
			case IDM_DRAUGHT_UNICLIP:		dMode = MAA_UNICLIP;	break;
			case IDM_DRAUGHT_SJISCLIP:		dMode = MAA_SJISCLIP;	break;

			case IDM_THUMB_DRAUGHT_ADD:		dMode = MAA_DRAUGHT;	break;
		}
		StringCchLengthA( pcAaItem, STRSAFE_MAX_CCH, &cbSize );

		ViewMaaMaterialise( hWnd, pcAaItem, cbSize, dMode );

		if( id != IDM_THUMB_DRAUGHT_ADD )
		{

			if( SUCCEEDED( AaItemsFavUpload( pcAaItem, cbSize ) ) )
			{
				FavContsRedrawRequest( hWnd );
			}
		}

		FREE(pcAaItem);
	}
	else
	{
		iItems = gvcDrtItems.size( );
		if( 0 >= iItems )	return E_OUTOFMEMORY;

		for( i = 0, itItem = gvcDrtItems.begin(); gvcDrtItems.end() != itItem; i++, itItem++ )
		{
			if( giTarget == i )
			{
				switch( id )
				{
					case IDM_DRAUGHT_INSERTEDIT:	dMode = MAA_INSERT;		break;
					case IDM_DRAUGHT_INTERRUPTEDIT:	dMode = MAA_INTERRUPT;	break;
					case IDM_DRAUGHT_LAYERBOX:		dMode = MAA_LAYERED;	break;
					default:
					case IDM_DRAUGHT_UNICLIP:		dMode = MAA_UNICLIP;	break;
					case IDM_DRAUGHT_SJISCLIP:		dMode = MAA_SJISCLIP;	break;
				}
				StringCchLengthA( itItem->pcItem, STRSAFE_MAX_CCH, &cbSize );

				ViewMaaMaterialise( hWnd, itItem->pcItem, cbSize, dMode );
			}
		}
	}

	return E_INVALIDARG;
}

INT DraughtItemDelete( CONST INT iTarget )
{
	INT_PTR	iItems, i;
	INT		delCnt;
	MAAM_ITR	itItem;

	iItems = gvcDrtItems.size( );
	if( 0 >= iItems )	return 0;

	delCnt = 0;
	for( i = 0, itItem = gvcDrtItems.begin(); gvcDrtItems.end() != itItem; i++, itItem++ )
	{
		if( iTarget == i || 0 > iTarget )
		{
			FREE( itItem->pcItem );
			DeleteBitmap( itItem->hThumbBmp );
			delCnt++;

			if( iTarget == i )
			{
				gvcDrtItems.erase( itItem );
				return 1;
			}
		}
	}

	if( 0 > iTarget ){	gvcDrtItems.clear( );	}

	return delCnt;
}

INT DraughtTargetItemSet( LPPOINT pstPos )
{
	INT	ix, iy, number;

	ix = pstPos->x / giItemWidth;
	iy = pstPos->y / giItemHeight;

	if( 0 > ix || TPNL_HORIZ <= ix || 0 > iy || TPNL_VERTI <= iy )	return -1;

	number = ix + iy * TPNL_HORIZ;

	return number;
}

DOUBLE DraughtAspectKeeping( LPSIZE pstSize, UINT bOrgRem )
{
	INT	iOrgWid, iOrgHei;
	INT	iZomWid, iZomHei;
	DOUBLE	ddPercent, ddBuff;

	iOrgWid = pstSize->cx;
	iOrgHei = pstSize->cy;
	ddPercent = 1.0;

	if( iOrgWid >= iOrgHei )
	{
		iZomWid = giItemWidth;

		if( giItemWidth == iOrgWid )
		{
			iZomHei = iOrgHei;
		}
		else
		{
			ddPercent = (DOUBLE)giItemWidth / (DOUBLE)iOrgWid;
			if( bOrgRem && giItemWidth > iOrgWid )
			{
				iZomWid = iOrgWid;
				iZomHei = iOrgHei;
			}
			else
			{
				ddBuff  = (DOUBLE)iOrgHei * ddPercent;
				ddBuff += 0.5;
				iZomHei = ddBuff;
			}
		}
	}
	else
	{
		iZomHei = giItemHeight;

		if( giItemHeight == iOrgHei )
		{
			iZomWid = iOrgWid;
		}
		else
		{
			ddPercent = (DOUBLE)giItemHeight / (DOUBLE)iOrgHei;
			if( bOrgRem && giItemHeight >  iOrgHei )
			{
				iZomWid = iOrgWid;
				iZomHei = iOrgHei;
			}
			else
			{
				ddBuff  = (DOUBLE)iOrgWid * ddPercent;
				ddBuff += 0.5;
				iZomWid = ddBuff;
			}
		}
	}

	pstSize->cx = iZomWid;
	pstSize->cy = iZomHei;

	return ddPercent;
}

HRESULT DraughtItemExport( HWND hWnd, LPTSTR ptPath )
{
	CONST CHAR	cacSplit[] = ("[SPLIT]\r\n");

	UINT_PTR	dItems, cbSize;
	TCHAR		atPath[MAX_PATH], atName[MAX_PATH];
	BOOLEAN		bOpened;
	OPENFILENAME	stOpenFile;

	MAAM_ITR	itItem;

	HANDLE	hFile;
	DWORD	wrote;

	dItems = gvcDrtItems.size();
	if( 0 >= dItems )	return E_NOTIMPL;

	ZeroMemory( atPath, sizeof(atPath) );
	ZeroMemory( atName, sizeof(atName) );

	ZeroMemory( &stOpenFile, sizeof(OPENFILENAME) );
	stOpenFile.lStructSize     = sizeof(OPENFILENAME);
	stOpenFile.hwndOwner       = ghPtWnd;
	stOpenFile.lpstrFilter     = TEXT("멀티라인 템플릿 파일(*.mlt)\0*.mlt\0모든 형식(*.*)\0*.*\0\0");
	stOpenFile.nFilterIndex    = 1;
	stOpenFile.lpstrFile       = atPath;
	stOpenFile.nMaxFile        = MAX_PATH;
	stOpenFile.lpstrFileTitle  = atName;
	stOpenFile.nMaxFileTitle   = MAX_PATH;
	stOpenFile.lpstrTitle      = TEXT("저장할 파일 이름을 지정하세요");
	stOpenFile.Flags           = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	stOpenFile.lpstrDefExt     = TEXT("mlt");

	bOpened = GetSaveFileName( &stOpenFile );
	wrote = CommDlgExtendedError();

	TRACE( TEXT("파일 저장 대화상자 통과[%X]"), wrote );

#ifndef _ORRVW
	ViewFocusSet(  );
#endif
	if( !(bOpened) ){	return  E_ABORT;	}

	hFile = CreateFile( atPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return E_HANDLE;	}

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	for( itItem = gvcDrtItems.begin(); gvcDrtItems.end() != itItem; itItem++ )
	{
		StringCchLengthA( itItem->pcItem, STRSAFE_MAX_LENGTH, &cbSize );

		WriteFile( hFile, itItem->pcItem, cbSize, &wrote, NULL );
		WriteFile( hFile, cacSplit, 9, &wrote, NULL );
	}

	CloseHandle( hFile );

	MessageBox( hWnd, TEXT("파일에 저장했습니다"), TEXT("오린의 알림"), MB_OK | MB_ICONINFORMATION );

	return S_OK;
}
