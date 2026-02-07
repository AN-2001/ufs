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


#include <stdint.h>
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

#define UFS_TEST( name ) \
    cmocka_unit_test_setup_teardown( name, ufsGetInstance, ufsCleanup )

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
static void test_ufs_add_storage_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddStorage( NULL,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( NULL,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
                        -1,
                        UFS_STORAGE_TYPE_DIRECTORY,
                        TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
                        -1,
                        UFS_STORAGE_TYPE_FILE,
                        TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            -1,
            NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
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

static void test_ufs_add_directory_non_root( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, dirId;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_0);
    ASSERT_UFS_NO_ERROR( dirId );

    id0 = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_1 );
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

static void test_ufs_add_directory_duplicate_non_root( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id, dirId;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
                        UFS_STORAGE_ROOT_IDENTIFIER,
                        UFS_STORAGE_TYPE_DIRECTORY,
                        TEST_DIRECTORY_NAME_0 );

    id = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_1 );
    ASSERT_UFS_NO_ERROR( id );

    id = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_1 );
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

static void test_ufs_add_directory_same_name_different_directory( void **state )
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
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsAddStorage( ufsStruct -> ufs,
            dirId1,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_not_equal( id0, id1 );
}

static void test_ufs_add_directory_same_name_different_directory_one_root( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1, dirId;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_0 );
    ASSERT_UFS_NO_ERROR( dirId );

    id0 = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_not_equal( id0, id1 );
}

static void test_ufs_add_directory_use_file_as_parent( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId, dirId;

    ufsStruct = *state;

    fileId = ufsAddStorage( ufsStruct -> ufs,
              UFS_STORAGE_ROOT_IDENTIFIER,
              UFS_STORAGE_TYPE_FILE,
              TEST_FILE_NAME_0);
    ASSERT_UFS_NO_ERROR( fileId );

    dirId = ufsAddStorage( ufsStruct -> ufs,
              fileId,
              UFS_STORAGE_TYPE_DIRECTORY,
              TEST_DIRECTORY_NAME);
    ASSERT_UFS_ERROR( dirId, UFS_PARENT_DOES_NOT_EXIST );
}

static void test_ufs_add_file( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0;

    ufsStruct = *state;

    id0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id0 );
}

static void test_ufs_add_file_non_root( void **state )
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
            UFS_STORAGE_TYPE_FILE,
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
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id );

    id = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_ALREADY_EXISTS );
}

static void test_ufs_add_file_duplicate_non_root( void **state )
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

static void test_ufs_add_file_same_name_different_directory_one_root( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1, dirId;

    ufsStruct = *state;

    dirId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME_0 );
    ASSERT_UFS_NO_ERROR( dirId );

    id0 = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_not_equal( id0, id1 );
}

static void test_ufs_add_file_use_file_as_parent( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId0, fileId1;

    ufsStruct = *state;

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
              UFS_STORAGE_ROOT_IDENTIFIER,
              UFS_STORAGE_TYPE_FILE,
              TEST_FILE_NAME_0);
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
              fileId0,
              UFS_STORAGE_TYPE_FILE,
              TEST_FILE_NAME_1);
    ASSERT_UFS_ERROR( fileId1, UFS_PARENT_DOES_NOT_EXIST );

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
    ufsStatusType status, areaId, fileId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
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
    ufsStatusType status, areaId, fileId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
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

static void test_ufs_add_mapping_parent_is_not_mapped( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId, fileId, dirId0, dirId1;

    ufsStruct = *state;

    dirId0 = ufsAddStorage( ufsStruct -> ufs, 
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );

    ASSERT_UFS_NO_ERROR( dirId0 );

    fileId = ufsAddStorage( ufsStruct -> ufs, 
            dirId0,
            UFS_STORAGE_TYPE_FILE,
            TEST_DIRECTORY_NAME );

    ASSERT_UFS_NO_ERROR( fileId );

    dirId1 = ufsAddStorage( ufsStruct -> ufs, 
            dirId0,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );

    ASSERT_UFS_NO_ERROR( dirId1 );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );

    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS( status, UFS_PARENT_IS_NOT_MAPPED );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId1 );
    ASSERT_UFS_STATUS( status, UFS_PARENT_IS_NOT_MAPPED );
}
/* ########################################################################## */

