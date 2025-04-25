#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"

typedef struct tagVIEWORDER
{
	UINT	index;
	UINT	dHeight;
	UINT	dUpper;
	UINT	dDownr;

} VIEWORDER, *LPVIEWORDER;

typedef struct tagAATITLE
{
	UINT	number;
	TCHAR	atTitle[MAX_STRING];

} AATITLE, *LPAATITLE;

#define TITLECBX_HEI	200

#define SBP_DIRECT		0xFF

static  HWND	ghItemsWnd;
#ifdef MAA_TOOLTIP
static  HWND	ghToolTipWnd;
EXTERNED HFONT	ghTipFont;
#endif

static  HWND	ghComboxWnd;

static WNDPROC	gpfOrgAaItemsProc;
static WNDPROC	gpfOrgAaTitleCbxProc;

static LPTSTR	gptTipBuffer;

static INT		gixTopItem;
static INT		gixMaxItem;

static  LONG	gixNowSel;
#ifdef USE_HOVERTIP
static  LONG	gixNowToolTip;
#endif

static  HWND	ghScrollWnd;

#ifdef _ORRVW
EXTERNED HFONT	ghAaFont;
#else
static HFONT	ghAaFont;
#endif

static HBRUSH	ghBkBrush;

static  HPEN	ghSepPen;
static BOOLEAN	gbLineSep;

#ifndef _ORRVW
static BOOLEAN	gbMaaRetFocus;
#endif

#ifdef MAA_TEXT_FIND
TCHAR	gatFindText[MAX_STRING];
#endif

extern  UINT	gbAAtipView;

extern  HWND	ghSplitaWnd;

static vector<VIEWORDER>	gvcViewOrder;
static vector<AATITLE>		gvcAaTitle;

#ifdef MAA_TOOLTIP
LRESULT	Aai_OnNotify( HWND , INT, LPNMHDR );
#endif
VOID	Aai_OnMouseMove( HWND, INT, INT, UINT );
VOID	Aai_OnLButtonUp( HWND, INT, INT, UINT );
VOID	Aai_OnMButtonUp( HWND, INT, INT, UINT );
VOID	Aai_OnContextMenu( HWND, HWND, UINT, UINT );
VOID	Aai_OnDropFiles( HWND , HDROP );

HRESULT	AaItemsFavDelete( LPSTR, UINT );

#ifdef MAA_TEXT_FIND
UINT	AacItemFindOnePage( HWND, LPTSTR, INT );
#endif

LRESULT	CALLBACK gpfAaItemsProc( HWND, UINT, WPARAM, LPARAM );
LRESULT	CALLBACK gpfAaTitleCbxProc( HWND, UINT, WPARAM, LPARAM );

#ifndef _ORRVW
INT_PTR	CALLBACK AaItemAddDlgProc( HWND, UINT, WPARAM, LPARAM );
#endif

#ifdef USE_HOVERTIP
LPTSTR	CALLBACK AaItemsHoverTipInfo( LPVOID  );
#endif

HRESULT AaItemsInitialise( HWND hWnd, HINSTANCE hInst, LPRECT ptRect )
{
#ifdef MAA_TOOLTIP
	INT		ttSize;
	TTTOOLINFO	stToolInfo;
#endif
	SCROLLINFO	stScrollInfo;
	RECT	rect;
	LOGFONT	stFont;

	COLORREF	dBkColour;

	if( !(hWnd) )
	{
		free( gptTipBuffer );
		SetWindowFont( ghItemsWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
#ifdef MAA_TOOLTIP
		SetWindowFont( ghToolTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
		DeleteFont( ghTipFont );
#endif
		DeleteFont( ghAaFont );
		DeletePen( ghSepPen );
		DeleteBrush( ghBkBrush );
		return S_FALSE;
	}

	ghSepPen  = CreatePen( PS_SOLID, 1, RGB(0xAA,0xAA,0xAA) );

	gbLineSep = InitParamValue( INIT_LOAD, VL_MAASEP_STYLE, 0 );

	dBkColour = (COLORREF)InitParamValue( INIT_LOAD, VL_MAA_BKCOLOUR, 0x00FFFFFF );
	ghBkBrush = CreateSolidBrush( dBkColour );

#ifndef _ORRVW

	gbMaaRetFocus = InitParamValue( INIT_LOAD, VL_MAA_RETFCS, 0 );
#endif

	gptTipBuffer = NULL;

	gixTopItem = 0;

	gixNowSel = -1;
#ifdef USE_HOVERTIP
	gixNowToolTip = -1;
#endif

#ifdef MAA_TEXT_FIND
	ZeroMemory( gatFindText, sizeof(gatFindText) );
#endif

#ifdef MAA_TOOLTIP

	ghToolTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, hInst, NULL );
#endif

	ghComboxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT(""), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT, TREE_WIDTH + SPLITBAR_WIDTH, 0, ptRect->right - TREE_WIDTH - LSSCL_WIDTH, TITLECBX_HEI, hWnd, (HMENU)IDCB_AAITEMTITLE, hInst, NULL );
	GetClientRect( ghComboxWnd, &rect );

	gpfOrgAaTitleCbxProc = SubclassWindow( ghComboxWnd, gpfAaTitleCbxProc );

	ghItemsWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_STATIC, TEXT(""), WS_VISIBLE | WS_CHILD | SS_OWNERDRAW | SS_NOTIFY, TREE_WIDTH + SPLITBAR_WIDTH, rect.bottom, ptRect->right - TREE_WIDTH - LSSCL_WIDTH, ptRect->bottom - rect.bottom, hWnd, (HMENU)IDSO_AAITEMS, hInst, NULL );

	gpfOrgAaItemsProc = SubclassWindow( ghItemsWnd, gpfAaItemsProc );

	ghScrollWnd = CreateWindowEx( 0, WC_SCROLLBAR, TEXT("scroll"), WS_VISIBLE | WS_CHILD | SBS_VERT, ptRect->right - LSSCL_WIDTH, rect.bottom, LSSCL_WIDTH, ptRect->bottom - rect.bottom, hWnd, (HMENU)IDSB_LISTSCROLL, hInst, NULL );

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_DISABLENOSCROLL;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	ViewingFontGet( &stFont );
	ghAaFont = CreateFontIndirect( &stFont );
	SetWindowFont( ghItemsWnd, ghAaFont, TRUE );

