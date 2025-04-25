#include "stdafx.h"
#ifdef _ORCOLL
#include "OrinrinCollector.h"
#else
#include "OrinrinEditor.h"
#endif
#include "Entity.h"

#ifndef _ORRVW
extern  UINT	gbUniRadixHex;
#endif

typedef struct tagFINDPATTERN
{
	TCHAR	cchMozi;
	INT		iDistance;

} FINDPATTERN, *LPFINDPATTERN;

typedef struct tagMSGBOXMSG
{
	TCHAR	atMsg1[MAX_PATH];
	TCHAR	atMsg2[MAX_PATH];

	UINT	bChecked;

} MSGBOXMSG, *LPMSGBOXMSG;

#ifdef SPMOZI_ENCODE

EXTERNED UINT	gbSpMoziEnc;

static CONST TCHAR	gatSpMoziList[] = {
	TEXT("①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅰⅱⅲⅳⅴⅵⅶⅷⅸⅹ㍉㌔㌢㍍㌘㌧㌃㌶㍑㍗")
	TEXT("㌍㌦㌣㌫㍊㌻㎜㎝㎞㎎㎏㏄㎡㍻〝〟№㏍℡㊤㊥㊦㊧㊨㈱㈲㈹㍾㍽㍼≒≡∫∮∑√⊥∠∟⊿∵∩∪¬¦＇＂丨纊褜")
	TEXT("鍈銈蓜俉炻昱棈鋹曻彅仡仼伀伃伹佖侒侊侚侔俍偀倢俿倞偆偰偂傔僴僘兊兤冝冾凬刕劜劦勀勛匀匇匤卲厓厲叝﨎咜")
	TEXT("咊咩哿喆坙坥垬埈埇﨏塚增墲夋奓奛奝奣妤妺孖寀甯寘寬尞岦岺峵崧嵓﨑嵂嵭嶸嶹巐弡弴彧德忞恝悅悊惞惕愠惲愑")
	TEXT("愷愰憘戓抦揵摠撝擎敎昀昕昻昉昮昞昤晥晗晙晴晳暙暠暲暿曺朎朗杦枻桒柀栁桄棏﨓楨﨔榘槢樰橫橆橳橾櫢櫤毖氿")
	TEXT("汜沆汯泚洄涇浯涖涬淏淸淲淼渹湜渧渼溿澈澵濵瀅瀇瀨炅炫焏焄煜煆煇凞燁燾犱犾猤猪獷玽珉珖珣珒琇珵琦琪琩琮")
	TEXT("瑢璉璟甁畯皂皜皞皛皦益睆劯砡硎硤硺礰礼神祥禔福禛竑竧靖竫箞精絈絜綷綠緖繒罇羡羽茁荢荿菇菶葈蒴蕓蕙蕫﨟")
	TEXT("薰蘒﨡蠇裵訒訷詹誧誾諟諸諶譓譿賰賴贒赶﨣軏﨤逸遧郞都鄕鄧釚釗釞釭釮釤釥鈆鈐鈊鈺鉀鈼鉎鉙鉑鈹鉧銧鉷鉸鋧")
	TEXT("鋗鋙鋐﨧鋕鋠鋓錥錡鋻﨨錞鋿錝錂鍰鍗鎤鏆鏞鏸鐱鑅鑈閒隆﨩隝隯霳霻靃靍靏靑靕顗顥飯飼餧館馞驎髙髜魵魲鮏鮱")
	TEXT("鮻鰀鵰鵫鶴鸙黑") };

#define SPMOZI_CNT	457
#endif

#ifdef SPMOZI_ENCODE

UINT IsSpMozi( TCHAR tMozi )
{
	UINT	i;

	if( !(gbSpMoziEnc) )	return 0;

	for( i = 0; SPMOZI_CNT > i; i++ )
	{
		if( gatSpMoziList[i] == tMozi )	return 1;
	}

	return 0;
}

#endif

