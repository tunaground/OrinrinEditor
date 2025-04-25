#include "stdafx.h"
#include "OrinrinEditor.h"

#define VERTSCRIPT_CLASS	TEXT("VERTSCRIPT_CLASS")
#define VT_WIDTH	320
#define VT_HEIGHT	240

#define VT_PARAMHEI	25

#define LEFT_PADD	15

#define IDEO_COMMA	TEXT('、')
#define IDEO_FSTOP	TEXT('。')

#define VERTVIEW_CLASS	TEXT("VERTVIEW_CLASS")

#define TB_ITEMS	5
static  TBBUTTON	gstVttbInfo[] = {
	{  0,	IDM_VLINE_DECIDE,		TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{  1,	IDM_VLINE_REFRESH,		TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  2,	IDCB_VLINE_LEFT_GO,		TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  3,	IDM_VLINE_TRANSPARENT,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  }
};

typedef struct tagVERTITEM
{
	TCHAR	cch;
	INT		iWidth;

	INT		iOffset;

	INT		iRow;
	INT		iColumn;

} VERTITEM, *LPVERTITEM;

extern FILES_ITR	gitFileIt;

extern INT			gixFocusPage;

extern HFONT		ghAaFont;

extern  BYTE		gbAlpha;

extern  HWND		ghViewWnd;
extern INT			gdHideXdot;
extern INT			gdViewTopLine;

static  HWND		ghVertToolBar;
static HIMAGELIST	ghVertImgLst;

static  ATOM		gVertAtom;
EXTERNED HWND		ghVertWnd;

static  HWND		ghTextWnd;
static INT			gdToolBarHei;

static  ATOM		gVertViewAtom;
static  HWND		ghVertViewWnd;

static POINT		gstViewOrigin;
static POINT		gstOffset;
static POINT		gstFrmSz;

static INT			gdVertInterval;
static  UINT		gbLeftGo;

static  UINT		gbSpTrans;

static LPTSTR		gptVtBuff;
static DWORD		gcchVtBuf;

static BOOLEAN		gbQuickClose;

static WNDPROC		gpfOrigVertEditProc;

static  vector<VERTITEM>	gvcVertItem;
typedef vector<VERTITEM>::iterator	VTIM_ITR;
typedef vector<VERTITEM>::reverse_iterator	VTIM_RITR;

static LRESULT	CALLBACK gpfVertEditProc( HWND , UINT, WPARAM, LPARAM );

LRESULT	CALLBACK VertProc( HWND, UINT, WPARAM, LPARAM );
VOID	Vrt_OnCommand( HWND , INT, HWND, UINT );
VOID	Vrt_OnPaint( HWND );
VOID	Vrt_OnDestroy( HWND );
LRESULT	Vrt_OnNotify( HWND , INT, LPNMHDR );

LRESULT	CALLBACK VertViewProc( HWND, UINT, WPARAM, LPARAM );
VOID	Vvw_OnKey( HWND, UINT, BOOL, INT, UINT );
VOID	Vvw_OnPaint( HWND );
VOID	Vvw_OnMoving( HWND, LPRECT );
BOOL	Vvw_OnWindowPosChanging( HWND, LPWINDOWPOS );
VOID	Vvw_OnWindowPosChanged( HWND, const LPWINDOWPOS );

HRESULT	VertTextAssemble( HWND );
VOID	VertViewDraw( HDC );
HRESULT	VertScriptInsert( HWND );

INT VertInitialise( LPTSTR ptCurrent, HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;
	HBITMAP		hImg, hMsq;

	if( !(ptCurrent) || !(hInstance) )
	{
		if( ghVertWnd ){	DestroyWindow( ghVertWnd  );	}

		FREE( gptVtBuff );

		return S_OK;
	}

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= VertProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= VERTSCRIPT_CLASS;
	wcex.hIconSm		= NULL;

	gVertAtom = RegisterClassEx( &wcex );

	ghVertWnd = NULL;

	ZeroMemory( &gstViewOrigin, sizeof(POINT) );

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= VertViewProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= VERTVIEW_CLASS;
	wcex.hIconSm		= NULL;

	gVertViewAtom = RegisterClassEx( &wcex );

	gptVtBuff = (LPTSTR)malloc( MAX_PATH * sizeof(TCHAR) );
	ZeroMemory( gptVtBuff, MAX_PATH * sizeof(TCHAR) );
	gcchVtBuf = MAX_PATH;

	ghVertImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 4, 1 );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_MOZI_WRITE ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_PAGENAMECHANGE ) );
	ImageList_Add( ghVertImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_REFRESH ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_REFRESH ) );
	ImageList_Add( ghVertImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_VERT_LEFT ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_VERT_LEFT ) );
	ImageList_Add( ghVertImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_VERT_TRANS ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_VERT_TRANS ) );
	ImageList_Add( ghVertImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	return 1;
}

