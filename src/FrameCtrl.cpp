#include "stdafx.h"
#include "OrinrinEditor.h"

#define FRAMEINSERTBOX_CLASS	TEXT("FRAMEINSBOX_CLASS")
#define FIB_WIDTH	600
#define FIB_HEIGHT	200

#define TRANCE_COLOUR	RGB(0x66,0x77,0x88)

#define TB_ITEMS	26
static  TBBUTTON	gstFIBTBInfo[] = {
	{ 20,	IDM_FRAME_INS_DECIDE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,		{0, 0}, 0, 0  },
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{  0,	IDM_INSFRAME_ALPHA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  1,	IDM_INSFRAME_BRAVO   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  2,	IDM_INSFRAME_CHARLIE ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  3,	IDM_INSFRAME_DELTA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  4,	IDM_INSFRAME_ECHO    ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  5,	IDM_INSFRAME_FOXTROT ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  6,	IDM_INSFRAME_GOLF    ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  7,	IDM_INSFRAME_HOTEL   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  8,	IDM_INSFRAME_INDIA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  9,	IDM_INSFRAME_JULIETTE,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 10,	IDM_INSFRAME_KILO,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 11,	IDM_INSFRAME_LIMA,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 12,	IDM_INSFRAME_MIKE,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 13,	IDM_INSFRAME_NOVEMBER,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 14,	IDM_INSFRAME_OSCAR,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 15,	IDM_INSFRAME_PAPA,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 16,	IDM_INSFRAME_QUEBEC,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 17,	IDM_INSFRAME_ROMEO,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 18,	IDM_INSFRAME_SIERRA,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 19,	IDM_INSFRAME_TANGO,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{ 21,	IDM_FRMINSBOX_QCLOSE,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,		{0, 0}, 0, 0  },
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{ 22,	IDM_FRMINSBOX_PADDING,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,		{0, 0}, 0, 0  }
};

CONST  TCHAR	*gatDefaultName[20] = {
	{ TEXT("ALPHA") },		{ TEXT("BRAVO") },	{ TEXT("CHARLIE") },	{ TEXT("DELTA") },		{ TEXT("ECHO") },
	{ TEXT("FOXTROT") },	{ TEXT("GOLF") },	{ TEXT("HOTEL") },		{ TEXT("INDIA") },		{ TEXT("JULIETTE") },
	{ TEXT("KILO") },		{ TEXT("LIMA") },	{ TEXT("MIKE") },		{ TEXT("NOVEMBER") },	{ TEXT("OSCAR") },
	{ TEXT("POPPA") },		{ TEXT("QUEBEC") },	{ TEXT("ROMEO") },		{ TEXT("SIERRA") },		{ TEXT("TANGO") }
};

extern FILES_ITR	gitFileIt;
extern INT		gixFocusPage;

extern  HWND	ghViewWnd;

extern INT		gdHideXdot;
extern INT		gdViewTopLine;

extern HFONT	ghAaFont;

static INT		gNowSel;

static TCHAR	gatFrameIni[MAX_PATH];

static  ATOM		gFrmInsAtom;
static  HWND		ghFrInbxWnd;
static  HWND		ghFIBtlbrWnd;
static HBRUSH		ghBgBrush;

static HIMAGELIST	ghFrameImgLst;

static POINT		gstViewOrigin;
static POINT		gstOffset;
static POINT		gstFrmSz;
static INT			gdToolBarHei;

static  UINT		gdSelect;
static BOOLEAN		gbQuickClose;

extern HFONT		ghAaFont;

static  RECT		gstOrigRect;

static LPTSTR		gptFrmSample;
static  RECT		gstSamplePos;
static FRAMEINFO	gstNowFrameInfo;

static LPTSTR		gptFrmBox;

static  UINT		gbMultiPaddTemp;

static FRAMEINFO	gstFrameInfo[FRAME_MAX];

INT_PTR	CALLBACK FrameEditDlgProc( HWND, UINT, WPARAM, LPARAM );

INT_PTR	Frm_OnInitDialog( HWND , HWND, LPARAM );
INT_PTR	Frm_OnCommand( HWND , INT, HWND, UINT );
INT_PTR	Frm_OnDrawItem( HWND , CONST LPDRAWITEMSTRUCT );
INT_PTR	Frm_OnNotify( HWND , INT, LPNMHDR );

HRESULT	InitFrameItem( UINT, UINT, LPFRAMEINFO );

HRESULT	FramePartsUpdate( HWND, HWND, LPFRAMEITEM );

HRESULT	FrameDataGet( UINT, LPFRAMEINFO );
HRESULT	FrameInfoDisp( HWND );

VOID	FrameDataTranslate( LPTSTR, UINT );

UINT	FrameMultiSubstring( LPCTSTR, CONST UINT, LPTSTR, CONST UINT_PTR, CONST INT );

INT		FrameMultiSizeGet( LPFRAMEINFO, PINT, PINT );
LPTSTR	FrameMakeOutsideBoundary( CONST INT, CONST INT, LPFRAMEINFO );
LPTSTR	FrameMakeInsideBoundary( UINT , PINT, LPFRAMEINFO );

INT		FrameInsBoxSizeGet( LPRECT );
VOID	FrameInsBoxFrmDraw( HDC );
VOID	FrameDrawItem( HDC, INT, LPTSTR );
INT_PTR	Frm_OnSize( HWND, UINT, INT, INT );
INT_PTR	Frm_OnWindowPosChanging( HWND, LPWINDOWPOS );

HRESULT	FrameInsBoxDoInsert( HWND );

LRESULT	CALLBACK FrameInsProc( HWND, UINT, WPARAM, LPARAM );
VOID	Fib_OnPaint( HWND );
VOID	Fib_OnCommand( HWND, INT, HWND, UINT );
VOID	Fib_OnDestroy( HWND );
VOID	Fib_OnMoving( HWND, LPRECT );
VOID	Fib_OnKey( HWND, UINT, BOOL, INT, UINT );
BOOL	Fib_OnWindowPosChanging( HWND, LPWINDOWPOS );
VOID	Fib_OnWindowPosChanged( HWND, const LPWINDOWPOS );

HRESULT FrameInitialise( LPTSTR ptCurrent, HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;
	UINT		resnum, ici;
	HBITMAP		hImg, hMsq;
	INT	iRslt;

	if( !(ptCurrent) || !(hInstance) )
	{
		if( ghFrInbxWnd ){	DestroyWindow( ghFrInbxWnd );	}
		if( ghBgBrush ){	DeleteBrush( ghBgBrush );	}

		if( ghFrameImgLst ){	ImageList_Destroy( ghFrameImgLst  );	}

		return S_OK;
	}

	StringCchCopy( gatFrameIni, MAX_PATH, ptCurrent );
	PathAppend( gatFrameIni, FRAME_INI_FILE );

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= FrameInsProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= FRAMEINSERTBOX_CLASS;
	wcex.hIconSm		= NULL;

	gFrmInsAtom = RegisterClassEx( &wcex );

	ghBgBrush = CreateSolidBrush( TRANCE_COLOUR );

	ghFrInbxWnd = NULL;

	gptFrmBox = NULL;

	gdSelect = 0;

	gbQuickClose = TRUE;

	ZeroMemory( &gstViewOrigin, sizeof(POINT) );
	ZeroMemory( &gstFrmSz, sizeof(POINT) );
	gdToolBarHei = 0;

	ghFrameImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 23, 1 );
	resnum = IDBMP_FRMINS_ALPHA;
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_FRMINS_SEL ) );
	for( ici = 0; FRAME_MAX > ici; ici++ )
	{
		hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (resnum++) ) );
		iRslt = ImageList_Add( ghFrameImgLst, hImg, hMsq );
		DeleteBitmap( hImg );
	}
	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_FRMINS_INSERT ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_FRMINS_INSERT ) );
	iRslt = ImageList_Add( ghFrameImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_REFRESH ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_REFRESH ) );
	iRslt = ImageList_Add( ghFrameImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_FRMINS_PADD ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_FRMINS_PADD ) );
	iRslt = ImageList_Add( ghFrameImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	return S_OK;
}

HRESULT FrameNameModifyMenu( HWND hWnd )
{
	HMENU	hMenu, hSubMenu;
	UINT	i, j, k;
	TCHAR	atBuffer[MAX_PATH], atName[MAX_STRING];

	hMenu = GetMenu( hWnd );
	hSubMenu = GetSubMenu( hMenu, 2 );

	for( i = 0, j = 1; FRAME_MAX > i; i++, j++ )
	{
		FrameNameLoad( i, atName, MAX_STRING );

		if(  9 >= j ){		k = j + '0';	}
		else if( 10 == j ){	k = '0';	}
		else{		k = 'A' + j - 11;	}
		StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s(&%c)"), atName, k );
		ModifyMenu( hSubMenu, IDM_INSFRAME_ALPHA+i, MF_BYCOMMAND | MFT_STRING, IDM_INSFRAME_ALPHA+i, atBuffer );

	}

	DrawMenuBar( hWnd );

	return S_OK;
}

