#include "stdafx.h"
#include "OrinrinEditor.h"

#define VIEW_TRACE_CLASS	TEXT("TRACE_VIEW")

static HINSTANCE	ghInst;

extern  HWND	ghPrntWnd;
extern  HWND	ghViewWnd;

static  HWND	ghVwTrcWnd;

extern INT		gdHideXdot;
extern INT		gdViewTopLine;
extern SIZE		gstViewArea;
extern INT		gdDispingLine;

extern HFONT	ghAaFont;