#ifdef MAA_TOOLTIP

	ttSize = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, FONTSZ_REDUCE );
	stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
	ghTipFont = CreateFontIndirect( &stFont );
	SetWindowFont( ghToolTipWnd, ghTipFont, TRUE );

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;
	stToolInfo.hwnd     = ghItemsWnd;
	stToolInfo.uId      = IDSO_AAITEMS;
	GetClientRect( ghItemsWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage( ghToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghToolTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );
#endif

	return S_OK;
}

LRESULT	CALLBACK gpfAaTitleCbxProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR, Maa_OnChar );
		default:	break;
	}

	return CallWindowProc( gpfOrgAaTitleCbxProc, hWnd, msg, wParam, lParam );
}

VOID AaTitleClear( VOID )
{
	ComboBox_SetCurSel( ghComboxWnd, -1 );

	while( ComboBox_GetCount( ghComboxWnd ) ){	ComboBox_DeleteString( ghComboxWnd, 0 );	}
	gvcAaTitle.clear( );

	return;
}

INT AaTitleAddString( UINT number, LPSTR pcTitle )
{
	AATITLE	stTitle;
	LPTSTR	ptTitle;

	ptTitle = SjisDecodeAlloc( pcTitle );

	ComboBox_AddString( ghComboxWnd, ptTitle );

	ZeroMemory( &stTitle, sizeof(AATITLE) );
	stTitle.number = number;
	StringCchCopy( stTitle.atTitle, MAX_STRING, ptTitle );

	gvcAaTitle.push_back( stTitle );

	FREE(ptTitle);

	return ComboBox_GetCount( ghComboxWnd );
}

VOID AaTitleSelect( HWND hWnd, UINT codeNotify )
{
	INT	iSel;
	INT_PTR	iItems;

	if( CBN_SELCHANGE == codeNotify )
	{
		iSel = ComboBox_GetCurSel( ghComboxWnd );

		iItems = gvcAaTitle.size( );
		if( iItems <= iSel )	return;

		Aai_OnVScroll( hWnd, ghScrollWnd, SBP_DIRECT, gvcAaTitle.at( iSel ).number );
	}

	if( CBN_CLOSEUP == codeNotify )
	{
		SetFocus( ghItemsWnd );
	}

	return;
}

VOID AaItemsResize( HWND hWnd, LPRECT ptRect )
{
	INT		dWidth, dLeft;
	RECT	sptRect, rect;
#ifdef MAA_TOOLTIP
	TTTOOLINFO	stToolInfo;
#endif

	SplitBarPosGet( ghSplitaWnd, &sptRect );

	dWidth = ptRect->right - (sptRect.left + SPLITBAR_WIDTH) - LSSCL_WIDTH;
	dLeft  = ptRect->right - LSSCL_WIDTH;

	MoveWindow( ghComboxWnd, sptRect.left + SPLITBAR_WIDTH, ptRect->top, dWidth, TITLECBX_HEI, TRUE );
	GetClientRect( ghComboxWnd, &rect );

	MoveWindow( ghItemsWnd,  sptRect.left + SPLITBAR_WIDTH, ptRect->top + rect.bottom, dWidth, ptRect->bottom - rect.bottom, TRUE );
	MoveWindow( ghScrollWnd, dLeft, ptRect->top + rect.bottom, LSSCL_WIDTH, ptRect->bottom - rect.bottom, TRUE );

#ifdef MAA_TOOLTIP

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize = sizeof(TTTOOLINFO);
	stToolInfo.hwnd   = ghItemsWnd;
	stToolInfo.uId    = IDSO_AAITEMS;
	GetClientRect( ghItemsWnd, &stToolInfo.rect );
	SendMessage( ghToolTipWnd, TTM_NEWTOOLRECT, 0, (LPARAM)&stToolInfo );
#endif
	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd );

	return;
}

