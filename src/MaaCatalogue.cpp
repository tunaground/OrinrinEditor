#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"

#define MLT_SEPARATER	("[SPLIT]")
#define AST_SEPARATER	("[AA]")

static  vector<AAMATRIX>	gvcArts;

static TCHAR	gatOpenFile[MAX_PATH];

static TCHAR	gatBkUpDir[MAX_PATH];

DWORD	AacInflateMlt( LPSTR, DWORD );
DWORD	AacInflateAst( LPSTR, DWORD );

UINT	AacTitleCheck( LPAAMATRIX );

LRESULT	CALLBACK AacFavInflate( UINT, UINT, UINT, LPCVOID );

#ifdef MAA_IADD_PLUS
UINT	AacItemBackUpCreate( LPVOID );
HRESULT	AacItemOutput( HWND );
#endif

VOID AacBackupDirectoryInit( LPTSTR ptCurrent )
{
	StringCchCopy( gatBkUpDir, MAX_PATH, ptCurrent );
	PathAppend( gatBkUpDir, BACKUP_DIR );
	CreateDirectory( gatBkUpDir, NULL );

	return;
}

DWORD AacAssembleFile( HWND hWnd, LPTSTR ptFileName )
{
	HANDLE	hFile;
	DWORD	rdFileSize, readed, rdCount;
	BOOL	isAST;
	LPSTR	pcFullBuff;

	hFile = CreateFile( ptFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return 0;	}

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	rdFileSize = GetFileSize( hFile, NULL );

	StringCchCopy( gatOpenFile , MAX_PATH, ptFileName );

	isAST = PathMatchSpec( ptFileName , TEXT("*.ast") );

	pcFullBuff = (LPSTR)malloc( rdFileSize + 1 );
	if( !(pcFullBuff) ){	return 0;	}

	ZeroMemory( pcFullBuff, rdFileSize + 1 );

	ReadFile( hFile, pcFullBuff, rdFileSize, &readed, NULL );

	CloseHandle( hFile );

	AacMatrixClear(   );

	if( isAST ){	rdCount = AacInflateAst( pcFullBuff, readed );	}
	else{			rdCount = AacInflateMlt( pcFullBuff, readed );	}

#pragma message ("旧末尾追加したやつは末端に0x0Dがくっついてる・飛ばして読み込む処理を")

	free( pcFullBuff );

	return rdCount;
}

DWORD AacInflateAst( LPSTR pcTotal, DWORD cbTotal )
{
	LPSTR	pcCaret;
	LPSTR	pcStart;
	LPSTR	pcEnd;

	UINT	iNumber;

	UINT	cbItem;
	BOOLEAN	bLast;

	AAMATRIX	stAAbuf;

	bLast = FALSE;
	iNumber = 0;

	pcCaret = StrStrA( pcTotal, AST_SEPARATER );
	if( !pcCaret )	return 0;

	do
	{
		ZeroMemory( &stAAbuf, sizeof(AAMATRIX) );

		stAAbuf.ixNum = iNumber;

		pcStart = NextLineA(  pcCaret );

		pcCaret += 5;
		cbItem  = pcStart - pcCaret;
		cbItem -= 3;

		if( 0 < cbItem )
		{
			StringCchCopyNA( stAAbuf.acAstName, MAX_STRING, pcCaret, cbItem );
			AaTitleAddString( iNumber , stAAbuf.acAstName );
		}

		pcCaret = pcStart;

		pcEnd = StrStrA( pcCaret, AST_SEPARATER );

		if( !pcEnd )
		{
			pcEnd = pcTotal + cbTotal;
			bLast = TRUE;
		}
		stAAbuf.cbItem = pcEnd - pcCaret;

		stAAbuf.pcItem = (LPSTR)malloc( stAAbuf.cbItem + 2 );
		ZeroMemory( stAAbuf.pcItem, stAAbuf.cbItem + 2 );

		if( 0 >= stAAbuf.cbItem )
		{
			stAAbuf.cbItem = 2;
			StringCchCopyA( stAAbuf.pcItem, stAAbuf.cbItem, (" ") );
		}
		else
		{
			CopyMemory( stAAbuf.pcItem, pcCaret, stAAbuf.cbItem );
		}

		gvcArts.push_back( stAAbuf );

		iNumber++;

		pcCaret = pcEnd;

	}while( *pcCaret );

	return iNumber;
}

