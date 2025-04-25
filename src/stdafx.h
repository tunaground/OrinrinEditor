#pragma once

#define STRICT

#pragma warning( disable : 4100 )

#pragma warning( disable : 4201 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4312 )

#include "targetver.h"

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <windowsx.h>

#if defined(__MINGW32__) || defined(__MINGW64__)

#undef FORWARD_WM_MOUSEWHEEL
#define FORWARD_WM_MOUSEWHEEL(hwnd,xPos,yPos,zDelta,fwKeys,fn) (void)(fn)((hwnd),WM_MOUSEWHEEL,MAKEWPARAM((fwKeys),(zDelta)),MAKELPARAM((xPos),(yPos)))
#endif

#pragma comment(lib, "shell32.lib")

#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")

#include <commctrl.h>
#pragma comment(lib, "ComCtl32.lib")

#ifndef _ORCOLL

#include <imm.h>
#pragma comment(lib, "imm32.lib")

#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")

#endif

#include <assert.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <malloc.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include <memory.h>
#include <tchar.h>
#include <time.h>

#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

#ifndef STRSAFE_MAX_LENGTH
#define STRSAFE_MAX_LENGTH  (STRSAFE_MAX_CCH - 1)
#endif

#pragma warning( disable : 4995 )
#include <shlwapi.h>
#pragma warning( default : 4995 )
#pragma comment(lib, "shlwapi.lib")

#pragma warning( disable : 4995 )
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#pragma warning( default : 4995 )

using namespace	std;

static CONST GUID gcstGUID = { 0x66D3E881, 0x972B, 0x458B, { 0x93, 0x5E, 0x9E, 0x78, 0xB9, 0x26, 0xB4, 0x15 } };

#define ACCELERATOR_EDIT
#define USE_HOVERTIP
#define EDGE_BLANK_STYLE

#define MAA_IADD_PLUS

#define MULTIACT_RELAY
#define DOT_SPLIT_MODE
#define SPLIT_BAR_POS_FIX

#define BIG_TEXT_SEPARATE

#define TODAY_HINT_STYLE
#define SPMOZI_ENCODE

#define FIND_STRINGS

#define MAA_TEXT_FIND

#define DO_TRY_CATCH
#define USE_NOTIFYICON

#define EXTERNED

#if defined(_DEBUG) || defined(WORK_LOG_OUT)
#ifdef __GNUC__
	#define TRACE(str,...)	OutputDebugStringPlus( GetLastError(), _CRT_WIDE(__FILE__), __LINE__, __FUNCTION__, str, ##__VA_ARGS__ )
#else
	#define TRACE(str,...)	OutputDebugStringPlus( GetLastError(), _CRT_WIDE(__FILE__), __LINE__, __FUNCTION__, str, __VA_ARGS__ )
#endif
	VOID	OutputDebugStringPlus( DWORD, LPTSTR, INT, LPCSTR, LPTSTR, ... );

#else
	#define TRACE(x,...)
#endif

#ifndef _ORCOLL
	#ifdef _DEBUG
		#define SQL_DEBUG(db)	SqlErrMsgView(db,__LINE__)
		VOID	SqlErrMsgView( sqlite3 *, DWORD );
	#else
		#define SQL_DEBUG(db)
	#endif
#endif

#ifdef DO_TRY_CATCH
#define ETC_MSG(str,ret)	ExceptionMessage( str, __FUNCTION__, __LINE__, ret )

LRESULT	ExceptionMessage( LPCSTR, LPCSTR, UINT, LPARAM );
#endif

#define FREE(pp)	{if(pp){free(pp);pp=NULL;}}

#define StatusBar_SetText(hwndSB,ipart,ptext)	(BOOLEAN)SNDMSG((hwndSB),SB_SETTEXT,ipart,(LPARAM)(LPCTSTR)(ptext))

#define MIN_STRING	20
#define SUB_STRING	64
#define MAX_STRING	130
#define BIG_STRING	520

