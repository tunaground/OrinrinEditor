#pragma once

#define STRICT

#include "resource.h"

ATOM	InitWndwClass( HINSTANCE );
BOOL	InitInstance( HINSTANCE, INT );
LRESULT	CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

BOOLEAN	Cls_OnCreate( HWND, LPCREATESTRUCT );
VOID	Cls_OnCommand( HWND , INT, HWND, UINT );
VOID	Cls_OnPaint( HWND );
VOID	Cls_OnDestroy( HWND );
HBRUSH	Cls_OnCtlColor( HWND , HDC, HWND, INT );
VOID	Cls_OnHotKey( HWND, INT, UINT, UINT );
VOID	Cls_OnDrawClipboard( HWND );
VOID	Cls_OnChangeCBChain( HWND, HWND, HWND );

VOID		WndTagSet( HWND, LONG_PTR );
LONG_PTR	WndTagGet( HWND );

BOOLEAN	SelectFileDlg( HWND, LPTSTR, UINT_PTR );

HRESULT	ToolTipSetting( HWND, UINT, LPTSTR );

HRESULT	TasktrayIconAdd( HWND );
VOID	TaskTrayIconEvent( HWND, UINT, UINT );
HRESULT	TaskTrayIconCaptionChange( HWND );
HRESULT	TaskTrayIconBalloon( HWND, LPTSTR, LPTSTR, DWORD );

INT		InitParamValue( UINT, UINT, INT );
HRESULT	InitClipStealOpen( UINT, UINT, LPTSTR );
HRESULT	InitSettingSave( HWND, UINT );

HRESULT	FileListViewInit( HWND );
INT		FileListViewAdd( HWND, LPTSTR );
HRESULT	FileListViewGet( HWND, INT, LPTSTR );
INT		FileListViewDelete( HWND  );

BOOLEAN	FileTypeCheck( LPTSTR );

HMENU	CreateFileSelMenu( HWND, UINT );

HRESULT	ClipStealDoing( HWND  );
LPTSTR	ClipboardDataGet( LPVOID  );

UINT	RegHotModExchange( UINT , BOOLEAN );

LPTSTR	SjisDecodeAlloc( LPSTR );
LPSTR	SjisEncodeAlloc( LPCTSTR );
LPSTR	SjisEntityExchange( LPCSTR );
TCHAR	UniRefCheck( LPSTR );
