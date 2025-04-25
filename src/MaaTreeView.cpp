#ifdef EXTRA_NODE_STYLE

#endif

#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"

#define NODE_DIR	1
#define NODE_FILE	0
#define NODE_EXTRA	(-1)

#define TICO_DIR_CLOSE	0
#define TICO_DIR_OPEN	1
#define TICO_DIR_EXTRA	2
#define TICO_FILE_AST	3
#define TICO_FILE_MLT	4
#define TICO_FILE_ETC	5

typedef struct tagMULTIPLEMAA
{
	INT		dTabNum;
	TCHAR	atFilePath[MAX_PATH];
	TCHAR	atBaseName[MAX_PATH];
	TCHAR	atDispName[MAX_PATH];

	UINT	dLastTop;

} MULTIPLEMAA, *LPMULTIPLEMAA;

extern  HWND		ghSplitaWnd;

extern HMENU		ghProfHisMenu;

static HFONT		ghTabFont;

static  HWND		ghTabWnd;

static  HWND		ghFavLtWnd;

static  HWND		ghTreeWnd;
static HTREEITEM	ghTreeRoot;

static TCHAR		gatAARoot[MAX_PATH];
static TCHAR		gatBaseName[MAX_PATH];

static INT			gixUseTab;

#ifdef HUKUTAB_DRAGMOVE
static POINT		gstMouseDown;
static INT			giDragSel;
static BOOLEAN		gbTabDraging;
#endif

static WNDPROC	gpfOriginFavListProc;
static WNDPROC	gpfOriginTreeViewProc;
static WNDPROC	gpfOriginTabMultiProc;

static list<MULTIPLEMAA>	gltMultiFiles;
typedef  list<MULTIPLEMAA>::iterator	MLTT_ITR;

HRESULT	TreeItemFromSqlII( HTREEITEM  );

#ifdef EXTRA_NODE_STYLE
UINT	TreeNodeExtraAdding( LPCTSTR  );
HRESULT	TreeExtraItemFromSql( HTREEITEM, UINT );
#endif

VOID	Mtv_OnMButtonUp( HWND, INT, INT, UINT );
VOID	Mtv_OnDropFiles( HWND , HDROP );

HRESULT	TabMultipleRestore( HWND  );
INT		TabMultipleSelect( HWND, INT, UINT );

HRESULT	TabMultipleDelete( HWND, CONST INT );
INT		TabMultipleAppend( HWND );

HRESULT	TabMultipleNameChange( HWND , INT );

HRESULT	TabLineMultiSingleToggle( HWND );

UINT	TabMultipleIsFavTab( INT, LPTSTR, UINT_PTR );

LRESULT	CALLBACK gpfFavListProc(  HWND , UINT, WPARAM, LPARAM );
LRESULT	CALLBACK gpfTreeViewProc( HWND , UINT, WPARAM, LPARAM );
LRESULT	CALLBACK gpfTabMultiProc( HWND , UINT, WPARAM, LPARAM );

VOID	Mtb_OnMButtonUp( HWND, INT, INT, UINT );

VOID	Mtb_OnLButtonDblclk( HWND, BOOL, INT, INT, UINT );

#ifdef HUKUTAB_DRAGMOVE
VOID	TabMultipleOnLButtonDown( HWND, INT, INT, UINT );
VOID	TabMultipleOnMouseMove(   HWND, INT, INT, UINT );
VOID	TabMultipleOnLButtonUp(   HWND, INT, INT, UINT );
#endif

HRESULT TreeInitialise( HWND hWnd, HINSTANCE hInst, LPRECT ptRect )
{
	TCITEM		stTcItem;
	RECT		itRect, clRect;

	DWORD		dwStyles;

	HIMAGELIST	hTreeImgList;
	HICON	hIcon;

	if( !(hWnd) )
	{
		SetWindowFont( ghTabWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
		DeleteFont( ghTabFont );

		TabMultipleStore( hWnd );

		return S_OK;
	}

	ghTabFont = CreateFont( 14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, TEXT("MS UI Gothic") );

	gixUseTab = ACT_ALLTREE;

	ZeroMemory( gatAARoot, sizeof(gatAARoot) );

	GetClientRect( hWnd, &clRect );

#ifdef HUKUTAB_DRAGMOVE
	gbTabDraging = FALSE;
#endif

	dwStyles = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_RIGHTJUSTIFY;

	if( !(InitParamValue( INIT_LOAD, VL_MAATAB_SNGL, 0 )) ){	dwStyles |= TCS_MULTILINE;	}

	ghTabWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("treetab"), dwStyles, 0, 0, TREE_WIDTH, 0, hWnd, (HMENU)IDTB_TREESEL, hInst, NULL );
	SetWindowFont( ghTabWnd, ghTabFont, FALSE );

	ZeroMemory( &stTcItem, sizeof(stTcItem) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = TEXT("全て");	TabCtrl_InsertItem( ghTabWnd, 0, &stTcItem );
	stTcItem.pszText = TEXT("使用");	TabCtrl_InsertItem( ghTabWnd, 1, &stTcItem );

	TabCtrl_GetItemRect( ghTabWnd, 1, &itRect );
	itRect.bottom -= itRect.top;

	itRect.right -= itRect.left;
	itRect.top  = 0;
	itRect.left = 0;
	TabCtrl_AdjustRect( ghTabWnd, 0, &itRect );

	MoveWindow( ghTabWnd, 0, 0, clRect.right, itRect.top, TRUE );

	gpfOriginTabMultiProc = SubclassWindow( ghTabWnd, gpfTabMultiProc );

	ghFavLtWnd = CreateWindowEx( WS_EX_CLIENTEDGE, WC_LISTBOX, TEXT("favlist"),
		WS_CHILD | WS_VSCROLL | LBS_NOTIFY | LBS_SORT | LBS_NOINTEGRALHEIGHT,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDLB_FAVLIST, hInst, NULL );

	gpfOriginFavListProc = SubclassWindow( ghFavLtWnd, gpfFavListProc );

	ghTreeWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_TREEVIEW, TEXT("itemtree"),
		WS_VISIBLE | WS_CHILD | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDTV_ITEMTREE, hInst, NULL );

	gpfOriginTreeViewProc = SubclassWindow( ghTreeWnd, gpfTreeViewProc );

	hTreeImgList = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 6, 0 );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_DIR_CLOSE) );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_DIR_OPEN)  );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_DIR_EXTRA) );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_FILE_AST)  );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_FILE_MLT)  );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_FILE_ETC)  );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	TreeView_SetImageList( ghTreeWnd, hTreeImgList, TVSIL_NORMAL );

	return S_OK;
}