/* ufsGetStorage                                                              */
static void test_ufs_get_storage_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;
    id = ufsGetStorage( NULL,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( NULL,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            -1,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            -1,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            -1,
            TEST_FILE_NAME );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
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

    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
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
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
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
    ufsIdentifierType areaId, fileId;
    ufsStatusType status0, status1;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
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
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsRemoveStorage                                                           */
static void test_ufs_remove_storage_bad_args( void **state )
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
    ufsIdentifierType areaId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
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
    ufsIdentifierType areaId, fileId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
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

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId );
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

    status = ufsAddMapping( ufsStruct -> ufs, areaId0, dirId0 );
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

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId );
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

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId );
    ASSERT_UFS_STATUS_NO_ERROR( status );


    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, dirId);
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId );
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

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId );
    ASSERT_UFS_STATUS_NO_ERROR( status );


    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, dirId);
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsProbeMapping( ufsStruct -> ufs, areaId, dirId );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_remove_mapping_child_is_mapped( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType dirId0, dirId1, areaId;
    ufsStatusType status;

    ufsStruct = *state;

    dirId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId0 );

    dirId1 = ufsAddStorage( ufsStruct -> ufs,
            dirId0,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId1 );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, dirId0 );

    ASSERT_UFS_STATUS( status, UFS_CHILD_EXISTS_IN_EXPLICIT_MAPPING );
}

/* ########################################################################## */


/* ufsResolveStorageInView                                                    */

static void test_ufs_resolve_storage_in_view_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id, area0, file0;
    ufsStatusType status;

    ufsStruct = *state;

    file0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( file0 );

    area0 = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( area0 );

    status = ufsAddMapping( ufsStruct -> ufs, area0, file0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view0 = { area0, UFS_VIEW_TERMINATOR };
    id = ufsResolveStorageInView( NULL, view0, file0 );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    ufsViewType view1 = { area0, area0, UFS_VIEW_TERMINATOR };
    id = ufsResolveStorageInView( ufsStruct -> ufs, view1, file0 );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    ufsViewType view2 = { UFS_AREA_BASE_IDENTIFIER, area0, UFS_VIEW_TERMINATOR };
    id = ufsResolveStorageInView( ufsStruct -> ufs, view2, file0 );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    ufsViewType view3 = { -10, UFS_VIEW_TERMINATOR };
    id = ufsResolveStorageInView( ufsStruct -> ufs, view3, file0 );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsResolveStorageInView( ufsStruct -> ufs, view0, -1 );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_resolve_storage_in_view_area_does_not_exist( void **state ) 
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;
    ufsViewType view = { 1, UFS_VIEW_TERMINATOR };

    ufsStruct = *state;

    id = ufsResolveStorageInView( ufsStruct -> ufs, view, 1 );
    ASSERT_UFS_ERROR( id, UFS_INVALID_AREA_IN_VIEW );
}

