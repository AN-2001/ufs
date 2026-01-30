/******************************************************************************\
*  test_ufs_core.c                                                             *
*                                                                              *
*  Test suite for ufs core implementations.                                    *
*  Implementations that pass this are considered valid implementations of the  *
*  ufs core spec.                                                              *
*                                                                              *
*              Written by A.N.                                  18-01-2026     *
*                                                                              *
\******************************************************************************/


#define UFS_TESTING

#ifndef UFS_TEST_DISABLE

#include <memory.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "ufs_core.h"
#include "utils.h"

#include <cmocka.h>

#define TEST_DIRECTORY_NAME TEST_DIRECTORY_NAME_0
#define TEST_DIRECTORY_NAME_0 ("testDirectory0")
#define TEST_DIRECTORY_NAME_1 ("testDirectory1")

#define TEST_FILE_NAME TEST_FILE_NAME_0
#define TEST_FILE_NAME_0 ("testFile0")
#define TEST_FILE_NAME_1 ("testFile1")

#define TEST_AREA_NAME TEST_AREA_NAME_0
#define TEST_AREA_NAME_0 ("testArea0")
#define TEST_AREA_NAME_1 ("testArea1")

static void test_ufs_init( void **state )
{
    (void) state;

    ufsType ufs = ufsInit();

    assert_non_null( ufs );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

 
    ufsDestroy( ufs );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );
}

/* ufsAddStorage tests                                                        */
static void test_ufs_add_directory_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddStorage( NULL,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );

    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
                        -1,
                        UFS_STORAGE_TYPE_DIRECTORY,
                        TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_add_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0;

    ufsStruct = *state;

    id0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( id0 );
}

static void test_ufs_add_directory_duplicate( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( id );

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_ALREADY_EXISTS );
}

static void test_ufs_add_directory_parent_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0;

    ufsStruct = *state;

    id0 = ufsAddStorage( ufsStruct -> ufs,
            1,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id0, UFS_PARENT_DOES_NOT_EXIST );
}

static void test_ufs_add_file_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddStorage( NULL,
            1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
            -1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_add_file( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, dirId;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    id0 = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id0 );
}

static void test_ufs_add_file_parent_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddStorage( ufsStruct -> ufs,
            1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_PARENT_DOES_NOT_EXIST );
}

static void test_ufs_add_file_duplicate( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id, dirId;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    id = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id );

    id = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_ALREADY_EXISTS );

}

static void test_ufs_add_file_same_name_different_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1, dirId0, dirId1;

    ufsStruct = *state;

    dirId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_0 );
    ASSERT_UFS_NO_ERROR( dirId0 );

    dirId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_1 );
    ASSERT_UFS_NO_ERROR( dirId1 );

    id0 = ufsAddStorage( ufsStruct -> ufs,
            dirId0,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsAddStorage( ufsStruct -> ufs,
            dirId1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_not_equal( id0, id1 );
}

/* ########################################################################## */

/* ufsAddArea tests                                                           */
static void test_ufs_add_area_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddArea( NULL, TEST_AREA_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddArea( ufsStruct -> ufs, NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_add_area( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( id );
}

static void test_ufs_add_area_duplicate( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( id );

    id = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_ERROR( id, UFS_ALREADY_EXISTS );
}

static void test_ufs_add_area_illegal_name( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddArea( ufsStruct -> ufs, UFS_AREA_BASE_NAME );
    ASSERT_UFS_ERROR( id, UFS_ILLEGAL_NAME );
}
/* ########################################################################## */

/* ufsAddMapping tests                                                        */
static void test_ufs_add_mapping_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsAddMapping( NULL, 1, 1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsAddMapping( ufsStruct -> ufs, -1, 1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsAddMapping( ufsStruct -> ufs, 1, -1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

}

static void test_ufs_add_mapping_area_file( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId, dirId, fileId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

}

static void test_ufs_add_mapping_area_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId, dirId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

}

static void test_ufs_add_mapping_duplicate( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId, dirId, fileId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS( status, UFS_ALREADY_EXISTS );

}

static void test_ufs_add_mapping_area_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, dirId, fileId;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsAddMapping( ufsStruct -> ufs, 1, fileId );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_add_mapping_file_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, 1 );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsGetStorage                                                              */
static void test_ufs_get_directory_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;
    id = ufsGetStorage( NULL,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            -1,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_get_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1;

    ufsStruct = *state;
    id0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsGetStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_equal( id0, id1 );
}

static void test_ufs_get_directory_parent_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;
    id = ufsGetStorage( ufsStruct -> ufs,
            1,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );

    ASSERT_UFS_ERROR( id, UFS_PARENT_DOES_NOT_EXIST );
}

static void test_ufs_get_directory_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsGetStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
}

static void test_ufs_get_file_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;
    id = ufsGetStorage( NULL,
            1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            -1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            1,
            UFS_STORAGE_TYPE_FILE,
            NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_get_file( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, dirId, id1;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    id0 = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsGetStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_equal( id0, id1 );
}

static void test_ufs_get_file_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id, dirId;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    id = ufsGetStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
}

static void test_ufs_get_file_parent_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsGetStorage( ufsStruct -> ufs,
            1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_PARENT_DOES_NOT_EXIST );
}

