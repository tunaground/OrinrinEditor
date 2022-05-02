/*! @file
	@brief ブラウザコンポーネント使ってPreviewをします
	このファイルは DocPreview.cpp です。
	@author	SikigamiHNQ
	@date	2011/09/01
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

/*
IEコンポつかうにはATLが要る
ttp://ysmt.blog21.fc2.com/blog-entry-244.html

VCExpressにはATL入ってないので、別口でゲットする・VCProとかには入ってるかも

WindowsDriverKit710に入ってる
がいど
ttp://www.microsoft.com/japan/whdc/DevTools/WDK/WDKpkg.mspx
ダウソ
ttp://www.microsoft.com/download/en/details.aspx?displaylang=en&id=11800


インクルードファイルパスに、"(DDKフォルダ)\inc\atl71"
ライブラリファイルパスに、"(DDKフォルダ)\lib\atl\i386"を追加。

名前はATL71だけどバージョンは80

リンクでエラーが出るなら、
リンカの追加の依存ファイルに、atlthunk.libを付ける。

*/

LRESULT	CALLBACK PreviewWndProc( HWND, UINT, WPARAM, LPARAM );	//!<	
VOID	Pvw_OnCommand( HWND , INT, HWND, UINT );	//!<	WM_COMMAND の処理
VOID	Pvw_OnSize( HWND , UINT, INT, INT );		//!<	
VOID	Pvw_OnPaint( HWND );						//!<	WM_PAINT の処理・枠線描画とか
VOID	Pvw_OnDestroy( HWND );						//!<	WM_DESTROY の処理・BRUSHとかのオブジェクトの破壊を忘れないように

HRESULT	PreviewHeaderGet( VOID );	//!<	

HRESULT	PreviewPageWrite( INT );	//!<	
//-------------------------------------------------------------------------------------------------

/*!
	プレビューウインドウクラスを作成
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hParentWnd	親ウインドウのハンドル
	@return	なし
*/
VOID PreviewInitialise( HINSTANCE hInstance, HWND hParentWnd ) {
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	テンプレディレクトリに入ってるHeaderFooterファイルの中身をゲットする
*/
HRESULT PreviewHeaderGet( VOID ) {
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------

/*！
	ウインドウを作成
	@param[in]	iNowPage	プレビュりたい頁番号
	@param[in]	bForeg		非０なら再プレビューのときにフォアグランドにする・０ならしない・プレビュー開いてないなら何もしない
	@return	HRESULT	終了状態コード
*/
HRESULT PreviewVisibalise( INT iNowPage, BOOLEAN bForeg ) {
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------


/*!
	ウインドウプロシージャ
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK PreviewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	COMMANDメッセージの受け取り。ボタン押されたとかで発生
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	id			メッセージを発生させた子ウインドウの識別子	LOWORD(wParam)
	@param[in]	hWndCtl		メッセージを発生させた子ウインドウのハンドル	lParam
	@param[in]	codeNotify	通知メッセージ	HIWORD(wParam)
	@return		なし
*/
VOID Pvw_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify ) {
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	サイズ変更された
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	state	なにかの状態
	@param[in]	cx		変更されたクライヤント幅
	@param[in]	cy		変更されたクライヤント高さ
*/
VOID Pvw_OnSize( HWND hWnd, UINT state, INT cx, INT cy ) {
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Pvw_OnPaint( HWND hWnd ) {
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウを閉じるときに発生。デバイスコンテキストとか確保した画面構造のメモリとかも終了。
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Pvw_OnDestroy( HWND hWnd ) {
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁を表示する
	@param[in]	iViewPage	頁番号・−１なら全頁
*/
HRESULT PreviewPageWrite( INT iViewPage ) {
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------
