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
    UFS_STATEMENT_INSERT_INTO_STORAGE,
    UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE,
    UFS_STATEMENT_QUERY_STORAGE_BY_ID,
    UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE,
    UFS_STATEMENT_INSERT_INTO_AREAS,
    UFS_STATEMENT_QUERY_AREAS_BY_NAME,
    UFS_STATEMENT_QUERY_AREAS_BY_ID,
    UFS_STATEMENT_INSERT_INTO_MAPPINGS,
    UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS,
    NUM_UFS_STATEMENTS,
};

typedef struct ufsSqliteStruct {
    sqlite3 *db;
    ufsIdentifierType rootId;
    sqlite3_stmt *statements[ NUM_UFS_STATEMENTS ];

} ufsSqliteStruct;

static const char *UFS_SQL_TEXT[ NUM_UFS_STATEMENTS + 2 ] = {

    /* Schema command:                                                        */
    "CREATE TABLE IF NOT EXISTS ufsStorage(id INTEGER PRIMARY KEY,"
                                          "name TEXT NOT NULL,"
                                          "parent INTEGER,"
                                          "type INTEGER );"
    "CREATE TABLE IF NOT EXISTS ufsAreas(id INTEGER PRIMARY KEY,"
                                         "name TEXT NOT NULL );"
    "CREATE TABLE IF NOT EXISTS ufsMappings(id INTEGER PRIMARY KEY,"
                                           "areaId INTEGER,"
                                           "storageId INTEGER,"
                                           "FOREIGN KEY (areaId) REFERENCES ufsAreas(id),"
                                           "FOREIGN KEY (storageId) REFERENCES ufsStorage(id) );"
    ,

    /* Insert into the storage table:                                         */
    "INSERT INTO ufsStorage (name, parent, type) VALUES (?, ?, ?);",

    /* Query storage by name, parent, type:                                   */
    "SELECT id from ufsStorage where name = ? and parent = ? and type = ?;",

    /* Query storage by id:                                                   */
    "SELECT id from ufsStorage where id = ?;",

    /* Query storage by id, type:                                             */
    "SELECT id from ufsStorage where id = ? and type = ?;",

    /* Insert into the area table:                                            */
    "INSERT INTO ufsAreas (name) VALUES (?);",

    /* Query areas by name:                                                   */
    "SELECT id from ufsAreas where name = ?;",

    /* Query areas by id:                                                     */
    "SELECT id from ufsAreas where id = ?;",

    /* Insert into mappings:                                                  */
    "INSERT INTO ufsMappings (areaId, storageId) VALUES (?, ?);",

    /* Query mappings by IDs:                                                 */
    "SELECT id from ufsMappings where areaId = ? and storageId = ?;",

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
        free( ufsSqlite );
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
            free( ufsSqlite );
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
        sqlite3_close( db );
        ufsErrno = UFS_UNKNOWN_ERROR;
        return NULL;
    }

    ret = prepareSqliteDb( db );
    if ( !ret ) {
        sqlite3_close( db );
        return NULL;
    }
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
    ufsSqliteStruct *ufsSqlite;
    int res;
    if ( !ufs || parent < 0 || !name ) {
        ufsErrno = UFS_BAD_CALL;
        return -1;
    }

    ufsSqlite = ufs;

    /* Make sure parent is a directory if it's not ROOT.                      */
    if ( parent > 0 ) {
        sqlite3_reset(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        sqlite3_clear_bindings(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        sqlite3_bind_int(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ],
                1, parent );
        sqlite3_bind_int(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ],
                2, UFS_STORAGE_TYPE_DIRECTORY );
        res = sqlite3_step(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        if ( res != SQLITE_ROW ) {
            ufsErrno = UFS_PARENT_DOES_NOT_EXIST;
            return -1;
        }
    }

    /* Make sure it doesn't exist.                                            */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    sqlite3_bind_text(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            1, name, -1, SQLITE_TRANSIENT );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            2, parent );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            3, UFS_STORAGE_TYPE_DIRECTORY );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    if ( res == SQLITE_ROW ) {
        ufsErrno = UFS_ALREADY_EXISTS;
        return -1;
    }

    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return -1;
    }

    /* Finally, insert the directory into the db.                             */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ] );
    sqlite3_bind_text(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ],
            1, name, -1, SQLITE_TRANSIENT );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ],
            2, parent );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ],
            3, UFS_STORAGE_TYPE_DIRECTORY );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ] );
    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return -1;
    }

    ufsErrno = UFS_NO_ERROR;
    return sqlite3_last_insert_rowid( ufsSqlite -> db );
}