HWND VertScripterCreate( HINSTANCE hInst, HWND hPrWnd )
{
	LONG	x, y;
	HWND	hDktpWnd;
	UINT	height;
	TCHAR	atBuffer[MAX_STRING];
	RECT	rect, vwRect, dtRect;

	hDktpWnd = GetDesktopWindow(  );
	GetWindowRect( hDktpWnd, &dtRect );

	GetWindowRect( hPrWnd, &rect );
	x = dtRect.right - rect.right;
	if( VT_WIDTH >  x ){	rect.right = dtRect.right - VT_WIDTH;	}

	if( ghVertWnd )
	{
		SetForegroundWindow( ghVertViewWnd );
		SetWindowPos( ghVertWnd, HWND_TOP, rect.right, rect.top, 0, 0, SWP_NOSIZE );
		SetForegroundWindow( ghVertWnd );

		return ghVertWnd;
	}

	gbLeftGo = 0;

	gbSpTrans = 0;

	gbQuickClose = 1;

	ghVertWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_TOPMOST, VERTSCRIPT_CLASS,
		TEXT("세로쓰기"), WS_POPUP | WS_CAPTION | WS_SYSMENU,
		rect.right, rect.top, VT_WIDTH, VT_HEIGHT, NULL, NULL, hInst, NULL );

	ghVertToolBar = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("verttoolbar"), WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, ghVertWnd, (HMENU)IDTB_VLINE_TOOLBAR, hInst, NULL );

	if( 0 == gdToolBarHei )
	{
		GetWindowRect( ghVertToolBar, &rect );
		gdToolBarHei = rect.bottom - rect.top;
	}

	SendMessage( ghVertToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	SendMessage( ghVertToolBar, TB_SETIMAGELIST, 0, (LPARAM)ghVertImgLst );
	SendMessage( ghVertToolBar, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghVertToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuffer, MAX_STRING, TEXT("문자열 삽입") );	gstVttbInfo[0].iString = SendMessage( ghVertToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("문자열 갱신") );	gstVttbInfo[2].iString = SendMessage( ghVertToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("왼쪽에서 배치") );	gstVttbInfo[3].iString = SendMessage( ghVertToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("공백을 투명하게") );	gstVttbInfo[4].iString = SendMessage( ghVertToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghVertToolBar , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstVttbInfo );

	SendMessage( ghVertToolBar , TB_AUTOSIZE, 0, 0 );
	InvalidateRect( ghVertToolBar , NULL, TRUE );

	GetClientRect( ghVertWnd, &rect );

	CreateWindowEx( 0, WC_STATIC, TEXT("행 간격"), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_RIGHT, 2, gdToolBarHei, 45, VT_PARAMHEI, ghVertWnd, (HMENU)IDS_VLINE_INTERVAL, hInst, NULL );

	gdVertInterval = 22;
	CreateWindowEx( 0, WC_EDIT, TEXT("22"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 49, gdToolBarHei, 50, VT_PARAMHEI, ghVertWnd, (HMENU)IDE_VLINE_INTERVAL, hInst, NULL );

	CreateWindowEx( 0, UPDOWN_CLASS, TEXT("intervalspin"), WS_CHILD | WS_VISIBLE | UDS_AUTOBUDDY, 99, gdToolBarHei, 10, VT_PARAMHEI, ghVertWnd, (HMENU)IDUD_VLINE_INTERVAL, hInst, NULL );

	CreateWindowEx( 0, WC_BUTTON, TEXT("확정 후 닫기"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 120, gdToolBarHei, 180, VT_PARAMHEI, ghVertWnd, (HMENU)IDCB_VLINE_QUICKCLOSE, hInst, NULL );
	CheckDlgButton( ghVertWnd, IDCB_VLINE_QUICKCLOSE, gbQuickClose ? BST_CHECKED : BST_UNCHECKED );

	height = gdToolBarHei + VT_PARAMHEI;

	ghTextWnd = CreateWindowEx( 0, WC_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE,
		0, height, rect.right, rect.bottom - height, ghVertWnd, (HMENU)IDE_VLINE_TEXT, hInst, NULL );
	SetWindowFont( ghTextWnd, ghAaFont, TRUE );

	gpfOrigVertEditProc = SubclassWindow( ghTextWnd, gpfVertEditProc );

	ShowWindow( ghVertWnd, SW_SHOW );
	UpdateWindow( ghVertWnd );

	ghVertViewWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_LAYERED, VERTVIEW_CLASS,
		TEXT("배치"), WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE, 0, 0, 160, 120, NULL, NULL, hInst, NULL);
	SetLayeredWindowAttributes( ghVertViewWnd, 0, gbAlpha, LWA_ALPHA );

	ZeroMemory( &gstFrmSz, sizeof(POINT) );
	ClientToScreen( ghVertViewWnd, &gstFrmSz );

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;
	gstViewOrigin.y = vwRect.top;
	x = (vwRect.left + LINENUM_WID) - gstFrmSz.x;
	y = (vwRect.top  + RULER_AREA)  - gstFrmSz.y;
	TRACE( TEXT("VERT %d x %d"), x, y );

#ifdef _DEBUG
	SetWindowPos( ghVertViewWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#else
	SetWindowPos( ghVertViewWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#endif
	gstOffset.x = x - vwRect.left;
	gstOffset.y = y - vwRect.top;

	return ghVertWnd;
}

LRESULT CALLBACK gpfVertEditProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT		len;
	INT		id;
	HWND	hWndCtl;
	UINT	codeNotify;

	switch( msg )
	{
		default:	break;

		case WM_COMMAND:
			id         = LOWORD(wParam);
			hWndCtl    = (HWND)lParam;
			codeNotify = HIWORD(wParam);
			TRACE( TEXT("[%X]VertEdit COMMAND %d"), hWnd, id );

			switch( id )
			{
				case IDM_PASTE:	SendMessage( hWnd, WM_PASTE, 0, 0 );	return 0;
				case IDM_COPY:	SendMessage( hWnd, WM_COPY,  0, 0 );	return 0;
				case IDM_CUT:	SendMessage( hWnd, WM_CUT,   0, 0 );	return 0;
				case IDM_UNDO:	SendMessage( hWnd, WM_UNDO,  0, 0 );	return 0;
				case IDM_ALLSEL:
					len = GetWindowTextLength( hWnd );
					SendMessage( hWnd, EM_SETSEL, 0, len );
					break;
				default:	break;
			}

			break;
	}

	return CallWindowProc( gpfOrigVertEditProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK VertProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT,   Vrt_OnPaint );
		HANDLE_MSG( hWnd, WM_NOTIFY,  Vrt_OnNotify );
		HANDLE_MSG( hWnd, WM_COMMAND, Vrt_OnCommand );
		HANDLE_MSG( hWnd, WM_DESTROY, Vrt_OnDestroy );

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Vrt_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	LRESULT	lRslt;

	switch( id )
	{
		case  IDM_VLINE_DECIDE:
			VertScriptInsert( hWnd );
			if( gbQuickClose  ){	DestroyWindow( hWnd );	}
			break;

		case  IDM_VLINE_REFRESH:	VertTextAssemble( hWnd );	break;

		case  IDE_VLINE_TEXT:
			if( EN_UPDATE == codeNotify ){	VertTextAssemble( hWnd );	}
			break;

		case  IDCB_VLINE_LEFT_GO:
			lRslt = SendMessage( ghVertToolBar, TB_ISBUTTONCHECKED, IDCB_VLINE_LEFT_GO, 0 );
			if( lRslt ){	gbLeftGo = 1;	}
			else{	gbLeftGo = 0;	}
			VertTextAssemble( hWnd );
			break;

		case  IDM_VLINE_TRANSPARENT:
			lRslt = SendMessage( ghVertToolBar, TB_ISBUTTONCHECKED, IDM_VLINE_TRANSPARENT, 0 );
			if( lRslt ){	gbSpTrans = 1;	}
			else{	gbSpTrans = 0;	}
			break;

		case  IDCB_VLINE_QUICKCLOSE:
			gbQuickClose = IsDlgButtonChecked( hWnd, IDCB_VLINE_QUICKCLOSE ) ? TRUE : FALSE;
			break;

		case  IDM_PASTE:	TRACE( TEXT("VT PASTE") );	SendMessage( ghTextWnd, WM_PASTE, 0, 0 );	return;
		case  IDM_COPY:		SendMessage( ghTextWnd, WM_COPY,  0, 0 );	return;
		case  IDM_CUT:		SendMessage( ghTextWnd, WM_CUT,   0, 0 );	return;

		default:	return;
	}

	return;
}

VOID Vrt_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	EndPaint( hWnd, &ps );

	return;
}

VOID Vrt_OnDestroy( HWND hWnd )
{
	MainStatusBarSetText( SB_LAYER, TEXT("") );

	if( ghVertViewWnd ){	DestroyWindow( ghVertViewWnd  );	}

	ghVertWnd = NULL;

	return;
}

LRESULT Vrt_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	LPNMUPDOWN	pstNmUpDown;

	if( IDUD_VLINE_INTERVAL == idFrom )
	{
		pstNmUpDown = (LPNMUPDOWN)pstNmhdr;

		if( UDN_DELTAPOS == pstNmUpDown->hdr.code )
		{

			TRACE( TEXT("UPDOWN %d"), pstNmUpDown->iDelta );
			gdVertInterval -= (pstNmUpDown->iDelta);
			SetDlgItemInt( hWnd, IDE_VLINE_INTERVAL, gdVertInterval, TRUE );
			VertTextAssemble( hWnd );
			InvalidateRect( ghVertViewWnd, NULL, TRUE );
		}
	}

	return 0;
}