DWORD AacInflateMlt( LPSTR pcTotal, DWORD cbTotal )
{
	LPSTR	pcCaret;
	LPSTR	pcEnd;

	DWORD	iNumber;
	AAMATRIX	stAAbuf;

	pcCaret = pcTotal;

	iNumber = 0;

	do
	{
		ZeroMemory( &stAAbuf, sizeof(AAMATRIX) );

		stAAbuf.ixNum = iNumber;

		pcEnd = StrStrA( pcCaret, MLT_SEPARATER );
		if( !pcEnd )
		{
			pcEnd = pcTotal + cbTotal;
		}
		stAAbuf.cbItem = pcEnd - pcCaret;

		stAAbuf.pcItem = (LPSTR)malloc( stAAbuf.cbItem + 2 );
		ZeroMemory( stAAbuf.pcItem, stAAbuf.cbItem + 2 );

		if( 0 >= stAAbuf.cbItem )
		{
			stAAbuf.cbItem = 2;
			StringCchCopyA( stAAbuf.pcItem, stAAbuf.cbItem, (" ") );
		}
		else
		{
			CopyMemory( stAAbuf.pcItem, pcCaret, stAAbuf.cbItem );
		}

		AacTitleCheck( &stAAbuf );

		gvcArts.push_back( stAAbuf );

		iNumber++;

		pcCaret = NextLineA( pcEnd );

	}while( *pcCaret );

	return iNumber;
}

UINT AacTitleCheck( LPAAMATRIX pstItem )
{
	LPSTR	pcCaret, pcEnd, pcOpen;
	LPSTR	pcLine;
	UINT	cbSize, d;

	if( 4 > pstItem->cbItem )	return 0;

	pcCaret = pstItem->pcItem;

	if( !( strncmp( pcCaret, "【", 2 ) ) )
	{
		pcOpen = pcCaret + 2;

		cbSize  = pstItem->cbItem;
		cbSize -= 2;

		pcEnd = NextLineA( pcCaret );

		if( *pcEnd )
		{
			pcCaret = pcOpen;
			cbSize = pcEnd - pcCaret;
		}

		pcLine = (LPSTR)malloc( cbSize+1 );
		ZeroMemory( pcLine, cbSize+1 );
		CopyMemory( pcLine, pcOpen, cbSize );
		pcCaret = pcLine;
		for( d = 0; cbSize > d; d++ )
		{
			if( !(strncmp( pcCaret, "】", 2 ) ) )
			{
				*pcCaret = NULL;
				break;
			}
			pcCaret++;
		}

		AaTitleAddString( pstItem->ixNum, pcLine );

		FREE(pcLine);
	}
	else
	{

		if( ':' ==  pcCaret[0] )	return 0;
		if( '_' ==  pcCaret[0] )	return 0;
		if( '(' ==  pcCaret[0] )	return 0;
		if( '.' ==  pcCaret[0] )	return 0;
		if( '/' ==  pcCaret[0] )	return 0;

		if( !( strncmp( pcCaret, "（", 2 ) ) )	return 0;
		if( !( strncmp( pcCaret, "　 ", 3 ) ) )	return 0;
		if( !( strncmp( pcCaret, "　　", 4 ) ) )	return 0;
		if( !( strncmp( pcCaret, "最終更", 4 ) ) )	return 0;
		if( !( strncmp( pcCaret, "Last Mod", 8 ) ) )	return 0;

		cbSize = pstItem->cbItem;

		pcEnd = NextLineA( pcCaret );

		if( *pcEnd )
		{
			cbSize = pcEnd - pcCaret;

			pcOpen = pcEnd;
			pcEnd  = NextLineA( pcOpen );

			if( *pcEnd )	return 0;
		}

		if( 42 <= cbSize )	return 0;

		pcLine = (LPSTR)malloc( cbSize );
		ZeroMemory( pcLine, cbSize );
		CopyMemory( pcLine, pcCaret, cbSize-2 );

		AaTitleAddString( pstItem->ixNum, pcLine );

		FREE(pcLine);
	}

	return 1;
}

INT_PTR AacItemCount( UINT reserve )
{
	return gvcArts.size();
}

