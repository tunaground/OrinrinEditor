#pragma once

#define STRICT

#include "resource.h"

#include "SplitBar.h"

#define MLT_SEPARATERW	TEXT("[SPLIT]")
#define MLT_SEPARATERA	("[SPLIT]")
#define MLT_SPRT_CCH	7

#define AST_SEPARATERW	TEXT("[AA]")
#define AST_SEPARATERA	("[AA]")
#define AST_SPRT_CCH	4

#define TMPLE_BEGINW	TEXT("[ListName=")
#define TMPLE_ENDW		TEXT("[end]")

#define CH_CRLFW	TEXT("\r\n")
#define CH_CRLFA	("\r\n")
#define CH_CRLF_CCH	2

#define EOF_SIZE	5
CONST  TCHAR	gatEOF[] = TEXT("[EOF]");
#define EOF_WIDTH	39

#define YY2_CRLF	6
#define STRB_CRLF	4

#define PAGE_BYTE_MAX	4096

#define MODIFY_MSG	TEXT("[変更]")

#define COLOUR_TAG_WHITE	TEXT("<jbbs fontcolor=\"#ffffff\">")
#define COLOUR_TAG_BLUE		TEXT("<jbbs fontcolor=\"#0000ff\">")
#define COLOUR_TAG_BLACK	TEXT("<jbbs fontcolor=\"#000000\">")
#define COLOUR_TAG_RED		TEXT("<jbbs fontcolor=\"#ff0000\">")
#define COLOUR_TAG_GREEN	TEXT("<jbbs fontcolor=\"#00ff00\">")

#define CC_TAB	0x09
#define CC_CR	0x0D
#define CC_LF	0x0A

#define DO_INSERT	1
#define DO_DELETE	2

#define LINE_HEIGHT	18

#define RULER_AREA	13

#define LINENUM_WID	37
#define LINENUM_COLOUR		0xFF8000

#define RUL_LNNUM_COLOURBK	0xC0C0C0

#define SPACE_HAN	5
#define SPACE_ZEN	11

#define CLR_BLACK	0x000000
#define CLR_MAROON	0x000080
#define CLR_GREEN	0x008000
#define CLR_OLIVE	0x008080
#define CLR_NAVY	0x800000
#define CLR_PURPLE	0x800080
#define CLR_TEAL	0x808000
#define CLR_GRAY	0x808080
#define CLR_SILVER	0xC0C0C0
#define CLR_RED		0x0000FF
#define CLR_LIME	0x00FF00
#define CLR_YELLOW	0x00FFFF
#define CLR_BLUE	0xFF0000
#define CLR_FUCHSIA	0xFF00FF
#define CLR_AQUA	0xFFFF00
#define CLR_WHITE	0xFFFFFF

#define CT_NORMAL	0x0000
#define CT_WARNING	0x0001
#define CT_SPACE	0x0002
#define CT_SELECT	0x0004
#define CT_CANTSJIS	0x0008
#define CT_LYR_TRNC	0x0010
#define CT_FINDED	0x0020

#define CT_SELRTN	0x0100
#define CT_LASTSP	0x0200
#define CT_RETURN	0x0400
#define CT_EOF		0x0800
#define CT_FINDRTN	0x1000

#define OPENHIST_MAX	12

typedef struct tagOPENHISTORY
{
	TCHAR	atFile[MAX_PATH];
	DWORD	dMenuNumber;

} OPENHIST, *LPOPENHIST;
typedef list<OPENHIST>::iterator	OPHIS_ITR;

typedef struct tagLETTER
{
	TCHAR	cchMozi;
	INT		rdWidth;
	UINT	mzStyle;
	CHAR	acSjis[10];
	INT_PTR	mzByte;

} LETTER, *LPLETTER;
typedef vector<LETTER>::iterator	LETR_ITR;

#ifndef _ORRVW

#define PARTS_CCH	130

typedef struct tagFRAMEITEM
{
	TCHAR	atParts[PARTS_CCH];
	INT		dDot;
	INT		iLine;
	INT		iNowLn;

} FRAMEITEM, *LPFRAMEITEM;

