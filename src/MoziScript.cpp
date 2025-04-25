#include "stdafx.h"
#include "OrinrinEditor.h"

#define MOZISCRIPT_CLASS	TEXT("MOZISCRIPT_CLASS")
#define MZ_WIDTH	320
#define MZ_HEIGHT	280

#define MZ_PARAMHEI	23

#define MOZIVIEW_CLASS	TEXT("MOZIVIEW_CLASS")

#define TB_ITEMS	3
static  TBBUTTON	gstMztbInfo[] = {
	{  0,	IDM_MOZI_DECIDE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  1,	IDM_MOZI_REFRESH,	TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  2,	IDM_MOZI_SETTING,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  }

};

typedef struct tagMOZIITEM
{
	TCHAR	cch;
	LPTSTR	ptAA;

	INT		iLeft;
	INT		iTop;
	INT		iWidth;
	INT		iLine;

} MOZIITEM, *LPMOZIITEM;

extern FILES_ITR	gitFileIt;

extern INT			gixFocusPage;

extern HFONT		ghAaFont;

extern  BYTE		gbAlpha;

extern  HWND		ghViewWnd;
extern INT			gdHideXdot;
extern INT			gdViewTopLine;

static TCHAR		gatMoziIni[MAX_PATH];

static  ATOM		gMoziAtom;
EXTERNED HWND		ghMoziWnd;
static  HWND		ghMoziToolBar;
static HIMAGELIST	ghMoziImgLst;

static  ATOM		gMoziViewAtom;
static  HWND		ghMoziViewWnd;

static  HWND		ghTextWnd;
static  HWND		ghIsolaLvWnd;
static  HWND		ghSettiLvWnd;

static POINT		gstViewOrigin;
static POINT		gstOffset;
static POINT		gstFrmSz;

static INT			gdToolBarHei;

static INT			gdMoziInterval;

static INT			gdNowMode;

static INT			gdAvrWidth;
static INT			gdMaxLine;

static LPTSTR		gptMzBuff;
static DWORD		gcchMzBuf;

static BOOLEAN		gbQuickClose;

static WNDPROC		gpfOrigMoziEditProc;

static sqlite3		*gpMoziTable;

static vector<MOZIITEM>	gvcMoziItem;

typedef vector<MOZIITEM>::iterator	MZTM_ITR;

static LRESULT	CALLBACK gpfMoziEditProc( HWND , UINT, WPARAM, LPARAM );

LRESULT	CALLBACK MoziProc( HWND, UINT, WPARAM, LPARAM );
VOID	Mzs_OnCommand( HWND , INT, HWND, UINT );
VOID	Mzs_OnPaint( HWND );
VOID	Mzs_OnDestroy( HWND );
VOID	Mzs_OnContextMenu( HWND, HWND, UINT, UINT );
VOID	Mzs_OnDropFiles( HWND , HDROP );
LRESULT	Mzs_OnNotify( HWND , INT, LPNMHDR );

HRESULT	MoziFileRefresh( HWND );
HRESULT	MoziFileRebuild( HWND, UINT );

HRESULT	MoziFileListAdd( LPTSTR );
HRESULT MoziFileListDelete( HWND  );

HRESULT	MoziFileStore( LPTSTR );
UINT	CALLBACK MoziItemTablise( LPTSTR, LPCTSTR, INT );
VOID	MoziItemRemovePeriod( LPTSTR  );

HRESULT	MoziSpaceCreate( VOID );

HRESULT	MoziScriptInsert( HWND );

LRESULT	CALLBACK MoziViewProc( HWND, UINT, WPARAM, LPARAM );
VOID	Mzv_OnKey( HWND, UINT, BOOL, INT, UINT );
VOID	Mzv_OnPaint( HWND );
VOID	Mzv_OnMoving( HWND, LPRECT );
BOOL	Mzv_OnWindowPosChanging( HWND, LPWINDOWPOS );
VOID	Mzv_OnWindowPosChanged( HWND, const LPWINDOWPOS );
VOID	MoziViewDraw( HDC );

HRESULT	MoziSqlTableOpenClose( UINT );
HRESULT	MoziSqlTransOnOff( BYTE );
UINT	MoziSqlItemInsert( LPTSTR, LPTSTR, INT, INT );
LPTSTR	MoziSqlItemSelect( TCHAR, LPINT, LPINT );
UINT	MoziSqlItemCount( LPINT, LPINT );
HRESULT	MoziSqlItemDeleteAll( VOID );