static void test_ufs_resolve_storage_in_view( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId, areaId, ret;
    ufsStatusType status;

    ufsStruct = *state;

    fileId = ufsAddStorage( ufsStruct -> ufs,
                UFS_STORAGE_ROOT_IDENTIFIER,
                UFS_STORAGE_TYPE_FILE,
                TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    areaId = ufsAddArea( ufsStruct -> ufs,
                    TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };
    ret = ufsResolveStorageInView( ufsStruct -> ufs, view, fileId );
    ASSERT_UFS_NO_ERROR( ret );

    assert_int_equal( ret, areaId );
}

static void test_ufs_resolve_storage_in_view_base_fallback( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, ret;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs,
                    TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    ufsViewType view = { areaId, UFS_AREA_BASE_IDENTIFIER, UFS_VIEW_TERMINATOR };
    ret = ufsResolveStorageInView( ufsStruct -> ufs, view, 1 );
    ASSERT_UFS_ERROR( ret, UFS_CHECK_BASE );
}

static void test_ufs_resolve_storage_in_view_two_areas( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId, areaId0, areaId1, ret;
    ufsStatusType status;

    ufsStruct = *state;

    fileId = ufsAddStorage( ufsStruct -> ufs,
                UFS_STORAGE_ROOT_IDENTIFIER,
                UFS_STORAGE_TYPE_FILE,
                TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    areaId0 = ufsAddArea( ufsStruct -> ufs,
                    TEST_AREA_NAME_0 );
    ASSERT_UFS_NO_ERROR( areaId0 );

    areaId1 = ufsAddArea( ufsStruct -> ufs,
                    TEST_AREA_NAME_1 );
    ASSERT_UFS_NO_ERROR( areaId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId0, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view0 = { areaId0, areaId1, UFS_VIEW_TERMINATOR };
    ret = ufsResolveStorageInView( ufsStruct -> ufs, view0, fileId );
    ASSERT_UFS_NO_ERROR( ret );
    assert_int_equal( ret, areaId0 );

    ufsViewType view1 = { areaId1, areaId0, UFS_VIEW_TERMINATOR };
    ret = ufsResolveStorageInView( ufsStruct -> ufs, view1, fileId );
    ASSERT_UFS_NO_ERROR( ret );
    assert_int_equal( ret, areaId0 );
}

static void test_ufs_resolve_storage_in_view_file_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, ret;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs,
                    TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };
    ret = ufsResolveStorageInView( ufsStruct -> ufs, view, 1 );
    ASSERT_UFS_ERROR( ret, UFS_DOES_NOT_EXIST );

}

static void test_ufs_resolve_storage_in_view_view_order( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId, areaId0, areaId1, ret;
    ufsStatusType status;

    ufsStruct = *state;

    fileId = ufsAddStorage( ufsStruct -> ufs,
                UFS_STORAGE_ROOT_IDENTIFIER,
                UFS_STORAGE_TYPE_FILE,
                TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    areaId0 = ufsAddArea( ufsStruct -> ufs,
                    TEST_AREA_NAME_0 );
    ASSERT_UFS_NO_ERROR( areaId0 );

    areaId1 = ufsAddArea( ufsStruct -> ufs,
                    TEST_AREA_NAME_1 );
    ASSERT_UFS_NO_ERROR( areaId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId0, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId1, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view0 = { areaId0, areaId1, UFS_VIEW_TERMINATOR };
    ret = ufsResolveStorageInView( ufsStruct -> ufs, view0, fileId );
    ASSERT_UFS_NO_ERROR( ret );
    assert_int_equal( ret, areaId0 );

    ufsViewType view1 = { areaId1, areaId0, UFS_VIEW_TERMINATOR };
    ret = ufsResolveStorageInView( ufsStruct -> ufs, view1, fileId );
    ASSERT_UFS_NO_ERROR( ret );
    assert_int_equal( ret, areaId1 );
}

static void test_ufs_resolve_storage_in_view_empty_view( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;
    ufsViewType view = { UFS_VIEW_TERMINATOR };

    id = ufsResolveStorageInView( ufsStruct -> ufs, view, 1 );
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
}

static void test_ufs_resolve_storage_in_view_only_base( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;
    ufsViewType view = { UFS_AREA_BASE_IDENTIFIER, UFS_VIEW_TERMINATOR };

    id = ufsResolveStorageInView( ufsStruct -> ufs, view, 1 );
    ASSERT_UFS_ERROR( id, UFS_CHECK_BASE );
}

static void test_ufs_resolve_storage_in_view_root( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id, areaId;

    ufsStruct = *state;
    ufsViewType view0 = { UFS_AREA_BASE_IDENTIFIER, UFS_VIEW_TERMINATOR };

    id = ufsResolveStorageInView( ufsStruct -> ufs, view0, UFS_STORAGE_ROOT_IDENTIFIER );
    ASSERT_UFS_NO_ERROR( id );
    assert_int_equal( id, UFS_AREA_BASE_IDENTIFIER );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    ufsViewType view1 = { areaId, UFS_VIEW_TERMINATOR };
    id = ufsResolveStorageInView( ufsStruct -> ufs, view1, UFS_STORAGE_ROOT_IDENTIFIER );
    ASSERT_UFS_NO_ERROR( id );
    assert_int_equal( id, areaId );

    ufsViewType view2 = { UFS_VIEW_TERMINATOR };
    id = ufsResolveStorageInView( ufsStruct -> ufs, view2, UFS_STORAGE_ROOT_IDENTIFIER );
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsIterDirInView                                                           */


struct iterValidationStruct {
    ufsIdentifierType idents[ 32 ];
    char *names[ 32 ];
    ufsIdentifierType seen[ 32 ];
};

static ufsStatusType iterDummy( ufsIdentifierType storage,
                                const char *name,
                                uint64_t currEntry,
                                uint64_t numEntries,
                                void *userData )
{
    int *v;
    if ( userData ) {
        v = userData;
        *v = 1;
    }
    return UFS_NO_ERROR;
}

static ufsStatusType iterStorageCheck( ufsIdentifierType storage,
                                const char *name,
                                uint64_t currEntry,
                                uint64_t numEntries,
                                void *userData )
{
    if ( storage < 0 )
        return UFS_BAD_CALL;
    return UFS_NO_ERROR;
}

static ufsStatusType iterNameCheck( ufsIdentifierType storage,
                                const char *name,
                                uint64_t currEntry,
                                uint64_t numEntries,
                                void *userData )
{
    if ( !name )
        return UFS_BAD_CALL;
    return UFS_NO_ERROR;
}

static ufsStatusType iterEntryCountCheck( ufsIdentifierType storage,
                                const char *name,
                                uint64_t currEntry,
                                uint64_t numEntries,
                                void *userData )
{
    if ( currEntry >= numEntries )
        return UFS_BAD_CALL;
    return UFS_NO_ERROR;
}

static ufsStatusType iterValidator( ufsIdentifierType storage,
                                    const char *name,
                                    uint64_t currEntry,
                                    uint64_t numEntries,
                                    void *userData )
{
    int i;
    struct iterValidationStruct *validator;

    validator = userData;
    for ( i = 0; validator -> idents[ i ] != -1; i++ )
        if ( validator -> idents[ i ] == storage &&
                strcmp( validator -> names[ i ], name ) == 0 )
            validator -> seen[ i ]++;
    return UFS_NO_ERROR;
}

static ufsStatusType iterReturnValidator( ufsIdentifierType storage,
                                    const char *name,
                                    uint64_t currEntry,
                                    uint64_t numEntries,
                                    void *userData )
{
    int *v;

    v = userData;
    (*v)++;
    return UFS_BAD_CALL;
}

static void test_ufs_iter_dir_in_view_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType area0, file0;
    ufsStatusType status;

    ufsStruct = *state;

    file0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( file0 );

    area0 = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( area0 );

    status = ufsAddMapping( ufsStruct -> ufs, area0, file0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view0 = { area0, UFS_VIEW_TERMINATOR };
    status = ufsIterateDirInView( NULL,
            view0,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterDummy,
            NULL );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    ufsViewType view1 = { area0, area0, UFS_VIEW_TERMINATOR };
    status = ufsIterateDirInView( ufsStruct -> ufs,
            view1,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterDummy,
            NULL );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    ufsViewType view2 = { UFS_AREA_BASE_IDENTIFIER, area0, UFS_VIEW_TERMINATOR };
    status = ufsIterateDirInView( ufsStruct -> ufs,
            view2,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterDummy,
            NULL );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    ufsViewType view3 = { -10, UFS_VIEW_TERMINATOR };
    status = ufsIterateDirInView( ufsStruct -> ufs,
            view3,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterDummy,
            NULL );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsIterateDirInView( ufsStruct -> ufs,
            view0,
            -1,
            iterDummy,
            NULL );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

    status = ufsIterateDirInView( ufsStruct -> ufs,
            view0,
            UFS_STORAGE_ROOT_IDENTIFIER,
            NULL,
            NULL );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );

}

static void test_ufs_iter_dir_in_view_area_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;
    ufsViewType view = { 1, UFS_VIEW_TERMINATOR };

    ufsStruct = *state;

    status = ufsIterateDirInView( ufsStruct -> ufs,
            view,
            1,
            iterDummy,
            NULL );
    ASSERT_UFS_STATUS( status, UFS_INVALID_AREA_IN_VIEW );
}

static void test_ufs_iter_dir_in_view_directory_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status;
    ufsViewType view = { UFS_VIEW_TERMINATOR };

    ufsStruct = *state;

    status = ufsIterateDirInView( ufsStruct -> ufs,
            view,
            1,
            iterDummy,
            NULL );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );
}

