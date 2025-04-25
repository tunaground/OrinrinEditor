#include "stdafx.h"
#include "OrinrinEditor.h"

typedef struct tagLAYERBOXSTRUCT
{
	LONG	id;

	POINT	stOffset;

	HWND	hBoxWnd;

	HWND	hTextWnd;

	HWND	hToolWnd;

	vector<ONELINE>	vcLyrImg;

} LAYERBOXSTRUCT, *LPLAYERBOXSTRUCT;

typedef list<LAYERBOXSTRUCT>::iterator	LAYER_ITR;
typedef vector<ONELINE>::iterator		LYLINE_ITR;

#define LAYERBOX_CLASS	TEXT("LAYER_BOX")
#define	LB_WIDTH	310
#define LB_HEIGHT	220

#define EDGE_BLANK_NARROW	16
#define EDGE_BLANK_WIDE		22

#define TB_ITEMS	8
static  TBBUTTON	gstTBInfo[] = {
	{ 0,	IDM_LYB_INSERT,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{ 1,	IDM_LYB_OVERRIDE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{ 0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 2,	IDM_LYB_COPY,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{ 0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 3,	IDM_LYB_DO_EDIT,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 4,	IDM_LYB_DELETE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  }

};

extern INT		gdDocXdot;
extern INT		gdDocLine;

extern INT		gdHideXdot;
extern INT		gdViewTopLine;

extern HFONT	ghAaFont;

extern  HWND	ghViewWnd;

static POINT	gstViewOrigin;

static  ATOM	gLyrBoxAtom;

static  LONG	gdBoxID;

static POINT	gstFrmSz;
static INT		gdToolBarHei;

EXTERNED BYTE	gbAlpha;

static BOOLEAN	gbQuickClose;

static WNDPROC	gpfOrigLyrTBProc;
static WNDPROC	gpfOrigLyrEditProc;

static HIMAGELIST	ghLayerImgLst;

static  list<LAYERBOXSTRUCT>	gltLayer;

static LRESULT	CALLBACK gpfLayerTBProc( HWND, UINT, WPARAM, LPARAM );
static LRESULT	CALLBACK gpfLyrEditProc( HWND, UINT, WPARAM, LPARAM );

LRESULT	CALLBACK LayerBoxProc( HWND, UINT, WPARAM, LPARAM );

BOOLEAN	Lyb_OnCreate( HWND, LPCREATESTRUCT );
VOID	Lyb_OnCommand( HWND , INT, HWND, UINT );

VOID	Lyb_OnKey( HWND, UINT, BOOL, INT, UINT );
VOID	Lyb_OnPaint( HWND );
VOID	Lyb_OnDestroy( HWND );
VOID	Lyb_OnMoving( HWND, LPRECT );
BOOL	Lyb_OnWindowPosChanging( HWND, LPWINDOWPOS );
VOID	Lyb_OnWindowPosChanged( HWND, const LPWINDOWPOS );
VOID	Lyb_OnLButtonDown( HWND, BOOL, INT, INT, UINT );
VOID	Lyb_OnContextMenu( HWND, HWND, UINT, UINT );

HRESULT	LayerEditOnOff( HWND, UINT );

HRESULT	LayerStringObliterate( LAYER_ITR  );
HRESULT	LayerFromString( LAYER_ITR, LPCTSTR );
HRESULT	LayerFromSelectArea( LAYER_ITR , UINT );
HRESULT	LayerFromClipboard( LAYER_ITR );
HRESULT	LayerForClipboard( HWND, UINT );
HRESULT	LayerOnDelete( HWND );
INT		LayerInputLetter( LAYER_ITR, INT, INT, TCHAR );
LPTSTR	LayerLineTextGetAlloc( LAYER_ITR, INT );
HRESULT	LayerBoxSetString( LAYER_ITR, LPCTSTR, UINT, LPPOINT, UINT );
HRESULT	LayerBoxSizeAdjust( LAYER_ITR );

INT		LayerTransparentAdjust( LAYER_ITR, INT, INT );

#ifdef EDGE_BLANK_STYLE
HRESULT	LayerEdgeBlankSizeCheck( HWND, INT );
#endif

VOID LayerBoxInitialise( HINSTANCE hInstance, LPRECT pstFrame )
{
	WNDCLASSEX	wcex;
	HBITMAP	hImg, hMsq;

	if( !(hInstance) )
	{
		ImageList_Destroy( ghLayerImgLst );

		return;
	}

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= LayerBoxProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_MENU+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= LAYERBOX_CLASS;
	wcex.hIconSm		= NULL;

	gLyrBoxAtom = RegisterClassEx( &wcex );

	gbQuickClose = TRUE;

	gdBoxID = 0;

	ghLayerImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 5, 1 );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_LAYERINSERT) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_LAYERINSERT) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_LAAYEROVERWRITE) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_LAAYEROVERWRITE) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_COPY) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_COPY) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_LAYERTEXTEDIT) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_LAYERTEXTEDIT) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_DELETE) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_DELETE) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	return;
}

HRESULT LayerBoxAlphaSet( UINT dParam )
{
	gbAlpha = dParam & 0xFF;

	return S_OK;
}

HWND LayerBoxVisibalise( HINSTANCE hInst, LPCTSTR ptStr, UINT bNormal )
{
	INT		x, y;
	RECT	vwRect, rect;
	DWORD	dwStyle;

	BOOLEAN	bSelect = FALSE;
	UINT	bSqSel = 0;

	LAYERBOXSTRUCT	stLayer;
	LAYER_ITR	itLyr;

	stLayer.id = gdBoxID;

	bSelect = IsSelecting( &bSqSel );

	stLayer.vcLyrImg.clear( );

	if( 0x10 & bNormal ){	dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU;	}
	else{		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;	}

	stLayer.hBoxWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_LAYERED, LAYERBOX_CLASS,
		TEXT("レイヤ"), dwStyle, 0, 0, LB_WIDTH, LB_HEIGHT, NULL, NULL, hInst, NULL);

	WndTagSet( stLayer.hBoxWnd, stLayer.id );

	SetLayeredWindowAttributes( stLayer.hBoxWnd, 0, gbAlpha, LWA_ALPHA );

	stLayer.hToolWnd = GetDlgItem( stLayer.hBoxWnd, IDW_LYB_TOOL_BAR );

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;
	gstViewOrigin.y = vwRect.top;

	x = gdDocXdot;
	y = gdDocLine * LINE_HEIGHT;
	ViewPositionTransform( &x, &y, TRUE );
	x += (vwRect.left - gstFrmSz.x);
	y += (vwRect.top  - gstFrmSz.y);
	TRACE( TEXT("%d x %d"), x, y );