LRESULT CALLBACK gpfFavListProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	UINT	ulRslt;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,    Maa_OnChar  );
		HANDLE_MSG( hWnd, WM_COMMAND, Maa_OnCommand );

		HANDLE_MSG( hWnd, WM_KEYDOWN, Aai_OnKey );
		HANDLE_MSG( hWnd, WM_KEYUP,   Aai_OnKey );

		case WM_MOUSEWHEEL:
			ulRslt = Maa_OnMouseWheel( hWnd, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (int)(short)HIWORD(wParam), (UINT)(short)LOWORD(wParam) );
			if( ulRslt )	return 0;
			break;

		default:	break;
	}

	return CallWindowProc( gpfOriginFavListProc, hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK gpfTreeViewProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	UINT	ulRslt;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,      Maa_OnChar  );
		HANDLE_MSG( hWnd, WM_COMMAND,   Maa_OnCommand );

		HANDLE_MSG( hWnd, WM_KEYDOWN,   Aai_OnKey );
		HANDLE_MSG( hWnd, WM_KEYUP,     Aai_OnKey );

		HANDLE_MSG( hWnd, WM_MBUTTONUP, Mtv_OnMButtonUp );
		HANDLE_MSG( hWnd, WM_DROPFILES, Mtv_OnDropFiles );

		case WM_MOUSEWHEEL:
			ulRslt = Maa_OnMouseWheel( hWnd, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (int)(short)HIWORD(wParam), (UINT)(short)LOWORD(wParam) );
			if( ulRslt )	return 0;
			break;

		default:	break;
	}

	return CallWindowProc( gpfOriginTreeViewProc, hWnd, msg, wParam, lParam );
}

VOID Mtv_OnMButtonUp( HWND hWnd, INT x, INT y, UINT flags )
{
	INT	iRslt;
	HTREEITEM	hTreeItem;
	TVHITTESTINFO	stTvItemInfo;

	TRACE( TEXT("ツリービューで中クルック[%d x %d]"), x, y );

	ZeroMemory( &stTvItemInfo, sizeof(TVHITTESTINFO) );
	stTvItemInfo.pt.x = x;
	stTvItemInfo.pt.y = y;

	hTreeItem = TreeView_HitTest( ghTreeWnd, &stTvItemInfo );

	iRslt = TreeSelItemProc( GetParent( hWnd ), hTreeItem, 1 );

	return;
}

VOID Mtv_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];

	BOOL	bRslt;

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	bRslt = PathIsDirectory( atFileName );

	TRACE( TEXT("MTV DROP[%u][%s]"), bRslt, atFileName );

	if( bRslt ){	 return;	}

#ifdef EXTRA_NODE_STYLE

	TreeNodeExtraAdding( atFileName );

#endif

	return;
}

LRESULT	CALLBACK gpfTabMultiProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,    Maa_OnChar  );
		HANDLE_MSG( hWnd, WM_COMMAND, Maa_OnCommand );

		HANDLE_MSG( hWnd, WM_KEYDOWN, Aai_OnKey );
		HANDLE_MSG( hWnd, WM_KEYUP,   Aai_OnKey );

		HANDLE_MSG( hWnd, WM_MBUTTONUP, Mtb_OnMButtonUp );

		HANDLE_MSG( hWnd, WM_LBUTTONDBLCLK, Mtb_OnLButtonDblclk );

#ifdef HUKUTAB_DRAGMOVE
		case WM_LBUTTONDOWN:	TabMultipleOnLButtonDown( hWnd, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(wParam) );	break;
		case WM_MOUSEMOVE:		TabMultipleOnMouseMove(   hWnd, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(wParam) );	break;
		case WM_LBUTTONUP:		TabMultipleOnLButtonUp(   hWnd, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(wParam) );	break;
#endif

		default:	break;
	}

	return CallWindowProc( gpfOriginTabMultiProc, hWnd, msg, wParam, lParam );
}

VOID Mtb_OnLButtonDblclk( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	INT	curSel;

	curSel = TabCtrl_GetCurSel( ghTabWnd );

	TRACE( TEXT("TAB DBLCLICK [%d] [%d x %d]"), curSel, x, y );

	if( 1 >= curSel ){	 return;	}

	TabMultipleNameChange( hWnd, curSel );

#pragma message ("ダブルクルックの機能を設定出来るようにするとおいしいかも")

	return;
}

VOID Mtb_OnMButtonUp( HWND hWnd, INT x, INT y, UINT flags )
{
	INT	curSel;
	TCHITTESTINFO	stTcHitInfo;

	stTcHitInfo.pt.x = x;
	stTcHitInfo.pt.y = y;
	curSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );

	TRACE( TEXT("MTAB start TAB [%d] [%d x %d]"), curSel, x, y );

	if( 1 >= curSel ){	 return;	}

	TabMultipleDelete( GetParent( ghTabWnd ), curSel );

	return;
}

#ifdef HUKUTAB_DRAGMOVE

VOID TabMultipleOnLButtonDown( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TCHITTESTINFO	stTcHitInfo;

	TRACE( TEXT("MTAB LDOWN [%d x %d]"), x, y );

	gstMouseDown.x = x;
	gstMouseDown.y = y;

	stTcHitInfo.pt = gstMouseDown;
	giDragSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );

	TRACE( TEXT("MTAB start TAB [%d]"), giDragSel );

	return;
}

VOID TabMultipleOnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	INT	mx, my, rx, ry;

	if( 1 >= giDragSel )	return;

	if( (keyFlags & MK_LBUTTON) && !(gbTabDraging) )
	{

		mx = abs( gstMouseDown.x - x );
		my = abs( gstMouseDown.y - y );

		rx = GetSystemMetrics( SM_CXDRAG );
		ry = GetSystemMetrics( SM_CYDRAG );

		if( rx < mx || ry < my )
		{
			TRACE( TEXT("MTAB start DRAG [%d x %d] [%d x %d]"), rx, ry, mx, my );
			SetCapture( hWnd  );
			gbTabDraging = TRUE;
		}

	}

	return;
}

VOID TabMultipleOnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	INT	iDragSel;
	POINT	point;
	TCHITTESTINFO	stTcHitInfo;

	TRACE( TEXT("MTAB LUP [%d x %d]"), x, y );
	point.x = x;
	point.y = y;

	if( gbTabDraging )
	{
		stTcHitInfo.pt = point;
		iDragSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );
		TRACE( TEXT("MTAB end TAB [%d]"), iDragSel );

		ReleaseCapture(  );
		gbTabDraging = FALSE;
	}

	return;
}

#endif

VOID Maa_OnChar( HWND hWnd, TCHAR ch, INT cRepeat )
{
	BOOLEAN	bShift;
	NMHDR	stNmHdr;
	INT		iTabs, iTarget;

	bShift = (0x8000 & GetKeyState(VK_SHIFT)) ? TRUE : FALSE;

	TRACE( TEXT("CHAR[%04X][%d]"), ch, bShift );

	if( VK_RETURN == ch )
	{
		AaItemsDoSelect( hWnd, MAA_DEFAULT, FALSE );
		return;
	}

	if( VK_TAB != ch ){	return;	}

	iTabs = TabCtrl_GetItemCount( ghTabWnd );

	if( bShift )
	{
		iTarget = gixUseTab - 1;
		if( 0 > iTarget ){	iTarget = iTabs - 1;	}
	}
	else
	{
		iTarget = gixUseTab + 1;
		if( iTabs <=  iTarget ){	iTarget = 0;	}
	}

	TabCtrl_SetCurSel( ghTabWnd, iTarget );

	stNmHdr.hwndFrom = ghTabWnd;
	stNmHdr.idFrom   = IDTB_TREESEL;
	stNmHdr.code     = TCN_SELCHANGE;

	TabBarNotify( hWnd, &stNmHdr );

	return;
}