static void test_ufs_get_file_exists_in_different_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1, dirId0, dirId1;

    ufsStruct = *state;

    dirId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_0 );
    ASSERT_UFS_NO_ERROR( dirId0 );

    dirId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_1 );
    ASSERT_UFS_NO_ERROR( dirId1 );

    id0 = ufsAddStorage( ufsStruct -> ufs,
            dirId0,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsGetStorage( ufsStruct -> ufs,
            dirId1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id1, UFS_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsGetArea                                                                 */
static void test_ufs_get_area_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsGetArea( NULL, TEST_AREA_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetArea( ufsStruct -> ufs, NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

}

static void test_ufs_get_area( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1;

    ufsStruct = *state;

    id0 = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsGetArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_equal( id0, id1 );
}

static void test_ufs_get_area_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsGetArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsProbeMapping                                                            */
static void test_ufs_probe_mapping_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsAddMapping( NULL, 1, 1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsAddMapping( ufsStruct -> ufs, -1, 1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsAddMapping( ufsStruct -> ufs, 1, -1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

}

static void test_ufs_probe_mapping( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId, fileId;
    ufsStatusType status0, status1;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status0 = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status0 );

    status1 = ufsProbeMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status1 );

}

static void test_ufs_probe_mapping_area_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsProbeMapping( ufsStruct -> ufs, 1, fileId );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_probe_mapping_file_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );


    status = ufsProbeMapping( ufsStruct -> ufs, areaId, 1 );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_probe_mapping_mapping_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsProbeMapping( ufsStruct -> ufs, 1, 1 );
    ASSERT_UFS_STATUS( status, UFS_MAPPING_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsRemoveStorage                                                           */
static void test_ufs_remove_directory_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsRemoveStorage( NULL, 1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsRemoveStorage( ufsStruct -> ufs, 0 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsRemoveStorage( ufsStruct -> ufs, -1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

}

static void test_ufs_remove_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;
    ufsStatusType status;

    ufsStruct = *state;

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( id );

    status = ufsRemoveStorage( ufsStruct -> ufs, id );
    ASSERT_UFS_STATUS_NO_ERROR( status );
}

static void test_ufs_remove_directory_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsRemoveStorage( ufsStruct -> ufs, 1 );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_remove_directory_contains_file( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );


    status = ufsRemoveStorage( ufsStruct -> ufs, dirId );
    ASSERT_UFS_STATUS( status, UFS_DIRECTORY_IS_NOT_EMPTY );

}

static void test_ufs_remove_directory_exists_in_mapping( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsRemoveStorage( ufsStruct -> ufs, dirId );
    ASSERT_UFS_STATUS( status, UFS_EXISTS_IN_EXPLICIT_MAPPING );

}

static void test_ufs_remove_directory_double_remove( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    status = ufsRemoveStorage( ufsStruct -> ufs, dirId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsRemoveStorage( ufsStruct -> ufs, dirId );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );
}

static void test_ufs_remove_directory_remove_then_add( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    status = ufsRemoveStorage( ufsStruct -> ufs, dirId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );
}

static void test_ufs_remove_directory_remove_then_get( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    status = ufsRemoveStorage( ufsStruct -> ufs, dirId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    dirId = ufsGetStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( dirId, UFS_DOES_NOT_EXIST );

}

static void test_ufs_remove_file_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsRemoveStorage( NULL, 1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsRemoveStorage( ufsStruct -> ufs, -1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

}

static void test_ufs_remove_file( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsRemoveStorage( ufsStruct -> ufs, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );
}

static void test_ufs_remove_file_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsRemoveStorage( ufsStruct -> ufs, 1 );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );
}

static void test_ufs_remove_file_exists_in_mapping( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsRemoveStorage( ufsStruct -> ufs, fileId );
    ASSERT_UFS_STATUS( status, UFS_EXISTS_IN_EXPLICIT_MAPPING );
}

static void test_ufs_remove_file_double_remove( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsRemoveStorage( ufsStruct -> ufs, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsRemoveStorage( ufsStruct -> ufs, fileId );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_remove_file_remove_then_add( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsRemoveStorage( ufsStruct -> ufs, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

}

static void test_ufs_remove_file_remove_then_get( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsRemoveStorage( ufsStruct -> ufs, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    fileId = ufsGetStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( fileId, UFS_DOES_NOT_EXIST );

}
/* ########################################################################## */

/* ufsRemoveArea                                                              */
static void test_ufs_remove_area_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsRemoveArea( NULL, 1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsRemoveArea( ufsStruct -> ufs, 0 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsRemoveArea( ufsStruct -> ufs, -1 );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

}

static void test_ufs_remove_area( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsRemoveArea( ufsStruct -> ufs, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );
}

static void test_ufs_remove_area_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsRemoveArea( ufsStruct -> ufs, 1 );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_remove_area_exists_in_mapping( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsRemoveArea( ufsStruct -> ufs, areaId );
    ASSERT_UFS_STATUS( status, UFS_EXISTS_IN_EXPLICIT_MAPPING );

}

static void test_ufs_remove_area_double_remove( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsRemoveArea( ufsStruct -> ufs, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsRemoveArea( ufsStruct -> ufs, areaId );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_remove_area_remove_then_add( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsRemoveArea( ufsStruct -> ufs, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

}

static void test_ufs_remove_area_remove_then_get( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsRemoveArea( ufsStruct -> ufs, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    areaId = ufsGetArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_ERROR( areaId, UFS_DOES_NOT_EXIST );

}
/* ########################################################################## */

/* ufsRemoveMapping                                                           */
static void test_ufs_remove_mapping_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsRemoveMapping( NULL, 1, 1);
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsRemoveMapping( ufsStruct -> ufs, -1, 1);
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsRemoveMapping( ufsStruct -> ufs, 1, -1);
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

}

static void test_ufs_remove_mapping( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, dirId, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );


    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, dirId);
    ASSERT_UFS_STATUS_NO_ERROR( status );
}

static void test_ufs_remove_mapping_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;

    ufsStruct = *state;

    status = ufsRemoveMapping( ufsStruct -> ufs, 1, 1 );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_remove_mapping_no_side_effects( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId0, areaId1, dirId0, dirId1;
    ufsStatusType status;

    ufsStruct = *state;

    dirId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId0 );

    areaId0 = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId0 );

    status = ufsAddMapping( ufsStruct -> ufs, dirId0, areaId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );


    status = ufsRemoveMapping( ufsStruct -> ufs, areaId0, dirId0);
    ASSERT_UFS_STATUS_NO_ERROR( status );

    areaId1 = ufsGetArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId1 );

    assert_int_equal( areaId0, areaId1 );

    dirId1 = ufsGetArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( dirId1 );

    assert_int_equal( dirId0, dirId1 );

}

static void test_ufs_remove_mapping_double_remove( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, dirId, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );


    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, dirId);
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, dirId);
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_remove_mapping_remove_then_add( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, dirId, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );


    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, dirId);
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, dirId, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

}

static void test_ufs_remove_mapping_remove_then_probe( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, dirId, areaId );
    ASSERT_UFS_STATUS_NO_ERROR( status );


    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, dirId);
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsProbeMapping( ufsStruct -> ufs, dirId, areaId );
    ASSERT_UFS_STATUS( status, UFS_MAPPING_DOES_NOT_EXIST );

}
/* ########################################################################## */