#ifdef _DEBUG
	SetWindowPos( stLayer.hBoxWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE );
#else
	SetWindowPos( stLayer.hBoxWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE );
#endif
	stLayer.stOffset.x = x - vwRect.left;
	stLayer.stOffset.y = y - vwRect.top;

	GetClientRect( stLayer.hBoxWnd, &rect );

	stLayer.hTextWnd = CreateWindowEx( 0, WC_EDIT, TEXT(""),
		WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
		0, gdToolBarHei, rect.right, rect.bottom - gdToolBarHei,
		stLayer.hBoxWnd, (HMENU)IDE_LYB_TEXTEDIT, hInst, NULL );
	SetWindowFont( stLayer.hTextWnd, ghAaFont, TRUE );

	gpfOrigLyrEditProc = SubclassWindow( stLayer.hTextWnd, gpfLyrEditProc );

	gltLayer.push_back( stLayer );
	itLyr = gltLayer.end();
	itLyr--;

	if( ptStr  )
	{
		TRACE( TEXT("LAYER from STRING") );
		LayerFromString( itLyr, ptStr );
	}
	else if( bSelect )
	{
		TRACE( TEXT("LAYER from Select") );
		LayerFromSelectArea( itLyr, bSqSel );
	}
	else
	{
		TRACE( TEXT("LAYER from ClipBoard") );
		LayerFromClipboard( itLyr );
	}

	if( !(0x10 & bNormal) )
	{
		ShowWindow( stLayer.hBoxWnd, SW_SHOW );
		UpdateWindow( stLayer.hBoxWnd );

		GetWindowRect( stLayer.hBoxWnd, &rect );
		Lyb_OnMoving( stLayer.hBoxWnd, &rect );
	}

	gdBoxID++;

	return stLayer.hBoxWnd;
}

HRESULT LayerBoxPositionChange( HWND hWnd, LONG x, LONG y )
{
	LAYER_ITR	itLyr;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			x -= gstFrmSz.x;
			y -= gstFrmSz.y;
			SetWindowPos( hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
		}
	}

	return S_OK;
}

LRESULT CALLBACK gpfLayerTBProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT		itemID;
	HDC		hdc;
	HWND	hWndChild;

	switch( msg )
	{
		case WM_CTLCOLORSTATIC:
			hdc = (HDC)(wParam);
			hWndChild = (HWND)(lParam);

			itemID = GetDlgCtrlID( hWndChild );

			if( IDCB_LAYER_QUICKCLOSE == itemID || IDCB_LAYER_EDGE_BLANK == itemID )
			{
				SetBkColor( hdc, GetSysColor( COLOR_WINDOW ) );
				return (LRESULT)GetSysColorBrush( COLOR_WINDOW );
			}
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if( SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0 ){	ReleaseCapture(   );	}
			return 0;
	}

	return CallWindowProc( gpfOrigLyrTBProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK gpfLyrEditProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
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
			TRACE( TEXT("[%X]LyrEdit COMMAND %d"), hWnd, id );

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

	return CallWindowProc( gpfOrigLyrEditProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK LayerBoxProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_CREATE,			Lyb_OnCreate );
		HANDLE_MSG( hWnd, WM_COMMAND,			Lyb_OnCommand );
		HANDLE_MSG( hWnd, WM_PAINT,				Lyb_OnPaint );
		HANDLE_MSG( hWnd, WM_DESTROY,			Lyb_OnDestroy );
		HANDLE_MSG( hWnd, WM_KEYDOWN,			Lyb_OnKey );
		HANDLE_MSG( hWnd, WM_LBUTTONDBLCLK,		Lyb_OnLButtonDown );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU,		Lyb_OnContextMenu );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING,	Lyb_OnWindowPosChanging );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGED,	Lyb_OnWindowPosChanged );

		case WM_MOVING:	Lyb_OnMoving( hWnd, (LPRECT)lParam );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

BOOLEAN Lyb_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	HINSTANCE	lcInst  = lpCreateStruct->hInstance;
	HWND	hToolWnd, hWorkWnd;
	TCHAR	atBuffer[MAX_STRING];

	RECT	tbRect;

	hToolWnd = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("toolbar"), WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, hWnd, (HMENU)IDW_LYB_TOOL_BAR, lcInst, NULL );

	SendMessage( hToolWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	SendMessage( hToolWnd, TB_SETIMAGELIST, 0, (LPARAM)ghLayerImgLst );

	SendMessage( hToolWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuffer, MAX_STRING, TEXT("この辺に挿入") );	gstTBInfo[0].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("ここらに上書") );	gstTBInfo[1].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("コピーする") );		gstTBInfo[3].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("テキスト編集") );	gstTBInfo[5].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("内容を削除") );		gstTBInfo[7].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( hToolWnd , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstTBInfo );

	SendMessage( hToolWnd , TB_AUTOSIZE, 0, 0 );
	InvalidateRect( hToolWnd , NULL, TRUE );

	gpfOrigLyrTBProc = SubclassWindow( hToolWnd, gpfLayerTBProc );

	CreateWindowEx( 0, WC_BUTTON, TEXT("貼付たら閉じる"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 150, 2, 138, 23, hToolWnd, (HMENU)IDCB_LAYER_QUICKCLOSE, lcInst, NULL );
	CheckDlgButton( hToolWnd, IDCB_LAYER_QUICKCLOSE, gbQuickClose ? BST_CHECKED : BST_UNCHECKED );

#ifdef EDGE_BLANK_STYLE
	hWorkWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT(""), WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 290, 0, 123, 70, hToolWnd, (HMENU)IDCB_LAYER_EDGE_BLANK, lcInst, NULL );
	ComboBox_AddString( hWorkWnd, TEXT("白抜きしない") );
	ComboBox_AddString( hWorkWnd, TEXT("狭く白抜き") );
	ComboBox_AddString( hWorkWnd, TEXT("広く白抜き") );
	ComboBox_SetCurSel( hWorkWnd, 0 );
#endif

	if( 0 == gdBoxID )
	{

		GetWindowRect( hToolWnd, &tbRect );
		gdToolBarHei = tbRect.bottom - tbRect.top;

		gstFrmSz.x = 0;
		gstFrmSz.y = gdToolBarHei;
		ClientToScreen( hWnd, &gstFrmSz );
		TRACE( TEXT("%d x %d"), gstFrmSz.x, gstFrmSz.y );
	}

	return TRUE;
}