#define W_WIDTH		480
#define W_HEIGHT	400

#ifdef _ORCOLL
#define WCL_WIDTH	480
#define WCL_HEIGHT	370
#endif

#define PLIST_DOCK	190
#define TMPL_DOCK	150

#ifdef USE_NOTIFYICON
#define WMP_TRAYNOTIFYICON	(WM_APP+1)
#endif
#define WMP_BRUSH_TOGGLE	(WM_APP+2)
#define WMP_PREVIEW_CLOSE	(WM_APP+3)

#define BASIC_COLOUR	RGB(0xF0,0xF0,0xF0)

#define USER_ITEM_FILE	TEXT("UserItem.ast")
#define USER_ITEM_MAX	16

#define AA_BRUSH_FILE	TEXT("aabrush.txt")
#define AA_LIST_FILE	TEXT("aalist.txt")

#define AA_MIRROR_FILE	TEXT("hantenX.txt")
#define AA_UPSET_FILE	TEXT("hantenY.txt")

#define MAA_FAVDB_FILE	TEXT("Favorite.qmt")
#define MAA_TREE_CACHE	TEXT("TreeCache.qor")

#define NAMELESS_DUMMY	TEXT("NoName0.txt")
#define NAME_DUMMY_NAME	TEXT("NoName")
#define NAME_DUMMY_EXT	TEXT("txt")

#define TEMPLATE_DIR	TEXT("Templates")
#define BACKUP_DIR		TEXT("BackUp")
#define PROFILE_DIR		TEXT("Profile")

#define DROP_OBJ_NAME	TEXT("[*DROP_OBJECT*]")

#define FRAME_MAX	20

#define INI_FILE		TEXT("Utuho.ini")
#define FRAME_INI_FILE	TEXT("Satori.ini")
#define MZCX_INI_FILE	TEXT("Koisi.ini")

#define HINT_FILE		TEXT("Today_Hint.txt")

#define INIT_LOAD		1
#define INIT_SAVE		0

#define WDP_MVIEW		1
#define WDP_PLIST		2
#define WDP_LNTMPL		3
#define WDP_BRTMPL		4
#define WDP_MAATPL		5
#define WDP_PREVIEW		6
#define WDP_MMAATPL		7

#define FONTSZ_NORMAL	16
#define FONTSZ_REDUCE	12

#define VL_CLASHCOVER	0
#define VL_GROUP_UNDO	1
#define VL_USE_UNICODE	2
#define VL_LAYER_TRANS	3
#define VL_RIGHT_SLIDE	4
#define VL_MAA_SPLIT	5
#define VL_MAA_LCLICK	6
#define VL_UNILISTLAST	7
#define VL_MAATIP_VIEW	8
#define VL_MAATIP_SIZE	9
#define VL_LINETMP_CLM	10
#define VL_BRUSHTMP_CLM	11
#define VL_UNIRADIX_HEX	12
#define VL_BACKUP_INTVL	13
#define VL_BACKUP_MSGON	14
#define VL_GRID_X_POS	15
#define VL_GRID_Y_POS	16
#define VL_MAA_TOPMOST	17
#define VL_R_RULER_POS	18
#define VL_CRLF_CODE	19
#define VL_SPACE_VIEW	20
#define VL_GRID_VIEW	21
#define VL_R_RULER_VIEW	22
#define VL_PAGETIP_VIEW	23
#define VL_PCOMBINE_NM	24
#define VL_PDIVIDE_NM	25
#define VL_PDELETE_NM	26
#define VL_MAASEP_STYLE	27
#define VL_USE_BALLOON	28
#define VL_CLIPFILECNT	29
#define VL_PLS_LN_DOCK	30