typedef struct tagFRAMEINFO
{
	TCHAR	atFrameName[MAX_STRING];

	FRAMEITEM	stDaybreak;
	FRAMEITEM	stMorning;
	FRAMEITEM	stNoon;
	FRAMEITEM	stAfternoon;
	FRAMEITEM	stNightfall;
	FRAMEITEM	stTwilight;
	FRAMEITEM	stMidnight;
	FRAMEITEM	stDawn;

	INT		dLeftOffset;
	INT		dRightOffset;

	UINT	dRestPadd;

} FRAMEINFO, *LPFRAMEINFO;

typedef struct tagTRACEPARAM
{
	POINT	stOffsetPt;
	INT		dContrast;
	INT		dGamma;
	INT		dGrayMoph;
	INT		dZooming;
	INT		dTurning;

	UINT	bUpset;
	UINT	bMirror;

	COLORREF	dMoziColour;

} TRACEPARAM, *LPTRACEPARAM;

typedef struct tagOPERATELOG
{
	UINT	dCommando;
	UINT	ixSequence;
	UINT	ixGroup;

	INT		rdXdot;

	INT		rdYline;

	LPTSTR	ptText;
	UINT	cchSize;

} OPERATELOG, *LPOPERATELOG;
typedef vector<OPERATELOG>::iterator	OPSQ_ITR;

typedef struct tagUNDOBUFF
{
	UINT_PTR	dNowSqn;
	UINT		dTopSqn;
	UINT		dGrpSqn;

	vector<OPERATELOG>	vcOpeSqn;

} UNDOBUFF, *LPUNDOBUFF;

typedef struct tagONELINE
{
	INT		iDotCnt;
	INT		iByteSz;
	UINT	dStyle;
	BOOLEAN	bBadSpace;

	vector<LETTER>	vcLine;

	INT		dFrtSpDot;
	INT		dFrtSpMozi;

} ONELINE, *LPONELINE;
typedef list<ONELINE>::iterator		LINE_ITR;

typedef struct tagONEPAGE
{
	TCHAR	atPageName[SUB_STRING];

	INT		dByteSz;

	INT		dSelLineTop;
	INT		dSelLineBottom;
	UNDOBUFF	stUndoLog;

	list<ONELINE>	ltPage;

	LPTSTR	ptRawData;
	INT		iLineCnt;
	INT		iMoziCnt;

} ONEPAGE, *LPONEPAGE;
typedef vector<ONEPAGE>::iterator	PAGE_ITR;

typedef struct tagONEFILE
{
	TCHAR	atFileName[MAX_PATH];

	UINT	dModify;

	LPARAM	dUnique;
	TCHAR	atDummyName[MAX_PATH];

	INT		dNowPage;

	POINT	stCaret;

	vector<ONEPAGE>	vcCont;

} ONEFILE, *LPONEFILE;

typedef list<ONEFILE>::iterator	FILES_ITR;

typedef struct tagAATEMPLATE
{
	TCHAR	atCtgryName[SUB_STRING];

	vector<wstring>	vcItems;

} AATEMPLATE, *LPAATEMPLATE;
typedef vector<AATEMPLATE>::iterator	TEMPL_ITR;

typedef struct tagREBARLAYOUTINFO
{
	UINT	wID;
	UINT	cx;
	UINT	fStyle;

} REBARLAYOUTINFO, *LPREBARLAYOUTINFO;

typedef struct tagPAGEINFOS
{
	UINT	dMasqus;

	INT_PTR	iLines;
	INT_PTR	iBytes;
	INT_PTR	iMozis;

	TCHAR	atPageName[SUB_STRING];

} PAGEINFOS, *LPPAGEINFOS;
#define PI_LINES	0x01
#define PI_BYTES	0x02
#define PI_MOZIS	0x04
#define PI_NAME		0x08
#define PI_RECALC	0x80000000

typedef UINT (CALLBACK* PAGELOAD)(LPTSTR, LPCTSTR, INT);