INT MoziInitialise( LPTSTR ptCurrent, HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;
	HBITMAP		hImg, hMsq;
	MZTM_ITR	itMzitm;

	if( !(ptCurrent) || !(hInstance) )
	{
		if( ghMoziWnd ){	DestroyWindow( ghMoziWnd  );	}

		if( ghMoziImgLst  ){	ImageList_Destroy( ghMoziImgLst );	}

		if( !( gvcMoziItem.empty( ) ) )
		{
			for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ )
			{
				FREE( itMzitm->ptAA );
			}
			gvcMoziItem.clear( );
		}

		MoziSqlTableOpenClose( M_DESTROY );

		FREE( gptMzBuff );

		return S_OK;
	}

	StringCchCopy( gatMoziIni, MAX_PATH, ptCurrent );
	PathAppend( gatMoziIni, MZCX_INI_FILE );

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MoziProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= MOZISCRIPT_CLASS;
	wcex.hIconSm		= NULL;

	gMoziAtom = RegisterClassEx( &wcex );

	ghMoziWnd = NULL;

	ZeroMemory( &gstViewOrigin, sizeof(POINT) );
	gdToolBarHei = 0;

	ghMoziImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 4, 1 );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_MOZI_WRITE ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_PAGENAMECHANGE ) );
	ImageList_Add( ghMoziImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_REFRESH ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_REFRESH ) );
	ImageList_Add( ghMoziImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_SETTING ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_SETTING ) );
	ImageList_Add( ghMoziImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	MoziSqlTableOpenClose( M_CREATE );

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MoziViewProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= MOZIVIEW_CLASS;
	wcex.hIconSm		= NULL;

	gMoziViewAtom = RegisterClassEx( &wcex );

	gptMzBuff = (LPTSTR)malloc( MAX_PATH * sizeof(TCHAR) );
	ZeroMemory( gptMzBuff, MAX_PATH * sizeof(TCHAR) );
	gcchMzBuf = MAX_PATH;

	return 1;
}