HRESULT FrameNameModifyPopUp( HMENU hPopMenu, UINT bMode )
{
	UINT	i, j, k;
	TCHAR	atBuffer[MAX_PATH], atName[MAX_STRING];

	for( i = 0, j = 1; FRAME_MAX > i; i++, j++ )
	{
		FrameNameLoad( i, atName, MAX_STRING );

		if( bMode )
		{
			if(  9 >= j ){		k = j + '0';	}
			else if( 10 == j ){	k = '0';	}
			else{		k = 'A' + j - 11;	}
			StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s(&%c)"), atName, k );
		}
		else
		{
			StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s"), atName );
		}
		ModifyMenu( hPopMenu, IDM_INSFRAME_ALPHA+i, MF_BYCOMMAND | MFT_STRING, IDM_INSFRAME_ALPHA+i, atBuffer );

	}

	return S_OK;
}

HRESULT FrameNameLoad( UINT dNumber, LPTSTR ptNamed, UINT_PTR cchSize )
{
	TCHAR	atAppName[MIN_STRING];

	if( !(ptNamed) || 0 >= cchSize )	return E_INVALIDARG;

	if( FRAME_MAX <= dNumber )	return E_OUTOFMEMORY;

	StringCchPrintf( atAppName, MIN_STRING, TEXT("Frame%u"), dNumber );

	GetPrivateProfileString( atAppName, TEXT("Name"), gatDefaultName[dNumber], ptNamed, cchSize, gatFrameIni );

	return S_OK;
}

HRESULT InitFrameItem( UINT dMode, UINT dNumber, LPFRAMEINFO pstInfo )
{
	TCHAR	atAppName[MIN_STRING], atBuff[MIN_STRING];
	TCHAR	atBuffer[PARTS_CCH];

	StringCchPrintf( atAppName, MIN_STRING, TEXT("Frame%u"), dNumber );

	if( dMode )
	{
		GetPrivateProfileString( atAppName, TEXT("Name"), gatDefaultName[dNumber], pstInfo->atFrameName, MAX_STRING, gatFrameIni );

		GetPrivateProfileString( atAppName, TEXT("Daybreak"),  TEXT("│"), pstInfo->stDaybreak.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Morning"),   TEXT("┌"), pstInfo->stMorning.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Noon"),      TEXT("─"), pstInfo->stNoon.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Afternoon"), TEXT("┐"), pstInfo->stAfternoon.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Nightfall"), TEXT("│"), pstInfo->stNightfall.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Twilight"),  TEXT("┘"), pstInfo->stTwilight.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Midnight"),  TEXT("─"), pstInfo->stMidnight.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Dawn"),      TEXT("└"), pstInfo->stDawn.atParts, PARTS_CCH, gatFrameIni );

		FrameDataTranslate( pstInfo->stDaybreak.atParts , 1 );
		FrameDataTranslate( pstInfo->stMorning.atParts , 1 );
		FrameDataTranslate( pstInfo->stNoon.atParts , 1 );
		FrameDataTranslate( pstInfo->stAfternoon.atParts , 1 );
		FrameDataTranslate( pstInfo->stNightfall.atParts , 1 );
		FrameDataTranslate( pstInfo->stTwilight.atParts , 1 );
		FrameDataTranslate( pstInfo->stMidnight.atParts , 1 );
		FrameDataTranslate( pstInfo->stDawn.atParts , 1 );

		GetPrivateProfileString( atAppName, TEXT("LEFTOFFSET"),  TEXT("0"), atBuff, MIN_STRING, gatFrameIni );
		pstInfo->dLeftOffset  = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("RIGHTOFFSET"), TEXT("0"), atBuff, MIN_STRING, gatFrameIni );
		pstInfo->dRightOffset = StrToInt( atBuff );

		GetPrivateProfileString( atAppName, TEXT("RestPadding"),  TEXT("1"), atBuff, MIN_STRING, gatFrameIni );
		pstInfo->dRestPadd = StrToInt( atBuff );

	}
	else
	{
		WritePrivateProfileString( atAppName, TEXT("Name"), pstInfo->atFrameName, gatFrameIni );

		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stDaybreak.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Daybreak"),  atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stMorning.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Morning"),   atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stNoon.atParts );		FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Noon"),      atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stAfternoon.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Afternoon"), atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stNightfall.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Nightfall"), atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stTwilight.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Twilight"),  atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stMidnight.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Midnight"),  atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stDawn.atParts );		FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Dawn"),      atBuffer, gatFrameIni );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dLeftOffset );
		WritePrivateProfileString( atAppName, TEXT("LEFTOFFSET"), atBuff, gatFrameIni );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dRightOffset );
		WritePrivateProfileString( atAppName, TEXT("RIGHTOFFSET"), atBuff, gatFrameIni );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dRestPadd );
		WritePrivateProfileString( atAppName, TEXT("RestPadding"), atBuff, gatFrameIni );
	}

	return S_OK;
}

INT Divinus( INT iLeft, INT iRight )
{
	 INT	iAnswer;

	if( 0 == iRight )	return 0;

	iAnswer = iLeft / iRight;

	return iAnswer;
}

INT_PTR FrameEditDialogue( HINSTANCE hInst, HWND hWnd, UINT dRsv )
{
	INT_PTR	iRslt;

	gNowSel = 0;

	iRslt = DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_FRAME_EDIT_DLG_2), hWnd, FrameEditDlgProc, 0 );

	if( IDYES == iRslt ){	FrameNameModifyMenu( hWnd );	}

	return iRslt;
}

INT_PTR CALLBACK FrameEditDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:	return Frm_OnInitDialog( hDlg, (HWND)(wParam), lParam );
		case WM_COMMAND:	return Frm_OnCommand( hDlg, (INT)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam) );
		case WM_DRAWITEM:	return Frm_OnDrawItem( hDlg, ((CONST LPDRAWITEMSTRUCT)(lParam)) );
		case WM_NOTIFY:		return Frm_OnNotify( hDlg, (INT)(wParam), (LPNMHDR)(lParam) );
		case WM_SIZE:		return Frm_OnSize( hDlg, (UINT)(wParam), (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam) );

		case WM_WINDOWPOSCHANGING:	return Frm_OnWindowPosChanging( hDlg, (LPWINDOWPOS)(lParam) );

	}
	return (INT_PTR)FALSE;
}

INT_PTR Frm_OnInitDialog( HWND hDlg, HWND hWndFocus, LPARAM lParam )
{
	HWND	hWorkWnd;
	UINT	i, ofs;
	RECT	rect;
	POINT	point;

	GetWindowRect( hDlg, &gstOrigRect );
	gstOrigRect.bottom -= gstOrigRect.top;
	gstOrigRect.right  -= gstOrigRect.left;
	gstOrigRect.top  = 0;
	gstOrigRect.left = 0;

	TRACE( TEXT("FRM DLG SIZE [%d x %d]"), gstOrigRect.right, gstOrigRect.bottom );

	hWorkWnd = GetDlgItem( hDlg, IDCB_BOX_NAME_SEL );

	for( i = 0; FRAME_MAX > i; i++ )
	{
		FrameDataGet( i, &(gstFrameInfo[i]) );

		ComboBox_AddString( hWorkWnd, gstFrameInfo[i].atFrameName );
	}

	ComboBox_SetCurSel( hWorkWnd, gNowSel );

	FrameInfoDisp( hDlg );

	SetWindowFont( GetDlgItem(hDlg,IDS_FRAME_IMAGE), ghAaFont, FALSE );

	GetWindowRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &gstSamplePos );

	gstSamplePos.right -= gstSamplePos.left;
	gstSamplePos.bottom -= gstSamplePos.top;

	GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
	ofs = gstSamplePos.right - rect.right;	gstSamplePos.right += ofs;
	ofs = gstSamplePos.bottom - rect.bottom;	gstSamplePos.bottom += ofs;

	point.x = gstSamplePos.left;	point.y = gstSamplePos.top;
	ScreenToClient( hDlg, &point );
	gstSamplePos.left = point.x;	gstSamplePos.top  = point.y;

	GetClientRect( hDlg, &rect );
	gstSamplePos.right  = rect.right  - gstSamplePos.right;
	gstSamplePos.bottom = rect.bottom - gstSamplePos.bottom;

	GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );

	gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

	return (INT_PTR)TRUE;
}

