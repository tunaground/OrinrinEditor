#include "stdafx.h"
#include "OrinrinEditor.h"

static  HWND	ghRebarWnd;

static  HWND	ghMainTBWnd;
static  HWND	ghEditTBWnd;
static  HWND	ghInsertTBWnd;
static  HWND	ghLayoutTBWnd;
static  HWND	ghViewTBWnd;

static HIMAGELIST	ghMainImgLst;
static HIMAGELIST	ghEditImgLst;
static HIMAGELIST	ghInsertImgLst;
static HIMAGELIST	ghLayoutImgLst;
static HIMAGELIST	ghViewImgLst;

static WNDPROC	gpfOrigTBProc;

extern HMENU	ghHistyMenu;

static LRESULT	CALLBACK gpfToolbarProc( HWND, UINT, WPARAM, LPARAM );

#define TBT_STRING	32

#define TB_MAIN_ITEMS	5
static  TBBUTTON	gstMainTBInfo[] = {
	{  0,	IDM_NEWFILE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,						{0, 0}, 0, 0  },
	{  1,	IDM_OPEN,			TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  2,	IDM_OVERWRITESAVE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,						{0, 0}, 0, 0  },
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{  3,	IDM_GENERAL_OPTION,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,						{0, 0}, 0, 0  }
};

static CONST TCHAR	gatMainTBtext[TB_MAIN_ITEMS][TBT_STRING] = {
	{ TEXT("새로 만들기") },	{ TEXT("파일 열기") },	{ TEXT("덮어쓰기 저장") },	{ TEXT("") },
	{ TEXT("일반 설정") }
};