HWND MoziScripterCreate( HINSTANCE hInst, HWND hPrWnd )
{
	LONG	x, y;
	HWND	hDktpWnd;
	UINT	dCount, height;
	TCHAR	atBuffer[MAX_STRING];
	RECT	rect, vwRect, dtRect;

	LVCOLUMN	stLvColm;

	hDktpWnd = GetDesktopWindow(  );
	GetWindowRect( hDktpWnd, &dtRect );

	GetWindowRect( hPrWnd, &rect );
	x = dtRect.right - rect.right;
	if( MZ_WIDTH >  x ){	rect.right = dtRect.right - MZ_WIDTH;	}

	if( ghMoziWnd )
	{
		SetForegroundWindow( ghMoziViewWnd );
		SetWindowPos( ghMoziWnd, HWND_TOP, rect.right, rect.top, 0, 0, SWP_NOSIZE );
		SetForegroundWindow( ghMoziWnd );

		return ghMoziWnd;
	}

	gdMoziInterval = 0;

	ghMoziWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_TOPMOST, MOZISCRIPT_CLASS,
		TEXT("문자 AA 변환"), WS_POPUP | WS_CAPTION | WS_SYSMENU,
		rect.right, rect.top, MZ_WIDTH, MZ_HEIGHT, NULL, NULL, hInst, NULL );

	gdNowMode = 0;

	gbQuickClose = TRUE;

	ghMoziToolBar = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("mozitoolbar"), WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, ghMoziWnd, (HMENU)IDTB_MZSCR_TOOLBAR, hInst, NULL );

	if( 0 == gdToolBarHei )
	{
		GetWindowRect( ghMoziToolBar, &rect );
		gdToolBarHei = rect.bottom - rect.top;
	}

	SendMessage( ghMoziToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	SendMessage( ghMoziToolBar, TB_SETIMAGELIST, 0, (LPARAM)ghMoziImgLst );
	SendMessage( ghMoziToolBar, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghMoziToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuffer, MAX_STRING, TEXT("문자 AA 삽입") );					gstMztbInfo[0].iString = SendMessage( ghMoziToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("문자열 갱신 / 사용 파일 갱신") );	gstMztbInfo[1].iString = SendMessage( ghMoziToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("사용 파일 설정") );				gstMztbInfo[2].iString = SendMessage( ghMoziToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghMoziToolBar , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstMztbInfo );

	SendMessage( ghMoziToolBar , TB_AUTOSIZE, 0, 0 );

	CreateWindowEx( 0, WC_BUTTON, TEXT("삽입 후 닫기"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 128, 2, 170, 23, ghMoziToolBar, (HMENU)IDCB_MZSCR_QUICKCLOSE, hInst, NULL );
	CheckDlgButton( ghMoziToolBar, IDCB_MZSCR_QUICKCLOSE, gbQuickClose ? BST_CHECKED : BST_UNCHECKED );

	InvalidateRect( ghMoziToolBar , NULL, TRUE );

	GetClientRect( ghMoziWnd, &rect );

	CreateWindowEx( 0, WC_STATIC, TEXT("문자 간격"), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, gdToolBarHei, 52, MZ_PARAMHEI, ghMoziWnd, (HMENU)IDS_MZSCR_INTERVAL, hInst, NULL );

	CreateWindowEx( 0, WC_EDIT, TEXT("0"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 54, gdToolBarHei, 50, MZ_PARAMHEI, ghMoziWnd, (HMENU)IDE_MZSCR_INTERVAL, hInst, NULL );

	CreateWindowEx( 0, UPDOWN_CLASS, TEXT("intervalspin"), WS_CHILD | WS_VISIBLE | UDS_AUTOBUDDY, 104, gdToolBarHei, 10, MZ_PARAMHEI, ghMoziWnd, (HMENU)IDUD_MZSCR_INTERVAL, hInst, NULL );

	CreateWindowEx( 0, WC_BUTTON, TEXT("문자 간격은 투명"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 130, gdToolBarHei, 120, MZ_PARAMHEI, ghMoziWnd, (HMENU)IDCB_MZSCR_TRANSPARENT, hInst, NULL );

	height = gdToolBarHei + MZ_PARAMHEI;

	ghTextWnd = CreateWindowEx( 0, WC_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE,
		0, height, rect.right, rect.bottom - height, ghMoziWnd, (HMENU)IDE_MZSCR_TEXT, hInst, NULL );
	SetWindowFont( ghTextWnd, ghAaFont, TRUE );

	gpfOrigMoziEditProc = SubclassWindow( ghTextWnd, gpfMoziEditProc );

	ghSettiLvWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("mozisetting"),
		WS_CHILD | WS_BORDER | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER,
		0, gdToolBarHei, rect.right, rect.bottom - gdToolBarHei, ghMoziWnd, (HMENU)IDLV_MZSCR_SETTING, hInst, NULL );
	ListView_SetExtendedListViewStyle( ghSettiLvWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt = LVCFMT_LEFT;
	stLvColm.pszText = TEXT("파일명");	stLvColm.cx = 200;	stLvColm.iSubItem = 0x00;	ListView_InsertColumn( ghSettiLvWnd, 0, &stLvColm );
	stLvColm.pszText = TEXT("전체 경로");	stLvColm.cx = 500;	stLvColm.iSubItem = 0x01;	ListView_InsertColumn( ghSettiLvWnd, 1, &stLvColm );

	dCount = MoziSqlItemCount( NULL, NULL );
	MoziFileRebuild( ghMoziWnd, dCount ? FALSE : TRUE );

	ShowWindow( ghMoziWnd, SW_SHOW );
	UpdateWindow( ghMoziWnd );

	ghMoziViewWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_LAYERED, MOZIVIEW_CLASS,
		TEXT("배치"), WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE, 0, 0, 160, 120, NULL, NULL, hInst, NULL);
	SetLayeredWindowAttributes( ghMoziViewWnd, 0, gbAlpha, LWA_ALPHA );

	ZeroMemory( &gstFrmSz, sizeof(POINT) );
	ClientToScreen( ghMoziViewWnd, &gstFrmSz );

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;
	gstViewOrigin.y = vwRect.top;
	x = (vwRect.left + LINENUM_WID) - gstFrmSz.x;
	y = (vwRect.top  + RULER_AREA)  - gstFrmSz.y;
	TRACE( TEXT("MOZI %d x %d"), x, y );

#ifdef _DEBUG
	SetWindowPos( ghMoziViewWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#else
	SetWindowPos( ghMoziViewWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#endif
	gstOffset.x = x - vwRect.left;
	gstOffset.y = y - vwRect.top;

	return ghMoziWnd;
}

HRESULT MoziEditAssemble( HWND hWnd )
{
	UINT_PTR	cchSz;
	INT			ileng, i, iNdot, iNlnDot, iMxLine, wid;
	INT			iMaxDot, iLastLine, iViewXdot, iYline, iViewYdot, cx, cy;
	HWND		hWorkWnd;
	LPTSTR		ptScript;
	RECT		rect;
	MOZIITEM	stMzitm;

	MZTM_ITR	itMzitm;

	hWorkWnd = GetDlgItem( hWnd, IDE_MZSCR_TEXT );

	ileng = Edit_GetTextLength( hWorkWnd );
	cchSz = ileng + 2;

	if( gcchMzBuf <  cchSz )
	{
		ptScript = (LPTSTR)realloc( gptMzBuff, cchSz * sizeof(TCHAR) );
		gptMzBuff = ptScript;
		gcchMzBuf = cchSz;
	}

	ZeroMemory( gptMzBuff, gcchMzBuf * sizeof(TCHAR) );
	Edit_GetText( hWorkWnd, gptMzBuff, cchSz );

	if( !( gvcMoziItem.empty( ) ) )
	{
		for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ ){	FREE( itMzitm->ptAA );	}
		gvcMoziItem.clear( );
	}

	iNdot   = 0;
	iNlnDot = 0;
	iMxLine = 1;
	iMaxDot   = 0;
	iLastLine = 0;
	for( i = 0; ileng > i; i++ )
	{
		ZeroMemory( &stMzitm, sizeof(MOZIITEM) );

		if( TEXT('\r') ==  gptMzBuff[i] )
		{
			iLastLine += iMxLine;

			iNdot = 0;
			iNlnDot += (iMxLine * LINE_HEIGHT);
			iMxLine = 1;
			i++;
		}
		else
		{
			stMzitm.ptAA  = MoziSqlItemSelect( gptMzBuff[i], &(stMzitm.iLine), &(stMzitm.iWidth) );
			if( !(stMzitm.ptAA) )
			{
				wid = ViewLetterWidthGet( gptMzBuff[i] );

				if( 10 <= wid ){	stMzitm.iWidth = gdAvrWidth;	}
				else{	stMzitm.iWidth = gdAvrWidth / 2;	}
				stMzitm.iLine = gdMaxLine;
			}
			stMzitm.cch   = gptMzBuff[i];
			stMzitm.iLeft = iNdot;
			stMzitm.iTop  = iNlnDot;
			if( iMxLine < stMzitm.iLine ){	iMxLine = stMzitm.iLine;	}

			gvcMoziItem.push_back( stMzitm );
			iNdot += stMzitm.iWidth;
		}

		if( iMaxDot < iNdot )	iMaxDot = iNdot;
	}
	iLastLine += iMxLine;
	iLastLine++;

	iYline = ViewAreaSizeGet( &iViewXdot );
	iViewYdot = iYline * LINE_HEIGHT;

	GetWindowRect( ghMoziViewWnd, &rect );
	cx = rect.right  - rect.left;
	cy = rect.bottom - rect.top;
	GetClientRect( ghMoziViewWnd, &rect );
	cx -= rect.right;
	cy -= rect.bottom;

	cx += iMaxDot;
	cy += (iLastLine * LINE_HEIGHT);

	if( iViewXdot < cx ){	cx =  iViewXdot;	}
	if( iViewYdot < cy ){	cy =  iViewYdot;	}

	if( 66 > cx ){	cx = 66;	}
	if( 66 > cy ){	cy = 66;	}

#ifdef _DEBUG
	SetWindowPos( ghMoziViewWnd, HWND_TOP, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
#else
	SetWindowPos( ghMoziViewWnd, HWND_TOPMOST, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
#endif

	InvalidateRect( ghMoziViewWnd, NULL, TRUE );

	return S_OK;
}

LRESULT CALLBACK gpfMoziEditProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
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
			TRACE( TEXT("[%X]MoziEdit COMMAND %d"), hWnd, id );

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

	return CallWindowProc( gpfOrigMoziEditProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK MoziViewProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_KEYDOWN,			Mzv_OnKey );
		HANDLE_MSG( hWnd, WM_PAINT,				Mzv_OnPaint );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING,	Mzv_OnWindowPosChanging );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGED,	Mzv_OnWindowPosChanged );

		case  WM_MOVING:	Mzv_OnMoving( hWnd, (LPRECT)lParam );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Mzv_OnMoving( HWND hWnd, LPRECT pstPos )
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

	xSb += gdHideXdot;
	dLine += gdViewTopLine;

	StringCchPrintf( atBuffer, SUB_STRING, TEXT("MOZI %d[dot] %d[line]"), xSb, dLine );
	MainStatusBarSetText( SB_LAYER, atBuffer );

	return;
}

BOOL Mzv_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
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

VOID Mzv_OnWindowPosChanged( HWND hWnd, const LPWINDOWPOS pstWpos )
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

VOID Mzv_OnKey( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
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
	Mzv_OnMoving( hWnd, &rect );

	return;
}

VOID Mzv_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	MoziViewDraw( hdc );

	EndPaint( hWnd, &ps );

	return;
}

VOID MoziViewDraw( HDC hDC )
{
	UINT_PTR	cchSize, dLeng, dPos;
	INT_PTR	iItems, iLn;
	INT		x, y, cmr;
	LPTSTR	ptHead;
	HFONT	hFtOld;

	MZTM_ITR	itMzitm;

	hFtOld = SelectFont( hDC, ghAaFont );
	SetBkMode( hDC, TRANSPARENT );

	iItems = gvcMoziItem.size( );

	cmr = 0;

	for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ )
	{
		if( !(itMzitm->ptAA) )	continue;

		StringCchLength( itMzitm->ptAA, STRSAFE_MAX_CCH, &cchSize );

		x      = itMzitm->iLeft;

		if( 0 >= x ){	cmr = 0;	}
		x += (gdMoziInterval * cmr);
		cmr++;

		y      = itMzitm->iTop;
		ptHead = itMzitm->ptAA;
		dLeng  = 0;
		dPos   = 0;
		for( iLn = 0; itMzitm->iLine >  iLn; iLn++ )
		{
			while( cchSize >= dLeng )
			{
				if( TEXT('\r') == itMzitm->ptAA[dLeng] )
				{
					ExtTextOut( hDC, x, y, 0, NULL, ptHead, dPos, NULL );

					y      += LINE_HEIGHT;
					dLeng  += 2;
					dPos    = 0;
					ptHead  = &(itMzitm->ptAA[dLeng]);
					break;
				}

				if( 0 == itMzitm->ptAA[dLeng] )
				{
					ExtTextOut( hDC, x, y, 0, NULL, ptHead, dPos, NULL );
					break;
				}

				dLeng++;	dPos++;
			}
		}
	}

	SelectFont( hDC, hFtOld );

	return;
}