HRESULT VertTextAssemble( HWND hWnd )
{
	UINT_PTR	cchSz;
	INT			ileng, i, iRow, iClm, iLine, iMaxRow;
	INT			iMaxY, iMaxX, iViewXdot, iViewYdot, iYline, cx, cy;
	INT			iLnDot, iLnCnt, bkLine;
	UINT_PTR	iMozis;
	HWND		hWorkWnd;
	LPTSTR		ptScript;
	RECT		rect;
	VERTITEM	stVtitm;

	VTIM_ITR	itVtitm;

	hWorkWnd = GetDlgItem( hWnd, IDE_VLINE_TEXT );

	ileng = Edit_GetTextLength( hWorkWnd );
	cchSz = ileng + 2;

	if( gcchVtBuf <  cchSz )
	{
		ptScript = (LPTSTR)realloc( gptVtBuff, cchSz * sizeof(TCHAR) );
		gptVtBuff = ptScript;
		gcchVtBuf = cchSz;
	}

	ZeroMemory( gptVtBuff, gcchVtBuf * sizeof(TCHAR) );
	Edit_GetText( hWorkWnd, gptVtBuff, cchSz );

	gvcVertItem.clear( );

	if( 0 >= ileng )	return S_FALSE;

	iRow  = 0;
	iClm  = 0;
	iLine = 1;
	iMaxRow = 0;
	for( i = 0; ileng > i; i++ )
	{
		ZeroMemory( &stVtitm, sizeof(VERTITEM) );

		if( TEXT('\r') ==  gptVtBuff[i] )
		{

			if( iMaxRow < iRow )	iMaxRow = iRow;

			i++;
			iLine++;
			iClm++;
			iRow = 0;
		}
		else
		{
			stVtitm.cch      = gptVtBuff[i];
			stVtitm.iWidth   = ViewLetterWidthGet( stVtitm.cch );
			stVtitm.iOffset  = LEFT_PADD;
			stVtitm.iRow     = iRow++;
			stVtitm.iColumn  = iClm;

			gvcVertItem.push_back( stVtitm );
		}
	}

	if( iMaxRow < iRow )	iMaxRow = iRow;

	iMaxY = (iMaxRow+1) * LINE_HEIGHT;
	iMaxX  = LEFT_PADD + (gdVertInterval * iLine );

	if( gbLeftGo ){	iLnCnt = 0;	}
	else{	iLnCnt = iClm;	}

	iLnDot = LEFT_PADD + (gdVertInterval * iLnCnt );
	bkLine = 0;
	iMozis = gvcVertItem.size( );

	for( itVtitm = gvcVertItem.begin(); itVtitm != gvcVertItem.end(); itVtitm++ )
	{
		if( bkLine != itVtitm->iColumn )
		{
			if( gbLeftGo ){	iLnCnt++;	}
			else{	iLnCnt--;	}
			if( 0 > iLnCnt ){	iLnCnt =  0;	}

			iLnDot = LEFT_PADD + (gdVertInterval * iLnCnt );
		}
		bkLine = itVtitm->iColumn;

		itVtitm->iOffset = iLnDot - ((itVtitm->iWidth+1) / 2);

		if( IDEO_COMMA == itVtitm->cch || IDEO_FSTOP == itVtitm->cch )
		{
			itVtitm->iOffset = iLnDot - 3;
		}

		if( 0 >  itVtitm->iOffset ){	itVtitm->iOffset = 0;	}
	}

	iYline = ViewAreaSizeGet( &iViewXdot );
	iViewYdot = iYline * LINE_HEIGHT;

	GetWindowRect( ghVertViewWnd, &rect );
	cx = rect.right  - rect.left;
	cy = rect.bottom - rect.top;
	GetClientRect( ghVertViewWnd, &rect );
	cx -= rect.right;
	cy -= rect.bottom;

	cx += iMaxX;
	cy += iMaxY;

	if( iViewXdot < cx ){	cx =  iViewXdot;	}
	if( iViewYdot < cy ){	cy =  iViewYdot;	}

	if( 66 > cx ){	cx = 66;	}
	if( 66 > cy ){	cy = 66;	}

#ifdef _DEBUG
	SetWindowPos( ghVertViewWnd, HWND_TOP, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
#else
	SetWindowPos( ghVertViewWnd, HWND_TOPMOST, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
#endif

	InvalidateRect( ghVertViewWnd, NULL, TRUE );

	return S_OK;
}

LRESULT CALLBACK VertViewProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_KEYDOWN,			Vvw_OnKey );
		HANDLE_MSG( hWnd, WM_PAINT,				Vvw_OnPaint );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING,	Vvw_OnWindowPosChanging );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGED,	Vvw_OnWindowPosChanged );

		case  WM_MOVING:	Vvw_OnMoving( hWnd, (LPRECT)lParam );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Vvw_OnMoving( HWND hWnd, LPRECT pstPos )
{
	LONG	xEt, yEt, xLy, yLy, xSb, ySb;
	LONG	dLine, dRema;
	BOOLEAN	bMinus = FALSE;
	TCHAR	atBuffer[SUB_STRING];

	xLy = pstPos->left + gstFrmSz.x;
	yLy = pstPos->top  + gstFrmSz.y;

	xEt = (gstViewOrigin.x + LINENUM_WID);
	yEt = (gstViewOrigin.y + RULER_AREA);

	xSb = xLy - xEt;
	ySb = yLy - yEt;

	if( 0 > ySb ){	ySb *= -1;	bMinus = TRUE;	}

	dLine = ySb / LINE_HEIGHT;
	dRema = ySb % LINE_HEIGHT;
	if( (LINE_HEIGHT/2) < dRema ){	dLine++;	}
	if( bMinus ){	dLine *= -1;	}else{	dLine++;	}

	xSb   += gdHideXdot;
	dLine += gdViewTopLine;

	StringCchPrintf( atBuffer, SUB_STRING, TEXT("VLINE %d[dot] %d[line]"), xSb, dLine );
	MainStatusBarSetText( SB_LAYER, atBuffer );

	return;
}

