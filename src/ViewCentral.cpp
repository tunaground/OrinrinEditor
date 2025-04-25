#include "stdafx.h"
#include "OrinrinEditor.h"

#define FIRST_STEP	TEXT("설명서.ast")

#define EDIT_VIEW_CLASS	TEXT("EDIT_VIEW")

typedef struct tagCOLOUROBJECT
{
	COLORREF	dTextColour;
	HBRUSH		hBackBrush;
	HPEN		hGridPen;
	HPEN		hCrLfPen;
	HBRUSH		hUniBackBrs;

} COLOUROBJECT, *LPCOLOUROBJECT;

static HINSTANCE	ghInst;

EXTERNED HWND	ghPrntWnd;
EXTERNED HWND	ghViewWnd;

extern  HWND	ghPgVwWnd;
extern  HWND	ghMaaWnd;
extern  HWND	ghLnTmplWnd;
extern  HWND	ghBrTmplWnd;
extern BOOLEAN	gbDockTmplView;

EXTERNED INT	gdXmemory;
EXTERNED INT	gdDocXdot;
EXTERNED INT	gdDocLine;
EXTERNED INT	gdDocMozi;

EXTERNED INT	gdHideXdot;
EXTERNED INT	gdViewTopLine;
EXTERNED SIZE	gstViewArea;
EXTERNED INT	gdDispingLine;

EXTERNED BOOLEAN	gbExtract;

EXTERNED HFONT	ghAaFont;
static HFONT	ghRulerFont;
static HFONT	ghNumFont4L;
static HFONT	ghNumFont5L;
static HFONT	ghNumFont6L;

static INT		gdAutoDiffBase;

static  UINT	gdUseMode;
static  UINT	gdUseSubMode;

static  UINT	gdSpaceView;

static BOOLEAN	gbGridView;
EXTERNED UINT	gdGridXpos;
EXTERNED UINT	gdGridYpos;

static BOOLEAN	gbRitRlrView;
EXTERNED UINT	gdRightRuler;

static BOOLEAN	gbUndRlrView;
EXTERNED UINT	gdUnderRuler;

static  UINT	gdWheelLine;

extern  UINT	gbSqSelect;
extern  UINT	gbBrushMode;

extern INT		gixFocusPage;

extern INT		gbTmpltDock;

extern  HWND	ghMainSplitWnd;
extern  LONG	grdSplitPos;

#ifdef PLUGIN_ENABLE

extern plugin::PLUGIN_FILE_LIST gPluginList;
#endif

static COLORREF	gaColourTable[] = {
	0x000000,
	0xFFFFFF, 0xABABAB, 0x0000FF, 0xAAAAAA, 0x000000,
	0xFFFFFF, 0x8080FF, 0xC0C000, 0xC0C000, 0x101010,
	0xEEEEEE, 0xFFCCCC, 0xFF0000, 0xE0E0E0, 0x00FFFF
};

#define CLRT_BASICPEN	0
#define CLRT_BASICBK	1
#define CLRT_SELECTBK	2
#define CLRT_SPACEWARN	3
#define CLRT_SPACELINE	4
#define CLRT_CARETFD	5
#define CLRT_CARETBK	6
#define CLRT_LASTSPWARN	7
#define CLRT_CRLF_MARK	8
#define CLRT_EOF_MARK	9
#define CLRT_RULER		10
#define CLRT_RULERBK	11
#define CLRT_CANTSJIS	12
#define CLRT_CARET_POS	13
#define CLRT_GRID_LINE	14
#define CLRT_FINDBACK	15

#define PENS_MAX	6
static  HPEN	gahPen[PENS_MAX];
#define PENT_CRLF_MARK	0
#define PENT_RULER		1
#define PENT_SPACEWARN	2
#define PENT_SPACELINE	3
#define PENT_CARET_POS	4
#define PENT_GRID_LINE	5

#define BRUSHS_MAX	6
static  HBRUSH	gahBrush[BRUSHS_MAX];
#define BRHT_BASICBK	0
#define BRHT_RULERBK	1
#define BRHT_SELECTBK	2
#define BRHT_LASTSPWARN	3
#define BRHT_CANTSJISBK	4
#define BRHT_FINDBACK	5

LRESULT	CALLBACK ViewWndProc( HWND, UINT, WPARAM, LPARAM );
BOOLEAN	Evw_OnCreate( HWND, LPCREATESTRUCT );
VOID	Evw_OnCommand( HWND , INT, HWND, UINT );
VOID	Evw_OnPaint( HWND );
VOID	Evw_OnDestroy( HWND );
VOID	Evw_OnVScroll( HWND, HWND, UINT, INT );
VOID	Evw_OnHScroll( HWND, HWND, UINT, INT );
VOID	Evw_OnContextMenu( HWND, HWND, UINT, UINT );

HRESULT	ViewScrollBarAdjust( LPVOID );

HRESULT	ViewRedrawDo( HWND, HDC );
HRESULT	ViewDrawMetricLine( HDC,UINT );
BOOLEAN	ViewDrawTextOut( HDC, INT, UINT, LPLETTER, UINT_PTR );
BOOLEAN	ViewDrawSpace( HDC, INT, UINT, LPTSTR, UINT_PTR, UINT );
HRESULT	ViewDrawReturnMark( HDC, INT, INT, UINT );
INT		ViewDrawEOFMark( HDC, INT, INT, UINT );

HRESULT	ViewDrawRuler( HDC );
HRESULT	ViewDrawLineNumber( HDC );

VOID	OperationUndoRedo( INT, PINT, PINT );

HRESULT	ViewColourEditDlg( HWND );
INT_PTR	CALLBACK ColourEditDlgProc( HWND, UINT, WPARAM, LPARAM );
UINT	ColourEditChoose( HWND, LPCOLORREF );
INT_PTR	ColourEditDrawItem( HWND, CONST LPDRAWITEMSTRUCT, LPCOLOUROBJECT );

VOID AaFontCreate( UINT bMode )
{
	LOGFONT	stFont;

	ViewingFontGet( &stFont );

	if( bMode ){	ghAaFont = CreateFontIndirect( &stFont );	}
	else{			DeleteFont( ghAaFont  );	}

	return;
}

#ifdef TODAY_HINT_STYLE

UINT XorShift( UINT seed )
{
	static  UINT	x = 123456789;
	static  UINT	y = 362436069;
	static  UINT	z = 521288629;
	static  UINT	w =  88675123;

	UINT	t;

	if( 0 != seed ){	x = seed;	}

	t = x ^ (x << 11);
	x = y;
	y = z;
	z = w;

	w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));

	return w;
}

UINT HintStringLoad( LPTSTR ptString, UINT_PTR cchLen, LPCTSTR ptHintPath )
{
	static  UINT	cdPreSel = 0;

	UINT	randVle, maxCnt, target;
	UINT	de;
	TCHAR	atKeyName[MIN_STRING];

	de = 0;
	maxCnt  = GetPrivateProfileInt( TEXT("HINT"), TEXT("count"), 0, ptHintPath );
	if( 2 <= maxCnt )
	{
		do
		{
			randVle = XorShift( 0 );
			target  = (randVle % maxCnt) + 1;
			de++;

			if( 5 <= de )	break;
		}
		while( cdPreSel == target );
	}
	else
	{
		target = 1;
	}
	cdPreSel = target;

	StringCchPrintf( atKeyName, MIN_STRING, TEXT("text%u"), target );

	GetPrivateProfileString( TEXT("HINT"), atKeyName, TEXT(""), ptString, cchLen, ptHintPath );

	if( 0 == ptString[0] )
	{
		StringCchCopy( ptString, cchLen, TEXT("힌트를 찾을 수 없습니다...") );
	}

	return target;
}

INT_PTR CALLBACK TodayHintDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static TCHAR	catHintPath[MAX_PATH];

	 INT	id;
	HWND	hWndCtl;
	UINT	codeNotify;
	TCHAR	atHintStr[BIG_STRING];

	UINT	dRslt;
	time_t	tmValue;

	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:
			time( &tmValue );
			XorShift( (UINT)tmValue );
			StringCchCopy( catHintPath, MAX_PATH, (LPCTSTR)lParam );

			SetWindowFont( GetDlgItem(hDlg,IDS_CHAR_IMAGE),  ghAaFont, TRUE );
			SetDlgItemText( hDlg, IDS_CHAR_IMAGE, TEXT("　　　　,、\r\n　　　//l_,....,_／l\r\n　　　|:レ´　　｀く|\r\n　　 {><}ノノハノ)ノ）\r\n　　 〈ヲ|リ ゜ ヮ゜ノ§\r\n　　 {X(*i:E`:';l]つ\r\n　　　,(ンi_ヲ;:V:>、\r\n　　　｀^'i_フ'i_ヲ'´") );

			HintStringLoad( atHintStr, BIG_STRING, catHintPath );
			SetWindowFont( GetDlgItem(hDlg,IDS_HINT_VIEWER), ghAaFont, TRUE );
			SetDlgItemText( hDlg, IDS_HINT_VIEWER, atHintStr );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = (INT)(LOWORD(wParam));
			hWndCtl = (HWND)(lParam);
			codeNotify = (UINT)HIWORD(wParam);
			switch( id )
			{
				case IDOK:
					dRslt = IsDlgButtonChecked( hDlg , IDCB_NEVER_VIEWING );
					if( dRslt ){	InitParamValue( INIT_SAVE, VL_HINT_ENABLE,  0 );	}
				case IDCANCEL:
					DestroyWindow( hDlg );	break;

				case IDB_NEXT_HINT:
					HintStringLoad( atHintStr, BIG_STRING, catHintPath );
					SetDlgItemText( hDlg, IDS_HINT_VIEWER, atHintStr );
					break;

				default:	break;
			}
			return (INT_PTR)TRUE;

		case WM_CLOSE:	DestroyWindow( hDlg );	return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}

VOID TodayHintPopup( HWND hWnd, HINSTANCE hInst, LPTSTR ptPath )
{
	HWND	hDlgWnd;
	TCHAR	atHintPath[MAX_PATH];
	HANDLE	hFile;

	if( !( InitParamValue( INIT_LOAD, VL_HINT_ENABLE, 1 ) ) )	return;

	StringCchCopy( atHintPath, MAX_PATH, ptPath );
	PathAppend( atHintPath, HINT_FILE );

	hFile = CreateFile( atHintPath, GENERIC_READ, 0, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE ==  hFile )	return;
	CloseHandle( hFile );

	hDlgWnd = CreateDialogParam( hInst, MAKEINTRESOURCE(IDD_TODAY_HINT_DLG), hWnd, TodayHintDlgProc, (LPARAM)atHintPath );
	if( hDlgWnd ){	ShowWindow( hDlgWnd , SW_SHOW );	}

	return;
}