HRESULT MoziMoveFromView( HWND hWnd, UINT state )
{
	RECT	vwRect = {0,0,0,0};
	POINT	lyPoint;

	if( !(ghMoziViewWnd) )	return S_FALSE;

	if( SIZE_MINIMIZED != state )
	{
		GetWindowRect( ghViewWnd, &vwRect );
		gstViewOrigin.x = vwRect.left;
		gstViewOrigin.y = vwRect.top;
	}

	if( SIZE_MINIMIZED == state )
	{
		ShowWindow( ghMoziViewWnd, SW_HIDE );
	}
	else
	{
		lyPoint.x = gstOffset.x + vwRect.left;
		lyPoint.y = gstOffset.y + vwRect.top;
#ifdef _DEBUG
		SetWindowPos( ghMoziViewWnd, HWND_TOP, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#else
		SetWindowPos( ghMoziViewWnd, HWND_TOPMOST, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#endif
	}

	return S_OK;
}

LRESULT CALLBACK MoziProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT,       Mzs_OnPaint );
		HANDLE_MSG( hWnd, WM_NOTIFY,      Mzs_OnNotify );
		HANDLE_MSG( hWnd, WM_COMMAND,     Mzs_OnCommand );
		HANDLE_MSG( hWnd, WM_DESTROY,     Mzs_OnDestroy );
		HANDLE_MSG( hWnd, WM_DROPFILES,   Mzs_OnDropFiles );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Mzs_OnContextMenu );

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Mzs_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	LRESULT	lRslt;

	switch( id )
	{
		case IDM_MOZI_DECIDE:
			if( !(0x10 & gdNowMode) )
			{
				MoziScriptInsert( hWnd );
				if( gbQuickClose  ){	DestroyWindow( hWnd );	}
			}
			break;

		case IDM_MOZI_REFRESH:
			if( 0x10 & gdNowMode  ){	MoziFileRefresh( hWnd );	 return;	}
			else{	MoziEditAssemble( hWnd );	}
			break;

		case IDE_MZSCR_TEXT:
			if( EN_UPDATE == codeNotify ){	MoziEditAssemble( hWnd );	}
			break;

		case IDM_MOZI_SETTING:
			lRslt = SendMessage( ghMoziToolBar, TB_ISBUTTONCHECKED, IDM_MOZI_SETTING, 0 );
			if( lRslt  )
			{
				gdNowMode |=  0x10;
				DragAcceptFiles( ghMoziWnd, TRUE );

				SetWindowPos( ghSettiLvWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );

				ShowWindow( ghTextWnd, SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDS_MZSCR_INTERVAL),	SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDE_MZSCR_INTERVAL),	SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDUD_MZSCR_INTERVAL),	SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDCB_MZSCR_TRANSPARENT),SW_HIDE );
			}
			else
			{
				gdNowMode &= ~0x10;
				DragAcceptFiles( ghMoziWnd, FALSE );

				SetWindowPos( ghTextWnd,    HWND_TOP,    0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
				ShowWindow( GetDlgItem(hWnd,IDS_MZSCR_INTERVAL),	SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDE_MZSCR_INTERVAL),	SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDUD_MZSCR_INTERVAL),	SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDCB_MZSCR_TRANSPARENT),SW_SHOW );

				ShowWindow( ghSettiLvWnd, SW_HIDE );
			}
			break;

		case IDCB_MZSCR_QUICKCLOSE:
			gbQuickClose = IsDlgButtonChecked( GetDlgItem(hWnd,IDTB_MZSCR_TOOLBAR), IDCB_MZSCR_QUICKCLOSE ) ? TRUE : FALSE;
			SetFocus( hWnd );
			break;

		case IDM_MOZI_LISTDEL:	MoziFileListDelete( hWnd  );	break;

		case IDM_PASTE:	TRACE( TEXT("MZ PASTE") );	SendMessage( ghTextWnd, WM_PASTE, 0, 0 );	return;
		case IDM_COPY:	SendMessage( ghTextWnd, WM_COPY,  0, 0 );	return;
		case IDM_CUT:	SendMessage( ghTextWnd, WM_CUT,   0, 0 );	return;

		default:	return;
	}

	return;
}