VOID Lyb_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	LRESULT	lRslt;
	INT		bEdgeBlank;
	INT		iXpos, iYln;

	switch( id )
	{
		case IDE_LYB_TEXTEDIT:
			if( EN_SETFOCUS  == codeNotify ){	TRACE( TEXT("LYREDIT_SETFOCUS") );	}

			if( EN_KILLFOCUS == codeNotify )
			{
				TRACE( TEXT("LYREDIT_KILLFOCUS") );
				ViewFocusSet(  );
			}
			break;

		case IDM_LYB_INSERT:
		case IDM_LYB_OVERRIDE:
			LayerContentsImportable( hWnd, id, &iXpos, &iYln, 0 );
			ViewPosResetCaret( iXpos, iYln );
			DocPageInfoRenew( -1, 1 );
			if( gbQuickClose  ){	DestroyWindow( hWnd );	}
			break;

		case IDM_LYB_COPY:
			LayerForClipboard( hWnd, D_UNI );
			break;

		case IDM_LYB_DO_EDIT:
			lRslt = SendMessage( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), TB_GETSTATE, IDM_LYB_DO_EDIT, 0 );
			LayerEditOnOff( hWnd, (lRslt&TBSTATE_CHECKED) ? TRUE : FALSE );
			SendMessage( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), TB_SETSTATE, IDM_LYB_INSERT,   (lRslt&TBSTATE_CHECKED) ? 0 : TBSTATE_ENABLED );
			SendMessage( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), TB_SETSTATE, IDM_LYB_OVERRIDE, (lRslt&TBSTATE_CHECKED) ? 0 : TBSTATE_ENABLED );
			break;

		case IDM_LYB_DELETE:
			LayerOnDelete( hWnd );
			break;

		case IDCB_LAYER_QUICKCLOSE:
			gbQuickClose = IsDlgButtonChecked( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), IDCB_LAYER_QUICKCLOSE ) ? TRUE : FALSE;
			SetFocus( hWnd );
			break;

#ifdef EDGE_BLANK_STYLE
		case IDCB_LAYER_EDGE_BLANK:
			if( CBN_SELCHANGE == codeNotify )
			{
				bEdgeBlank = ComboBox_GetCurSel( hWndCtl );
				if( 1 == bEdgeBlank ){			LayerEdgeBlankSizeCheck( hWnd, EDGE_BLANK_NARROW );	}
				else if( 2 ==  bEdgeBlank ){	LayerEdgeBlankSizeCheck( hWnd, EDGE_BLANK_WIDE );	}
			}
			break;
#endif

		case IDM_LYB_TRANCE_RELEASE:
			LayerTransparentToggle( hWnd, 0 );
			InvalidateRect( hWnd, NULL, TRUE );
			break;

		case IDM_LYB_TRANCE_ALL:
			LayerTransparentToggle( hWnd, 1 );
			InvalidateRect( hWnd, NULL, TRUE );
			break;

		default:	TRACE( TEXT("Layer未知のコマンド %d"), id );	break;
	}

	return;
}

VOID Lyb_OnKey( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
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
	Lyb_OnMoving( hWnd, &rect );

	return;
}

VOID Lyb_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HFONT		hFtOld;
	COLORREF	clrTextOld, clrBackOld;
	HDC			hdc;
	INT			height;
	UINT_PTR	iLines, i;
	LPTSTR		ptText;
	RECT		rect;
	LAYER_ITR	itLyr;

	UINT_PTR	mz, cchLen;
	UINT		bStyle, cchMr, cbSize;
	INT			width, rdStart;
	BOOLEAN		doDraw, bRslt;

	hdc = BeginPaint( hWnd, &ps );

#ifdef DO_TRY_CATCH
	try{
#endif

	height = gdToolBarHei;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			clrTextOld = SetTextColor( hdc, CLR_BLACK );
			clrBackOld = SetBkColor(   hdc, CLR_WHITE );

			GetClientRect( hWnd, &rect );
			FillRect( hdc, &rect, GetStockBrush( WHITE_BRUSH ) );

			hFtOld = SelectFont( hdc, ghAaFont );

			iLines = itLyr->vcLyrImg.size( );

			for( i = 0; iLines > i; i++ )
			{
				cchLen = itLyr->vcLyrImg.at( i ).vcLine.size(  );
				if( 0 >= cchLen ){	height += LINE_HEIGHT;	continue;	}

				cbSize = (cchLen+1) * sizeof(TCHAR);
				ptText = (LPTSTR)malloc( cbSize );
				ZeroMemory( ptText, cbSize );

				bStyle  = itLyr->vcLyrImg.at( i ).vcLine.at( 0 ).mzStyle;
				bStyle &= CT_LYR_TRNC;
				cchMr   = 0;
				width   = 0;
				rdStart = 0;
				doDraw  = FALSE;

				for( mz = 0; cchLen >= mz; mz++ )
				{
					if( cchLen ==  mz ){	doDraw = TRUE;	}
					else
					{

						if( bStyle == (itLyr->vcLyrImg.at( i ).vcLine.at( mz ).mzStyle & CT_LYR_TRNC) )
						{
							ptText[cchMr++] = itLyr->vcLyrImg.at( i ).vcLine.at( mz ).cchMozi;
							width += itLyr->vcLyrImg.at( i ).vcLine.at( mz ).rdWidth;
						}
						else{	doDraw = TRUE;	}
					}

					if( doDraw )
					{
						if( bStyle & CT_LYR_TRNC )
						{
							SetBkColor(   hdc, CLR_SILVER );

							SetRect( &rect, rdStart, height, rdStart + width, height + LINE_HEIGHT );
							FillRect( hdc, &rect, GetStockBrush( LTGRAY_BRUSH ) );
						}
						else
						{
							SetBkColor(   hdc, CLR_WHITE );
						}

						bRslt = ExtTextOut( hdc, rdStart, height, 0, NULL, ptText, cchMr, NULL );
						if( !(bRslt)  ){	TRACE( TEXT("ExtTextOut error") );	}

						if( cchLen != mz )
						{
							rdStart += width;

							bStyle  = itLyr->vcLyrImg.at( i ).vcLine.at( mz ).mzStyle;
							bStyle &= CT_LYR_TRNC;
							ZeroMemory( ptText, cbSize );
							ptText[0] = itLyr->vcLyrImg.at( i ).vcLine.at( mz ).cchMozi;
							width  = itLyr->vcLyrImg.at( i ).vcLine.at( mz ).rdWidth;
							cchMr  = 1;
						}
						doDraw = FALSE;
					}
				}

				FREE( ptText );

				height += LINE_HEIGHT;
			}

			SelectFont( hdc, hFtOld );

			break;
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	ETC_MSG( err.what(), 0 );	 return;	}
	catch( ... ){	ETC_MSG( ("etc error"), 0 );	 return;	}