#endif

HWND ViewInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame, LPTSTR ptArgv )
{
	TCHAR		atFile[MAX_PATH], atFirstStep[MAX_PATH];
	WNDCLASSEX	wcex;
	RECT		vwRect, rect;

	LOGFONT		stFont;

	INT			iFiles, i;
	LPARAM		dNumber;
	BOOLEAN		bOpen = FALSE;

	INT			spPos, iOpMode, iRslt;

	ghInst = hInstance;

	gdWheelLine = 0;
	SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &gdWheelLine, 0 );
	if( 0 == gdWheelLine )	gdWheelLine = 3;

	gaColourTable[CLRT_SELECTBK] = GetSysColor( COLOR_HIGHLIGHT );

	gaColourTable[CLRT_BASICPEN]  = InitColourValue( INIT_LOAD, CLRV_BASICPEN, gaColourTable[CLRT_BASICPEN] );
	gaColourTable[CLRT_BASICBK]   = InitColourValue( INIT_LOAD, CLRV_BASICBK,  gaColourTable[CLRT_BASICBK] );
	gaColourTable[CLRT_GRID_LINE] = InitColourValue( INIT_LOAD, CLRV_GRIDLINE, gaColourTable[CLRT_GRID_LINE] );
	gaColourTable[CLRT_CRLF_MARK] = InitColourValue( INIT_LOAD, CLRV_CRLFMARK, gaColourTable[CLRT_CRLF_MARK] );
	gaColourTable[CLRT_CANTSJIS]  = InitColourValue( INIT_LOAD, CLRV_CANTSJIS, gaColourTable[CLRT_CANTSJIS] );

	gahBrush[BRHT_BASICBK]    = CreateSolidBrush( gaColourTable[CLRT_BASICBK] );
	gahBrush[BRHT_RULERBK]    = CreateSolidBrush( gaColourTable[CLRT_RULERBK] );
	gahBrush[BRHT_SELECTBK]   = CreateSolidBrush( gaColourTable[CLRT_SELECTBK] );
	gahBrush[BRHT_LASTSPWARN] = CreateSolidBrush( gaColourTable[CLRT_SPACEWARN] );
	gahBrush[BRHT_CANTSJISBK] = CreateSolidBrush( gaColourTable[CLRT_CANTSJIS] );
	gahBrush[BRHT_FINDBACK]   = CreateSolidBrush( gaColourTable[CLRT_FINDBACK] );

	gahPen[PENT_CRLF_MARK] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_CRLF_MARK] );
	gahPen[PENT_RULER]     = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_RULER] );
	gahPen[PENT_SPACEWARN] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_SPACEWARN] );
	gahPen[PENT_SPACELINE] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_SPACELINE] );
	gahPen[PENT_CARET_POS] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_CARET_POS] );
	gahPen[PENT_GRID_LINE] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_GRID_LINE] );

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= ViewWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_IBEAM);
	wcex.hbrBackground	= gahBrush[BRHT_BASICBK];
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= EDIT_VIEW_CLASS;
	wcex.hIconSm		= NULL;

	RegisterClassEx( &wcex );

	gdHideXdot = 0;
	gdViewTopLine  = 0;

	gdAutoDiffBase = 0;

	gdGridXpos   = InitParamValue( INIT_LOAD, VL_GRID_X_POS, 54 );
	gdGridYpos   = InitParamValue( INIT_LOAD, VL_GRID_Y_POS, 54 );
	gbGridView   = InitParamValue( INIT_LOAD, VL_GRID_VIEW, 0 );
	MenuItemCheckOnOff( IDM_GRID_VIEW_TOGGLE, gbGridView );

	gdRightRuler = InitParamValue( INIT_LOAD, VL_R_RULER_POS, 800 );
	gbRitRlrView = InitParamValue( INIT_LOAD, VL_R_RULER_VIEW, 1 );
	MenuItemCheckOnOff( IDM_RIGHT_RULER_TOGGLE, gbRitRlrView );

	gdUnderRuler = InitParamValue( INIT_LOAD, VL_U_RULER_POS, 30 );
	gbUndRlrView = InitParamValue( INIT_LOAD, VL_U_RULER_VIEW, 1 );
	MenuItemCheckOnOff( IDM_UNDER_RULER_TOGGLE, gbUndRlrView );

	gdSpaceView = InitParamValue( INIT_LOAD, VL_SPACE_VIEW, TRUE );
	MenuItemCheckOnOff( IDM_SPACE_VIEW_TOGGLE, gdSpaceView );
	OperationOnStatusBar(  );

	ghPrntWnd =  hParentWnd;

	rect = *pstFrame;
	if( gbTmpltDock )
	{
		spPos = grdSplitPos;
		rect.right -= spPos;
	};

	ghViewWnd = CreateWindowEx( 0, EDIT_VIEW_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
		rect.left, rect.top, rect.right, rect.bottom, hParentWnd, NULL, hInstance, NULL);

	if( !ghViewWnd ){	return NULL;	}

	ViewingFontGet( &stFont );
	stFont.lfPitchAndFamily = FIXED_PITCH;
	StringCchCopy( stFont.lfFaceName, LF_FACESIZE, TEXT("MS 고딕") );
	ghNumFont4L = CreateFontIndirect( &stFont );

	stFont.lfHeight = 13;
	ghNumFont5L = CreateFontIndirect( &stFont );

	stFont.lfHeight = 11;
	ghNumFont6L = CreateFontIndirect( &stFont );

	stFont.lfHeight = FONTSZ_REDUCE;
	stFont.lfPitchAndFamily = VARIABLE_PITCH;
	StringCchCopy( stFont.lfFaceName, LF_FACESIZE, TEXT("MS UI Gothic") );
	ghRulerFont = CreateFontIndirect( &stFont );

	GetClientRect( ghViewWnd, &vwRect );
	gstViewArea.cx = vwRect.right - LINENUM_WID;
	gstViewArea.cy = vwRect.bottom - RULER_AREA;

	gdDispingLine = gstViewArea.cy / LINE_HEIGHT;

	DocInitialise( TRUE );

	ZeroMemory( atFile, sizeof(atFile) );

	bOpen = FALSE;

	iOpMode = InitParamValue( INIT_LOAD, VL_LAST_OPEN, LASTOPEN_DO );
	if( LASTOPEN_ASK <= iOpMode )
	{
		iRslt = MessageBox( NULL, TEXT("마지막으로 열었던 파일을 열까요?"), TEXT("오린의 확인"), MB_YESNO | MB_ICONQUESTION );
		if( IDYES == iRslt )	iOpMode = LASTOPEN_DO;
		else					iOpMode = LASTOPEN_NON;
	}

	if( iOpMode ){	iFiles =  0;	}
	else{	iFiles = InitMultiFileTabOpen( INIT_LOAD, -1, NULL );	}

	for( i = 0; iFiles >= i; i++ )
	{

		if( iFiles == i ){	StringCchCopy( atFile, MAX_PATH, ptArgv );	}
		else{				InitMultiFileTabOpen( INIT_LOAD, i, atFile );	}

		dNumber = DocFileInflate( atFile  );
		if( 0 < dNumber )
		{
			if( !(bOpen) )
			{
				MultiFileTabFirst( atFile );
				bOpen = TRUE;
			}
			else
			{
				MultiFileTabAppend( dNumber, atFile );
			}

			AppTitleChange( atFile );
		}
	}

	if( 0 == InitParamValue( INIT_LOAD, VL_FIRST_READED, 0 ) )
	{
		GetModuleFileName( hInstance, atFirstStep, MAX_PATH );
		PathRemoveFileSpec( atFirstStep );
		PathAppend( atFirstStep, FIRST_STEP );

		dNumber = DocFileInflate( atFirstStep );
		if( 0 < dNumber )
		{
			if( !(bOpen) )
			{
				MultiFileTabFirst( atFirstStep );
				bOpen = TRUE;
			}
			else
			{
				MultiFileTabAppend( dNumber , atFirstStep );
			}
			AppTitleChange( atFirstStep );

			InitParamValue( INIT_SAVE, VL_FIRST_READED, 1 );
		}
	}

	if( !(bOpen) )
	{
		DocActivateEmptyCreate( atFile );

	}

	ViewScrollBarAdjust( NULL );

	ShowWindow( ghViewWnd, SW_SHOW );
	UpdateWindow( ghViewWnd );

	ViewCaretCreate( ghViewWnd, gaColourTable[CLRT_CARETFD], gaColourTable[CLRT_CARETBK] );

	gdDocXdot = 0;
	gdDocMozi = 0;
	gdDocLine = 0;
	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );

	gdXmemory = 0;

	ViewNowPosStatus(  );

	return ghViewWnd;
}

COLORREF ViewMoziColourGet( LPCOLORREF pCrtColour )
{
	if( pCrtColour )	*pCrtColour = gaColourTable[CLRT_CARETFD];

	return gaColourTable[CLRT_BASICPEN];
}

COLORREF ViewBackColourGet( LPVOID pVoid )
{
	return gaColourTable[CLRT_BASICBK];
}

HRESULT ViewSizeMove( HWND hPrntWnd, LPRECT pstFrame )
{
	LONG	iLeftPos;
	RECT	rect;

	rect = *pstFrame;

	if( gbTmpltDock )
	{
		iLeftPos = SplitBarResize( ghMainSplitWnd, pstFrame );
		grdSplitPos = rect.right - iLeftPos;

		PageListResize( hPrntWnd, pstFrame );
		LineTmpleResize( hPrntWnd, pstFrame );
		BrushTmpleResize( hPrntWnd, pstFrame );

		rect.right -= grdSplitPos;
		InitParamValue( INIT_SAVE, VL_MAIN_SPLIT, grdSplitPos );
	};

	SetWindowPos( ghViewWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW );

	GetClientRect( ghViewWnd, &rect );
	gstViewArea.cx = rect.right - LINENUM_WID;
	gstViewArea.cy = rect.bottom - RULER_AREA;

	gdDispingLine = gstViewArea.cy / LINE_HEIGHT;

	ViewScrollBarAdjust( NULL );

	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );

	return S_OK;
}

