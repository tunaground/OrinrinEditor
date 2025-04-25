#include "stdafx.h"
#include "OrinrinEditor.h"

LRESULT	CALLBACK PreviewWndProc( HWND, UINT, WPARAM, LPARAM );
VOID	Pvw_OnCommand( HWND , INT, HWND, UINT );
VOID	Pvw_OnSize( HWND , UINT, INT, INT );
VOID	Pvw_OnPaint( HWND );
VOID	Pvw_OnDestroy( HWND );

HRESULT	PreviewHeaderGet( VOID );

HRESULT	PreviewPageWrite( INT );

VOID PreviewInitialise( HINSTANCE hInstance, HWND hParentWnd ) {
	return;
}

HRESULT PreviewHeaderGet( VOID ) {
	return E_FAIL;
}

HRESULT PreviewVisibalise( INT iNowPage, BOOLEAN bForeg ) {
	return E_FAIL;
}

LRESULT CALLBACK PreviewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	return DefWindowProc( hWnd, message, wParam, lParam );
}

VOID Pvw_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify ) {
	return;
}

VOID Pvw_OnSize( HWND hWnd, UINT state, INT cx, INT cy ) {
	return;
}

VOID Pvw_OnPaint( HWND hWnd ) {
	return;
}

VOID Pvw_OnDestroy( HWND hWnd ) {
	return;
}

HRESULT PreviewPageWrite( INT iViewPage ) {
	return E_FAIL;
}