LRESULT CALLBACK gpfAaItemsProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,        Maa_OnChar );
		HANDLE_MSG( hWnd, WM_KEYDOWN,     Aai_OnKey );
		HANDLE_MSG( hWnd, WM_KEYUP,       Aai_OnKey );

		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Aai_OnMouseMove );
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Aai_OnLButtonUp );
		HANDLE_MSG( hWnd, WM_MBUTTONUP,   Aai_OnMButtonUp );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Aai_OnContextMenu );
		HANDLE_MSG( hWnd, WM_DROPFILES,   Aai_OnDropFiles );
#ifdef MAA_TOOLTIP
		HANDLE_MSG( hWnd, WM_NOTIFY,      Aai_OnNotify );
#endif

#ifdef USE_HOVERTIP
		case WM_MOUSEHOVER:
			HoverTipOnMouseHover( hWnd, wParam, lParam, AaItemsHoverTipInfo );
			return 0;

		case WM_MOUSELEAVE:
			HoverTipOnMouseLeave( hWnd );
			gixNowToolTip = -1;
			return 0;
#endif

		default:	break;
	}

	return CallWindowProc( gpfOrgAaItemsProc, hWnd, msg, wParam, lParam );
}

VOID AaItemsDrawItem( HWND hWnd, CONST DRAWITEMSTRUCT *pstDrawItem )
{
	UINT_PTR	rdLen;
	INT_PTR	rdLength;
	LPSTR	pcConts = NULL;
	LPTSTR	ptConStr = NULL;
	RECT	rect, drawRect;
	POINT	stPoint;

	VIEWORDER	stVwrder;

	INT		rdNextItem;
	LONG	rdDrawPxTop, rdBottom;
	LONG	rdHeight, rdWidth;

	HPEN	hOldPen;

	rect = pstDrawItem->rcItem;
	rdBottom = rect.bottom;
	rdWidth  = rect.right;
	rdDrawPxTop = 0;

	SetBkMode( pstDrawItem->hDC , TRANSPARENT );

	FillRect( pstDrawItem->hDC, &rect, ghBkBrush );

	gvcViewOrder.clear();

	rdNextItem = gixTopItem;
	for( rdDrawPxTop = 0; rdBottom > rdDrawPxTop; rdNextItem++ )
	{
		pcConts = AacAsciiArtGet( rdNextItem );
		if( !pcConts ){	break;	}

		stVwrder.index = rdNextItem;

		ptConStr = SjisDecodeAlloc( pcConts );
		StringCchLength( ptConStr, STRSAFE_MAX_CCH, &rdLen );
		rdLength = rdLen;

		free( pcConts );
#pragma message ("MAAの行間、ここで正しく計算するべき")

		DrawText( pstDrawItem->hDC, ptConStr, rdLength, &rect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX | DT_CALCRECT );
		drawRect = rect;
		rdHeight = drawRect.bottom;
		drawRect.bottom += rdDrawPxTop;
		drawRect.top     = rdDrawPxTop;
		if( drawRect.right < rdWidth )	drawRect.right = rdWidth;

		stVwrder.dHeight = rdHeight;
		stVwrder.dUpper  = drawRect.top;
		stVwrder.dDownr  = drawRect.bottom;

		if( gbLineSep ){	FillRect( pstDrawItem->hDC, &drawRect, ghBkBrush );	}
		else
		{
			if( 1 & rdNextItem )	FillRect( pstDrawItem->hDC, &drawRect, GetStockBrush(LTGRAY_BRUSH) );
			else					FillRect( pstDrawItem->hDC, &drawRect, ghBkBrush );
		}

		DrawText( pstDrawItem->hDC, ptConStr, rdLength, &drawRect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX );

		if( gbLineSep )
		{
			hOldPen = SelectPen( pstDrawItem->hDC, ghSepPen );
			MoveToEx( pstDrawItem->hDC, drawRect.left, drawRect.bottom-1, NULL );
			LineTo( pstDrawItem->hDC, drawRect.right, drawRect.bottom-1 );
			SelectPen( pstDrawItem->hDC, hOldPen );
		}

		gvcViewOrder.push_back( stVwrder );

		rdDrawPxTop += rdHeight;

		free( ptConStr );
	}
	InvalidateRect( ghScrollWnd, NULL, TRUE );
	UpdateWindow( ghScrollWnd );

	GetCursorPos( &stPoint );
	ScreenToClient( ghItemsWnd, &stPoint );
	Aai_OnMouseMove( hWnd, stPoint.x, stPoint.y, 0 );

	return;
}