static const struct CMUnitTest ufs_test_suite[] = {

    cmocka_unit_test( test_ufs_init ),

    /* ufsAddStore tests.                                                     */
    cmocka_unit_test_setup_teardown( test_ufs_add_directory_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_directory, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_directory_duplicate, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_directory_parent_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_file_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_file, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_file_duplicate, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_file_parent_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_file_same_name_different_directory, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsAddArea tests.                                                      */
    cmocka_unit_test_setup_teardown( test_ufs_add_area_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_area, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_area_duplicate, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_area_illegal_name, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsAddMapping tests.                                                   */
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_area_file, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_area_directory, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_duplicate, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_area_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_file_does_not_exist, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsGetStorage tests.                                                   */
    cmocka_unit_test_setup_teardown( test_ufs_get_directory_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_directory, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_directory_parent_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_directory_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file_parent_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file_exists_in_different_directory, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsGetArea tests.                                                      */
    cmocka_unit_test_setup_teardown( test_ufs_get_area_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_area, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_area_does_not_exist, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsProbeMapping tests.                                                 */
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping_area_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping_file_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping_mapping_does_not_exist, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsRemoveStorage tests.                                                */
    cmocka_unit_test_setup_teardown( test_ufs_remove_directory_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_directory, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_directory_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_directory_contains_file, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_directory_exists_in_mapping, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_directory_double_remove, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_directory_remove_then_add, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_directory_remove_then_get, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_file_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_file, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_file_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_file_exists_in_mapping, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_file_double_remove, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_file_remove_then_add, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_file_remove_then_get, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsRemoveArea tests.                                                   */
    cmocka_unit_test_setup_teardown( test_ufs_remove_area_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_area, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_area_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_area_exists_in_mapping, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_area_double_remove, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_area_remove_then_add, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_area_remove_then_get, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsRemoveMapping tests.                                                */
    cmocka_unit_test_setup_teardown( test_ufs_remove_mapping_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_mapping, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_mapping_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_mapping_no_side_effects, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_mapping_double_remove, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_mapping_remove_then_add, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_remove_mapping_remove_then_probe, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */
};

int main( void ) {
    return cmocka_run_group_tests( ufs_test_suite, NULL, NULL );
}

#else

int main( void ) {
    return 0;
}

#endif /* UFS_TEST_DISABLE */