INT_PTR Frm_OnCommand( HWND hDlg, INT id, HWND hWndCtl, UINT codeNotify )
{
	static BOOLEAN	cbNameMod = FALSE;
	static BOOLEAN	cbNameChg = FALSE;
	UINT	i;
	INT		iRslt;
	HWND	hCmboxWnd;
	RECT	rect;

	switch( id )
	{
		default:	break;

		case IDCANCEL:
		case IDB_CANCEL:
			FREE( gptFrmSample );
			EndDialog( hDlg, IDCANCEL );
			return (INT_PTR)TRUE;

		case IDB_APPLY:
		case IDB_OK:
			hCmboxWnd = GetDlgItem( hDlg, IDCB_BOX_NAME_SEL );
			for( i = 0; FRAME_MAX > i; i++ )
			{
				InitFrameItem( INIT_SAVE, i, &(gstFrameInfo[i]) );
				if( cbNameChg )
				{
					ComboBox_DeleteString( hCmboxWnd, 0 );
					ComboBox_AddString( hCmboxWnd, gstFrameInfo[i].atFrameName );
				}
			}
			ComboBox_SetCurSel( hCmboxWnd, gNowSel );
			cbNameChg = FALSE;
			if( IDB_OK ==  id )
			{
				FREE( gptFrmSample );
				EndDialog( hDlg, cbNameMod ? IDYES : IDOK );
			}
			return (INT_PTR)TRUE;

		case IDE_BOXP_MORNING:		if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stMorning) );	}		return (INT_PTR)TRUE;
		case IDE_BOXP_NOON:			if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stNoon) );	}		return (INT_PTR)TRUE;
		case IDE_BOXP_AFTERNOON:	if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stAfternoon) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_DAYBREAK:		if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stDaybreak) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_NIGHTFALL:	if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stNightfall) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_TWILIGHT:		if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stTwilight) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_MIDNIGHT:		if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stMidnight) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_DAWN:			if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stDawn) );	}		return (INT_PTR)TRUE;

		case IDS_FRAME_IMAGE:
			if( STN_DBLCLK == codeNotify )
			{
				TRACE( TEXT("だぼーくるっく") );
				InvalidateRect( hWndCtl, NULL, TRUE );
			}
			return (INT_PTR)TRUE;

		case IDB_FRM_PADDING:
			iRslt = Button_GetCheck( hWndCtl );
			gstFrameInfo[gNowSel].dRestPadd = (BST_CHECKED == iRslt) ? TRUE : FALSE;

			GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
			FREE( gptFrmSample );
			gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

			InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );
			return (INT_PTR)TRUE;

		case IDB_BOXP_NAME_APPLY:
			Edit_GetText( GetDlgItem(hDlg,IDE_BOXP_NAME_EDIT), gstFrameInfo[gNowSel].atFrameName, MAX_STRING );
			cbNameMod = TRUE;	cbNameChg = TRUE;
			return (INT_PTR)TRUE;

		case IDCB_BOX_NAME_SEL:
			if( CBN_SELCHANGE == codeNotify )
			{
				gNowSel = ComboBox_GetCurSel( hWndCtl );
				FrameInfoDisp( hDlg );

				GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
				FREE( gptFrmSample );
				gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

				InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );
			}
			return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}

HRESULT FramePartsUpdate( HWND hDlg, HWND hWndCtl, LPFRAMEITEM pstItem )
{
	TCHAR	atBuffer[PARTS_CCH];

	if( Edit_GetTextLength( hWndCtl ) )
	{
		Edit_GetText( hWndCtl, atBuffer, PARTS_CCH );
		atBuffer[PARTS_CCH-1] = 0;
		StringCchCopy( pstItem->atParts, PARTS_CCH, atBuffer );
	}
	else
	{
		StringCchCopy( pstItem->atParts, PARTS_CCH, TEXT("　") );
	}

	pstItem->iLine = DocStringInfoCount( pstItem->atParts, 0, &(pstItem->dDot), NULL );

	InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );

	return S_OK;
}

INT_PTR Frm_OnWindowPosChanging( HWND hDlg, LPWINDOWPOS pstWpos )
{

	if( SWP_NOSIZE & pstWpos->flags )	return FALSE;

	TRACE( TEXT("FRM CHANGING [%d x %d][%d x %d]"), pstWpos->x, pstWpos->y, pstWpos->cx, pstWpos->cy );

	if( gstOrigRect.right > pstWpos->cx )	pstWpos->cx = gstOrigRect.right;
	if( gstOrigRect.bottom > pstWpos->cy )	pstWpos->cy = gstOrigRect.bottom;

	return (INT_PTR)TRUE;
}

INT_PTR Frm_OnSize( HWND hDlg, UINT state, INT cx, INT cy )
{
	HWND	hSmpWnd;
	INT		xx, yy;
	RECT	rect;

	TRACE( TEXT("FRM SIZE [%d x %d]"), cx, cy );

	hSmpWnd = GetDlgItem( hDlg, IDS_FRAME_IMAGE );

	xx = cx - gstSamplePos.right;
	yy = cy - gstSamplePos.bottom;

	SetWindowPos( hSmpWnd, HWND_TOP, 0, 0, xx, yy, SWP_NOMOVE | SWP_NOZORDER );

	GetClientRect( hSmpWnd, &rect );
	FREE( gptFrmSample );
	gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

	InvalidateRect( GetDlgItem( hDlg, IDS_FRAME_IMAGE ), NULL, TRUE );

	return (INT_PTR)TRUE;
}

INT_PTR Frm_OnDrawItem( HWND hDlg, CONST LPDRAWITEMSTRUCT pstDrawItem )
{

	if( IDS_FRAME_IMAGE != pstDrawItem->CtlID ){	return (INT_PTR)FALSE;	}

	FillRect( pstDrawItem->hDC, &(pstDrawItem->rcItem), GetSysColorBrush( COLOR_WINDOW ) );
	SetBkMode( pstDrawItem->hDC, TRANSPARENT );

	DrawText( pstDrawItem->hDC, gptFrmSample, -1, &(pstDrawItem->rcItem), DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK );

	return (INT_PTR)TRUE;
}

UINT FrameMakeMultiSubLine( CONST BOOLEAN bEnable, LPFRAMEITEM pstItem, LPTSTR ptDest, CONST UINT_PTR cchSz )
{
	LPTSTR	ptBufStr;

	if( bEnable )
	{

		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, ptDest, cchSz, pstItem->dDot );

		pstItem->iNowLn++;
	}
	else
	{
		ptBufStr = DocPaddingSpaceWithPeriod( pstItem->dDot, NULL, NULL, NULL, TRUE );
		StringCchCopy( ptDest, cchSz, ptBufStr );
		FREE( ptBufStr );
	}

	return 1;
}

INT FrameMultiSizeGet( LPFRAMEINFO pstInfo, PINT piUpLine, PINT piDnLine )
{
	INT	iUpLine, iDnLine;

	iUpLine = pstInfo->stMorning.iLine;
	if( iUpLine < pstInfo->stNoon.iLine )	iUpLine = pstInfo->stNoon.iLine;
	if( iUpLine < pstInfo->stAfternoon.iLine )	iUpLine = pstInfo->stAfternoon.iLine;

	iDnLine = pstInfo->stDawn.iLine;
	if( iDnLine < pstInfo->stMidnight.iLine )	iDnLine = pstInfo->stMidnight.iLine;
	if( iDnLine < pstInfo->stTwilight.iLine )	iDnLine = pstInfo->stTwilight.iLine;

	if( piUpLine )	*piUpLine = iUpLine;
	if( piDnLine )	*piDnLine = iDnLine;

	return pstInfo->stDaybreak.dDot;
}

