#include "stdafx.h"
#include "OrinrinEditor.h"

typedef struct tagCONTEXTITEM
{
	TCHAR	atString[MIN_STRING];
	UINT	dCommandoID;

} CONTEXTITEM, *LPCONTEXTITEM;

CONST static CONTEXTITEM	gstContextItem[] =
{
	{  TEXT("새로 만들기"),			IDM_NEWFILE				},
	{  TEXT("파일 열기"),					IDM_OPEN				},
		{  TEXT("덮어쓰기 저장"),						IDM_OVERWRITESAVE		},
		{  TEXT("다른 이름으로 저장"),				IDM_RENAMESAVE			},
		{  TEXT("현재 페이지를 이미지로 저장"),			IDM_IMAGE_SAVE			},
	{  TEXT("일반 설정"),						IDM_GENERAL_OPTION		},
		{  TEXT("(구분자)"),					0						},
		{  TEXT("되돌리기"),				IDM_UNDO				},
		{  TEXT("다시 실행"),						IDM_REDO				},
		{  TEXT("잘라내기"),						IDM_CUT					},
	{  TEXT("Unicode 복사"),					IDM_COPY				},
		{  TEXT("SJIS 복사"),						IDM_SJISCOPY			},
		{  TEXT("전체를 SJIS 복사"),				IDM_SJISCOPY_ALL		},
		{  TEXT("붙여넣기"),							IDM_PASTE				},
		{  TEXT("삭제"),							IDM_DELETE				},
	{  TEXT("전체 선택"),							IDM_ALLSEL				},
		{  TEXT("사각형 선택"),						IDM_SQSELECT			},
		{  TEXT("사각형 붙여넣기"),						IDM_SQUARE_PASTE		},
		{  TEXT("레이어 박스"),					IDM_LAYERBOX			},
		{  TEXT("부분 추출 모드"),					IDM_EXTRACTION_MODE		},
	{  TEXT("다음 줄 이후를 새 페이지로 분할"),			IDM_PAGEL_DIVIDE		},
		{  TEXT("선택 문자열을 페이지 이름으로 설정"),			IDM_PAGENAME_SELASSIGN	},
		{  TEXT("(구분자)"),					0						},
		{  TEXT("유니코드 공백"),			IDM_MN_UNISPACE			},
		{  TEXT(" 1 도트 공백"),					IDM_IN_01SPACE			},
	{  TEXT(" 2 도트 공백"),					IDM_IN_02SPACE			},
		{  TEXT(" 3 도트 공백"),					IDM_IN_03SPACE			},
		{  TEXT(" 4 도트 공백"),					IDM_IN_04SPACE			},
		{  TEXT(" 5 도트 공백"),					IDM_IN_05SPACE			},
		{  TEXT(" 8 도트 공백"),					IDM_IN_08SPACE			},
	{  TEXT("10 도트 공백"),					IDM_IN_10SPACE			},
		{  TEXT("16 도트 공백"),					IDM_IN_16SPACE			},
		{  TEXT("색 지정"),							IDM_MN_COLOUR_SEL		},
		{  TEXT("흰색"),								IDM_INSTAG_WHITE		},
		{  TEXT("파란색"),								IDM_INSTAG_BLUE			},
	{  TEXT("검은색"),								IDM_INSTAG_BLACK		},
		{  TEXT("빨간색"),								IDM_INSTAG_RED			},
		{  TEXT("녹색"),								IDM_INSTAG_GREEN		},
		{  TEXT("프레임 선택"),							IDM_MN_INSFRAME_SEL		},
		{  TEXT("프레임 (1)"),						IDM_INSFRAME_ALPHA		},
	{  TEXT("프레임 (2)"),						IDM_INSFRAME_BRAVO		},
		{  TEXT("프레임 (3)"),						IDM_INSFRAME_CHARLIE	},
		{  TEXT("프레임 (4)"),						IDM_INSFRAME_DELTA		},
		{  TEXT("프레임 (5)"),						IDM_INSFRAME_ECHO		},
		{  TEXT("프레임 (6)"),						IDM_INSFRAME_FOXTROT	},
	{  TEXT("프레임 (7)"),						IDM_INSFRAME_GOLF		},
		{  TEXT("프레임 (8)"),						IDM_INSFRAME_HOTEL		},
		{  TEXT("프레임 (9)"),						IDM_INSFRAME_INDIA		},
		{  TEXT("프레임 (10)"),						IDM_INSFRAME_JULIETTE	},
		{  TEXT("프레임 (11)"),						IDM_INSFRAME_KILO		},
	{  TEXT("프레임 (12)"),						IDM_INSFRAME_LIMA		},
		{  TEXT("프레임 (13)"),						IDM_INSFRAME_MIKE		},
		{  TEXT("프레임 (14)"),						IDM_INSFRAME_NOVEMBER	},
		{  TEXT("프레임 (15)"),						IDM_INSFRAME_OSCAR		},
		{  TEXT("프레임 (16)"),						IDM_INSFRAME_PAPA		},
	{  TEXT("프레임 (17)"),						IDM_INSFRAME_QUEBEC		},
		{  TEXT("프레임 (18)"),						IDM_INSFRAME_ROMEO		},
		{  TEXT("프레임 (19)"),						IDM_INSFRAME_SIERRA		},
		{  TEXT("프레임 (20)"),						IDM_INSFRAME_TANGO		},
		{  TEXT("프레임 편집"),							IDM_INSFRAME_EDIT		},
	{  TEXT("프레임 삽입 박스"),					IDM_FRMINSBOX_OPEN		},
		{  TEXT("문자 AA 변환 박스"),			IDM_MOZI_SCR_OPEN		},
		{  TEXT("세로쓰기 변환 박스"),				IDM_VERT_SCRIPT_OPEN	},
		{  TEXT("사용자 정의 아이템"),				IDM_MN_USER_REFS		},
		{  TEXT("사용자 아이템 (1)"),			IDM_USER_ITEM_ALPHA		},
	{  TEXT("사용자 아이템 (2)"),			IDM_USER_ITEM_BRAVO		},
		{  TEXT("사용자 아이템 (3)"),			IDM_USER_ITEM_CHARLIE	},
		{  TEXT("사용자 아이템 (4)"),			IDM_USER_ITEM_DELTA		},
		{  TEXT("사용자 아이템 (5)"),			IDM_USER_ITEM_ECHO		},
		{  TEXT("사용자 아이템 (6)"),			IDM_USER_ITEM_FOXTROT	},
	{  TEXT("사용자 아이템 (7)"),			IDM_USER_ITEM_GOLF		},
		{  TEXT("사용자 아이템 (8)"),			IDM_USER_ITEM_HOTEL		},
		{  TEXT("사용자 아이템 (9)"),			IDM_USER_ITEM_INDIA		},
		{  TEXT("사용자 아이템 (10)"),			IDM_USER_ITEM_JULIETTE	},
		{  TEXT("사용자 아이템 (11)"),			IDM_USER_ITEM_KILO		},
	{  TEXT("사용자 아이템 (12)"),			IDM_USER_ITEM_LIMA		},
		{  TEXT("사용자 아이템 (13)"),			IDM_USER_ITEM_MIKE		},
		{  TEXT("사용자 아이템 (14)"),			IDM_USER_ITEM_NOVEMBER	},
		{  TEXT("사용자 아이템 (15)"),			IDM_USER_ITEM_OSCAR		},
		{  TEXT("사용자 아이템 (16)"),			IDM_USER_ITEM_PAPA		},
	{  TEXT("각 페이지에 연속 번호 삽입"),			IDM_PAGENUM_DLG_OPEN	},
		{  TEXT("(구분자)"),					0						},
		{  TEXT("오른쪽 정렬선"),				IDM_RIGHT_GUIDE_SET		},
		{  TEXT("행 시작에 전각 공백 추가"),				IDM_INS_TOPSPACE		},
		{  TEXT("행 시작 공백 삭제"),					IDM_DEL_TOPSPACE		},
	{  TEXT("행 끝 공백 삭제"),					IDM_DEL_LASTSPACE		},
		{  TEXT("행 끝 문자 삭제"),					IDM_DEL_LASTLETTER		},
		{  TEXT("선택 범위를 공백으로 채우기"),			IDM_FILL_SPACE			},
		{  TEXT("페이지 전체를 공백으로 채우기"),			IDM_FILL_ZENSP			},
		{  TEXT("행 시작 반각공백을 유니코드로 변환"),	IDM_HEADHALF_EXCHANGE	},
	{  TEXT("좌우 반전"),						IDM_MIRROR_INVERSE		},
		{  TEXT("상하 반전"),						IDM_UPSET_INVERSE		},
		{  TEXT("오른쪽으로 밀기"),						IDM_RIGHT_SLIDE			},
		{  TEXT("1 도트 증가"),					IDM_INCREMENT_DOT		},
		{  TEXT("1 도트 감소"),					IDM_DECREMENT_DOT		},
	{  TEXT("전체를 1 도트 오른쪽으로"),				IDM_INCR_DOT_LINES		},
		{  TEXT("전체를 1 도트 왼쪽으로"),				IDM_DECR_DOT_LINES		},
		{  TEXT("지정 위치에서 오른쪽으로 확장"),			IDM_DOT_SPLIT_RIGHT		},
		{  TEXT("지정 위치에서 왼쪽으로 축소"),			IDM_DOT_SPLIT_LEFT		},
		{  TEXT("조정 기준 잠금"),					IDM_DOTDIFF_LOCK		},
	{  TEXT("커서 위치에서 조정"),				IDM_DOTDIFF_ADJT		},
		{  TEXT("(구분자)"),					0						},
		{  TEXT("공백 표시"),				IDM_SPACE_VIEW_TOGGLE	},
		{  TEXT("그리드 선 표시"),				IDM_GRID_VIEW_TOGGLE	},
		{  TEXT("오른쪽 가이드 선 표시"),				IDM_RIGHT_RULER_TOGGLE	},
	{  TEXT("행 가이드 선 표시"),				IDM_UNDER_RULER_TOGGLE	},
		{  TEXT("다중 행 템플릿 표시"),			IDM_MAATMPLE_VIEW		},
		{  TEXT("페이지 목록 표시"),						IDM_PAGELIST_VIEW		},
		{  TEXT("한 줄 템플릿 표시"),			IDM_LINE_TEMPLATE		},
		{  TEXT("채우기 브러시 표시"),				IDM_BRUSH_PALETTE		},
	{  TEXT("유니코드 표 열기"),				IDM_UNI_PALETTE			},
		{  TEXT("트레이스 모드"),					IDM_TRACE_MODE_ON		},
		{  TEXT("미리보기"),						IDM_ON_PREVIEW			},
		{  TEXT("드래프트 보드 열기"),			IDM_DRAUGHT_OPEN		},
		{  TEXT("썸네일 표시"),				IDM_MAA_THUMBNAIL_OPEN	},
	{  TEXT("(구분자)"),					0						},
		{  TEXT("선택 범위를 드래프트 보드로"),		IDM_COPY_TO_DRAUGHT		},
		{  TEXT("(구분자)"),	0		},
		{  TEXT("선택 페이지 복제"),					IDM_PAGEL_DUPLICATE		},
		{  TEXT("선택 페이지 삭제"),					IDM_PAGEL_DELETE		},
	{  TEXT("다음 페이지와 통합"),					IDM_PAGEL_COMBINE		},
		{  TEXT("선택 페이지 다음에 새로 만들기"),			IDM_PAGEL_INSERT		},
		{  TEXT("끝에 페이지 새로 만들기"),				IDM_PAGEL_ADD			},
		{  TEXT("페이지를 아래로 이동"),					IDM_PAGEL_DOWNSINK		},
		{  TEXT("페이지를 위로 이동"),					IDM_PAGEL_UPFLOW		},
	{  TEXT("페이지 이름 변경"),					IDM_PAGEL_RENAME		},
		{  TEXT("트레이스 이미지 표시/숨기기"),			IDM_TRC_VIEWTOGGLE		},
		{  TEXT("템플릿 그룹 전환 ↑"),			IDM_TMPLT_GROUP_PREV	},
		{  TEXT("템플릿 그룹 전환 ↓"),			IDM_TMPLT_GROUP_NEXT	},
		{  TEXT("창 포커스 전환 ↑"),				IDM_WINDOW_CHANGE		},
	{  TEXT("창 포커스 전환 ↓"),				IDM_WINDOW_CHG_RVRS		},
		{  TEXT("파일 닫기"),				IDM_FILE_CLOSE			},
		{  TEXT("파일 전환 ↑"),					IDM_FILE_PREV			},
		{  TEXT("파일 전환 ↓"),					IDM_FILE_NEXT			},
		{  TEXT("이전 페이지로 이동"),					IDM_PAGE_PREV			},
	{  TEXT("다음 페이지로 이동"),					IDM_PAGE_NEXT			},
		{  TEXT("템플릿 그리드 증가"),			IDM_TMPL_GRID_INCREASE	},
		{  TEXT("템플릿 그리드 감소"),			IDM_TMPL_GRID_DECREASE	},
		{  TEXT("화면 다시 그리기"),					IDM_NOW_PAGE_REFRESH	},
		{  TEXT("(미구현) 문자열 검색"),			IDM_FIND_DLG_OPEN		},
	{  TEXT("(미구현) 검색 설정 초기화"),		IDM_FIND_HIGHLIGHT_OFF	},
		{  TEXT("(미구현) 다음 검색 위치"),			IDM_FIND_JUMP_NEXT		},
		{  TEXT("(미구현) 이전 검색 위치"),			IDM_FIND_JUMP_PREV		},
		{  TEXT("(미구현) 새로운 문자열 검색"),	IDM_FIND_TARGET_SET		},
		{  TEXT("(구분자)"),					0						}
};

