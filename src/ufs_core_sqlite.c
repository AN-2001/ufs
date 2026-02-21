/******************************************************************************\
*  ufs_core_sqlite.c                                                           *
*                                                                              *
*  Sqlite implementation of ufs_core.                                          *
*                                                                              *
*              Written by A.N.                                  24-01-2026     *
*                                                                              *
\******************************************************************************/


#include "sqlite3.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ufs_core.h"
#include "ufs_utils.h"

#define UFS_SQLITE_BUFF_SIZE (4096)
#define UFS_SQLITE_BUFF_SIZE_BIG (4096 * 2)

enum ufsSqliteStatementType {
    UFS_STATEMENT_INSERT_INTO_STORAGE,
    UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE,
    UFS_STATEMENT_QUERY_STORAGE_BY_ID,
    UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE,
    UFS_STATEMENT_QUERY_STORAGE_ID_TYPE_BY_ID,
    UFS_STATEMENT_QUERY_STORAGE_BY_PARENT,
    UFS_STATEMENT_REMOVE_STORAGE_BY_ID,   
    UFS_STATEMENT_INSERT_INTO_AREAS,
    UFS_STATEMENT_QUERY_AREAS_BY_NAME,
    UFS_STATEMENT_QUERY_AREAS_BY_ID,
    UFS_STATEMENT_REMOVE_AREA_BY_ID,   
    UFS_STATEMENT_INSERT_INTO_MAPPINGS,
    UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS,
    UFS_STATEMENT_QUERY_MAPPINGS_BY_STORAGE_ID,
    UFS_STATEMENT_QUERY_MAPPINGS_BY_AREA_ID,
    UFS_STATEMENT_REMOVE_MAPPINGS_BY_IDS,
    UFS_STATEMENT_COMPOSITE_FIND_CHILD_MAPPINGS,
    NUM_UFS_STATEMENTS,
};

typedef struct ufsSqliteStruct {
    sqlite3 *db;
    ufsIdentifierType rootId;
    sqlite3_stmt *statements[ NUM_UFS_STATEMENTS ];

} ufsSqliteStruct;

static const char *UFS_SQL_TEXT[ NUM_UFS_STATEMENTS + 2 ] = {

    /* Schema command:                                                        */
    "create table if not exists ufsstorage(id integer primary key,"
                                          "name text not null,"
                                          "parent integer,"
                                          "type integer );"
    "create table if not exists ufsareas(id integer primary key,"
                                         "name text not null );"
    "create table if not exists ufsmappings(id integer primary key,"
                                           "areaid integer,"
                                           "storageid integer,"
                                           "foreign key (areaid) references ufsareas(id),"
                                           "foreign key (storageid) references ufsstorage(id) );"
    ,


    /* STORAGE STATEMENTS:                                                    */
    /* Insert into the storage table:                                         */
    "INSERT INTO ufsStorage (name, parent, type) VALUES (?, ?, ?);",

    /* Query storage by name, parent, type:                                   */
    "SELECT id from ufsStorage where name = ? and parent = ? and type = ?;",

    /* Query storage by id:                                                   */
    "SELECT id, parent from ufsStorage where id = ?;",

    /* Query storage by id, type:                                             */
    "SELECT id from ufsStorage where id = ? and type = ?;",

    /* Query storage id, type by id:                                          */
    "SELECT id, type from ufsStorage where id = ?;",

    /* Query storage by parent:                                               */
    "SELECT id from ufsStorage where parent = ?;",

    /* Remove storage by id:                                                  */
    "DELETE from ufsStorage where id = ?;",

    /* AREA STATEMENTS:                                                       */
    /* Insert into the area table:                                            */
    "INSERT INTO ufsAreas (name) VALUES (?);",

    /* Query areas by name:                                                   */
    "SELECT id from ufsAreas where name = ?;",

    /* Query areas by id:                                                     */
    "SELECT id from ufsAreas where id = ?;",

    /* Remove area by id:                                                     */
    "DELETE from ufsAreas where id = ?;",

    /* MAPPING STATEMENTS:                                                    */
    /* Insert into mappings:                                                  */
    "INSERT INTO ufsMappings (areaId, storageId) VALUES (?, ?);",

    /* Query mappings by IDs:                                                 */
    "SELECT id from ufsMappings where areaId = ? and storageId = ?;",

    /* Query mappings by storage ID:                                          */
    "SELECT id from ufsMappings where storageId = ?;",

    /* Query mappings by area ID:                                             */
    "SELECT id from ufsMappings where areaId = ?;",

    /* Query mappings by area ID:                                             */
    "DELETE from ufsMappings where areaId = ? and storageId = ?;",

    /* Given a (area, directory) mapping this query will find all the         */
    /* (area, storage) mappings where area is the same, and storage is a chi- */
    /* ld of directory.                                                       */
    "SELECT ufsMappings.areaid, ufsStorage.id "
    "FROM ufsStorage "
    "JOIN ufsMappings "
    "ON ufsStorage.id = ufsMappings.storageid "
    "WHERE ufsMappings.areaid = ? "
    "AND ufsStorage.parent = ? " 
    "LIMIT 1;",

    NULL
};