VOID Mzs_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	EndPaint( hWnd, &ps );

	return;
}

VOID Mzs_OnDestroy( HWND hWnd )
{
	MZTM_ITR	itMzitm;

	MainStatusBarSetText( SB_LAYER, TEXT("") );

	if( ghMoziViewWnd ){	DestroyWindow( ghMoziViewWnd  );	}

	if( !( gvcMoziItem.empty( ) ) )
	{
		for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ ){	FREE( itMzitm->ptAA );	}
		gvcMoziItem.clear( );
	}

	ghMoziWnd = NULL;

	return;
}

VOID Mzs_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu;
	POINT	stPoint;

	stPoint.x = (SHORT)xPos;
	stPoint.y = (SHORT)yPos;

	if( ghSettiLvWnd == hWndContext )
	{
		hMenu = CreatePopupMenu(  );

		AppendMenu( hMenu, MF_STRING, IDM_MOZI_LISTDEL, TEXT("리스트에서 삭제") );

		TrackPopupMenu( hMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );

		return;
	}

	return;
}

LRESULT Mzs_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	LPNMUPDOWN	pstNmUpDown;

	if( IDUD_MZSCR_INTERVAL == idFrom )
	{
		pstNmUpDown = (LPNMUPDOWN)pstNmhdr;

		if( UDN_DELTAPOS == pstNmUpDown->hdr.code )
		{

			TRACE( TEXT("UPDOWN %d"), pstNmUpDown->iDelta );
			gdMoziInterval += (pstNmUpDown->iDelta);
			SetDlgItemInt( hWnd, IDE_MZSCR_INTERVAL, gdMoziInterval, TRUE );

			InvalidateRect( ghMoziViewWnd, NULL, TRUE );
		}
	}

	return 0;
}

VOID Mzs_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	TRACE( TEXT("MOZI DROP[%s]"), atFileName );

	if( FileExtensionCheck( atFileName, TEXT(".ast") ) )
	{
		MoziFileListAdd( atFileName );
	}

	return;
}