VOID WndTagSet( HWND hWnd, LONG_PTR tag )
{
	SetWindowLongPtr( hWnd, GWLP_USERDATA, tag );
}

LONG_PTR WndTagGet( HWND hWnd )
{
	return GetWindowLongPtr( hWnd, GWLP_USERDATA );
}

BOOLEAN FileExtensionCheck( LPTSTR ptFile, LPTSTR ptExte )
{
	TCHAR	atExBuf[10];
	LPTSTR	ptExten;

	ptExten = PathFindExtension( ptFile );
	if( 0 == *ptExten ){	return 0;	}

	StringCchCopy( atExBuf, 10, ptExten );
	CharLower( atExBuf );

	if( StrCmp( atExBuf, ptExte ) ){	return 0;	}

	return 1;
}

BOOLEAN HtmlEntityCheckA( TCHAR adMozi, LPSTR pcStr, UINT_PTR cbSize )
{
	INT	i;

	ZeroMemory( pcStr, cbSize );

	for( i = 0; 4 > i; i++ )
	{
		if( 0 == gstEttySP[i].dUniCode )	break;

		if( gstEttySP[i].dUniCode == adMozi )
		{
			StringCchCopyA( pcStr, cbSize, gstEttySP[i].acCodeA );
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN HtmlEntityCheckW( TCHAR adMozi, LPTSTR ptStr, UINT_PTR cchSize )
{
	INT	i;

	ZeroMemory( ptStr, cchSize * sizeof(TCHAR) );

	for( i = 0; 4 > i; i++ )
	{
		if( 0 == gstEttySP[i].dUniCode )	break;

		if( gstEttySP[i].dUniCode == adMozi )
		{
			StringCchCopy( ptStr, cchSize, gstEttySP[i].atCodeW );
			return TRUE;
		}
	}

	return FALSE;
}

TCHAR UniRefCheck( LPSTR pcStr )
{
	CHAR	acValue[10];
	PCHAR	pcEnd;
	UINT	i, code;
	INT		radix = 10;
	BOOLEAN	bXcode = FALSE;

	ZeroMemory( acValue, sizeof(acValue) );
	if( NULL == pcStr[2] )	return 0x0000;

	pcStr += 2;

	if( 'x' == pcStr[0] || 'X' == pcStr[0] )
	{
		bXcode = TRUE;
		pcStr++;
		radix = 16;
	}

	for( i = 0; 10 > i; i++ )
	{
		if( ';' == pcStr[i] )	break;

		if( NULL == pcStr[i] )	return 0x0000;

		if( 0 > (INT)(pcStr[i]) )	return 0x0000;

		if( bXcode )
		{
			if( isxdigit( pcStr[i] ) ){	acValue[i] = pcStr[i];	}
			else	return 0x0000;
		}
		else
		{
			if( isdigit( pcStr[i] ) ){	acValue[i] = pcStr[i];	}
			else	return 0x0000;
		}
	}
	if( 10 <= i ){	return 0x0000;	}

	code = strtoul( acValue, &pcEnd, radix );

	if( 0xFFFF < code ){	code = 0x0000;	}

	return (TCHAR)code;
}

LPSTR SjisEntityExchange( LPCSTR pcMoto )
{
	LPSTR		pcOutput, pcTemp;
	CHAR		acSrp[12], acChk[3], acUni[10];
	UINT		check, el;
	UINT_PTR	szMoto, szStr, sc, dt, se, rp, cbSz;
	BOOLEAN		bStr = FALSE, bHit = FALSE;

	CONST ENTITYLIST	*pstEty;

	szMoto = strlen( pcMoto );
	szStr = szMoto + 2;
	pcOutput = (LPSTR)malloc( szStr );
	if( !(pcOutput) )	return NULL;
	ZeroMemory( pcOutput, szStr );

	for( sc = 0, dt = 0; szMoto > sc; sc++ )
	{
		if( '&' == pcMoto[sc] )
		{
			se = sc;
			bStr = FALSE;

			for( rp = 0; 10 > rp; rp++ )
			{
				acSrp[rp] = pcMoto[se++];	acSrp[rp+1] = 0x00;
				if( ';'  == acSrp[rp] ){	bStr = TRUE;	break;	}

				if( 1 == rp )
				{
					if( 0 > acSrp[1] )	break;
					if( !( isalpha( acSrp[1] ) ) )	break;
				}
			}

			if( !(bStr) ){	pcOutput[dt++] = pcMoto[sc];	continue;	}

			acChk[0] = acSrp[1];	acChk[1] = 0x00;
			CharLowerA( acChk );
			check = acChk[0] - 'a';

			if( 26 <= check ){	pcOutput[dt++] = pcMoto[sc];	continue;	}

			pstEty = gpstEntitys[check];

			bHit = FALSE;

			for( el = 0; 0 != pstEty[el].dUniCode; el++ )
			{
				if( 0 == StrCmpA( acSrp, pstEty[el].acCodeA ) )
				{
					szStr += 4;
					pcTemp = (LPSTR)realloc( pcOutput, szStr );
					if( pcTemp )	pcOutput = pcTemp;

					bHit = TRUE;
					StringCchPrintfA( acUni, 10, ("&#%d;"), pstEty[el].dUniCode );
					StringCchLengthA( acUni, 10, &cbSz );
					for( rp = 0; cbSz > rp; rp++ )
					{
						pcOutput[dt++] = acUni[rp];
					}
					sc = se - 1;
					break;
				}
			}

			if( !(bHit) ){	pcOutput[dt++] = pcMoto[sc];	}
		}
		else
		{
			pcOutput[dt++] = pcMoto[sc];
		}
	}

	return pcOutput;
}

LPTSTR SjisDecodeAlloc( LPSTR pcBuff )
{
	DWORD	cbWrtSize;
	LPSTR	pcPos, pcChk, pcPosEx;

	DWORD	cchSize, cchWrtSize;
	LPTSTR	ptBuffer, ptWrtpo;
	TCHAR	chMozi;

	DWORD	dStart, dEnd;

	if( !(pcBuff) ){	return NULL;	}

	cchSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcBuff, -1, NULL, 0 );

	cchSize += 2;
	ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	ZeroMemory( ptBuffer, cchSize * sizeof(TCHAR) );

	dStart = 0;
	dEnd   = 0;

	ptWrtpo = ptBuffer;

	pcPosEx = SjisEntityExchange( pcBuff );

	pcPos = pcPosEx;

	pcChk = StrStrA( pcPosEx, "&#" );
	while( pcChk )
	{

		cbWrtSize  = pcChk - pcPos;
		cchWrtSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcPos, cbWrtSize, ptWrtpo, cchSize );

		ptWrtpo += cchWrtSize;
		cchSize -= cchWrtSize;

		chMozi = UniRefCheck( pcChk );
		if( 0 != chMozi )
		{
			*ptWrtpo = chMozi;
			ptWrtpo++;
			cchSize--;
			pcChk = StrStrA( pcChk, ";" );
			pcChk++;
		}
		else
		{
			cchWrtSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcChk, 2, ptWrtpo, cchSize );

			ptWrtpo += cchWrtSize;
			cchSize -= cchWrtSize;
			pcChk += 2;
		}
		pcPos = pcChk;

		pcChk = StrStrA( pcPos , "&#" );
	}

	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcPos, -1, ptWrtpo, cchSize );

	FREE( pcPosEx );

	return ptBuffer;
}