#endif

typedef struct tagAAMATRIX
{
	CHAR	acAstName[MAX_STRING];

	UINT	ixNum;
	DWORD	cbItem;

	LPSTR	pcItem;

	INT		iByteSize;

	INT		iMaxDot;
	INT		iLines;

	SIZE	stSize;
	HBITMAP	hThumbBmp;

} AAMATRIX, *LPAAMATRIX;
typedef vector<AAMATRIX>::iterator	MAAM_ITR;

#ifdef USE_HOVERTIP

typedef LPTSTR (CALLBACK* HOVERTIPDISP)( LPVOID );

HRESULT	HoverTipInitialise( HINSTANCE, HWND );
HRESULT	HoverTipResist( HWND  );
HRESULT	HoverTipSizeChange( INT );
LRESULT	HoverTipOnMouseHover( HWND, WPARAM, LPARAM, HOVERTIPDISP );
LRESULT	HoverTipOnMouseLeave( HWND );

#endif

INT_PTR		CALLBACK About( HWND, UINT, WPARAM, LPARAM );

INT_PTR		MessageBoxCheckBox( HWND, HINSTANCE, UINT );

VOID		WndTagSet( HWND, LONG_PTR );
LONG_PTR	WndTagGet( HWND );

#ifdef SPMOZI_ENCODE
UINT		IsSpMozi( TCHAR );
#endif

HRESULT		InitWindowPos( UINT, UINT, LPRECT );
INT			InitParamValue( UINT, UINT, INT );
HRESULT		InitParamString( UINT, UINT, LPTSTR );

HRESULT		OpenProfileInitialise( HWND );
HRESULT		InitProfHistory( UINT, UINT, LPTSTR );
  #ifdef _ORRVW
HRESULT		OpenProfMenuModify( HWND );
  #endif

BOOLEAN		SelectDirectoryDlg( HWND, LPTSTR, UINT_PTR );

UINT		ViewMaaMaterialise( HWND, LPSTR, UINT, UINT );
INT			ViewStringWidthGet( LPCTSTR );
INT			ViewLetterWidthGet( TCHAR );

UINT		ViewMaaItemsModeGet( PUINT );

LPTSTR		SjisDecodeAlloc( LPSTR );
LPSTR		SjisEntityExchange( LPCSTR );
BOOLEAN		HtmlEntityCheckA( TCHAR, LPSTR , UINT_PTR );
BOOLEAN		HtmlEntityCheckW( TCHAR, LPTSTR, UINT_PTR );

BOOLEAN		DocIsSjisTrance( TCHAR, LPSTR );
INT_PTR		DocLetterByteCheck( LPLETTER );
INT_PTR		DocLetterDataCheck( LPLETTER, TCHAR );

BOOLEAN		FileExtensionCheck( LPTSTR, LPTSTR );

HWND		MaaTmpltInitialise( HINSTANCE, HWND, LPRECT );
HRESULT		MaaTmpltPositionReset( HWND );
VOID		MaaTabBarSizeGet( LPRECT  );

HRESULT		AaItemsTipSizeChange( INT, UINT );

HRESULT		ViewingFontGet( LPLOGFONT );

LPTSTR		FindStringProc( LPTSTR, LPTSTR, LPINT );

#ifndef _ORRVW

VOID		AacBackupDirectoryInit( LPTSTR );

BOOLEAN		MaaViewToggle( UINT );

UINT		UnicodeUseToggle( LPVOID  );

LPSTR		SjisEncodeAlloc( LPCTSTR  );

ATOM		InitWndwClass( HINSTANCE  );
BOOL		InitInstance( HINSTANCE , INT, LPTSTR );
LRESULT		CALLBACK WndProc( HWND , UINT, WPARAM, LPARAM );

HRESULT		MainStatusBarSetText( INT, LPCTSTR );
HRESULT		MainSttBarSetByteCount( UINT  );

HRESULT		WindowPositionReset( HWND );