VOID Maa_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		curSel, iRslt;
	TCHAR	atText[MAX_PATH], atName[MAX_PATH];
	LPARAM	lPrm;
	DWORD	dwStyles;
#ifdef EXTRA_NODE_STYLE
	LPARAM	iSelID = 0;
#endif
	UINT_PTR	cchSize;
	LONG_PTR	rdExStyle;
	TCHAR	atSelName[MAX_PATH], atMenuStr[MAX_PATH], atMenuStr2[MAX_PATH];
	MULTIPLEMAA		stMulti;
	POINT			stPost;
	TVHITTESTINFO	stTvHitInfo;
	TCHITTESTINFO	stTcHitInfo;
	TCITEM			stTcItem;
	MENUITEMINFO	stMenuItemInfo;

	HTREEITEM		hTvHitItem;

	stPost.x = (SHORT)xPos;
	stPost.y = (SHORT)yPos;

	TRACE( TEXT("MAAコンテキストメニュー") );

	if( ghFavLtWnd == hWndContext )
	{
		ZeroMemory( atSelName, sizeof(atSelName) );
		ZeroMemory( atMenuStr, sizeof(atMenuStr) );

		curSel = ListBox_GetCurSel( ghFavLtWnd );
		TRACE( TEXT("リストボックスコンテキスト %d"), curSel );
		if( 0 > curSel )	return;

		ListBox_GetText( ghFavLtWnd, curSel, atSelName );
		StringCchPrintf( atMenuStr,  MAX_PATH, TEXT("[ %s ]で副タブを追加"), atSelName );
		StringCchPrintf( atMenuStr2, MAX_PATH, TEXT("[ %s ]グループを削除"), atSelName );

		hMenu = CreatePopupMenu(  );

		AppendMenu( hMenu, MF_STRING, IDM_AATREE_SUBADD, atMenuStr );
		AppendMenu( hMenu, MF_SEPARATOR, 0, TEXT("----") );
		AppendMenu( hMenu, MF_STRING, IDM_MAA_FAVFLDR_DELETE, atMenuStr2 );

		dRslt = TrackPopupMenu( hMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );
		switch( dRslt )
		{
			case IDM_AATREE_SUBADD:
				ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
				StringCchCopy( stMulti.atBaseName, MAX_PATH, atSelName );

				stMulti.dTabNum = 0;

				gltMultiFiles.push_back( stMulti );
				TabMultipleAppend( hWnd );
				break;

			case IDM_MAA_FAVFLDR_DELETE:
				SqlFavFolderDelete( atSelName );

				while( ListBox_GetCount( ghFavLtWnd ) ){	ListBox_DeleteString( ghFavLtWnd, 0 );	}
				SqlFavFolderEnum( FavListFolderNameBack );
				break;

			default:	break;
		}
		DestroyMenu( hMenu );
		return;
	}

	if( ghTreeWnd == hWndContext )
	{
		hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AATREE_POPUP) );
		hSubMenu = GetSubMenu( hMenu, 0 );

		stTvHitInfo.pt = stPost;
		ScreenToClient( ghTreeWnd, &(stTvHitInfo.pt) );
		hTvHitItem = TreeView_HitTest( ghTreeWnd, &stTvHitInfo );

		if( hTvHitItem )
		{

			lPrm = TreeItemInfoGet( hTvHitItem, atName, MAX_PATH );

#ifdef EXTRA_NODE_STYLE

			if( NODE_DIR != lPrm && NODE_FILE != lPrm && NODE_EXTRA != lPrm )
			{
				iSelID = lPrm;
				lPrm = NODE_FILE;
				EnableMenuItem( hSubMenu, IDM_MAA_ITEM_DELETE, MF_ENABLED );

			}
#endif

			StringCchCat( atName, MAX_PATH, TEXT(" の操作") );

			ModifyMenu( hSubMenu, IDM_DUMMY, MF_BYCOMMAND | MF_STRING | MF_GRAYED, IDM_DUMMY, atName );
		}

		if( NODE_FILE != lPrm || !(hTvHitItem) )
		{
			EnableMenuItem( hSubMenu, IDM_AATREE_MAINOPEN, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_SUBADD,   MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT,   MF_GRAYED );

		}

		ModifyMenu( hSubMenu, IDM_OPEN_HISTORY, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)ghProfHisMenu, TEXT("ファイル使用履歴(&H)") );

#ifdef _ORRVW
		rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
		if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}
#endif

		dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );
		RemoveMenu( hSubMenu, 2, MF_BYPOSITION );
		DestroyMenu( hMenu );

		switch( dRslt )
		{

			case IDM_MAA_PROFILE_MAKE:	TreeProfileOpen( hWnd );	break;

			case IDM_TREE_RECONSTRUCT:	TreeProfileRebuild( hWnd  );	break;

			case IDM_FINDMAA_DLG_OPEN:	TreeMaaFileFind( hWnd );	break;

			case IDM_AATREE_MAINOPEN:	TreeSelItemProc( hWnd, hTvHitItem , 0 );	break;

			case  IDM_AATREE_SUBADD:	TreeSelItemProc( hWnd, hTvHitItem , 1 );	break;

#ifndef _ORRVW
			case  IDM_AATREE_GOEDIT:	TreeSelItemProc( hWnd, hTvHitItem , 2 );	break;
  #ifndef MAA_IADD_PLUS

  #endif
#endif
#ifdef EXTRA_NODE_STYLE
			case IDM_MAA_ITEM_DELETE:	TreeSelItemProc( hWnd, hTvHitItem , 4 );	break;
#endif

			case IDM_OPEN_HIS_CLEAR:	OpenProfileLogging( hWnd, NULL );	break;

			default:

				if( IDM_OPEN_HIS_FIRST <= dRslt && dRslt <= IDM_OPEN_HIS_LAST )
				{
					OpenProfileLoad( hWnd, dRslt );
				}
#ifdef _ORRVW
				else
				{
					Maa_OnCommand( hWnd, dRslt, hWndContext, 0 );
				}
#endif
				break;
		}

		return;
	}

	if( ghTabWnd == hWndContext )
	{
		stTcHitInfo.pt = stPost;
		ScreenToClient( ghTabWnd, &(stTcHitInfo.pt) );
		curSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );

		if( 1 >= curSel )	return;