BOOL Vvw_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
{
	INT		clPosY, vwTopY, dSabun, dRem;
	BOOLEAN	bMinus = FALSE;
	RECT	vwRect;

	if( SWP_NOMOVE & pstWpos->flags )	return TRUE;

	clPosY = pstWpos->y + gstFrmSz.y;

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;
	gstViewOrigin.y = vwRect.top;
	vwTopY = (vwRect.top  + RULER_AREA);

	dSabun = vwTopY - clPosY;
	if( 0 > dSabun ){	dSabun *= -1;	bMinus = TRUE;	}

	dRem = dSabun % LINE_HEIGHT;

	if( 0 == dRem ){	return TRUE;	}

	if( (LINE_HEIGHT/2) < dRem ){	dRem = dRem - LINE_HEIGHT;	}

	if( bMinus ){	dRem *=  -1;	}

	pstWpos->y += dRem;

	return FALSE;
}

VOID Vvw_OnWindowPosChanged( HWND hWnd, const LPWINDOWPOS pstWpos )
{
	RECT	vwRect;

	InvalidateRect( hWnd, NULL, TRUE );

	if( SWP_NOMOVE & pstWpos->flags )	return;

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;
	gstViewOrigin.y = vwRect.top;

	gstOffset.x = pstWpos->x - vwRect.left;
	gstOffset.y = pstWpos->y - vwRect.top;

	return;
}