HRESULT ViewScrollBarAdjust( LPVOID pVoid )
{
	INT	dMargin, dRange, dDot, dPos, dLines;

	dMargin = gstViewArea.cx / 2;

	dDot = DocPageMaxDotGet( -1, -1 );
	dRange = dMargin + dDot;
	dRange -= gstViewArea.cx;

	if( 0 >= dRange )
	{
		EnableScrollBar( ghViewWnd, SB_HORZ, ESB_DISABLE_BOTH );
		if( 0 != gdHideXdot )
		{
			gdHideXdot = 0;
		}
	}
	else
	{
		EnableScrollBar( ghViewWnd, SB_HORZ, ESB_ENABLE_BOTH );
		SetScrollRange( ghViewWnd, SB_HORZ, 0, dRange, TRUE );
		dPos = gdHideXdot;
		if( 0 > dPos )	dPos = 0;
		SetScrollPos( ghViewWnd, SB_HORZ, dPos, TRUE );
	}

	dLines = DocNowFilePageLineCount(  );
	dRange = dLines - gdDispingLine;

	if( 0 >= dRange )
	{
		EnableScrollBar( ghViewWnd, SB_VERT, ESB_DISABLE_BOTH );

		if( 0 != gdViewTopLine )
		{
			gdViewTopLine = 0;
			ViewRedrawSetLine( -1 );
		}
	}
	else
	{
		EnableScrollBar( ghViewWnd, SB_VERT, ESB_ENABLE_BOTH );
		SetScrollRange( ghViewWnd, SB_VERT, 0, dRange, TRUE );
		dPos = gdViewTopLine;
		if( 0 > dPos )	dPos = 0;
		SetScrollPos( ghViewWnd, SB_VERT, dPos, TRUE );
	}

	return S_OK;
}

HRESULT ViewPositionTransform( PINT pDotX, PINT pDotY, BOOLEAN bTrans )
{
	assert( pDotX );
	assert( pDotY );

	if( bTrans )
	{
		*pDotX = *pDotX + LINENUM_WID;
		*pDotX = *pDotX - gdHideXdot;

		*pDotY = *pDotY + RULER_AREA;
		*pDotY = *pDotY - (gdViewTopLine*LINE_HEIGHT);
	}
	else
	{
		*pDotX = *pDotX + gdHideXdot;
		*pDotX = *pDotX - LINENUM_WID;

		*pDotY = *pDotY - RULER_AREA;
		*pDotY = *pDotY + (gdViewTopLine*LINE_HEIGHT);
	}

	return S_OK;
}

BOOLEAN ViewIsPosOnFrame( INT xx, INT yy )
{
	POINT	stPoint;
	RECT	stRect;

	SetRect( &stRect, 0, 0, gstViewArea.cx, gstViewArea.cy );

	stPoint.x = xx - LINENUM_WID;
	stPoint.y = yy - RULER_AREA;

	return PtInRect( &stRect, stPoint );
}

INT ViewAreaSizeGet( PINT pdXdot )
{
	if( pdXdot )	*pdXdot = gstViewArea.cx;

	return gdDispingLine;
}

HRESULT ViewEditReset( VOID )
{
	gdDocXdot = 0;
	gdDocLine = 0;
	gdDocMozi = 0;

	gdHideXdot = 0;
	gdViewTopLine = 0;

	ViewDrawCaret( 0, 0, TRUE );

	ViewScrollBarAdjust( NULL );

	ViewRedrawSetLine( -1 );

	return S_OK;
}

LRESULT CALLBACK ViewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	HIMC	hImc;
	LOGFONT	stFont;

	switch( message )
	{
		HANDLE_MSG( hWnd, WM_CREATE,        Evw_OnCreate );
		HANDLE_MSG( hWnd, WM_PAINT,         Evw_OnPaint );
		HANDLE_MSG( hWnd, WM_COMMAND,       Evw_OnCommand );
		HANDLE_MSG( hWnd, WM_DESTROY,       Evw_OnDestroy );
		HANDLE_MSG( hWnd, WM_VSCROLL,       Evw_OnVScroll );
		HANDLE_MSG( hWnd, WM_HSCROLL,       Evw_OnHScroll );
		HANDLE_MSG( hWnd, WM_KEYDOWN,       Evw_OnKey );
		HANDLE_MSG( hWnd, WM_KEYUP,         Evw_OnKey );
		HANDLE_MSG( hWnd, WM_CHAR,          Evw_OnChar );
		HANDLE_MSG( hWnd, WM_MOUSEMOVE,     Evw_OnMouseMove );
		HANDLE_MSG( hWnd, WM_MOUSEWHEEL,    Evw_OnMouseWheel );
		HANDLE_MSG( hWnd, WM_LBUTTONDOWN,   Evw_OnLButtonDown );
		HANDLE_MSG( hWnd, WM_LBUTTONDBLCLK, Evw_OnLButtonDown );
		HANDLE_MSG( hWnd, WM_LBUTTONUP,     Evw_OnLButtonUp );
		HANDLE_MSG( hWnd, WM_RBUTTONDOWN,   Evw_OnRButtonDown );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU,   Evw_OnContextMenu );

		case WM_SETFOCUS:
			TRACE( TEXT("VIEW_WM_SETFOCUS[0x%X][0x%X]"), wParam, lParam );
			ViewShowCaret(  );
			break;

		case WM_KILLFOCUS:
			TRACE( TEXT("VIEW_WM_KILLFOCUS[0x%X][0x%X]"), wParam, lParam );
			ViewHideCaret(  );
			break;

		case WM_ACTIVATE:
			TRACE( TEXT("VIEW_WM_ACTIVATE[0x%X][0x%X]"), wParam, lParam );
			if( WA_INACTIVE == LOWORD(wParam) ){	ViewHideCaret(  );	}
			break;

		case WM_IME_NOTIFY:
			TRACE( TEXT("WM_IME_NOTIFY[0x%X][0x%X]"), wParam, lParam );
			break;

		case WM_IME_REQUEST:
			TRACE( TEXT("WM_IME_REQUEST[0x%X][0x%X]"), wParam, lParam );
			break;

		case WM_IME_STARTCOMPOSITION:
			TRACE( TEXT("WM_IME_STARTCOMPOSITION[0x%X][0x%X]"), wParam, lParam );
				hImc = ImmGetContext( ghViewWnd );
				if( hImc )
				{
					ViewingFontGet( &stFont );
					ImmSetCompositionFont( hImc , &stFont );
					ImmReleaseContext( ghViewWnd , hImc );
				}
			break;

		case WM_IME_ENDCOMPOSITION:
			TRACE( TEXT("WM_IME_ENDCOMPOSITION[0x%X][0x%X]"), wParam, lParam );
			break;

		case WM_IME_COMPOSITION:
			Evw_OnImeComposition( hWnd, wParam, lParam );
			break;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

BOOLEAN Evw_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	HINSTANCE lcInst = lpCreateStruct->hInstance;
	UNREFERENCED_PARAMETER(lcInst);

	return TRUE;
}

VOID Evw_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	OperationOnCommand( ghPrntWnd, id, hWndCtl, codeNotify );

	return;
}

VOID Evw_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	ViewRedrawDo( hWnd, hdc );

	EndPaint( hWnd, &ps );

	return;
}

VOID Evw_OnDestroy( HWND hWnd )
{
	UINT	i;

	SetWindowFont( hWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
	DeleteFont( ghRulerFont );
	DeleteFont( ghNumFont4L );
	DeleteFont( ghNumFont5L );
	DeleteFont( ghNumFont6L );

	ViewCaretDelete(  );

	for( i = 0; PENS_MAX > i; i++ )
	{
		DeletePen( gahPen[i] );
	}

	for( i = 0; BRUSHS_MAX > i; i++ )
	{
		DeleteBrush( gahBrush[i] );
	}

	DocMultiFileCloseAll(  );

	PostQuitMessage( 0 );

	return;
}

VOID Evw_OnHScroll( HWND hWnd, HWND hWndCtl, UINT code, INT pos )
{
	SCROLLINFO	stScrollInfo;
	INT	dDot = gdHideXdot;

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( hWnd, SB_HORZ, &stScrollInfo );

	switch( code )
	{
		case SB_LINEUP:
			dDot--;
			break;

		case SB_PAGEUP:
			dDot -= gstViewArea.cx / 5;
			break;

		case SB_THUMBTRACK:
			dDot = stScrollInfo.nTrackPos;
			break;

		case SB_PAGEDOWN:
			dDot += gstViewArea.cx / 5;
			break;

		case SB_LINEDOWN:
			dDot++;
			break;

		default:	return;
	}

	if( 0 > dDot )	dDot = 0;
	if( stScrollInfo.nMax < dDot )	dDot = stScrollInfo.nMax;

	gdHideXdot = dDot;

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = dDot;
	SetScrollInfo( ghViewWnd, SB_HORZ, &stScrollInfo, TRUE );

	ViewRedrawSetLine( -1 );

#if 0

	if( gdHideXdot >  gdDocXdot )	gdDocXdot = gdHideXdot + 5;
	if( (gdHideXdot+gstViewArea.cx-EOF_WIDTH) <= gdDocXdot )	gdDocXdot = (gstViewArea.cx - EOF_WIDTH);
	DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0, 0 );
#endif

	ViewDrawCaret( gdDocXdot, gdDocLine, 0 );

	return;
}

VOID Evw_OnVScroll( HWND hWnd, HWND hWndCtl, UINT code, INT pos )
{
	SCROLLINFO	stScrollInfo;
	INT	dPos = gdViewTopLine, iLines, dPrev;

	iLines = DocNowFilePageLineCount(  );
	if( gdDispingLine >= iLines )	return;

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( hWnd, SB_VERT, &stScrollInfo );

	TRACE( TEXT("code[%d] pos[%d] dPos[%d] InfoMax[%d]"), code, pos, dPos, stScrollInfo.nMax );

	dPrev = dPos;

	switch( code )
	{
		case SB_LINEUP:
			if( pos ){	dPos = dPos - gdWheelLine;	}
			else{	 dPos--;	}
			break;

		case SB_PAGEUP:
			dPos -= gdDispingLine / 2;
			break;

		case SB_THUMBTRACK:
			dPos = stScrollInfo.nTrackPos;
			break;

		case SB_PAGEDOWN:
			dPos += gdDispingLine / 2;
			break;

		case SB_LINEDOWN:
			if( pos ){	dPos = dPos + gdWheelLine;	}
			else{	 dPos++;	}
			break;

		default:	return;
	}

	if( 0 > dPos )	dPos = 0;
	if( stScrollInfo.nMax < dPos )	dPos = stScrollInfo.nMax;

	gdViewTopLine = dPos;

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = dPos;
	SetScrollInfo( ghViewWnd, SB_VERT, &stScrollInfo, TRUE );

	if( dPrev != dPos ){	ViewRedrawSetLine( -1 );	}

#if 0

	if( gdViewTopLine >  gdDocLine )	gdViewTopLine = ++gdDocLine;
	if( (gdViewTopLine+gdDispingLine) <= gdDocLine )	gdDocLine--;
	DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0, 0 );
#endif

	ViewDrawCaret( gdDocXdot, gdDocLine, 0 );

	return;
}