#pragma message ("MAAタブの固定タブの判別に注意")

		hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AATABS_POPUP) );
		hSubMenu = GetSubMenu( hMenu, 0 );

		ZeroMemory( &stTcItem, sizeof(TCITEM) );
		stTcItem.mask       = TCIF_TEXT;
		stTcItem.pszText    = atText;
		stTcItem.cchTextMax = MAX_PATH;
		TabCtrl_GetItem( ghTabWnd, curSel, &stTcItem );

		StringCchCat( atText, MAX_PATH, TEXT(" を閉じる(&Q)") );
		StringCchLength( atText, MAX_PATH, &cchSize );

		ZeroMemory( &stMenuItemInfo, sizeof(MENUITEMINFO) );
		stMenuItemInfo.cbSize     = sizeof(MENUITEMINFO);
		stMenuItemInfo.fMask      = MIIM_TYPE;
		stMenuItemInfo.fType      = MFT_STRING;
		stMenuItemInfo.cch        = cchSize;
		stMenuItemInfo.dwTypeData = atText;
		SetMenuItemInfo( hSubMenu, IDM_AATABS_DELETE, FALSE, &stMenuItemInfo );

		if( TabMultipleIsFavTab( curSel, NULL, 0 ) ){	EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT, MF_GRAYED );	}

		dwStyles = GetWindowStyle( ghTabWnd );
		if( !(TCS_MULTILINE & dwStyles) ){	CheckMenuItem( hSubMenu, IDM_AATABS_SINGLETAB, MF_CHECKED );	}

		dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
		switch( dRslt )
		{
			case  IDM_AATABS_DELETE:	TabMultipleDelete( hWnd, curSel );	break;
			case  IDM_AATREE_GOEDIT:	TabMultipleSelect( hWnd, curSel, 1 );	break;

			case  IDM_AATABS_ALLDELETE:
				iRslt = MessageBox( hWnd, TEXT("全ての副タブを閉じようとしてるよ。\r\n本当に閉じちゃっていいかい？"), TEXT("お燐からの確認"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );
				if( IDYES == iRslt ){	TabMultipleDeleteAll( hWnd );	}

			case IDM_AATABS_RENAME:	TabMultipleNameChange( hWnd, curSel );	break;

			case IDM_AATABS_SINGLETAB:	TabLineMultiSingleToggle( hWnd );	break;

			default:	break;
		}

		return;
	}

#ifndef _ORRVW

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_TEMPLATE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
	if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}

	dRslt = TrackPopupMenu( hSubMenu, 0, stPost.x, stPost.y, 0, hWnd, NULL );

	DestroyMenu( hMenu );
#endif
	return;
}

HRESULT TreeResize( HWND hWnd, LPRECT ptRect )
{
	RECT	rect, sptRect;

	MaaTabBarSizeGet( &rect );

	SplitBarPosGet( ghSplitaWnd, &sptRect );

	MoveWindow( ghFavLtWnd, 0, rect.bottom, sptRect.left, ptRect->bottom, TRUE );
	MoveWindow( ghTreeWnd,  0, rect.bottom, sptRect.left, ptRect->bottom, TRUE );

	return S_OK;
}

HRESULT TreeConstruct( HWND hWnd, LPCTSTR ptCurrent, BOOLEAN bSubTabReb )
{
	TVINSERTSTRUCT	stTreeIns;

	TCHAR	atRoote[MAX_PATH];

	ZeroMemory( gatAARoot, sizeof(gatAARoot) );
	StringCchCopy( gatAARoot, MAX_PATH, ptCurrent );

	StringCchPrintf( atRoote, MAX_PATH, TEXT("ROOT[%s]"), gatAARoot );

	StatusBarMsgSet( SBMAA_FILENAME, TEXT("ツリーを構築中です") );

	TreeView_DeleteAllItems( ghTreeWnd );

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent        = TVI_ROOT;
	stTreeIns.hInsertAfter   = TVI_SORT;
	stTreeIns.item.mask      = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	stTreeIns.item.pszText   = atRoote;
	stTreeIns.item.lParam    = NODE_DIR;
	stTreeIns.item.cChildren = 1;

	stTreeIns.item.iImage = I_IMAGECALLBACK;

	stTreeIns.item.iSelectedImage = I_IMAGECALLBACK;

	ghTreeRoot = TreeView_InsertItem( ghTreeWnd, &stTreeIns );

	if( 0 == ptCurrent[0] )
	{
		StatusBarMsgSet( SBMAA_FILENAME, TEXT("") );
		return E_INVALIDARG;
	}

	StatusBarMsgSet( SBMAA_FILENAME, TEXT("") );
	TreeView_Expand( ghTreeWnd, ghTreeRoot, TVE_EXPAND );

	if( bSubTabReb ){	TabMultipleRestore( hWnd  );	}

	return S_OK;
}

UINT TreeNodePathGet( HTREEITEM hNode, LPTSTR ptPath )
{
	UINT	i;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH];
	HTREEITEM	hParent;

	if( ghTreeRoot == hNode )
	{
		StringCchCopy( ptPath, MAX_PATH, gatAARoot );
		return 1;
	}

	TreeItemInfoGet( hNode, atName, MAX_PATH );

	for( i = 0; 12 > i; i++ )
	{
		hParent = TreeView_GetParent( ghTreeWnd, hNode );
		if( !(hParent) )	return 0;
		if( ghTreeRoot == hParent ){	break;	}

		TreeItemInfoGet( hParent, atPath, MAX_PATH );

		PathAppend( atPath, atName );
		StringCchCopy( atName, MAX_PATH, atPath );

		hNode = hParent;
	}

	StringCchCopy( atPath, MAX_PATH, gatAARoot );
	PathAppend( atPath, atName );

	StringCchCopy( ptPath, MAX_PATH, atPath );

	return 1;
}

#ifdef EXTRA_NODE_STYLE

UINT TreeNodeExtraAdding( LPCTSTR ptPath )
{
	UINT	id;
	LPARAM	lParam;
	HTREEITEM	hTreeRoot, hChildItem, hNextItem, hBuffItem;

	id = SqlTreeNodeExtraIsFileExist( ptPath );
	if( 0 < id )
	{
		MessageBox( GetDesktopWindow( ), TEXT("已に登録してあるみたいだよ。"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONINFORMATION );
		return id;
	}

	hTreeRoot = TreeView_GetRoot( ghTreeWnd );

	hChildItem = TreeView_GetChild( ghTreeWnd, hTreeRoot );
	hNextItem = NULL;

	do{

		lParam = TreeItemInfoGet( hChildItem, NULL, 0 );

		if( NODE_EXTRA == lParam  ){	break;	}

		hNextItem = TreeView_GetNextSibling( ghTreeWnd, hChildItem );
		if( hNextItem == hChildItem ){	hNextItem = NULL;	}
		hChildItem = hNextItem;

	}while( hChildItem );

	if( !(hChildItem) )	return 0;

	TreeView_Expand( ghTreeWnd, hChildItem, TVE_EXPAND );

	hBuffItem = TreeView_GetChild( ghTreeWnd, hChildItem );

	id = SqlTreeNodeExtraInsert( 0, ptPath );
	if( 0 >= id )	return 0;

	if( hBuffItem ){	TreeExtraItemFromSql( hChildItem, id-1 );	}
	else{	TreeView_Expand( ghTreeWnd, hChildItem, TVE_EXPAND );	}

	return id;
}

HRESULT TreeExtraItemFromSql( HTREEITEM hTreeParent, UINT dFinID )
{
	TCHAR	atPath[MAX_PATH], atNodeName[MAX_PATH];
	UINT	tgtID;
	INT		iFileType;

	HTREEITEM	hNewParent;
	TVINSERTSTRUCT	stTreeIns;

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = hTreeParent;
	stTreeIns.hInsertAfter = TVI_LAST;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	tgtID = dFinID;

	do{
		ZeroMemory( atPath, sizeof(atPath) );
		ZeroMemory( atNodeName, sizeof(atNodeName) );

		tgtID = SqlTreeNodeExtraSelect( 0, tgtID, atPath );
		if( 0 == tgtID )	break;

		if( FileExtensionCheck( atPath, TEXT(".ast") ) ){	iFileType = TICO_FILE_AST;	}
		else if( FileExtensionCheck( atPath, TEXT(".mlt") ) ){	iFileType = TICO_FILE_MLT;	}
		else{	iFileType = TICO_FILE_ETC;	}

		stTreeIns.item.iImage = iFileType;

		stTreeIns.item.iSelectedImage = iFileType;
		stTreeIns.item.pszText = PathFindFileName( atPath );

		stTreeIns.item.lParam    = tgtID;
		stTreeIns.item.cChildren = 0;
		stTreeIns.hInsertAfter   = TVI_LAST;
		hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );

	}while( tgtID );

	return S_OK;
}

