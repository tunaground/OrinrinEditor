/*! @file
	@brief トレス機能の面倒見ます・画像保存の面倒も見ます
	このファイルは TraceCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/07/19
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 - 2013 Orinrin/SikigamiHNQ

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "OrinrinEditor.h"

//-------------------------------------------------------------------------------------------------

INT_PTR	CALLBACK TraceCtrlDlgProc( HWND, UINT, WPARAM, LPARAM );

HRESULT	TraceImageFileOpen( HWND );
HRESULT	TraceMoziColourChoice( HWND );

HRESULT	TraceRedrawIamge( VOID );

INT_PTR	TraceOnScroll( HWND, HWND, UINT, INT );
//-------------------------------------------------------------------------------------------------

/*!
	ImgCtl.dllをロードする
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	bMode	非０作成　０破壊
*/
INT TraceInitialise( HWND hWnd, UINT bMode ) {
	return -1;
}
//-------------------------------------------------------------------------------------------------

/*!
	モーダレストレスダイヤログを開けるを試みるですだよ
	@param[in]	hInst	アプリの実存
	@param[in]	hWnd	メインウインドウハンドルであるように
*/
HRESULT TraceDialogueOpen( HINSTANCE hInst, HWND hWnd ) {
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------

/*!
	トレスコントロールダイヤログのプロシージャ
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK TraceCtrlDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------



/*!
	開くダイアログのフックプロシージャ
*/
UINT_PTR CALLBACK ImageOpenDlgHookProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------


/*!
	画像を開いてアッー！
	@param[in]	hDlg	ダイヤログハンドル
*/
HRESULT TraceImageFileOpen( HWND hDlg ) {
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字用の、色選択ダイヤログーを開いてアッー！
	@param[in]	hDlg	ダイヤログハンドル
*/
HRESULT TraceMoziColourChoice( HWND hDlg ) {
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------

/*!
	各すらいだ〜が操作された
	@param[in]	hDlg	ダイヤローグハンドル
	@param[in]	hWndCtl	スライダのハンドル
	@param[in]	code	スクロールコード
	@param[in]	pos		スクロールボックス（つまみ）の位置
	@return		無し
*/
INT_PTR TraceOnScroll( HWND hDlg, HWND hWndCtl, UINT code, INT pos ) {
	return (INT_PTR)TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	イメージを再描画する
*/
HRESULT TraceRedrawIamge( VOID ) {
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------

/*!
	画像の表示非表示のON/OFFする
*/
HRESULT TraceImgViewTglExt( VOID ) {
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字描画色をお知らせする
	@param[in]	pColour	選択されている色を入れる
	@retval	非０	トレス中
	@retval	０		トレスモードになっていない
*/
UINT TraceMoziColourGet( LPCOLORREF pColour ) {
	return FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	イメージ開いてて表示有効な・ビュー描画で呼ばれる
	@param[in]	hdc		描画するデバイスコンテキスト
	@param[in]	iScrlX	描画領域のＸスクロールDot数
	@param[in]	iScrlY	描画領域のＹスクロールDot数
	@retval	非０		画像イメージあった
	@retval	０			画像は開かれていない
*/
UINT TraceImageAppear( HDC hdc, INT iScrlX, INT iScrlY ) {
	return FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	デバイスコンテキストの内容を、各形式で保存する
	@param[in]	hDC		保存したい内容がはいってるディバイスコンティキスト
	@param[in]	ptName	保存したいフルパス・拡張子は指定に合わせて修正しちゃう・MAX_PATHであること
	@param[in]	iType	ファイルタイプ指定　１：ＢＭＰ　２：ＪＰＧ　３：ＰＮＧ
	@return		HRESULT	終了状態コード
*/
HRESULT ImageFileSaveDC( HDC hDC, LPTSTR ptName, INT iType )
{
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------