#if 0

VOID AaItemsMeasureItem( HWND hWnd, LPMEASUREITEMSTRUCT pstMeasureItem )
{
	HDC		hDC;
	INT		rdLength, rdHeight;
	LPSTR	pcConts;
	RECT	stRect;

	pcConts = AacAsciiArtGet( pstMeasureItem->itemID );
	if( !pcConts )	return;

	rdLength = strlen( pcConts );

	ListBox_GetItemRect( ghItemsWnd, pstMeasureItem->itemID, &stRect );

	hDC = GetDC( ghItemsWnd );
	DrawTextExA( hDC, pcConts, rdLength, &stRect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX | DT_CALCRECT );
	ReleaseDC( ghItemsWnd, hDC );

	pstMeasureItem->itemHeight = (stRect.bottom - stRect.top);
	if( 256 <= pstMeasureItem->itemHeight )	pstMeasureItem->itemHeight = 255;

	free( pcConts );

	return;
}

#endif

VOID Aai_OnKey( HWND hWnd, UINT vk, BOOL fDown, INT cRepeat, UINT flags )
{
	TRACE( TEXT("KEY[%u][%u]"), vk, cRepeat );

	if( !(fDown) )	 return;

	switch( vk )
	{
		default:	return;

		case  VK_NEXT:
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_PAGEDOWN, 0 );
			break;

		case  VK_DOWN:
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_LINEDOWN, 0 );
			break;

		case  VK_UP:
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_LINEUP, 0 );
			break;

		case  VK_PRIOR:
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_PAGEUP, 0 );
			break;
	}

	return;
}

VOID Aai_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TCHAR		atBuffer[MAX_STRING];
	UINT_PTR	i, max;
	LONG		iItem = -1, bottom;
	BOOLEAN		bReDraw = FALSE;

	INT		iDot = 0, iLine = 0, iByte = 0;

	if( !( gvcViewOrder.empty() ) )
	{
		max = gvcViewOrder.size();

		bottom = 0;
		for( i = 0; max > i; i++ )
		{
			bottom += gvcViewOrder.at( i ).dHeight;

			if( y < bottom ){	iItem = gvcViewOrder.at(  i ).index;	break;	}
		}
	}

#ifdef USE_HOVERTIP
	if( gixNowToolTip != iItem ){	bReDraw =  TRUE;	}
	gixNowSel = iItem;
	gixNowToolTip = gixNowSel;

	if( bReDraw && gbAAtipView ){	HoverTipResist( ghItemsWnd );	}
#endif

#ifdef MAA_TOOLTIP
	if( gixNowSel != iItem ){	bReDraw =  TRUE;	}
	gixNowSel = iItem;

	if( bReDraw && gbAAtipView )	SendMessage( ghToolTipWnd, TTM_UPDATE, 0, 0 );
#endif

	if( bReDraw )
	{
		iDot = AacArtSizeGet( iItem, &iLine, &iByte );

		TRACE( TEXT("MAA MOUSE %3d[%dDOT x %dLINE]%dByte"), iItem+1, iDot, iLine, iByte );
#ifdef _ORRVW
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("%3d[%dDOT x %dLINE]"), iItem+1, iDot, iLine );
#else
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("%3d[%dDOT x %dLINE] %dByte"), iItem+1, iDot, iLine, iByte );
#endif
		StatusBarMsgSet( SBMAA_AXIS, atBuffer );
	}

	return;
}

VOID Aai_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	AaItemsDoSelect( hWnd, MAA_DEFAULT, TRUE );

#ifndef _ORRVW
	if( gbMaaRetFocus ){	ViewFocusSet(  );	}
#endif
	return;
}

VOID Aai_OnMButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	AaItemsDoSelect( hWnd, MAA_SUBDEFAULT, TRUE );

#ifndef _ORRVW
	if( gbMaaRetFocus ){	ViewFocusSet(  );	}
#endif
	return;
}

VOID Aai_OnVScroll( HWND hWnd, HWND hwndCtl, UINT code, INT pos )
{
	INT	maePos;
	SCROLLINFO	stScrollInfo;

	if( ghScrollWnd != hwndCtl )	return;

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo );

	maePos = gixTopItem;

	switch( code )
	{
		case SB_TOP:
			gixTopItem = 0;
			break;

		case SB_LINEUP:
		case SB_PAGEUP:
			gixTopItem--;
			if( 0 > gixTopItem )	gixTopItem = 0;
			break;

		case SB_LINEDOWN:
		case SB_PAGEDOWN:
			gixTopItem++;
			if( gixMaxItem <=  gixTopItem ){	gixTopItem = gixMaxItem-1;	}
			break;

		case SB_BOTTOM:
			gixTopItem = gixMaxItem - 1;
			break;

		case SB_THUMBTRACK:
			gixTopItem = stScrollInfo.nTrackPos;
			break;

		case SBP_DIRECT:
			gixTopItem = pos;
			break;

		default:	return;
	}

	if( maePos == gixTopItem )	return;

	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd  );

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = gixTopItem;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	TabMultipleTopMemory( gixTopItem );

	return;
}