HBITMAP AacArtImageGet( HWND hWnd, INT iNumber, LPSIZE pstSize, LPSIZE pstArea )
{
	INT_PTR		iItems, i;
	MAAM_ITR	itArts;

	pstSize->cx = 0;
	pstSize->cy = 0;

	pstArea->cx = 0;
	pstArea->cy = 0;

	iItems = gvcArts.size( );
	if( iItems <= iNumber ){	return NULL;	}

	itArts = gvcArts.begin();
	for( i = 0; iNumber >  i; i++ ){	itArts++;	}

	if( itArts->hThumbBmp )
	{
		pstSize->cx = itArts->stSize.cx;
		pstSize->cy = itArts->stSize.cy;

		pstArea->cx = itArts->iMaxDot;
		pstArea->cy = itArts->iLines;

		return	itArts->hThumbBmp;
	}

	DraughtAaImageing( hWnd, &(*itArts) );

	pstSize->cx = itArts->stSize.cx;
	pstSize->cy = itArts->stSize.cy;

	pstArea->cx = itArts->iMaxDot;
	pstArea->cy = itArts->iLines;

	return itArts->hThumbBmp;
}

LPSTR AacAsciiArtGet( DWORD iNumber )
{
	size_t	items;
	LPSTR	pcBuff;

	items = gvcArts.size( );
	if( items <= iNumber )	return NULL;

	pcBuff = (LPSTR)malloc( gvcArts.at( iNumber ).cbItem + 1 );
	ZeroMemory( pcBuff, gvcArts.at( iNumber ).cbItem + 1 );
	CopyMemory( pcBuff, gvcArts.at( iNumber ).pcItem, gvcArts.at( iNumber ).cbItem );

	return pcBuff;
}

INT AacArtSizeGet( DWORD iNumber, PINT piLine, PINT pBytes )
{
	INT	iDot, cx, cy, iByte = 0;
	size_t	items;
	LPSTR	pcConts;
	LPTSTR	ptString;

	items = gvcArts.size( );
	if( items <=  iNumber ){	return 0;	}

	if( 0 >= gvcArts.at( iNumber ).stSize.cx || 0 >= gvcArts.at( iNumber ).stSize.cy )
	{
		pcConts = (LPSTR)malloc( gvcArts.at( iNumber ).cbItem + 1 );
		if( pcConts )
		{
			TRACE( TEXT("AA Size Calculate[%d]"), iNumber );

			ZeroMemory( pcConts, gvcArts.at( iNumber ).cbItem + 1 );
			CopyMemory( pcConts, gvcArts.at( iNumber ).pcItem, gvcArts.at( iNumber ).cbItem );

			ptString = SjisDecodeAlloc( pcConts );
			FREE( pcConts );
			if( ptString )
			{
				cx = TextViewSizeGet( ptString, &cy );
#ifndef _ORRVW
				DocRawDataParamGet( ptString, NULL, &iByte );
#endif
				FREE( ptString );

				gvcArts.at( iNumber ).stSize.cx = cx;
				gvcArts.at( iNumber ).stSize.cy = cy;
				gvcArts.at( iNumber ).iByteSize = iByte;
			}
		}
	}

	iDot    = gvcArts.at( iNumber ).stSize.cx;
	if( piLine ){	*piLine = gvcArts.at( iNumber ).stSize.cy;	}
	if( pBytes ){	*pBytes = gvcArts.at( iNumber ).iByteSize;	}

	return iDot;
}

HRESULT AacMatrixClear( VOID )
{
	MAAM_ITR	itArts;

	for( itArts = gvcArts.begin(); itArts != gvcArts.end(); itArts++ )
	{
		FREE( itArts->pcItem );

		if( itArts->hThumbBmp )
		{

			DeleteBitmap( itArts->hThumbBmp );

		}
	}

	gvcArts.clear();

	return S_OK;
}

DWORD AacAssembleSql( HWND hWnd, LPCTSTR ptBlockName )
{
	DWORD	ixItems;

	ixItems = SqlFavCount( ptBlockName, NULL );

	AacMatrixClear(   );

	if( 0 == ixItems )	return 0;

	SqlFavArtEnum( ptBlockName, AacFavInflate );

	return ixItems;
}

