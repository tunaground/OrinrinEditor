#include "stdafx.h"
#include "OrinrinEditor.h"

#ifdef FIND_STRINGS

extern list<ONEFILE>	gltMultiFiles;

extern FILES_ITR	gitFileIt;

extern INT		gixFocusPage;

EXTERNED HWND	ghFindDlg;

static TCHAR	gatLastPtn[MAX_PATH];

static TCHAR	atSetPattern[MAX_PATH];
static INT		giSetRange;
static BOOLEAN	gbSetModCrlf;

static  UINT	gdNextStart;
static   INT	giSearchPage;

INT_PTR		CALLBACK FindStrDlgProc( HWND, UINT, WPARAM, LPARAM );
HRESULT		FindExecute( HWND );
INT_PTR		FindPageSearch( LPTSTR, INT, FILES_ITR );

UINT_PTR	SearchPatternStruct( LPTSTR, UINT_PTR, LPTSTR, BOOLEAN );

#ifdef SEARCH_HIGHLIGHT
INT		FindPageHighlightOff( INT , FILES_ITR );
HRESULT	FindPageHighlightSet( INT, INT, INT, FILES_ITR );
HRESULT	FindLineHighlightOff( UINT , LINE_ITR );
#endif
HRESULT		FindPageSelectSet( INT, INT, INT, FILES_ITR );

HRESULT FindDialogueOpen( HINSTANCE hInst, HWND hWnd )
{

	if( !(hInst) || !(hWnd) )
	{
		gdNextStart = 0;
		giSearchPage = 0;

		ZeroMemory( gatLastPtn, sizeof(gatLastPtn) );

		return S_OK;
	}

	if( ghFindDlg )
	{
		SetForegroundWindow( ghFindDlg );
		return S_FALSE;
	}

	ghFindDlg = CreateDialogParam( hInst, MAKEINTRESOURCE(IDD_FIND_STRING_DLG), hWnd, FindStrDlgProc, 0 );

	ShowWindow( ghFindDlg, SW_SHOW );

	return S_OK;
}

INT_PTR CALLBACK FindStrDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	HWND	hWorkWnd;
	UINT	id;

	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:
#ifndef SEARCH_HIGHLIGHT
			EnableWindow( GetDlgItem(hDlg,IDB_FIND_CLEAR), FALSE );
			ShowWindow( GetDlgItem(hDlg,IDB_FIND_CLEAR), SW_HIDE );
#endif
			ZeroMemory( atSetPattern, sizeof(atSetPattern) );
			giSetRange = 0;
			gbSetModCrlf = 0;
			gdNextStart = 0;
			giSearchPage = 0;

			hWorkWnd = GetDlgItem( hDlg, IDCB_FIND_TARGET );
			ComboBox_InsertString( hWorkWnd, 0, TEXT("현재 보고 있는 페이지") );
			ComboBox_InsertString( hWorkWnd, 1, TEXT("이 파일 전체") );

			ComboBox_SetCurSel(  hWorkWnd, giSetRange );

			hWorkWnd = GetDlgItem( hDlg, IDE_FIND_TEXT );
			Edit_SetText( hWorkWnd, gatLastPtn );
			SetFocus( hWorkWnd );

			return (INT_PTR)FALSE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			hWorkWnd = GetDlgItem( hDlg, IDE_FIND_TEXT );
			switch( id )
			{
				case IDCANCEL:	DestroyWindow( hDlg );	return (INT_PTR)TRUE;
				case IDOK:		FindExecute( hDlg );	return (INT_PTR)TRUE;

				case IDM_PASTE:	SendMessage( hWorkWnd, WM_PASTE, 0, 0 );	return (INT_PTR)TRUE;
				case IDM_COPY:	SendMessage( hWorkWnd, WM_COPY,  0, 0 );	return (INT_PTR)TRUE;
				case IDM_CUT:	SendMessage( hWorkWnd, WM_CUT,   0, 0 );	return (INT_PTR)TRUE;
				case IDM_UNDO:	SendMessage( hWorkWnd, WM_UNDO,  0, 0 );	return (INT_PTR)TRUE;

#ifdef SEARCH_HIGHLIGHT
				case IDB_FIND_CLEAR:	FindHighlightOff(  );	return (INT_PTR)TRUE;
#endif
				default:	break;
			}
			break;

		case WM_CLOSE:
			DestroyWindow( hDlg );
			return (INT_PTR)TRUE;

		case WM_DESTROY:
			ghFindDlg = NULL;
			ViewFocusSet(  );
			return (INT_PTR)TRUE;

	}

	return (INT_PTR)FALSE;
}