#define ALL_ITEMS	 117
#define FULL_ITEMS	 145

#define CTS_UNISPACE	24
#define CTS_COLOURINS	33
#define CTS_FRAMEINS	39
#define CTS_USERITEM	64

#define DEF_ITEMS	32
const static UINT	gadDefItem[] =
{
	IDM_CUT,
	IDM_COPY,
	IDM_PASTE,
	IDM_ALLSEL,
	0,
	IDM_COPY_TO_DRAUGHT,
	0,
	IDM_SJISCOPY,
	IDM_SJISCOPY_ALL,
	0,
	IDM_SQSELECT,
	0,
	IDM_LAYERBOX,
	IDM_FRMINSBOX_OPEN,
	IDM_MOZI_SCR_OPEN,
	0,
	IDM_RIGHT_GUIDE_SET,
	IDM_INS_TOPSPACE,
	IDM_DEL_TOPSPACE,
	IDM_DEL_LASTSPACE,
	IDM_DEL_LASTLETTER,
	IDM_FILL_SPACE,
	IDM_RIGHT_SLIDE,
	0,
	IDM_INCR_DOT_LINES,
	IDM_DECR_DOT_LINES,
	0,
	IDM_SPACE_VIEW_TOGGLE,
	IDM_GRID_VIEW_TOGGLE,
	IDM_RIGHT_RULER_TOGGLE,
	0,
	IDM_PAGEL_DIVIDE
};

static HINSTANCE	ghInst;
static TCHAR		gatCntxIni[MAX_PATH];

static HMENU	ghPopupMenu;
static HMENU	ghUniSpMenu;
static HMENU	ghColourMenu;
static HMENU	ghFrameMenu;
static HMENU	ghUsrDefMenu;

static vector<CONTEXTITEM>	gvcCntxItem;
typedef vector<CONTEXTITEM>::iterator	CTXI_VITR;

static list<CONTEXTITEM>	gltCntxEdit;
typedef list<CONTEXTITEM>::iterator	CTXI_LITR;

VOID	CntxEditBuild( VOID );

INT_PTR	CALLBACK CntxEditDlgProc( HWND, UINT, WPARAM, LPARAM );

VOID	CntxDlgLvInit( HWND );
VOID	CntxDlgAllListUp( HWND );
VOID	CntxDlgBuildListUp( HWND );

VOID	CntxDlgItemAdd( HWND );
VOID	CntxDlgItemDel( HWND );
VOID	CntxDlgItemSpinUp( HWND );
VOID	CntxDlgItemSpinDown( HWND );

INT_PTR	CALLBACK AccelKeyDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR	AccelKeyNotify( HWND, INT, LPNMHDR, list<ACCEL> * );

HRESULT	AccelKeyBindExistCheck( HWND, LPACCEL, list<ACCEL> * );
HRESULT	AccelKeyBindListMod( HWND, INT, LPACCEL, list<ACCEL> * );
HRESULT	AccelKeySettingReset( HWND, list<ACCEL> * );
HRESULT	AccelKeyListOutput( HWND );
HRESULT	AccelKeyBindString( LPACCEL, LPTSTR, UINT_PTR );
VOID	AccelKeyListInit( HWND, list<ACCEL> * );
BYTE	AccelHotModExchange( BYTE, BOOLEAN );
HRESULT	AccelKeyTableSave( list<ACCEL> * );