static const char * const VIEW_RESOLVER_QUERY = 
    "WITH viewData(areaid, viewOrder) AS ( VALUES %s ) "
    "SELECT ufsMappings.areaid "
    "FROM ufsMappings "
    "JOIN viewData ON viewData.areaid = ufsMappings.areaid "
    "WHERE ufsMappings.storageid = ? "
    "ORDER BY viewData.viewOrder "
    "LIMIT 1;";

static const char * const DIR_ITERATOR_QUERY = 
    "WITH viewData(areaid, viewOrder) AS ( VALUES %s ) "
    "SELECT DISTINCT ufsStorage.id, ufsStorage.name, COUNT(*) OVER() AS total_count "
    "FROM ufsStorage "
    "JOIN ufsMappings "
    "ON ufsMappings.storageid = ufsStorage.id "
    "JOIN viewData "
    "ON ufsMappings.areaid = viewData.areaid "
    "WHERE ufsStorage.parent = ?;";

struct ufsSqliteStruct *prepareSqliteDb( sqlite3 *db, ufsStatusType *statusNo );


static inline bool validateViewStructure( ufsViewType view, uint64_t *viewSize );
static inline bool validateViewSemantics( ufsSqliteStruct *ufsSqlite,
                                          ufsViewType view,
                                          uint64_t viewSize );
static inline void buildViewString( ufsViewType view,
                                    uint64_t viewSize,
                                    char buff[ UFS_SQLITE_BUFF_SIZE ] );

static inline int identifierComparator( const void *a, const void *b )
{
    const ufsIdentifierType 
        *n1 = a,
        *n2 = b;
    return ( *n1 > *n2 )  - ( *n2 > *n1 );
}

/* Makes sure that this view is in correct syntax.                            */
bool validateViewStructure( ufsViewType view, uint64_t *viewSize )
{
    ufsViewType copyView;
    uint64_t i;

    for ( *viewSize = 0;
          view[ *viewSize ] != UFS_VIEW_TERMINATOR &&
          *viewSize < UFS_VIEW_MAX_SIZE;
          ++(*viewSize) );
    if ( !*viewSize )
        return true;

    /* Validate two things:                                                   */
    /* * If BASE exists it must be at the end.                                */
    /* * All areas are greater or equal than 0.                               */
    for (i = 0; i < *viewSize; i++ ) {
        if ( view[ i ] < 0 )
            return false;
        if ( view[ i ] == UFS_AREA_BASE_IDENTIFIER && i != ( *viewSize - 1 ))
            return false;
    }

    /* Now check for duplicates, we'll do it in O( n log n ) for now.         */
    memcpy( copyView, view, sizeof( view[ 0 ] ) * *viewSize );

    qsort( copyView, *viewSize, sizeof( copyView[0] ), identifierComparator );

    for ( i = 0; i < *viewSize - 1; i++ )
        if ( copyView[ i ] == copyView[ i + 1 ] )
            return false;

    return true;
}