#ifndef _ORCOLL

INT TextViewSizeGet( LPCTSTR ptText, PINT piLine )
{
	UINT_PTR	cchSize, i;
	INT		xDot, yLine, dMaxDot;

	wstring	wString;

	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

	yLine = 1;	dMaxDot = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )
		{

			xDot = ViewStringWidthGet( wString.c_str() );
			if( dMaxDot < xDot )	dMaxDot = xDot;

			wString.clear( );
			i++;
			yLine++;
		}
		else if( CC_TAB == ptText[i] )
		{

		}
		else
		{
			wString += ptText[i];
		}
	}

	if( 1 <= wString.size() )
	{

		xDot = ViewStringWidthGet( wString.c_str() );
		if( dMaxDot < xDot )	dMaxDot = xDot;
	}

	if( piLine )	*piLine = yLine;
	return dMaxDot;

}
#endif

LPCTSTR NextLineW( LPCTSTR pt )
{
	while( *pt && *pt != 0x000D ){	pt++;	}

	if( 0x000D == *pt )
	{
		pt++;
		if( 0x000A == *pt ){	pt++;	}
	}

	return pt;
}

LPTSTR NextLineW( LPTSTR pt )
{
	while( *pt && *pt != 0x000D ){	pt++;	}

	if( 0x000D == *pt )
	{
		pt++;
		if( 0x000A == *pt ){	pt++;	}
	}

	return pt;
}