HRESULT CntxEditInitialise( LPTSTR ptCurrent, HINSTANCE hInstance )
{
	UINT	dCount, cid;
	UINT	ams, ims;
	TCHAR	atKeyName[MIN_STRING];

	if( !(ptCurrent) || !(hInstance) )
	{
		if( ghPopupMenu ){	DestroyMenu( ghPopupMenu  );	}
		if( ghUniSpMenu ){	DestroyMenu( ghUniSpMenu  );	}
		if( ghColourMenu ){	DestroyMenu( ghColourMenu );	}
		if( ghFrameMenu ){	DestroyMenu( ghFrameMenu  );	}
		if( ghUsrDefMenu ){	DestroyMenu( ghUsrDefMenu );	}

		return S_OK;
	}

	ghInst = hInstance;

	StringCchCopy( gatCntxIni, MAX_PATH, ptCurrent );
	PathAppend( gatCntxIni, MZCX_INI_FILE );

	gvcCntxItem.clear();

	dCount = GetPrivateProfileInt( TEXT("Context"), TEXT("Count"), 0, gatCntxIni );
	if( 1 <= dCount )
	{
		for( ims = 0; dCount > ims; ims++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("CmdID%u"), ims );
			cid = GetPrivateProfileInt( TEXT("Context"), atKeyName, 0, gatCntxIni );

			for( ams = 0; ALL_ITEMS > ams; ams++ )
			{
				if( cid == gstContextItem[ams].dCommandoID )
				{
					gvcCntxItem.push_back( gstContextItem[ams] );
					break;
				}
			}
		}
	}
	else
	{
		for( ims = 0; DEF_ITEMS > ims; ims++ )
		{
			for( ams = 0; ALL_ITEMS > ams; ams++ )
			{
				if( gadDefItem[ims] == gstContextItem[ams].dCommandoID )
				{
					gvcCntxItem.push_back( gstContextItem[ams] );
					break;
				}
			}
		}
	}

	CntxEditBuild(  );

	return S_OK;
}

HMENU CntxMenuGet( VOID )
{
	return ghPopupMenu;
}

VOID CntxEditBuild( VOID )
{
	UINT	d, e, num;
	TCHAR	atItem[MAX_STRING], atKey[MIN_STRING], atBuffer[SUB_STRING];
	CTXI_VITR	itMnItm;

	if( ghPopupMenu ){	DestroyMenu( ghPopupMenu  );	}	ghPopupMenu  = NULL;
	if( ghUniSpMenu ){	DestroyMenu( ghUniSpMenu  );	}	ghUniSpMenu  = NULL;
	if( ghColourMenu ){	DestroyMenu( ghColourMenu );	}	ghColourMenu = NULL;
	if( ghFrameMenu ){	DestroyMenu( ghFrameMenu  );	}	ghFrameMenu  = NULL;
	if( ghUsrDefMenu ){	DestroyMenu( ghUsrDefMenu );	}	ghUsrDefMenu = NULL;

	ghPopupMenu = CreatePopupMenu(  );

	e = 0;
	for( itMnItm = gvcCntxItem.begin(); gvcCntxItem.end() != itMnItm; itMnItm++ )
	{
		if( 0 >= itMnItm->dCommandoID )
		{
			AppendMenu( ghPopupMenu, MF_SEPARATOR, 0, NULL );
		}
		else
		{
			ZeroMemory( atBuffer, sizeof(atBuffer) );
			StringCchCopy( atItem, MAX_STRING, itMnItm->atString );

			if( IDM_INSFRAME_ALPHA <= itMnItm->dCommandoID && itMnItm->dCommandoID <= IDM_INSFRAME_TANGO )
			{
				num = itMnItm->dCommandoID - IDM_INSFRAME_ALPHA;
				FrameNameLoad( num, atBuffer, SUB_STRING );
				StringCchPrintf( atItem, MAX_STRING, TEXT("프레임：%s"), atBuffer );
			}
			else if( IDM_USER_ITEM_ALPHA <= itMnItm->dCommandoID && itMnItm->dCommandoID <= IDM_USER_ITEM_PAPA )
			{
				num = itMnItm->dCommandoID - IDM_USER_ITEM_ALPHA;
				UserDefItemNameget( num, atBuffer, SUB_STRING );
				StringCchPrintf( atItem, MAX_STRING, TEXT("사용자：%s"), atBuffer );
			}
			else
			{

			}

			if( 26 > e )
			{
				StringCchPrintf( atKey, MIN_STRING, TEXT("(&%c)"), 'A' + e );
				StringCchCat( atItem, MAX_STRING, atKey );
				e++;
			}

			switch( itMnItm->dCommandoID )
			{
				default:	AppendMenu( ghPopupMenu, MF_STRING, itMnItm->dCommandoID, atItem );	break;

				case IDM_MN_UNISPACE:
					ghUniSpMenu = CreatePopupMenu(  );
					for( d = 0; 8 > d; d++ ){	AppendMenu( ghUniSpMenu, MF_STRING, gstContextItem[CTS_UNISPACE+d].dCommandoID, gstContextItem[CTS_UNISPACE+d].atString );	}
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghUniSpMenu, atItem );
					break;

				case IDM_MN_COLOUR_SEL:
					ghColourMenu = CreatePopupMenu(  );
					for( d = 0; 5 > d; d++ ){	AppendMenu( ghColourMenu, MF_STRING, gstContextItem[CTS_COLOURINS+d].dCommandoID, gstContextItem[CTS_COLOURINS+d].atString );	}
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghColourMenu, atItem );
					break;

				case IDM_MN_INSFRAME_SEL:
					ghFrameMenu = CreatePopupMenu(  );
					for( d = 0; FRAME_MAX > d; d++ )
					{

						AppendMenu( ghFrameMenu, MF_STRING, gstContextItem[CTS_FRAMEINS+d].dCommandoID, gstContextItem[CTS_FRAMEINS+d].atString );
					}
					FrameNameModifyPopUp( ghFrameMenu, 1 );
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghFrameMenu, atItem );
					break;

				case IDM_MN_USER_REFS:
					ghUsrDefMenu = CreatePopupMenu(  );
					UserDefMenuWrite( ghUsrDefMenu, 1 );
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghUsrDefMenu, atItem );
					break;
			}
		}
	}

	return;
}

HRESULT CntxEditDlgOpen( HWND hWnd )
{
	INT_PTR	iRslt, i;
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	CTXI_LITR	itEdit;

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_CONTEXT_ITEM_DLG), hWnd, CntxEditDlgProc, 0 );
	if( IDOK == iRslt )
	{

		ZeroMemory( atBuff, sizeof(atBuff) );
		WritePrivateProfileSection( TEXT("Context"), atBuff, gatCntxIni );

		gvcCntxItem.clear();	i = 0;
		for( itEdit = gltCntxEdit.begin(); gltCntxEdit.end() != itEdit; itEdit++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("CmdID%d"), i );
			StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), itEdit->dCommandoID );
			WritePrivateProfileString( TEXT("Context"), atKeyName, atBuff, gatCntxIni );

			gvcCntxItem.push_back( *itEdit );
			i++;
		}

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), i );
		WritePrivateProfileString( TEXT("Context"), TEXT("Count"), atBuff, gatCntxIni );

		CntxEditBuild(  );

		return S_OK;
	}

	return E_ABORT;
}

INT_PTR CALLBACK CntxEditDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	INT		id;
	HWND	hWndCtl;
	UINT	codeNotify;

	CTXI_VITR	itMnItm;

	switch( message )
	{
		case WM_INITDIALOG:
			gltCntxEdit.clear();
			for( itMnItm = gvcCntxItem.begin(); gvcCntxItem.end() != itMnItm; itMnItm++ ){	gltCntxEdit.push_back( *itMnItm );	}
			CntxDlgLvInit( hDlg );
			CntxDlgAllListUp( hDlg );
			CntxDlgBuildListUp( hDlg );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id         = LOWORD(wParam);
			hWndCtl    = (HWND)lParam;
			codeNotify = HIWORD(wParam);
			switch( id )
			{
				case IDOK:		EndDialog( hDlg, IDOK );		return (INT_PTR)TRUE;
				case IDCANCEL:	EndDialog( hDlg, IDCANCEL );	return (INT_PTR)TRUE;

				case IDB_MENUITEM_ADD:		CntxDlgItemAdd( hDlg );	return (INT_PTR)TRUE;
				case IDB_MENUITEM_DEL:		CntxDlgItemDel( hDlg );	return (INT_PTR)TRUE;

				case IDB_MENUITEM_SPINUP:	CntxDlgItemSpinUp( hDlg );		return (INT_PTR)TRUE;
				case IDB_MENUITEM_SPINDOWN:	CntxDlgItemSpinDown( hDlg );	return (INT_PTR)TRUE;

				default:	break;
			}
			break;
	}

	return (INT_PTR)FALSE;
}