#endif

HRESULT TreeItemFromSqlII( HTREEITEM hTreeParent )
{
	TCHAR	atPath[MAX_PATH], atCurrent[MAX_PATH], atNodeName[MAX_PATH];
	UINT	dPrntID, tgtID, type;
	INT		iFileType;

	HTREEITEM	hNewParent, hLastDir = TVI_FIRST;
	TVINSERTSTRUCT	stTreeIns;

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = hTreeParent;
	stTreeIns.hInsertAfter = TVI_LAST;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	ZeroMemory( atCurrent, sizeof(atCurrent) );
	TreeNodePathGet( hTreeParent, atCurrent );

	dPrntID = MaaSearchTreeID( hTreeParent );

	tgtID = 0;

#ifdef EXTRA_NODE_STYLE

	if( 0 == dPrntID )
	{
		StringCchCopy( atNodeName, MAX_PATH, EXTRA_NODE );

		stTreeIns.item.iImage = TICO_DIR_EXTRA;

		stTreeIns.item.iSelectedImage = TICO_DIR_EXTRA;
		stTreeIns.item.pszText = atNodeName;
		stTreeIns.item.lParam    = NODE_EXTRA;
		stTreeIns.item.cChildren = 1;
		stTreeIns.hInsertAfter   = hLastDir;
		hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		hLastDir = hNewParent;
	}
#endif

	do{
		ZeroMemory( atNodeName, sizeof(atNodeName) );
		tgtID = SqlChildNodePickUpID( dPrntID, tgtID, &type, atNodeName );
		if( 0 == tgtID )	break;

		StringCchCopy( atPath, MAX_PATH, atCurrent );
		PathAppend( atPath, atNodeName );

		stTreeIns.item.pszText = atNodeName;

		if( FILE_ATTRIBUTE_DIRECTORY == type )
		{
			stTreeIns.item.iImage         = I_IMAGECALLBACK;
			stTreeIns.item.iSelectedImage = I_IMAGECALLBACK;
			stTreeIns.item.lParam    = NODE_DIR;
			stTreeIns.item.cChildren = 1;
			stTreeIns.hInsertAfter   = hLastDir;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
			hLastDir = hNewParent;
		}
		else
		{
			if( FileExtensionCheck( atNodeName, TEXT(".ast") ) ){	iFileType = TICO_FILE_AST;	}
			else if( FileExtensionCheck( atNodeName, TEXT(".mlt") ) ){	iFileType = TICO_FILE_MLT;	}
			else{	iFileType = TICO_FILE_ETC;	}

			stTreeIns.item.iImage = iFileType;
			stTreeIns.item.iSelectedImage = iFileType;
			stTreeIns.item.lParam    = NODE_FILE;
			stTreeIns.item.cChildren = 0;
			stTreeIns.hInsertAfter   = TVI_LAST;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		}

	}while( tgtID );

	return S_OK;
}

INT MaaSearchTreeID( HTREEITEM hItem )
{
	TCHAR	atName[MAX_PATH];
	HTREEITEM	hPrntItem;
	UINT	dPrntID, ownID;

	TreeItemInfoGet( hItem, atName, MAX_PATH );

	hPrntItem = TreeView_GetParent( ghTreeWnd, hItem );

	if( !(hPrntItem)  ){	return 0;	}
	else{	dPrntID = MaaSearchTreeID( hPrntItem );	}

	ownID = SqlTreeFileGetOnParent( atName, dPrntID );

	return ownID;
}

HTREEITEM MaaSearchTreeItem( INT dOwnID )
{
	UINT	dType, dPrntID;
	TCHAR	atFileName[MAX_PATH], atCmprName[MAX_PATH];
	HTREEITEM	hPrntItem, hChildItem, hNextItem;

	ZeroMemory( atFileName, sizeof(atFileName) );
	dType   = 0;
	dPrntID = 0;

	SqlTreeNodePickUpID( dOwnID, &dType, &dPrntID, atFileName, 0x11 );

	if( dPrntID )
	{
		hPrntItem = MaaSearchTreeItem( dPrntID );
	}
	else
	{
		hPrntItem = ghTreeRoot;
	}

	if( !(hPrntItem) )	return NULL;

	TreeView_Expand( ghTreeWnd, hPrntItem, TVE_EXPAND );

	hChildItem = TreeView_GetChild( ghTreeWnd, hPrntItem );

	do{

		TreeItemInfoGet( hChildItem, atCmprName, MAX_PATH );

		if( !( StrCmp( atFileName, atCmprName ) ) )
		{

			break;
		}

		hNextItem = TreeView_GetNextSibling( ghTreeWnd, hChildItem );
		hChildItem = hNextItem;

	}while( hNextItem );

	return hChildItem;
}

HTREEITEM MaaSelectIDfile( HWND hDlg, INT tgtID )
{
	HTREEITEM	hTgtItem;

	hTgtItem = MaaSearchTreeItem( tgtID );

	if( hTgtItem ){	TreeView_SelectItem( ghTreeWnd, hTgtItem );	}

	return hTgtItem;
}

LPARAM TreeItemInfoGet( HTREEITEM hTrItem, LPTSTR ptName, size_t cchName )
{
	TCHAR	atBuffer[MAX_PATH];
	TVITEM	stTvItem;

	ZeroMemory( &stTvItem, sizeof(TVITEM) );
	ZeroMemory( atBuffer, sizeof(atBuffer) );

	stTvItem.hItem      = hTrItem;
	stTvItem.mask       = TVIF_TEXT | TVIF_PARAM;
	stTvItem.pszText    = atBuffer;
	stTvItem.cchTextMax = MAX_PATH;
	TreeView_GetItem( ghTreeWnd, &stTvItem );

	if( ptName )
	{
		ZeroMemory( ptName, sizeof(TCHAR) * cchName );
		StringCchCopy( ptName, cchName, atBuffer );
	}

	return stTvItem.lParam;
}