#define VS_PROFILE_NAME	32
#define VS_PAGE_FORMAT	33
#define VL_SWAP_COPY	34
#define VL_MAIN_SPLIT	35
#define VL_MAXIMISED	36
#define VL_DRT_LCLICK	37
#define VL_FIRST_READED	38
#define VL_LAST_OPEN	39
#define VL_MAA_MCLICK	40
#define VL_DRT_MCLICK	41
#define VS_FONT_NAME	42
#define VL_WORKLOG		43
#define VL_PAGE_UNDER	44
#define VL_PAGE_OVWRITE	45
#define VL_COLLECT_AON	46
#define VL_COLHOT_MODY	47
#define VL_COLHOT_VKEY	48
#define VL_MAA_RETFCS	49
#define VL_PGL_RETFCS	50
#define VL_U_RULER_POS	51
#define VL_U_RULER_VIEW	52
#define VL_PAGEBYTE_MAX	53
#define VS_UNI_USE_LOG	54
#define VL_MAA_BKCOLOUR	55
#define VS_RGUIDE_MOZI	56
#define VL_THUMB_HORIZ	57
#define VL_THUMB_VERTI	58
#define VL_MULTI_ACT_E	59
#define VL_SAVE_MSGON	60
#define VL_MAATAB_SNGL	61
#define VL_HINT_ENABLE	62
#define VL_SPMOZI_ENC	63
#define VS_EXT_M2H_PATH	64

#define CLRV_BASICPEN	101
#define CLRV_BASICBK	102
#define CLRV_GRIDLINE	103
#define CLRV_CRLFMARK	104
#define CLRV_CANTSJIS	105

#define SB_MODIFY	0
#define SB_OP_STYLE	1
#define SB_MOUSEPOS	2
#define SB_CURSOR	3
#define SB_LAYER	4
#define SB_BYTECNT	5
#define SB_SELBYTE	6

#define WND_MAIN	1
#define WND_MAAT	2
#define WND_PAGE	3
#define WND_LINE	4
#define WND_BRUSH	5
#define WND_TAIL	5

#define M_DESTROY	0
#define M_CREATE	1
#define M_EXISTENCE	2

#define MAA_DEFAULT		0xFF
#define MAA_SUBDEFAULT	0xFE
#define MAA_INSERT		0
#define MAA_INTERRUPT	1
#define MAA_LAYERED		2
#define MAA_UNICLIP		3
#define MAA_SJISCLIP	4
#define MAA_DRAUGHT		5

#define LASTOPEN_DO		0
#define LASTOPEN_NON	1
#define LASTOPEN_ASK	2

#define D_SJIS		0x00
#define D_UNI		0x01
#define D_SQUARE	0x02
#define D_INVISI	0x10
#define D_RENAME	0x80

#define ISAVE_BMP	0x1

#define ISAVE_PNG	0x3

#define CLIP_FORMAT	TEXT("ORINRIN_EDITOR_STYLE")
#define CLIP_SQUARE	TEXT("MSDEVColumnSelect")

#define	VK_0	0x30
#define	VK_1	0x31
#define	VK_2	0x32
#define	VK_3	0x33
#define	VK_4	0x34
#define	VK_5	0x35
#define	VK_6	0x36
#define	VK_7	0x37
#define	VK_8	0x38
#define	VK_9	0x39

#define	VK_A	0x41
#define	VK_B	0x42
#define	VK_C	0x43
#define	VK_D	0x44
#define	VK_E	0x45
#define	VK_F	0x46
#define	VK_G	0x47
#define	VK_H	0x48
#define	VK_I	0x49
#define	VK_J	0x4A
#define	VK_K	0x4B
#define	VK_L	0x4C
#define	VK_M	0x4D
#define	VK_N	0x4E
#define	VK_O	0x4F
#define	VK_P	0x50
#define	VK_Q	0x51
#define	VK_R	0x52
#define	VK_S	0x53
#define	VK_T	0x54
#define	VK_U	0x55
#define	VK_V	0x56
#define	VK_W	0x57
#define	VK_X	0x58
#define	VK_Y	0x59
#define	VK_Z	0x5A