UINT StringWidthAdjust( CONST UINT iFwOffs, LPTSTR ptStr, CONST UINT_PTR cchSz, CONST INT iMaxDot )
{
	INT			iDot, iPadd;
	INT			iDotCnt, iBuf;
	UINT_PTR	dm, dMozi;
	TCHAR		atWork[MAX_PATH];
	LPTSTR		ptBufStr;

	ZeroMemory( atWork, sizeof(atWork) );

	if( 1 <= iFwOffs )
	{
		ptBufStr = DocPaddingSpaceWithPeriod( iFwOffs, NULL, NULL, NULL, TRUE );
		if( ptBufStr )
		{
			StringCchCopy( atWork, MAX_PATH, ptBufStr );
			FREE( ptBufStr );
		}
	}
	StringCchCat( atWork, MAX_PATH, ptStr );

	iDot = ViewStringWidthGet( atWork );

	if( (0 != iMaxDot) && (iDot != iMaxDot) )
	{
		if( iDot < iMaxDot )
		{
			iPadd = iMaxDot - iDot;
			ptBufStr = DocPaddingSpaceWithPeriod( iPadd, NULL, NULL, NULL, TRUE );
			if( ptBufStr )
			{
				StringCchCat( atWork, MAX_PATH, ptBufStr );
				FREE( ptBufStr );
			}
		}
		else
		{
			StringCchLength( atWork, MAX_PATH, &dMozi );
			iDotCnt = 0;
			for( dm = 0; dMozi > dm; dm++ )
			{
				iBuf = ViewLetterWidthGet( atWork[dm] );
				if( iMaxDot < (iDotCnt+iBuf) )
				{
					atWork[dm] = NULL;
					iBuf = iMaxDot - iDotCnt;
					if( 0 < iBuf )
					{
						ptBufStr = DocPaddingSpaceWithPeriod( iBuf, NULL, NULL, NULL, TRUE );
						if( ptBufStr )
						{
							StringCchCat( atWork, MAX_PATH, ptBufStr );
							FREE( ptBufStr );
						}
					}
					break;
				}
				iDotCnt += iBuf;
			}
		}

		iDot = ViewStringWidthGet( atWork );
	}

	StringCchCopy( ptStr, cchSz, atWork );

	return iDot;
}

LPTSTR FrameMakeOutsideBoundary( CONST INT iWidth, CONST INT iHeight, LPFRAMEINFO pstInfo )
{
	LPFRAMEITEM	pstItem;

	TCHAR		atSubStr[MAX_PATH];
	LPTSTR		ptBufStr;
	INT	iLines, i;
	INT	iUpLine, iMdLine, iDnLine;

	INT	iRitOccup;
	INT	iOfsLine, iRight;
	INT	iRoofDot, iFloorDot;
	INT	iRoofCnt, iFloorCnt;
	INT	iRfRstDot, iFlRstDot;
	INT	iRitOff;
	INT	iRitBuf;
	INT	iRealWid;
	INT	ic;
	INT	iTgtLn;
	UINT	bMultiPadd;
	BOOLEAN	bEnable;

	UINT_PTR	cchTotal, dCount, d;

	HRESULT	hRslt;

	wstring	wsWorkStr;
	vector<wstring>	vcString;

#ifdef DO_TRY_CATCH
	try{
#endif

	wsWorkStr.assign( TEXT("") );

	iLines =  iHeight / LINE_HEIGHT;
	TRACE( TEXT("MF LINE %d"), iLines );

	bMultiPadd = pstInfo->dRestPadd;

	FrameMultiSizeGet( pstInfo, &iUpLine, &iDnLine );

	iMdLine = iLines - (iUpLine + iDnLine);
	TRACE( TEXT("MF R[%d] P[%d] F[%d]"), iUpLine, iMdLine, iDnLine );

	if( 0 > iMdLine ){	iLines -= iMdLine;	iMdLine = 0;	}

	for( i = 0; iLines > i; i++ )
	{
		vcString.push_back( wsWorkStr );
	}

	iRealWid = iWidth;

	iRitOccup = pstInfo->stAfternoon.dDot;
	if( iRitOccup <  pstInfo->stTwilight.dDot ){	iRitOccup = pstInfo->stTwilight.dDot;	}
	iRitBuf = pstInfo->dRightOffset + pstInfo->stNightfall.dDot;
	if( iRitOccup < iRitBuf ){	iRitOccup = iRitBuf;	}

	iRitOff = iWidth - iRitOccup;

	iRoofDot  = iRitOff - pstInfo->stMorning.dDot;
	if( 1 <= pstInfo->dLeftOffset ){	iRoofDot -= pstInfo->dLeftOffset;	}
	iRoofCnt  = Divinus( iRoofDot, pstInfo->stNoon.dDot );
	iRfRstDot = iRoofDot - (iRoofCnt * pstInfo->stNoon.dDot);

	iFloorDot = iRitOff - pstInfo->stDawn.dDot;
	if( 1 <= pstInfo->dLeftOffset ){	iFloorDot -= pstInfo->dLeftOffset;	}
	iFloorCnt = Divinus( iFloorDot , pstInfo->stMidnight.dDot );
	iFlRstDot = iFloorDot - (iFloorCnt * pstInfo->stMidnight.dDot);

	if( bMultiPadd )
	{
		iRight = iRitOff + pstInfo->dRightOffset;

		if( -1 >= pstInfo->dLeftOffset ){	iRight +=  pstInfo->dLeftOffset;	}

	}
	else
	{
		iRight = (iRoofCnt * pstInfo->stNoon.dDot) + pstInfo->stMorning.dDot + pstInfo->dRightOffset;
		iRitBuf = (iFloorCnt * pstInfo->stMidnight.dDot) + pstInfo->stDawn.dDot + pstInfo->dRightOffset;
		if( iRight < iRitBuf ){	iRight = iRitBuf;	};

		iRight +=  pstInfo->dLeftOffset;
	}

	TRACE( TEXT("MF RD[%d]C[%d][%d] FD[%d]C[%d][%d]"), iRoofDot, iRoofCnt, iRfRstDot, iFloorDot, iFloorCnt, iFlRstDot );

	pstInfo->stDaybreak.iNowLn  = 0;
	pstInfo->stMorning.iNowLn   = 0;
	pstInfo->stNoon.iNowLn      = 0;
	pstInfo->stAfternoon.iNowLn = 0;
	pstInfo->stNightfall.iNowLn = 0;
	pstInfo->stTwilight.iNowLn  = 0;
	pstInfo->stMidnight.iNowLn  = 0;
	pstInfo->stDawn.iNowLn      = 0;

	for( iTgtLn = 0, iOfsLine = 0; iUpLine > iTgtLn; iTgtLn++, iOfsLine++ )
	{

		if( 0 >= (iUpLine - iTgtLn) - pstInfo->stMorning.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stMorning), atSubStr, MAX_PATH );
		if( 1 <= pstInfo->dLeftOffset ){	StringWidthAdjust( pstInfo->dLeftOffset, atSubStr, MAX_PATH, 0 );	}
		vcString.at( iOfsLine ).append( atSubStr );

		if( 0 >= (iUpLine - iTgtLn) - pstInfo->stNoon.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stNoon), atSubStr, MAX_PATH );

		for( ic = 0; iRoofCnt >  ic; ic++ ){	vcString.at( iOfsLine ).append( atSubStr  );	}

		if( (1 <= iRfRstDot) && bMultiPadd )
		{
			StringWidthAdjust( 0, atSubStr, MAX_PATH, iRfRstDot );
			vcString.at( iOfsLine ).append( atSubStr );
		}

		if( 0 >= (iUpLine - iTgtLn) - pstInfo->stAfternoon.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stAfternoon), atSubStr, MAX_PATH );
		vcString.at( iOfsLine ).append( atSubStr );
	}

	for( iTgtLn = 0; iMdLine > iTgtLn; iTgtLn++, iOfsLine++ )
	{

		pstItem = &(pstInfo->stDaybreak);
		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, atSubStr, MAX_PATH, iRight );

		if( -1 >= pstInfo->dLeftOffset ){	StringWidthAdjust( -(pstInfo->dLeftOffset), atSubStr, MAX_PATH, 0 );	}
		pstItem->iNowLn++;
		if( pstItem->iLine <= pstItem->iNowLn ){	pstItem->iNowLn = 0;	}
		vcString.at( iOfsLine ).append( atSubStr );

		pstItem = &(pstInfo->stNightfall);
		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, atSubStr, MAX_PATH, 0 );
		pstItem->iNowLn++;
		if( pstItem->iLine <= pstItem->iNowLn ){	pstItem->iNowLn = 0;	}
		vcString.at( iOfsLine ).append( atSubStr );
	}

	for( iTgtLn = 0; iDnLine > iTgtLn; iTgtLn++, iOfsLine++ )
	{

		if( iTgtLn < pstInfo->stDawn.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stDawn), atSubStr, MAX_PATH );
		if( 1 <= pstInfo->dLeftOffset ){	StringWidthAdjust( pstInfo->dLeftOffset, atSubStr, MAX_PATH, 0 );	}
		vcString.at( iOfsLine ).append( atSubStr );

		if( iTgtLn < pstInfo->stMidnight.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stMidnight), atSubStr, MAX_PATH );

		for( ic = 0; iFloorCnt > ic; ic++ ){	vcString.at( iOfsLine ).append( atSubStr );	}

		if( (1 <= iFlRstDot) && bMultiPadd )
		{
			StringWidthAdjust( 0, atSubStr, MAX_PATH, iFlRstDot );
			vcString.at( iOfsLine ).append( atSubStr );
		}

		if( iTgtLn < pstInfo->stTwilight.iLine )
		{
			FrameMultiSubstring( pstInfo->stTwilight.atParts, pstInfo->stTwilight.iNowLn, atSubStr, MAX_PATH, 0 );
			pstInfo->stTwilight.iNowLn++;
			vcString.at( iOfsLine ).append( atSubStr );
		}
	}

	cchTotal = 0;
	dCount = vcString.size();

	for( d = 0; dCount > d; d++ ){	cchTotal += vcString.at( d ).size();	}
	cchTotal += (dCount * sizeof(TCHAR));
	ptBufStr = (LPTSTR)malloc( cchTotal * sizeof(TCHAR) );
	if( ptBufStr )
	{
		ZeroMemory( ptBufStr, cchTotal * sizeof(TCHAR) );
		for( d = 0; dCount > d; d++ )
		{
			if( 0 != d )	hRslt = StringCchCat( ptBufStr, cchTotal, TEXT("\r\n") );
			hRslt = StringCchCat( ptBufStr, cchTotal, vcString.at( d ).c_str() );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (LPTSTR)ETC_MSG( err.what( ), NULL );	}
	catch( ... ){	return (LPTSTR)ETC_MSG( ("etc error"), NULL );	}
#endif

	return ptBufStr;
}