static inline void buildViewString( ufsViewType view,
                                    uint64_t viewSize,
                                    char buff[ UFS_SQLITE_BUFF_SIZE ] )
{
    uint64_t i;
    char *ptr;

    ptr = buff;

    for (i = 0; i < viewSize - 1; i++)
        ptr += sprintf( ptr, "( %ld, %ld ),", view[ i ], i + 1 );
    ptr += sprintf( ptr, "( %ld, %ld )", view[ i ], i + 1 );
}


/* Note: this function that view is in correct syntax.                        */
bool validateViewSemantics( ufsSqliteStruct *ufsSqlite,
                            ufsViewType view,
                            uint64_t viewSize )
{
    int res;
    uint64_t i;
    if (!viewSize)
        return true;
    /* Validate that all areas inside the view exist inside ufs.              */
    for ( i = 0; i < viewSize; i++ ) {
        if ( view[ i ] == UFS_AREA_BASE_IDENTIFIER )
            break; /* We know BASE would be at the end.                       */

        sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_ID ] );
        sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_ID ] );
        sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_ID ],
            1, view[ i ] );
        res = sqlite3_step(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_ID ] );
        if ( res != SQLITE_ROW ) {
            return false;
        }
    }

    return true;
}

struct ufsSqliteStruct *prepareSqliteDb( sqlite3 *db, ufsStatusType *statusNo )
{
    int res, i;
    ufsSqliteStruct *ufsSqlite;

    ufsSqlite = malloc( sizeof( *ufsSqlite ) );
    if ( !ufsSqlite ) {
        SET_STATUS( UFS_OUT_OF_MEMORY );
        return NULL;
    }

    ufsSqlite -> db = db;
    res = sqlite3_exec( db, UFS_SQL_TEXT[ 0 ], NULL, NULL, NULL );
    if ( res != SQLITE_OK ) {
        free( ufsSqlite );
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return NULL;
    }

    for ( i = 1; UFS_SQL_TEXT[ i ]; ++i ) {
        res = sqlite3_prepare_v2( db,
                                  UFS_SQL_TEXT[ i ],
                                  -1,
                                  &ufsSqlite -> statements[ i - 1 ],
                                  NULL );
        if (res != SQLITE_OK) {
               fprintf(stderr,
        "sqlite3_prepare_v2 failed: %s\n",
        sqlite3_errmsg(db) );
            free( ufsSqlite );
            SET_STATUS( UFS_UNKNOWN_ERROR );
            return NULL;
        }
    }

    SET_STATUS( UFS_NO_ERROR );
    return ufsSqlite;
}

ufsType ufsInit( ufsStatusType *statusNo )
{
    ufsSqliteStruct *ret;
    sqlite3 *db;
    int res;

    res = sqlite3_open( ":memory:", &db );
    if ( !db ) {
        SET_STATUS( UFS_OUT_OF_MEMORY );
        return NULL;
    }

    if ( res != SQLITE_OK ) {
        sqlite3_close( db );
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return NULL;
    }

    ret = prepareSqliteDb( db, statusNo );
    if ( !ret ) {
        sqlite3_close( db );
        return NULL;
    }

    SET_STATUS( UFS_NO_ERROR );
    return ret;
}

void ufsDestroy( ufsType ufs, ufsStatusType *statusNo )
{
    int i;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs ) {
        SET_STATUS( UFS_NO_ERROR );
        return;
    }

    ufsSqlite = ufs;
    for (i = 0; i < NUM_UFS_STATEMENTS; i++)
        sqlite3_finalize( ufsSqlite -> statements[ i ] );
    sqlite3_close( ufsSqlite -> db );
    free( ufsSqlite );
    SET_STATUS( UFS_NO_ERROR );
}

