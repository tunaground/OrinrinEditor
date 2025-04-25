#pragma once

#define STRICT

#define SPLITBAR_CLASS		TEXT("CSplitBar")
#define SPLITBAR_WIDTH		4
#define SPLITBAR_LEFTLIMIT	120

#define SIZE_SPLITBAR_MOVED	0xFFFF

ATOM	SplitBarClass( HINSTANCE );
HWND	SplitBarCreate( HINSTANCE, HWND, INT, INT, INT );
VOID	SplitBarPosGet( HWND, LPRECT );
LONG	SplitBarResize( HWND, LPRECT );