LPTSTR FrameMakeInsideBoundary( UINT dType, PINT piValue, LPFRAMEINFO pstInfo )
{
	static INT	iRoofCnt, iFloorCnt;
	static INT	iRfRstDot, iFlRstDot;

	TCHAR		atSubStr[MAX_PATH];
	LPTSTR		ptBufStr;

	UINT	bMultiPadd;
	INT	iUpLine, iDnLine;
	INT	i, ic;
	INT	iRitOff;
	INT	iRitBuf, iRitVle;
	INT	iRoofDot, iFloorDot;
	INT	iTgtLn;
	INT	iOfsLine;
	BOOLEAN	bEnable;

	UINT_PTR	cchTotal, dCount, d;

	HRESULT	hRslt;

	wstring	wsWorkStr;
	vector<wstring>	vcString;

#ifdef DO_TRY_CATCH
	try{
#endif

	wsWorkStr.assign( TEXT("") );

	bMultiPadd = pstInfo->dRestPadd;

	if( 0 == dType )
	{

		iRitOff = pstInfo->stDaybreak.dDot + *piValue;

		if( -1 >= pstInfo->dLeftOffset ){	iRitOff +=  -(pstInfo->dLeftOffset);	}

		if( -1 >= pstInfo->dRightOffset ){	iRitOff +=  -(pstInfo->dRightOffset);	}

		iRoofDot  = iRitOff - pstInfo->stMorning.dDot;
		if( 1 <= pstInfo->dLeftOffset ){	iRoofDot -= pstInfo->dLeftOffset;	}
		iRoofCnt  = Divinus( iRoofDot, pstInfo->stNoon.dDot );
		iRfRstDot = iRoofDot - (iRoofCnt * pstInfo->stNoon.dDot);

		iFloorDot = iRitOff - pstInfo->stDawn.dDot;
		if( 1 <= pstInfo->dLeftOffset ){	iFloorDot -= pstInfo->dLeftOffset;	}
		iFloorCnt = Divinus( iFloorDot , pstInfo->stMidnight.dDot );
		iFlRstDot = iFloorDot - (iFloorCnt * pstInfo->stMidnight.dDot);

		if( !(bMultiPadd) )
		{

			if( 0 != iRfRstDot ){	iRoofCnt++;		}
			if( 0 != iFlRstDot ){	iFloorCnt++;	}

			iRitVle = pstInfo->stNoon.dDot - iFlRstDot;
			iRitBuf = pstInfo->stMidnight.dDot - iFlRstDot;
			if( iRitVle < iRitBuf ){	iRitVle = iRitBuf;	}

			iRitOff += iRitVle;
		}

		if( 1 <= pstInfo->dRightOffset ){	iRitOff += pstInfo->dRightOffset;	}

		*piValue = iRitOff;

		return NULL;
	}
	else if( 1 == dType )
	{
		FrameMultiSizeGet( pstInfo, &iUpLine, NULL );

		pstInfo->stMorning.iNowLn   = 0;
		pstInfo->stNoon.iNowLn      = 0;
		pstInfo->stAfternoon.iNowLn = 0;

		for( i = 0; iUpLine > i; i++ ){	vcString.push_back( wsWorkStr );	}

		for( iTgtLn = 0, iOfsLine = 0; iUpLine > iTgtLn; iTgtLn++, iOfsLine++ )
		{

			if( 0 >= (iUpLine - iTgtLn) - pstInfo->stMorning.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stMorning), atSubStr, MAX_PATH );
			if( 1 <= pstInfo->dLeftOffset ){	StringWidthAdjust( pstInfo->dLeftOffset, atSubStr, MAX_PATH, 0 );	}
			vcString.at( iOfsLine ).append( atSubStr );

			if( 0 >= (iUpLine - iTgtLn) - pstInfo->stNoon.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stNoon), atSubStr, MAX_PATH );

			for( ic = 0; iRoofCnt > ic; ic++ ){	vcString.at( iOfsLine ).append( atSubStr );	}

			if( (1 <= iRfRstDot) && bMultiPadd )
			{
				StringWidthAdjust( 0, atSubStr, MAX_PATH, iRfRstDot );
				vcString.at( iOfsLine ).append( atSubStr );
			}

			if( 0 >= (iUpLine - iTgtLn) - pstInfo->stAfternoon.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stAfternoon), atSubStr, MAX_PATH );
			vcString.at( iOfsLine ).append( atSubStr );
		}
	}
	else if( 2 == dType )
	{
		FrameMultiSizeGet( pstInfo, NULL, &iDnLine );

		pstInfo->stTwilight.iNowLn  = 0;
		pstInfo->stMidnight.iNowLn  = 0;
		pstInfo->stDawn.iNowLn      = 0;

		for( i = 0; iDnLine > i; i++ ){	vcString.push_back( wsWorkStr );	}

		for( iTgtLn = 0, iOfsLine = 0; iDnLine > iTgtLn; iTgtLn++, iOfsLine++ )
		{

			if( iTgtLn < pstInfo->stDawn.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stDawn), atSubStr, MAX_PATH );
			if( 1 <= pstInfo->dLeftOffset ){	StringWidthAdjust( pstInfo->dLeftOffset, atSubStr, MAX_PATH, 0 );	}
			vcString.at( iOfsLine ).append( atSubStr );

			if( iTgtLn < pstInfo->stMidnight.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stMidnight), atSubStr, MAX_PATH );

			for( ic = 0; iFloorCnt > ic; ic++ ){	vcString.at( iOfsLine ).append( atSubStr );	}

			if( (1 <= iFlRstDot) && bMultiPadd )
			{
				StringWidthAdjust( 0, atSubStr, MAX_PATH, iFlRstDot );
				vcString.at( iOfsLine ).append( atSubStr );
			}

			if( iTgtLn < pstInfo->stTwilight.iLine )
			{
				FrameMultiSubstring( pstInfo->stTwilight.atParts, pstInfo->stTwilight.iNowLn, atSubStr, MAX_PATH, 0 );
				pstInfo->stTwilight.iNowLn++;
				vcString.at( iOfsLine ).append( atSubStr );
			}
		}

	}
	else{	return NULL;	}

	cchTotal = 0;
	dCount = vcString.size();

	for( d = 0; dCount > d; d++ ){	cchTotal += vcString.at( d ).size();	}
	cchTotal += ((dCount+1) * sizeof(TCHAR));
	ptBufStr = (LPTSTR)malloc( cchTotal * sizeof(TCHAR) );
	if( ptBufStr )
	{
		ZeroMemory( ptBufStr, cchTotal * sizeof(TCHAR) );
		for( d = 0; dCount > d; d++ )
		{
			hRslt = StringCchCat( ptBufStr, cchTotal, vcString.at( d ).c_str() );
			hRslt = StringCchCat( ptBufStr, cchTotal, TEXT("\r\n") );

		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (LPTSTR)ETC_MSG( err.what( ), NULL );	}
	catch( ... ){	return (LPTSTR)ETC_MSG( ("etc error"), NULL );	}
#endif

	return ptBufStr;
}