ufsIdentifierType ufsAddStorage( ufsType ufs,
                                 ufsIdentifierType parent,
                                 ufsStorageTypeEnum type,
                                 const char *name,
                                 ufsStatusType *statusNo )
{
    ufsSqliteStruct *ufsSqlite;
    int res;
    if ( !ufs || parent < 0 || !name ||
         type < 0 || type >= UFS_STORAGE_TOTAL_TYPES ) {
        SET_STATUS( UFS_BAD_CALL );
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
            SET_STATUS( UFS_PARENT_DOES_NOT_EXIST );
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
        3, type );
    res = sqlite3_step(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );
    if ( res == SQLITE_ROW ) {
        SET_STATUS( UFS_ALREADY_EXISTS );
        return -1;
    }

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return -1;
    }

    /* Finally, insert the storage into the db.                               */
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
            3, type );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_INSERT_INTO_STORAGE ] );
    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return -1;
    }

    SET_STATUS( UFS_NO_ERROR );
    return sqlite3_last_insert_rowid( ufsSqlite -> db );
}

ufsIdentifierType ufsAddArea( ufsType ufs,
                              const char *name,
                              ufsStatusType *statusNo )
{
    ufsSqliteStruct *ufsSqlite;
    int res;
    if ( !ufs || !name ) {
        SET_STATUS( UFS_BAD_CALL );
        return -1;
    }

    if (strncmp( name,
                UFS_AREA_BASE_NAME,
                sizeof( UFS_AREA_BASE_NAME ) /
                sizeof( char )) == 0 ) {
        SET_STATUS( UFS_ILLEGAL_NAME );
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
        SET_STATUS( UFS_ALREADY_EXISTS );
        return -1;
    }

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
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
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return -1;
    }

    SET_STATUS( UFS_NO_ERROR );
    return sqlite3_last_insert_rowid( ufsSqlite -> db );
}

bool ufsAddMapping( ufsType ufs,
                             ufsIdentifierType area,
                             ufsIdentifierType storage,
                             ufsStatusType *statusNo )
{
    int res;
    ufsSqliteStruct *ufsSqlite;
    ufsIdentifierType parent;
    if ( !ufs || area <= 0 || storage < 0 ) {
        SET_STATUS( UFS_BAD_CALL );
        return false;
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
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return false;
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
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return false;
    }

    parent = sqlite3_column_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID ],
            1);

    /* verify that the parent is mapped if it's not root.                     */
    if ( parent != UFS_STORAGE_ROOT_IDENTIFIER ) {
        sqlite3_reset(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ] );
        sqlite3_clear_bindings(
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ] );
        sqlite3_bind_int( 
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ],
                1, area );
        sqlite3_bind_int( 
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ],
                2, parent );
        res = sqlite3_step( 
                ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_IDS ] );
        if ( res != SQLITE_ROW ) {
            SET_STATUS( UFS_PARENT_IS_NOT_MAPPED );
            return false;
        }
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
        SET_STATUS( UFS_ALREADY_EXISTS );
        return false;
    }

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return false;
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
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return false;
    }


    SET_STATUS( UFS_NO_ERROR );
	return true;
}

ufsIdentifierType ufsGetStorage( ufsType ufs,
                                 ufsIdentifierType parent,
                                 ufsStorageTypeEnum type,
                                 const char *name,
                                 ufsStatusType *statusNo )
{
    int res;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs || parent < 0 ||
         !name || type < 0 || type >= UFS_STORAGE_TOTAL_TYPES ) {
        SET_STATUS( UFS_BAD_CALL );
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
        3, type );
    res = sqlite3_step(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ] );

    if ( res != SQLITE_ROW ) {
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return -1;
    }

    SET_STATUS( UFS_NO_ERROR );
	return sqlite3_column_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_NAME_TYPE ],
            0 );
}