#ifdef MAA_TOOLTIP

LRESULT Aai_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	UINT_PTR		rdLength;
	LPSTR			pcConts = NULL;
	LPNMTTDISPINFO	pstDispInfo;

	if( TTN_GETDISPINFO ==  pstNmhdr->code )
	{
		pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

		if( !(gbAAtipView) )
		{
			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );
			pstDispInfo->lpszText = NULL;
			return 0;
		}

		FREE( gptTipBuffer );

		pcConts = AacAsciiArtGet( gixNowSel );
		if( !pcConts  ){	return 0;	}

		rdLength = strlen( pcConts );

		gptTipBuffer = SjisDecodeAlloc( pcConts );

		pstDispInfo->lpszText = gptTipBuffer;

		free( pcConts );
	}

	return 0;
}

#endif

VOID Aai_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		dOpen;
	LPSTR	pcConts = NULL;
	UINT_PTR	rdLength;
	INT		sx, sy;

	dOpen = TabMultipleNowSel(  );

#pragma message ("Editorとviewerの、メニューリソースの整合性に注意セヨ")
	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AALIST_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

#ifdef _ORRVW

	if( ACT_FAVLIST ==  dOpen ){	EnableMenuItem( hSubMenu, IDM_MAA_FAV_DELETE , MF_ENABLED );	}

#endif

	if( gbAAtipView ){	CheckMenuItem( hSubMenu, IDM_MAA_AATIP_TOGGLE, MF_CHECKED );	}

#ifndef _ORRVW

	if( gbMaaRetFocus ){	CheckMenuItem( hSubMenu, IDM_MAA_RETURN_FOCUS, MF_CHECKED );	}
#endif

	sx = (SHORT)xPos;
	sy = (SHORT)yPos;

	dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, sx, sy, 0, hWnd, NULL );
	DestroyMenu( hMenu );
	switch( dRslt )
	{
		case IDM_MAA_FAV_DELETE:
			if( ACT_FAVLIST == dOpen )
			{
				pcConts = AacAsciiArtGet( gixNowSel );
				if( !pcConts ){	return;	}

				rdLength = strlen( pcConts );
				AaItemsFavDelete( pcConts, rdLength );
				FavContsRedrawRequest( hWnd );
			}
#ifndef _ORRVW
			else
			{
				AacItemDelete( hWnd, gixNowSel );
			}
#endif
			break;

#ifndef _ORRVW
		case IDM_MAA_INSERT_EDIT:		AaItemsDoSelect( hWnd, MAA_INSERT,   TRUE );	break;
		case IDM_MAA_INTERRUPT_EDIT:	AaItemsDoSelect( hWnd, MAA_INTERRUPT, TRUE );	break;
		case IDM_MAA_SET_LAYERBOX:		AaItemsDoSelect( hWnd, MAA_LAYERED,  TRUE );	break;
#endif
		case IDM_MAA_CLIP_UNICODE:		AaItemsDoSelect( hWnd, MAA_UNICLIP,  TRUE );	break;
		case IDM_MAA_CLIP_SHIFTJIS:		AaItemsDoSelect( hWnd, MAA_SJISCLIP, TRUE );	break;

		case IDM_DRAUGHT_ADDING:		AaItemsDoSelect( hWnd, MAA_DRAUGHT,  TRUE );	break;
#ifdef _ORRVW
		case IDM_DRAUGHT_OPEN:			Maa_OnCommand( hWnd, IDM_DRAUGHT_OPEN, NULL, 0 );	break;
#endif

		case IDM_MAA_AATIP_TOGGLE:
			gbAAtipView = gbAAtipView ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAATIP_VIEW, gbAAtipView );
			break;

		case IDM_MAA_SEP_STYLE_TOGGLE:
			gbLineSep = gbLineSep ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAASEP_STYLE, gbLineSep );
			InvalidateRect( ghItemsWnd, NULL, TRUE );
			break;

		case IDM_MAA_THUMBNAIL_OPEN:	Maa_OnCommand( hWnd , IDM_MAA_THUMBNAIL_OPEN, NULL, 0 );	break;

		case IDM_MAAITEM_BKCOLOUR:		MaaBackColourChoose( hWnd );	break;

#ifndef _ORRVW

		case IDM_MAA_RETURN_FOCUS:
			gbMaaRetFocus = gbMaaRetFocus ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAA_RETFCS, gbMaaRetFocus );
			break;

  #ifdef MAA_IADD_PLUS

		case IDM_MAA_ITEM_INSERT:		AacItemInsert( hWnd, gixNowSel );	break;
  #endif
#endif
	}

	return;
}

