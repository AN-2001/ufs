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

#define NUM_SQLITE_STATEMENTS ( 2 )

typedef struct ufsSqliteStruct {
    sqlite3 *db;
    sqlite3_stmt *statements[ NUM_SQLITE_STATEMENTS ];

} ufsSqliteStruct;

static const char *UFS_SQL_TEXT[] = {

    /* Schema command:                                                        */
    "CREATE TABLE IF NOT EXISTS ufsStorage(id INTEGER PRIMARY KEY, name STRING NOT NULL, isDir BOOLEAN);",

    /* Insert into the storage table:                                         */
    "INSERT INTO ufsStorage (name, isDir) VALUES (?, ?);",

    /* Query storage by name:                                                 */
    "SELECT id from ufsStorage where name = ? and isDir = ?;",

    NULL
};

static inline ufsSqliteStruct *prepareSqliteDb( sqlite3 *db );


struct ufsSqliteStruct *prepareSqliteDb( sqlite3 *db )
{
    char *errorMsg;
    int res, i;
    ufsSqliteStruct *ufsSqlite;

    ufsSqlite = malloc( sizeof( *ufsSqlite ) );
    if ( !ufsSqlite ) {
        ufsErrno = UFS_OUT_OF_MEMORY;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "could not allocate memory for ufsSqliteStruct.\n", ufsStatusStrings[ ufsErrno ] );
        return NULL;
    }

    ufsSqlite -> db = db;
    res = sqlite3_exec( db, UFS_SQL_TEXT[ 0 ], NULL, NULL, &errorMsg );
    if ( res != SQLITE_OK ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "%s.\n", ufsStatusStrings[ ufsErrno ], errorMsg );
        sqlite3_free( errorMsg );
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
            fprintf( stderr, "ufsSqlite: received error %s:"
                    "%s.\n", ufsStatusStrings[ ufsErrno ], sqlite3_errmsg( db ) );
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
        fprintf( stderr, "ufsSqlite: received error %s:"
                "could not allocate memory for db connection.\n", ufsStatusStrings[ ufsErrno ] );
        return NULL;
    }

    if ( res != SQLITE_OK ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "%s.\n",ufsStatusStrings[ ufsErrno ],  sqlite3_errmsg( db ) );
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
    for (i = 0; i < NUM_SQLITE_STATEMENTS; i++)
        sqlite3_finalize( ufsSqlite -> statements[ i ] );
    sqlite3_close( ufsSqlite -> db );
    free( ufsSqlite );
    ufsErrno = UFS_NO_ERROR;
}

ufsIdentifierType ufsAddDirectory( ufsType ufs,
                                   const char *name )
{
    ufsSqliteStruct *ufsSqlite;
    int res;
    if (!ufs || !name) {
        ufsErrno = UFS_BAD_CALL;
        return -1;
    }

    ufsSqlite = ufs;

    sqlite3_bind_text( ufsSqlite -> statements[ 1 ], 1, name, -1, SQLITE_TRANSIENT );
    sqlite3_bind_int( ufsSqlite -> statements[ 1 ], 2, 1);

    res = sqlite3_step( ufsSqlite -> statements[ 1 ] );
    if ( res == SQLITE_ROW ) {
        ufsErrno = UFS_ALREADY_EXISTS;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "Directory already exists.\n", ufsStatusStrings[ ufsErrno ] );
        return -1;
    }

    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "%s.\n",ufsStatusStrings[ ufsErrno ], sqlite3_errmsg( ufsSqlite -> db ) );
        return -1;
    }

    sqlite3_bind_text( ufsSqlite -> statements[ 0 ], 1, name, -1, SQLITE_TRANSIENT );
    sqlite3_bind_int( ufsSqlite -> statements[ 0 ], 2, 1);

    res = sqlite3_step( ufsSqlite -> statements[ 0 ] );
    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        fprintf( stderr, "ufsSqlite: received error %s:"
                "%s.\n",ufsStatusStrings[ ufsErrno ], sqlite3_errmsg( ufsSqlite -> db ) );
        return -1;
    }

    ufsErrno = UFS_NO_ERROR;
    return sqlite3_last_insert_rowid( ufsSqlite -> db );
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
