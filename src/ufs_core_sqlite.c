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

typedef struct ufsSqliteStruct {
    sqlite3 *db;
    sqlite3_stmt *statements[1];

} ufsSqliteStruct;

static const char *UFS_SCHEMA =
    "CREATE TABLE IF NOT EXISTS ufsStorage(id INTEGER PRIMARY KEY, name STRING NOT NULL, isDir BOOLEAN);";

static inline ufsSqliteStruct *prepareSqliteDb( sqlite3 *db );


struct ufsSqliteStruct *prepareSqliteDb( sqlite3 *db )
{
    char *errorMsg;
    int res;
    ufsSqliteStruct *ufsSqlite;

    ufsSqlite = malloc( sizeof( *ufsSqlite ) );
    if (!ufsSqlite) {
        ufsErrno = UFS_OUT_OF_MEMORY;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "could not allocate memory for ufsSqliteStruct.\n", ufsStatusStrings[ ufsErrno ] );
        return NULL;
    }

    ufsSqlite -> db = db;
    res = sqlite3_exec( db, UFS_SCHEMA, NULL, NULL, &errorMsg );
    if (res != SQLITE_OK) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "%s.\n", ufsStatusStrings[ ufsErrno ], errorMsg );
        sqlite3_free( errorMsg );
        return NULL;
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
    if (!db) {
        ufsErrno = UFS_OUT_OF_MEMORY;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "could not allocate memory for db connection.\n", ufsStatusStrings[ ufsErrno ] );
        return NULL;
    }

    if (res != SQLITE_OK) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "%s.\n",ufsStatusStrings[ ufsErrno ],  sqlite3_errmsg( db ) );
        return NULL;

    }
    ret = prepareSqliteDb( db );
    if (!ret)
        return NULL;
    ufsErrno = UFS_NO_ERROR;
    return ret;
}

void ufsDestroy( ufsType ufs )
{
    ufsSqliteStruct *ufsSqlite;
    if (!ufs) {
        ufsErrno = UFS_NO_ERROR;
        return;
    }

    ufsSqlite = ufs;
    sqlite3_close( ufsSqlite -> db );
    free( ufsSqlite );
    ufsErrno = UFS_NO_ERROR;
}

ufsIdentifierType ufsAddDirectory( ufsType ufs,
                                   const char *name )
{
    ufsErrno = UFS_NO_ERROR;
    return 0;
}

ufsIdentifierType ufsAddFile( ufsType ufs,
                              ufsIdentifierType directory,     
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
                                   const char *name )
{
    ufsErrno = UFS_NO_ERROR;
	return 0;
}

ufsIdentifierType ufsGetFile( ufsType ufs,
                              ufsIdentifierType directory,
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
