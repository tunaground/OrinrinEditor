#include "stdafx.h"

#include "MaaTemplate.h"

#define TREE_TBL_VER	1500
#define SUBTAB_NAME_VER	2000

CONST CHAR	cacDetectTable[] = { ("SELECT name FROM sqlite_master WHERE type='table'") };

CONST CHAR	cacVersionTable[]  = { ("CREATE TABLE BuildVer ( id INTEGER PRIMARY KEY, number INTEGER NOT NULL, vertext TEXT NOT NULL)") };
CONST CHAR	cacVerStrInsFmt[]  = { ("INSERT INTO BuildVer ( number, vertext ) VALUES ( %d, '%s' )") };
CONST CHAR	cacVersionNumGet[] = { ("SELECT number FROM BuildVer WHERE id == 1") };
CONST CHAR	cacVersionUpdate[] = { ("UPDATE BuildVer SET number = %d, vertext = '%s' WHERE id == 1") };

static sqlite3	*gpDataBase;

static sqlite3	*gpTreeCache;

HRESULT	SqlFavUpdate( UINT );
HRESULT	SqlFavInsert( LPTSTR, DWORD, LPSTR, UINT );

VOID SqlErrMsgView( sqlite3 *psqTarget, DWORD dline )
{

	TRACE( TEXT("%s[%u]"), sqlite3_errmsg16( psqTarget ), dline );
	return;
}

HRESULT String_Cch_Copy( LPTSTR ptDest, size_t cchSize, LPCTSTR ptSrc )
{
	ZeroMemory( ptDest, cchSize * sizeof(TCHAR) );

	if( !(ptSrc) )	return S_FALSE;

	return StringCchCopy( ptDest, cchSize , ptSrc );
}

HRESULT SqlDatabaseOpenClose( BYTE mode, LPCTSTR ptDbase )
{
	INT		rslt;
	TCHAR	atDbPath[MAX_PATH];

	if( M_CREATE == mode )
	{
		StringCchPrintf( atDbPath, MAX_PATH, ptDbase );

		rslt = sqlite3_open16( atDbPath, &gpDataBase );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_FAIL;	}

		if( 0 == StrCmp( atDbPath, TEXT(":memory:") ) )
		{
			StringCchCopy( atDbPath, MAX_PATH, TEXT("memory.qor") );
		}

		PathStripPath( atDbPath );
		PathRemoveExtension( atDbPath );

		SqlTreeTableCreate( atDbPath );
	}
	else if( M_DESTROY == mode )
	{
		if( gpDataBase )
		{
			rslt =  sqlite3_close( gpDataBase );
			gpDataBase = NULL;
		}
	}
	else
	{
		if( !(gpDataBase) ){	return E_OUTOFMEMORY;	}
	}

	return S_OK;
}

UINT SqlFavCount( LPCTSTR ptFdrName, PUINT pdMax )
{
	TCHAR	atQuery[MAX_STRING];
	UINT	rslt, iCount, iMax;
	sqlite3_stmt*	statement;

	if( ptFdrName )
	{
		StringCchPrintf( atQuery, MAX_STRING, TEXT("SELECT COUNT(id) FROM ArtList WHERE folder == '%s'"), ptFdrName );
	}
	else
	{
		StringCchCopy( atQuery, MAX_STRING, TEXT("SELECT COUNT(id) FROM ArtList") );
	}

	rslt = sqlite3_prepare16( gpDataBase, atQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_step( statement );
	iCount = sqlite3_column_int( statement, 0 );
	sqlite3_finalize(statement);

	if( !pdMax )	return iCount;

	if( !ptFdrName )
	{
		StringCchPrintf( atQuery, MAX_STRING, TEXT("SELECT MAX(id) FROM ArtList WHERE folder == '%s'"), ptFdrName );
	}
	else
	{
		StringCchCopy( atQuery, MAX_STRING, TEXT("SELECT MAX(id) FROM ArtList") );
	}

	rslt = sqlite3_prepare16( gpDataBase, atQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_step( statement );
	iMax = sqlite3_column_int( statement, 0 );
	sqlite3_finalize(statement);

	*pdMax = iMax;

	return iCount;
}

HRESULT SqlFavUpload( LPTSTR ptBaseName, DWORD dHash, LPSTR pcConts, UINT rdLength )
{
	CONST CHAR	acArtSelect[] = { ("SELECT id, folder FROM ArtList WHERE hash == ?") };
	INT		rslt;
	UINT	index, cntID, d;

	TCHAR	atFolder[MAX_PATH];
	BOOLEAN	bIsExist;
	sqlite3_stmt	*statement;

	rslt = sqlite3_prepare( gpDataBase, acArtSelect, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}

	bIsExist = FALSE;
	cntID = SqlFavCount( NULL, NULL );

	sqlite3_reset( statement );
	sqlite3_bind_int( statement, 1, dHash );

	for( d = 0; cntID >  d; d++ )
	{
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt )
		{
			ZeroMemory( atFolder, sizeof(atFolder) );

			index = sqlite3_column_int( statement , 0 );
			String_Cch_Copy( atFolder, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 1 ) );

			SqlFavUpdate( index );

			if( !( StrCmp( ptBaseName, atFolder ) ) ){	bIsExist = TRUE;	}
		}
		else{	break;	}
	}

	sqlite3_finalize( statement );

	if( !(bIsExist) )
	{
		SqlFavInsert( ptBaseName, dHash, pcConts, rdLength );
	}

	return S_OK;
}