LRESULT TreeNotify( HWND hWnd, LPNMTREEVIEW pstNmTrView )
{
	INT		nmCode;

#ifdef EXTRA_NODE_STYLE
	LPARAM	lParam;
#endif

	HTREEITEM		hSelItem;
	LPTVITEM		pstTvItem;
	LPNMTVDISPINFO	pstDispInfo;

	nmCode = pstNmTrView->hdr.code;

	if( TVN_SELCHANGED == nmCode )
	{
		hSelItem = TreeView_GetSelection( ghTreeWnd );

		TreeSelItemProc( hWnd, hSelItem, 0 );
	}

	if( TVN_ITEMEXPANDING == nmCode )
	{
		TRACE( TEXT("TVN_ITEMEXPANDING[%X]"), pstNmTrView->action );

		if( pstNmTrView->action & TVE_EXPAND )
		{
			pstTvItem = &(pstNmTrView->itemNew);

			if( pstTvItem->state & TVIS_EXPANDEDONCE )	return 0;

#ifdef EXTRA_NODE_STYLE

			lParam = TreeItemInfoGet( pstTvItem->hItem, NULL, 0 );
			if( NODE_EXTRA == lParam )
			{
				TRACE( TEXT("TREE EX DIR") );
				TreeExtraItemFromSql( pstTvItem->hItem, 0 );
			}
			else
			{
#endif
				TreeItemFromSqlII( pstTvItem->hItem );
#ifdef EXTRA_NODE_STYLE
			}
#endif
		}

		if( pstNmTrView->action & TVE_COLLAPSE )
		{
			pstTvItem = &(pstNmTrView->itemNew);
			TRACE( TEXT("TVE_COLLAPSE[%d, %d]"), pstTvItem->iImage, pstTvItem->iSelectedImage );
		}
	}

	if( TVN_GETDISPINFO == nmCode )
	{
		pstDispInfo = (LPNMTVDISPINFO)pstNmTrView;
		TRACE( TEXT("TVN_GETDISPINFO[%X]"), pstDispInfo->item.mask );

		if(pstDispInfo->item.mask & TVIF_IMAGE || pstDispInfo->item.mask & TVIF_SELECTEDIMAGE)
		{
			if( pstDispInfo->item.state & TVIS_EXPANDED )
			{
				pstDispInfo->item.iImage         = TICO_DIR_OPEN;
				pstDispInfo->item.iSelectedImage = TICO_DIR_OPEN;
			}
			else
			{
				pstDispInfo->item.iImage         = TICO_DIR_CLOSE;
				pstDispInfo->item.iSelectedImage = TICO_DIR_CLOSE;
			}
		}
	}

	return 0;
}

INT TreeSelItemProc( HWND hWnd, HTREEITEM hSelItem, UINT dMode )
{
	UINT	i;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH], atBaseName[MAX_PATH];
	LPARAM	lParam;
	HTREEITEM	hParentItem;
	MULTIPLEMAA	stMulti;

#ifdef EXTRA_NODE_STYLE
	UINT	id = 0;
#endif

	if( !(hSelItem) ){	return 0;	}

#ifdef EXTRA_NODE_STYLE
	hParentItem = TreeView_GetParent( ghTreeWnd, hSelItem );
	lParam = TreeItemInfoGet( hParentItem, NULL, 0 );
	if( NODE_EXTRA == lParam )
	{

		lParam = TreeItemInfoGet( hSelItem, NULL, 0 );

		id = SqlTreeNodeExtraSelect( lParam, 0, atPath );
		if( 0 == id )	return 0;

		if( 0 == dMode )
		{
			StringCchCopy( gatBaseName, MAX_PATH, PathFindFileName( atPath ) );
			StatusBarMsgSet( SBMAA_FILENAME , gatBaseName );
		}
		StringCchCopy( atBaseName, MAX_PATH, PathFindFileName( atPath ) );
	}
	else
	{
#endif

		lParam = TreeItemInfoGet( hSelItem, atName, MAX_PATH );

		if( NODE_FILE != lParam ){	return 0;	}

		if( 0 == dMode )
		{
			StringCchCopy( gatBaseName, MAX_PATH, atName );
			StatusBarMsgSet( SBMAA_FILENAME, atName );
		}
		StringCchCopy( atBaseName, MAX_PATH, atName );

		for( i = 0; 12 > i; i++ )
		{
			hParentItem = TreeView_GetParent( ghTreeWnd, hSelItem );
			if( !(hParentItem) )	return 0;
			if( ghTreeRoot == hParentItem ){	break;	}

			TreeItemInfoGet( hParentItem, atPath, MAX_PATH );

			if( 0 == i )
			{
				if( 0 == dMode ){	StringCchCopy( gatBaseName, MAX_PATH, atPath );	}
				else{				StringCchCopy( atBaseName, MAX_PATH, atPath );	}
			}

			PathAppend( atPath, atName );
			StringCchCopy( atName, MAX_PATH, atPath );

			hSelItem = hParentItem;
		}

		StringCchCopy( atPath, MAX_PATH, gatAARoot );
		PathAppend( atPath, atName );

#ifdef EXTRA_NODE_STYLE
	}
#endif

	switch( dMode )
	{
		default:
		case  0:	AaItemsDoShow( hWnd , atPath, ACT_ALLTREE );	break;

		case  1:
			ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
			StringCchCopy( stMulti.atFilePath, MAX_PATH, atPath );
			StringCchCopy( stMulti.atBaseName, MAX_PATH, atBaseName );
			stMulti.dTabNum = 0;

			gltMultiFiles.push_back( stMulti );
			TabMultipleAppend( hWnd );
			break;
#ifndef _ORRVW
		case  2:
			DocDoOpenFile( hWnd, atPath );
			break;

  #ifndef MAA_IADD_PLUS

  #endif
#endif

#ifdef EXTRA_NODE_STYLE
		case  4:
			if( 0 < id )
			{
				TreeView_DeleteItem( ghTreeWnd , hSelItem );
				SqlTreeNodeExtraDelete( id );
			}
			break;
#endif
	}

	return 1;
}

LPTSTR TreeBaseNameGet( VOID )
{
	return gatBaseName;
}

VOID MaaTabBarSizeGet( LPRECT pstRect )
{

	RECT	itRect;
	LONG	height;

	assert( pstRect );

	pstRect->left   = 0;
	pstRect->top    = 0;

	TabCtrl_GetItemRect( ghTabWnd, 1, &itRect );
	itRect.bottom -= itRect.top;
	itRect.right -= itRect.left;
	itRect.top = 0;
	itRect.left = 0;
	TabCtrl_AdjustRect( ghTabWnd, 0, &itRect );
	height = itRect.top;
	GetWindowRect( ghTabWnd, &itRect );
	itRect.right -= itRect.left;

	pstRect->right  = itRect.right;
	pstRect->bottom = height;

	return;
}

VOID TabBarResize( HWND hWnd, LPRECT pstRect )
{
	RECT	tbRect;

	MoveWindow( ghTabWnd, 0, 0, pstRect->right, pstRect->bottom, TRUE );
	MaaTabBarSizeGet( &tbRect );
	MoveWindow( ghTabWnd, 0, 0, tbRect.right, tbRect.bottom, TRUE );

	return;
}