INT_PTR Frm_OnNotify( HWND hDlg, INT idFrom, LPNMHDR pstNmhdr )
{
	INT	nmCode;
	TCHAR	atBuff[MIN_STRING];
	LPNMUPDOWN	pstUpDown;
	RECT	rect;

	nmCode = pstNmhdr->code;

	if( IDSP_LEFT_OFFSET == idFrom )
	{
		pstUpDown = (LPNMUPDOWN)pstNmhdr;
		if( UDN_DELTAPOS == nmCode )
		{
			TRACE( TEXT("%d %d"), pstUpDown->iPos, pstUpDown->iDelta );
			if( 0 < pstUpDown->iDelta ){		gstFrameInfo[gNowSel].dLeftOffset +=  1;	}
			else if( 0 >pstUpDown->iDelta ){	gstFrameInfo[gNowSel].dLeftOffset -=  1;	}

			StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), gstFrameInfo[gNowSel].dLeftOffset );
			Edit_SetText( GetDlgItem(hDlg,IDE_LEFT_OFFSET), atBuff );

			GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
			FREE( gptFrmSample );
			gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

			InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );
		}
		return (INT_PTR)TRUE;
	}

	if( IDSP_RIGHT_OFFSET == idFrom )
	{
		pstUpDown = (LPNMUPDOWN)pstNmhdr;
		if( UDN_DELTAPOS == nmCode )
		{
			TRACE( TEXT("%d %d"), pstUpDown->iPos, pstUpDown->iDelta );
			if( 0 < pstUpDown->iDelta ){		gstFrameInfo[gNowSel].dRightOffset -= 1;	}
			else if( 0 > pstUpDown->iDelta ){	gstFrameInfo[gNowSel].dRightOffset += 1;	}

			StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), gstFrameInfo[gNowSel].dRightOffset );
			Edit_SetText( GetDlgItem(hDlg,IDE_RIGHT_OFFSET), atBuff );

			GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
			FREE( gptFrmSample );
			gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

			InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );
		}
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}

HRESULT FrameDataGet( UINT dNumber, LPFRAMEINFO pstFrame )
{
	InitFrameItem( INIT_LOAD, dNumber, pstFrame );

	pstFrame->stDaybreak.iLine  = DocStringInfoCount( pstFrame->stDaybreak.atParts,  0, &(pstFrame->stDaybreak.dDot), NULL );
	pstFrame->stMorning.iLine   = DocStringInfoCount( pstFrame->stMorning.atParts,   0, &(pstFrame->stMorning.dDot), NULL );
	pstFrame->stNoon.iLine      = DocStringInfoCount( pstFrame->stNoon.atParts,      0, &(pstFrame->stNoon.dDot), NULL );
	pstFrame->stAfternoon.iLine = DocStringInfoCount( pstFrame->stAfternoon.atParts, 0, &(pstFrame->stAfternoon.dDot), NULL );
	pstFrame->stNightfall.iLine = DocStringInfoCount( pstFrame->stNightfall.atParts, 0, &(pstFrame->stNightfall.dDot), NULL );
	pstFrame->stTwilight.iLine  = DocStringInfoCount( pstFrame->stTwilight.atParts,  0, &(pstFrame->stTwilight.dDot), NULL );
	pstFrame->stMidnight.iLine  = DocStringInfoCount( pstFrame->stMidnight.atParts,  0, &(pstFrame->stMidnight.dDot), NULL );
	pstFrame->stDawn.iLine      = DocStringInfoCount( pstFrame->stDawn.atParts,      0, &(pstFrame->stDawn.dDot), NULL );

	return S_OK;
}

HRESULT FrameInfoDisp( HWND hDlg )
{
	TCHAR	atBuff[MIN_STRING];

	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_NAME_EDIT), gstFrameInfo[gNowSel].atFrameName );

	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_MORNING),   gstFrameInfo[gNowSel].stMorning.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_NOON),      gstFrameInfo[gNowSel].stNoon.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_AFTERNOON), gstFrameInfo[gNowSel].stAfternoon.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_DAYBREAK),  gstFrameInfo[gNowSel].stDaybreak.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_NIGHTFALL), gstFrameInfo[gNowSel].stNightfall.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_TWILIGHT),  gstFrameInfo[gNowSel].stTwilight.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_MIDNIGHT),  gstFrameInfo[gNowSel].stMidnight.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_DAWN),      gstFrameInfo[gNowSel].stDawn.atParts );

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"),   gstFrameInfo[gNowSel].dLeftOffset );
	Edit_SetText( GetDlgItem(hDlg,IDE_LEFT_OFFSET),    atBuff );

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"),   gstFrameInfo[gNowSel].dRightOffset );
	Edit_SetText( GetDlgItem(hDlg,IDE_RIGHT_OFFSET),   atBuff );

	Button_SetCheck( GetDlgItem( hDlg, IDB_FRM_PADDING ), gstFrameInfo[gNowSel].dRestPadd ? BST_CHECKED : BST_UNCHECKED );

	return S_OK;
}

HRESULT DocFrameInsert( INT dMode, INT dStyle )
{
	UINT_PTR	iLines;
	INT_PTR		iLns, iLast;
	INT			iTop, iBtm, iInX, iEndot, iPadding, i, baseDot;
	INT			xMidLen;
	LPTSTR		ptPadding;
	LPTSTR		ptString;

	FRAMEINFO	stInfo;

	INT			iMidLine, iUpLine, iDnLine;
	LPFRAMEITEM	pstItem;
	TCHAR		atSubStr[MAX_PATH];

#ifdef DO_TRY_CATCH
	try{
#endif

	FrameDataGet( dMode, &stInfo );

	iLines = DocNowFilePageLineCount( );

	iTop = gitFileIt->vcCont.at( gixFocusPage ).dSelLineTop;
	iBtm = gitFileIt->vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 >  iTop ){	iTop = 0;	}
	if( 0 >  iBtm ){	iBtm = iLines - 1;	}

	iInX = DocLineParamGet( iBtm, NULL, NULL );
	if( 0 == iInX ){	 iBtm--;	}

	ViewSelPageAll( -1 );

	baseDot = DocPageMaxDotGet( iTop, iBtm );

	iMidLine = (iBtm - iTop) + 1;

	xMidLen = baseDot;
	FrameMakeInsideBoundary( 0, &xMidLen, &stInfo );

	ptString = FrameMakeInsideBoundary( 1, &xMidLen, &stInfo );
	FrameMultiSizeGet( &stInfo, &iUpLine, NULL );
	iLns = iTop;
	iInX = 0;
	DocInsertString( &iInX, &iLns, NULL, ptString, 0, TRUE );
	FREE( ptString );

	stInfo.stDaybreak.iNowLn  = 0;
	stInfo.stNightfall.iNowLn = 0;
	for( i = 0; iMidLine > i; i++, iLns++ )
	{

		pstItem = &(stInfo.stDaybreak);

		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, atSubStr, MAX_PATH, pstItem->dDot );

		if( -1 >= stInfo.dLeftOffset ){	StringWidthAdjust( -(stInfo.dLeftOffset), atSubStr, MAX_PATH, 0 );	}
		pstItem->iNowLn++;
		if( pstItem->iLine <= pstItem->iNowLn ){	pstItem->iNowLn = 0;	}
		iInX = 0;
		DocInsertString( &iInX, &iLns, NULL, atSubStr, 0, FALSE );

		iEndot = DocLineParamGet( iLns, NULL, NULL );
		iPadding = xMidLen - iEndot;
		ptPadding = DocPaddingSpaceWithPeriod( iPadding, NULL, NULL, NULL, TRUE );
		if( ptPadding )
		{
			DocInsertString( &iEndot, &iLns, NULL, ptPadding, 0, FALSE );
			FREE( ptPadding );
		}
		pstItem = &(stInfo.stNightfall);
		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, atSubStr, MAX_PATH, 0 );
		pstItem->iNowLn++;
		if( pstItem->iLine <= pstItem->iNowLn ){	pstItem->iNowLn = 0;	}
		DocInsertString( &iEndot, &iLns, NULL, atSubStr, 0, FALSE );
	}

	iLast = DocPageParamGet( NULL, NULL );
	if( iLast <= iLns )
	{
		iLns = iLast - 1;
		iInX = DocLineParamGet( iLns, NULL, NULL );
		DocCrLfAdd( iInX , iLns, FALSE );
		iLns++;
	}

	ptString = FrameMakeInsideBoundary( 2, &xMidLen, &stInfo );
	FrameMultiSizeGet( &stInfo, NULL, &iDnLine );
	iInX = 0;
	DocInsertString( &iInX, &iLns, NULL, ptString, 0, FALSE );
	FREE( ptString );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif
#ifdef DO_TRY_CATCH
	try{
#endif

	ViewPosResetCaret( iInX , iLns );

	ViewRedrawSetLine( iTop );
	DocBadSpaceCheck( iTop );

	iLns = DocNowFilePageLineCount( );
	for( i = iTop; iLns > i; i++ )
	{
		DocBadSpaceCheck( i );
		ViewRedrawSetLine(  i );
	}

	DocPageInfoRenew( -1, 1 );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif
	return S_OK;
}

VOID FrameDataTranslate( LPTSTR ptData, UINT bMode )
{
	TCHAR	atBuffer[SUB_STRING];
	UINT_PTR	i, j, cchLen;

	ZeroMemory( atBuffer, sizeof(atBuffer) );

	StringCchLength( ptData, PARTS_CCH, &cchLen );

	for( i = 0, j = 0; cchLen > i; i++, j++ )
	{
		if( 0x0000 == ptData[i] )	break;

		if( bMode  )
		{
			if( 0x005C == ptData[i] )
			{
				i++;
				if( 'n' == ptData[i] )
				{
					atBuffer[j++] = 0x000D;
					atBuffer[j] = 0x000A;
				}
				else if( 's' == ptData[i] )
				{
					atBuffer[j] = 0x0020;
				}
				else
				{
					atBuffer[j] = ptData[i];
				}
			}
			else
			{
				atBuffer[j] = ptData[i];
			}
		}
		else
		{
			if( 0x005C == ptData[i] )
			{
				atBuffer[j++] = 0x005C;
				atBuffer[j] = 0x005C;
			}
			else if( 0x000D == ptData[i] )
			{
				atBuffer[j++] = 0x005C;
				atBuffer[j] = TEXT('n');
				i++;
			}
			else if( 0x0020 == ptData[i] )
			{
				atBuffer[j++] = 0x005C;
				atBuffer[j] = TEXT('s');
			}
			else
			{
				atBuffer[j] = ptData[i];
			}
		}
	}

	StringCchCopy( ptData, PARTS_CCH, atBuffer );

	return;
}

UINT FrameMultiSubstring( LPCTSTR ptSrc, CONST UINT dLine, LPTSTR ptDest, CONST UINT_PTR cchSz, CONST INT iUseDot )
{
	LPTSTR		ptPadding;
	INT			iPaDot, iStrDot;
	UINT_PTR	cchSrc, c, d;
	UINT		iLnCnt;

	StringCchLength( ptSrc, STRSAFE_MAX_CCH , &cchSrc );

	ZeroMemory( ptDest, cchSz * sizeof(TCHAR) );

	iLnCnt = 0;	d = 0;
	for( c = 0; cchSrc > c; c++ )
	{
		if( 0x000D == ptSrc[c] )
		{
			c++;
			iLnCnt++;
		}
		else
		{
			if( dLine == iLnCnt )
			{
				if( cchSz > d ){	ptDest[d] = ptSrc[c];	d++;	}
			}
		}
	}
	ptDest[(cchSz-1)] = NULL;

	iStrDot = ViewStringWidthGet( ptDest );

	iPaDot = iUseDot - iStrDot;
	if( 1 <= iPaDot )
	{
		ptPadding = DocPaddingSpaceWithPeriod( iPaDot, NULL, NULL, NULL, TRUE );
		StringCchCat( ptDest, cchSz, ptPadding );
		FREE( ptPadding );
	}

	iLnCnt++;
	return iLnCnt;
}

HWND FrameInsBoxCreate( HINSTANCE hInst, HWND hPrWnd )
{
	INT			x, y;
	UINT		d;
	TCHAR		atBuffer[MAX_STRING];
	RECT		rect, vwRect;

	RECT	stFrmRct;
	INT		topOst;

	if( ghFrInbxWnd )
	{
		SetForegroundWindow( ghFrInbxWnd );
		return ghFrInbxWnd;
	}

	ghFrInbxWnd = CreateWindowEx( WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		FRAMEINSERTBOX_CLASS, TEXT("枠挿入ボックス"),
		WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU,
		0, 0, FIB_WIDTH, FIB_HEIGHT, NULL, NULL, hInst, NULL );
	SetLayeredWindowAttributes( ghFrInbxWnd, TRANCE_COLOUR, 0xFF, LWA_COLORKEY );

	ghFIBtlbrWnd = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("fibtoolbar"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,
		0, 0, 0, 0, ghFrInbxWnd, (HMENU)IDTB_FRMINSBOX_TOOLBAR, hInst, NULL );

	if( 0 == gdToolBarHei )
	{
		GetWindowRect( ghFIBtlbrWnd, &rect );
		gdToolBarHei = rect.bottom - rect.top;

		gstFrmSz.x = 0;
		gstFrmSz.y = gdToolBarHei;
		ClientToScreen( ghFrInbxWnd, &gstFrmSz );
	}

	SendMessage( ghFIBtlbrWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	SendMessage( ghFIBtlbrWnd, TB_SETIMAGELIST, 0, (LPARAM)ghFrameImgLst );
	SendMessage( ghFIBtlbrWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );
	SendMessage( ghFIBtlbrWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuffer, MAX_STRING, TEXT("挿入する") );
	gstFIBTBInfo[0].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	for( d = 0; FRAME_MAX > d; d++ )
	{
		FrameNameLoad( d, atBuffer, MAX_STRING );
		gstFIBTBInfo[d+2].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	}
	StringCchCopy( atBuffer, MAX_STRING, TEXT("挿入したら閉じる") );
	gstFIBTBInfo[23].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("上下の端数を埋める\r\n（ここでの変更は保存されません）") );
	gstFIBTBInfo[25].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghFIBtlbrWnd , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstFIBTBInfo );

	SendMessage( ghFIBtlbrWnd , TB_AUTOSIZE, 0, 0 );
	InvalidateRect( ghFIBtlbrWnd , NULL, TRUE );

	SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_INSFRAME_ALPHA, TRUE );
	gdSelect = 0;

	SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_FRMINSBOX_QCLOSE, gbQuickClose );

	FrameDataGet( gdSelect , &gstNowFrameInfo );

	SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_FRMINSBOX_PADDING, gstNowFrameInfo.dRestPadd );
	gbMultiPaddTemp = gstNowFrameInfo.dRestPadd;

	topOst = FrameInsBoxSizeGet( &stFrmRct );
	gptFrmBox = FrameMakeOutsideBoundary( stFrmRct.right, stFrmRct.bottom, &gstNowFrameInfo );

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;
	gstViewOrigin.y = vwRect.top;
	x = (vwRect.left + LINENUM_WID) - gstFrmSz.x;
	y = (vwRect.top  + RULER_AREA)  - gstFrmSz.y;
	TRACE( TEXT("Frame %d x %d"), x, y );