LPSTR NextLineA( LPSTR pt )
{
	while( *pt && *pt != 0x0D ){	pt++;	}

	if( 0x0D == *pt )
	{
		pt++;
		if( 0x0A == *pt ){	pt++;	}
	}

	return pt;
}

#ifndef _ORRVW

LPSTR SjisEncodeAlloc( LPCTSTR ptTexts )
{
	TCHAR	atMozi[2];
	CHAR	acSjis[10];
	BOOL	bCant = FALSE;
	INT		iRslt;
	UINT_PTR	cchSize, d, cbSize;
	LPSTR	pcString;

	string	sString;

	if( !(ptTexts) ){	return NULL;	}

	StringCchLength( ptTexts, STRSAFE_MAX_CCH, &cchSize );

	sString.clear( );

	atMozi[1] = 0;
	for( d = 0; cchSize > d; d++ )
	{
		atMozi[0] = ptTexts[d];
		ZeroMemory( acSjis, sizeof(acSjis) );

		iRslt = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atMozi, 1, acSjis, 10, "?", &bCant );

		if( bCant )
		{
			if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10 , ("&#x%X;"), ptTexts[d] );	}
			else{					StringCchPrintfA( acSjis, 10 , ("&#%d;"),  ptTexts[d] );	}
		}

#ifdef SPMOZI_ENCODE
		if( IsSpMozi( ptTexts[d] ) )
		{
			if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10 , ("&#x%X;"), ptTexts[d] );	}
			else{					StringCchPrintfA( acSjis, 10 , ("&#%d;"),  ptTexts[d] );	}
		}
#endif

		sString += string( acSjis );
	}

	cbSize = sString.size( ) + 2;
	pcString = (LPSTR)malloc( cbSize );
	ZeroMemory( pcString, cbSize );
	StringCchCopyA( pcString, cbSize, sString.c_str() );

	return pcString;
}

#endif

LPFINDPATTERN FindTableMake( LPCTSTR ptPattern )
{
	UINT		i;
	UINT_PTR	dLength;
	LPFINDPATTERN	pstPtrn;

	StringCchLength( ptPattern, STRSAFE_MAX_CCH, &dLength );
	pstPtrn = (LPFINDPATTERN)malloc( (dLength+1) * sizeof(FINDPATTERN) );
	ZeroMemory( pstPtrn, (dLength+1) * sizeof(FINDPATTERN) );

	for( i = 0; dLength >= i; i++ ){	pstPtrn[i].iDistance =  dLength;	}

	while( dLength > 0 )
	{
		i = 0;
		while( pstPtrn[i].cchMozi )
		{
			if( pstPtrn[i].cchMozi ==  *ptPattern ){	break;	}
			i++;
		}
		pstPtrn[i].cchMozi   = *ptPattern;
		pstPtrn[i].iDistance = --dLength;

		ptPattern++;
	}

	return pstPtrn;
}