VOID Evw_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	INT	posX, posY;

	HMENU	hSubMenu;
	UINT	dRslt;

	posX = (SHORT)xPos;
	posY = (SHORT)yPos;

	TRACE( TEXT("VIEW_WM_CONTEXTMENU %d x %d"), posX, posY );

	hSubMenu = CntxMenuGet(  );

	CheckMenuItem( hSubMenu , IDM_SQSELECT,           gbSqSelect   ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem( hSubMenu , IDM_SPACE_VIEW_TOGGLE,  gdSpaceView  ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem( hSubMenu , IDM_GRID_VIEW_TOGGLE,   gbGridView   ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem( hSubMenu , IDM_RIGHT_RULER_TOGGLE, gbRitRlrView ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem( hSubMenu , IDM_UNDER_RULER_TOGGLE, gbUndRlrView ? MF_CHECKED : MF_UNCHECKED );

	dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, posX, posY, 0, hWnd, NULL );

	FORWARD_WM_COMMAND( ghViewWnd, dRslt, hWndContext, 0, PostMessage );

	return;
}

HRESULT ViewFocusSet( VOID )
{

	SetFocus( ghViewWnd );

	SetWindowPos( ghPrntWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );

	TRACE( TEXT("키보드 포커스 설정") );

	return S_OK;
}

HRESULT ViewNowPosStatus( VOID )
{
	static INT	cdPreDot;
	TCHAR	atString[SUB_STRING];

	StringCchPrintf( atString, SUB_STRING, TEXT("%d[dot] %d[char] %d[line]"), gdDocXdot, gdDocMozi, gdDocLine + 1 );

	MainStatusBarSetText( SB_CURSOR, atString );

	cdPreDot = gdDocXdot;

	return S_OK;
}

INT ViewLetterWidthGet( TCHAR ch )
{
	SIZE	stSize;
	HDC		hdc= GetDC( ghViewWnd );
	HFONT	hFtOld;

	hFtOld = SelectFont( hdc, ghAaFont );

	GetTextExtentPoint32( hdc, &ch, 1, &stSize );

	SelectFont( hdc, hFtOld );

	ReleaseDC( ghViewWnd, hdc );

	return stSize.cx;
}

INT ViewStringWidthGet( LPCTSTR ptStr )
{
	SIZE	stSize;
	UINT	cchSize;
	HDC		hdc= GetDC( ghViewWnd );
	HFONT	hFtOld;

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

	if( 0 >= cchSize )	return 0;

	hFtOld = SelectFont( hdc, ghAaFont );

	GetTextExtentPoint32( hdc, ptStr, cchSize, &stSize );

	SelectFont( hdc, hFtOld );

	ReleaseDC( ghViewWnd, hdc );

	return stSize.cx;
}

HRESULT ViewRedrawSetRect( LPRECT pstRect )
{
	RECT	rect;

	if( !(pstRect) )	return E_INVALIDARG;

	rect = *pstRect;
	rect.right++;
	rect.bottom++;

	ViewPositionTransform( (PINT)&(rect.left),  (PINT)&(rect.top),    1 );
	ViewPositionTransform( (PINT)&(rect.right), (PINT)&(rect.bottom), 1 );

	InvalidateRect( ghViewWnd, &rect, TRUE );

	return S_OK;
}

HRESULT ViewRedrawSetVartRuler( INT rdLine )
{
	RECT	rect;
	INT	dDummy = 0;

	if( gdViewTopLine > rdLine )	return S_FALSE;
	if( (gdViewTopLine + gdDispingLine + 1) < rdLine )	return S_FALSE;

	rect.top    = rdLine * LINE_HEIGHT;
	ViewPositionTransform( &dDummy, (PINT)&(rect.top), 1 );

	rect.bottom = rect.top + LINE_HEIGHT;
	rect.left   = 0;
	rect.right  = LINENUM_WID + 2;

	InvalidateRect( ghViewWnd, &rect, TRUE );

	return S_OK;
}

HRESULT ViewRedrawSetLine( INT rdLine )
{
	RECT	rect, clRect;
	INT	dDummy;

	ViewScrollBarAdjust( NULL );

	if( 0 > rdLine )
	{
		InvalidateRect( ghViewWnd, NULL, TRUE );
		return S_OK;
	}

	if( gdViewTopLine > rdLine )	return S_FALSE;
	if( (gdViewTopLine + gdDispingLine + 1) < rdLine )	return S_FALSE;

	GetClientRect( ghViewWnd, &clRect );

	SetRect( &rect, 0, rdLine * LINE_HEIGHT, clRect.right, (rdLine+1) * LINE_HEIGHT );

	dDummy = 0;
	ViewPositionTransform( &dDummy, (PINT)&(rect.top), 1 );
	ViewPositionTransform( &dDummy, (PINT)&(rect.bottom), 1 );

	InvalidateRect( ghViewWnd, &rect, TRUE );

	return S_OK;
}

HRESULT ViewRedrawDo( HWND hWnd, HDC hdc )
{
	LPLETTER	pstTexts = NULL;
	INT		cchLen = 0, dot, iLines, i, vwLines;
	UINT	dFlag = 0;
	HFONT	hFtOld;

	UINT	bTrace = FALSE;

	hFtOld = SelectFont( hdc, ghAaFont );

	iLines = DocPageParamGet( NULL, NULL );

	vwLines = gdDispingLine + 2 + gdViewTopLine;

	bTrace = TraceImageAppear( hdc, gdHideXdot, gdViewTopLine * LINE_HEIGHT );
	if( bTrace )	SetBkMode( hdc, TRANSPARENT );

	ViewDrawMetricLine( hdc, 0 );

	for( i = 0; iLines > i; i++ )
	{

		if( gdViewTopLine > i ){	continue;	}
		if( vwLines <= i )	break;

		dot = DocLineDataGetAlloc( i, 0, &(pstTexts), &cchLen, &dFlag );
		if( 0 < cchLen )
		{

			ViewDrawTextOut( hdc, 0, i, pstTexts, cchLen );
		}
		FREE( pstTexts );

		if( dFlag & CT_RETURN )
		{
			ViewDrawReturnMark( hdc, dot, i, dFlag );
		}

		if( dFlag & CT_EOF )
		{
			ViewDrawEOFMark( hdc, dot, i, dFlag );
		}
	}

	SelectFont( hdc, hFtOld );

	ViewDrawRuler( hdc );
	ViewDrawLineNumber( hdc );

	return S_OK;
}

BOOLEAN ViewDrawTextOut( HDC hdc, INT dDot, UINT rdLine, LPLETTER pstTexts, UINT_PTR cchLen )
{
	UINT_PTR	mz, cchMr;
	COLORREF	clrTextOld, clrBackOld, clrTrcMozi, clrMozi, clrRvsMozi;
	INT		dX, dY;
	INT		width, rdStart;
	LPTSTR	ptText;
	UINT	bStyle, cbSize;
	BOOLEAN	bRslt, doDraw;
	RECT	rect;

	dX = dDot;
	dY = rdLine * LINE_HEIGHT;

	cbSize = (cchLen + 1) * sizeof(TCHAR);
	ptText = (LPTSTR)malloc( cbSize );
	if( !(ptText) ){	TRACE( TEXT("malloc error") );	return FALSE;	}
	ZeroMemory( ptText, cbSize );

	if( TraceMoziColourGet( &clrTrcMozi ) ){	clrMozi = clrTrcMozi;	}
	else{					clrMozi =  gaColourTable[CLRT_BASICPEN];	}
	clrTextOld = SetTextColor( hdc, clrMozi );

	clrRvsMozi = ~clrMozi;
	clrRvsMozi &= 0x00FFFFFF;

	clrBackOld = SetBkColor(   hdc, gaColourTable[CLRT_BASICBK] );

	bStyle  = pstTexts[0].mzStyle;
	cchMr   = 0;
	width   = 0;
	rdStart = 0;
	doDraw  = FALSE;

	ViewPositionTransform( &rdStart, &dY, 1 );

	SetBkMode( hdc, TRANSPARENT );

	for( mz = 0; cchLen >= mz; mz++ )
	{

		if( bStyle == pstTexts[mz].mzStyle )
		{
			ptText[cchMr++] = pstTexts[mz].cchMozi;
			width += pstTexts[mz].rdWidth;
		}
		else{	doDraw = TRUE;	}

		if( cchLen ==  mz ){	doDraw = TRUE;	}

		if( doDraw )
		{

			if( bStyle & CT_SPACE )
			{
				ViewDrawSpace( hdc, rdStart, dY, ptText, cchMr, bStyle );

			}
			else
			{
				if( bStyle & CT_SELECT )
				{
					SetTextColor( hdc, clrRvsMozi );
					SetBkColor(   hdc, gaColourTable[CLRT_SELECTBK] );

					SetRect( &rect, rdStart, dY, rdStart + width, dY + LINE_HEIGHT );
					FillRect( hdc, &rect, gahBrush[BRHT_SELECTBK] );
				}
				else if( bStyle & CT_FINDED )
				{
					SetTextColor( hdc, clrMozi );
					SetBkColor(   hdc, gaColourTable[CLRT_FINDBACK] );

					SetRect( &rect, rdStart, dY, rdStart + width, dY + LINE_HEIGHT );
					FillRect( hdc, &rect, gahBrush[BRHT_FINDBACK] );
				}
				else if( bStyle & CT_CANTSJIS )
				{
					SetTextColor( hdc, clrMozi );
					SetBkColor(   hdc, gaColourTable[CLRT_CANTSJIS] );

					SetRect( &rect, rdStart, dY, rdStart + width, dY + LINE_HEIGHT );
					FillRect( hdc, &rect, gahBrush[BRHT_CANTSJISBK] );
				}
				else
				{
					SetTextColor( hdc, clrMozi );
					SetBkColor(   hdc, gaColourTable[CLRT_BASICBK] );
				}

				bRslt = ExtTextOut( hdc, rdStart, dY, 0, NULL, ptText, cchMr, NULL );
				if( !(bRslt)  ){	TRACE( TEXT("ExtTextOut error") );	return FALSE;	}
			}

			rdStart += width;

			bStyle = pstTexts[mz].mzStyle;
			ZeroMemory( ptText, cbSize );
			ptText[0] = pstTexts[mz].cchMozi;
			width  = pstTexts[mz].rdWidth;
			cchMr  = 1;

			doDraw = FALSE;
		}
	}

	FREE( ptText );

	SetTextColor( hdc, clrTextOld );
	SetBkColor(   hdc, clrBackOld );

	return TRUE;
}

BOOLEAN ViewDrawSpace( HDC hdc, INT dX, UINT dY, LPTSTR ptText, UINT_PTR cchLen, UINT bStyle )
{
	HPEN	hPenOld;
	INT		width, xx, yy;
	UINT	cchPos;
	SIZE	stSize;
	RECT	stRect;

	xx = dX;
	yy = dY;

	dY += (LINE_HEIGHT - 2);

	if( bStyle & CT_WARNING )
	{
		hPenOld = SelectPen( hdc , gahPen[PENT_SPACEWARN] );
	}
	else
	{
		hPenOld = SelectPen( hdc , gahPen[PENT_SPACELINE] );
	}

	GetTextExtentPoint32( hdc, ptText, cchLen, &stSize );
	if( bStyle & CT_SELECT )
	{
		SetRect( &stRect, xx, yy, xx + stSize.cx, yy + stSize.cy );
		FillRect( hdc, &stRect, gahBrush[BRHT_SELECTBK] );
	}
	else if( bStyle & CT_FINDED )
	{
		SetRect( &stRect, xx, yy, xx + stSize.cx, yy + stSize.cy );
		FillRect( hdc, &stRect, gahBrush[BRHT_FINDBACK] );
	}
	else if( bStyle & CT_CANTSJIS )
	{
		SetRect( &stRect, xx, yy, xx + stSize.cx, yy + stSize.cy );
		if( gdSpaceView )	FillRect( hdc, &stRect, gahBrush[BRHT_CANTSJISBK] );

	}

	if( gdSpaceView || (bStyle & CT_WARNING) )
	{
		for( cchPos = 0; cchLen > cchPos; cchPos++ )
		{
			if( TEXT(' ') == ptText[cchPos] )
			{
				width = SPACE_HAN;
			}
			else if( TEXT('　') == ptText[cchPos] )
			{
				width = SPACE_ZEN;
			}
			else
			{
				width = ViewLetterWidthGet( ptText[cchPos] );
			}

			MoveToEx( hdc, dX, dY, NULL );
			LineTo(   hdc, (dX + width - 1), dY  );

			dX += width;
		}
	}

	SelectPen( hdc, hPenOld );

	return TRUE;
}

HRESULT ViewDrawMetricLine( HDC hdc, UINT bUpper )
{
	HPEN	hPenOld;
	INT		dX, dY;
	INT		aX, aY;
	LONG	width, height;

	width = gstViewArea.cx + LINENUM_WID;
	height = gstViewArea.cy + RULER_AREA;

	if( gbGridView )
	{

		hPenOld = SelectPen( hdc , gahPen[PENT_GRID_LINE] );

		aX = gdGridXpos;
		aY = gdGridYpos;
		ViewPositionTransform( &aX, &aY, 1 );

		while( height > aY )
		{
			MoveToEx( hdc , LINENUM_WID, aY, NULL );
			LineTo(   hdc , width, aY );
			aY += gdGridYpos;
		}

		while( width  > aX )
		{
			MoveToEx( hdc, aX, RULER_AREA-1, NULL );
			LineTo(   hdc, aX, height );
			aX += gdGridXpos;
		}

		SelectPen( hdc, hPenOld );
	}

	if( gbRitRlrView || gbUndRlrView )
	{
		hPenOld = SelectPen( hdc , gahPen[PENT_SPACEWARN] );

		if( gbRitRlrView )
		{
			dX = gdRightRuler;
			dY = 0;
			ViewPositionTransform( &dX, &dY, 1 );

			MoveToEx( hdc, dX, RULER_AREA-1, NULL  );
			LineTo(   hdc, dX, height  );
		}

		if( gbUndRlrView )
		{
			dX = 0;
			dY = gdUnderRuler * LINE_HEIGHT;
			ViewPositionTransform( &dX, &dY, 1 );

			MoveToEx( hdc, LINENUM_WID, dY, NULL  );
			LineTo(   hdc, width, dY  );
		}

		SelectPen( hdc, hPenOld );
	}

	return S_OK;
}

HRESULT ViewDrawReturnMark( HDC hdc, INT dDot, INT rdLine, UINT dFlag )
{
	HPEN	hPenOld;
	INT		dX, dY;
	INT		aX, aY;
	COLORREF	clrBackOld = 0;
	RECT	rect;

	dX = dDot;
	dY = rdLine * LINE_HEIGHT;

	ViewPositionTransform( &dX, &dY, 1 );

	SetRect( &rect, dX, dY, dX + SPACE_ZEN, dY + LINE_HEIGHT );

	if( dFlag & CT_SELRTN )
	{
		clrBackOld = SetBkColor( hdc , gaColourTable[CLRT_SELECTBK] );
		FillRect( hdc, &rect, gahBrush[BRHT_SELECTBK] );
	}
	else if( dFlag & CT_FINDRTN )
	{
		clrBackOld = SetBkColor( hdc , gaColourTable[CLRT_FINDBACK] );
		FillRect( hdc, &rect, gahBrush[BRHT_FINDBACK] );
	}
	else if( dFlag & CT_LASTSP )
	{
		clrBackOld = SetBkColor( hdc , gaColourTable[CLRT_LASTSPWARN] );
		FillRect( hdc, &rect, gahBrush[BRHT_LASTSPWARN] );
	}
	else
	{
		clrBackOld = SetBkColor( hdc , gaColourTable[CLRT_BASICBK] );
		FillRect( hdc, &rect, gahBrush[BRHT_BASICBK] );
	}

	SetBkColor( hdc, clrBackOld );

	hPenOld = SelectPen( hdc , gahPen[PENT_CRLF_MARK] );

	aX = dX + 3;
	aY = dY + 3;
	MoveToEx( hdc, aX, aY, NULL );
	LineTo(   hdc, aX, aY + 12  );
	LineTo(   hdc, dX, aY + 9  );
	MoveToEx( hdc, aX, aY + 12, NULL );
	LineTo(   hdc, aX + 3, aY + 9 );

	SelectPen( hdc, hPenOld );

	return S_OK;
}

INT ViewDrawEOFMark( HDC hdc, INT dDot, INT rdLine, UINT dFlag )
{

	INT			dX, dY;
	COLORREF	clrTextOld, clrBackOld = 0;
	RECT		stClip;
	SIZE		stSize;

	dX = dDot;
	dY = rdLine * LINE_HEIGHT;

	ViewPositionTransform( &dX, &dY, 1 );

	clrTextOld = SetTextColor( hdc , gaColourTable[CLRT_EOF_MARK] );
	if( dFlag & CT_LASTSP )
	{
		clrBackOld = SetBkColor(   hdc , gaColourTable[CLRT_LASTSPWARN] );
		SetBkMode( hdc, OPAQUE );
	}

	GetTextExtentPoint32( hdc, gatEOF, EOF_SIZE, &stSize );

	stClip.left   = dX + 1;
	stClip.right  = dX + 1 + stSize.cx;
	stClip.top    = dY + 1;
	stClip.bottom = dY + LINE_HEIGHT;

	ExtTextOut( hdc, stClip.left, stClip.top, 0, &stClip, gatEOF, EOF_SIZE, NULL );

	SetTextColor( hdc, clrTextOld );
	if( dFlag & CT_LASTSP ){	SetBkColor( hdc, clrBackOld );	SetBkMode( hdc, TRANSPARENT );	}

	return stSize.cx;
}

HRESULT ViewRulerRedraw( INT iBgn, INT iEnd )
{
	RECT	rect;

	GetClientRect( ghViewWnd, &rect );
	rect.bottom = RULER_AREA;

	if( 0 <= iBgn ){	rect.left  = iBgn;	}
	if( 0 <= iEnd ){	rect.right = iEnd;	}

	InvalidateRect( ghViewWnd, &rect, TRUE );

	return S_OK;
}

HRESULT ViewDrawRuler( HDC hdc )
{
	HPEN	hPenOld;
	HFONT	hFtOld;
	LONG	width, pos, ln, start, dif, sbn, hei;
	TCHAR	atStr[10];
	UINT_PTR	count;
	RECT		rect;

	hPenOld = SelectPen( hdc, gahPen[PENT_RULER] );

	width = gstViewArea.cx + LINENUM_WID;

	SetBkMode( hdc, TRANSPARENT );

	SetRect( &rect, 0, 0, width, RULER_AREA );
	FillRect( hdc, &rect, gahBrush[BRHT_RULERBK] );

	MoveToEx( hdc, LINENUM_WID, RULER_AREA-1, NULL );
	LineTo(   hdc, width, RULER_AREA-1 );

	start = gdHideXdot;

	dif = start % 10;
	sbn = start / 10;
	if( dif ){	sbn++;	dif =  10 - dif;	}

	for( pos = 0, ln = sbn; width > pos; pos+=10, ln++ )
	{
		hei = 6;
		if( !( ln % 5 ) )	hei = 3;
		if( !( ln % 10 ) )	hei = 0;
		MoveToEx( hdc, LINENUM_WID+pos+dif, hei, NULL );
		LineTo(   hdc, LINENUM_WID+pos+dif, RULER_AREA-1 );
	}

	SelectPen( hdc, hPenOld );

	hFtOld = SelectFont( hdc, ghRulerFont );

	dif = start % 100;	if( dif )	dif = 100 - dif;
	sbn = start / 100;	if( dif )	sbn++;
	sbn *= 100;
	for( pos = 0, ln = sbn; width > pos; pos+=100, ln+=100 )
	{
		StringCchPrintf( atStr, 10, TEXT("%d"), ln );
		StringCchLength( atStr, 10, &count );
		ExtTextOut( hdc, LINENUM_WID+pos+2+dif, 0, 0, NULL, atStr, count, NULL );
	}

	SelectFont( hdc, hFtOld );

	hPenOld = SelectPen( hdc, gahPen[PENT_CARET_POS] );

	MoveToEx( hdc, LINENUM_WID + gdDocXdot, 1, NULL );
	LineTo(   hdc, LINENUM_WID + gdDocXdot, RULER_AREA-1 );

	SelectPen( hdc, hPenOld );

	if( 1 <= gdAutoDiffBase )
	{

		hPenOld = SelectPen( hdc, gahPen[PENT_SPACEWARN] );

		MoveToEx( hdc, LINENUM_WID + gdAutoDiffBase, 1, NULL );
		LineTo(   hdc, LINENUM_WID + gdAutoDiffBase, RULER_AREA-1 );

		SelectPen( hdc, hPenOld );
	}

	return S_OK;
}

HRESULT ViewDrawLineNumber( HDC hdc )
{
	HPEN		hPenOld;
	HFONT		hFtOld;
	LONG		height, num, hei;
	TCHAR		atStr[10];
	UINT_PTR	count;
	UINT		figure = 4;
	RECT		rect;

	hPenOld = SelectPen( hdc , gahPen[PENT_RULER] );

	height = gstViewArea.cy + RULER_AREA;

	SetBkMode( hdc, TRANSPARENT );

	SetRect( &rect, 0, 0, LINENUM_WID-1, height );
	FillRect( hdc, &rect, gahBrush[BRHT_RULERBK] );

	MoveToEx( hdc, LINENUM_WID-2, RULER_AREA-1, NULL  );
	LineTo(   hdc, LINENUM_WID-2, height  );

	SelectPen( hdc, hPenOld );

	num = gdViewTopLine;
	if( 9999 > num )
	{
		figure =  1;
		hFtOld = SelectFont( hdc, ghNumFont4L );
	}
	else if( 9999 <= num && num < 99999 )
	{
		figure =  3;
		hFtOld = SelectFont( hdc, ghNumFont5L );
	}
	else
	{
		figure =  5;
		hFtOld = SelectFont( hdc, ghNumFont6L );
	}

	for( hei = 0; height > hei; hei+=LINE_HEIGHT, num++ )
	{
		if( 1 == figure && 9999 <= num )
		{	figure =  3;	SelectFont( hdc , ghNumFont5L );	}

		if( 3 == figure && 99999 <= num )
		{	figure =  5;	SelectFont( hdc , ghNumFont6L );	}

		if( DocBadSpaceIsExist( num )  )
		{
			SetRect( &rect, 0, hei+RULER_AREA, LINENUM_WID-2, hei+RULER_AREA+LINE_HEIGHT );
			FillRect( hdc, &rect, gahBrush[BRHT_LASTSPWARN] );
		}

		switch( figure )
		{
			default:
			case  1:	StringCchPrintf( atStr, 10 , TEXT("%4d"), num + 1 );	break;
			case  3:	StringCchPrintf( atStr, 10 , TEXT("%5d"), num + 1 );	break;
			case  5:	StringCchPrintf( atStr, 10 , TEXT("%6d"), num + 1 );	break;
		}
		StringCchLength( atStr, 10, &count );
		ExtTextOut( hdc, 0, hei + RULER_AREA + figure, 0, NULL, atStr, count, NULL );

	}

	SelectFont( hdc, hFtOld );

	return S_OK;
}

HRESULT ViewFrameInsert( INT dMode )
{
	return DocFrameInsert( dMode , gbSqSelect );
}

HRESULT ViewMaaItemsModeSet( UINT dMode, UINT dSubMode )
{
	gdUseMode = dMode;
	gdUseSubMode = dSubMode;

	return S_OK;
}

UINT ViewMaaItemsModeGet( PUINT pdSubMode )
{
	if( pdSubMode ){	*pdSubMode = gdUseSubMode;	}

	return gdUseMode;
}

UINT ViewMaaMaterialise( HWND hWnd, LPSTR pcCont, UINT cbSize, UINT dMode )
{
	LPTSTR		ptString;
	UINT_PTR	cchSize;
	UINT		uRslt = TRUE;
	INT			xDot;

	if( MAA_DEFAULT ==  dMode ){	dMode = gdUseMode;	}
	if( MAA_SUBDEFAULT == dMode ){	dMode = gdUseSubMode;	}

	if( MAA_SJISCLIP == dMode )
	{
		DocClipboardDataSet( pcCont, (cbSize + 1), D_SJIS );
		return uRslt;
	}

	if( MAA_DRAUGHT == dMode )
	{
		DraughtItemAdding( hWnd, pcCont );
		return uRslt;
	}

	xDot = 0;

	ptString = SjisDecodeAlloc( pcCont );
	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	switch( dMode )
	{
		case MAA_UNICLIP:
			DocClipboardDataSet( ptString, (cchSize + 1) * sizeof(TCHAR), D_UNI );
			break;

		case MAA_LAYERED:
			LayerBoxVisibalise( ghInst, ptString, 0x00 );
			break;

		case MAA_INTERRUPT:
			DocInsertString( &gdDocXdot, &gdDocLine, NULL, ptString, D_SQUARE, TRUE );
			DocPageInfoRenew( -1, 1 );
			ViewPosResetCaret( 0, gdDocLine );

			break;

		case MAA_INSERT:
			DocInsertString( &xDot, &gdDocLine, NULL, ptString, 0, TRUE );
			DocPageInfoRenew( -1, 1 );
			ViewPosResetCaret( xDot, gdDocLine );
			break;

		default:	break;
	}

	FREE(ptString);

	return uRslt;
}

HRESULT OperationOnStatusBar( VOID )
{
	CONST  TCHAR	*catTexts[] = { { TEXT("[직사각형]") }, { TEXT("[채우기]") },
		{ TEXT("[추출]") }, { TEXT("[공백]") } };

	TCHAR	atString[SUB_STRING];

	ZeroMemory( atString, sizeof(atString) );

	if( gbSqSelect ){	StringCchCat( atString, SUB_STRING, catTexts[0] );	}
	if( gbBrushMode ){	StringCchCat( atString, SUB_STRING, catTexts[1] );	}
	if( gbExtract ){	StringCchCat( atString, SUB_STRING, catTexts[2] );	}
	if( gdSpaceView ){	StringCchCat( atString, SUB_STRING, catTexts[3] );	}

	MainStatusBarSetText( SB_OP_STYLE, atString );

	return S_OK;
}

VOID OperationUndoRedo( INT id, PINT pxDot, PINT pyLine )
{
	INT		dCrLf;

	DocPageSelStateToggle( -1 );

	if( IDM_UNDO == id ){		dCrLf = DocUndoExecute( pxDot, pyLine );	}
	else if( IDM_REDO == id ){	dCrLf = DocRedoExecute( pxDot, pyLine );	}
	else{	 return;	}

	if( dCrLf ){	ViewRedrawSetLine( -1 );	}
	else{		ViewRedrawSetLine( *pyLine );	}

	ViewDrawCaret( *pxDot, *pyLine, TRUE );

	return;
}

VOID OperationOnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	UINT	bMode;

	if( IDM_USERINS_ITEM_FIRST <= id && id <= IDM_USERINS_ITEM_LAST )
	{
		UserDefItemInsert( hWnd, (id - IDM_USERINS_ITEM_FIRST) );
		return;
	}

	if( IDM_OPEN_HIS_FIRST <= id && id <= IDM_OPEN_HIS_LAST )
	{
		OpenHistoryLoad( hWnd, id );
		return;
	}
	else if( IDM_OPEN_HIS_CLEAR ==  id )
	{
		OpenHistoryLogging( hWnd, NULL );
		return;
	}

#ifdef PLUGIN_ENABLE

	if( id >= IDM_PLUGIN_ITEM_BASE )
	{
		if( plugin::RunPlugin( gPluginList, id - IDM_PLUGIN_ITEM_BASE ) )
		{
			return;
		}
	}

#endif

	switch( id )
	{
		default:					TRACE( TEXT("미구현") );	break;

		case IDM_FILE_CLOSE:		MultiFileTabClose( -1 );	break;

		case  IDM_UNI_PALETTE:		UniDialogueEntry( ghInst, hWnd );	break;

		case  IDM_ON_PREVIEW:		PreviewVisibalise( gixFocusPage, TRUE );	break;

		case  IDM_PAGELIST_VIEW:	ShowWindow( ghPgVwWnd , SW_SHOW );		SetForegroundWindow( ghPgVwWnd );	break;

		case  IDM_LINE_TEMPLATE:	ShowWindow( ghLnTmplWnd , SW_SHOW );	SetForegroundWindow( ghLnTmplWnd  );	break;

		case  IDM_BRUSH_PALETTE:	ShowWindow( ghBrTmplWnd , SW_SHOW );	SetForegroundWindow( ghBrTmplWnd  );	break;

		case  IDM_INSFRAME_EDIT:	FrameEditDialogue( ghInst, hWnd, 0 );	break;

		case  IDM_GENERAL_OPTION:	OptionDialogueOpen(   );	break;

		case  IDM_TRACE_MODE_ON:	TraceDialogueOpen( ghInst, hWnd );	break;

		case  IDM_MOZI_SCR_OPEN:	MoziScripterCreate( ghInst , hWnd );	break;

		case IDM_VERT_SCRIPT_OPEN:	VertScripterCreate( ghInst , hWnd );	break;

		case IDM_COLOUR_EDIT_OPEN:	ViewColourEditDlg( hWnd );	break;

		case IDM_IN_UNI_SPACE:
		case IDM_INSTAG_COLOUR:
		case IDM_USERINS_NA:		ToolBarPseudoDropDown( hWnd , id );	break;

		case IDM_PAGENUM_DLG_OPEN:	DocPageNumInsert( ghInst, hWnd );	DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );	break;

		case IDM_BRUSH_STYLE:		BrushModeToggle(  );	break;

		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_MAIN,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_MAIN, -1 );	break;

		case IDM_TRC_VIEWTOGGLE:	TraceImgViewTglExt(   );	break;

		case IDM_NEWFILE:			DocOpenFromNull( hWnd );	break;

		case IDM_OPEN:				DocFileOpen( hWnd );	break;

		case IDM_OVERWRITESAVE:		DocFileSave( hWnd, D_SJIS );	PreviewVisibalise( gixFocusPage, FALSE );	break;

		case IDM_RENAMESAVE:		DocFileSave( hWnd , (D_SJIS|D_RENAME) );	PreviewVisibalise( gixFocusPage, FALSE );	break;

		case IDM_IMAGE_SAVE:		DocImageSave( hWnd, 0, ghAaFont );	break;

		case IDM_HTML_EXPORTE:		DocHtmlExport( hWnd );	break;

		case IDM_INSFRAME_ALPHA:	ViewFrameInsert( 0 );	break;
		case IDM_INSFRAME_BRAVO:	ViewFrameInsert( 1 );	break;
		case IDM_INSFRAME_CHARLIE:	ViewFrameInsert( 2 );	break;
		case IDM_INSFRAME_DELTA:	ViewFrameInsert( 3 );	break;
		case IDM_INSFRAME_ECHO:		ViewFrameInsert( 4 );	break;
		case IDM_INSFRAME_FOXTROT:	ViewFrameInsert( 5 );	break;
		case IDM_INSFRAME_GOLF:		ViewFrameInsert( 6 );	break;
		case IDM_INSFRAME_HOTEL:	ViewFrameInsert( 7 );	break;
		case IDM_INSFRAME_INDIA:	ViewFrameInsert( 8 );	break;
		case IDM_INSFRAME_JULIETTE:	ViewFrameInsert( 9 );	break;

		case IDM_INSFRAME_KILO:		ViewFrameInsert( 10 );	break;
		case IDM_INSFRAME_LIMA:		ViewFrameInsert( 11 );	break;
		case IDM_INSFRAME_MIKE:		ViewFrameInsert( 12 );	break;
		case IDM_INSFRAME_NOVEMBER:	ViewFrameInsert( 13 );	break;
		case IDM_INSFRAME_OSCAR:	ViewFrameInsert( 14 );	break;
		case IDM_INSFRAME_PAPA:		ViewFrameInsert( 15 );	break;
		case IDM_INSFRAME_QUEBEC:	ViewFrameInsert( 16 );	break;
		case IDM_INSFRAME_ROMEO:	ViewFrameInsert( 17 );	break;
		case IDM_INSFRAME_SIERRA:	ViewFrameInsert( 18 );	break;
		case IDM_INSFRAME_TANGO:	ViewFrameInsert( 19 );	break;

		case IDM_FRMINSBOX_OPEN:	FrameInsBoxCreate( ghInst, hWnd );	break;

		case IDM_MENUEDIT_DLG_OPEN:	CntxEditDlgOpen( hWnd );	break;

#ifdef ACCELERATOR_EDIT

		case IDM_ACCELKEY_EDIT_DLG_OPEN:	AccelKeyDlgOpen( hWnd );	break;
#endif

#ifdef FIND_STRINGS

		case  IDM_FIND_DLG_OPEN:		FindDialogueOpen( ghInst, hWnd );	break;
#ifdef SEARCH_HIGHLIGHT
		case IDM_FIND_HIGHLIGHT_OFF:	FindHighlightOff(  );	break;
#endif

		case IDM_FIND_JUMP_NEXT:	FindDirectly( ghInst, hWnd, IDM_FIND_JUMP_NEXT );	break;

		case IDM_FIND_TARGET_SET:	FindDirectly( ghInst, hWnd, IDM_FIND_TARGET_SET );	break;

#endif
		case IDM_PAGENAME_SELASSIGN:	DocSelText2PageName(  );	break;

		case IDM_UNDO:	OperationUndoRedo( IDM_UNDO, &gdDocXdot, &gdDocLine );	break;

		case IDM_REDO:	OperationUndoRedo( IDM_REDO, &gdDocXdot, &gdDocLine );	break;

		case IDM_CUT:
			DocExClipSelect( D_UNI | gbSqSelect );
			if( IsSelecting( NULL ) ){	Evw_OnKey( hWnd, VK_DELETE, TRUE, 0, 0 );	}
			break;

		case IDM_COPY:
			if( gbExtract )
			{
				DocExtractExecute( NULL );
				gbExtract = FALSE;
				ViewSelPageAll( -1 );
				ViewRedrawSetLine( -1 );
				MenuItemCheckOnOff( IDM_EXTRACTION_MODE, 0 );
				OperationOnStatusBar(  );
			}
			else
			{
				DocExClipSelect( D_UNI | gbSqSelect );
			}
			break;

		case IDM_SJISCOPY:	DocExClipSelect( D_SJIS | gbSqSelect  );	break;

		case IDM_SJISCOPY_ALL:	DocPageAllCopy( D_SJIS );	break;

		case IDM_COPY_TO_DRAUGHT:	DraughtItemAddFromSelect( hWnd, gbSqSelect  );	break;

		case IDM_PASTE:			DocInputFromClipboard( &gdDocXdot, &gdDocLine, &gdDocMozi , 0 );	break;

		case IDM_SQUARE_PASTE:	DocInputFromClipboard( &gdDocXdot, &gdDocLine, &gdDocMozi , 1 );	break;

		case IDM_DELETE:		Evw_OnKey( hWnd, VK_DELETE, TRUE, 0, 0 );	break;

		case IDM_ALLSEL:		ViewSelPageAll( 1 );	break;

		case IDM_SQSELECT:		ViewSqSelModeToggle( 1 , NULL );	break;

		case IDM_FILL_SPACE:
			DocSelectedBrushFilling( NULL, &gdDocXdot , &gdDocLine );
			ViewDrawCaret( gdDocXdot, gdDocLine, 1 );
			DocPageInfoRenew( -1, 1 );
			break;

		case IDM_NOW_PAGE_REFRESH:
#if defined(FIND_STRINGS) && defined(SEARCH_HIGHLIGHT)
			FindNowPageReSearch(  );
#endif
			ViewRedrawSetLine( -1 );
			PreviewVisibalise( gixFocusPage, FALSE );
			break;

		case IDM_FILL_ZENSP:	DocScreenFill( TEXT("　") );	break;

		case IDM_EXTRACTION_MODE:
			if( gbExtract )
			{
				gbExtract = FALSE;
				ViewSelPageAll( -1 );
				ViewRedrawSetLine( -1 );
			}
			else{	gbExtract = TRUE;	}
			MenuItemCheckOnOff( IDM_EXTRACTION_MODE, gbExtract );
			OperationOnStatusBar(  );
			break;

		case IDM_LAYERBOX:
			if( gbExtract )
			{
				DocExtractExecute( ghInst );
				gbExtract = FALSE;
				ViewSelPageAll( -1 );
				ViewRedrawSetLine( -1 );
				MenuItemCheckOnOff( IDM_EXTRACTION_MODE, 0 );
				OperationOnStatusBar(  );
			}
			else
			{
				LayerBoxVisibalise( ghInst, NULL, 0x00 );
			}
			break;

		case IDM_IN_01SPACE:
		case IDM_IN_02SPACE:
		case IDM_IN_03SPACE:
		case IDM_IN_04SPACE:
		case IDM_IN_05SPACE:
		case IDM_IN_08SPACE:
		case IDM_IN_10SPACE:
		case IDM_IN_16SPACE:		ViewInsertUniSpace( id );	break;

		case IDM_INSTAG_WHITE:
		case IDM_INSTAG_BLUE:
		case IDM_INSTAG_BLACK:
		case IDM_INSTAG_RED:
		case IDM_INSTAG_GREEN:		ViewInsertColourTag( id );	break;

		case IDM_RIGHT_GUIDE_SET:	DocRightGuideline( NULL );	break;

		case IDM_RIGHT_SLIDE:		DocRightSlide( &gdDocXdot , gdDocLine );	break;

		case  IDM_DEL_LASTSPACE:	DocLastSpaceErase( &gdDocXdot , gdDocLine );	break;

		case IDM_INS_TOPSPACE:		DocTopLetterInsert( TEXT('　'), &gdDocXdot, gdDocLine );	break;

		case IDM_DEL_TOPSPACE:		DocTopSpaceErase( &gdDocXdot, gdDocLine );	break;

		case IDM_DEL_LASTLETTER:	DocLastLetterErase( &gdDocXdot, gdDocLine );	break;

		case  IDM_INCREMENT_DOT:	DocSpaceShiftProc( VK_RIGHT, &gdDocXdot, gdDocLine );	break;

		case  IDM_DECREMENT_DOT:	DocSpaceShiftProc( VK_LEFT,  &gdDocXdot, gdDocLine );	break;

		case IDM_INCR_DOT_LINES:	DocPositionShift( VK_RIGHT, &gdDocXdot, gdDocLine );	break;

		case IDM_DECR_DOT_LINES:	DocPositionShift( VK_LEFT,  &gdDocXdot, gdDocLine );	break;

#ifdef DOT_SPLIT_MODE

		case IDM_DOT_SPLIT_RIGHT:	DocCentreWidthShift( VK_RIGHT, &gdDocXdot, gdDocLine );	break;

		case IDM_DOT_SPLIT_LEFT:	DocCentreWidthShift( VK_LEFT,  &gdDocXdot, gdDocLine );	break;
#else
		case IDM_DOT_SPLIT_RIGHT:
		case IDM_DOT_SPLIT_LEFT:	MessageBox( hWnd, TEXT("まだ出来てないよ"), TEXT("Coming Soon ! !"), MB_OK );	break;
#endif

		case IDM_DOTDIFF_LOCK:
			gdAutoDiffBase = DocDiffAdjBaseSet( gdDocLine );
			ViewRulerRedraw( -1, -1 );
			break;

		case IDM_DOTDIFF_ADJT:	DocDiffAdjExec( &gdDocXdot, gdDocLine );	break;

		case IDM_HEADHALF_EXCHANGE:	DocHeadHalfSpaceExchange( hWnd );	break;

		case  IDM_MAATMPLE_VIEW:
			bMode = MaaViewToggle( TRUE );
			InitParamValue( INIT_SAVE, VL_MAA_TOPMOST, bMode );
			MenuItemCheckOnOff( IDM_MAATMPLE_VIEW, bMode );
			break;

		case IDM_SPACE_VIEW_TOGGLE:
			gdSpaceView = !(gdSpaceView);
			InitParamValue( INIT_SAVE, VL_SPACE_VIEW, gdSpaceView );
			MenuItemCheckOnOff( IDM_SPACE_VIEW_TOGGLE, gdSpaceView );
			OperationOnStatusBar(  );
			ViewRedrawSetLine( -1 );
			break;

		case IDM_GRID_VIEW_TOGGLE:
			gbGridView = !(gbGridView);
			InitParamValue( INIT_SAVE, VL_GRID_VIEW, gbGridView );
			MenuItemCheckOnOff( IDM_GRID_VIEW_TOGGLE, gbGridView );
			ViewRedrawSetLine( -1 );
			break;

		case IDM_RIGHT_RULER_TOGGLE:
			gbRitRlrView = !(gbRitRlrView);
			InitParamValue( INIT_SAVE, VL_R_RULER_VIEW, gbRitRlrView );
			MenuItemCheckOnOff( IDM_RIGHT_RULER_TOGGLE, gbRitRlrView );
			ViewRedrawSetLine( -1 );
			break;

		case IDM_UNDER_RULER_TOGGLE:
			gbUndRlrView = !(gbUndRlrView);
			InitParamValue( INIT_SAVE, VL_U_RULER_VIEW, gbUndRlrView );
			MenuItemCheckOnOff( IDM_UNDER_RULER_TOGGLE, gbUndRlrView );
			ViewRedrawSetLine( -1 );
			break;

		case IDM_UNICODE_TOGGLE:	UnicodeUseToggle( NULL );	break;

		case IDM_PAGEL_DIVIDE:	DocPageDivide( hWnd, ghInst, gdDocLine );	break;

		case IDM_REBER_DORESET:	ToolBarBandReset( hWnd );	break;

		case IDM_PAGEL_ADD:
		case IDM_PAGEL_INSERT:
		case IDM_PAGEL_DELETE:
		case IDM_PAGEL_DUPLICATE:
		case IDM_PAGEL_COMBINE:
		case IDM_PAGEL_UPFLOW:
		case IDM_PAGEL_DOWNSINK:
		case IDM_PAGEL_RENAME:
		case IDM_PAGE_PREV:
		case IDM_PAGE_NEXT:
			FORWARD_WM_COMMAND( ghPgVwWnd, id, hWndCtl, codeNotify, SendMessage );
			break;

		case IDM_TMPLT_GROUP_PREV:
		case IDM_TMPLT_GROUP_NEXT:
		case IDM_TMPL_GRID_INCREASE:
		case IDM_TMPL_GRID_DECREASE:
			if( gbTmpltDock )
			{
				if( IsWindowVisible( ghLnTmplWnd ) ){		FORWARD_WM_COMMAND( ghLnTmplWnd, id, hWndCtl, codeNotify, SendMessage );	}
				else if( IsWindowVisible( ghBrTmplWnd ) ){	FORWARD_WM_COMMAND( ghBrTmplWnd, id, hWndCtl, codeNotify, SendMessage );	}
			}
			break;

		case  IDM_FILE_PREV:	MultiFileTabSlide( -1 );	break;
		case  IDM_FILE_NEXT:	MultiFileTabSlide(  1 );	break;

		case IDM_DRAUGHT_OPEN:	DraughtWindowCreate( GetModuleHandle(NULL), hWnd, 0 );	break;

		case IDM_MAA_THUMBNAIL_OPEN:	DraughtWindowCreate( GetModuleHandle(NULL), hWnd, 1 );	break;

		case IDM_LINE_BRUSH_TMPL_VIEW:	DockingTmplViewToggle(  0 );	break;

		case IDM_MIRROR_INVERSE:	DocInverseTransform( gbSqSelect, 1, &gdDocXdot, gdDocLine );	break;
		case IDM_UPSET_INVERSE:		DocInverseTransform( gbSqSelect, 0, &gdDocXdot, gdDocLine );	break;

		case IDM_TESTCODE:
			TRACE( TEXT("기능 테스트") );
			break;
	}

	return;
}