#ifdef _DEBUG
	SetWindowPos( ghFrInbxWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#else
	SetWindowPos( ghFrInbxWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#endif
	gstOffset.x = x - vwRect.left;
	gstOffset.y = y - vwRect.top;

	return ghFrInbxWnd;
}

INT FrameInsBoxSizeGet( LPRECT pstRect )
{
	RECT	rect;

	GetClientRect( ghFrInbxWnd, &rect );

	rect.bottom -= gdToolBarHei;

	*pstRect = rect;

	return gdToolBarHei;
}

HRESULT FrameInsBoxDoInsert( HWND hWnd )
{
	INT			iX, iY;
	HWND		hLyrWnd;
	RECT		rect;

	hLyrWnd = LayerBoxVisibalise( GetModuleHandle(NULL), gptFrmBox, 0x10 );

	GetWindowRect( hWnd, &rect );
	LayerBoxPositionChange( hLyrWnd, (rect.left + gstFrmSz.x), (rect.top + gstFrmSz.y) );

	LayerTransparentToggle( hLyrWnd, 1 );

	LayerContentsImportable( hLyrWnd, IDM_LYB_OVERRIDE, &iX, &iY, D_INVISI );
	ViewPosResetCaret( iX, iY );

	DestroyWindow( hLyrWnd );

	if( gbQuickClose )	DestroyWindow( hWnd );

	return S_OK;
}

LRESULT CALLBACK FrameInsProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT,		Fib_OnPaint );
		HANDLE_MSG( hWnd, WM_KEYDOWN,	Fib_OnKey );
		HANDLE_MSG( hWnd, WM_COMMAND,	Fib_OnCommand );
		HANDLE_MSG( hWnd, WM_DESTROY,	Fib_OnDestroy );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING, Fib_OnWindowPosChanging );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGED,  Fib_OnWindowPosChanged );

		case WM_MOVING:	Fib_OnMoving( hWnd, (LPRECT)lParam );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Fib_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	RECT	stFrmRct;
	INT		topOst, iRslt = -1;

	switch( id )
	{
		case IDM_FRAME_INS_DECIDE:	FrameInsBoxDoInsert( hWnd );	return;

		case IDM_INSFRAME_ALPHA:	gdSelect = 0;	break;
		case IDM_INSFRAME_BRAVO:	gdSelect = 1;	break;
		case IDM_INSFRAME_CHARLIE:	gdSelect = 2;	break;
		case IDM_INSFRAME_DELTA:	gdSelect = 3;	break;
		case IDM_INSFRAME_ECHO:		gdSelect = 4;	break;
		case IDM_INSFRAME_FOXTROT:	gdSelect = 5;	break;
		case IDM_INSFRAME_GOLF:		gdSelect = 6;	break;
		case IDM_INSFRAME_HOTEL:	gdSelect = 7;	break;
		case IDM_INSFRAME_INDIA:	gdSelect = 8;	break;
		case IDM_INSFRAME_JULIETTE:	gdSelect = 9;	break;

		case IDM_INSFRAME_KILO:		gdSelect = 10;	break;
		case IDM_INSFRAME_LIMA:		gdSelect = 11;	break;
		case IDM_INSFRAME_MIKE:		gdSelect = 12;	break;
		case IDM_INSFRAME_NOVEMBER:	gdSelect = 13;	break;
		case IDM_INSFRAME_OSCAR:	gdSelect = 14;	break;
		case IDM_INSFRAME_PAPA:		gdSelect = 15;	break;
		case IDM_INSFRAME_QUEBEC:	gdSelect = 16;	break;
		case IDM_INSFRAME_ROMEO:	gdSelect = 17;	break;
		case IDM_INSFRAME_SIERRA:	gdSelect = 18;	break;
		case IDM_INSFRAME_TANGO:	gdSelect = 19;	break;

		case IDM_FRMINSBOX_QCLOSE:	gbQuickClose = SendMessage( ghFIBtlbrWnd, TB_ISBUTTONCHECKED, IDM_FRMINSBOX_QCLOSE, 0 );	return;

		case IDM_FRMINSBOX_PADDING:	iRslt = SendMessage( ghFIBtlbrWnd, TB_ISBUTTONCHECKED, IDM_FRMINSBOX_PADDING, 0 );	break;

		default:	return;
	}

	FrameDataGet( gdSelect, &gstNowFrameInfo );

	if( 0 <= iRslt ){	gstNowFrameInfo.dRestPadd = iRslt;	gbMultiPaddTemp = iRslt;	}
	else
	{
		gbMultiPaddTemp = gstNowFrameInfo.dRestPadd;
		SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_FRMINSBOX_PADDING, gstNowFrameInfo.dRestPadd );
	}

	topOst = FrameInsBoxSizeGet( &stFrmRct );
	FREE( gptFrmBox );
	gptFrmBox = FrameMakeOutsideBoundary( stFrmRct.right, stFrmRct.bottom, &gstNowFrameInfo );

	InvalidateRect( hWnd, NULL, TRUE );

	return;
}

VOID Fib_OnKey( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
{
	RECT	rect;

	GetWindowRect( hWnd, &rect );

	if( fDown )
	{
		switch( vk )
		{
			case VK_RIGHT:	TRACE( TEXT("右") );	rect.left++;	break;
			case VK_LEFT:	TRACE( TEXT("左") );	rect.left--;	break;
			case VK_DOWN:	TRACE( TEXT("下") );	rect.top += LINE_HEIGHT;	break;
			case  VK_UP:	TRACE( TEXT("上") );	rect.top -= LINE_HEIGHT;	break;
			default:	return;
		}
	}

	SetWindowPos( hWnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	Fib_OnMoving( hWnd, &rect );

	return;
}

VOID Fib_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	RECT	rect;

	GetClientRect( hWnd, &rect );

	hdc = BeginPaint( hWnd, &ps );

	FillRect( hdc, &rect, ghBgBrush );

	FrameInsBoxFrmDraw( hdc );

	EndPaint( hWnd, &ps );

	return;
}

VOID FrameInsBoxFrmDraw( HDC hDC )
{

	HFONT	hOldFnt;
	INT		topOst, iYpos;
	RECT	stFrmRct;

	UINT	dLines, d;

	TCHAR	atBuffer[MAX_PATH];

	SetBkColor( hDC, ViewBackColourGet( NULL ) );

	hOldFnt = SelectFont( hDC, ghAaFont );

	topOst = FrameInsBoxSizeGet( &stFrmRct );

	dLines = DocStringInfoCount( gptFrmBox, 0, NULL, NULL );

	iYpos = topOst;
	for( d = 0; dLines > d; d++ )
	{
		FrameMultiSubstring( gptFrmBox, d, atBuffer, MAX_PATH, 0 );
		FrameDrawItem( hDC, iYpos, atBuffer );
		iYpos += LINE_HEIGHT;
	}

	SelectFont( hDC , hOldFnt );

	return;
}

VOID FrameDrawItem( HDC hDC, INT iY, LPTSTR ptLine )
{
	UINT_PTR	cchSize, cl;
	UINT		iX, caret, len;
	INT			mRslt, mBase;
	LPTSTR		ptBgn;
	SIZE		stSize;

	StringCchLength( ptLine, STRSAFE_MAX_CCH, &cchSize );

	caret = 0;
	iX = 0;
	for( cl = 0; cchSize > cl; )
	{
		mRslt = iswspace(  ptLine[cl] );
		ptBgn = &(ptLine[cl]);

		for( len= 0; cchSize > cl; len++, cl++ )
		{
			mBase = iswspace(  ptBgn[len] );
			if( mRslt != mBase ){	break;	}
		}
		GetTextExtentPoint32( hDC, ptBgn, len, &stSize );

		if( mRslt ){	SetBkMode( hDC, TRANSPARENT );	}
		else{	SetBkMode( hDC, OPAQUE );	}

		ExtTextOut( hDC, iX, iY, 0, NULL, ptBgn, len, NULL );

		iX += stSize.cx;
	}

	return;
}

VOID Fib_OnDestroy( HWND hWnd )
{
	FREE( gptFrmBox );

	MainStatusBarSetText( SB_LAYER, TEXT("") );

	ghFrInbxWnd = NULL;

	return;
}

VOID Fib_OnMoving( HWND hWnd, LPRECT pstPos )
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

	StringCchPrintf( atBuffer, SUB_STRING, TEXT("Frame %d[dot] %d[line]"), xSb, dLine );
	MainStatusBarSetText( SB_LAYER, atBuffer );

	return;
}

BOOL Fib_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
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

VOID Fib_OnWindowPosChanged( HWND hWnd, const LPWINDOWPOS pstWpos )
{
	RECT	vwRect;
	RECT	stFrmRct;
	INT		topOst;

	MoveWindow( ghFIBtlbrWnd, 0, 0, 0, 0, TRUE );

	FrameDataGet( gdSelect, &gstNowFrameInfo );
	gstNowFrameInfo.dRestPadd = gbMultiPaddTemp;

	topOst = FrameInsBoxSizeGet( &stFrmRct );
	FREE( gptFrmBox );
	gptFrmBox = FrameMakeOutsideBoundary( stFrmRct.right, stFrmRct.bottom, &gstNowFrameInfo );

	InvalidateRect( hWnd, NULL, TRUE );

	if( SWP_NOMOVE & pstWpos->flags )	return;

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;
	gstViewOrigin.y = vwRect.top;

	gstOffset.x = pstWpos->x - vwRect.left;
	gstOffset.y = pstWpos->y - vwRect.top;

	return;
}

HRESULT FrameMoveFromView( HWND hWnd, UINT state )
{
	RECT	vwRect = {0,0,0,0};
	POINT	lyPoint;

	if( !(ghFrInbxWnd) )	return S_FALSE;

	if( SIZE_MINIMIZED != state )
	{
		GetWindowRect( ghViewWnd, &vwRect );
		gstViewOrigin.x = vwRect.left;
		gstViewOrigin.y = vwRect.top;
	}

	if( SIZE_MINIMIZED == state )
	{
		ShowWindow( ghFrInbxWnd, SW_HIDE );
	}
	else
	{
		lyPoint.x = gstOffset.x + vwRect.left;
		lyPoint.y = gstOffset.y + vwRect.top;
#ifdef _DEBUG
		SetWindowPos( ghFrInbxWnd, HWND_TOP, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#else
		SetWindowPos( ghFrInbxWnd, HWND_TOPMOST, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#endif
	}

	return S_OK;
}