HRESULT SqlFavUpdate( UINT index )
{
	CONST CHAR	acArtSelCount[] = { ("SELECT count FROM ArtList WHERE id == %u") };
	CONST CHAR	acArtUpdate[] = { ("UPDATE ArtList SET count = %u, lastuse = %f WHERE id == %u") };

	CHAR	acQuery[MAX_STRING];
	UINT	iCount, rslt;
	DOUBLE	ddJulius;
	sqlite3_stmt	*statement;

	StringCchPrintfA( acQuery, MAX_STRING, acArtSelCount, index );

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	rslt = sqlite3_step( statement );
	iCount = sqlite3_column_int( statement , 0 );
	sqlite3_finalize( statement );

	iCount++;

	rslt = sqlite3_prepare( gpDataBase, ("SELECT julianday('now')"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_step( statement );
	ddJulius = sqlite3_column_double( statement, 0 );
	sqlite3_finalize( statement );

	StringCchPrintfA( acQuery, MAX_STRING, acArtUpdate, iCount, ddJulius, index );
	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize(statement);

	return S_OK;
}

HRESULT SqlFavInsert( LPTSTR ptBaseName, DWORD dHash, LPSTR pcConts, UINT rdLength )
{
	CONST CHAR	acArtInsert[] = { ("INSERT INTO ArtList ( count, folder, lastuse, hash, conts ) VALUES ( ?, ?, ?, ?, ? )") };

	INT		rslt;
	CHAR	acText[MAX_PATH];
	DOUBLE	ddJulius;
	sqlite3_stmt	*statement;

	ZeroMemory( acText, sizeof(acText) );

	rslt = sqlite3_prepare( gpDataBase, ("SELECT julianday('now')"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_step( statement );
	ddJulius = sqlite3_column_double( statement, 0 );
	sqlite3_finalize( statement );

	rslt = sqlite3_prepare( gpDataBase, acArtInsert, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_reset( statement );
	rslt = sqlite3_bind_int(    statement, 1, 1 );
	rslt = sqlite3_bind_text16( statement, 2, ptBaseName, -1, SQLITE_STATIC );
	rslt = sqlite3_bind_double( statement, 3, ddJulius );
	rslt = sqlite3_bind_int(    statement, 4, dHash );
	rslt = sqlite3_bind_blob(   statement, 5, pcConts, rdLength, SQLITE_STATIC );

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}
	sqlite3_finalize(statement);

	return S_OK;
}

HRESULT SqlFavFolderEnum( BUFFERBACK pfFolderNameSet )
{
	CONST CHAR	acQuery[] = { ("SELECT DISTINCT folder FROM ArtList") };

	INT		rslt, count, i;
	sqlite3_stmt	*statement;

	if( !(pfFolderNameSet) ){	return E_INVALIDARG;	}
	if( !(gpDataBase) ){	return E_OUTOFMEMORY;	}

	count = SqlFavCount( NULL, NULL );

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_FAIL;	}
	rslt = sqlite3_reset( statement );

	for( i = 0; count > i; i++ )
	{
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt )
		{
			pfFolderNameSet( i, 0, 0, sqlite3_column_text16( statement, 0 ) );
		}
		else{	break;	}
	}

	sqlite3_finalize(statement);

	return S_OK;
}

HRESULT SqlFavArtEnum( LPCTSTR ptFdrName, BUFFERBACK pfInflate )
{
	TCHAR	atQuery[MAX_STRING];
	UINT	rslt, iCount, i, szCont;
	sqlite3_stmt*	statement;

	if( ptFdrName )
	{
		StringCchPrintf( atQuery, MAX_STRING, TEXT("SELECT conts FROM ArtList WHERE folder == '%s' ORDER BY lastuse DESC"), ptFdrName );
	}
	else
	{
		StringCchCopy( atQuery, MAX_STRING, TEXT("SELECT conts FROM ArtList ORDER BY lastuse") );
	}

	rslt = sqlite3_prepare16( gpDataBase, atQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_FAIL;	}
	rslt = sqlite3_reset( statement );

	iCount = SqlFavCount( ptFdrName, NULL );

	for( i = 0; iCount > i; i++ )
	{
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt )
		{
			szCont = sqlite3_column_bytes( statement, 0 );
			pfInflate( szCont, 0, 0, sqlite3_column_blob( statement, 0 ) );
		}
		else{	break;	}
	}

	sqlite3_finalize(statement);

	return S_OK;
}

HRESULT SqlFavDelete( LPTSTR ptBaseName, DWORD dHash )
{
	CONST CHAR	acArtDelete[] = { ("DELETE FROM ArtList WHERE hash == ?") };
	INT		rslt;
	sqlite3_stmt	*statement;

	rslt = sqlite3_prepare( gpDataBase, acArtDelete, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_reset( statement );
	sqlite3_bind_int( statement, 1, dHash );
	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}

HRESULT SqlFavFolderDelete( LPTSTR ptBaseName )
{
	CONST CHAR	acFolderDelete[] = { ("DELETE FROM ArtList WHERE folder == ?") };
	INT		rslt;
	sqlite3_stmt	*statement;

	rslt = sqlite3_prepare( gpDataBase, acFolderDelete, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_reset( statement );
	sqlite3_bind_text16( statement, 1, ptBaseName, -1, SQLITE_STATIC );
	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}

#if 0

HRESULT SqlTreeOpenClose( BYTE mode, LPCTSTR ptDbName )
{
	INT		rslt;
	TCHAR	atDbPath[MAX_PATH];

	if( M_CREATE == mode )
	{
		StringCchPrintf( atDbPath, MAX_PATH, ptDbName );

		rslt = sqlite3_open16( atDbPath, &gpTreeCache );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return E_FAIL;	}

		if( 0 == StrCmp( atDbPath, TEXT(":memory:") ) )
		{
			StringCchCopy( atDbPath, MAX_PATH, TEXT("memory.qor") );
		}

		PathStripPath( atDbPath );
		PathRemoveExtension( atDbPath );

		SqlTreeTableCreate( atDbPath );
	}
	else if( M_DESTROY == mode )
	{
		if( gpTreeCache )
		{
			rslt =  sqlite3_close( gpTreeCache );
			gpTreeCache = NULL;
		}
	}
	else
	{
		if( !(gpTreeCache) ){	return E_OUTOFMEMORY;	}
	}

	return S_OK;
}

#endif

HRESULT SqlTransactionOnOff( BYTE mode )
{
	if( mode )
	{
		sqlite3_exec( gpDataBase, "BEGIN TRANSACTION", NULL, NULL, NULL );
	}
	else
	{
		sqlite3_exec( gpDataBase, "COMMIT TRANSACTION", NULL, NULL, NULL );
	}

	return S_OK;
}

HRESULT SqlTreeTableCreate( LPTSTR ptProfName )
{

	CONST CHAR	cacProfilesTable[] = { ("CREATE TABLE Profiles ( id INTEGER PRIMARY KEY, profname TEXT NOT NULL, rootpath TEXT NULL)") };

	CONST TCHAR	catProfInsFmt[] = { TEXT("INSERT INTO Profiles ( profname ) VALUES( '%s' )") };

	CONST CHAR	cacTreeNodeTable[] = { ("CREATE TABLE TreeNode ( id INTEGER PRIMARY KEY, type INTEGER NOT NULL, parentid INTEGER NOT NULL, nodename TEXT NOT NULL )") };

	CONST CHAR	cacMultiTabTable[] = { ("CREATE TABLE MultiTab ( id INTEGER PRIMARY KEY, filepath TEXT NOT NULL, basename TEXT NOT NULL, dispname TEXT )") };

	CONST CHAR	cacArtListTable[]  = { ("CREATE TABLE ArtList ( id INTEGER PRIMARY KEY, count INTEGER NOT NULL, folder TEXT NOT NULL, lastuse  REAL NOT NULL, hash INTEGER NOT NULL, conts BLOB NOT NULL )") };

	CHAR	acBuildVer[SUB_STRING], acText[MAX_PATH];
	TCHAR	atStr[MAX_STRING];
	SYSTEMTIME	stSysTime;

	BYTE	yMode = FALSE;
	INT		rslt;

	UINT	dVersion;
	CHAR	acQuery[MAX_PATH];

#ifdef _DEBUG
	INT		i;
#endif
	sqlite3_stmt	*statement;

	ZeroMemory( acText, sizeof(acText) );

	rslt = sqlite3_prepare( gpDataBase, cacDetectTable, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
#ifdef _DEBUG
		for( i = 0; 10 > i; i++ )
		{
			TRACE( TEXT("[TREE] %s"), (LPCTSTR)sqlite3_column_text16( statement, 0 ) );
			rslt = sqlite3_step( statement );
			if( SQLITE_DONE == rslt ){	break;	}
		}
#endif
		rslt = sqlite3_finalize( statement );
		yMode = TRUE;
	}

	if( !(yMode) )
	{

		TRACE( TEXT("ツリー用テーブルが見つからなかった") );

		rslt = sqlite3_finalize( statement );

		rslt = sqlite3_prepare( gpDataBase, cacVersionTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

		GetLocalTime( &stSysTime );
		StringCchPrintfA( acBuildVer, SUB_STRING, ("%d.%02d%02d.%02d%02d.%d"), stSysTime.wYear, stSysTime.wMonth, stSysTime.wDay, stSysTime.wHour, stSysTime.wMinute, SUBTAB_NAME_VER );
		StringCchPrintfA( acText, MAX_PATH, cacVerStrInsFmt, SUBTAB_NAME_VER, acBuildVer );

		rslt = sqlite3_prepare( gpDataBase, acText, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		sqlite3_reset( statement );
		rslt = sqlite3_step( statement );
		sqlite3_finalize(statement);

		rslt = sqlite3_prepare( gpDataBase, cacProfilesTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

		ZeroMemory( atStr, sizeof(atStr) );
		StringCchPrintf( atStr, MAX_STRING, catProfInsFmt, ptProfName );
		rslt = sqlite3_prepare16( gpDataBase, atStr, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		sqlite3_reset( statement );
		rslt = sqlite3_step( statement );
		sqlite3_finalize(statement);

		rslt = sqlite3_prepare( gpDataBase, cacTreeNodeTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

		rslt = sqlite3_prepare( gpDataBase, cacMultiTabTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

		rslt = sqlite3_prepare( gpDataBase, cacArtListTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

	}
	else
	{
		dVersion = 0;
		rslt = sqlite3_prepare( gpDataBase, cacVersionNumGet, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt ){	dVersion = sqlite3_column_int( statement, 0 );	}
		sqlite3_finalize( statement );
		TRACE( TEXT("PROFILE VERSION[%d]"), dVersion );

		if( TREE_TBL_VER == dVersion )
		{

			rslt = sqlite3_prepare( gpDataBase, ("ALTER TABLE MultiTab ADD COLUMN dispname TEXT DEFAULT \"\" "), -1, &statement, NULL );
			if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
			rslt = sqlite3_step( statement );
			if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
			rslt = sqlite3_finalize( statement );

			GetLocalTime( &stSysTime  );
			StringCchPrintfA( acBuildVer, SUB_STRING, ("%d.%02d%02d.%02d%02d.%d"),
				stSysTime.wYear, stSysTime.wMonth, stSysTime.wDay,
				stSysTime.wHour, stSysTime.wMinute, SUBTAB_NAME_VER );

			StringCchPrintfA( acQuery, MAX_PATH, cacVersionUpdate, SUBTAB_NAME_VER, acBuildVer );

			rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL);
			if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}

			sqlite3_reset( statement );
			rslt = sqlite3_step( statement );
			sqlite3_finalize( statement );

			dVersion = SUBTAB_NAME_VER;
		}
	}

	return S_OK;
}

UINT SqlTreeCount( UINT bType, PUINT pdMax )
{
	CHAR	acQuery[MAX_STRING];
	UINT	rslt, iCount, iMax;
	sqlite3_stmt*	statement;

	sqlite3	*pDB;

	switch( bType )
	{
		case  0:	pDB = gpDataBase;	StringCchCopyA( acQuery, MAX_STRING, ("SELECT COUNT(id) FROM Profiles") );	break;
		case  1:	pDB = gpDataBase;	StringCchCopyA( acQuery, MAX_STRING, ("SELECT COUNT(id) FROM TreeNode") );	break;
		case  2:	pDB = gpDataBase;	StringCchCopyA( acQuery, MAX_STRING, ("SELECT COUNT(id) FROM MultiTab") );	break;
		case  3:	pDB = gpTreeCache;	StringCchCopyA( acQuery, MAX_STRING, ("SELECT COUNT(id) FROM TreeNode") );	break;
		default:	return 0;
	}

	rslt = sqlite3_prepare( pDB, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( pDB );	return 0;	}

	sqlite3_step( statement );
	iCount = sqlite3_column_int( statement, 0 );
	sqlite3_finalize(statement);

	if( !pdMax )	return iCount;

	switch( bType )
	{
		case  0:	StringCchCopyA( acQuery, MAX_STRING, ("SELECT MAX(id) FROM Profiles") );	break;
		case  1:	StringCchCopyA( acQuery, MAX_STRING, ("SELECT MAX(id) FROM TreeNode") );	break;
		case  2:	StringCchCopyA( acQuery, MAX_STRING, ("SELECT MAX(id) FROM MultiTab") );	break;
		case  3:	StringCchCopyA( acQuery, MAX_STRING, ("SELECT MAX(id) FROM TreeNode") );	break;
		default:	return 0;
	}

	rslt = sqlite3_prepare( pDB, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( pDB );	return 0;	}

	sqlite3_step( statement );
	iMax = sqlite3_column_int( statement, 0 );
	sqlite3_finalize( statement );

	*pdMax = iMax;

	return iCount;
}

HRESULT SqlTreeProfUpdate( LPCTSTR ptProfName, LPCTSTR ptRootPath )
{

	CONST  TCHAR	catUpdateName[] = { TEXT("UPDATE Profiles SET profname = '%s' WHERE id == 1") };
	CONST  TCHAR	catUpdateRoot[] = { TEXT("UPDATE Profiles SET rootpath = '%s' WHERE id == 1") };

	TCHAR	atText[MAX_PATH];
	INT		rslt;
	sqlite3_stmt	*statement;

	if( ptProfName )
	{
		StringCchPrintf( atText, MAX_PATH, catUpdateName, ptProfName );

		rslt = sqlite3_prepare16( gpDataBase, atText, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
		rslt = sqlite3_step( statement );	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}
		sqlite3_finalize( statement );
	}

	if( ptRootPath )
	{
		StringCchPrintf( atText, MAX_PATH, catUpdateRoot, ptRootPath );

		rslt = sqlite3_prepare16( gpDataBase, atText, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
		rslt = sqlite3_step( statement );	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}
		sqlite3_finalize( statement );
	}

	return S_OK;
}

HRESULT SqlTreeProfSelect( LPTSTR ptProfName, UINT szName, LPTSTR ptRootPath, UINT szRoot )
{
	CONST CHAR	cacSelectQuery[] = { ("SELECT * FROM Profiles WHERE id == 1") };

	TCHAR	atName[MAX_STRING], atRoot[MAX_PATH];
	INT		index;
	INT		rslt;
	sqlite3_stmt	*statement;

	ZeroMemory( atName, sizeof(atName) );
	ZeroMemory( atRoot, sizeof(atRoot) );

	rslt = sqlite3_prepare( gpDataBase, cacSelectQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}

	rslt = sqlite3_reset( statement );

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		index = sqlite3_column_int( statement , 0 );
		String_Cch_Copy( atName , MAX_STRING, (LPTSTR)sqlite3_column_text16( statement, 1 ) );
		String_Cch_Copy( atRoot , MAX_STRING, (LPTSTR)sqlite3_column_text16( statement, 2 ) );

	}
	sqlite3_finalize( statement );

	if( ptProfName )	StringCchCopy( ptProfName, szName, atName );
	if( ptRootPath )	StringCchCopy( ptRootPath, szRoot, atRoot );

	return S_OK;
}

UINT SqlTreeNodeInsert( UINT uqID, UINT dType, UINT dPrnt, LPTSTR ptName )
{
	CONST CHAR	acTreeNodeInsert[] = { ("INSERT INTO TreeNode ( id, type, parentid, nodename ) VALUES ( ?, ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM TreeNode") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, acTreeNodeInsert, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_int(    statement, 1, uqID );
	rslt = sqlite3_bind_int(    statement, 2, dType );
	rslt = sqlite3_bind_int(    statement, 3, dPrnt );
	rslt = sqlite3_bind_text16( statement, 4, ptName, -1, SQLITE_STATIC );

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize( statement );

	rslt = sqlite3_prepare( gpDataBase, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}

#ifdef EXTRA_NODE_STYLE

UINT SqlTreeNodeExtraInsert( UINT dType, LPCTSTR ptName )
{
	CONST CHAR	acTreeNodeExIns[] = { ("INSERT INTO TreeNode ( type, parentid, nodename ) VALUES ( ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM TreeNode") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, acTreeNodeExIns, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_int(    statement, 1, 0 );
	rslt = sqlite3_bind_int(    statement, 2, -1 );
	rslt = sqlite3_bind_text16( statement, 3, ptName, -1, SQLITE_STATIC );

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize( statement );

	rslt = sqlite3_prepare( gpDataBase, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}

UINT SqlTreeNodeExtraDelete( UINT delID )
{
	CONST CHAR	cacDelete[] = { ("DELETE FROM TreeNode WHERE id == ?") };

	INT		rslt;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, cacDelete, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_int( statement, 1 , delID );

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize( statement );

	return 1;
}

UINT SqlTreeNodeExtraSelect( UINT seekID, UINT tgtID, LPTSTR ptName )
{

	CHAR	acQuery[MAX_STRING];
	INT		rslt;
	UINT	id = 0;
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	if( 1 <= seekID ){	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT id, nodename FROM TreeNode WHERE id == %u"), seekID );	}
	else{	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT id, nodename FROM TreeNode WHERE id > %u AND parentid == -1 ORDER BY id ASC"), tgtID );	}

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}
	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id = sqlite3_column_int( statement, 0 );
		String_Cch_Copy( ptName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 1 ) );

	}

	sqlite3_finalize( statement );

	return id;
}

UINT SqlTreeNodeExtraIsFileExist( LPCTSTR ptName )
{
	CONST CHAR	cacNameSearch[] = { ("SELECT id FROM TreeNode WHERE nodename == ?") };

	INT		rslt;
	UINT	id = 0;
	sqlite3_stmt*	statement;

	rslt = sqlite3_prepare( gpDataBase, cacNameSearch, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_text16( statement, 1, ptName, -1, SQLITE_STATIC );

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id = sqlite3_column_int( statement, 0 );
	}

	sqlite3_finalize( statement );

	return id;
}

#endif

#if 0

HRESULT SqlTreeNodeEnum( UINT dProfID, BUFFERBACK pfDataing )
{
	CONST CHAR	acQuery[] = { ("SELECT * FROM TreeNode ORDER BY id ASC") };
	INT		rslt;
	UINT	i, cnt;
	UINT	id, type, prnt;
	TCHAR	atName[MAX_PATH];
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	return E_OUTOFMEMORY;	}

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_FAIL;	}

	cnt = SqlTreeCount( 1, NULL );

	for( i = 0; cnt > i; i++ )
	{
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt )
		{
			id   = sqlite3_column_int( statement, 0 );
			type = sqlite3_column_int( statement, 1 );
			prnt = sqlite3_column_int( statement, 2 );
			String_Cch_Copy( atName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 3 ) );

			pfDataing( id, type, prnt, atName );
		}
		else{	break;	}
	}

	sqlite3_finalize( statement );

	return S_OK;
}

#endif

#if 0

UINT SqlTreeNodeRootSearch( LPTSTR ptDirName )
{
	TCHAR	atQuery[BIG_STRING];
	INT		rslt;
	UINT	id = 0;
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	return 0;	}

	StringCchPrintf( atQuery, BIG_STRING, TEXT("SELECT id FROM TreeNode WHERE parentid == 0 AND nodename == '%s'"), ptDirName );

	rslt = sqlite3_prepare16( gpDataBase, atQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id = sqlite3_column_int( statement, 0 );
	}

	sqlite3_finalize( statement );

	return id;
}

#endif

UINT SqlTreeNodePickUpID( UINT tgtID, PUINT pType, PUINT pPrntID, LPTSTR ptName, UINT bStyle )
{
	CHAR	acQuery[MAX_STRING];
	INT		rslt;
	UINT	id = 0;
	sqlite3_stmt*	statement;

	sqlite3	*pDB;

	if( bStyle & 0x01 ){	pDB = gpDataBase;	}
	else{					pDB = gpTreeCache;	}

	if( !(pDB) ){	return 0;	}

	if( bStyle & 0x10 ){	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT * FROM TreeNode WHERE id == %u"), tgtID );	}
	else{	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT * FROM TreeNode WHERE id > %u ORDER BY id ASC"), tgtID );	}

	rslt = sqlite3_prepare( pDB, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( pDB );	return 0;	}
	sqlite3_reset( statement );
	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id       = sqlite3_column_int( statement, 0 );
		*pType   = sqlite3_column_int( statement, 1 );
		*pPrntID = sqlite3_column_int( statement, 2 );
		String_Cch_Copy( ptName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 3 ) );
	}

	sqlite3_finalize( statement );

	return id;
}

UINT SqlChildNodePickUpID( UINT dPrntID, UINT tgtID, PUINT pType, LPTSTR ptName )
{
	CHAR	acQuery[MAX_PATH];
	INT		rslt;
	UINT	id = 0, dummy;
	sqlite3_stmt*	statement;

	StringCchPrintfA( acQuery, MAX_PATH, ("SELECT * FROM TreeNode WHERE parentid == %u AND id > %u ORDER BY id ASC"), dPrntID, tgtID );

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}
	sqlite3_reset( statement );
	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id     = sqlite3_column_int( statement, 0 );
		*pType = sqlite3_column_int( statement, 1 );
		dummy  = sqlite3_column_int( statement, 2 );
		String_Cch_Copy( ptName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 3 ) );
	}

	sqlite3_finalize( statement );

	return id;
}

HRESULT SqlTreeNodeAllDelete( UINT bStyle )
{
	CONST CHAR	acTreeDelete[] = { ("DELETE FROM TreeNode") };
	INT		rslt;
	sqlite3_stmt	*statement;

	sqlite3	*pDB;

	if( bStyle )	pDB = gpDataBase;
	else			pDB = gpTreeCache;

	rslt = sqlite3_prepare( pDB, acTreeDelete, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( pDB );	return E_OUTOFMEMORY;	}

	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}

UINT SqlTreeFileSearch( LPTSTR ptName, UINT dStart )
{
	TCHAR	atReqest[SUB_STRING];
	INT		rslt;
	UINT	dxID;
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, ("SELECT id FROM TreeNode WHERE nodename LIKE ? AND id > ? ORDER BY id ASC"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );

	StringCchPrintf( atReqest, SUB_STRING, TEXT("%%%s%%"), ptName );
	rslt = sqlite3_bind_text16( statement, 1, atReqest, -1, SQLITE_STATIC );
	sqlite3_bind_int( statement, 2, dStart );

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt ){	dxID = sqlite3_column_int( statement, 0 );	}
	else{	dxID = 0;	}

	sqlite3_finalize( statement );

	return dxID;
}