#endif

	EndPaint( hWnd, &ps );

	return;
}

VOID Lyb_OnDestroy( HWND hWnd )
{
	LAYER_ITR	itLyr;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			LayerStringObliterate( itLyr );
			MainStatusBarSetText( SB_LAYER, TEXT("") );

			gltLayer.erase( itLyr );

			break;
		}
	}

	return;
}

BOOL Lyb_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
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

VOID Lyb_OnWindowPosChanged( HWND hWnd, const LPWINDOWPOS pstWpos )
{
	BOOLEAN	bHit = FALSE;
	LAYER_ITR	itLyr;
	RECT	vwRect, rect;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	bHit = TRUE;	break;	}
	}
	if( !(bHit) )	return;

	GetClientRect( hWnd, &rect );
	MoveWindow( itLyr->hToolWnd, 0, 0, 0, 0, TRUE );
	SetWindowPos( itLyr->hTextWnd, HWND_TOP, 0, 0, rect.right, rect.bottom - gdToolBarHei, SWP_NOMOVE | SWP_NOZORDER );

	if( SWP_NOMOVE & pstWpos->flags )	return;

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;
	gstViewOrigin.y = vwRect.top;

	itLyr->stOffset.x = pstWpos->x - vwRect.left;
	itLyr->stOffset.y = pstWpos->y - vwRect.top;

	return;
}

VOID Lyb_OnMoving( HWND hWnd, LPRECT pstPos )
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

	StringCchPrintf( atBuffer, SUB_STRING, TEXT("Layer %d[dot] %d[line]"), xSb, dLine );
	MainStatusBarSetText( SB_LAYER, atBuffer );

	return;
}

VOID Lyb_OnLButtonDown( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	INT			sy, iDot, iLine;
	LAYER_ITR	itLyr;
	RECT		rect;
	BOOLEAN		bGet = FALSE;

	iDot = x;
	sy = y - gdToolBarHei;	if( 0 > sy )	sy = 0;
	iLine = sy / LINE_HEIGHT;

	TRACE( TEXT("マウスボタンダウン[%d][%dx%d(%d)]"), fDoubleClick, iDot, sy, iLine );

	if( !(fDoubleClick) )	 return;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			bGet = TRUE;	break;
		}
	}
	if( !(bGet) )	 return;

	if( LayerTransparentAdjust( itLyr, iDot, iLine ) )
	{
		GetClientRect( hWnd, &rect );
		rect.top    = (iLine * LINE_HEIGHT) + gdToolBarHei;
		rect.bottom = rect.top + LINE_HEIGHT;
		InvalidateRect( hWnd, &rect, TRUE );
	}

	return;
}

VOID Lyb_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	INT		posX, posY;
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;

	posX = (SHORT)xPos;
	posY = (SHORT)yPos;

	TRACE( TEXT("LAYER_WM_CONTEXTMENU %d x %d"), posX, posY );

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_LAYERBOX_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	dRslt = TrackPopupMenu( hSubMenu, 0, posX, posY, 0, hWnd, NULL );
	DestroyMenu( hMenu );

	return;
}