#define TB_EDIT_ITEMS	21
static  TBBUTTON	gstEditTBInfo[] = {
	{  0,	IDM_UNDO,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  1,	IDM_REDO,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{  2,	IDM_CUT,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  3,	IDM_COPY,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  4,	IDM_PASTE,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  5,	IDM_DELETE,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{  6,	IDM_SJISCOPY,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  7,	IDM_SJISCOPY_ALL,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{  8,	IDM_ALLSEL,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  9,	IDM_SQSELECT,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 10,	IDM_LAYERBOX,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 11,	IDM_PAGEL_DIVIDE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 12,	IDM_EXTRACTION_MODE,TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 13,	IDM_UNICODE_TOGGLE,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
};

static CONST TCHAR	gatEditTBtext[TB_EDIT_ITEMS][TBT_STRING] = {
	{ TEXT("되돌리기") },	{ TEXT("다시 하기") },	{ TEXT("") },
	{ TEXT("잘라내기") },	{ TEXT("복사") },	{ TEXT("붙여넣기") },	{ TEXT("삭제") },	{ TEXT("") },
	{ TEXT("SJIS 복사") },	{ TEXT("전체 SJIS 복사") },	{ TEXT("") },
	{ TEXT("전체 선택") },	{ TEXT("사각형 선택") },	{ TEXT("") },
	{ TEXT("레이어 박스") },	{ TEXT("") },
	{ TEXT("다음 행부터 새 페이지로 분할") },	{ TEXT("") },
	{ TEXT("부분 추출 모드") },	{ TEXT("") },
	{ TEXT("유니코드 공백 사용") }
};

#define  TB_INSERT_ITEMS	6
static  TBBUTTON	gstInsertTBInfo[] = {
	{  0, IDM_IN_UNI_SPACE,		TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  1, IDM_INSTAG_COLOUR,	TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  2, IDM_FRMINSBOX_OPEN,	TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  3, IDM_USERINS_NA,		TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{  4, IDM_MOZI_SCR_OPEN,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,						{0, 0}, 0, 0  }
};

static CONST TCHAR	gatInsertTBtext[TB_INSERT_ITEMS][TBT_STRING] = {
	{ TEXT("유니코드 공백") },	{ TEXT("색 변경 태그") },	{ TEXT("테두리") },	{ TEXT("사용자 정의") },	{ TEXT("") },
	{ TEXT("문자 AA 변환 박스") }
};

#define  TB_LAYOUT_ITEMS	19
static  TBBUTTON	gstLayoutTBInfo[] = {
	{  0, IDM_RIGHT_GUIDE_SET,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  1, IDM_INS_TOPSPACE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{  2, IDM_DEL_TOPSPACE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  3, IDM_DEL_LASTSPACE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  4, IDM_DEL_LASTLETTER,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{ 10, IDM_MIRROR_INVERSE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 11, IDM_UPSET_INVERSE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{  5, IDM_RIGHT_SLIDE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  6, IDM_INCREMENT_DOT,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  7, IDM_DECREMENT_DOT,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{  8, IDM_INCR_DOT_LINES,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  9, IDM_DECR_DOT_LINES,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{ 12, IDM_DOT_SPLIT_LEFT,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{ 13, IDM_DOT_SPLIT_RIGHT,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  }
};

static CONST TCHAR	gatLayoutTBtext[TB_LAYOUT_ITEMS][TBT_STRING] = {
	{ TEXT("오른쪽 정렬 가이드") },	{ TEXT("행 시작에 전각 공백 추가") },	{ TEXT("") },
	{ TEXT("행 시작 공백 삭제") },	{ TEXT("행 끝 공백 삭제") },	{ TEXT("행 끝 문자 삭제") },	{ TEXT("") },
	{ TEXT("좌우 반전") },	{ TEXT("상하 반전") },	{ TEXT("") },
	{ TEXT("오른쪽으로 밀기") },	{ TEXT("1 도트 증가") },	{ TEXT("1 도트 감소") },	{ TEXT("") },
	{ TEXT("전체를 1 도트 오른쪽으로") },	{ TEXT("전체를 1 도트 왼쪽으로") },	{ TEXT("") },
	{ TEXT("지정 위치에서 왼쪽으로 좁히기") },	{ TEXT("지정 위치에서 오른쪽으로 넓히기") }
};

#define TB_VIEW_ITEMS	5
static  TBBUTTON	gstViewTBInfo[] = {
	{  0, IDM_UNI_PALETTE,			TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  1, IDM_TRACE_MODE_ON,		TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },
	{  2, IDM_ON_PREVIEW,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  3, IDM_DRAUGHT_OPEN,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },
	{  4, IDM_MAA_THUMBNAIL_OPEN,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  }
};

static CONST TCHAR	gatViewTBtext[TB_VIEW_ITEMS][TBT_STRING] = {
	{ TEXT("유니코드 팔레트") },	{ TEXT("트레이스 모드") },	{ TEXT("미리보기") },	{ TEXT("드래프트 보드") },	{ TEXT("다중 행 썸네일") }
};

#define TB_BAND_COUNT	5
static CONST REBARLAYOUTINFO	gcstReBarDef[] = {
	{ IDTB_MAIN_TOOLBAR,   180, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE               },
	{ IDTB_EDIT_TOOLBAR,   450, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE               },
	{ IDTB_INSERT_TOOLBAR, 280, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE | RBBS_BREAK  },
	{ IDTB_LAYOUT_TOOLBAR, 310, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE               },
	{ IDTB_VIEW_TOOLBAR,   140, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE               }
};

LRESULT CALLBACK gpfToolbarProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_CONTEXTMENU:
			TRACE( TEXT("툴바 컨텍스트[%X]"), hWnd );
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if( SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0 ){	ReleaseCapture(   );	}
			return 0;
	}

	return CallWindowProc( gpfOrigTBProc, hWnd, msg, wParam, lParam );
}

VOID ToolBarCreate( HWND hWnd, HINSTANCE lcInst )
{

	TCHAR	atBuff[MAX_STRING];

	UINT			ici, resnum, d;
	REBARINFO		stRbrInfo;
	REBARBANDINFO	stRbandInfo;
	REBARLAYOUTINFO	stInfo[TB_BAND_COUNT];

	HBITMAP	hImg, hMsq;

	ghRebarWnd = CreateWindowEx( WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | RBS_BANDBORDERS | RBS_DBLCLKTOGGLE | CCS_NODIVIDER | CCS_TOP,
		0, 0, 0, 0, hWnd, (HMENU)IDRB_REBAR, lcInst, NULL);

	ZeroMemory( &stRbrInfo, sizeof(REBARINFO) );
	stRbrInfo.cbSize = sizeof(REBARINFO);
	SendMessage( ghRebarWnd, RB_SETBARINFO, 0, (LPARAM)&stRbrInfo );

	ZeroMemory( stInfo, sizeof(stInfo) );
	for( d = 0; TB_BAND_COUNT > d; d++ )
	{
		stInfo[d].wID    = gcstReBarDef[d].wID;
		stInfo[d].cx     = gcstReBarDef[d].cx;
		stInfo[d].fStyle = gcstReBarDef[d].fStyle;
	}

	InitToolBarLayout( INIT_LOAD, TB_BAND_COUNT, stInfo );

	ghMainTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("maintb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER
		, 0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_MAIN_TOOLBAR, lcInst, NULL);

	SendMessage( ghMainTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS );

	ghMainImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 4, 1 );
	resnum = IDBMPQ_MAIN_TB_FIRST;
	for( ici = 0; 4 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghMainImgLst, hImg, hMsq );
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	SendMessage( ghMainTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghMainImgLst );
	SendMessage( ghMainTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghMainTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuff, MAX_STRING, gatMainTBtext[0] );	gstMainTBInfo[0].iString = SendMessage( ghMainTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatMainTBtext[1] );	gstMainTBInfo[1].iString = SendMessage( ghMainTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatMainTBtext[2] );	gstMainTBInfo[2].iString = SendMessage( ghMainTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatMainTBtext[4] );	gstMainTBInfo[4].iString = SendMessage( ghMainTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );

	SendMessage( ghMainTBWnd, TB_ADDBUTTONS, (WPARAM)TB_MAIN_ITEMS, (LPARAM)&gstMainTBInfo );

	SendMessage( ghMainTBWnd, TB_AUTOSIZE, 0, 0 );

	gpfOrigTBProc = SubclassWindow( ghMainTBWnd, gpfToolbarProc );

	ghEditTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("edittb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER
		, 0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_EDIT_TOOLBAR, lcInst, NULL);

	SendMessage( ghEditTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	ghEditImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 14, 1 );
	resnum = IDBMPQ_EDIT_TB_FIRST;
	for( ici = 0; 14 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghEditImgLst, hImg, hMsq );
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	SendMessage( ghEditTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghEditImgLst );

	SendMessage( ghEditTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghEditTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 0] );	gstEditTBInfo[ 0].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 1] );	gstEditTBInfo[ 1].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 3] );	gstEditTBInfo[ 3].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 4] );	gstEditTBInfo[ 4].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 5] );	gstEditTBInfo[ 5].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 6] );	gstEditTBInfo[ 6].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 8] );	gstEditTBInfo[ 8].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 9] );	gstEditTBInfo[ 9].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[11] );	gstEditTBInfo[11].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[12] );	gstEditTBInfo[12].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[14] );	gstEditTBInfo[14].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[16] );	gstEditTBInfo[16].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[18] );	gstEditTBInfo[18].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[20] );	gstEditTBInfo[20].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	SendMessage( ghEditTBWnd, TB_ADDBUTTONS, (WPARAM)TB_EDIT_ITEMS, (LPARAM)&gstEditTBInfo );
	SendMessage( ghEditTBWnd, TB_AUTOSIZE, 0, 0 );

	gpfOrigTBProc = SubclassWindow( ghEditTBWnd, gpfToolbarProc );

	ghInsertTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("inserttb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER,
		0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_INSERT_TOOLBAR, lcInst, NULL);

	SendMessage( ghInsertTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS );

	ghInsertImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 5, 1 );
	resnum = IDBMPQ_INSERT_TB_FIRST;
	for( ici = 0; 4 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghInsertImgLst, hImg, hMsq );
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMP_MOZI_SCRIPT ) );
	hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMQ_MOZI_SCRIPT ) );
	ImageList_Add( ghInsertImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	SendMessage( ghInsertTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghInsertImgLst );

	SendMessage( ghInsertTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghInsertTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 0] );	gstInsertTBInfo[ 0].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 1] );	gstInsertTBInfo[ 1].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 2] );	gstInsertTBInfo[ 2].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 3] );	gstInsertTBInfo[ 3].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 5] );	gstInsertTBInfo[ 5].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	SendMessage( ghInsertTBWnd , TB_ADDBUTTONS, (WPARAM)TB_INSERT_ITEMS, (LPARAM)&gstInsertTBInfo );
	SendMessage( ghInsertTBWnd, TB_AUTOSIZE, 0, 0 );

	gpfOrigTBProc = SubclassWindow( ghInsertTBWnd, gpfToolbarProc );

	ghLayoutTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("layouttb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER,
		0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_LAYOUT_TOOLBAR, lcInst, NULL);

	SendMessage( ghLayoutTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	ghLayoutImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 14, 1 );
	resnum = IDBMPQ_LAYOUT_TB_FIRST;
	for( ici = 0; 12 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghLayoutImgLst, hImg, hMsq );
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMP_SPLIT_LEFT ) );
	hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMQ_SPLIT_LEFT ) );
	ImageList_Add( ghLayoutImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMP_SPLIT_RIGHT ) );
	hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMQ_SPLIT_RIGHT ) );
	ImageList_Add( ghLayoutImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	SendMessage( ghLayoutTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghLayoutImgLst );

	SendMessage( ghLayoutTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghLayoutTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 0] );	gstLayoutTBInfo[ 0].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 1] );	gstLayoutTBInfo[ 1].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 3] );	gstLayoutTBInfo[ 3].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 4] );	gstLayoutTBInfo[ 4].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 5] );	gstLayoutTBInfo[ 5].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 7] );	gstLayoutTBInfo[ 7].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 8] );	gstLayoutTBInfo[ 8].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[10] );	gstLayoutTBInfo[10].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[11] );	gstLayoutTBInfo[11].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[12] );	gstLayoutTBInfo[12].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[14] );	gstLayoutTBInfo[14].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[15] );	gstLayoutTBInfo[15].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[17] );	gstLayoutTBInfo[17].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[18] );	gstLayoutTBInfo[18].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );

	SendMessage( ghLayoutTBWnd , TB_ADDBUTTONS, (WPARAM)TB_LAYOUT_ITEMS, (LPARAM)&gstLayoutTBInfo );

	SendMessage( ghLayoutTBWnd, TB_AUTOSIZE, 0, 0 );

	gpfOrigTBProc = SubclassWindow( ghLayoutTBWnd, gpfToolbarProc );

	ghViewTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("viewtb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER,
		0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_VIEW_TOOLBAR, lcInst, NULL);

	SendMessage( ghViewTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	ghViewImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 5, 1 );
	resnum = IDBMPQ_VIEW_TB_FIRST;
	for( ici = 0; 5 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghViewImgLst, hImg, hMsq );
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	SendMessage( ghViewTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghViewImgLst );

	SendMessage( ghViewTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghViewTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 0] );	gstViewTBInfo[ 0].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 1] );	gstViewTBInfo[ 1].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 2] );	gstViewTBInfo[ 2].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 3] );	gstViewTBInfo[ 3].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 4] );	gstViewTBInfo[ 4].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );

	SendMessage( ghViewTBWnd, TB_ADDBUTTONS, (WPARAM)TB_VIEW_ITEMS, (LPARAM)&gstViewTBInfo );
	SendMessage( ghViewTBWnd, TB_AUTOSIZE, 0, 0 );

	gpfOrigTBProc = SubclassWindow( ghViewTBWnd, gpfToolbarProc );

	ZeroMemory( &stRbandInfo, sizeof(REBARBANDINFO) );
	stRbandInfo.cbSize     = sizeof(REBARBANDINFO);
	stRbandInfo.fMask      = RBBIM_TEXT | RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_ID;
	stRbandInfo.cxMinChild = 0;
	stRbandInfo.cyMinChild = 25;

	for( d = 0; TB_BAND_COUNT > d; d++ )
	{
		switch( stInfo[d].wID )
		{
			case IDTB_MAIN_TOOLBAR:
				stRbandInfo.lpText    = TEXT("파일");
				stRbandInfo.hwndChild = ghMainTBWnd;
				break;

			case IDTB_EDIT_TOOLBAR:
				stRbandInfo.lpText    = TEXT("편집");
				stRbandInfo.hwndChild = ghEditTBWnd;
				break;

			case IDTB_INSERT_TOOLBAR:
				stRbandInfo.lpText    = TEXT("삽입");
				stRbandInfo.hwndChild = ghInsertTBWnd;
				break;

			case IDTB_LAYOUT_TOOLBAR:
				stRbandInfo.lpText    = TEXT("정렬");
				stRbandInfo.hwndChild = ghLayoutTBWnd;
				break;

			case IDTB_VIEW_TOOLBAR:
				stRbandInfo.lpText    = TEXT("보기");
				stRbandInfo.hwndChild = ghViewTBWnd;
				break;

			default:	continue;	break;
		}

		stRbandInfo.wID       = stInfo[d].wID;
		stRbandInfo.cx        = stInfo[d].cx;
		stRbandInfo.fStyle    = stInfo[d].fStyle;

		SendMessage( ghRebarWnd, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&stRbandInfo );
	}

	return;
}