UINT SqlTreeFileGetOnParent( LPTSTR ptName, UINT dPrntID )
{
	INT		rslt;
	UINT	dxID;
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, ("SELECT id FROM TreeNode WHERE nodename == ? AND parentid == ?"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );

	rslt = sqlite3_bind_text16( statement, 1, ptName, -1, SQLITE_STATIC );
	sqlite3_bind_int( statement, 2, dPrntID );

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt ){	dxID = sqlite3_column_int( statement, 0 );	}
	else{	dxID = 0;	}

	sqlite3_finalize( statement );

	return dxID;
}

HRESULT SqlTreeCacheOpenClose( UINT bMode )
{

	CONST CHAR	cacTreeNodeTable[] = { ("CREATE TABLE TreeNode ( id INTEGER PRIMARY KEY, type INTEGER NOT NULL, parentid INTEGER NOT NULL, nodename TEXT NOT NULL )") };
	INT		rslt;
	sqlite3_stmt	*statement;

	if( bMode )
	{
		rslt = sqlite3_open( (":memory:"), &gpTreeCache );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return E_FAIL;	}

		rslt = sqlite3_prepare( gpTreeCache, cacTreeNodeTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpTreeCache );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

		sqlite3_exec( gpTreeCache, "BEGIN TRANSACTION", NULL, NULL, NULL );
	}
	else
	{
		sqlite3_exec( gpTreeCache, "COMMIT TRANSACTION", NULL, NULL, NULL );

		if( gpTreeCache ){	rslt = sqlite3_close( gpTreeCache );	}
		gpTreeCache = NULL;
	}

	return S_OK;
}