HRESULT LayerTransparentToggle( HWND hWnd, UINT bMode )
{
	TCHAR	chb;
	INT_PTR	iLines, iL;
	LETR_ITR	itMozi;
	LAYER_ITR	itLyr;

#ifdef DO_TRY_CATCH
	try{
#endif

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;

	TRACE( TEXT("透過選択を解除か選択 %u"), bMode );

	iLines = itLyr->vcLyrImg.size(  );

	for( iL = 0; iLines > iL; iL++ )
	{

		for( itMozi = itLyr->vcLyrImg.at( iL ).vcLine.begin( );
		itMozi != itLyr->vcLyrImg.at( iL ).vcLine.end( ); itMozi++ )
		{
			if( bMode )
			{
				chb = itMozi->cchMozi;
				if( iswspace( chb ) ){	itMozi->mzStyle |=  CT_LYR_TRNC;	}
			}
			else
			{
				itMozi->mzStyle &= ~CT_LYR_TRNC;
			}
		}

	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

INT LayerTransparentAdjust( LAYER_ITR itLyr, INT dNowDot, INT rdLine )
{
	INT_PTR	i, iCount, iLines, iLetter;
	INT		dDotCnt = 0, dPrvCnt = 0, rdWidth = 0;
	TCHAR	ch, chb;
	LETR_ITR	itMozi, itHead, itTail, itTemp;

#ifdef DO_TRY_CATCH
	try{
#endif

	iLines = itLyr->vcLyrImg.size(  );
	if( 0 >= iLines )	return 0;
	if( iLines <= rdLine )	return 0;

	iCount = itLyr->vcLyrImg.at( rdLine ).vcLine.size(  );
	if( 0 >= iCount )	return 0;

	itMozi = itLyr->vcLyrImg.at( rdLine ).vcLine.begin( );

	for( i = 0, iLetter = 0; iCount > i; i++, iLetter++ )
	{
		if( dNowDot <= dDotCnt ){	break;	}

		dPrvCnt = dDotCnt;
		rdWidth = itLyr->vcLyrImg.at( rdLine ).vcLine.at( i ).rdWidth;
		dDotCnt += rdWidth;
	}

	if( iCount <= iLetter )	return 0;

	if(  1 <= iLetter )
	{
		iLetter--;
		itMozi += iLetter;
	}

	ch = itLyr->vcLyrImg.at( rdLine ).vcLine.at( iLetter ).cchMozi;

	if( !( iswspace( ch ) ) )	return 0;

	itHead = itLyr->vcLyrImg.at( rdLine ).vcLine.begin( );
	for( ; itHead != itMozi; itMozi-- )
	{
		chb = itMozi->cchMozi;
		if(  !( iswspace( chb ) ) ){	itMozi++;	break;	}
	}
	if( itHead == itMozi )
	{
		chb = itMozi->cchMozi;
		if(  !( iswspace( chb ) ) ){	itMozi++;	}
	}

	itTail = itLyr->vcLyrImg.at( rdLine ).vcLine.end( );
	for( itTemp = itMozi; itTemp != itTail; itTemp++ )
	{
		chb = itTemp->cchMozi;
		if(  !( iswspace( chb ) ) ){	break;	}

		itTemp->mzStyle ^= CT_LYR_TRNC;
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return iLetter;
}

HRESULT LayerMoveFromView( HWND hWnd, UINT state )
{
	LAYER_ITR	itLyr;
	RECT	vwRect = {0,0,0,0};
	POINT	lyPoint;

	if( SIZE_MINIMIZED != state )
	{
		GetWindowRect( ghViewWnd, &vwRect );
		gstViewOrigin.x = vwRect.left;
		gstViewOrigin.y = vwRect.top;
	}

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( SIZE_MINIMIZED == state )
		{
			ShowWindow( itLyr->hBoxWnd, SW_HIDE );
		}
		else
		{

			lyPoint.x = itLyr->stOffset.x + vwRect.left;
			lyPoint.y = itLyr->stOffset.y + vwRect.top;
	#ifdef _DEBUG
			SetWindowPos( itLyr->hBoxWnd, HWND_TOP, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
	#else
			SetWindowPos( itLyr->hBoxWnd, HWND_TOPMOST, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
	#endif
		}
	}

	return S_OK;
}

LPTSTR LayerLineTextGetAlloc( LAYER_ITR itLyr, INT il )
{
	UINT_PTR	cchSize, i = 0;
	LPTSTR	ptText;

	cchSize = itLyr->vcLyrImg.at( il ).vcLine.size( );
	if( 0 >= cchSize )	return NULL;

	ptText = (LPTSTR)malloc( (cchSize+1) * sizeof(TCHAR) );
	ZeroMemory( ptText, (cchSize+1) * sizeof(TCHAR) );

	for( i = 0; cchSize > i; i++ )
	{
		ptText[i] = itLyr->vcLyrImg.at( il ).vcLine.at( i ).cchMozi;
	}

	return ptText;
}

HRESULT LayerStringObliterate( LAYER_ITR itLyr )
{
	UINT_PTR	j, iLine;

	iLine = itLyr->vcLyrImg.size( );
	for( j = 0; iLine > j; j++ )
	{
		itLyr->vcLyrImg.at( j ).vcLine.clear( );
	}
	itLyr->vcLyrImg.clear(  );

	return S_OK;
}

HRESULT LayerEditOnOff( HWND hWnd, UINT dStyle )
{
	UINT_PTR	i, iLines;
	INT			ndx;
	UINT		cchSize;
	LPTSTR		ptStr;
	ONELINE		stLine;
	LAYER_ITR	itLyr;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			if( dStyle )
			{

				Edit_SetText( itLyr->hTextWnd, TEXT("") );

				SetFocus( itLyr->hTextWnd );

				iLines = itLyr->vcLyrImg.size( );
				for( i = 0; iLines > i; i++ )
				{
					if( 0 != i )
					{
						ndx = GetWindowTextLength( itLyr->hTextWnd );
						SendMessage( itLyr->hTextWnd, EM_SETSEL, ndx, ndx );
						SendMessage( itLyr->hTextWnd, EM_REPLACESEL, 0, (LPARAM)(CH_CRLFW) );
					}

					ptStr = LayerLineTextGetAlloc( itLyr, i );
					if( ptStr )
					{
						ndx = GetWindowTextLength( itLyr->hTextWnd );
						SendMessage( itLyr->hTextWnd, EM_SETSEL, ndx, ndx );
						SendMessage( itLyr->hTextWnd, EM_REPLACESEL, 0, (LPARAM)ptStr );

						FREE(ptStr);
					}
				}

				ShowWindow( itLyr->hTextWnd, SW_SHOW );
			}
			else
			{
				ndx = Edit_GetTextLength( itLyr->hTextWnd );
				ndx += 2;
				ptStr = (LPTSTR)malloc( ndx * sizeof(TCHAR) );
				ZeroMemory( ptStr, ndx * sizeof(TCHAR) );
				Edit_GetText( itLyr->hTextWnd, ptStr, ndx );
				ShowWindow( itLyr->hTextWnd, SW_HIDE );

				StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

				LayerStringObliterate( itLyr );
				ZeroONELINE( &stLine );
				itLyr->vcLyrImg.push_back( stLine );

				LayerBoxSetString( itLyr, ptStr, cchSize, NULL, 0x00 );

				FREE(ptStr);

				InvalidateRect( hWnd, NULL, TRUE );

			}

			break;
		}
	}

	UpdateWindow( hWnd );

	return S_OK;
}

HRESULT LayerStringReplace( HWND hLyrWnd, LPTSTR ptStr )
{
	UINT		cchSize;
	ONELINE		stLine;
	LAYER_ITR	itLyr;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hLyrWnd )
		{
			StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

			LayerStringObliterate( itLyr );
			ZeroONELINE( &stLine );
			itLyr->vcLyrImg.push_back( stLine );

			LayerBoxSetString( itLyr, ptStr, cchSize, NULL, 0x00 );

			break;
		}
	}

	return S_OK;
}

HRESULT LayerFromString( LAYER_ITR itLyr, LPCTSTR ptStr )
{
	UINT	cchSize;
	ONELINE	stLine;

	ZeroONELINE( &stLine );

	itLyr->vcLyrImg.push_back( stLine );

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

	LayerBoxSetString( itLyr, ptStr, cchSize, NULL, 0x01 );

	return S_OK;
}

HRESULT LayerFromSelectArea( LAYER_ITR itLyr, UINT bSqSel )
{
	LPTSTR	ptString = NULL;
	UINT	cchSize, cbSize;
	LPPOINT	pstPos;
	ONELINE	stLine;

	TRACE( TEXT("選択範囲から取得") );
#ifdef DO_TRY_CATCH
	try{
#endif

	ZeroONELINE( &stLine );

	itLyr->vcLyrImg.push_back( stLine );

	cbSize = DocSelectTextGetAlloc( D_UNI | bSqSel, (LPVOID *)(&ptString), &pstPos );

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	LayerBoxSetString( itLyr, ptString, cchSize, (bSqSel & D_SQUARE) ? pstPos : NULL, 0x01 );

	FREE(ptString);
	FREE(pstPos);

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif
	return S_OK;
}

HRESULT LayerFromClipboard( LAYER_ITR itLyr )
{
	LPTSTR	ptString = NULL;
	UINT	cchSize, dStyle;

	ONELINE	stLine;

	ZeroONELINE( &stLine );

	itLyr->vcLyrImg.push_back( stLine );

	ptString = DocClipboardDataGet( &dStyle );

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	LayerBoxSetString( itLyr, ptString, cchSize, NULL, 0x01 );

	FREE( ptString );

	return S_OK;
}

HRESULT LayerBoxSizeAdjust( LAYER_ITR itLyr )
{
	INT	dViewXdot, dYline, dViewYdot;
	INT	iMaxDot = 0, iYdot;
	INT_PTR	iLine, i;
	SIZE	wdSize, tgtSize;

#ifdef DO_TRY_CATCH
	try{
#endif

	dYline = ViewAreaSizeGet( &dViewXdot );
	dViewYdot = dYline * LINE_HEIGHT;

	iLine = itLyr->vcLyrImg.size(  );
	iYdot = iLine * LINE_HEIGHT;
	for( i = 0; iLine > i; i++ )
	{
		if( iMaxDot < itLyr->vcLyrImg.at( i ).iDotCnt ){	iMaxDot = itLyr->vcLyrImg.at( i ).iDotCnt;	}
	}

	wdSize.cx = gstFrmSz.x + iMaxDot + gstFrmSz.x;
	wdSize.cy = gstFrmSz.y + iYdot + gstFrmSz.x;

	if( LB_WIDTH >  wdSize.cx ){	tgtSize.cx = LB_WIDTH;	}
	else if( dViewXdot < wdSize.cx ){	tgtSize.cx =  dViewXdot;	}
	else{	tgtSize.cx =  wdSize.cx;	}

	if( LB_HEIGHT > wdSize.cy ){	tgtSize.cy =  LB_HEIGHT;	}
	else if( dViewYdot < wdSize.cy ){	tgtSize.cy =  dViewYdot;	}
	else{	tgtSize.cy =  wdSize.cy;	}

#ifdef _DEBUG
	SetWindowPos( itLyr->hBoxWnd, HWND_TOP, 0, 0, tgtSize.cx, tgtSize.cy, SWP_NOMOVE | SWP_NOZORDER );
#else
	SetWindowPos( itLyr->hBoxWnd, HWND_TOPMOST, 0, 0, tgtSize.cx, tgtSize.cy, SWP_NOMOVE | SWP_NOZORDER );
#endif

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

HRESULT LayerBoxSetString( LAYER_ITR itLyr, LPCTSTR ptText, UINT cchSize, LPPOINT pstPt, UINT bStyle )
{
	UINT_PTR	i, j, iLine, iTexts;
	LONG	dMin = 0;
	INT		insDot, yLine, dSpDot, dSpMozi, iLines = 0, dOffset;
	LPTSTR	ptBuff, ptSpace = NULL;
	ONELINE	stLine;

#ifdef DO_TRY_CATCH
	try{
#endif
	ZeroONELINE( &stLine );

	if( pstPt )
	{
		dMin = pstPt[0].x;

		yLine = 0;
		for( i = 0; cchSize > i; i++ )
		{
			if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )
			{

				if( dMin > pstPt[yLine].x ){	dMin = pstPt[yLine].x;	}

				i++;
				yLine++;
			}
		}

		iLines = yLine;

		insDot = 0;
		dOffset = pstPt[0].x - dMin;
		ptSpace = DocPaddingSpaceUni( dOffset, NULL, NULL, NULL );

		StringCchLength( ptSpace, STRSAFE_MAX_CCH, &iTexts );
		for( j = 0; iTexts > j; j++ )
		{
			insDot += LayerInputLetter( itLyr, insDot, 0, ptSpace[j] );
		}
		FREE(ptSpace);
	}

	yLine = 0;	insDot = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )
		{
			itLyr->vcLyrImg.push_back( stLine );

			i++;
			yLine++;
			insDot = 0;

			if( pstPt && (iLines > yLine) )
			{
				dOffset = pstPt[yLine].x - dMin;
				ptSpace = DocPaddingSpaceUni( dOffset, NULL, NULL, NULL );

				StringCchLength( ptSpace, STRSAFE_MAX_CCH, &iTexts );
				for( j = 0; iTexts > j; j++ )
				{
					insDot += LayerInputLetter( itLyr, insDot, yLine, ptSpace[j] );
				}
				FREE(ptSpace);
			}

		}
		else if( CC_TAB == ptText[i] )
		{

		}
		else
		{
			insDot += LayerInputLetter( itLyr, insDot, yLine, ptText[i] );
		}
	}

	iLine = itLyr->vcLyrImg.size( );
	for( i = 0; iLine > i; i++ )
	{

		ptBuff = DocLastSpDel( &(itLyr->vcLyrImg.at( i ).vcLine) );
		FREE(ptBuff);

		dSpMozi = 0;
		dSpDot = LayerHeadSpaceCheck( &(itLyr->vcLyrImg.at( i ).vcLine), &dSpMozi );

		itLyr->vcLyrImg.at( i ).dFrtSpDot  = dSpDot;
		itLyr->vcLyrImg.at( i ).dFrtSpMozi = dSpMozi;

	}

	if( bStyle ){	LayerBoxSizeAdjust( itLyr );	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

INT LayerHeadSpaceCheck( vector<LETTER> *vcTgLine, PINT pdMozi )
{
	TCHAR		ch;
	INT			cchSp, dDot;
	UINT_PTR	i, iMozi;

#ifdef DO_TRY_CATCH
	try{
#endif
	iMozi = vcTgLine->size(  );

	dDot = 0;	cchSp = 0;
	for( i = 0; iMozi > i; i++ )
	{
		ch = vcTgLine->at( i ).cchMozi;

		if( !( iswspace(ch) ) && TEXT('.') != ch )
		{
			if( pdMozi ){	*pdMozi = cchSp;	}
			return dDot;
		}

		dDot += vcTgLine->at( i ).rdWidth;
		cchSp++;
	}

	if( pdMozi ){	*pdMozi = cchSp;	}
#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return dDot;
}

INT LayerInputLetter( LAYER_ITR itLyr, INT nowDot, INT rdLine, TCHAR ch )
{
	LETTER	stLetter;

#ifdef DO_TRY_CATCH
	try{
#endif

	DocLetterDataCheck( &stLetter, ch );

	itLyr->vcLyrImg.at( rdLine ).vcLine.push_back( stLetter );

	itLyr->vcLyrImg.at( rdLine ).iDotCnt += stLetter.rdWidth;
	itLyr->vcLyrImg.at( rdLine ).iByteSz += stLetter.mzByte;

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return stLetter.rdWidth;
}

HRESULT LayerContentsImportable( HWND hWnd, UINT cmdID, LPINT pXdot, LPINT pYline, UINT dStyle )
{
	RECT		vwRect, lyRect;
	POINT		conPoint;
	INT			xTgDot, xDot, iSrcDot, iSabun, iDivid, iSpDot;
	INT			dGap, dInLen, dInBgn, dInEnd, dEndot;
	INT			dLeft, dRight;
	INT			iPageLine, yTgLine, dWkLine, dLyLine;
	INT			iMinus, iMozi, iStMozi, iEdMozi;
	INT			dBkLeft, dBkRight, dBkStMozi, dBkEdMozi;
	INT_PTR		dNeedLine;
	UINT_PTR	cchSize;
	LPTSTR		ptStr, ptBuffer;
	BOOLEAN		bFirst = TRUE;
	BOOLEAN		bSpace, bBkSpase;

#ifdef EDGE_BLANK_STYLE
	INT		bEdgeBlank;
	INT		xDotEx, iMoziEx;
#endif
	LAYER_ITR	itLyr;

	LETR_ITR	itLtr, itDel;
	wstring	wsBuff;

#ifdef DO_TRY_CATCH
	try{
#endif

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;

	GetWindowRect( ghViewWnd, &vwRect );
	vwRect.left += LINENUM_WID;
	vwRect.top  += RULER_AREA;

	GetWindowRect( itLyr->hBoxWnd, &lyRect );
	conPoint.x = lyRect.left + gstFrmSz.x;
	conPoint.y = lyRect.top  + gstFrmSz.y;

	xTgDot   =  conPoint.x - vwRect.left;
	yTgLine  =  conPoint.y - vwRect.top;

	yTgLine /= LINE_HEIGHT;

	xTgDot  += gdHideXdot;
	yTgLine += gdViewTopLine;

	xDot = xTgDot;

	TRACE( TEXT("LAYER IMPORT[%d:%d]"), xTgDot, yTgLine );

	if( pXdot  )	*pXdot  = xTgDot;
	if( pYline )	*pYline = yTgLine;

	dNeedLine = itLyr->vcLyrImg.size( );

	ptStr = LayerLineTextGetAlloc( itLyr, dNeedLine - 1 );
	if( !(ptStr) )	dNeedLine--;
	FREE(ptStr);

	iPageLine = DocPageParamGet( NULL, NULL );

	if( iPageLine < (dNeedLine + yTgLine) )
	{
		iMinus = ( dNeedLine + yTgLine ) - iPageLine;
		DocAdditionalLine( iMinus, &bFirst );
		TRACE( TEXT("ADD LINE[%d]"), iMinus );
	}

	bEdgeBlank = ComboBox_GetCurSel( GetDlgItem( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), IDCB_LAYER_EDGE_BLANK ) );
	if( 1 == bEdgeBlank ){			LayerEdgeBlankSizeCheck( hWnd, EDGE_BLANK_NARROW );	}
	else if( 2 ==  bEdgeBlank ){	LayerEdgeBlankSizeCheck( hWnd, EDGE_BLANK_WIDE );	}

	for( dWkLine = yTgLine, dLyLine = 0; (yTgLine+dNeedLine) > dWkLine; dWkLine++, dLyLine++ )
	{
		if( 0 > dWkLine )	continue;

		TRACE( TEXT("Check Line V[%d] L[%d]"), dWkLine, dLyLine );

		iSpDot  = itLyr->vcLyrImg.at( dLyLine ).dFrtSpDot;

		xDot   = xTgDot + iSpDot;

		itLtr  = itLyr->vcLyrImg.at( dLyLine ).vcLine.begin( );
		itLtr += itLyr->vcLyrImg.at( dLyLine ).dFrtSpMozi;

		while( itLtr != itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ) )
		{
			while( 0 > xDot )
			{

				if( itLtr == itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ) )	break;

				if( itLtr->mzStyle & CT_LYR_TRNC )	break;

				xDot   += itLtr->rdWidth;
				iSpDot += itLtr->rdWidth;

				itLtr++;
			}

			wsBuff.clear( );	dInLen = 0;
			for(  ; itLtr != itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ); itLtr++ )
			{

				if( itLtr->mzStyle & CT_LYR_TRNC )	break;

				wsBuff += itLtr->cchMozi;	dInLen += itLtr->rdWidth;
			}

			if( 0 != dInLen )
			{
				cchSize = wsBuff.size( ) + 1;
				ptStr = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
				StringCchCopy( ptStr, cchSize, wsBuff.c_str( ) );

				dGap = 0;

				iSrcDot = DocLineParamGet( dWkLine, NULL, NULL );
				iSabun  = xTgDot - iSrcDot;
				iDivid  = iSabun + iSpDot;
				if( 0 < iDivid )
				{
					xDot = iSrcDot;
					ptBuffer = DocPaddingSpaceWithPeriod( iDivid, NULL, NULL, NULL, TRUE );

					if( ptBuffer )
					{
						DocInsertString( &xDot, &dWkLine, NULL, ptBuffer, dStyle, bFirst );	bFirst = FALSE;
						FREE(ptBuffer);
					}

				}
				else if( 0 > iDivid )
				{

					iMozi = DocLetterPosGetAdjust( &xDot, dWkLine, -1 );

#ifndef EDGE_BLANK_STYLE

					DocLineStateCheckWithDot( xDot, dWkLine, &dLeft, &dRight, &iStMozi, NULL, &bSpace );

#endif

					if( IDM_LYB_OVERRIDE == cmdID )
					{
						dInBgn  = xTgDot + iSpDot;
						dInEnd  = dInBgn + dInLen;

						dEndot  = dInEnd;
#ifdef EDGE_BLANK_STYLE

						if( 1 == bEdgeBlank ){		dEndot += EDGE_BLANK_NARROW;	}
						else if( 2 == bEdgeBlank ){	dEndot += EDGE_BLANK_WIDE;	}
#endif
						iEdMozi = DocLetterPosGetAdjust( &dEndot , dWkLine, 1 );

						DocLineStateCheckWithDot( dEndot, dWkLine, &dBkLeft, &dBkRight, &dBkStMozi, &dBkEdMozi, &bBkSpase );
						if( bBkSpase )
						{
							dEndot  = dBkRight;
							iEdMozi = DocLetterPosGetAdjust( &dEndot , dWkLine, 1 );

							dGap    = dBkRight - dInEnd;

						}
						else
						{

							dGap    = dEndot - dInEnd;

						}

						DocRangeDeleteByMozi( xDot, dWkLine, iMozi, iEdMozi, &bFirst );

						if( 0 < dGap )
						{
							dInBgn  = xDot;

							ptBuffer = DocPaddingSpaceWithPeriod( dGap, NULL, NULL, NULL, TRUE );
							if( ptBuffer )
							{
								DocInsertString( &dInBgn, &dWkLine, NULL, ptBuffer, dStyle, bFirst );	bFirst = FALSE;
								FREE(ptBuffer);
							}
						}
					}

#ifdef EDGE_BLANK_STYLE
					if( bEdgeBlank )
					{

						xDotEx  = (xTgDot + iSpDot);

						if( 1 == bEdgeBlank ){		xDotEx -= EDGE_BLANK_NARROW;	}
						else if( 2 == bEdgeBlank ){	xDotEx -= EDGE_BLANK_WIDE;	}
						else{	;	}

						if( 0 > xDotEx ){	xDotEx =  0;	}

						iMoziEx = DocLetterPosGetAdjust( &xDotEx, dWkLine, -1 );

						DocLineStateCheckWithDot( xDotEx, dWkLine, &dLeft, &dRight, &iStMozi, NULL, &bSpace );

						dGap = (xTgDot + iSpDot) - xDotEx;
						xDot = xDotEx;
					}
					else
					{

						DocLineStateCheckWithDot( xDot, dWkLine, &dLeft, &dRight, &iStMozi, NULL, &bSpace );
						iMoziEx = iStMozi;
#endif
						dGap = (xTgDot + iSpDot) - xDot;
#ifdef EDGE_BLANK_STYLE
					}
#endif
					if( bSpace )
					{
						dGap  += (xDot - dLeft);

						DocRangeDeleteByMozi( dLeft, dWkLine, iStMozi, iMozi, &bFirst );
					}
					else
					{
#ifdef EDGE_BLANK_STYLE

						if( bEdgeBlank )	DocRangeDeleteByMozi( xDot, dWkLine, iMoziEx, iMozi, &bFirst );
#endif
						dLeft = xDot;
					}

					ptBuffer = DocPaddingSpaceWithPeriod( dGap, NULL, NULL, NULL, TRUE );
					if( ptBuffer )
					{
						DocInsertString( &dLeft, &dWkLine, NULL, ptBuffer, dStyle, bFirst );	bFirst = FALSE;
						FREE(ptBuffer);
					}

					xDot = dLeft;
				}
				else
				{

				}

				DocInsertString( &xDot, &dWkLine, NULL, ptStr, dStyle, bFirst );	bFirst = FALSE;

				FREE(ptStr);
			}

			if( itLtr == itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ) )	break;

			for(  ; itLtr != itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ); itLtr++ )
			{

				if( !(itLtr->mzStyle & CT_LYR_TRNC) )	break;
				xDot += itLtr->rdWidth;
			}

			iSpDot = xDot;
			iSpDot -= xTgDot;
		}

	}

	TRACE( TEXT("Layer Insert OK！") );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

HRESULT LayerForClipboard( HWND hWnd, UINT bStyle )
{
	INT_PTR	iLines, iL, cchSize, cbSize;
	LETR_ITR	itMozi;

	LAYER_ITR	itLyr;

	string	srString;
	wstring	wsString;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;

	iLines = itLyr->vcLyrImg.size( );

	srString.clear( );
	wsString.clear( );

	for( iL = 0; iLines > iL; iL++ )
	{

		for( itMozi = itLyr->vcLyrImg.at( iL ).vcLine.begin( ); itMozi != itLyr->vcLyrImg.at( iL ).vcLine.end( ); itMozi++ )
		{
			srString += string( itMozi->acSjis );
			wsString += itMozi->cchMozi;
		}

		srString +=  string( CH_CRLFA );
		wsString += wstring( CH_CRLFW );
	}

	if( bStyle & D_UNI )
	{
		cchSize = wsString.size( ) + 1;
		DocClipboardDataSet( (LPTSTR)(wsString.c_str()), cchSize * sizeof(TCHAR), bStyle );
	}
	else
	{
		cbSize = srString.size( ) + 1;
		DocClipboardDataSet( (LPSTR)(srString.c_str()), cbSize, bStyle );
	}

	return S_OK;
}

HRESULT LayerOnDelete( HWND hWnd )
{
	ONELINE		stLine;

	LAYER_ITR	itLyr;

#ifdef DO_TRY_CATCH
	try{
#endif

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;

	LayerStringObliterate( itLyr );
	ZeroONELINE( &stLine );
	itLyr->vcLyrImg.push_back( stLine );

	InvalidateRect( hWnd, NULL, TRUE );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}

#ifdef EDGE_BLANK_STYLE

HRESULT LayerEdgeBlankSizeCheck( HWND hWnd, INT iCanWid )
{
	INT_PTR	iLines;
	INT		iWidth;

	LAYER_ITR	itLyr;

	LYLINE_ITR	itLine;
	LETR_ITR	itMozi, itMzx;

#ifdef DO_TRY_CATCH
	try{
#endif

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;

	iLines = itLyr->vcLyrImg.size( );

	for( itLine = itLyr->vcLyrImg.begin( ); itLine != itLyr->vcLyrImg.end( ); itLine++ )
	{

		for( itMozi = itLine->vcLine.begin( ); itMozi != itLine->vcLine.end( ); itMozi++ )
		{
			if(  itMozi->mzStyle & CT_LYR_TRNC )
			{

				iWidth = 0;
				for( itMzx = itMozi; itMzx != itLine->vcLine.end( ); itMzx++ )
				{
					if( !(itMzx->mzStyle & CT_LYR_TRNC) )	break;
					iWidth += itMzx->rdWidth;
				}

				if( iCanWid >=  iWidth )
				{
					for( ; itMzx != itMozi; itMozi++ )
					{
						itMozi->mzStyle &= ~CT_LYR_TRNC;
					}
				}
				else
				{
					itMozi = itMzx;
				}
				itMozi--;
			}
		}
	}

	InvalidateRect( hWnd , NULL, TRUE );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif
	return S_OK;
}

#endif