HRESULT ViewColourEditDlg( HWND hWnd )
{
	INT_PTR	iRslt;

	COLORREF	cadColourSet[5];

	cadColourSet[0] =  gaColourTable[CLRT_BASICPEN];
	cadColourSet[1] =  gaColourTable[CLRT_BASICBK];
	cadColourSet[2] =  gaColourTable[CLRT_GRID_LINE];
	cadColourSet[3] =  gaColourTable[CLRT_CRLF_MARK];
	cadColourSet[4] =  gaColourTable[CLRT_CANTSJIS];

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_COLOUR_DLG), hWnd, ColourEditDlgProc, (LPARAM)cadColourSet );
	if( IDOK == iRslt )
	{
		gaColourTable[CLRT_BASICPEN]  = cadColourSet[0];
		gaColourTable[CLRT_BASICBK]   = cadColourSet[1];
		gaColourTable[CLRT_GRID_LINE] = cadColourSet[2];
		gaColourTable[CLRT_CRLF_MARK] = cadColourSet[3];
		gaColourTable[CLRT_CANTSJIS]  = cadColourSet[4];

		DeleteBrush( gahBrush[BRHT_BASICBK] );
		DeletePen(   gahPen[PENT_CRLF_MARK] );
		DeletePen(   gahPen[PENT_GRID_LINE] );
		DeleteBrush( gahBrush[BRHT_CANTSJISBK] );

		gahBrush[BRHT_BASICBK]    = CreateSolidBrush( gaColourTable[CLRT_BASICBK] );
		gahPen[PENT_CRLF_MARK]    = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_CRLF_MARK] );
		gahPen[PENT_GRID_LINE]    = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_GRID_LINE] );
		gahBrush[BRHT_CANTSJISBK] = CreateSolidBrush( gaColourTable[CLRT_CANTSJIS] );

		InitColourValue( INIT_SAVE, CLRV_BASICPEN, gaColourTable[CLRT_BASICPEN] );
		InitColourValue( INIT_SAVE, CLRV_BASICBK,  gaColourTable[CLRT_BASICBK] );
		InitColourValue( INIT_SAVE, CLRV_GRIDLINE, gaColourTable[CLRT_GRID_LINE] );
		InitColourValue( INIT_SAVE, CLRV_CRLFMARK, gaColourTable[CLRT_CRLF_MARK] );
		InitColourValue( INIT_SAVE, CLRV_CANTSJIS, gaColourTable[CLRT_CANTSJIS] );

		SetClassLongPtr( ghViewWnd, GCL_HBRBACKGROUND, (LONG_PTR)(gahBrush[BRHT_BASICBK]) );

		InvalidateRect( ghViewWnd, NULL, TRUE );
	}

	return S_OK;
}