ufsIdentifierType ufsGetArea( ufsType ufs,
                              const char *name,
                              ufsStatusType *statusNo )
{
    int res;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs || !name ) {
        SET_STATUS( UFS_BAD_CALL );
        return -1;
    }

    /* BASE is defined to have identifier 0.                                  */
    if (strncmp( name, UFS_AREA_BASE_NAME,
                sizeof( UFS_AREA_BASE_NAME )) == 0) {
        SET_STATUS( UFS_NO_ERROR );
        return UFS_AREA_BASE_IDENTIFIER;
    }

    ufsSqlite = ufs;

    /* Query the db and get the identifier.                                   */
    sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ] );
    sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ] );
    sqlite3_bind_text(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ],
            1, name, -1, SQLITE_TRANSIENT );
    res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ] );

    if ( res != SQLITE_ROW ) {
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return -1;
    }

    SET_STATUS( UFS_NO_ERROR );
	return sqlite3_column_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_AREAS_BY_NAME ],
            0 );
}

bool ufsProbeMapping( ufsType ufs,
                      ufsIdentifierType area,
                      ufsIdentifierType storage,
                      ufsStatusType *statusNo )
{
    int res;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs || area < 0 || storage < 0 ) {
        SET_STATUS( UFS_BAD_CALL );
        return false;
    }

    ufsSqlite = ufs;

    /* Query the db and get the identifier.                                   */
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

    if ( res != SQLITE_ROW ) {
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return false;
    }

    SET_STATUS( UFS_NO_ERROR );
	return true;
}

bool ufsRemoveStorage( ufsType ufs,
                       ufsIdentifierType identifier,
                       ufsStatusType *statusNo )
{
    int res, type;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs || identifier <= 0 ) {
        SET_STATUS( UFS_BAD_CALL );
        return false;
    }

    ufsSqlite = ufs;

    /* First query the storage make sure it exists, and fetch its type.       */
    sqlite3_reset(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_ID_TYPE_BY_ID ] );
    sqlite3_clear_bindings(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_ID_TYPE_BY_ID ] );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_ID_TYPE_BY_ID ],
        1, identifier );
    res = sqlite3_step(
         ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_ID_TYPE_BY_ID ] );

    if ( res != SQLITE_ROW ) {
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return false;
    }

    type = sqlite3_column_int( 
         ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_ID_TYPE_BY_ID ],
         1 );

    /* If it's a directory, make sure it doesn't contain anything.            */
    if ( type == UFS_STORAGE_TYPE_DIRECTORY ) {

        sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_PARENT ] );
        sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_PARENT ] );
        sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_PARENT ],
            1, identifier );
        res = sqlite3_step(
             ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_PARENT ] );

        if ( res != SQLITE_DONE ) {
            SET_STATUS( UFS_DIRECTORY_IS_NOT_EMPTY );
            return false;
        }
    }

    /* Now make sure this storage does not exist in a mapping.                */
    sqlite3_reset(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_STORAGE_ID ] );
    sqlite3_clear_bindings(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_STORAGE_ID ] );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_STORAGE_ID ],
        1, identifier );
    res = sqlite3_step(
         ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_STORAGE_ID ] );

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_EXISTS_IN_EXPLICIT_MAPPING );
        return false;
    }

    /* We can now safely remove this storage.                                 */
    sqlite3_reset(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_STORAGE_BY_ID ] );
    sqlite3_clear_bindings(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_STORAGE_BY_ID ] );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_STORAGE_BY_ID ],
        1, identifier );
    res = sqlite3_step(
         ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_STORAGE_BY_ID ] );

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return false;
    }

    SET_STATUS( UFS_NO_ERROR );
	return true;
}