LPTSTR FindStringProc( LPTSTR ptText, LPTSTR ptPattern, LPINT pdCch )
{
	UINT_PTR	dPtrnLen, dLength;
	LPTSTR	ptTextEnd;
	INT		i, j, k, jump, cch;

	LPFINDPATTERN	pstPattern;

	StringCchLength( ptText, STRSAFE_MAX_CCH, &dLength );

	StringCchLength( ptPattern, STRSAFE_MAX_CCH, &dPtrnLen );
	dPtrnLen--;

	ptTextEnd = ptText + dLength - dPtrnLen;

	pstPattern = FindTableMake( ptPattern );

	cch = 0;
	while( ptText < ptTextEnd )
	{
		for( i = dPtrnLen ; i >= 0 ; i-- )
		{
			if( ptText[i] != ptPattern[i] ){	break;	}
		}

		if( i < 0 ){	FREE( pstPattern  );	*pdCch = cch;	return( ptText );	}

		k = 0;
		while( pstPattern[k].cchMozi )
		{
			if( pstPattern[k].cchMozi == ptText[i] ){	break;	}
			k++;
		}
		j = pstPattern[k].iDistance - ( dPtrnLen - i );
		jump = ( 0 < j ) ? j : 2;
		ptText += jump;
		cch += jump;
	}

	FREE( pstPattern );

	*pdCch = 0;

	return( NULL );
}

INT_PTR CALLBACK MsgCheckBoxProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPMSGBOXMSG	pcstMsg = NULL;
	INT	id;

	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:
			pcstMsg = (LPMSGBOXMSG)lParam;
			SetDlgItemText( hDlg, IDS_MC_MSG1, pcstMsg->atMsg1 );
			SetDlgItemText( hDlg, IDS_MC_MSG2, pcstMsg->atMsg2 );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			if( pcstMsg ){	pcstMsg->bChecked = IsDlgButtonChecked( hDlg, IDCB_MC_CHECKBOX );	}
			if( IDYES == id || IDOK == id ){	EndDialog( hDlg, IDYES );	}
			if( IDNO == id || IDCANCEL == id ){	EndDialog( hDlg, IDNO );	}
			return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR MessageBoxCheckBox( HWND hWnd, HINSTANCE hInst, UINT dStyle )
{
	INT_PTR	iRslt;
	UINT	number;
	MSGBOXMSG	stMsg;

	switch( dStyle )
	{
		case  0:
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("統合しちゃったら復帰できないよ") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にくっつけていい？") );
			number = VL_PCOMBINE_NM;
			break;

		case  1:
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("分割しちゃったら復帰できないよ") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にバラしていい？") );
			number = VL_PDIVIDE_NM;
			break;

		case  2:
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("削除しちゃったら復帰できないよ") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にあぼーんしていい？") );
			number = VL_PDELETE_NM;
			break;

		default:	return IDCANCEL;
	}

	stMsg.bChecked = InitParamValue( INIT_LOAD, number, 0 );
	if( 1 == stMsg.bChecked ){	iRslt = IDYES;	}
	else
	{
		iRslt = DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_MSGCHECKBOX_DLG), hWnd, MsgCheckBoxProc, (LPARAM)(&stMsg) );
		InitParamValue( INIT_SAVE, number, (BST_CHECKED==stMsg.bChecked) ? 1 : 0 );
	}
	return iRslt;
}

#ifdef DO_TRY_CATCH

LRESULT ExceptionMessage( LPCSTR pcExpMsg, LPCSTR pcFuncName, UINT dLine, LPARAM lReturn )
{
	CHAR	acMessage[BIG_STRING];

	StringCchPrintfA( acMessage, BIG_STRING, ("異常発生＜%s＞[%s:%u]\r\nプログラムを続行できません。"), pcExpMsg, pcFuncName, dLine );

	MessageBoxA( GetDesktopWindow(), acMessage, ("致命的エラー発生"), MB_OK );

	return lReturn;
}

#endif