HRESULT MoziFileListAdd( LPTSTR ptFilePath )
{
	TCHAR	atFileName[MAX_PATH];

	UINT	iItem;
	LVITEM	stLvi;

	StringCchCopy( atFileName, MAX_PATH, ptFilePath );
	PathStripPath( atFileName );

	iItem = ListView_GetItemCount( ghSettiLvWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask  = LVIF_TEXT;
	stLvi.iItem = iItem;

	stLvi.pszText  = atFileName;
	stLvi.iSubItem = 0;
	ListView_InsertItem( ghSettiLvWnd, &stLvi );

	stLvi.pszText  = ptFilePath;
	stLvi.iSubItem = 1;
	ListView_SetItem( ghSettiLvWnd, &stLvi );

	return S_OK;
}

HRESULT MoziFileListDelete( HWND hWnd )
{
	INT	iItem;

	iItem = ListView_GetNextItem( ghSettiLvWnd, -1, LVNI_ALL | LVNI_SELECTED );

	if( 0 >  iItem ){	return  E_ABORT;	}

	ListView_DeleteItem( ghSettiLvWnd, iItem );

	return S_OK;
}

HRESULT MoziFileRefresh( HWND hWnd )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	TCHAR	atFilePath[MAX_PATH];
	INT		iItem, i;
	UINT	bCheck;
	LVITEM	stLvi;

	iItem = ListView_GetItemCount( ghSettiLvWnd );

	ZeroMemory( atBuff, sizeof(atBuff) );
	WritePrivateProfileSection( TEXT("MoziScript"), atBuff, gatMoziIni );

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), iItem );
	WritePrivateProfileString( TEXT("MoziScript"), TEXT("Count"), atBuff, gatMoziIni );

	MoziSqlItemDeleteAll(  );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask       = LVIF_TEXT;
	stLvi.pszText    = atFilePath;
	stLvi.iSubItem   = 1;
	stLvi.cchTextMax = MAX_PATH;
	for( i = 0; iItem > i; i++ )
	{
		stLvi.iItem   = i;
		ListView_GetItem( ghSettiLvWnd, &stLvi );
		bCheck = ListView_GetCheckState( ghSettiLvWnd, i );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("File%d"), i );
		WritePrivateProfileString( TEXT("MoziScript"), atKeyName, atFilePath, gatMoziIni );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("Use%d"), i );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), bCheck );
		WritePrivateProfileString( TEXT("MoziScript"), atKeyName, atBuff, gatMoziIni );

		if( bCheck ){	MoziFileStore( atFilePath );	}
	}

	if( bCheck ){	MoziSpaceCreate(  );	}

	return S_OK;
}

HRESULT MoziFileRebuild( HWND hWnd, UINT bMode )
{
	UINT	dCount, d;
	UINT	bCheck;
	TCHAR	atFileName[MAX_PATH], atFilePath[MAX_PATH];
	TCHAR	atKeyName[MIN_STRING];
	LVITEM	stLvi;

	dCount = GetPrivateProfileInt( TEXT("MoziScript"), TEXT("Count"), 0, gatMoziIni );

	if( bMode ){	MoziSqlItemDeleteAll(  );	}
	ListView_DeleteAllItems( ghSettiLvWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask       = LVIF_TEXT;

	for( d = 0; dCount > d; d++ )
	{
		StringCchPrintf( atKeyName, MIN_STRING, TEXT("File%u"), d );

		ZeroMemory( atFilePath, sizeof(atFilePath) );
		GetPrivateProfileString( TEXT("MoziScript"), atKeyName, TEXT(""), atFilePath, MAX_PATH, gatMoziIni );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("Use%u"), d );
		bCheck = GetPrivateProfileInt( TEXT("MoziScript"), atKeyName, 0, gatMoziIni );

		StringCchCopy( atFileName, MAX_PATH, atFilePath );
		PathStripPath( atFileName );

		stLvi.iItem    = d;

		stLvi.pszText  = atFileName;
		stLvi.iSubItem = 0;
		ListView_InsertItem( ghSettiLvWnd, &stLvi );

		stLvi.pszText  = atFilePath;
		stLvi.iSubItem = 1;
		ListView_SetItem( ghSettiLvWnd, &stLvi );

		ListView_SetCheckState( ghSettiLvWnd, d, bCheck );

		if( bCheck && bMode ){	MoziFileStore( atFilePath );	}
	}

	if( bCheck && bMode ){	MoziSpaceCreate(  );	}

	return S_OK;
}

HRESULT MoziSpaceCreate( VOID )
{
	INT	iLine, iAvDot;

	MoziSqlItemCount( &iLine, &iAvDot );

	gdAvrWidth = iAvDot;
	gdMaxLine  = iLine;

	return S_OK;
}