HRESULT		MenuItemCheckOnOff( UINT, UINT );
HRESULT		NotifyBalloonExist( LPTSTR, LPTSTR, DWORD );

HRESULT		BrushModeToggle( VOID );

HRESULT		WindowFocusChange( INT, INT );

HRESULT		OptionDialogueOpen( VOID  );

COLORREF	InitColourValue( UINT, UINT, COLORREF );
INT			InitTraceValue( UINT, LPTRACEPARAM );

INT			InitWindowTopMost( UINT, UINT, INT );
HRESULT		InitToolBarLayout( UINT, INT, LPREBARLAYOUTINFO );

UINT		DocHugeFileTreatment( UINT );

#ifdef ACCELERATOR_EDIT
LPACCEL		AccelKeyTableGetAlloc( LPINT  );
LPACCEL		AccelKeyTableLoadAlloc( LPINT );
HRESULT		AccelKeyDlgOpen( HWND );
HACCEL		AccelKeyHandleGet( HINSTANCE  );

HACCEL		AccelKeyTableCreate( LPACCEL, INT );
HRESULT		AccelKeyMenuRewrite( HWND, LPACCEL, CONST INT );
#endif

HRESULT		OpenHistoryInitialise( HWND );
HRESULT		OpenHistoryLogging( HWND , LPTSTR );
HRESULT		OpenHistoryLoad( HWND, INT );

VOID		ToolBarCreate( HWND, HINSTANCE );
HRESULT		ToolBarInfoChange( LPACCEL, INT );
VOID		ToolBarDestroy( VOID  );
HRESULT		ToolBarSizeGet( LPRECT );
HRESULT		ToolBarCheckOnOff( UINT, UINT );
HRESULT		ToolBarOnSize( HWND, UINT, INT, INT );
LRESULT		ToolBarOnNotify( HWND, INT, LPNMHDR );
LRESULT		ToolBarOnContextMenu( HWND , HWND, LONG, LONG );
VOID		ToolBarPseudoDropDown( HWND , INT );
UINT		ToolBarBandInfoGet( LPVOID );
HRESULT		ToolBarBandReset( HWND );

UINT		AppClientAreaCalc( LPRECT );

HRESULT		AppTitleChange( LPTSTR );
HRESULT		AppTitleTrace( UINT );

LPTSTR		ExePathGet( VOID  );

HRESULT		UniDlgInitialise( HWND , UINT );
HRESULT		UniDialogueEntry( HINSTANCE, HWND );

HRESULT		FrameInitialise( LPTSTR, HINSTANCE );
HRESULT		FrameNameModifyPopUp( HMENU, UINT );
INT_PTR		FrameEditDialogue( HINSTANCE, HWND, UINT );
HRESULT		FrameNameLoad( UINT, LPTSTR, UINT_PTR );

HWND		FrameInsBoxCreate( HINSTANCE, HWND );
HRESULT		FrameMoveFromView( HWND, UINT );

HRESULT		CntxEditInitialise( LPTSTR, HINSTANCE );
HRESULT		CntxEditDlgOpen( HWND );
HMENU		CntxMenuGet( VOID );

HRESULT		AccelKeyTextBuild( LPTSTR, UINT_PTR, DWORD, LPACCEL, INT );

HRESULT		MultiFileTabFirst( LPTSTR );
HRESULT		MultiFileTabAppend( LPARAM, LPTSTR );
HRESULT		MultiFileTabSelect( LPARAM );
HRESULT		MultiFileTabSlide( INT );
HRESULT		MultiFileTabRename( LPARAM, LPTSTR );
HRESULT		MultiFileTabClose( INT );
INT			MultiFileTabSearch( LPARAM );
INT			InitMultiFileTabOpen( UINT, INT, LPTSTR );

VOID		OperationOnCommand( HWND, INT, HWND, UINT );

VOID		AaFontCreate( UINT );

#ifdef TODAY_HINT_STYLE
VOID		TodayHintPopup( HWND, HINSTANCE, LPTSTR );
#endif

