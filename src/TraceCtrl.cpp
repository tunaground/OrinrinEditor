#include "stdafx.h"
#include "OrinrinEditor.h"

INT_PTR	CALLBACK TraceCtrlDlgProc( HWND, UINT, WPARAM, LPARAM );

HRESULT	TraceImageFileOpen( HWND );
HRESULT	TraceMoziColourChoice( HWND );

HRESULT	TraceRedrawIamge( VOID );

INT_PTR	TraceOnScroll( HWND, HWND, UINT, INT );

INT TraceInitialise( HWND hWnd, UINT bMode ) {
	return -1;
}

HRESULT TraceDialogueOpen( HINSTANCE hInst, HWND hWnd ) {
	return E_FAIL;
}

INT_PTR CALLBACK TraceCtrlDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
	return (INT_PTR)FALSE;
}

UINT_PTR CALLBACK ImageOpenDlgHookProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
	return (INT_PTR)FALSE;
}

HRESULT TraceImageFileOpen( HWND hDlg ) {
	return E_FAIL;
}

HRESULT TraceMoziColourChoice( HWND hDlg ) {
	return E_FAIL;
}

INT_PTR TraceOnScroll( HWND hDlg, HWND hWndCtl, UINT code, INT pos ) {
	return (INT_PTR)TRUE;
}

HRESULT TraceRedrawIamge( VOID ) {
	return E_FAIL;
}

HRESULT TraceImgViewTglExt( VOID ) {
	return E_FAIL;
}

UINT TraceMoziColourGet( LPCOLORREF pColour ) {
	return FALSE;
}

UINT TraceImageAppear( HDC hdc, INT iScrlX, INT iScrlY ) {
	return FALSE;
}

HRESULT ImageFileSaveDC( HDC hDC, LPTSTR ptName, INT iType )
{
	return E_FAIL;
}