HRESULT FindDirectly( HINSTANCE hInst, HWND hWnd, INT dCommand )
{
	 BOOLEAN	bOnCrLf = FALSE;
	 INT		cbSize;
	UINT_PTR	cchSize, d;
	LPTSTR		ptText;

	TCHAR	atGetPttn[MAX_PATH];

	if( IDM_FIND_JUMP_NEXT == dCommand )
	{
		FindExecute( NULL );
	}
	else if( IDM_FIND_TARGET_SET == dCommand )
	{
		ZeroMemory( atGetPttn, sizeof(atGetPttn) );

		cbSize = DocSelectTextGetAlloc( D_UNI, (LPVOID *)(&ptText), NULL );
		StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );
		if(  0 == cchSize ){		FREE(ptText);	return  E_ABORT;	}
		if( MAX_PATH <= cchSize ){	FREE(ptText);	return  E_ABORT;	}

		for( d = 0; cchSize > d; d++ )
		{
			if( 0x000D == ptText[d] && 0x000A ==ptText[d+1] )
			{
				atGetPttn[d] = TEXT('\\');	d++;
				atGetPttn[d] = TEXT('n');

				bOnCrLf = TRUE;
			}
			else
			{
				atGetPttn[d] = ptText[d];
			}
		}

		//	검색 문자열을 변경한다
		StringCchCopy( gatLastPtn, MAX_PATH, atGetPttn );
		gbSetModCrlf = bOnCrLf;	//	개행 체크
		giSetRange   = 1;	//	검색 범위는 파일 전체로 고정
		gdNextStart  = 0;	//	새로운 검색
		giSearchPage = 0;

		FindExecute( NULL );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	검색 실행
	@param[in]	hDlg	다이얼로그 핸들
*/
HRESULT FindExecute( HWND hDlg )
{
//	HWND	hWorkWnd;
//	UINT		dStartPage;
	UINT_PTR	cchSzPtn;
	 INT_PTR	iPage;
	 INT_PTR	iFindTop;
	 INT		dRange;	//	검색 범위 0 페이지 1 파일  취소＞2 전체 열린 파일
	 BOOLEAN	bModCrlf;

//	 BOOLEAN	bSequenSearch = FALSE;	//	연속 검색 중인지

	TCHAR	atPattern[MAX_PATH], atBuf[MAX_PATH];

	if( hDlg )
	{
		//검색 패턴 확보
		Edit_GetText( GetDlgItem(hDlg,IDE_FIND_TEXT), atBuf, MAX_PATH );
		if( !(atBuf[0]) )	return  E_ABORT;	//	빈 문자열이면 아무것도 하지 않음

		//	¥n을 개행, ¥¥을 ¥로 변환할지
		bModCrlf = IsDlgButtonChecked( hDlg, IDCB_MOD_CRLF_YEN );

		//	검색 범위	0 페이지 1 파일
		dRange = ComboBox_GetCurSel( GetDlgItem(hDlg,IDCB_FIND_TARGET) );

		//	검색 조건이 모두 같다면, 이어서
		if( !StrCmp( atSetPattern, atBuf ) && (gbSetModCrlf == bModCrlf) && (giSetRange == dRange) )
		{
	//		bSequenSearch = TRUE;
		}
		else	//	다르면 처음부터
		{
			gdNextStart = 0;
			giSearchPage = 0;
		}

		StringCchCopy( atSetPattern, MAX_PATH, atBuf );
		gbSetModCrlf = bModCrlf;
		giSetRange = dRange;

		StringCchCopy( gatLastPtn, MAX_PATH, atBuf );	//	다음에 다이얼로그를 열 때 표시용
	}
	else	//	F3으로 직접 왔을 때
	{
		if( 0 == gatLastPtn[0] )	return  E_ABORT;	//	아무것도 하지 않음

		//	직전 설정을 재사용
		StringCchCopy( atBuf, MAX_PATH, gatLastPtn );
		bModCrlf = gbSetModCrlf;
		dRange = giSetRange;
	}

	//	검색 패턴 확정
	cchSzPtn = SearchPatternStruct( atPattern, MAX_PATH, atBuf, bModCrlf );
#if 0
	ZeroMemory( atPattern, sizeof(atPattern) );
//	giCrLfCnt = 0;	//	개행 카운트・보통 0일 듯
	if( bModCrlf )	//	이스케이프 시퀀스를 확장
	{
		for( d = 0, h = 0; MAX_PATH > d; d++, h++ )
		{
			atPattern[h] = atBuf[d];
			if( 0x005C == atBuf[d] )	//	0x005C는 ¥
			{
				d++;
				if( TEXT('n') ==  atBuf[d] )	//	개행 지시일 경우
				{
					atPattern[h] = TEXT('\r');	h++;
					atPattern[h] = TEXT('\n');
				//	giCrLfCnt++;	//	개행 카운트 증가
				}
			}
			if( 0x0000 == atBuf[d] )	break;
		}
	}
	else
	{
		StringCchCopy( atPattern, MAX_PATH, atBuf );
	}

	StringCchLength( atPattern, MAX_PATH, &cchSzPtn );
#endif

#ifdef SEARCH_HIGHLIGHT
#error 기능을 변경했으므로 하이라이트는 사용할 수 없습니다
		FindHighlightOff(  );	//	이전 패턴 폐기
#endif

	if( dRange )	//	전체 페이지 검색
	{
		iPage = DocNowFilePageCount(  );	//	페이지 수 확보

		do{
			iFindTop = FindPageSearch( atPattern, giSearchPage, gitFileIt );	//	대상 페이지 검색
			if(  0 <= iFindTop )	//	발견
			{
				if( giSearchPage != gixFocusPage )	//	현재 페이지가 아니라면 해당 페이지로 이동
				{
					DocPageChange( giSearchPage );	//	페이지 이동・gixFocusPage가 변경됨
				}
				FindPageSelectSet( iFindTop, cchSzPtn, gixFocusPage, gitFileIt );	//	해당 위치로 커서 점프하고 선택 상태로 만듦
				gdNextStart = iFindTop + cchSzPtn;
				break;
			}
			else	//	이 페이지에는 없었음
			{
				 giSearchPage++;	//	다음 페이지로 이동
				gdNextStart = 0;	//	다음은 다시 처음부터 검색
				if( iPage <=  giSearchPage )	//	끝을 넘어가면
				{
					giSearchPage = 0;
					break;
				}
			}

		}while( 0 > iFindTop );	//	찾을 때까지 페이지 이동
	}
	else	//	현재 페이지만
	{
		iFindTop = FindPageSearch( atPattern, gixFocusPage, gitFileIt );	//	단일 페이지 검색
		if( 0 <=  iFindTop )	//	처음부터의 문자 수
		{
			FindPageSelectSet( iFindTop, cchSzPtn, gixFocusPage, gitFileIt );	//	해당 위치로 커서 점프하고 선택 상태로 만듦
			gdNextStart = iFindTop + cchSzPtn;
		}
		else{	gdNextStart = 0;	}	//	처음부터
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	검색 패턴을 확정한다
	@param[out]	ptDest		패턴을 넣을 버퍼
	@param[in]	cchSize		버퍼 크기
	@param[in]	ptSource	원본 문자열
	@param[in]	bCrLf		이스케이프 시퀀스를 확장할지
	@return	문자 수
*/
UINT_PTR SearchPatternStruct( LPTSTR ptDest, UINT_PTR cchSize, LPTSTR ptSource, BOOLEAN bCrLf )
{
	UINT_PTR	d, h;
	UINT_PTR	cchSzPtn;

	ZeroMemory( ptDest, sizeof(TCHAR) * cchSize );

//	giCrLfCnt = 0;	//	개행 카운트・보통 0일 듯

	if( bCrLf )	//	이스케이프 시퀀스를 확장
	{
		for( d = 0, h = 0; cchSize > d; d++, h++ )
		{
			ptDest[h] = ptSource[d];
			if( 0x005C == ptSource[d] )	//	0x005C는 ¥
			{
				d++;
				if( TEXT('n') ==  ptSource[d] )	//	개행 지시일 경우
				{
					ptDest[h] = TEXT('\r');	h++;
					ptDest[h] = TEXT('\n');
				//	giCrLfCnt++;	//	개행 카운트 증가
				}
			}
			if( 0x0000 == ptSource[d] )	break;
		}
	}
	else
	{
		StringCchCopy( ptDest, cchSize, ptSource );
	}

	StringCchLength( ptDest, cchSize, &cchSzPtn );

	return cchSzPtn;
}
//-------------------------------------------------------------------------------------------------

/*!
	지정 패턴을, 지정 파일의, 지정 페이지에서 검색
	@param[in]	ptPattern	검색 패턴・NULL은 불가
	@param[in]	iTgtPage	대상 페이지
	@param[in]	itFile		파일 이터레이터
	@return	히트하지 않으면 -1, 히트하면, 처음부터의 문자 수・개행은 2문자 취급
*/
INT_PTR FindPageSearch( LPTSTR ptPattern, INT iTgtPage, FILES_ITR itFile )
{
	 INT		dCch;//, dLeng;
	 INT_PTR	iRslt;
	 INT_PTR	dBytes;
	UINT_PTR	cchSize, cchSzPtn;
	LPTSTR		ptPage, ptCaret, ptFind = NULL;

//	TCHAR	ttBuf;

	TRACE( TEXT("PAGE[%d] SEARCH"), iTgtPage );

	if( !(ptPattern)  ){	return -1;	}	//	NULL 불가

	StringCchLength( ptPattern, MAX_PATH, &cchSzPtn );

	//	지연되었다면 체크하지 않아도 됨＜그럴 리 없음
//	if( PageIsDelayed( itFile, iTgtPage ) ){	return   -1;	}

	//	페이지 전체 확보
	dBytes = DocPageTextGetAlloc( itFile, iTgtPage, D_UNI, (LPVOID *)(&ptPage), FALSE );
	StringCchLength( ptPage, STRSAFE_MAX_CCH, &cchSize );

	ptCaret = ptPage;

	ptCaret += gdNextStart;	//	직전 검색 위치까지 오프셋

	iRslt = -1;

#ifdef SEARCH_HIGHLIGHT
	//	검색
	dLeng = 0;
	do
	{
		ptFind = FindStringProc( ptCaret, ptPattern, &dCch );	//	검색 본체・에디터 측
		if( !(ptFind) ){	break;	}	//	히트하지 않음

		dLeng += dCch;	//	문자 위치・0 인덱스
		ttBuf = ptPage[dLeng];

		//	히트한 부분에 색을 입힘
		FindPageHighlightSet( dLeng, cchSzPtn, iTgtPage, itFile );	//	내부에서 해당 영역의 재그리기까지 지정

		hRslt = S_OK;

		ptCaret = ptFind;
		ptCaret++;

		dLeng++;

	}while( *ptCaret );
#else
	ptFind = FindStringProc( ptCaret, ptPattern, &dCch );	//	검색 본체・에디터 측
	if( ptFind )	//	무언가 있음
	{
		dCch += gdNextStart;	//	오프셋 양을 더해둠

	//	FindPageSelectSet( dCch, cchSzPtn, iTgtPage, itFile );	//	해당 위치로 커서 점프하고 선택 상태로 만듦
		//외부에서 처리하도록 함

	//	iRslt = dCch + cchSzPtn;
		iRslt = dCch;
	}

#endif

	FREE(ptPage);

	return iRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	지정 파일의 지정 페이지의 지정 문자 위치에서 지정 문자 수를 선택 상태로 만듦. 개행 코드 포함.
	@param[in]	iOffset	페이지 처음부터의 문자 수・개행 코드 포함.
	@param[in]	iRange	검색 대상의 문자 수
	@param[in]	iPage	대상 페이지
	@param[in]	itFile	파일 이터레이터
	@return		HRESULT	종료 상태 코드
*/
HRESULT FindPageSelectSet( INT iOffset, INT iRange, INT iPage, FILES_ITR itFile )
{
	UINT_PTR	ln, iLetters;//, iLines;
	 INT_PTR	dMozis;
	 INT		iTotal, iDot, iLnTop, iSlide, mz, iNext, iWid = 0;
	 INT		iEndTotal, iEndOffset;
//	RECT		inRect;

	LINE_ITR	itLine, itLnEnd;

	itLine  = itFile->vcCont.at( iPage ).ltPage.begin();
	itLnEnd = itFile->vcCont.at( iPage ).ltPage.end();

	iEndOffset = iOffset + iRange;
	iEndTotal = 0;
	iTotal = 0;
	iLnTop = 0;
	for( ln = 0; itLnEnd != itLine; itLine++, ln++ )
	{
		dMozis = itLine->vcLine.size( );	//	이 행의 문자 수 확인
		iLetters = dMozis + 2;	//	개행 코드

		iTotal += iLetters;

		if( iOffset < iTotal )	//	행 끝까지의 문자 수보다 오프셋이 작으면, 그 행에 포함됨
		{
			iSlide = iOffset - iLnTop;	//	그 행 처음부터의 문자 수
			//	만약 개행부터 검색이라면, iSlide = dMozis가 됨
			iNext = 0;	//	개행이 있는 경우의 남은 문자 수

			//	여기서 개행의 포함 상황을 확인하고, 다음 행 겹침 등을 체크?
			//if( dMozis < (iSlide + iRange) )	//	성립하면, 개행이 넘침
			//{
			//	iNext  = iRange;
			//	iRange = dMozis - iSlide;	//	남은 문자 수
			//	iNext -= iRange;	//	개행을 포함한 남은 문자 수
			//}

			iDot = 0;	//	거기까지의 점 수를 모아둠
			for( mz = 0; iSlide > mz; mz++ )	//	해당 문자까지 진행하여 점 수 모아둠
			{
				//	만약 개행부터 검색이라면 이게 성립
				if( dMozis <=  mz ){	iDot += iWid;	break;	}

				iDot += itLine->vcLine.at( mz ).rdWidth;

				iWid  = itLine->vcLine.at( mz ).rdWidth;	//	이 문자의 폭
			}

			//	해당 범위를 선택 상태로 만듦
			DocPageSelStateToggle(  FALSE );	//	일단 선택 상태는 해제

			ViewPosResetCaret( iDot, ln );	//	커서를 그곳으로 이동
			ViewSelMoveCheck( FALSE );	//	범위 선택 시작 상태
			ViewSelPositionSet( NULL );

			//	범위 선택, ViewSelAreaSelect를 참고

			break;
		}

		iLnTop += iLetters;

		iEndTotal += iLetters;	//	검색 내용의 끝 검출용
	}

	//	끝 위치 특정 시작
	for( ; itLnEnd != itLine; itLine++, ln++ )	//	오프셋 발견된 행부터 시작
	{
		dMozis = itLine->vcLine.size( );	//	이 행의 문자 수 확인
		iLetters = dMozis + 2;	//	개행 코드

		iEndTotal += iLetters;

		if( iEndOffset < iEndTotal )	//	행 끝까지의 문자 수보다 오프셋이 작으면, 그 행에 포함됨
		{
			iSlide = iEndOffset - iLnTop;	//	그 행 처음부터의 문자 수

			iWid = 0;
			iDot = 0;	//	거기까지의 점 수를 모아둠
			for( mz = 0; iSlide > mz; mz++ )	//	해당 문자까지 진행하여 점 수 모아둠
			{
				//	만약 개행부터 검색이라면 이게 성립
				if( dMozis <=  mz ){	iDot += iWid;	break;	}

				iDot += itLine->vcLine.at( mz ).rdWidth;

				iWid  = itLine->vcLine.at( mz ).rdWidth;	//	이 문자의 폭
			}

			ViewPosResetCaret( iDot, ln );	//	커서를 그곳으로 이동
			//	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	여기서 캐럿도 이동

			ViewSelMoveCheck( TRUE );
			ViewSelPositionSet( NULL );	//	이동한 위치를 기록

			break;
		}

		iLnTop += iLetters;
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#ifdef SEARCH_HIGHLIGHT
/*!
	지정 파일의 지정 페이지의 지정 문자 위치에서 지정 문자 수를 하이라이트 지정. 개행 코드 포함.
	@param[in]	iOffset	페이지 처음부터의 문자 수・개행 코드 포함.
	@param[in]	iRange	하이라이트 문자 수
	@param[in]	iPage	대상 페이지
	@param[in]	itFile	파일 이터레이터
	@return		HRESULT	종료 상태 코드
*/
HRESULT FindPageHighlightSet( INT iOffset, INT iRange, INT iPage, FILES_ITR itFile )
{
	UINT_PTR	ln, iLetters;//, iLines;
	INT_PTR		dMozis;
	INT			iTotal, iDot, iLnTop, iSlide, mz, iNext, iWid = 0;
	RECT		inRect;

	LINE_ITR	itLine, itLnEnd;

	itLine  = itFile->vcCont.at( iPage ).ltPage.begin();
	itLnEnd = itFile->vcCont.at( iPage ).ltPage.end();

	iTotal = 0;
	iLnTop = 0;
	for( ln = 0; itLnEnd != itLine; itLine++, ln++ )
	{
		inRect.top    = ln * LINE_HEIGHT;
		inRect.bottom = inRect.top + LINE_HEIGHT;
		inRect.left   = 0;
		inRect.right  = 0;

		dMozis = itLine->vcLine.size( );	//	이 행의 문자 수 확인
		iLetters = dMozis + 2;	//	개행 코드

		iTotal += iLetters;

		if( iOffset < iTotal )	//	행 끝까지의 문자 수보다 오프셋이 작으면, 그 행에 포함됨
		{
			iSlide = iOffset - iLnTop;	//	그 행 처음부터의 문자 수
			//	만약 개행부터 검색이라면, iSlide = dMozis가 됨
			iNext = 0;	//	개행이 있는 경우의 남은 문자 수

			//	여기서 개행의 포함 상황을 확인하고, 다음 행 겹침 등을 체크?
			if( dMozis < (iSlide + iRange) )	//	성립하면, 개행이 넘침
			{
				iNext  = iRange;
				iRange = dMozis - iSlide;	//	남은 문자 수
				iNext -= iRange;	//	개행을 포함한 남은 문자 수
			}

			iDot = 0;	//	거기까지의 점 수를 모아둠
			for( mz = 0; iSlide > mz; mz++ )	//	해당 문자까지 진행하여 점 수 모아둠
			{
				//	만약 개행부터 검색이라면 이게 성립
				if( dMozis <=  mz ){	iDot += iWid;	break;	}

				iDot += itLine->vcLine.at( mz ).rdWidth;

				iWid  = itLine->vcLine.at( mz ).rdWidth;	//	이 문자의 폭
			}

			for(  ; (iSlide+iRange) > mz; mz++ )
			{
				//	개행까지 파고들면 여기 성립
				if( dMozis <=  mz ){	break;	}

				iWid = itLine->vcLine.at( mz ).rdWidth;

				inRect.left  = iDot;
				inRect.right = iDot + iWid;

				itLine->vcLine.at( mz ).mzStyle |= CT_FINDED;
				ViewRedrawSetRect( &inRect );

				iDot += iWid;
			}

			if( 0 <	 iNext )	//	개행이 있으면 수치가 존재
			{
				inRect.right += 20;

				itLine->dStyle |= CT_FINDRTN;
				ViewRedrawSetRect( &inRect );

				iNext -=  2;	//	개행 문자만큼 줄임

			}

			//	필요에 따라, 여기서 끝내거나 다음 행의 처리로 이동
			if( 0 >= iNext ){	break;	}
			else
			{
				iOffset += (2 + iRange);	//	개행 문자만큼＋이 행에서 사용한 문자 수만큼 진행
				iRange  = iNext;	//	남은 문자 수 변경
			}
		}

		iLnTop += iLetters;
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	모든 하이라이트 OFF
	@return	HRESULT	종료 상태 코드
*/
HRESULT FindHighlightOff( VOID )
{
	UINT_PTR	dPage, d;

	dPage = DocNowFilePageCount( );

	for( d = 0; dPage > d; d++ )
	{
		FindPageHighlightOff( d, gitFileIt );
	}

	//	현재 내용도 초기화
	FindDialogueOpen( NULL, NULL );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	지정 파일의 지정 페이지의 검색 하이라이트를 OFF
	@param[in]	iPage	대상 페이지
	@param[in]	itFile	파일 이터레이터
	@return		특별히 없음
*/
INT FindPageHighlightOff( INT iPage, FILES_ITR itFile )
{
	UINT_PTR	ln;//, iLines, iLetters, mz;
//	UINT		dStyle;
//	INT			iDot, iWid;
//	RECT		inRect;

	LINE_ITR	itLine, itLnEnd;

	if( 0 >  iPage )	return 0;	//	특수한 상황에서는 처리하지 않음

	ZeroMemory( gatLastPtn, sizeof(gatLastPtn) );

	itLine  = itFile->vcCont.at( iPage ).ltPage.begin();
	itLnEnd = itFile->vcCont.at( iPage ).ltPage.end();

	for( ln = 0; itLnEnd != itLine; itLine++, ln++ )
	{
		FindLineHighlightOff( ln, itLine );	//	내부에서 REDRAW 처리
	}

//	ViewRedrawSetLine( -1 );	//	화면 표시 갱신

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	지정 행의 검색 하이라이트를 OFF
	@param[in]	iLine	행 수・0 인덱스
	@param[in]	itFile	행 이터레이터
	@return	HRESULT	종료 상태 코드
*/
HRESULT FindLineHighlightOff( UINT iLine, LINE_ITR itLine )
{
	UINT_PTR	iLetters, mz;//, iLines, ln;
	UINT		dStyle;
	INT			iDot, iWid;
	RECT		inRect;

	iDot = 0;	//	거기까지의 점 수를 모아둠
	inRect.top    = iLine * LINE_HEIGHT;
	inRect.bottom = inRect.top + LINE_HEIGHT;

	iLetters = itLine->vcLine.size();
	//	한 문자씩, 전부를 체크
	for( mz = 0; iLetters > mz; mz++ )
	{
		//	직전 상태
		dStyle = itLine->vcLine.at( mz ).mzStyle;
		iWid   = itLine->vcLine.at( mz ).rdWidth;

		inRect.left  = iDot;
		inRect.right = iDot + iWid;

		itLine->vcLine.at( mz ).mzStyle &= ~CT_FINDED;
		if( dStyle & CT_FINDED )	ViewRedrawSetRect( &inRect );

		iDot += iWid;
	}

	//	한 행 끝나면 끝 상황 확인. 개행・본문 끝에 개행 없음・선택일 때만
	dStyle = itLine->dStyle;
	inRect.left  = iDot;
	inRect.right = iDot + 20;	//	개행 그리기 영역・대략 이 정도

	itLine->dStyle &=  ~CT_FINDRTN;
	if( dStyle & CT_FINDRTN )	ViewRedrawSetRect( &inRect );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#endif

#if 0
/*!
	검색 위치로 점프
	@param[in]	dMode	0 다음으로 1 이전으로
	@param[in]	pxDot	커서 점 위치・처리 후 반환
	@param[in]	pyLine	커서 행・처리 후 반환
	@param[in]	pMozi	캐럿의 왼쪽 문자 수・처리 후 반환
	@return		INT		음수: 에러 0: 히트 없음 1〜: 히트함
*/
INT FindStringJump( UINT dMode, PINT pXdot, PINT pYline, PINT pMozi )
{
	INT			iXdot, iYline, iMozi;
	BOOLEAN		bStart, bBegin;
	INT_PTR		dTotalPage, dTotalLine;
	PAGE_ITR	itPage;	//	페이지를 순서대로 봄
	LINE_ITR	itLine;	//	행을 순서대로 봄
	LETR_ITR	itMozi;	//	문자를 순서대로 봄

	if( !(pXdot) || !(pYline) || !(pMozi) ){	return -1;	}

	//	페이지 초과 체크
	dTotalPage = DocNowFilePageCount();
	if( !(0 <= gixFocusPage && gixFocusPage < dTotalPage) ){	return -1;	}
	//	현재 페이지에 주목
	itPage = gitFileIt->vcCont.begin();
	std::advance( itPage, gixFocusPage );

	iXdot  = *pXdot;
	iYline = *pYline;
	iMozi  = *pMozi;

	//	행 초과 체크
	dTotalLine = itPage->ltPage.size();
	if( !(0 <= iYline && iYline < dTotalLine) ){	return -1;	}
	//	현재 행에 주목
	itLine = itPage->ltPage.begin();
	std::advance( itLine, iYline );

	itMozi = itLine->vcLine.begin();
	std::advance( itMozi, iMozi );

	bStart = TRUE;	bBegin = TRUE;
	for( ; itPage->ltPage.end() != itLine; itLine++ )
	{
		//	처음은 그대로, 다음부터는 그 행의 첫 문자부터 봄
		if( bStart ){	bStart =  FALSE;	}
		else{	itMozi = itLine->vcLine.begin();	}

		for( ;itLine->vcLine.end() != itMozi; itMozi++ )
		{
			if( itMozi->mzStyle & CT_FINDED )	//	히트
			{
				if( !(bBegin)  )	//	검색 시작 위치는 무시
				{
					*pXdot  = iXdot;
					*pYline = iYline;
					*pMozi  = iMozi;

					ViewDrawCaret( iXdot, iYline, TRUE );	//	캐럿도 이동

					return 1;
					//	페이지 넘김은 어떻게 할지
				}
			}
			else{	bBegin =  FALSE;	}
			//

			iXdot += itMozi->rdWidth;
			iMozi++;
		}

		iYline++;	//	행 번호 증가
		iXdot = 0;
		iMozi = 0;
	}

	//	페이지 넘김은 어떻게 할지

	return 0;
}
//-------------------------------------------------------------------------------------------------
#endif

#ifdef SEARCH_HIGHLIGHT
/*!
	현재 페이지를 재검색・화면 재그리기 등
	@return	HRESULT	종료 상태 코드
*/
HRESULT FindNowPageReSearch( VOID )
{
	FindHighlightOff(  );

	FindPageSearch( gatLastPtn, gixFocusPage, gitFileIt );	//	무거움

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	지연 로드 페이지를 열 때, 검색 하이라이트를 처리
	@param[in]	iTgtPage	대상 페이지 번호
	@return	HRESULT	종료 상태 코드
*/
HRESULT FindDelayPageReSearch( INT iTgtPage )
{

	//	전체 검색이 아니거나, 검색 문자열이 비어 있으면 무시
	if(  1 != giSetRange || NULL == gatLastPtn[0] ){	return  E_ABORT;	}

	//	일단 페이지 검색
	FindPageSearch( NULL, iTgtPage, gitFileIt );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
#endif

#if 0
/*!
	문자가 삽입 삭제될 때, 그 부분을 체크
	@param[in]	iTgtLine	대상 행 번호
	@return	HRESULT	종료 상태 코드
*/
HRESULT FindTextModifyLine( INT iTgtLine )
{
//해당 행과 개행이 미치는 범위의 행만, 일단 체크 해제하고, 해당 범위의 행만 체크
//복사 붙여넣기된 경우는? 삽입 삭제 처리가 이루어진 후 체크하면, 영향 범위 주의

	//	giCrLfCnt

	//행 수 확인・넘치지 않도록

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
#endif

#endif	//	FIND_STRINGS