VOID Aai_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	TRACE( TEXT("AAI DROP[%s]"), atFileName );

	TabMultipleDropAdd( GetParent( hWnd ), atFileName );

	return;
}

HRESULT AaItemsDoShow( HWND hWnd, LPTSTR ptFileName, UINT type )
{
	SCROLLINFO	stScrollInfo;

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);

	gixTopItem = 0;

	AaTitleClear(  );

	switch( type )
	{
		case ACT_ALLTREE:	gixMaxItem = AacAssembleFile( hWnd, ptFileName );	break;
		case ACT_FAVLIST:	gixMaxItem = AacAssembleSql( hWnd, ptFileName );	break;
		default:
			gixMaxItem = AacAssembleFile( hWnd, ptFileName );

			gixTopItem = TabMultipleTopMemory( -1 );
			break;
	}

	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd );

	if( 0 == gixMaxItem )
	{
		stScrollInfo.fMask = SIF_DISABLENOSCROLL;
		SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );
		return E_FAIL;
	}

	stScrollInfo.fMask = SIF_ALL;
	stScrollInfo.nMax  = gixMaxItem-1;
	stScrollInfo.nPos  = gixTopItem;
	stScrollInfo.nPage = 1;
	stScrollInfo.nTrackPos = 0;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	return S_OK;
}

HRESULT AaItemsFavUpload( LPSTR pcConts, UINT rdLength )
{
	LPTSTR	ptBaseName;
	DWORD	dHash;

	ptBaseName = TreeBaseNameGet(  );

	if( !( StrCmp( DROP_OBJ_NAME, ptBaseName ) ) )	return E_ABORT;

	HashData( (LPBYTE)pcConts, rdLength, (LPBYTE)(&(dHash)), 4 );

	SqlFavUpload( ptBaseName, dHash, pcConts, rdLength );

	return S_OK;
}

HRESULT AaItemsFavDelete( LPSTR pcConts, UINT rdLength )
{
	LPTSTR	ptBaseName;
	DWORD	dHash;

	ptBaseName = TreeBaseNameGet(  );

	HashData( (LPBYTE)pcConts, rdLength, (LPBYTE)(&(dHash)), 4 );

	SqlFavDelete( ptBaseName, dHash );

	return S_OK;
}

HRESULT MaaBackColourChoose( HWND hWnd )
{
	BOOL	bRslt;
	COLORREF	adColourTemp[16], dColour;
	CHOOSECOLOR	stChColour;

	ZeroMemory( adColourTemp, sizeof(adColourTemp) );

	dColour = (COLORREF)InitParamValue( INIT_LOAD, VL_MAA_BKCOLOUR, 0x00FFFFFF );

	adColourTemp[0] = dColour;

	ZeroMemory( &stChColour, sizeof(CHOOSECOLOR) );
	stChColour.lStructSize  = sizeof(CHOOSECOLOR);
	stChColour.hwndOwner    = hWnd;

	stChColour.rgbResult    = dColour;
	stChColour.lpCustColors = adColourTemp;
	stChColour.Flags        = CC_RGBINIT;

	bRslt = ChooseColor( &stChColour  );
	if( bRslt )
	{
		dColour =  stChColour.rgbResult;
		InitParamValue( INIT_SAVE, VL_MAA_BKCOLOUR, (INT)dColour );

		DeleteBrush( ghBkBrush );
		ghBkBrush = CreateSolidBrush( dColour );

		InvalidateRect( ghItemsWnd, NULL, TRUE );

		return S_OK;
	}

	return E_ABORT;
}

UINT AaItemsIsUnderCursor( HWND hWnd, HWND hChdWnd, INT zDelta )
{
	UINT	dCode;

	if( ghItemsWnd != hChdWnd )	return 0;

	if( 0 < zDelta )		dCode = SB_LINEUP;
	else if( 0 > zDelta )	dCode = SB_LINEDOWN;
	else					dCode = SB_ENDSCROLL;

	FORWARD_WM_VSCROLL( hWnd, ghScrollWnd, dCode, 0, PostMessage );

	return 1;
}

UINT AaItemsDoSelect( HWND hWnd, UINT dMode, UINT dDirct )
{
	LPSTR		pcConts = NULL;
	UINT		uRslt;
	UINT_PTR	rdLength;

	if( dDirct ){	pcConts = AacAsciiArtGet( gixNowSel );	}
	else{			pcConts = AacAsciiArtGet( gixTopItem );	}

	if( !pcConts  ){	return 0;	}

	rdLength = strlen( pcConts );

	uRslt = ViewMaaMaterialise( hWnd, pcConts, rdLength, dMode );

	if( SUCCEEDED( AaItemsFavUpload( pcConts, rdLength ) ) )
	{
		FavContsRedrawRequest( hWnd );
	}

	free( pcConts );

	return 1;
}