VOID CntxDlgLvInit( HWND hDlg )
{
	HWND	hLvWnd;
	LVCOLUMN	stLvColm;
	RECT	rect;

	hLvWnd = GetDlgItem( hDlg, IDLV_MENU_ALLITEM );
	GetClientRect( hLvWnd, &rect );

	ListView_SetExtendedListViewStyle( hLvWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt      = LVCFMT_LEFT;
	stLvColm.iSubItem = 0;

	stLvColm.pszText  = TEXT("메뉴 아이템");
	stLvColm.cx       = rect.right - 23;
	ListView_InsertColumn( hLvWnd, 0, &stLvColm );

	hLvWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );
	GetClientRect( hLvWnd, &rect );

	ListView_SetExtendedListViewStyle( hLvWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP );

	stLvColm.cx       = rect.right - 23;
	ListView_InsertColumn( hLvWnd, 0, &stLvColm );

}

VOID CntxDlgAllListUp( HWND hDlg )
{
	HWND	hLvWnd;
	UINT	d;
	LVITEM	stLvi;
	TCHAR	atItem[SUB_STRING];

	hLvWnd = GetDlgItem( hDlg, IDLV_MENU_ALLITEM );

	ListView_DeleteAllItems( hLvWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask    = LVIF_TEXT;
	stLvi.pszText = atItem;

	for( d = 0; ALL_ITEMS > d; d++ )
	{
		StringCchCopy( atItem, SUB_STRING, gstContextItem[d].atString );

		if( IDM_MN_UNISPACE == gstContextItem[d].dCommandoID ||
		IDM_MN_COLOUR_SEL   == gstContextItem[d].dCommandoID ||
		IDM_MN_INSFRAME_SEL == gstContextItem[d].dCommandoID ||
		IDM_MN_USER_REFS    == gstContextItem[d].dCommandoID )
		{
			StringCchCat( atItem, SUB_STRING, TEXT("(서브메뉴 확장)") );
		}

		stLvi.iItem = d;
		ListView_InsertItem( hLvWnd, &stLvi );
	}
}

VOID CntxDlgBuildListUp( HWND hDlg )
{
	HWND	hLvWnd;
	UINT	d;
	LVITEM	stLvi;
	TCHAR	atItem[SUB_STRING];

	CTXI_LITR	itMnItm;

	hLvWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	ListView_DeleteAllItems( hLvWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask    = LVIF_TEXT;
	stLvi.pszText = atItem;

	for( itMnItm = gltCntxEdit.begin(), d = 0; gltCntxEdit.end() != itMnItm; itMnItm++, d++ )
	{
		stLvi.iItem = d;
		if( 0 >= itMnItm->dCommandoID )
		{
			StringCchCopy( atItem, SUB_STRING, TEXT("---------------") );
		}
		else
		{
			StringCchCopy( atItem, SUB_STRING, itMnItm->atString );

			if( IDM_MN_UNISPACE == itMnItm->dCommandoID ||
			IDM_MN_COLOUR_SEL   == itMnItm->dCommandoID ||
			IDM_MN_INSFRAME_SEL == itMnItm->dCommandoID ||
			IDM_MN_USER_REFS    == itMnItm->dCommandoID )
			{
				StringCchCat( atItem, SUB_STRING, TEXT("　　[＞") );
			}

		}

		ListView_InsertItem( hLvWnd, &stLvi );
	}

	return;
}

VOID CntxDlgItemAdd( HWND hDlg )
{
	HWND	hListWnd, hBuildWnd;
	INT		iSel, iIns, iCount;
	CTXI_LITR	itMnItm;

	hListWnd  = GetDlgItem( hDlg, IDLV_MENU_ALLITEM );
	hBuildWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	iSel = ListView_GetNextItem( hListWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( 0 > iSel )	 return;

	iCount = ListView_GetItemCount( hBuildWnd );

	iIns = ListView_GetNextItem( hBuildWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( (0 > iIns) || ((iIns+1) >= iCount) ){	gltCntxEdit.push_back( gstContextItem[iSel] );	}
	else
	{
		itMnItm = gltCntxEdit.begin();
		std::advance( itMnItm, iIns+1 );

		gltCntxEdit.insert( itMnItm, gstContextItem[iSel] );
	}

	CntxDlgBuildListUp( hDlg );

	return;
}

VOID CntxDlgItemDel( HWND hDlg )
{
	HWND	hListWnd, hBuildWnd;
	INT		iSel;
	CTXI_LITR	itMnItm;

	hListWnd  = GetDlgItem( hDlg, IDLV_MENU_ALLITEM );
	hBuildWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	iSel = ListView_GetNextItem( hBuildWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( 0 > iSel )	return;

	itMnItm = gltCntxEdit.begin();
	std::advance( itMnItm, iSel );

	gltCntxEdit.erase( itMnItm );

	CntxDlgBuildListUp( hDlg );

	return;
}

VOID CntxDlgItemSpinUp( HWND hDlg )
{
	HWND	hBuildWnd;
	INT		iSel;
	CTXI_LITR	itTgtItm, itSwpItm;
	CONTEXTITEM	stItem;

	hBuildWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	iSel = ListView_GetNextItem( hBuildWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( 0 >= iSel ){	 return;	}

	itTgtItm = gltCntxEdit.begin();
	std::advance( itTgtItm, iSel );
	itSwpItm = itTgtItm;
	itSwpItm--;

	StringCchCopy( stItem.atString, MIN_STRING, itTgtItm->atString );
	stItem.dCommandoID = itTgtItm->dCommandoID;

	gltCntxEdit.erase( itTgtItm );
	gltCntxEdit.insert( itSwpItm, stItem );

	CntxDlgBuildListUp( hDlg );

	ListView_SetItemState( hBuildWnd, --iSel, LVIS_SELECTED, LVIS_SELECTED );

	return;
}

VOID CntxDlgItemSpinDown( HWND hDlg )
{
	HWND	hBuildWnd;
	INT		iSel, iCount;
	CTXI_LITR	itTgtItm, itSwpItm;
	CONTEXTITEM	stItem;

	hBuildWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	iSel = ListView_GetNextItem( hBuildWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( 0 > iSel )	return;

	iCount = ListView_GetItemCount( hBuildWnd );
	if( iSel >= (iCount-1) )	 return;

	itSwpItm = gltCntxEdit.begin();
	std::advance( itSwpItm, iSel );
	itTgtItm = itSwpItm;
	itTgtItm++;

	StringCchCopy( stItem.atString, MIN_STRING, itTgtItm->atString );
	stItem.dCommandoID = itTgtItm->dCommandoID;

	gltCntxEdit.erase( itTgtItm );
	gltCntxEdit.insert( itSwpItm, stItem );

	CntxDlgBuildListUp( hDlg );

	ListView_SetItemState( hBuildWnd, ++iSel, LVIS_SELECTED, LVIS_SELECTED );

	return;
}

#ifdef ACCELERATOR_EDIT

HRESULT AccelKeyDlgOpen( HWND hWnd )
{
	INT_PTR	iRslt;

	LPACCEL	pstAccel;
	INT	iEntry;

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_ACCEL_KEY_DLG), hWnd, AccelKeyDlgProc, 0 );
	if( IDOK == iRslt )
	{

		pstAccel = AccelKeyTableLoadAlloc( &iEntry );
		AccelKeyTableCreate( pstAccel, iEntry );

		ToolBarInfoChange( pstAccel, iEntry );

		FREE( pstAccel );
	}

	return S_OK;
}

INT_PTR CALLBACK AccelKeyDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static list<ACCEL>	cltAccel;

	INT		iAccEntry;
	LPACCEL	pstAccel;

	static  HWND	hHokyWnd;
	HWND	hLvWnd;
	LRESULT	lRslt;
	ACCEL	stAcce;

	INT		i;
	INT		iItem;

	INT		id;
	HWND	hWndCtl;
	UINT	codeNotify;

	switch( message )
	{
		case WM_INITDIALOG:
			hLvWnd = GetDlgItem( hDlg, IDLV_FUNCKEY_LIST );
			ListView_SetExtendedListViewStyle( hLvWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP );

			cltAccel.clear();
			pstAccel = AccelKeyTableGetAlloc( &iAccEntry );

			for( i = 0; iAccEntry > i; i++ ){	cltAccel.push_back( pstAccel[i] );	}
			FREE( pstAccel );

			AccelKeyListInit( hDlg, &cltAccel );

			hHokyWnd = GetDlgItem( hDlg, IDHKC_FUNCKEY_INPUT );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id         = LOWORD(wParam);
			hWndCtl    = (HWND)lParam;
			codeNotify = HIWORD(wParam);
			switch( id )
			{
				case IDOK:
					AccelKeyTableSave( &cltAccel );
				case IDCANCEL:
					cltAccel.clear();
					EndDialog( hDlg, id );
					return (INT_PTR)TRUE;

				case IDB_FUNCKEY_CLEAR:
					SendMessage( hHokyWnd, HKM_SETHOTKEY, 0, 0 );
					iItem = WndTagGet( hHokyWnd );
					AccelKeyBindListMod( hDlg, iItem, NULL, &cltAccel );
					return (INT_PTR)TRUE;

				case IDB_FUNCKEY_SET:
					lRslt = SendMessage( hHokyWnd, HKM_GETHOTKEY, 0, 0 );
					stAcce.key   = LOBYTE( lRslt );
					if( BST_CHECKED == IsDlgButtonChecked( hDlg, IDCB_FUNCKEY_SPACE ) )
					{	stAcce.key = VK_SPACE;	}
					stAcce.fVirt  = AccelHotModExchange( HIBYTE( lRslt ), 0 );
					stAcce.cmd    = 0;
					if( SUCCEEDED( AccelKeyBindExistCheck( hDlg, &stAcce, &cltAccel ) ) )
					{

						iItem = WndTagGet( hHokyWnd );
						AccelKeyBindListMod( hDlg, iItem, &stAcce, &cltAccel );
					}
					return (INT_PTR)TRUE;

				case IDB_FUNCKEY_INIT:
					if( IDOK == MessageBox( hDlg, TEXT("키 설정을 초기 상태로 되돌립니다"), TEXT("설정 초기화"), MB_OKCANCEL | MB_ICONQUESTION ) )
					{
						AccelKeySettingReset( hDlg, &cltAccel );
						AccelKeyListInit( hDlg, &cltAccel );
					}
					return (INT_PTR)TRUE;

				case IDB_FUNCKEY_FILEOUT:
					AccelKeyListOutput( hDlg );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		case WM_NOTIFY:
			return AccelKeyNotify( hDlg, (INT)(wParam), (LPNMHDR)(lParam), &cltAccel );

		default:	break;
	}

	return (INT_PTR)FALSE;
}

INT_PTR AccelKeyNotify( HWND hDlg, INT idFrom, LPNMHDR pstNmhdr, list<ACCEL> *pltAccel )
{
	LPNMLISTVIEW	pstLv;
	LVITEM			stLvi;
	BYTE	bMod;
	HWND	hHokyWnd;

	list<ACCEL>::iterator	itAccel;

	if( IDLV_FUNCKEY_LIST == idFrom )
	{
		if( NM_CLICK == pstNmhdr->code )
		{
			pstLv = (LPNMLISTVIEW)pstNmhdr;
			pstLv->iItem;

			ZeroMemory( &stLvi, sizeof(stLvi) );
			stLvi.mask     = LVIF_PARAM;
			stLvi.iItem    = pstLv->iItem;
			ListView_GetItem( pstNmhdr->hwndFrom, &stLvi );

			hHokyWnd = GetDlgItem( hDlg, IDHKC_FUNCKEY_INPUT );
			WndTagSet( hHokyWnd , stLvi.iItem );

			for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
			{
				if( stLvi.lParam == itAccel->cmd )
				{
					bMod = AccelHotModExchange( itAccel->fVirt, 1 );
					if( 0x20 == itAccel->key )
					{	SendMessage( hHokyWnd , HKM_SETHOTKEY, MAKEWORD(itAccel->key, bMod), 0 );	}
					else{	SendMessage( hHokyWnd, HKM_SETHOTKEY, MAKEWORD(itAccel->key, (bMod|HOTKEYF_EXT)), 0 );	}

					break;
				}
			}
			if( itAccel == (*pltAccel).end() )	SendMessage( hHokyWnd, HKM_SETHOTKEY, 0, 0 );

			SetFocus( GetDlgItem( hDlg, IDHKC_FUNCKEY_INPUT ) );
		}
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}

LPACCEL AccelKeyTableLoadAlloc( LPINT piEntry )
{
	UINT	dCount, dValue;
	UINT	i, aim = 0;
	TCHAR	atKeyName[MIN_STRING];
	LPACCEL	pstAccel = NULL;

	dCount = GetPrivateProfileInt( TEXT("Accelerator"), TEXT("Count"), 0, gatCntxIni );
	if( 1 <= dCount )
	{
		pstAccel = (LPACCEL)malloc( dCount * sizeof(ACCEL) );

		for( i = 0; dCount > i; i++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcCMD%d"), i );
			dValue = GetPrivateProfileInt( TEXT("Accelerator"), atKeyName, 0, gatCntxIni );
			if( 0 == dValue )	continue;
			pstAccel[aim].cmd = dValue;

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcVirt%d"), i );
			dValue = GetPrivateProfileInt( TEXT("Accelerator"), atKeyName, 0, gatCntxIni );
			pstAccel[aim].fVirt = dValue;

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcKey%d"), i );
			dValue = GetPrivateProfileInt( TEXT("Accelerator"), atKeyName, 0, gatCntxIni );
			pstAccel[aim].key = dValue;

			aim++;
		}
	}

	if( piEntry )	*piEntry = aim;

	return pstAccel;
}

HRESULT AccelKeyBindString( LPACCEL pstAccel, LPTSTR ptBuffer, UINT_PTR cchSize )
{
	TCHAR	atKey[MIN_STRING];

	ZeroMemory( ptBuffer, cchSize * sizeof(TCHAR) );

	if( FCONTROL & pstAccel->fVirt )	StringCchCat( ptBuffer, cchSize, TEXT("Ctrl + ") );
	if( FSHIFT   & pstAccel->fVirt )	StringCchCat( ptBuffer, cchSize, TEXT("Shift + ") );
	if( FALT     & pstAccel->fVirt )	StringCchCat( ptBuffer, cchSize, TEXT("Alt + ") );

	if( '0' <= pstAccel->key && pstAccel->key <= '9' )
	{
		StringCchPrintf( atKey, MIN_STRING, TEXT("%c"), pstAccel->key );
	}
	else if( 'A' <= pstAccel->key && pstAccel->key <= 'Z' )
	{
		StringCchPrintf( atKey, MIN_STRING, TEXT("%c"), pstAccel->key );
	}
	else if( 0x60 <= pstAccel->key && pstAccel->key <= 0x69 )
	{
		StringCchPrintf( atKey, MIN_STRING, TEXT("NUMPAD%u"), pstAccel->key - 0x60 );
	}
	else if( 0x70 <= pstAccel->key && pstAccel->key <= 0x87 )
	{
		StringCchPrintf( atKey, MIN_STRING, TEXT("F%u"), pstAccel->key - 0x70 + 1 );
	}
	else
	{
		switch( pstAccel->key )
		{
			case VK_CANCEL:		StringCchCopy( atKey, MIN_STRING, TEXT("Break") );	break;
			case VK_BACK:		StringCchCopy( atKey, MIN_STRING, TEXT("BackSpace") );	break;
			case VK_TAB:		StringCchCopy( atKey, MIN_STRING, TEXT("TAB") );	break;
			case VK_CLEAR:		StringCchCopy( atKey, MIN_STRING, TEXT("CLEAR") );	break;
			case VK_RETURN:		StringCchCopy( atKey, MIN_STRING, TEXT("Enter") );	break;
			case VK_PAUSE:		StringCchCopy( atKey, MIN_STRING, TEXT("Pause") );	break;
			case VK_CAPITAL:	StringCchCopy( atKey, MIN_STRING, TEXT("CAPITAL") );	break;
			case VK_KANA:		StringCchCopy( atKey, MIN_STRING, TEXT("KANA") );	break;
			case VK_ESCAPE:		StringCchCopy( atKey, MIN_STRING, TEXT("Esc") );	break;
			case VK_CONVERT:	StringCchCopy( atKey, MIN_STRING, TEXT("변환") );	break;
			case VK_NONCONVERT:	StringCchCopy( atKey, MIN_STRING, TEXT("무변환") );	break;
			case VK_SPACE:		StringCchCopy( atKey, MIN_STRING, TEXT("Space") );	break;
			case VK_PRIOR:		StringCchCopy( atKey, MIN_STRING, TEXT("PageUp") );	break;
			case VK_NEXT:		StringCchCopy( atKey, MIN_STRING, TEXT("PageDown") );	break;
			case VK_END:		StringCchCopy( atKey, MIN_STRING, TEXT("End") );	break;
			case VK_HOME:		StringCchCopy( atKey, MIN_STRING, TEXT("Home") );	break;
			case VK_LEFT:		StringCchCopy( atKey, MIN_STRING, TEXT("←") );	break;
			case VK_UP:			StringCchCopy( atKey, MIN_STRING, TEXT("↑") );	break;
			case VK_RIGHT:		StringCchCopy( atKey, MIN_STRING, TEXT("→") );	break;
			case VK_DOWN:		StringCchCopy( atKey, MIN_STRING, TEXT("↓") );	break;
			case VK_SELECT:		StringCchCopy( atKey, MIN_STRING, TEXT("SELECT") );	break;
			case VK_PRINT:		StringCchCopy( atKey, MIN_STRING, TEXT("PRINT") );	break;
			case VK_EXECUTE:	StringCchCopy( atKey, MIN_STRING, TEXT("EXECUTE") );	break;
			case VK_SNAPSHOT:	StringCchCopy( atKey, MIN_STRING, TEXT("PrintScr") );	break;
			case VK_INSERT:		StringCchCopy( atKey, MIN_STRING, TEXT("Insert") );	break;
			case VK_DELETE:		StringCchCopy( atKey, MIN_STRING, TEXT("Delete") );	break;
			case VK_HELP:		StringCchCopy( atKey, MIN_STRING, TEXT("Help") );	break;
			case VK_LWIN:		StringCchCopy( atKey, MIN_STRING, TEXT("왼쪽 Win") );	break;
			case VK_RWIN:		StringCchCopy( atKey, MIN_STRING, TEXT("오른쪽 Win") );	break;
			case VK_APPS:		StringCchCopy( atKey, MIN_STRING, TEXT("APPZ") );	break;
			case VK_SLEEP:		StringCchCopy( atKey, MIN_STRING, TEXT("SLEEP") );	break;
			case VK_MULTIPLY:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM *") );	break;
			case VK_ADD:		StringCchCopy( atKey, MIN_STRING, TEXT("NUM +") );	break;
			case VK_SEPARATOR:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM ,") );	break;
			case VK_SUBTRACT:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM -") );	break;
			case VK_DECIMAL:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM .") );	break;
			case VK_DIVIDE:		StringCchCopy( atKey, MIN_STRING, TEXT("NUM /") );	break;
			case VK_NUMLOCK:	StringCchCopy( atKey, MIN_STRING, TEXT("NumLock") );	break;
			case VK_SCROLL:		StringCchCopy( atKey, MIN_STRING, TEXT("ScrollLock") );	break;
			case VK_OEM_NEC_EQUAL:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM =") );	break;
			case VK_BROWSER_BACK:		StringCchCopy( atKey, MIN_STRING, TEXT("뒤로") );	break;
			case VK_BROWSER_FORWARD:	StringCchCopy( atKey, MIN_STRING, TEXT("앞으로") );	break;
			case VK_BROWSER_REFRESH:	StringCchCopy( atKey, MIN_STRING, TEXT("새로고침") );	break;
			case VK_BROWSER_STOP:		StringCchCopy( atKey, MIN_STRING, TEXT("정지") );	break;
			case VK_BROWSER_SEARCH:		StringCchCopy( atKey, MIN_STRING, TEXT("검색") );	break;
			case VK_BROWSER_FAVORITES:	StringCchCopy( atKey, MIN_STRING, TEXT("즐겨찾기") );	break;
			case VK_BROWSER_HOME:		StringCchCopy( atKey, MIN_STRING, TEXT("홈") );	break;
			case VK_VOLUME_MUTE:		StringCchCopy( atKey, MIN_STRING, TEXT("음소거") );	break;
			case VK_VOLUME_DOWN:		StringCchCopy( atKey, MIN_STRING, TEXT("볼륨 다운") );	break;
			case VK_VOLUME_UP:			StringCchCopy( atKey, MIN_STRING, TEXT("볼륨 업") );	break;
			case VK_MEDIA_NEXT_TRACK:	StringCchCopy( atKey, MIN_STRING, TEXT("다음 트랙") );	break;
			case VK_MEDIA_PREV_TRACK:	StringCchCopy( atKey, MIN_STRING, TEXT("이전 트랙") );	break;
			case VK_MEDIA_STOP:			StringCchCopy( atKey, MIN_STRING, TEXT("정지") );	break;
			case VK_MEDIA_PLAY_PAUSE:	StringCchCopy( atKey, MIN_STRING, TEXT("재생/일시정지") );	break;
			case VK_LAUNCH_MAIL:		StringCchCopy( atKey, MIN_STRING, TEXT("메일") );	break;
			case VK_LAUNCH_MEDIA_SELECT:StringCchCopy( atKey, MIN_STRING, TEXT("선택") );	break;
			case VK_LAUNCH_APP1:		StringCchCopy( atKey, MIN_STRING, TEXT("APP1") );	break;
			case VK_LAUNCH_APP2:		StringCchCopy( atKey, MIN_STRING, TEXT("APP2") );	break;
			case VK_OEM_1:		StringCchCopy( atKey, MIN_STRING, TEXT(":") );	break;
			case VK_OEM_PLUS:	StringCchCopy( atKey, MIN_STRING, TEXT(";") );	break;
			case VK_OEM_COMMA:	StringCchCopy( atKey, MIN_STRING, TEXT(",") );	break;
			case VK_OEM_MINUS:	StringCchCopy( atKey, MIN_STRING, TEXT("-") );	break;
			case VK_OEM_PERIOD:	StringCchCopy( atKey, MIN_STRING, TEXT(".") );	break;
			case VK_OEM_2:		StringCchCopy( atKey, MIN_STRING, TEXT("/") );	break;
			case VK_OEM_3:		StringCchCopy( atKey, MIN_STRING, TEXT("@") );	break;
			case VK_OEM_4:		StringCchCopy( atKey, MIN_STRING, TEXT("[") );	break;
			case VK_OEM_5:		StringCchCopy( atKey, MIN_STRING, TEXT("\\") );	break;	//	¥
			case VK_OEM_6:		StringCchCopy( atKey, MIN_STRING, TEXT("]") );	break;
			case VK_OEM_7:		StringCchCopy( atKey, MIN_STRING, TEXT("^") );	break;
			case VK_OEM_8:		StringCchCopy( atKey, MIN_STRING, TEXT("_") );	break;
			case VK_OEM_102:	StringCchCopy( atKey, MIN_STRING, TEXT("ろ") );	break;	//	ろ
			case VK_OEM_ATTN:	StringCchCopy( atKey, MIN_STRING, TEXT("CapsLock") );	break;
			case VK_OEM_COPY:	StringCchCopy( atKey, MIN_STRING, TEXT("カ夕ひら") );	break;
			case VK_OEM_AUTO:	StringCchCopy( atKey, MIN_STRING, TEXT("半/全 漢1") );	break;
			case VK_OEM_ENLW:	StringCchCopy( atKey, MIN_STRING, TEXT("半/全 漢2") );	break;

			//	足りない分はＳＤＫから追加する
			default:	StringCchPrintf( atKey, MIN_STRING, TEXT("0x%02X"), pstAccel->key );	break;
		}
	}
//0x40 : unassigned
//0x07 : unassigned
//0x0A - 0x0B : reserved
//0x5E : reserved
//0x88 - 0x8F : unassigned
//0x97 - 0x9F : unassigned
//0xB8 - 0xB9 : reserved
//0xC1 - 0xD7 : reserved
//0xD8 - 0xDA : unassigned
//0xE0 : reserved
//0xE8 : unassigned
//0xFF : reserved

	StringCchCat( ptBuffer, cchSize, atKey );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	コマンド番号を参照して、ヒットしたらアクセル文字列を作ってくっつける
	@param[in]	ptText		処理結果をくっつける文字列ポインター
	@param[in]	cchSize		バッファの文字数
	@param[in]	dCommand	コマンド番号
	@param[in]	pstAccel	アクセラキーテーブル
	@param[in]	iEntry		テーブルのデータ数
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyTextBuild( LPTSTR ptText, UINT_PTR cchSize, DWORD dCommand, CONST LPACCEL pstAccel, INT iEntry )
{
	TCHAR	atKeystr[SUB_STRING];
	INT		i;

	for( i = 0; iEntry > i; i++ )
	{
		if( pstAccel[i].cmd == dCommand )
		{
			AccelKeyBindString( &(pstAccel[i]), atKeystr, SUB_STRING );

			StringCchCat( ptText, cchSize, TEXT("\r\n") );
			StringCchCat( ptText, cchSize, atKeystr );

			return S_OK;
		}
	}

	return E_OUTOFMEMORY;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルキー編集のリストビュー初期化
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	*pltAccel	アクセラキーテーブル
*/
VOID AccelKeyListInit( HWND hDlg, list<ACCEL> *pltAccel )
{
	HWND		hLvWnd;
	LVCOLUMN	stLvColm;
	LVITEM		stItem;
	RECT		rect;
	LONG		width, i, j;
	TCHAR		atBuffer[SUB_STRING];

	list<ACCEL>::iterator	itAccel;

	hLvWnd = GetDlgItem( hDlg, IDLV_FUNCKEY_LIST );
	GetClientRect( hLvWnd, &rect );
	width = rect.right - 23;
	width /= 2;

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt      = LVCFMT_LEFT;

	stLvColm.iSubItem = 0;
	stLvColm.pszText  = TEXT("기능");
	stLvColm.cx       = width;
	ListView_InsertColumn( hLvWnd, 0, &stLvColm );

	stLvColm.iSubItem = 1;
	stLvColm.pszText  = TEXT("키 바인딩");
	stLvColm.cx       = width;
	ListView_InsertColumn( hLvWnd, 1, &stLvColm );

	ZeroMemory( &stItem, sizeof(LVITEM) );

	for( i = 0, j = 0; ALL_ITEMS > i; i++ )
	{
		stItem.iItem = j;

		//	関係無いやつは飛ばす
		if( 0 == gstContextItem[i].dCommandoID ||
		IDM_MN_UNISPACE     == gstContextItem[i].dCommandoID ||
		IDM_MN_COLOUR_SEL   == gstContextItem[i].dCommandoID ||
		IDM_MN_INSFRAME_SEL == gstContextItem[i].dCommandoID ||
		IDM_MN_USER_REFS    == gstContextItem[i].dCommandoID )
		{	continue;	}

		StringCchCopy( atBuffer, SUB_STRING, gstContextItem[i].atString );
		stItem.mask     = LVIF_TEXT | LVIF_PARAM;
		stItem.pszText  = atBuffer;
		stItem.lParam   = gstContextItem[i].dCommandoID;
		stItem.iSubItem = 0;
		ListView_InsertItem( hLvWnd, &stItem );

		ZeroMemory( atBuffer, sizeof(atBuffer) );

		for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
		{
			if( gstContextItem[i].dCommandoID == itAccel->cmd )
			{
				AccelKeyBindString( &(*itAccel), atBuffer, SUB_STRING );
				break;
			}
		}

		stItem.mask     = LVIF_TEXT;
		stItem.pszText  = atBuffer;
		stItem.iSubItem = 1;
		ListView_SetItem( hLvWnd, &stItem );

		j++;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルキーとホットキーの修飾子を入替
	@param[in]	bSrc	元の修飾子コード
	@param[in]	bDrct	非０アクセル→ホット　０ホット→アクセル
	@return	変換したコード
*/
BYTE AccelHotModExchange( BYTE bSrc, BOOLEAN bDrct )
{
	BYTE	bDest = 0;

	if( bDrct )	//	アクセル→ホット
	{
		if( bSrc & FSHIFT )		bDest |= HOTKEYF_SHIFT;		//	シフト
		if( bSrc & FCONTROL )	bDest |= HOTKEYF_CONTROL;	//	コントロール
		if( bSrc & FALT )		bDest |= HOTKEYF_ALT;		//	アルタネート
	}
	else	//	ホット→アクセル
	{
		if( bSrc & HOTKEYF_SHIFT )		bDest |= FSHIFT;	//	シフト
		if( bSrc & HOTKEYF_CONTROL )	bDest |= FCONTROL;	//	コントロール
		if( bSrc & HOTKEYF_ALT )		bDest |= FALT;		//	アルタネート

		bDest |= (FVIRTKEY|FNOINVERT);	//	常にある
	}

	return bDest;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルテーブルを保存
	@param[in]	*pltAccel	アクセラキーテーブル
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyTableSave( list<ACCEL> *pltAccel )
{
	INT_PTR	i;
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	list<ACCEL>::iterator	itAccel;

	//	一旦セクションを空にする
	ZeroMemory( atBuff, sizeof(atBuff) );
	WritePrivateProfileSection( TEXT("Accelerator"), atBuff, gatCntxIni );

	i = 0;
	for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
	{
		StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcCMD%d"), i );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), itAccel->cmd );
		WritePrivateProfileString( TEXT("Accelerator"), atKeyName, atBuff, gatCntxIni );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcVirt%d"), i );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), itAccel->fVirt );
		WritePrivateProfileString( TEXT("Accelerator"), atKeyName, atBuff, gatCntxIni );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcKey%d"), i );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), itAccel->key );
		WritePrivateProfileString( TEXT("Accelerator"), atKeyName, atBuff, gatCntxIni );

		i++;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), i );
	WritePrivateProfileString( TEXT("Accelerator"), TEXT("Count"), atBuff, gatCntxIni );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセル設定を初期状態にアッー
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	*pltAccel	アクセラキーテーブル
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeySettingReset( HWND hDlg, list<ACCEL> *pltAccel )
{
	HWND	hLvWnd;
	HACCEL	hAccel;	//
	LPACCEL	pstAccel = NULL;
	INT		iItems, i;

	//	元々のテーブルを確保
	hAccel = LoadAccelerators( ghInst, MAKEINTRESOURCE(IDC_ORINRINEDITOR) );

	//	まず個数確保
	iItems = CopyAcceleratorTable( hAccel, NULL, 0 );
	if( 0 >= iItems )	return E_POINTER;

	pstAccel = (LPACCEL)malloc( iItems * sizeof(ACCEL) );
	if( !(pstAccel) )	return NULL;

	//	本体確保
	iItems = CopyAcceleratorTable( hAccel, pstAccel, iItems );

	DestroyAcceleratorTable( hAccel );	//	全部汚倭ったらぶっ壊しておく

	(*pltAccel).clear();	//	クルヤー

	for( i = 0; iItems > i; i++ ){	(*pltAccel).push_back( pstAccel[i] );	}

	FREE( pstAccel );

	hLvWnd = GetDlgItem( hDlg, IDLV_FUNCKEY_LIST );
	ListView_DeleteAllItems( hLvWnd );	//	リストビューは壊しておく方が早い

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	設定をファイルにエクスポートする
	@param[in]	hDlg	ダイヤログハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT AccelKeyListOutput( HWND hDlg )
{
//	CONST  WCHAR	rtHead = 0xFEFF;	//	ユニコードテキストヘッダ
	//	ファイル形式は？　SJISかUTF8でいい

	HANDLE	hFile;
	DWORD	wrote;
	BOOLEAN	bOpened;

	OPENFILENAME	stSaveFile;

	INT		i, iAccEntry;
	INT		j;
	LPACCEL	pstAccel;

	UINT_PTR	cchSz;
	INT		cbSize;

	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];
	TCHAR	atCmdName[MIN_STRING], atKeyBind[SUB_STRING];
	TCHAR	atBuffer[MAX_PATH];

	CHAR	acString[BIG_STRING];

	ZeroMemory( &stSaveFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );
	ZeroMemory( atBuffer,  sizeof(atBuffer) );

	StringCchCopy( atFilePath, MAX_PATH, TEXT("Accelerator.txt") );
#if 1
	//ここで FileSaveDialogue を出す
	stSaveFile.lStructSize     = sizeof(OPENFILENAME);
	stSaveFile.hwndOwner       = hDlg;
	stSaveFile.lpstrFilter     = TEXT("텍스트 파일 ( *.txt )\0*.txt\0모든 파일 ( *.* )\0*.*\0\0");
	stSaveFile.nFilterIndex    = 1;	//	デフォのフィルタ選択肢
	stSaveFile.lpstrFile       = atFilePath;
	stSaveFile.nMaxFile        = MAX_PATH;
	stSaveFile.lpstrFileTitle  = atFileName;
	stSaveFile.nMaxFileTitle   = MAX_STRING;
	stSaveFile.lpstrDefExt     = TEXT("txt");
//	stSaveFile.lpstrInitialDir =
	stSaveFile.lpstrTitle      = TEXT("저장할 파일 이름을 지정하세요");
	stSaveFile.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	bOpened = GetSaveFileName( &stSaveFile );
	if( !(bOpened) ){	return  E_ABORT;	}
	//	キャンセルしてたら何もしない
#endif
	hFile = CreateFile( atFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		MessageBox( hDlg, TEXT("파일을 열 수 없었습니다..."), NULL, MB_OK | MB_ICONERROR );
		return E_HANDLE;
	}
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	//WriteFile( hFile, &rtHead, 2, &wrote, NULL );

	//	設定されてるテーブルを確保
	pstAccel = AccelKeyTableGetAlloc( &iAccEntry );
	for( i = 0; iAccEntry > i; i++ )
	{
		AccelKeyBindString( &(pstAccel[i]), atKeyBind, SUB_STRING );

		StringCchCopy( atCmdName, MIN_STRING, TEXT("(이름 없음)") );
		for( j = 0; FULL_ITEMS > j; j++ )
		{
			if( gstContextItem[j].dCommandoID == pstAccel[i].cmd )
			{
				StringCchCopy( atCmdName, MIN_STRING, gstContextItem[j].atString );
				break;
			}
		}

		StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s\t%s\r\n"), atCmdName, atKeyBind );
		StringCchLength( atBuffer, MAX_PATH, &cchSz );

		cbSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atBuffer, -1, acString, BIG_STRING, NULL, NULL );
									//	CP_UTF8, 0
		//	変換文字数にはヌルターミネータが含まれているので注意
		WriteFile( hFile, acString, cbSize-1, &wrote, NULL );
	}
	FREE( pstAccel );

	SetEndOfFile( hFile );
	CloseHandle( hFile );

	MessageBox( hDlg, TEXT("파일 출력 완료."), TEXT("(・∀・)ｂ"), MB_OK | MB_ICONINFORMATION );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定のコマンドが使われてるかどうか確認
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	pstAccel	アクセラキーテーブル
	@param[in]	*pltAccel	既存のやつの一覧
	@return		HRESULT		終了状態コード　S_OKかぶり無し　E_ACCESSDENIEDかぶり有った
*/
HRESULT AccelKeyBindExistCheck( HWND hDlg, LPACCEL pstAccel, list<ACCEL> *pltAccel )
{
	INT	i;
	//BYTE	fVirt;
	WORD	dCommand;
	list<ACCEL>::iterator	itAccel;
	TCHAR	atFuncName[MIN_STRING], atMsg[MAX_STRING];

	for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
	{
		//	同じのがあるかどうか探す
		//fVirt= itAccel->fVirt;
		//fVirt &= ~(FVIRTKEY|FNOINVERT);変換時に付け足してるので要らない
		if( pstAccel->key == itAccel->key && pstAccel->fVirt == itAccel->fVirt )
		{
			//	同じのがあった
			dCommand = itAccel->cmd;
			StringCchCopy( atFuncName, MIN_STRING, TEXT("(이름 없음)") );

			for( i = 0; FULL_ITEMS > i; i++ )
			{
				if( dCommand == gstContextItem[i].dCommandoID )
				{
					StringCchCopy( atFuncName, MIN_STRING, gstContextItem[i].atString );
					break;
				}
			}

			StringCchPrintf( atMsg, MAX_STRING, TEXT("이 키 바인딩은 '%s'에서 사용 중입니다."), atFuncName );
			MessageBox( hDlg, atMsg, TEXT("중복되었습니다"), MB_OK | MB_ICONWARNING );

			return E_ACCESSDENIED;
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルキー編集のリストビューにデータ書込
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	iItem		操作する行番号
	@param[in]	pstAccel	アクセラキーテーブル
	@param[in]	*pltAccel	既存のやつの一覧
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyBindListMod( HWND hDlg, INT iItem, LPACCEL pstAccel, list<ACCEL> *pltAccel )
{
	HWND	hLvWnd = GetDlgItem( hDlg, IDLV_FUNCKEY_LIST );
	HWND	hHkcWnd = GetDlgItem( hDlg, IDHKC_FUNCKEY_INPUT );
	LVITEM	stLvi;
	TCHAR	atBuffer[SUB_STRING];
	WORD	dCommand;
	list<ACCEL>::iterator	itAccel;

	//	該当行から、コマンド番号を持ってくる
	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask     = LVIF_PARAM;	//	コマンドコードを確保する
	stLvi.iItem    = iItem;
	ListView_GetItem( hLvWnd, &stLvi );
	dCommand = stLvi.lParam;

	for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
	{
		if( dCommand ==  itAccel->cmd ){	break;	}
		//	ヒットしたらソレでおｋ
	}

	if( pstAccel )	//	変更・追加
	{
		if( itAccel == (*pltAccel).end() )	//	追加
		{
			pstAccel->cmd = dCommand;
			(*pltAccel).push_back( *pstAccel );
		}
		else	//	変更
		{
			itAccel->key   = pstAccel->key;
			itAccel->fVirt = pstAccel->fVirt;
		}

		//	表示用文字列作って
		AccelKeyBindString( pstAccel, atBuffer, SUB_STRING );
		ZeroMemory( &stLvi, sizeof(stLvi) );
		stLvi.mask     = LVIF_TEXT;	//	コマンドコードを確保する
		stLvi.iItem    = iItem;
		stLvi.iSubItem = 1;
		stLvi.pszText  = atBuffer;
		ListView_SetItem( hLvWnd, &stLvi );
	}
	else	//	解除
	{
		//	ヒットしてないなんてことは無いはずだが
		if( itAccel == (*pltAccel).end( ) ){	return E_HANDLE;	}

		//	該当要素を削除
		(*pltAccel).erase( itAccel );
		//	ホットキーコントロールを空に
		SendMessage( hHkcWnd, HKM_SETHOTKEY, 0, 0 );
		//	リストビューの表示も空にする
		ZeroMemory( atBuffer, sizeof(atBuffer) );
		ZeroMemory( &stLvi, sizeof(stLvi) );
		stLvi.mask     = LVIF_TEXT;	//	コマンドコードを確保する
		stLvi.iItem    = iItem;
		stLvi.iSubItem = 1;
		stLvi.pszText  = atBuffer;
		ListView_SetItem( hLvWnd, &stLvi );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	メニュー文字列に、アクセラーキー名称を割り当てていく
	@param[in]	hWnd		メニューのあるウインドウハンドル
	@param[in]	pstAccel	内容テーブル・無い時はNULL
	@param[in]	iEntry		テーブルのエントリ数
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyMenuRewrite( HWND hWnd, LPACCEL pstAccel, CONST INT iEntry )
{
	HMENU		hMenu;
	WORD		dCmd;
	INT			i, j, iRslt;
	UINT		mRslt;
	UINT_PTR	d, cchSz;
	BOOLEAN		bModify;
	TCHAR		atBuffer[MAX_STRING], atBind[SUB_STRING];

	hMenu = GetMenu( hWnd );	//	サブメニューまで全部イケる

	for( i = 0; FULL_ITEMS > i; i++ )
	{
		//	関係無いやつは飛ばす
		if( 0 == gstContextItem[i].dCommandoID )	continue;

		dCmd = gstContextItem[i].dCommandoID;	//	基本的にこれでヒットするはず
		ZeroMemory( atBuffer, sizeof(atBuffer) );
		iRslt = GetMenuString( hMenu, dCmd, atBuffer, MAX_STRING, MF_BYCOMMAND );
		if( !(iRslt) )	continue;

		bModify = FALSE;

		//	先の内容を破壊する
		StringCchLength( atBuffer, MAX_STRING, &cchSz );
		for( d = 0; cchSz > d; d++ )
		{
			if( TEXT('\t') == atBuffer[d] )
			{
				atBuffer[d] = 0;
				bModify = TRUE;
				break;
			}
		}

		//	このコマンドのエントリーはあるか
		for( j = 0; iEntry > j; j++ )
		{
			if( dCmd == pstAccel[j].cmd )	//	あったら作成
			{
				ZeroMemory( atBind, sizeof(atBind) );
				AccelKeyBindString( &(pstAccel[j]), atBind, SUB_STRING );

				StringCchCat( atBuffer, MAX_STRING, TEXT("\t") );
				StringCchCat( atBuffer, MAX_STRING, atBind );

				bModify = TRUE;
				break;
			}
		}

		if( bModify )
		{
			//	必要に応じてチェック状態を確保する
			mRslt = GetMenuState( hMenu, dCmd, MF_BYCOMMAND );

			ModifyMenu( hMenu, dCmd, (MF_CHECKED & mRslt), dCmd, atBuffer );
			//	MF_BYCOMMAND | MF_STRING は両方０なので、必要なのはチェックの是非だけ
		}
	}

	DrawMenuBar( hWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#endif