HRESULT MoziScriptInsert( HWND hWnd )
{
	UINT	bTranst;
	INT		ixNowPage;
	INT		ixTmpPage;
	INT		iXhideBuf = 0, iYhideBuf = 0;

	LPVOID		pBuffer;
	INT			x, y, iByteSize, cmr;
	INT			iX = 0, iY = 0;
	HWND		hLyrWnd;
	RECT		rect;

	MZTM_ITR	itMzitm;

	bTranst = IsDlgButtonChecked( hWnd, IDCB_MZSCR_TRANSPARENT );

	ixNowPage = gixFocusPage;
	ixTmpPage = gixFocusPage;

#ifdef DO_TRY_CATCH
	try{
#endif

	if( !(bTranst) )
	{
		ixTmpPage = DocPageCreate( -1 );
		gixFocusPage = ixTmpPage;

		iXhideBuf = gdHideXdot;
		iYhideBuf = gdViewTopLine;
		gdHideXdot = 0;
		gdViewTopLine = 0;
	}

	hLyrWnd = LayerBoxVisibalise( GetModuleHandle(NULL), TEXT(" "), 0x10 );

	GetWindowRect( ghMoziViewWnd, &rect );

	cmr = 0;

	for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ )
	{
		if( !(itMzitm->ptAA) )	continue;

		LayerStringReplace( hLyrWnd, itMzitm->ptAA );

		x = (rect.left + gstFrmSz.x) + itMzitm->iLeft;

		if( 0 >= itMzitm->iLeft ){	cmr = 0;	}
		x += (gdMoziInterval * cmr);
		cmr++;

		y = (rect.top + gstFrmSz.y)  + itMzitm->iTop;

		LayerBoxPositionChange( hLyrWnd, x, y );

		LayerContentsImportable( hLyrWnd, IDM_LYB_OVERRIDE, &iX, &iY, D_INVISI );
	}

	if( bTranst )
	{

	}
	else
	{
		iByteSize = DocPageTextGetAlloc( gitFileIt, ixTmpPage, D_UNI, &pBuffer, TRUE );

		DocPageDelete( ixTmpPage, ixNowPage );

		GetWindowRect( ghViewWnd, &rect );
		x = rect.left;
		y = rect.top;
		ViewPositionTransform( &x, &y, 1 );
		LayerBoxPositionChange( hLyrWnd, x, y );

		gdHideXdot = iXhideBuf;
		gdViewTopLine = iYhideBuf;

		LayerStringReplace( hLyrWnd, (LPTSTR)pBuffer );

		LayerContentsImportable( hLyrWnd, IDM_LYB_OVERRIDE, &iX, &iY, D_INVISI );

		FREE(pBuffer);
	}

	GetWindowRect( ghViewWnd, &rect );
	x = rect.left + LINENUM_WID;
	y = rect.top  + RULER_AREA;

	GetWindowRect( ghMoziViewWnd, &rect );
	iX = (rect.left + gstFrmSz.x) - x;
	iY = (rect.top  + gstFrmSz.y) - y;

	iY /= LINE_HEIGHT;

	iX += gdHideXdot;
	iY += gdViewTopLine;

	if( gdHideXdot >    iX )	iX = gdHideXdot + 11;
	if( gdViewTopLine > iY )	iY = gdViewTopLine;

	ViewPosResetCaret( iX, iY );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), E_FAIL );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), E_FAIL );	}
#endif

	DestroyWindow( hLyrWnd );

	return S_OK;
}

UINT CALLBACK MoziItemTablise( LPTSTR ptLter, LPCTSTR ptCont, INT cchSize )
{
	UINT_PTR	cchSz;
	LPTSTR		ptItem;

	LPTSTR	ptCaret, ptNext;

	INT		cl, dot, maxd;

	StringCchLength( ptLter, STRSAFE_MAX_CCH, &cchSz );
	if( 1 != cchSz )	return 0;

	ptItem = (LPTSTR)malloc( (cchSize+1) * sizeof(TCHAR) );
	ZeroMemory( ptItem, (cchSize+1) * sizeof(TCHAR) );
	StringCchCopyN( ptItem, (cchSize+1), ptCont, cchSize );

	cl = 0;
	maxd = 0;
	ptCaret = ptItem;
	ptNext  = ptCaret;
	while( *ptCaret )
	{
		if( 0x000D == *ptCaret )
		{
			*ptCaret = 0x0000;
			MoziItemRemovePeriod( ptNext  );
			dot = ViewStringWidthGet( ptNext );
			*ptCaret = 0x000D;

			if( maxd <= dot )	maxd =  dot;
			cl++;

			ptCaret++;
			ptCaret++;
			ptNext  = ptCaret;
		}
		else
		{
			ptCaret++;
		}
	}

	MoziItemRemovePeriod( ptNext  );
	dot = ViewStringWidthGet( ptNext );
	if( maxd <= dot )	maxd = dot;
	cl++;

	MoziSqlItemInsert( ptLter, ptItem, cl, maxd );

	FREE(ptItem);

	return 1;
}

VOID MoziItemRemovePeriod( LPTSTR ptText )
{
	UINT_PTR	cchSize;
	UINT		d;

	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );
	if( 0 == cchSize )	return;

	for( d = 0; cchSize > d; d++ )
	{
		if( TEXT('.') != ptText[d] )	break;

		ptText[d] = (TCHAR)0x2006;
	}

	for( d = (cchSize-1); 0 < d; d-- )
	{
		if( TEXT('.') != ptText[d] )	break;

		ptText[d] = (TCHAR)0x2006;
	}

	return;
}

HRESULT MoziFileStore( LPTSTR ptFilePath )
{
	HANDLE		hFile;
	DWORD		readed, dByteSz;
	UINT_PTR	cchSize;
	LPSTR		pcStr;
	LPTSTR		ptText;

	hFile = CreateFile( ptFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return E_INVALIDARG;	}

	dByteSz = GetFileSize( hFile, NULL );
	pcStr = (LPSTR)malloc( dByteSz + 2 );
	ZeroMemory( pcStr, dByteSz + 2 );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pcStr, dByteSz, &readed, NULL );
	CloseHandle( hFile );

	if( FileExtensionCheck( ptFilePath, TEXT(".ast") ) )
	{
		ptText = SjisDecodeAlloc( pcStr );
		FREE(pcStr);

		StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

		MoziSqlTransOnOff( M_CREATE );
		DocStringSplitAST( ptText, cchSize, MoziItemTablise );
		MoziSqlTransOnOff( M_DESTROY );

		FREE(ptText);
	}
	else
	{
		FREE(pcStr);
		return E_NOTIMPL;
	}

	return S_OK;
}