HRESULT AaItemsTipSizeChange( INT ttSize, UINT bView )
{
#ifdef MAA_TOOLTIP
	LOGFONT	stFont;
#endif
	gbAAtipView = bView ? TRUE : FALSE;

#ifdef MAA_TOOLTIP
	SetWindowFont( ghToolTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
	DeleteFont( ghTipFont );

	ViewingFontGet( &stFont );
	stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
	ghTipFont = CreateFontIndirect( &stFont );

	SetWindowFont( ghToolTipWnd, ghTipFont, TRUE );
#endif

	return S_OK;
}

#ifdef USE_HOVERTIP

LPTSTR CALLBACK AaItemsHoverTipInfo( LPVOID pVoid )
{
	UINT_PTR	rdLength;
	LPSTR		pcConts = NULL;
	LPTSTR		ptBuffer = NULL;

	if( !(gbAAtipView) ){		return NULL;	}
	if( 0 > gixNowToolTip ){	return NULL;	}

	pcConts = AacAsciiArtGet( gixNowToolTip );
	if( !pcConts  ){	return 0;	}

	ptBuffer = SjisDecodeAlloc( pcConts );
	rdLength = lstrlen( ptBuffer  );

	free( pcConts );

	TRACE( TEXT("MAA HOVER CALL %d, by[%d]"), gixNowToolTip, rdLength );

	return ptBuffer;
}

#endif

#ifdef MAA_TEXT_FIND

HRESULT AacFindTextEntry( HWND hWnd, UINT bMode )
{
	TCHAR	atString[MAX_STRING];
	UINT	isNowPage, dRslt;
	INT		iPage, i;

	TRACE( TEXT("MAA：検索始め") );

	ZeroMemory( atString, sizeof(atString) );

	SetDlgItemText( hWnd, IDS_MAA_TXTFIND_MSGBOX, TEXT("") );

	if( bMode )
	{
		GetDlgItemText( hWnd, IDE_MAA_TXTFIND_TEXT, atString, MAX_STRING );

		if( NULL == atString[0] )	return E_NOTIMPL;

		isNowPage = IsDlgButtonChecked( hWnd, IDB_MAA_TXTFIND_TOP_GO ) ? FALSE : TRUE;
	}
	else
	{

		if( NULL == gatFindText[0] )	return E_NOTIMPL;

		StringCchCopy( atString, MAX_STRING, gatFindText );
	}

	if( StrCmp( gatFindText, atString ) )
	{

		StringCchCopy( gatFindText, MAX_STRING, atString );

		if( !(isNowPage)  ){	iPage = 0;	}
		else{	iPage =  gixTopItem + 1;	}
		if( gixMaxItem <= iPage ){	iPage = 0;	}

	}
	else
	{
		iPage =  gixTopItem + 1;
		if( gixMaxItem <= iPage ){	iPage = 0;	}
	}

	dRslt = FALSE;

	for( i = 0; gixMaxItem > i; i++ )
	{
		dRslt = AacItemFindOnePage( hWnd, atString, iPage );
		if( dRslt ){	break;	}

		iPage++;
		if( gixMaxItem <= iPage ){	iPage = 0;	}
	}

	if( dRslt )
	{
		Aai_OnVScroll( hWnd, ghScrollWnd, SBP_DIRECT, iPage );
	}
	else
	{
		SetDlgItemText( hWnd, IDS_MAA_TXTFIND_MSGBOX, TEXT("見つからないよ") );

	}

	return S_OK;
}

UINT AacItemFindOnePage( HWND hWnd, LPTSTR ptFindText, INT iTargetPage )
{
	LPSTR	pcItem;
	LPTSTR	ptTarget, ptFindPos;
	INT		iMoziPos;
	UINT	dFound = FALSE;

	pcItem = AacAsciiArtGet( iTargetPage );

	ptTarget = SjisDecodeAlloc( pcItem );
	FREE( pcItem );

	ptFindPos = FindStringProc( ptTarget, ptFindText, &iMoziPos );
	if( ptFindPos ){	dFound = TRUE;	}

	FREE( ptTarget );

	return dFound;
}

#endif

#ifndef MAA_IADD_PLUS

#ifndef _ORRVW

typedef struct tagITEMADDINFO
{
	LPTSTR	ptContent;
	TCHAR	atSep[MAX_PATH];
	INT		bType;

} ITEMADDINFO, *LPITEMADDINFO;

HRESULT AacItemAdding( HWND hWnd, LPTSTR ptFile )
{
	HANDLE	hFile;

	LPTSTR		ptBuffer;
	LPSTR		pcOutput;

	CHAR		acCheck[6];
	DWORD		readed, wrote;
	UINT_PTR	cchSize, cchSep, cbSize;
	ITEMADDINFO	stIaInfo;

	ZeroMemory( &stIaInfo, sizeof(ITEMADDINFO) );

	ZeroMemory( acCheck, sizeof(acCheck) );

	if( FileExtensionCheck( ptFile, TEXT(".ast") ) ){	stIaInfo.bType =  0;	}
	else{	stIaInfo.bType =  1;	}

	if( DialogBoxParam( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAA_IADD_DLG), hWnd, AaItemAddDlgProc, (LPARAM)(&stIaInfo) ) )
	{
		if( stIaInfo.ptContent )
		{
			hFile = CreateFile( ptFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if( INVALID_HANDLE_VALUE != hFile )
			{

				SetFilePointer( hFile, -2, NULL, FILE_END );
				ReadFile( hFile, acCheck, 6, &readed, NULL );
				SetFilePointer( hFile,  0, NULL, FILE_END );
				if( acCheck[0] != '\r' || acCheck[1] != '\n' )
				{
					acCheck[0] = '\r';	acCheck[1] = '\n';	acCheck[2] = NULL;
					WriteFile( hFile, acCheck, 2, &wrote, NULL );
				}

				StringCchLength( stIaInfo.atSep, MAX_PATH, &cchSep );
				StringCchLength( stIaInfo.ptContent, STRSAFE_MAX_CCH, &cchSize );
				cchSize += (cchSep+6);
				ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
				ZeroMemory( ptBuffer, cchSize * sizeof(TCHAR) );

				StringCchPrintf( ptBuffer, cchSize, TEXT("%s%s\r\n"), stIaInfo.atSep, stIaInfo.ptContent );
				pcOutput = SjisEncodeAlloc( ptBuffer );
				cbSize = strlen( pcOutput );

				WriteFile( hFile , pcOutput, cbSize, &wrote, NULL );

				SetEndOfFile( hFile );
				CloseHandle( hFile );
			}

			FREE(stIaInfo.ptContent);
		}

	}

	return S_OK;
}

INT_PTR CALLBACK AaItemAddDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPITEMADDINFO	pstIaInfo;
	static LPTSTR	ptBuffer;
	UINT_PTR	cchSize;
	TCHAR	atName[MAX_PATH];
	INT		id;
	RECT	rect;

	switch( message )
	{
		case WM_INITDIALOG:
			pstIaInfo = (LPITEMADDINFO)(lParam);
			GetClientRect( hDlg, &rect );
			CreateWindowEx( 0, WC_BUTTON, TEXT("今の頁"),         WS_CHILD | WS_VISIBLE, 0, 0, 75, 23, hDlg, (HMENU)IDB_MAID_NOWPAGE, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("クリップボード"), WS_CHILD | WS_VISIBLE, 75, 0, 120, 23, hDlg, (HMENU)IDB_MAID_CLIPBOARD, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 195, 0, rect.right-195-50, 23, hDlg, (HMENU)IDE_MAID_ITEMNAME, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("追加"),           WS_CHILD | WS_VISIBLE, rect.right-50, 0, 50, 23, hDlg, (HMENU)IDB_MAID_ADDGO, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY, 0, 23, rect.right, rect.bottom-23, hDlg, (HMENU)IDE_MAID_CONTENTS, GetModuleHandle(NULL), NULL );

			if( pstIaInfo->bType )
			{
				SetDlgItemText( hDlg, IDE_MAID_ITEMNAME, TEXT("名称はASTでないと使用できないのです") );
				EnableWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), FALSE );
				StringCchCopy( pstIaInfo->atSep, MAX_PATH, TEXT("[SPLIT]\r\n") );
			}

			ptBuffer = DocClipboardDataGet( NULL );
			if( !(ptBuffer) ){	DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );	}

			SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDCANCEL:
					FREE(ptBuffer);
					EndDialog(hDlg, 0 );
					return (INT_PTR)TRUE;

				case IDB_MAID_ADDGO:
					if( ptBuffer )
					{
						StringCchLength( ptBuffer, STRSAFE_MAX_CCH, &cchSize );
						cchSize += 2;
						pstIaInfo->ptContent = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
						StringCchCopy( pstIaInfo->ptContent, cchSize, ptBuffer );

						if( !(pstIaInfo->bType) )
						{
							GetDlgItemText( hDlg, IDE_MAID_ITEMNAME, atName, MAX_PATH );
							StringCchPrintf( pstIaInfo->atSep, MAX_PATH, TEXT("[AA][%s]\r\n"), atName );
						}
					}
					FREE(ptBuffer);
					EndDialog(hDlg, 1 );
					return (INT_PTR)TRUE;

				case IDB_MAID_CLIPBOARD:
					FREE(ptBuffer);
					ptBuffer = DocClipboardDataGet( NULL );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				case IDB_MAID_NOWPAGE:
					FREE(ptBuffer);
					DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		case WM_SIZE:
			GetClientRect( hDlg, &rect );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), 195, 0, rect.right-195-50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDB_MAID_ADDGO),    rect.right-50, 0, 50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_CONTENTS), 0, 23, rect.right, rect.bottom-23, TRUE );
			break;
	}

	return (INT_PTR)FALSE;
}

#endif

#endif