LRESULT TabBarNotify( HWND hWnd, LPNMHDR pstNmhdr )
{
	INT		nmCode;
	INT		curSel;

	NMTREEVIEW	stNmTrView;

	nmCode   = pstNmhdr->code;

	if( TCN_SELCHANGE == nmCode )
	{
		curSel = TabCtrl_GetCurSel( ghTabWnd );

		TRACE( TEXT("TAB sel [%d]"), curSel );

		ShowWindow( ghTreeWnd,  SW_HIDE );
		ShowWindow( ghFavLtWnd, SW_HIDE );

		if( ACT_ALLTREE == curSel )
		{

			ShowWindow( ghTreeWnd, SW_SHOW );
			gixUseTab = ACT_ALLTREE;

			ZeroMemory( &stNmTrView, sizeof(NMTREEVIEW) );
			stNmTrView.hdr.hwndFrom = ghTreeWnd;
			stNmTrView.hdr.idFrom   = IDTV_ITEMTREE;
			stNmTrView.hdr.code     = TVN_SELCHANGED;

			TreeNotify( hWnd, &stNmTrView );
		}
		else if( ACT_FAVLIST == curSel )
		{

			while( ListBox_GetCount( ghFavLtWnd ) ){	ListBox_DeleteString( ghFavLtWnd, 0 );	}
			SqlFavFolderEnum( FavListFolderNameBack );

			ShowWindow( ghFavLtWnd, SW_SHOW );
			gixUseTab = ACT_FAVLIST;
		}
		else
		{
			AaTitleClear(  );
			TabMultipleSelect( hWnd, curSel, 0 );
		}
	}

	return 0;
}

UINT TabMultipleIsFavTab( INT tabSel, LPTSTR ptBase, UINT_PTR cchSize )
{
	MLTT_ITR	itNulti;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( tabSel == itNulti->dTabNum )
		{

			if( ptBase ){	StringCchCopy( ptBase, cchSize, itNulti->atBaseName );	}

			if( NULL == itNulti->atFilePath[0] )	return 1;
			else	return 0;
		}
	}

	return 0;
}

INT TabMultipleSelect( HWND hWnd, INT tabSel, UINT dMode )
{
	MLTT_ITR	itNulti;
	TCHAR	atName[MAX_PATH];
#ifndef _ORRVW

#endif

	if( 0 == dMode )	gixUseTab = tabSel;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( tabSel == itNulti->dTabNum )
		{
			if( 0 == dMode )
			{

				StringCchCopy( gatBaseName, MAX_PATH, itNulti->atBaseName );

				if( NULL == itNulti->atFilePath[0] )
				{
					StringCchCopy( atName, MAX_PATH, gatBaseName );
					StringCchCat(  atName, MAX_PATH, TEXT("[F]") );

					AaItemsDoShow( hWnd, gatBaseName, ACT_FAVLIST );
				}
				else
				{

					StringCchCopy( atName, MAX_PATH, itNulti->atFilePath );
					PathStripPath( atName );

					AaItemsDoShow( hWnd, itNulti->atFilePath, ACT_SUBITEM );
				}

				StatusBarMsgSet( SBMAA_FILENAME, atName );
			}
#ifndef _ORRVW
			else
			{
				DocDoOpenFile( hWnd, itNulti->atFilePath );
			}
#endif
			return 1;
		}
	}

	return 0;
}

INT TabMultipleTopMemory( INT dTop )
{
	MLTT_ITR	itNulti;

	if( ACT_SUBITEM > gixUseTab )	return 0;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( gixUseTab == itNulti->dTabNum )
		{

			if( 0 >  dTop ){	dTop = itNulti->dLastTop;	}
			else{	itNulti->dLastTop = dTop;	}
			break;
		}
	}

	if( 0 > dTop )	dTop = 0;

	return dTop;
}

HRESULT TabMultipleStore( HWND hWnd )
{
	MLTT_ITR	itNulti;

	SqlMultiTabDelete(  );

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{

		if( StrCmp( DROP_OBJ_NAME, itNulti->atBaseName ) )
		{
			SqlMultiTabInsert( itNulti->atFilePath, itNulti->atBaseName, itNulti->atDispName );
		}
	}

	return S_OK;
}

HRESULT TabMultipleRestore( HWND hWnd )
{
	INT	iCount, i;
	MULTIPLEMAA	stMulti;

	TabMultipleDeleteAll( hWnd );

	iCount = SqlTreeCount( 2, NULL );

	for( i = 0; iCount > i; i++ )
	{
		ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );

		SqlMultiTabSelect( i+1, stMulti.atFilePath, stMulti.atBaseName, stMulti.atDispName );

		gltMultiFiles.push_back( stMulti );
		TabMultipleAppend( hWnd );
	}

	return S_OK;
}

HRESULT TabMultipleDropAdd( HWND hWnd, LPCTSTR ptFile )
{
	MULTIPLEMAA		stMulti;
	INT		iTabNum;

	ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
	StringCchCopy( stMulti.atFilePath, MAX_PATH, ptFile );
	StringCchCopy( stMulti.atBaseName, MAX_PATH, DROP_OBJ_NAME );
	stMulti.dTabNum = 0;

	gltMultiFiles.push_back( stMulti );
	iTabNum = TabMultipleAppend( hWnd );

	TabCtrl_SetCurSel( ghTabWnd, iTabNum );
	TabMultipleSelect( hWnd, iTabNum, 0 );

	return S_OK;
}

INT TabMultipleAppend( HWND hWnd )
{
	TCHAR	atName[MAX_PATH];
	LONG	tCount;

	TCITEM	stTcItem;

	MLTT_ITR	itNulti;

	itNulti = gltMultiFiles.end( );
	itNulti--;
	StringCchCopy( atName, MAX_PATH, itNulti->atFilePath );
	if( NULL !=  atName[0] )
	{
		PathStripPath( atName );
		PathRemoveExtension( atName );
	}
	else
	{
		StringCchCopy( atName, MAX_PATH, itNulti->atBaseName );
		StringCchCat(  atName, MAX_PATH, TEXT("[F]") );
	}

	if( NULL == itNulti->atDispName[0] )
	{	StringCchCopy( itNulti->atDispName , MAX_PATH, atName );	}

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;

	tCount = TabCtrl_GetItemCount( ghTabWnd );

	stTcItem.lParam  = 0;
	stTcItem.pszText = itNulti->atDispName;
	TabCtrl_InsertItem( ghTabWnd, tCount, &stTcItem );

	itNulti->dTabNum = tCount;

	Maa_OnSize( hWnd, 0, 0, 0 );

	return tCount;
}

INT TabMultipleNowSel( VOID )
{
	return gixUseTab;
}