static void test_ufs_iter_dir_in_view_callback_is_called( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    int isCalled;
    ufsIdentifierType fileId, areaId;
    ufsStatusType status;

    ufsStruct = *state;

    fileId = ufsAddStorage( ufsStruct -> ufs, 
                            UFS_STORAGE_ROOT_IDENTIFIER,
                            UFS_STORAGE_TYPE_FILE,
                            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    isCalled = 0;
    status = ufsIterateDirInView( ufsStruct -> ufs,
            view,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterDummy, 
            &isCalled );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    assert_int_equal( isCalled, 1 );
}

static void test_ufs_iter_dir_in_view_name_is_not_null( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId, areaId;
    ufsStatusType status;

    ufsStruct = *state;

    fileId = ufsAddStorage( ufsStruct -> ufs, 
                            UFS_STORAGE_ROOT_IDENTIFIER,
                            UFS_STORAGE_TYPE_FILE,
                            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };

    status = ufsIterateDirInView( ufsStruct -> ufs,
            view,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterNameCheck, 
            NULL );
    ASSERT_UFS_STATUS_NO_ERROR( status );
}

static void test_ufs_iter_dir_in_view_storage_is_valid( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId, areaId;
    ufsStatusType status;

    ufsStruct = *state;

    fileId = ufsAddStorage( ufsStruct -> ufs, 
                            UFS_STORAGE_ROOT_IDENTIFIER,
                            UFS_STORAGE_TYPE_FILE,
                            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };

    status = ufsIterateDirInView( ufsStruct -> ufs,
            view,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterStorageCheck,
            NULL );
    ASSERT_UFS_STATUS_NO_ERROR( status );
}

static void test_ufs_iter_dir_in_view_entry_counters_are_valid( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId, areaId;
    ufsStatusType status;

    ufsStruct = *state;

    fileId = ufsAddStorage( ufsStruct -> ufs, 
                            UFS_STORAGE_ROOT_IDENTIFIER,
                            UFS_STORAGE_TYPE_FILE,
                            TEST_FILE_NAME );
    ASSERT_UFS_NO_ERROR( fileId );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };

    status = ufsIterateDirInView( ufsStruct -> ufs,
            view,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterEntryCountCheck,
            NULL );
    ASSERT_UFS_STATUS_NO_ERROR( status );

}