VOID Vvw_OnKey( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
{
	RECT	rect;

	GetWindowRect( hWnd, &rect );

	if( fDown )
	{
		switch( vk )
		{
			case VK_RIGHT:	TRACE( TEXT("오른쪽") );	rect.left++;	break;
			case VK_LEFT:	TRACE( TEXT("왼쪽") );	rect.left--;	break;
			case VK_DOWN:	TRACE( TEXT("아래") );	rect.top += LINE_HEIGHT;	break;
			case  VK_UP:	TRACE( TEXT("위") );	rect.top -= LINE_HEIGHT;	break;
			default:	return;
		}
	}

#ifdef _DEBUG
	SetWindowPos( hWnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
#else
	SetWindowPos( hWnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
#endif

	Vvw_OnMoving( hWnd, &rect );

	return;
}

VOID Vvw_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	VertViewDraw( hdc );

	EndPaint( hWnd, &ps );

	return;
}

VOID VertViewDraw( HDC hDC )
{
	INT_PTR	iItems;
	INT		x, y;
	TCHAR	atMozi[2];
	HFONT	hFtOld;

	VTIM_ITR	itVtitm;

	hFtOld = SelectFont( hDC, ghAaFont );
	SetBkMode( hDC, TRANSPARENT );

	iItems = gvcVertItem.size( );

	atMozi[1] = 0;

	for( itVtitm = gvcVertItem.begin(); itVtitm != gvcVertItem.end(); itVtitm++ )
	{
		atMozi[0] = itVtitm->cch;
		x = itVtitm->iOffset;
		y = itVtitm->iRow * LINE_HEIGHT;
		ExtTextOut( hDC, x, y, 0, NULL, atMozi, 1, NULL );
	}

	SelectFont( hDC, hFtOld );

	return;
}

HRESULT	VertScriptInsert( HWND hWnd )
{
	LPTSTR		ptText;
	INT			iTgtRow, iMaxRow, iMaxClm;
	INT			iRitDot, iNeedPadd;
	HWND		hLyrWnd;
	INT			iX, iY;
	RECT		rect;

	wstring		wsBuffer;

	VTIM_ITR	itVtitm;
	VTIM_RITR	itRvsVtitm;

	iMaxRow = 0;	iMaxClm = 0;
	for( itVtitm = gvcVertItem.begin(); itVtitm != gvcVertItem.end(); itVtitm++ )
	{
		if( iMaxRow < itVtitm->iRow )		iMaxRow = itVtitm->iRow;
		if( iMaxClm < itVtitm->iColumn )	iMaxClm = itVtitm->iColumn;
	}

	ptText = NULL;
	wsBuffer.clear();
	for( iTgtRow = 0; iMaxRow >= iTgtRow; iTgtRow++ )
	{
		iRitDot = 0;

		if( gbLeftGo )
		{
			for( itVtitm = gvcVertItem.begin(); itVtitm != gvcVertItem.end(); itVtitm++ )
			{
				if( iTgtRow == itVtitm->iRow )
				{
					iNeedPadd = itVtitm->iOffset - iRitDot;
					if( 0 > iNeedPadd ){	iNeedPadd = 0;	}

					ptText = DocPaddingSpaceMake( iNeedPadd );
					if( ptText )
					{
						wsBuffer += wstring( ptText );
						FREE( ptText );
					}
					wsBuffer += itVtitm->cch;

					iRitDot += iNeedPadd;
					iRitDot += itVtitm->iWidth;
				}
			}
		}
		else
		{

			for( itRvsVtitm = gvcVertItem.rbegin(); itRvsVtitm != gvcVertItem.rend(); itRvsVtitm++ )
			{
				if( iTgtRow == itRvsVtitm->iRow )
				{
					iNeedPadd = itRvsVtitm->iOffset - iRitDot;
					if( 0 > iNeedPadd ){	iNeedPadd = 0;	}

					ptText = DocPaddingSpaceMake( iNeedPadd );
					if( ptText )
					{
						wsBuffer += wstring( ptText );
						FREE( ptText );
					}
					wsBuffer += itRvsVtitm->cch;

					iRitDot += iNeedPadd;
					iRitDot += itRvsVtitm->iWidth;
				}
			}
		}

		wsBuffer += wstring( CH_CRLFW );
	}

	hLyrWnd = LayerBoxVisibalise( GetModuleHandle(NULL), wsBuffer.c_str(), 0x10 );

	GetWindowRect( ghVertViewWnd, &rect );
	LayerBoxPositionChange( hLyrWnd, (rect.left + gstFrmSz.x), (rect.top + gstFrmSz.y) );

	if( gbSpTrans ){	LayerTransparentToggle( hLyrWnd, 1 );	}

	LayerContentsImportable( hLyrWnd, IDM_LYB_OVERRIDE, &iX, &iY, D_INVISI );
	ViewPosResetCaret( iX, iY );

	DestroyWindow( hLyrWnd );

	return S_OK;
}

HRESULT VertMoveFromView( HWND hWnd, UINT state )
{
	RECT	vwRect = {0,0,0,0};
	POINT	lyPoint;

	if( !(ghVertViewWnd) )	return S_FALSE;

	if( SIZE_MINIMIZED != state )
	{
		GetWindowRect( ghViewWnd, &vwRect );
		gstViewOrigin.x = vwRect.left;
		gstViewOrigin.y = vwRect.top;
	}

	if( SIZE_MINIMIZED == state )
	{
		ShowWindow( ghVertViewWnd, SW_HIDE );
	}
	else
	{
		lyPoint.x = gstOffset.x + vwRect.left;
		lyPoint.y = gstOffset.y + vwRect.top;
#ifdef _DEBUG
		SetWindowPos( ghVertViewWnd, HWND_TOP, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#else
		SetWindowPos( ghVertViewWnd, HWND_TOPMOST, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#endif
	}

	return S_OK;
}