HRESULT SqlTreeCacheDelID( INT tgtID )
{
	CONST CHAR	acTreeDel[] = { ("DELETE FROM TreeNode WHERE id == ? OR parentid == ?") };
	INT		rslt;
	sqlite3_stmt	*statement;

	rslt = sqlite3_prepare( gpTreeCache, acTreeDel, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return E_OUTOFMEMORY;	}

	sqlite3_reset( statement );
	sqlite3_bind_int( statement, 1, tgtID );
	sqlite3_bind_int( statement, 2, tgtID );
	sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}

UINT SqlTreeCacheInsert( UINT dType, UINT dPrnt, LPTSTR ptName )
{
	CONST CHAR	acTreeNodeInsert[] = { ("INSERT INTO TreeNode ( type, parentid, nodename ) VALUES ( ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM TreeNode") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;

	if( !(gpTreeCache) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpTreeCache, acTreeNodeInsert, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_int(    statement, 1, dType );
	rslt = sqlite3_bind_int(    statement, 2, dPrnt );
	rslt = sqlite3_bind_text16( statement, 3, ptName, -1, SQLITE_STATIC );

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpTreeCache );	}

	sqlite3_finalize( statement );

	rslt = sqlite3_prepare( gpTreeCache, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}

UINT SqlMultiTabInsert( LPCTSTR ptFilePath, LPCTSTR ptBaseName, LPCTSTR ptDispName )
{
	CONST CHAR	acMultitabInsert[] = { ("INSERT INTO MultiTab ( filepath, basename, dispname ) VALUES ( ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM MultiTab") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, acMultitabInsert, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_text16( statement, 1, ptFilePath, -1, SQLITE_STATIC );
	rslt = sqlite3_bind_text16( statement, 2, ptBaseName, -1, SQLITE_STATIC );
	rslt = sqlite3_bind_text16( statement, 3, ptDispName, -1, SQLITE_STATIC );

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize( statement );

	rslt = sqlite3_prepare( gpDataBase, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}

UINT SqlMultiTabSelect( INT id, LPTSTR ptFilePath, LPTSTR ptBaseName, LPTSTR ptDispName )
{
	CHAR	acQuery[MAX_STRING];
	INT		rslt;
	UINT	index = 0;
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	return 0;	}

	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT * FROM MultiTab WHERE id == %d"), id );

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		index = sqlite3_column_int( statement , 0 );
		String_Cch_Copy( ptFilePath, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 1 ) );
		String_Cch_Copy( ptBaseName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 2 ) );
		String_Cch_Copy( ptDispName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 3 ) );
	}

	sqlite3_finalize( statement );

	return index;
}

HRESULT SqlMultiTabDelete( VOID )
{
	INT		rslt;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, ("DELETE FROM MultiTab"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}