HRESULT ToolBarInfoChange( LPACCEL pstAccel, INT iEntry )
{
	TCHAR	atText[MAX_STRING];
	TBBUTTONINFO	stButtonInfo;

	INT	i;

	ZeroMemory( &stButtonInfo, sizeof(TBBUTTONINFO) );
	stButtonInfo.cbSize  = sizeof(TBBUTTONINFO);
	stButtonInfo.dwMask  = TBIF_TEXT;
	stButtonInfo.pszText = atText;
	stButtonInfo.cchText = MAX_STRING;

	for( i = 0; TB_MAIN_ITEMS > i; i++ )
	{
		if( 0 == gstMainTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatMainTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstMainTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghMainTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstMainTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	for( i = 0; TB_EDIT_ITEMS > i; i++ )
	{
		if( 0 == gstEditTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatEditTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstEditTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghEditTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstEditTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	for( i = 0; TB_INSERT_ITEMS > i; i++ )
	{
		if( 0 == gstInsertTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatInsertTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstInsertTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghInsertTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstInsertTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	for( i = 0; TB_LAYOUT_ITEMS > i; i++ )
	{
		if( 0 == gstLayoutTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatLayoutTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstLayoutTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghLayoutTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstLayoutTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	for( i = 0; TB_VIEW_ITEMS > i; i++ )
	{
		if( 0 == gstViewTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatViewTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstViewTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghViewTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstViewTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	return S_OK;
}

VOID ToolBarDestroy( VOID )
{
	ImageList_Destroy( ghMainImgLst );
	ImageList_Destroy( ghEditImgLst );
	ImageList_Destroy( ghLayoutImgLst );
	ImageList_Destroy( ghInsertImgLst );
	ImageList_Destroy( ghViewImgLst );

	return;
}

HRESULT ToolBarSizeGet( LPRECT pstRect )
{
	RECT	rect;

	GetWindowRect( ghRebarWnd, &rect );

	rect.right  -= rect.left;
	rect.bottom -= rect.top;
	rect.left    = 0;
	rect.top     = 0;

	SetRect( pstRect, rect.left, rect.top, rect.right, rect.bottom );

	return S_OK;
}

HRESULT ToolBarCheckOnOff( UINT itemID, UINT bCheck )
{
	HWND	hTlBrWnd;

	switch( itemID )
	{
		default:	return S_OK;

		case IDM_EXTRACTION_MODE:	hTlBrWnd =  ghEditTBWnd;	break;
		case IDM_SQSELECT:			hTlBrWnd =  ghEditTBWnd;	break;
		case IDM_UNICODE_TOGGLE:	hTlBrWnd =  ghEditTBWnd;	break;

		case IDM_UNI_PALETTE:		hTlBrWnd =  ghViewTBWnd;	break;
		case IDM_TRACE_MODE_ON:		hTlBrWnd =  ghViewTBWnd;	break;
	}

	SendMessage( hTlBrWnd, TB_CHECKBUTTON, itemID, bCheck ? TRUE : FALSE );

	return S_OK;
}

HRESULT ToolBarOnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	FORWARD_WM_SIZE( ghRebarWnd, state, cx, cy, SendMessage );

	return S_OK;
}

VOID ToolBarPseudoDropDown( HWND hWnd, INT itemID )
{
	NMTOOLBAR	stNmToolBar;
	INT	iFrom;

	ZeroMemory( &stNmToolBar, sizeof(NMTOOLBAR) );

	switch( itemID )
	{
		case IDM_IN_UNI_SPACE:
		case IDM_INSTAG_COLOUR:
		case IDM_USERINS_NA:
			stNmToolBar.hdr.hwndFrom = ghInsertTBWnd;
			iFrom = IDTB_INSERT_TOOLBAR;
			break;

		default:	 return;
	}

	stNmToolBar.hdr.idFrom   = iFrom;
	stNmToolBar.hdr.code     = TBN_DROPDOWN;
	stNmToolBar.iItem        = itemID;

	ToolBarOnNotify( hWnd, iFrom, (LPNMHDR)(&stNmToolBar) );

	return;
}

LRESULT ToolBarOnContextMenu( HWND hWnd, HWND hWndContext, LONG xPos, LONG yPos )
{
	HMENU	hPopupMenu;

	if( ghRebarWnd != hWndContext ){	return 0;	}

	TRACE( TEXT("리바 컨텍스트[%d x %d]"), xPos, yPos );

	hPopupMenu = CreatePopupMenu(  );
	AppendMenu( hPopupMenu, MF_STRING, IDM_REBER_DORESET, TEXT("배치를 초기 상태로 되돌리기") );
	TrackPopupMenu( hPopupMenu, 0, xPos, yPos, 0, hWnd, NULL );
	DestroyMenu( hPopupMenu );

	return 1;
}

LRESULT ToolBarOnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	INT			iItem, i;
	HWND		hWndFrom;
	HMENU		hPopupMenu = NULL, hMainMenu;
	LPNMTOOLBAR	pstNmToolBar;
	TPMPARAMS	stTpmParam;
	RECT		rect;

	if( IDRB_REBAR == idFrom )
	{
		if( RBN_HEIGHTCHANGE == pstNmhdr->code )
		{
			if( !( AppClientAreaCalc( &rect ) ) )	return 0;
			ViewSizeMove( hWnd, &rect );
			InvalidateRect( hWnd, NULL, TRUE );
			InvalidateRect( ghRebarWnd, NULL, TRUE );
		}
	}

	if( TBN_DROPDOWN == pstNmhdr->code )
	{
		TRACE( TEXT("드롭다운 발생") );
		pstNmToolBar = (LPNMTOOLBAR)pstNmhdr;

		iItem    = pstNmToolBar->iItem;
		hWndFrom = pstNmToolBar->hdr.hwndFrom;

		hMainMenu = GetMenu( hWnd );

		SendMessage( hWndFrom, TB_GETRECT, (WPARAM)iItem, (LPARAM)(&rect) );

		MapWindowPoints( hWndFrom, HWND_DESKTOP, (LPPOINT)(&rect), 2 );

		ZeroMemory( &stTpmParam, sizeof(TPMPARAMS) );
		stTpmParam.cbSize = sizeof(TPMPARAMS);
		stTpmParam.rcExclude = rect;

		switch( iItem )
		{
			case IDM_OPEN:
				TrackPopupMenuEx( ghHistyMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				break;

			case IDM_IN_UNI_SPACE:
				hPopupMenu = GetSubMenu( GetSubMenu(hMainMenu,2), 0 );
				TrackPopupMenuEx( hPopupMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				break;

			case IDM_INSTAG_COLOUR:
				hPopupMenu = GetSubMenu( GetSubMenu(hMainMenu,2), 1 );
				TrackPopupMenuEx( hPopupMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				break;

			case IDM_FRMINSBOX_OPEN:
				hPopupMenu = CreatePopupMenu(  );
				for( i = 0; FRAME_MAX > i; i++ ){	AppendMenu( hPopupMenu, MF_STRING, (IDM_INSFRAME_ALPHA + i), TEXT("테두리") );	}
				FrameNameModifyPopUp( hPopupMenu, 1 );
				TrackPopupMenuEx( hPopupMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				DestroyMenu( hPopupMenu );
				break;

			case IDM_USERINS_NA:
				hPopupMenu = CreatePopupMenu(  );
				UserDefMenuWrite( hPopupMenu, 0 );
				TrackPopupMenuEx( hPopupMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				DestroyMenu( hPopupMenu );
				break;

			default:	break;
		}

	}

	return 1;
}

HRESULT ToolBarBandReset( HWND hWnd )
{
	INT		index;
	UINT	d;
	REBARBANDINFO	stRbandInfo;

	TRACE( TEXT("리바 리셋") );

	ZeroMemory( &stRbandInfo, sizeof(REBARBANDINFO) );
	stRbandInfo.cbSize = sizeof(REBARBANDINFO);
	stRbandInfo.fMask  = RBBIM_STYLE | RBBIM_SIZE;

	for( d = 0; TB_BAND_COUNT > d; d++ )
	{
		index = SendMessage( ghRebarWnd, RB_IDTOINDEX, gcstReBarDef[d].wID, 0 );
		if( 0 > index ){	continue;	}

		SendMessage( ghRebarWnd, RB_MOVEBAND, index, d );

		stRbandInfo.cx     = gcstReBarDef[d].cx;
		stRbandInfo.fStyle = gcstReBarDef[d].fStyle;
		SendMessage( ghRebarWnd, RB_SETBANDINFO, (WPARAM)d, (LPARAM)&stRbandInfo );
	}

	InvalidateRect( ghRebarWnd, NULL, TRUE );

	return S_OK;
}

UINT ToolBarBandInfoGet( LPVOID pVoid )
{
	UINT	d;
	REBARBANDINFO	stBandInfo;
	REBARLAYOUTINFO	stInfo[TB_BAND_COUNT];

	ZeroMemory( &stBandInfo, sizeof(REBARBANDINFO) );
	stBandInfo.cbSize = sizeof(REBARBANDINFO);
	stBandInfo.fMask = RBBIM_ID | RBBIM_STYLE | RBBIM_SIZE;

	ZeroMemory( stInfo, sizeof(stInfo) );

	for( d = 0; TB_BAND_COUNT > d; d++ )
	{
		SendMessage( ghRebarWnd, RB_GETBANDINFO, (WPARAM)d, (LPARAM)(&stBandInfo) );
		TRACE( TEXT("ID[%u]  CX[%d]  STYLE[%u]"), stBandInfo.wID, stBandInfo.cx, stBandInfo.fStyle );

		stInfo[d].wID    = stBandInfo.wID;
		stInfo[d].cx     = stBandInfo.cx;
		stInfo[d].fStyle = stBandInfo.fStyle;
	}

	InitToolBarLayout( INIT_SAVE, TB_BAND_COUNT, stInfo );

	return TB_BAND_COUNT;
}