HRESULT TabMultipleDeleteAll( HWND hWnd )
{
	INT	ttlSel, i;
	NMHDR	stNmHdr;

	ttlSel = TabCtrl_GetItemCount( ghTabWnd );

	for( i = (ttlSel-1); 2 <= i; i-- ){	TabCtrl_DeleteItem( ghTabWnd, i );	}

	gltMultiFiles.clear();

	TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
	stNmHdr.hwndFrom = ghTabWnd;
	stNmHdr.idFrom   = IDTB_TREESEL;
	stNmHdr.code     = TCN_SELCHANGE;
	TabBarNotify( hWnd, &stNmHdr );

	return S_OK;
}

HRESULT TabMultipleDelete( HWND hWnd, CONST INT tabSel )
{
	INT	nowSel, i;
	NMHDR	stNmHdr;
	MLTT_ITR	itNulti;

	nowSel = TabCtrl_GetCurSel( ghTabWnd );

	TRACE( TEXT("TAB del [%d][%d]"), nowSel, tabSel );

	TabCtrl_DeleteItem( ghTabWnd, tabSel );

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( tabSel == itNulti->dTabNum )
		{
			gltMultiFiles.erase( itNulti );
			break;
		}
	}

	i = 2;
	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		itNulti->dTabNum = i;
		i++;
	}

	if( nowSel == tabSel )
	{
		TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
		stNmHdr.hwndFrom = ghTabWnd;
		stNmHdr.idFrom   = IDTB_TREESEL;
		stNmHdr.code     = TCN_SELCHANGE;
		TabBarNotify( hWnd, &stNmHdr );
	}

	Maa_OnSize( hWnd, 0, 0, 0 );

	return S_OK;
}

HRESULT TabMultipleSelDelete( HWND hWnd )
{
	INT		curSel;

	curSel = TabCtrl_GetCurSel( ghTabWnd );

	TRACE( TEXT("VIEW FILE CLOSE [%d]"), curSel );

	if( 1 >= curSel )	return E_ACCESSDENIED;

	return TabMultipleDelete( hWnd, curSel );
}

HRESULT TabMultipleCtrlFromFind( HWND hWnd )
{
	NMHDR	stNmHdr;

	TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
	stNmHdr.hwndFrom = ghTabWnd;
	stNmHdr.idFrom   = IDTB_TREESEL;
	stNmHdr.code     = TCN_SELCHANGE;
	TabBarNotify( hWnd, &stNmHdr );

	return S_OK;
}

LRESULT CALLBACK FavListFolderNameBack( UINT dNumber, UINT dummy, UINT fake, LPCVOID ptFdrName )
{
	INT	iOrder;

	iOrder = ListBox_AddString( ghFavLtWnd, (LPCTSTR)ptFdrName );

	return 1;
}

HRESULT FavListSelected( HWND hWnd, UINT iCode )
{
	TCHAR	atFdrName[MAX_PATH];
	INT	selIndex;

	if( LBN_SELCHANGE == iCode )
	{
		selIndex = ListBox_GetCurSel( ghFavLtWnd );
		if( LB_ERR == selIndex )	return E_OUTOFMEMORY;

		ListBox_GetText( ghFavLtWnd, selIndex, atFdrName );

		StringCchCopy( gatBaseName, MAX_PATH, atFdrName );

		AaItemsDoShow( hWnd, atFdrName, ACT_FAVLIST );
	}

	return S_OK;
}

HRESULT FavContsRedrawRequest( HWND hWnd )
{

	if( ACT_FAVLIST == gixUseTab || TabMultipleIsFavTab( gixUseTab, NULL, 0 ) )
	{
		AaItemsDoShow( hWnd, gatBaseName, ACT_FAVLIST );
	}

	return S_OK;
}

UINT TreeFavIsUnderCursor( HWND hWnd, HWND hChdWnd, INT xPos, INT yPos, INT zDelta, UINT fwKeys )
{
	if( ghTreeWnd == hChdWnd )
	{
		FORWARD_WM_MOUSEWHEEL( ghTreeWnd, xPos, yPos, zDelta, fwKeys, PostMessage );
		TRACE( TEXT("TreeUnderCursor[%d]"), zDelta );
		return 1;
	}

	if( ghFavLtWnd == hChdWnd )
	{
		FORWARD_WM_MOUSEWHEEL( ghFavLtWnd, xPos, yPos, zDelta, fwKeys, PostMessage );
		TRACE( TEXT("FavUnderCursor[%d]"), zDelta );
		return 1;
	}

	return 0;
}

INT_PTR CALLBACK TabMultipleRenameDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPTSTR	cptName;
	TCHAR	atBuffer[MAX_PATH];

	switch( message )
	{
		case WM_INITDIALOG:
			cptName = (LPTSTR)lParam;
			Edit_SetText( GetDlgItem(hDlg,IDE_PAGENAME), cptName );
			SetFocus( GetDlgItem(hDlg,IDE_PAGENAME) );
			return (INT_PTR)FALSE;

		case WM_COMMAND:
			if( IDOK == LOWORD(wParam) )
			{
				Edit_GetText( GetDlgItem(hDlg,IDE_PAGENAME), atBuffer, MAX_PATH );
				StringCchCopy( cptName, MAX_PATH, atBuffer );
				EndDialog( hDlg, IDOK );
				return (INT_PTR)TRUE;
			}

			if( IDCANCEL == LOWORD(wParam) )
			{
				EndDialog( hDlg, IDCANCEL );
				return (INT_PTR)TRUE;
			}

			break;
	}
	return (INT_PTR)FALSE;
}

HRESULT TabMultipleNameChange( HWND hWnd, INT iTabSel )
{
	INT_PTR	iRslt;
	TCHAR	atName[MAX_PATH];
	TCITEM	stTcItem;
	MLTT_ITR	itNulti;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( iTabSel == itNulti->dTabNum )
		{
			StringCchCopy( atName, MAX_PATH, itNulti->atDispName );

			iRslt = DialogBoxParam( GetModuleHandle( NULL ), MAKEINTRESOURCE(IDD_PAGE_NAME_DLG), hWnd, TabMultipleRenameDlgProc, (LPARAM)atName );
			if( IDOK == iRslt )
			{
				StringCchCopy( itNulti->atDispName, MAX_PATH, atName );

				ZeroMemory( &stTcItem, sizeof(TCITEM) );
				stTcItem.mask = TCIF_TEXT;
				stTcItem.pszText = atName;
				TabCtrl_SetItem( ghTabWnd, iTabSel, &stTcItem );

				return S_OK;
			}

			return E_ABORT;
		}
	}

	return E_OUTOFMEMORY;
}

HRESULT TabLineMultiSingleToggle( HWND hWnd )
{
	 INT	doSingle;
	DWORD	dWndwStyle;

	dWndwStyle = GetWindowStyle( ghTabWnd );

	if( TCS_MULTILINE & dWndwStyle )
	{
		doSingle = 1;
		dWndwStyle &= ~TCS_MULTILINE;
	}
	else
	{
		doSingle = 0;
		dWndwStyle |= TCS_MULTILINE;
	}

	SetWindowLong( ghTabWnd, GWL_STYLE, dWndwStyle );

	InitParamValue( INIT_SAVE, VL_MAATAB_SNGL, doSingle );

	Maa_OnSize( hWnd, 0, 0, 0 );

	return S_OK;
}