HWND		ViewInitialise( HINSTANCE, HWND, LPRECT, LPTSTR );
HRESULT		ViewSizeMove( HWND, LPRECT );
HRESULT		ViewFocusSet( VOID );

BOOL		ViewShowCaret( VOID );
VOID		ViewHideCaret( VOID );
INT			ViewCaretPosGet( PINT, PINT );

HRESULT		ViewFrameInsert( INT  );
HRESULT		ViewMaaItemsModeSet( UINT, UINT );

HRESULT		ViewNowPosStatus( VOID );

HRESULT		ViewRedrawSetLine( INT );
HRESULT		ViewRedrawSetRect( LPRECT );
HRESULT		ViewRedrawSetVartRuler( INT );
HRESULT		ViewRulerRedraw( INT, INT );
HRESULT		ViewEditReset( VOID );

COLORREF	ViewMoziColourGet( LPCOLORREF );
COLORREF	ViewBackColourGet( LPVOID );

HRESULT		ViewCaretCreate( HWND, COLORREF, COLORREF );
HRESULT		ViewCaretDelete( VOID );
BOOLEAN		ViewDrawCaret( INT, INT , BOOLEAN );
BOOLEAN		ViewPosResetCaret( INT, INT );
HRESULT		ViewCaretReColour( COLORREF );

HRESULT		ViewPositionTransform( PINT, PINT, BOOLEAN );
BOOLEAN		ViewIsPosOnFrame( INT, INT );
INT			ViewAreaSizeGet( PINT );

HRESULT		ViewSelPositionSet( LPVOID );
HRESULT		ViewSelMoveCheck( UINT );
UINT		ViewSelRangeCheck( UINT );
UINT		ViewSelBackCheck( INT );
INT			ViewSelPageAll( INT );
UINT		ViewSqSelModeToggle( UINT, LPVOID );
HRESULT		ViewSelAreaSelect( LPVOID );

INT			ViewInsertUniSpace( UINT );
INT			ViewInsertColourTag( UINT );
INT			ViewInsertTmpleString( LPCTSTR );

HRESULT		ViewBrushStyleSetting( UINT, LPTSTR );

VOID		Evw_OnMouseMove( HWND, INT, INT, UINT );
VOID		Evw_OnLButtonDown( HWND, BOOL, INT, INT, UINT );
VOID		Evw_OnLButtonUp( HWND, INT, INT, UINT );
VOID		Evw_OnRButtonDown( HWND, BOOL, INT, INT, UINT );

VOID		Evw_OnKey( HWND, UINT, BOOL, INT, UINT );
VOID		Evw_OnChar( HWND, TCHAR, INT );
VOID		Evw_OnMouseWheel( HWND, INT, INT, INT, UINT );

VOID		Evw_OnImeComposition( HWND, WPARAM, LPARAM );

BOOLEAN		IsSelecting( PUINT );

HRESULT		OperationOnStatusBar( VOID );

HWND		PageListInitialise( HINSTANCE, HWND, LPRECT );
VOID		PageListResize( HWND , LPRECT );
HRESULT		PageListClear( VOID );
HRESULT		PageListInsert( INT );
HRESULT		PageListDelete( INT );
HRESULT		PageListViewChange( INT , INT );
HRESULT		PageListInfoSet( INT, INT, INT );
HRESULT		PageListNameSet( INT , LPTSTR );
HRESULT		PageListNameRewrite( LPTSTR );
INT			PageListIsNamed( FILES_ITR );
HRESULT		PageListPositionReset( HWND );

HRESULT		PageListViewRewrite( INT  );
HRESULT		PageListBuild( LPVOID );

HRESULT		TemplateItemLoad( LPTSTR, PAGELOAD );
UINT		TemplateGridFluctuate( HWND , INT );

HWND		LineTmpleInitialise( HINSTANCE , HWND, LPRECT );
VOID		LineTmpleResize( HWND, LPRECT );
HRESULT		LineTmplePositionReset( HWND  );