LRESULT CALLBACK AacFavInflate( UINT dLength, UINT dummy, UINT fake, LPCVOID pcConts )
{
	UINT_PTR	iNumber;
	AAMATRIX	stAAbuf;

	iNumber = gvcArts.size( );

	TRACE( TEXT("NUM[%d] byte[%d]"), iNumber, dLength );

	ZeroMemory( &stAAbuf, sizeof(AAMATRIX) );

	stAAbuf.ixNum = iNumber;

	stAAbuf.cbItem = dLength;
	stAAbuf.pcItem = (LPSTR)malloc( stAAbuf.cbItem + 2 );
	ZeroMemory( stAAbuf.pcItem, stAAbuf.cbItem + 2 );

	CopyMemory( stAAbuf.pcItem, pcConts, stAAbuf.cbItem );

	gvcArts.push_back( stAAbuf );

	return 0;
}

#ifdef MAA_IADD_PLUS

#ifndef _ORRVW

typedef struct tagITEMADDINFO
{
	LPTSTR	ptContent;
	TCHAR	atSep[MAX_STRING];
	INT		bType;

} ITEMADDINFO, *LPITEMADDINFO;

INT_PTR CALLBACK AaItemAddDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPITEMADDINFO	pstIaInfo;
	static LPTSTR	ptBuffer;
	UINT_PTR	cchSize;
	TCHAR	atName[MAX_PATH];
	INT		id;
	RECT	rect;

	switch( message )
	{
		case WM_INITDIALOG:
			pstIaInfo = (LPITEMADDINFO)(lParam);
			GetClientRect( hDlg, &rect );
			CreateWindowEx( 0, WC_BUTTON, TEXT("今の頁"),         WS_CHILD | WS_VISIBLE, 0, 0, 75, 23, hDlg, (HMENU)IDB_MAID_NOWPAGE, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("クリップボード"), WS_CHILD | WS_VISIBLE, 75, 0, 120, 23, hDlg, (HMENU)IDB_MAID_CLIPBOARD, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 195, 0, rect.right-195-50, 23, hDlg, (HMENU)IDE_MAID_ITEMNAME, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("追加"),           WS_CHILD | WS_VISIBLE, rect.right-50, 0, 50, 23, hDlg, (HMENU)IDB_MAID_ADDGO, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY, 0, 23, rect.right, rect.bottom-23, hDlg, (HMENU)IDE_MAID_CONTENTS, GetModuleHandle(NULL), NULL );

			if( pstIaInfo->bType )
			{
				SetDlgItemText( hDlg, IDE_MAID_ITEMNAME, TEXT("名称はASTでないと使用できないよ") );
				EnableWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), FALSE );
				StringCchCopy( pstIaInfo->atSep, MAX_STRING, TEXT("[SPLIT]\r\n") );
			}

			ptBuffer = DocClipboardDataGet( NULL );
			if( !(ptBuffer) ){	DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );	}

			SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDCANCEL:
					FREE(ptBuffer);
					EndDialog(hDlg, 0 );
					return (INT_PTR)TRUE;

				case IDB_MAID_ADDGO:
					if( ptBuffer )
					{
						StringCchLength( ptBuffer, STRSAFE_MAX_CCH, &cchSize );
						cchSize += 4;
						pstIaInfo->ptContent = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
						StringCchCopy( pstIaInfo->ptContent, cchSize, ptBuffer );

						StringCchCat( pstIaInfo->ptContent, cchSize, CH_CRLFW );

						if( !(pstIaInfo->bType) )
						{
							GetDlgItemText( hDlg, IDE_MAID_ITEMNAME, atName, MAX_PATH );

							StringCchCopy( pstIaInfo->atSep, MAX_STRING, atName );
						}
					}
					FREE(ptBuffer);
					EndDialog(hDlg, 1 );
					return (INT_PTR)TRUE;

				case IDB_MAID_CLIPBOARD:
					FREE(ptBuffer);
					ptBuffer = DocClipboardDataGet( NULL );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				case IDB_MAID_NOWPAGE:
					FREE(ptBuffer);
					DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		case WM_SIZE:
			GetClientRect( hDlg, &rect );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), 195, 0, rect.right-195-50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDB_MAID_ADDGO),    rect.right-50, 0, 50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_CONTENTS), 0, 23, rect.right, rect.bottom-23, TRUE );
			break;
	}

	return (INT_PTR)FALSE;
}