static void test_ufs_iter_dir_in_view_return_is_propogated( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType fileId0, fileId1, areaId;
    ufsStatusType status;
    int numCalls;

    ufsStruct = *state;

    fileId0 = ufsAddStorage( ufsStruct -> ufs, 
                            UFS_STORAGE_ROOT_IDENTIFIER,
                            UFS_STORAGE_TYPE_FILE,
                            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs, 
                            UFS_STORAGE_ROOT_IDENTIFIER,
                            UFS_STORAGE_TYPE_FILE,
                            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };

    numCalls = 0;
    status = ufsIterateDirInView( ufsStruct -> ufs,
            view,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterReturnValidator,
            &numCalls );
    ASSERT_UFS_STATUS( status, UFS_BAD_CALL );
    assert_int_equal( numCalls, 1 );
}

static void test_ufs_iter_dir_in_view( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, fileId0, fileId1;
    ufsStatusType status;
    int i;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator = {
        .idents = { fileId0, fileId1, -1 },
        .names = { TEST_FILE_NAME_0, TEST_FILE_NAME_1 }
    };
    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };
    memset( validator.seen, 0, sizeof( validator.seen ) );
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            UFS_STORAGE_ROOT_IDENTIFIER, 
            iterValidator,
            &validator );
    ASSERT_UFS_STATUS_NO_ERROR( status );
    for (i = 0; validator.idents[ i ] != -1; i++)
        assert_int_equal( validator.seen[ i ], 1 );
}

static void test_ufs_iter_dir_in_view_multiple_areas( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId0, areaId1, fileId0, fileId1;
    ufsStatusType status;
    int i;

    ufsStruct = *state;

    areaId0 = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME_0 );
    ASSERT_UFS_NO_ERROR( areaId0 );

    areaId1 = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME_1 );
    ASSERT_UFS_NO_ERROR( areaId1 );

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId0, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId1, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator = {
        .idents = { fileId0, fileId1, -1 },
        .names = { TEST_FILE_NAME_0, TEST_FILE_NAME_1 }
    };
    ufsViewType view = { areaId0, areaId1, UFS_VIEW_TERMINATOR };
    memset( validator.seen, 0, sizeof( validator.seen ) );
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            UFS_STORAGE_ROOT_IDENTIFIER, 
            iterValidator,
            &validator );
    ASSERT_UFS_STATUS_NO_ERROR( status );
    for (i = 0; validator.idents[ i ] != -1; i++)
        assert_int_equal( validator.seen[ i ], 1 );

}

static void test_ufs_iter_dir_in_view_dir_is_not_root( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, dirId, fileId0, fileId1;
    ufsStatusType status;
    int i;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddStorage( ufsStruct -> ufs, 
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_DIRECTORY,
            TEST_DIRECTORY_NAME );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
            dirId,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );


    status = ufsAddMapping( ufsStruct -> ufs, areaId, dirId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator = {
        .idents = { fileId0, fileId1, -1 },
        .names = { TEST_FILE_NAME_0, TEST_FILE_NAME_1 }
    };
    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };
    memset( validator.seen, 0, sizeof( validator.seen ) );
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            dirId, 
            iterValidator,
            &validator );
    ASSERT_UFS_STATUS_NO_ERROR( status );
    for (i = 0; validator.idents[ i ] != -1; i++)
        assert_int_equal( validator.seen[ i ], 1 );

}