HRESULT MoziSqlTableOpenClose( UINT bMode )
{

	CONST CHAR	cacMoziTable[] = { ("CREATE TABLE MoziScr ( id INTEGER PRIMARY KEY, letter TEXT, aacont TEXT, line INTEGER, dot INTEGER )") };
	INT		rslt;
	sqlite3_stmt	*statement;

	if( bMode )
	{
		rslt = sqlite3_open( (":memory:"), &gpMoziTable );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return E_FAIL;	}

		rslt = sqlite3_prepare( gpMoziTable, cacMoziTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpMoziTable );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

	}
	else
	{

		if( gpMoziTable ){	rslt = sqlite3_close( gpMoziTable );	}
		gpMoziTable = NULL;
	}

	return S_OK;
}

HRESULT MoziSqlTransOnOff( BYTE mode )
{
	if( mode )
	{
		sqlite3_exec( gpMoziTable, "BEGIN TRANSACTION",  NULL, NULL, NULL );
	}
	else
	{
		sqlite3_exec( gpMoziTable, "COMMIT TRANSACTION", NULL, NULL, NULL );
	}

	return S_OK;
}

UINT MoziSqlItemInsert( LPTSTR ptLter, LPTSTR ptCont, INT iLine, INT iDot )
{
	CONST CHAR	acMoziItemInsert[] = { ("INSERT INTO MoziScr ( letter, aacont, line, dot ) VALUES ( ?, ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM MoziScr") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;

	if( !(gpMoziTable) ){	TRACE( TEXT("MOZI NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpMoziTable, acMoziItemInsert, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_text16( statement, 1, ptLter, -1, SQLITE_STATIC );
	rslt = sqlite3_bind_text16( statement, 2, ptCont, -1, SQLITE_STATIC );
	rslt = sqlite3_bind_int(    statement, 3, iLine );
	rslt = sqlite3_bind_int(    statement, 4, iDot );

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpMoziTable );	}

	sqlite3_finalize( statement );

	rslt = sqlite3_prepare( gpMoziTable, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}

LPTSTR MoziSqlItemSelect( TCHAR ch, LPINT piLine, LPINT piDot )
{
	CONST CHAR	acSelect[] = { ("SELECT * FROM MoziScr WHERE letter == ?") };
	INT		rslt,iLine, iDot;
	UINT	index = 0;
	TCHAR	atMozi[3];
	LPTSTR	ptAac = NULL;
	sqlite3_stmt*	statement;

	if( !(gpMoziTable) ){	return NULL;	}

	if( !(piLine) ){	return NULL;	}	*piLine = 0;
	if( !(piDot)  ){	return NULL;	}	*piDot  = 0;

	rslt = sqlite3_prepare( gpMoziTable, acSelect, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return 0;	}

	atMozi[ 0] = ch;	atMozi[1] = NULL;
	sqlite3_reset( statement );
	rslt = sqlite3_bind_text16( statement, 1, atMozi, -1, SQLITE_STATIC );

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		index = sqlite3_column_int( statement , 0 );
		StringCchCopy( atMozi, 3, (LPCTSTR)sqlite3_column_text16( statement, 1 ) );
		rslt = sqlite3_column_bytes16( statement, 2 );
		rslt += 2;
		ptAac = (LPTSTR)malloc( rslt );
		ZeroMemory( ptAac, rslt );
		StringCchCopy( ptAac, (rslt / sizeof(TCHAR)), (LPCTSTR)sqlite3_column_text16( statement, 2 ) );
		iLine = sqlite3_column_int( statement , 3 );
		iDot  = sqlite3_column_int( statement , 4 );
		*piLine = iLine;
		*piDot  = iDot;
	}

	sqlite3_finalize( statement );

	return ptAac;
}

UINT MoziSqlItemCount( LPINT piLine, LPINT piAvDot )
{
	INT		iLine, iAvDot;
	UINT	rslt, iCount;
	sqlite3_stmt*	statement;

	rslt = sqlite3_prepare( gpMoziTable, ("SELECT COUNT(id), MAX(line), AVG(dot) FROM MoziScr"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return 0;	}

	sqlite3_step( statement );
	iCount = sqlite3_column_int( statement, 0 );
	iLine  = sqlite3_column_int( statement, 1 );
	iAvDot = sqlite3_column_int( statement, 2 );
	sqlite3_finalize(statement);

	if( piLine )	*piLine  = iLine;
	if( piAvDot )	*piAvDot = iAvDot;

	return iCount;
}

HRESULT MoziSqlItemDeleteAll( VOID )
{
	INT		rslt;
	sqlite3_stmt	*statement;

	if( !(gpMoziTable) ){	return E_NOTIMPL;	}

	rslt = sqlite3_prepare( gpMoziTable, ("DELETE FROM MoziScr"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return E_OUTOFMEMORY;	}
	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}