HRESULT AacItemDelete( HWND hWnd, LONG iNumber )
{
	UINT		curSel;
	MAAM_ITR	itMaaItem;

	if( 0 > iNumber )	return E_OUTOFMEMORY;

	itMaaItem = gvcArts.begin();
	std::advance( itMaaItem , iNumber );

	gvcArts.erase( itMaaItem );

	AacItemBackUpCreate( NULL );

	AacItemOutput( hWnd );

	curSel = TabMultipleNowSel(  );
	AaItemsDoShow( hWnd, gatOpenFile, curSel );

	return S_OK;
}

HRESULT AacItemInsert( HWND hWnd, LONG iNumber )
{
	UINT_PTR	cbSize;
	LPSTR		pcName;
	INT			curSel;
	AAMATRIX	stAAbuf;
	ITEMADDINFO	stIaInfo;

	ZeroMemory( &stAAbuf, sizeof(AAMATRIX) );
	ZeroMemory( &stIaInfo, sizeof(ITEMADDINFO) );

	if( FileExtensionCheck( gatOpenFile, TEXT(".ast") ) ){	stIaInfo.bType =  0;	}
	else{	stIaInfo.bType =  1;	}

	if( DialogBoxParam( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAA_IADD_DLG), hWnd, AaItemAddDlgProc, (LPARAM)(&stIaInfo) ) )
	{
		if( stIaInfo.ptContent )
		{

			pcName = SjisEncodeAlloc( stIaInfo.atSep  );
			StringCchLengthA( pcName, STRSAFE_MAX_CCH, &cbSize );
			StringCchCopyA( stAAbuf.acAstName, MAX_STRING, pcName );
			FREE( pcName );
			stAAbuf.cbItem = cbSize;

			stAAbuf.pcItem = SjisEncodeAlloc( stIaInfo.ptContent  );

			if( 0 <= iNumber )	gvcArts.insert( gvcArts.begin() + iNumber, stAAbuf );
			else				gvcArts.push_back( stAAbuf );

			FREE( stIaInfo.ptContent );

			AacItemBackUpCreate( NULL );

			AacItemOutput( hWnd );

			curSel = TabMultipleNowSel(  );
			AaItemsDoShow( hWnd, gatOpenFile, curSel );
		}
	}

	return S_OK;
}

UINT AacItemBackUpCreate( LPVOID pVoid )
{
	TCHAR	atOutFile[MAX_PATH], atFileName[MAX_PATH];

	StringCchCopy( atFileName, MAX_PATH, PathFindFileName( gatOpenFile ) );

	StringCchCat( atFileName, MAX_PATH, TEXT(".abk") );

	StringCchCopy( atOutFile, MAX_PATH, gatBkUpDir );
	PathAppend( atOutFile, atFileName );

	CopyFile( gatOpenFile, atOutFile, FALSE );

	return 1;
}

HRESULT AacItemOutput( HWND hWnd )
{
	BOOLEAN	isAST;
	CHAR	acSep[MAX_STRING];

	HANDLE	hFile;
	DWORD	wrote;
	UINT_PTR	cbSize;

	INT_PTR	i, iPage;

	MAAM_ITR	itAamx;

	if( FileExtensionCheck( gatOpenFile, TEXT(".ast") ) ){	isAST = TRUE;	}
	else{	isAST = FALSE;	}

	hFile = CreateFile( gatOpenFile, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		MessageBox( hWnd, TEXT("ファイルを開けなかったよ"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
		return E_HANDLE;
	}

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	iPage = gvcArts.size();

	for( i = 0, itAamx = gvcArts.begin(); gvcArts.end() != itAamx; itAamx++, i++ )
	{

		ZeroMemory( acSep, sizeof(acSep) );	cbSize = 0;
		if( isAST )
		{
			StringCchPrintfA( acSep , MAX_STRING, ("[AA][%s]%s"), itAamx->acAstName, CH_CRLFA );
		}
		else
		{
			if( 1 <= i )
			{
				StringCchPrintfA( acSep, MAX_STRING, ("%s%s"), MLT_SEPARATERA, CH_CRLFA );
			}
		}
		StringCchLengthA( acSep , MAX_STRING, &cbSize );
		if( cbSize ){	WriteFile( hFile, acSep, cbSize, &wrote, NULL );	}

		StringCchLengthA( itAamx->pcItem, STRSAFE_MAX_CCH, &cbSize );
		WriteFile( hFile, itAamx->pcItem, cbSize, &wrote, NULL );
	}

	SetEndOfFile( hFile );
	CloseHandle( hFile );

	return S_OK;
}

#endif

#endif