static void test_ufs_iter_dir_in_view_multiple_areas_with_duplicates( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId0, areaId1, fileId0, fileId1;
    ufsStatusType status;
    int i;

    ufsStruct = *state;

    areaId0 = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME_0 );
    ASSERT_UFS_NO_ERROR( areaId0 );

    areaId1 = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME_1 );
    ASSERT_UFS_NO_ERROR( areaId1 );

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId0, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId0, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId1, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId1, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator = {
        .idents = { fileId0, fileId1, -1 },
        .names = { TEST_FILE_NAME_0, TEST_FILE_NAME_1 }
    };
    ufsViewType view = { areaId0, areaId1, UFS_VIEW_TERMINATOR };
    memset( validator.seen, 0, sizeof( validator.seen ) );
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            UFS_STORAGE_ROOT_IDENTIFIER, 
            iterValidator,
            &validator );
    ASSERT_UFS_STATUS_NO_ERROR( status );
    for (i = 0; validator.idents[ i ] != -1; i++)
        assert_int_equal( validator.seen[ i ], 1 );

}

static void test_ufs_iter_dir_in_view_empty_dir( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    int isCalled;
    ufsIdentifierType areaId;
    ufsStatusType status;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };

    isCalled = 0;
    status = ufsIterateDirInView( ufsStruct -> ufs,
            view,
            UFS_STORAGE_ROOT_IDENTIFIER,
            iterDummy, 
            &isCalled );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    assert_int_equal( isCalled, 0 );
}

static void test_ufs_iter_dir_in_view_ends_with_base( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, fileId0, fileId1;
    ufsStatusType status;
    int i;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator = {
        .idents = { fileId0, fileId1, -1 },
        .names = { TEST_FILE_NAME_0, TEST_FILE_NAME_1 }
    };
    ufsViewType view = { areaId, UFS_AREA_BASE_IDENTIFIER, UFS_VIEW_TERMINATOR };
    memset( validator.seen, 0, sizeof( validator.seen ) );
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            UFS_STORAGE_ROOT_IDENTIFIER, 
            iterValidator,
            &validator );
    ASSERT_UFS_STATUS_NO_ERROR( status );
    for (i = 0; validator.idents[ i ] != -1; i++)
        assert_int_equal( validator.seen[ i ], 1 );

}

static void test_ufs_iter_dir_in_view_only_base( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, fileId0, fileId1;
    ufsStatusType status;
    int isCalled;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator = {
        .idents = { fileId0, fileId1, -1 },
        .names = { TEST_FILE_NAME_0, TEST_FILE_NAME_1 }
    };
    ufsViewType view = { UFS_AREA_BASE_IDENTIFIER, UFS_VIEW_TERMINATOR };
    memset( validator.seen, 0, sizeof( validator.seen ) );
    isCalled = 0;
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            UFS_STORAGE_ROOT_IDENTIFIER, 
            iterDummy,
            &isCalled );

    ASSERT_UFS_STATUS_NO_ERROR( status );
    assert_int_equal( isCalled, 0 );
}

static void test_ufs_iter_dir_in_view_empty_view( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, fileId0, fileId1;
    ufsStatusType status;
    int isCalled;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator = {
        .idents = { fileId0, fileId1, -1 },
        .names = { TEST_FILE_NAME_0, TEST_FILE_NAME_1 }
    };
    ufsViewType view = { UFS_VIEW_TERMINATOR };
    memset( validator.seen, 0, sizeof( validator.seen ) );
    isCalled = 0;
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            UFS_STORAGE_ROOT_IDENTIFIER, 
            iterDummy,
            &isCalled );

    ASSERT_UFS_STATUS_NO_ERROR( status );
    assert_int_equal( isCalled, 0 );

}

static void test_ufs_iter_dir_in_view_remove_consinstency( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType areaId, fileId0, fileId1;
    ufsStatusType status;
    int i;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, TEST_AREA_NAME );
    ASSERT_UFS_NO_ERROR( areaId );

    fileId0 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_0 );
    ASSERT_UFS_NO_ERROR( fileId0 );

    fileId1 = ufsAddStorage( ufsStruct -> ufs,
            UFS_STORAGE_ROOT_IDENTIFIER,
            UFS_STORAGE_TYPE_FILE,
            TEST_FILE_NAME_1 );
    ASSERT_UFS_NO_ERROR( fileId1 );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator0 = {
        .idents = { fileId0, fileId1, -1 },
        .names = { TEST_FILE_NAME_0, TEST_FILE_NAME_1 }
    };
    ufsViewType view = { areaId, UFS_VIEW_TERMINATOR };
    memset( validator0.seen, 0, sizeof( validator0.seen ) );
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            UFS_STORAGE_ROOT_IDENTIFIER, 
            iterValidator,
            &validator0 );
    ASSERT_UFS_STATUS_NO_ERROR( status );
    for (i = 0; validator0.idents[ i ] != -1; i++)
        assert_int_equal( validator0.seen[ i ], 1 );

    status = ufsRemoveMapping( ufsStruct -> ufs, areaId, fileId1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    struct iterValidationStruct validator1 = {
        .idents = { fileId0, -1 },
        .names = { TEST_FILE_NAME_0 }
    };
    memset( validator1.seen, 0, sizeof( validator1.seen ) );
    status = ufsIterateDirInView( 
            ufsStruct -> ufs, 
            view, 
            UFS_STORAGE_ROOT_IDENTIFIER, 
            iterValidator,
            &validator1 );
    ASSERT_UFS_STATUS_NO_ERROR( status );

    for (i = 0; validator1.idents[ i ] != -1; i++)
        assert_int_equal( validator1.seen[ i ], 1 );

}
/* ########################################################################## */

