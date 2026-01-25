/******************************************************************************\
*  ufs_core_sqlite.c                                                           *
*                                                                              *
*  Sqlite implementation of ufs_core.                                          *
*                                                                              *
*              Written by A.N.                                  24-01-2026     *
*                                                                              *
\******************************************************************************/


#include "sqlite3.h"
#include "ufs_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum ufsSqliteStatementType {
    UFS_STATMENT_INSERT_INTO_STORAGE,
    UFS_STATMENT_QUERY_STORAGE_BY_NAME_TYPE,
    UFS_STATMENT_QUERY_STORAGE_BY_ID,
    UFS_STATMENT_QUERY_FILES_BY_IDS,
    UFS_STATMENT_INSERT_INTO_FILES,
    UFS_STATMENT_INSERT_INTO_AREAS,
    UFS_STATMENT_QUERY_AREAS_BY_NAME,
    NUM_UFS_STATEMENTS,
};

typedef struct ufsSqliteStruct {
    sqlite3 *db;
    sqlite3_stmt *statements[ NUM_UFS_STATEMENTS ];

} ufsSqliteStruct;

static const char *UFS_SQL_TEXT[ NUM_UFS_STATEMENTS + 2 ] = {

    /* Schema command:                                                        */
    "CREATE TABLE IF NOT EXISTS ufsStorage(id INTEGER PRIMARY KEY, name STRING NOT NULL, isDir BOOLEAN);" ""
    "CREATE TABLE IF NOT EXISTS ufsFiles( id INTEGER PRIMARY KEY, dirId INTEGER, fileId INTEGER,"
                                                 "FOREIGN KEY (dirId) REFERENCES ufsStorage(id),"
                                                 "FOREIGN KEY (fileId) REFERENCES ufsStorage(id) );"
    "CREATE TABLE IF NOT EXISTS ufsAreas(id INTEGER PRIMARY KEY, name STRING NOT NULL);" "",

    /* Insert into the storage table:                                         */
    "INSERT INTO ufsStorage (name, isDir) VALUES (?, ?);",

    /* Query storage by name, type:                                           */
    "SELECT id from ufsStorage where name = ? and isDir = ?;",

    /* Query storage by id:                                                   */
    "SELECT id from ufsStorage where id = ?;",

    /* Query files by ids:                                                    */
    "SELECT id from ufsFiles where dirId = ? and fileId = ?;",

    /* Insert into files:                                                     */
    "INSERT INTO ufsFiles (dirId, fileId) VALUES (?, ?);", 

    /* Insert into areas:                                                     */
    "INSERT INTO ufsAreas (name) VALUES (?);", 

    /* Query area by name:                                                    */
    "SELECT id from ufsAreas WHERE name = ?;", 

    NULL
};

static inline ufsSqliteStruct *prepareSqliteDb( sqlite3 *db );


struct ufsSqliteStruct *prepareSqliteDb( sqlite3 *db )
{
    int res, i;
    ufsSqliteStruct *ufsSqlite;

    ufsSqlite = malloc( sizeof( *ufsSqlite ) );
    if ( !ufsSqlite ) {
        ufsErrno = UFS_OUT_OF_MEMORY;
        return NULL;
    }

    ufsSqlite -> db = db;
    res = sqlite3_exec( db, UFS_SQL_TEXT[ 0 ], NULL, NULL, NULL );
    if ( res != SQLITE_OK ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return NULL;
    }

    for ( i = 1; UFS_SQL_TEXT[ i ]; i++ ) {
        res = sqlite3_prepare_v2( db,
                                  UFS_SQL_TEXT[ i ],
                                  -1,
                                  &ufsSqlite -> statements[ i - 1 ],
                                  NULL );
        if (res != SQLITE_OK) {
            ufsErrno = UFS_UNKNOWN_ERROR;
            return NULL;
        }
    }

    ufsErrno = UFS_NO_ERROR;
    return ufsSqlite;
}

ufsType ufsInit()
{
    ufsSqliteStruct *ret;
    sqlite3 *db;
    int res;

    res = sqlite3_open( ":memory:", &db );
    if ( !db ) {
        ufsErrno = UFS_OUT_OF_MEMORY;
        return NULL;
    }

    if ( res != SQLITE_OK ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return NULL;
    }

    ret = prepareSqliteDb( db );
    if ( !ret )
        return NULL;
    ufsErrno = UFS_NO_ERROR;
    return ret;
}

void ufsDestroy( ufsType ufs )
{
    int i;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs ) {
        ufsErrno = UFS_NO_ERROR;
        return;
    }

    ufsSqlite = ufs;
    for (i = 0; i < NUM_UFS_STATEMENTS; i++)
        sqlite3_finalize( ufsSqlite -> statements[ i ] );
    sqlite3_close( ufsSqlite -> db );
    free( ufsSqlite );
    ufsErrno = UFS_NO_ERROR;
}

ufsIdentifierType ufsAddDirectory( ufsType ufs,
                                   ufsIdentifierType parent,
                                   const char *name )
{
    ufsErrno = UFS_NO_ERROR;
    return 0;
}

ufsIdentifierType ufsAddFile( ufsType ufs,
                              ufsIdentifierType parent,     
                              const char *name )
{
    ufsErrno = UFS_NO_ERROR;
    return 0;
}

ufsIdentifierType ufsAddArea( ufsType ufs,
                              const char *name )
{
    ufsErrno = UFS_NO_ERROR;
    return 0;
}

ufsStatusType ufsAddMapping( ufsType ufs,
                             ufsIdentifierType area,
                             ufsIdentifierType storage )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsIdentifierType ufsGetDirectory( ufsType ufs,
                                   ufsIdentifierType parent,
                                   const char *name )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsIdentifierType ufsGetFile( ufsType ufs,
                              ufsIdentifierType parent,
                              const char *name )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsIdentifierType ufsGetArea( ufsType ufs,
                              const char *name )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsStatusType ufsProbeMapping( ufsType ufs,
                               ufsIdentifierType area,
                               ufsIdentifierType storage )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsStatusType ufsRemoveDirectory( ufsType ufs,
                                  ufsIdentifierType directory )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsStatusType ufsRemoveFile( ufsType ufs,
                             ufsIdentifierType file )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsStatusType ufsRemoveArea( ufsType ufs,
                             ufsIdentifierType area )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsStatusType ufsRemoveMapping( ufsType ufs,
                                ufsIdentifierType area,
                                ufsIdentifierType storage )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsIdentifierType ufsResolveStorageInView( ufsType ufs,
                                           ufsViewType view,
                                           ufsIdentifierType storage )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsStatusType ufsIterateDirInView( ufsType ufs,
                                   ufsViewType view,
                                   ufsIdentifierType directory,
                                   ufsDirIter iterator,
                                   void *userData )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsStatusType ufsCollapse( ufsType ufs,
                           ufsViewType view )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}