bool ufsRemoveArea( ufsType ufs,
                    ufsIdentifierType area,
                    ufsStatusType *statusNo )
{
    int res;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs || area <= 0 ) {
        SET_STATUS( UFS_BAD_CALL );
        return false;

    }
    ufsSqlite = ufs;

    /* First query the area make sure it exists.                              */
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
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return false;
    }

    /* Now make sure this area does not exist in a mapping.                   */
    sqlite3_reset(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_AREA_ID ] );
    sqlite3_clear_bindings(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_AREA_ID ] );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_AREA_ID ],
        1, area );
    res = sqlite3_step(
         ufsSqlite -> statements[ UFS_STATEMENT_QUERY_MAPPINGS_BY_AREA_ID ] );

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_EXISTS_IN_EXPLICIT_MAPPING );
        return false;
    }

    /* We can now safely remove this area.                                    */
    sqlite3_reset(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_AREA_BY_ID ] );
    sqlite3_clear_bindings(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_AREA_BY_ID ] );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_AREA_BY_ID ],
        1, area );
    res = sqlite3_step(
         ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_AREA_BY_ID ] );

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return false;
    }

    SET_STATUS( UFS_NO_ERROR );
	return true;
}

bool ufsRemoveMapping( ufsType ufs,
                       ufsIdentifierType area,
                       ufsIdentifierType storage,
                       ufsStatusType *statusNo )
{
    int res;
    ufsSqliteStruct *ufsSqlite;
    if ( !ufs || area <= 0 || storage <= 0 ) {
        SET_STATUS( UFS_BAD_CALL );
        return false;
    }

    ufsSqlite = ufs;

    /* First query the area make sure it exists.                              */
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

    if ( res != SQLITE_ROW ) {
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return false;
    }

    /* Check if one of the children of this directory is mapped to this area. */
    sqlite3_reset(
        ufsSqlite -> statements[ UFS_STATEMENT_COMPOSITE_FIND_CHILD_MAPPINGS ] );
    sqlite3_clear_bindings(
        ufsSqlite -> statements[ UFS_STATEMENT_COMPOSITE_FIND_CHILD_MAPPINGS ] );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_COMPOSITE_FIND_CHILD_MAPPINGS ],
        1, area );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_COMPOSITE_FIND_CHILD_MAPPINGS ],
        2, storage );
    res = sqlite3_step(
         ufsSqlite -> statements[ UFS_STATEMENT_COMPOSITE_FIND_CHILD_MAPPINGS ] );

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_CHILD_EXISTS_IN_EXPLICIT_MAPPING );
        return false;
    }

    /* We can just remove, no need to perform any more checks.                */
    sqlite3_reset(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_MAPPINGS_BY_IDS ] );
    sqlite3_clear_bindings(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_MAPPINGS_BY_IDS ] );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_MAPPINGS_BY_IDS ],
        1, area );
    sqlite3_bind_int(
        ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_MAPPINGS_BY_IDS ],
        2, storage );
    res = sqlite3_step(
         ufsSqlite -> statements[ UFS_STATEMENT_REMOVE_MAPPINGS_BY_IDS ] );

    if ( res != SQLITE_DONE ) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return false;
    }

    SET_STATUS( UFS_NO_ERROR );
	return true;
}

ufsIdentifierType ufsResolveStorageInView( ufsType ufs,
                                           ufsViewType view,
                                           ufsIdentifierType storage,
                                           ufsStatusType *statusNo )
{
    char areaList[ UFS_SQLITE_BUFF_SIZE ], query[ UFS_SQLITE_BUFF_SIZE_BIG ];
    int res;
    sqlite3_stmt *statement;
    ufsSqliteStruct *ufsSqlite;
    uint64_t viewSize;
    if ( !ufs || !validateViewStructure( view, &viewSize ) || storage < 0 ) {
        SET_STATUS( UFS_BAD_CALL );
        return -1;
    }

    ufsSqlite = ufs;

    if ( !validateViewSemantics( ufsSqlite, view, viewSize ) ) {
        SET_STATUS( UFS_INVALID_AREA_IN_VIEW );
        return -1;
    }

    if ( !viewSize ) {
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return -1;
    }

    if ( storage == UFS_STORAGE_ROOT_IDENTIFIER ) {
        SET_STATUS( UFS_NO_ERROR );
        return view[ 0 ];
    }

    buildViewString( view, viewSize, areaList );
    sprintf( query, VIEW_RESOLVER_QUERY, areaList );

    res = sqlite3_prepare_v2( ufsSqlite -> db,
                              query,
                              -1,
                              &statement,
                              NULL );
    if (res != SQLITE_OK) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return -1;
    }
        

    sqlite3_bind_int( statement, 1, storage );

    res = sqlite3_step( statement );

    if ( res != SQLITE_ROW ) {
        sqlite3_finalize( statement );

        if ( view[ viewSize - 1 ] == UFS_AREA_BASE_IDENTIFIER ) {
            SET_STATUS( UFS_CHECK_BASE );
            return -1;
        }
        SET_STATUS( UFS_DOES_NOT_EXIST );
        return -1;
    }

    SET_STATUS( UFS_NO_ERROR );
    res = sqlite3_column_int( statement, 0 );
    sqlite3_finalize( statement );
	return res;
}