static const struct CMUnitTest ufs_test_suite[] = {

    cmocka_unit_test( test_ufs_init ),

    /* ufsAddStorage tests.                                                     */
    UFS_TEST( test_ufs_add_storage_bad_args ),
    UFS_TEST( test_ufs_add_directory ),
    UFS_TEST( test_ufs_add_directory_non_root ),
    UFS_TEST( test_ufs_add_directory_duplicate ),
    UFS_TEST( test_ufs_add_directory_duplicate_non_root ),
    UFS_TEST( test_ufs_add_directory_parent_does_not_exist ),
    UFS_TEST( test_ufs_add_directory_same_name_different_directory ),
    UFS_TEST( test_ufs_add_directory_same_name_different_directory_one_root ),
    UFS_TEST( test_ufs_add_directory_use_file_as_parent ),
    UFS_TEST( test_ufs_add_file ),
    UFS_TEST( test_ufs_add_file_non_root ),
    UFS_TEST( test_ufs_add_file_duplicate ),
    UFS_TEST( test_ufs_add_file_duplicate_non_root ),
    UFS_TEST( test_ufs_add_file_parent_does_not_exist ),
    UFS_TEST( test_ufs_add_file_same_name_different_directory ),
    UFS_TEST( test_ufs_add_file_same_name_different_directory_one_root ),
    UFS_TEST( test_ufs_add_file_use_file_as_parent ),
    /* ====================================================================== */

    /* ufsAddArea tests.                                                      */
    UFS_TEST( test_ufs_add_area_bad_args ),
    UFS_TEST( test_ufs_add_area ),
    UFS_TEST( test_ufs_add_area_duplicate ),
    UFS_TEST( test_ufs_add_area_illegal_name ),
    /* ====================================================================== */

    /* ufsAddMapping tests.                                                   */
    UFS_TEST( test_ufs_add_mapping_bad_args ),
    UFS_TEST( test_ufs_add_mapping_area_file ),
    UFS_TEST( test_ufs_add_mapping_area_directory ),
    UFS_TEST( test_ufs_add_mapping_duplicate ),
    UFS_TEST( test_ufs_add_mapping_area_does_not_exist ),
    UFS_TEST( test_ufs_add_mapping_file_does_not_exist ),
    UFS_TEST( test_ufs_add_mapping_parent_is_not_mapped ),
    /* ====================================================================== */

    /* ufsGetStorage tests.                                                   */
    UFS_TEST( test_ufs_get_storage_bad_args ),
    UFS_TEST( test_ufs_get_directory ),
    UFS_TEST( test_ufs_get_directory_parent_does_not_exist ),
    UFS_TEST( test_ufs_get_directory_does_not_exist ),
    UFS_TEST( test_ufs_get_file ),
    UFS_TEST( test_ufs_get_file_does_not_exist ),
    UFS_TEST( test_ufs_get_file_parent_does_not_exist ),
    UFS_TEST( test_ufs_get_file_exists_in_different_directory ),
    /* ====================================================================== */

    /* ufsGetArea tests.                                                      */
    UFS_TEST( test_ufs_get_area_bad_args ),
    UFS_TEST( test_ufs_get_area ),
    UFS_TEST( test_ufs_get_area_does_not_exist ),
    /* ====================================================================== */

    /* ufsProbeMapping tests.                                                 */
    UFS_TEST( test_ufs_probe_mapping_bad_args ),
    UFS_TEST( test_ufs_probe_mapping ),
    UFS_TEST( test_ufs_probe_mapping_area_does_not_exist ),
    UFS_TEST( test_ufs_probe_mapping_file_does_not_exist ),
    UFS_TEST( test_ufs_probe_mapping_mapping_does_not_exist ),
    /* ====================================================================== */

    /* ufsRemoveStorage tests.                                                */
    UFS_TEST( test_ufs_remove_storage_bad_args ),
    UFS_TEST( test_ufs_remove_directory ),
    UFS_TEST( test_ufs_remove_directory_does_not_exist ),
    UFS_TEST( test_ufs_remove_directory_contains_file ),
    UFS_TEST( test_ufs_remove_directory_exists_in_mapping ),
    UFS_TEST( test_ufs_remove_directory_double_remove ),
    UFS_TEST( test_ufs_remove_directory_remove_then_add ),
    UFS_TEST( test_ufs_remove_directory_remove_then_get ),
    UFS_TEST( test_ufs_remove_file ),
    UFS_TEST( test_ufs_remove_file_does_not_exist ),
    UFS_TEST( test_ufs_remove_file_exists_in_mapping ),
    UFS_TEST( test_ufs_remove_file_double_remove ),
    UFS_TEST( test_ufs_remove_file_remove_then_add ),
    UFS_TEST( test_ufs_remove_file_remove_then_get ),
    /* ====================================================================== */

    /* ufsRemoveArea tests.                                                   */
    UFS_TEST( test_ufs_remove_area_bad_args ),
    UFS_TEST( test_ufs_remove_area ),
    UFS_TEST( test_ufs_remove_area_does_not_exist ),
    UFS_TEST( test_ufs_remove_area_exists_in_mapping ),
    UFS_TEST( test_ufs_remove_area_double_remove ),
    UFS_TEST( test_ufs_remove_area_remove_then_add ),
    UFS_TEST( test_ufs_remove_area_remove_then_get ),
    /* ====================================================================== */

    /* ufsRemoveMapping tests.                                                */
    UFS_TEST( test_ufs_remove_mapping_bad_args ),
    UFS_TEST( test_ufs_remove_mapping ),
    UFS_TEST( test_ufs_remove_mapping_does_not_exist ),
    UFS_TEST( test_ufs_remove_mapping_no_side_effects ),
    UFS_TEST( test_ufs_remove_mapping_double_remove ),
    UFS_TEST( test_ufs_remove_mapping_remove_then_add ),
    UFS_TEST( test_ufs_remove_mapping_remove_then_probe ),
    UFS_TEST( test_ufs_remove_mapping_child_is_mapped ),
    /* ====================================================================== */

    /* ufsResolveStorageInView                                                */
    UFS_TEST( test_ufs_resolve_storage_in_view_bad_args ),
    UFS_TEST( test_ufs_resolve_storage_in_view_area_does_not_exist ),
    UFS_TEST( test_ufs_resolve_storage_in_view ),
    UFS_TEST( test_ufs_resolve_storage_in_view_base_fallback ),
    UFS_TEST( test_ufs_resolve_storage_in_view_two_areas ),
    UFS_TEST( test_ufs_resolve_storage_in_view_file_does_not_exist ),
    UFS_TEST( test_ufs_resolve_storage_in_view_view_order ),
    UFS_TEST( test_ufs_resolve_storage_in_view_empty_view ),
    UFS_TEST( test_ufs_resolve_storage_in_view_only_base ),
    UFS_TEST( test_ufs_resolve_storage_in_view_root ),
    /* ====================================================================== */

    /* ufsIterDirInView                                                       */
    UFS_TEST( test_ufs_iter_dir_in_view_bad_args ),
    UFS_TEST( test_ufs_iter_dir_in_view_area_does_not_exist ),
    UFS_TEST( test_ufs_iter_dir_in_view_directory_does_not_exist ),
    UFS_TEST( test_ufs_iter_dir_in_view_callback_is_called ),
    UFS_TEST( test_ufs_iter_dir_in_view_name_is_not_null ),
    UFS_TEST( test_ufs_iter_dir_in_view_storage_is_valid ),
    UFS_TEST( test_ufs_iter_dir_in_view_entry_counters_are_valid ),
    UFS_TEST( test_ufs_iter_dir_in_view_return_is_propogated ),
    UFS_TEST( test_ufs_iter_dir_in_view ),
    UFS_TEST( test_ufs_iter_dir_in_view_multiple_areas ),
    UFS_TEST( test_ufs_iter_dir_in_view_dir_is_not_root ),
    UFS_TEST( test_ufs_iter_dir_in_view_multiple_areas_with_duplicates ),
    UFS_TEST( test_ufs_iter_dir_in_view_empty_dir ),
    UFS_TEST( test_ufs_iter_dir_in_view_ends_with_base ),
    UFS_TEST( test_ufs_iter_dir_in_view_only_base ),
    UFS_TEST( test_ufs_iter_dir_in_view_empty_view ),
    UFS_TEST( test_ufs_iter_dir_in_view_remove_consinstency )
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