INT_PTR CALLBACK ColourEditDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPCOLORREF	pcadColour;
	static COLOUROBJECT	cstColours;

	INT	id;

	switch( message )
	{
		case WM_INITDIALOG:
			pcadColour = (LPCOLORREF)lParam;
			cstColours.dTextColour = pcadColour[0];
			cstColours.hBackBrush  = CreateSolidBrush( pcadColour[1] );
			cstColours.hGridPen    = CreatePen( PS_SOLID, 1, pcadColour[2] );
			cstColours.hCrLfPen    = CreatePen( PS_SOLID, 1, pcadColour[3] );
			cstColours.hUniBackBrs = CreateSolidBrush( pcadColour[4] );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDOK:
				case IDCANCEL:
					DeleteBrush( cstColours.hBackBrush );
					DeletePen(   cstColours.hGridPen );
					DeletePen(   cstColours.hCrLfPen );
					DeleteBrush( cstColours.hUniBackBrs );
					EndDialog( hDlg, id );	return (INT_PTR)TRUE;

				case IDB_COLOUR_BASIC_PEN:
					if( ColourEditChoose( hDlg, &(pcadColour[0]) ) )
					{
						cstColours.dTextColour = pcadColour[0];
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				case IDB_COLOUR_BASIC_BACK:
					if( ColourEditChoose( hDlg, &(pcadColour[1]) ) )
					{
						DeleteBrush( cstColours.hBackBrush );
						cstColours.hBackBrush  = CreateSolidBrush( pcadColour[1] );
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				case IDB_COLOUR_GRID_LINE:
					if( ColourEditChoose( hDlg, &(pcadColour[2]) ) )
					{
						DeletePen(   cstColours.hGridPen );
						cstColours.hGridPen  = CreatePen( PS_SOLID, 1, pcadColour[2] );
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				case IDB_COLOUR_CRLF_MARK:
					if( ColourEditChoose( hDlg, &(pcadColour[3]) ) )
					{
						DeletePen(   cstColours.hCrLfPen );
						cstColours.hCrLfPen  = CreatePen( PS_SOLID, 1, pcadColour[3] );
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				case IDB_COLOUR_CANT_SJIS:
					if( ColourEditChoose( hDlg, &(pcadColour[4]) ) )
					{
						DeleteBrush( cstColours.hUniBackBrs );
						cstColours.hUniBackBrs  = CreateSolidBrush( pcadColour[4] );
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				default:	return (INT_PTR)FALSE;
			}
			break;

		case WM_DRAWITEM:	return ColourEditDrawItem( hDlg, ((CONST LPDRAWITEMSTRUCT)(lParam)), &cstColours );

	}
	return (INT_PTR)FALSE;
}

UINT ColourEditChoose( HWND hWnd, LPCOLORREF pdTgtColour )
{
	BOOL	bRslt;
	COLORREF	adColourTemp[16];
	CHOOSECOLOR	stChColour;

	ZeroMemory( adColourTemp, sizeof(adColourTemp) );
	adColourTemp[0] = *pdTgtColour;

	ZeroMemory( &stChColour, sizeof(CHOOSECOLOR) );
	stChColour.lStructSize  = sizeof(CHOOSECOLOR);
	stChColour.hwndOwner    = hWnd;

	stChColour.rgbResult    = *pdTgtColour;
	stChColour.lpCustColors = adColourTemp;
	stChColour.Flags        = CC_RGBINIT;

	bRslt = ChooseColor( &stChColour );
	if( bRslt ){	*pdTgtColour = stChColour.rgbResult;	}

	return bRslt;
}

INT_PTR ColourEditDrawItem( HWND hDlg, CONST LPDRAWITEMSTRUCT pstDrawItem, LPCOLOUROBJECT pstColours )
{
	const  TCHAR	catMihon[] = { TEXT("후란쨩 우후후~") };

	const  TCHAR	catUniMhn[] = { 0x2600, 0x2006, 0x2665, 0x0000 };

	RECT	rect;
	INT		xpos;

	INT		dotlen;
	INT		dX = 6 , dY = 6;
	INT		aX , aY;

	HFONT	hFtOld;
	HPEN	hPenOld;

	if( IDS_COLOUR_IMAGE != pstDrawItem->CtlID ){	return (INT_PTR)FALSE;	}

	GetClientRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), &rect );

	hFtOld = SelectFont( pstDrawItem->hDC, ghAaFont );
	SetBkMode( pstDrawItem->hDC, TRANSPARENT );

	SetTextColor( pstDrawItem->hDC, pstColours->dTextColour );

	FillRect( pstDrawItem->hDC, &(pstDrawItem->rcItem), pstColours->hBackBrush );

	hPenOld = SelectPen( pstDrawItem->hDC, pstColours->hGridPen );
	for( xpos = 40; rect.right > xpos; xpos += 40 )
	{
		MoveToEx( pstDrawItem->hDC, xpos, 0, NULL );
		LineTo(   pstDrawItem->hDC, xpos, rect.bottom );
	}

	dotlen = ViewStringWidthGet( catMihon );
	ExtTextOut( pstDrawItem->hDC, dX, dY, 0, NULL, catMihon, 9, NULL );
	dX += dotlen;

	SelectPen( pstDrawItem->hDC, pstColours->hCrLfPen );
	aX = dX + 3;
	aY = dY + 3;
	MoveToEx( pstDrawItem->hDC, aX, aY, NULL );
	LineTo(   pstDrawItem->hDC, aX, aY + 12  );
	LineTo(   pstDrawItem->hDC, dX, aY + 9  );
	MoveToEx( pstDrawItem->hDC, aX, aY + 12, NULL );
	LineTo(   pstDrawItem->hDC, aX + 3, aY + 9 );

	dX  = 6;
	dY += LINE_HEIGHT;
	dotlen = ViewStringWidthGet( catUniMhn );
	SetRect( &rect, dX, dY, dX + dotlen, dY + LINE_HEIGHT );
	FillRect( pstDrawItem->hDC, &rect, pstColours->hUniBackBrs );
	ExtTextOut( pstDrawItem->hDC, dX, dY, 0, NULL, catUniMhn, 3, NULL );

	SelectPen( pstDrawItem->hDC, hPenOld );
	SelectFont( pstDrawItem->hDC, hFtOld );

	return (INT_PTR)TRUE;
}