bool ufsIterateDirInView( ufsType ufs,
                          ufsViewType view,
                          ufsIdentifierType directory,
                          ufsDirIter iterator,
                          void *userData,
                          ufsStatusType *statusNo )
{
    char areaList[ UFS_SQLITE_BUFF_SIZE ], query[ UFS_SQLITE_BUFF_SIZE_BIG ];
    int res;
    ufsSqliteStruct *ufsSqlite;
    sqlite3_stmt *statement;
    const char *name;
    ufsIdentifierType storage;
    ufsStatusType status;
    uint64_t viewSize, total, i;
    if ( !ufs || !validateViewStructure( view, &viewSize ) ||
            directory < 0 || !iterator ) {
        SET_STATUS( UFS_BAD_CALL );
        return false;
    }

    ufsSqlite = ufs;

    if ( !validateViewSemantics( ufsSqlite, view, viewSize ) ){
        SET_STATUS( UFS_INVALID_AREA_IN_VIEW );
        return false;
    }

    /* First check if the directory exists if it's not root.                  */
    if ( directory != UFS_STORAGE_ROOT_IDENTIFIER ) {
        sqlite3_reset(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        sqlite3_clear_bindings(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ],
            1, directory );
        sqlite3_bind_int(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ],
            2, UFS_STORAGE_TYPE_DIRECTORY );
        res = sqlite3_step(
            ufsSqlite -> statements[ UFS_STATEMENT_QUERY_STORAGE_BY_ID_TYPE ] );
        if ( res != SQLITE_ROW ) {
            SET_STATUS( UFS_DOES_NOT_EXIST );
            return false;
        }
    }

    /* Next, if the view is only BASE don't query, just return.               */
    if ( !viewSize || view[ 0 ] == UFS_AREA_BASE_IDENTIFIER ) {
        SET_STATUS( UFS_NO_ERROR );
        return true;
    }

    /* Next do the actual union query.                                        */
    buildViewString( view, viewSize, areaList );
    sprintf( query, DIR_ITERATOR_QUERY, areaList );

    res = sqlite3_prepare_v2( ufsSqlite -> db,
                              query,
                              -1,
                              &statement,
                              NULL );
    if (res != SQLITE_OK ) {
        SET_STATUS( UFS_UNKNOWN_ERROR );
        return false;
    }

    sqlite3_bind_int( statement, 1, directory );
    res = sqlite3_step( statement );
    if ( res != SQLITE_ROW ) {
        sqlite3_finalize( statement );
        SET_STATUS( UFS_NO_ERROR );
        return true;
    }

    i = 0;
    total = sqlite3_column_int( statement, 2 );

    do {
        storage = sqlite3_column_int( statement, 0 );
        name = (const char *)sqlite3_column_text( statement, 1 );

        if ( ( status = iterator( storage, name, i, total, userData ) ) != UFS_NO_ERROR ) {
            sqlite3_finalize( statement );
            SET_STATUS( status );
            return false;
        }
        
        res = sqlite3_step( statement );
        i++;
    } while ( i < total && res == SQLITE_ROW );

    sqlite3_finalize( statement );
    SET_STATUS( UFS_NO_ERROR );
	return true;
}