VOID		DockingTabSizeGet( LPRECT );
HRESULT		DockingTabContextMenu( HWND, HWND, LONG, LONG );
HWND		DockingTabGet( VOID );
HRESULT		DockingTmplViewToggle( UINT );

HWND		BrushTmpleInitialise( HINSTANCE, HWND, LPRECT, HWND );
LPTSTR		BrushStringMake( INT , LPTSTR );
VOID		BrushTmpleResize( HWND, LPRECT );
HRESULT		BrushTmplePositionReset( HWND );

INT			UserDefInitialise( HWND, UINT );
HRESULT		UserDefItemInsert( HWND, UINT );
HRESULT		UserDefMenuWrite( HMENU, UINT );
HRESULT		UserDefItemNameget( UINT, LPTSTR, UINT_PTR );
HRESULT		UserDefSetString( vector<ONELINE> *, LPCTSTR, UINT );

HRESULT		FrameNameModifyMenu( HWND );

VOID		PreviewInitialise( HINSTANCE, HWND );
HRESULT		PreviewVisibalise( INT, BOOLEAN );

INT			TraceInitialise( HWND, UINT );
HRESULT		TraceDialogueOpen( HINSTANCE, HWND );
HRESULT		TraceImgViewTglExt( VOID );
UINT		TraceImageAppear( HDC, INT, INT );
UINT		TraceMoziColourGet( LPCOLORREF );

HRESULT		ImageFileSaveDC( HDC, LPTSTR, INT );

VOID		LayerBoxInitialise( HINSTANCE, LPRECT );
HRESULT		LayerBoxAlphaSet( UINT );
HRESULT		LayerMoveFromView( HWND, UINT );
HWND		LayerBoxVisibalise( HINSTANCE, LPCTSTR, UINT );
INT			LayerHeadSpaceCheck( vector<LETTER> *, PINT );
HRESULT		LayerTransparentToggle( HWND, UINT );
HRESULT		LayerContentsImportable( HWND, UINT, LPINT, LPINT, UINT );
HRESULT		LayerBoxPositionChange( HWND , LONG, LONG );
HRESULT		LayerStringReplace( HWND , LPTSTR );

HRESULT		DocInitialise( UINT );

BOOLEAN		DocRangeIsError( FILES_ITR , INT, INT );

UINT_PTR	DocNowFilePageCount( VOID );
UINT_PTR	DocNowFilePageLineCount( VOID );

UINT		DocRawDataParamGet( LPCTSTR, PINT, PINT );

VOID		DocCaretPosMemory( UINT , LPPOINT );

HRESULT		DocOpenFromNull( HWND );

UINT		DocPageParamGet( PINT, PINT );
UINT		DocPageByteCount( FILES_ITR , INT, PINT, PINT );
HRESULT		DocPageInfoRenew( INT, UINT );
INT			DocPageMaxDotGet( INT, INT );
HRESULT		DocPageNameSet( LPTSTR );

INT			DocPageCreate( INT );
HRESULT		DocPageDelete( INT, INT );
HRESULT		DocPageChange( INT );

UINT		DocDelayPageLoad( FILES_ITR , INT );

HRESULT		DocModifyContent( UINT );

LPARAM		DocMultiFileCreate( LPTSTR );
HRESULT		DocActivateEmptyCreate( LPTSTR );

INT			DocLineParamGet( INT , PINT, PINT );

UINT		DocBadSpaceCheck( INT );
BOOLEAN		DocBadSpaceIsExist( INT );

HRESULT		DocPageDivide( HWND, HINSTANCE, INT );

INT			DocInputLetter( INT, INT, TCHAR );
INT			DocInputBkSpace( PINT, PINT );
INT			DocInputDelete( INT , INT );
INT			DocInputFromClipboard( PINT, PINT, PINT, UINT );

INT			DocAdditionalLine( INT, PBOOLEAN );

INT			DocStringAdd( PINT, PINT, LPCTSTR, INT );
HRESULT		DocCrLfAdd( INT, INT, BOOLEAN );
INT			DocSquareAdd( PINT, PINT, LPCTSTR, INT, LPPOINT * );
INT			DocStringErase( INT, INT, LPTSTR, INT );