ufsIdentifierType ufsAddFile( ufsType ufs,
                              ufsIdentifierType parent,     
                              const char *name )
{
    ufsSqliteStruct *ufsSqlite;
    int res;
    if ( !ufs || parent < 0 || !name ) {
        ufsErrno = UFS_BAD_CALL;
        return -1;
    }

    ufsSqlite = ufs;

    /* Make sure parent is a directory if it's not ROOT.                      */
    if ( parent > 0 ) {
        sqlite3_reset(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        sqlite3_clear_bindings(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        sqlite3_bind_int(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ],
                1, parent );
        sqlite3_bind_int(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ],
                2, UFS_STORAGE_TYPE_DIRECTORY );
        res = sqlite3_step(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        if ( res != SQLITE_ROW ) {
            ufsErrno = UFS_PARENT_DOES_NOT_EXIST;
            return -1;
        }
    }

    /* Make sure it doesn't exist.                                            */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    sqlite3_bind_text(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            1, name, -1, SQLITE_TRANSIENT );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            2, parent );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            3, UFS_STORAGE_TYPE_FILE );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    if ( res == SQLITE_ROW ) {
        ufsErrno = UFS_ALREADY_EXISTS;
        return -1;
    }

    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return -1;
    }

    /* Finally, insert the file into the db.                                  */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ] );
    sqlite3_bind_text(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ],
            1, name, -1, SQLITE_TRANSIENT );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ],
            2, parent );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ],
            3, UFS_STORAGE_TYPE_FILE );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ] );
    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return -1;
    }

    ufsErrno = UFS_NO_ERROR;
    return sqlite3_last_insert_rowid( ufsSqlite -> db );
}

ufsIdentifierType ufsAddArea( ufsType ufs,
                              const char *name )
{
    ufsSqliteStruct *ufsSqlite;
    int res;
    if ( !ufs || !name ) {
        ufsErrno = UFS_BAD_CALL;
        return -1;
    }

    if (strncmp( name,
                UFS_AREA_BASE_NAME,
                sizeof( UFS_AREA_BASE_NAME ) /
                sizeof( char )) == 0 ) {
        ufsErrno = UFS_ILLEGAL_NAME;
        return -1;
    }

    ufsSqlite = ufs;

    /* First verify that the area doesn't exist.                              */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ] );
    sqlite3_bind_text( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ],
            1, name, -1, SQLITE_TRANSIENT );
    res = sqlite3_step( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ] );
    if ( res == SQLITE_ROW ) {
        ufsErrno = UFS_ALREADY_EXISTS;
        return -1;
    }

    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return -1;
    }

    /* Finally, insert the area into the db.                                  */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_AREAS ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_AREAS ] );
    sqlite3_bind_text(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_AREAS ],
            1, name, -1, SQLITE_TRANSIENT );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_AREAS ] );
    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return -1;
    }

    ufsErrno = UFS_NO_ERROR;
    return sqlite3_last_insert_rowid( ufsSqlite -> db );
}

ufsStatusType ufsAddMapping( ufsType ufs,
                             ufsIdentifierType area,
                             ufsIdentifierType storage )
{
    int res;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs || area <= 0 || storage < 0 ) {
        ufsErrno = UFS_BAD_CALL;
        return ufsErrno;
    }

    ufsSqlite = ufs;

    /* First verify that area exists.                                         */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_ID ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_ID ] );
    sqlite3_bind_int( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_ID ],
            1, area );
    res = sqlite3_step( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_ID ] );
    if ( res != SQLITE_ROW ) {
        ufsErrno = UFS_DOES_NOT_EXIST;
        return ufsErrno;
    }

    /* Then verify that the storage exists.                                   */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID ] );
    sqlite3_bind_int( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID ],
            1, storage );
    res = sqlite3_step( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID ] );
    if ( res != SQLITE_ROW ) {
        ufsErrno = UFS_DOES_NOT_EXIST;
        return ufsErrno;
    }

    /* verify that the mapping doesn't exist.                                 */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ] );
    sqlite3_bind_int( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ],
            1, area );
    sqlite3_bind_int( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ],
            2, storage );
    res = sqlite3_step( 
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ] );
    if ( res == SQLITE_ROW ) {
        ufsErrno = UFS_ALREADY_EXISTS;
        return ufsErrno;
    }

    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return ufsErrno;
    }

    /* Finally, insert the area into the db.                                  */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_MAPPINGS ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_MAPPINGS ] );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_MAPPINGS ],
            1, area );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_MAPPINGS ],
            2, storage );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_MAPPINGS ] );
    if ( res != SQLITE_DONE ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        return ufsErrno;
    }


    ufsErrno = UFS_NO_ERROR;
	return ufsErrno;
}

ufsIdentifierType ufsGetDirectory( ufsType ufs,
                                   ufsIdentifierType parent,
                                   const char *name )
{
    int res;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs || parent < 0 || !name ) {
        ufsErrno = UFS_BAD_CALL;
        return -1;
    }

    ufsSqlite = ufs;

    /* Query the db and get the identifier.                                   */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    sqlite3_bind_text(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            1, name, -1, SQLITE_TRANSIENT );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            2, parent );
    sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            3, UFS_STORAGE_TYPE_DIRECTORY );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );

    if ( res != SQLITE_ROW ) {
        ufsErrno = UFS_DOES_NOT_EXIST;
        return -1;
    }

    ufsErrno = UFS_NO_ERROR;
	return sqlite3_column_int( ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
                               0 );
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