INT			DocInsertLetter( PINT, INT, TCHAR );
INT			DocInsertString( PINT, PINT, PINT, LPCTSTR, UINT, BOOLEAN );

INT			DocIterateDelete( LETR_ITR, INT );
HRESULT		DocLineCombine( INT );

HRESULT		DocLineErase( INT, PBOOLEAN );

HRESULT		DocFrameInsert( INT, INT );
HRESULT		DocScreenFill( LPTSTR );

HRESULT		DocPageNumInsert( HINSTANCE, HWND );

INT			DocExClipSelect( UINT );
HRESULT		DocPageAllCopy( UINT );

INT			DocLetterShiftPos( INT, INT, INT, PINT, PBOOLEAN );
INT			DocLetterPosGetAdjust( PINT, INT, INT );

HRESULT		DocReturnSelStateToggle( INT, INT );
INT			DocRangeSelStateToggle( INT, INT, INT, INT );
UINT		DocLetterSelStateGet( INT, INT );
INT			DocPageSelStateToggle( INT );
HRESULT		DocSelRangeSet( INT, INT );
HRESULT		DocSelRangeGet( PINT, PINT );
HRESULT		DocSelRangeReset( PINT, PINT );
VOID		DocSelByteSet( INT );

HRESULT		DocSelText2PageName( VOID );

LPTSTR		DocClipboardDataGet( PUINT );
HRESULT		DocClipboardDataSet( LPVOID, INT, UINT );

INT			DocLineDataGetAlloc( INT, INT, LPLETTER *, PINT, PUINT );
LPSTR		DocPageTextPreviewAlloc( INT, PINT );

HRESULT		DocThreadDropCopy( VOID );

INT			DocPageTextGetAlloc( FILES_ITR, INT, UINT, LPVOID *, BOOLEAN );
INT			DocPageGetAlloc( UINT, LPVOID * );

INT			DocLineTextGetAlloc( FILES_ITR, INT, UINT, UINT, LPVOID * );

INT			DocSelectedDelete( PINT, PINT, UINT, BOOLEAN );
INT			DocSelectedBrushFilling( LPTSTR, PINT, PINT );
INT			DocSelectTextGetAlloc( UINT, LPVOID *, LPPOINT * );

HRESULT		DocExtractExecute( HINSTANCE  );

LPARAM		DocOpendFileCheck( LPTSTR );
HRESULT		DocFileSave( HWND, UINT );
HRESULT		DocFileOpen( HWND );
HRESULT		DocDoOpenFile( HWND, LPTSTR );
HRESULT		DocImageSave( HWND, UINT, HFONT );

HRESULT		DocHtmlExport( HWND );

UINT		DocStringSplitMLT( LPTSTR, INT, PAGELOAD );
UINT		DocStringSplitAST( LPTSTR, INT, PAGELOAD );

UINT		DocImportSplitASD( LPSTR, INT, PAGELOAD );

INT			DocLineStateCheckWithDot( INT, INT, PINT, PINT, PINT, PINT, PBOOLEAN );
HRESULT		DocRightGuideline( LPVOID );
INT			DocSpaceShiftProc( UINT, PINT, INT );
LPTSTR		DocPaddingSpaceMake( INT  );
LPTSTR		DocPaddingSpaceUni( INT, PINT, PINT, PINT );
LPTSTR		DocPaddingSpaceWithGap( INT, PINT, PINT );
LPTSTR		DocPaddingSpaceWithPeriod( INT, PINT, PINT, PINT, BOOLEAN );
HRESULT		DocLastSpaceErase( PINT , INT );
HRESULT		DocTopLetterInsert( TCHAR, PINT, INT );
HRESULT		DocLastLetterErase( PINT, INT );
HRESULT		DocTopSpaceErase( PINT, INT );
HRESULT		DocRightSlide( PINT , INT );

HRESULT		DocPositionShift( UINT, PINT, INT );
#ifdef DOT_SPLIT_MODE
HRESULT		DocCentreWidthShift( UINT vk, PINT, INT );
#endif
HRESULT		DocHeadHalfSpaceExchange( HWND );

LPTSTR		DocLastSpDel( vector<LETTER> * );

INT			DocDiffAdjBaseSet( INT );
INT			DocDiffAdjExec( PINT, INT );

VOID		ZeroONELINE( LPONELINE );
INT			DocStringInfoCount( LPCTSTR, UINT_PTR, PINT, PINT );

BOOLEAN		NowPageInfoGet( UINT, LPPAGEINFOS );

BOOLEAN		PageIsDelayed( FILES_ITR, UINT );

UINT		DocRangeDeleteByMozi( INT, INT, INT, INT, PBOOLEAN );

INT			DocUndoExecute( PINT, PINT );
INT			DocRedoExecute( PINT, PINT );

LPARAM		DocFileInflate( LPTSTR );
INT			DocFileCloseCheck( HWND, UINT );
HRESULT		DocClipLetter( TCHAR  );
VOID		DocBackupDirectoryInit( LPTSTR );
HRESULT		DocFileBackup( HWND );

HRESULT		DocMultiFileCloseAll( VOID );
LPARAM		DocMultiFileClose( HWND, LPARAM );
HRESULT		DocMultiFileSelect( LPARAM );
HRESULT		DocMultiFileModify( UINT  );
HRESULT		DocMultiFileStore( LPTSTR );
INT			DocMultiFileFetch( INT, LPTSTR, LPTSTR );
LPTSTR		DocMultiFileNameGet( INT  );

HRESULT		DocInverseInit( UINT  );
HRESULT		DocInverseTransform( UINT, UINT, PINT, INT );

HRESULT		SqnInitialise( LPUNDOBUFF );
HRESULT		SqnFreeAll( LPUNDOBUFF );
HRESULT		SqnSetting( VOID  );
UINT		SqnAppendLetter( LPUNDOBUFF, UINT, TCHAR, INT, INT, UINT );
UINT		SqnAppendString( LPUNDOBUFF, UINT, LPCTSTR, INT, INT, UINT );
UINT		SqnAppendSquare( LPUNDOBUFF, UINT, LPCTSTR, LPPOINT, INT, UINT );

HRESULT		UnicodeRadixExchange( LPVOID  );

INT			MoziInitialise( LPTSTR, HINSTANCE );
HWND		MoziScripterCreate( HINSTANCE, HWND );
HRESULT		MoziMoveFromView( HWND , UINT );

INT			VertInitialise( LPTSTR, HINSTANCE );
HWND		VertScripterCreate( HINSTANCE, HWND );
HRESULT		VertMoveFromView( HWND , UINT );

#ifdef FIND_STRINGS
HRESULT		FindDialogueOpen( HINSTANCE, HWND );
HRESULT		FindDirectly( HINSTANCE, HWND, INT );

#ifdef SEARCH_HIGHLIGHT
HRESULT		FindNowPageReSearch( VOID );
HRESULT		FindDelayPageReSearch( INT );
HRESULT		FindHighlightOff( VOID );
#endif
#endif

#endif

LPCTSTR		NextLineW( LPCTSTR );
LPTSTR		NextLineW( LPTSTR );

LPSTR		NextLineA( LPSTR  );

HRESULT	DraughtInitialise( HINSTANCE, HWND );
HWND	DraughtWindowCreate( HINSTANCE, HWND, UINT );

UINT	DraughtItemAddFromSelect( HWND , UINT );
UINT	DraughtItemAdding( HWND, LPSTR );

UINT	DraughtAaImageing( HWND, LPAAMATRIX );

INT		TextViewSizeGet( LPCTSTR, PINT );

INT_PTR	AacItemCount( UINT );
HBITMAP	AacArtImageGet( HWND, INT, LPSIZE, LPSIZE );

LPSTR	AacAsciiArtGet( DWORD );
INT		AacArtSizeGet( DWORD, PINT, PINT );
