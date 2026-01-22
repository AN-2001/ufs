/******************************************************************************\
*  test_ufs.c                                                                  *
*                                                                              *
*  Test suite for ufs implementation.                                          *
*                                                                              *
*              Written by A.N.                                  18-01-2026     *
*                                                                              *
\******************************************************************************/


#define UFS_TESTING

#ifndef UFS_TEST_DISABLE

#include <fcntl.h>
#include <memory.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ufs.h"
#include <unistd.h>
#include "utils.h"

#include <cmocka.h>

static void test_ufs_init( void **state ) {
    (void) state;

    ufsType ufs = ufsInit();

    assert_non_null( ufs );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

 
    ufsDestroy( ufs );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );
}

/* ufsAddDirectory tests                                                      */
static void test_ufs_add_directory_bad_args( void **state ) {
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddDirectory( NULL, "testDir" );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );


    ufsAddDirectory( ufsStruct -> ufs, NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_add_directory( void **state ) {
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0;

    ufsStruct = *state;

    id0 = ufsAddDirectory( ufsStruct -> ufs, "testDir" );
    ASSERT_UFS_NO_ERROR( id0 );
}

static void test_ufs_add_directory_duplicate( void **state ) {
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddDirectory( ufsStruct -> ufs, "testDir" );
    ASSERT_UFS_NO_ERROR( id );

    id = ufsAddDirectory( ufsStruct -> ufs, "testDir" );
    ASSERT_UFS_ERROR( id, UFS_ALREADY_EXISTS );
}
/* ########################################################################## */

/* ufsAddFile tests                                                           */
static void test_ufs_add_file_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddFile( NULL, 1, "testName" );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddFile( ufsStruct -> ufs, -1, "testName" );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddFile( ufsStruct -> ufs, 1, NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_add_file( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, dirId;

    ufsStruct = *state;

    dirId = ufsAddDirectory( ufsStruct -> ufs, "newDir" );
    ASSERT_UFS_NO_ERROR( dirId );

    id0 = ufsAddFile( ufsStruct -> ufs, dirId, "testName" );
    ASSERT_UFS_NO_ERROR( id0 );
}

static void test_ufs_add_file_no_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddFile( ufsStruct -> ufs, 1, "testName" );
    ASSERT_UFS_ERROR( id, UFS_DIRECTORY_DOES_NOT_EXIST );
}

static void test_ufs_add_file_duplicate( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id, dirId;

    ufsStruct = *state;

    dirId = ufsAddDirectory( ufsStruct -> ufs, "newDir" );
    ASSERT_UFS_NO_ERROR( dirId );

    id = ufsAddFile( ufsStruct -> ufs, dirId, "testName" );
    ASSERT_UFS_NO_ERROR( id );

    id = ufsAddFile( ufsStruct -> ufs, dirId, "testName" );
    ASSERT_UFS_ERROR( id, UFS_ALREADY_EXISTS );

}

static void test_ufs_add_file_same_name_different_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1, dirId0, dirId1;

    ufsStruct = *state;

    dirId0 = ufsAddDirectory( ufsStruct -> ufs, "newDir0" );
    ASSERT_UFS_NO_ERROR( dirId0 );

    dirId1 = ufsAddDirectory( ufsStruct -> ufs, "newDir1" );
    ASSERT_UFS_NO_ERROR( dirId1 );

    id0 = ufsAddFile( ufsStruct -> ufs, dirId0, "testName" );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsAddFile( ufsStruct -> ufs, dirId1, "testName" );
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

    id = ufsAddArea( NULL, "test" );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsAddArea( ufsStruct -> ufs, NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_add_area( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddArea( ufsStruct -> ufs, "test" );
    ASSERT_UFS_NO_ERROR( id );
}

static void test_ufs_add_area_duplicate( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddArea( ufsStruct -> ufs, "test" );
    ASSERT_UFS_NO_ERROR( id );

    id = ufsAddArea( ufsStruct -> ufs, "test" );
    ASSERT_UFS_ERROR( id, UFS_ALREADY_EXISTS );
}

static void test_ufs_add_area_illegal_name( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsAddArea( ufsStruct -> ufs, "BASE" );
    ASSERT_UFS_ERROR( id, UFS_ILLEGAL_AREA_NAME );
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

static void test_ufs_add_mapping( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId, dirId, fileId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, "testArea" );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddDirectory( ufsStruct -> ufs, "testDirectory" );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddFile( ufsStruct -> ufs, dirId, "testFile" );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status );

}

static void test_ufs_add_mapping_duplicate( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId, dirId, fileId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, "testArea" );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddDirectory( ufsStruct -> ufs, "testDirectory" );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddFile( ufsStruct -> ufs, dirId, "testFile" );
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

    dirId = ufsAddDirectory( ufsStruct -> ufs, "testDirectory" );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddFile( ufsStruct -> ufs, dirId, "testFile" );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsAddMapping( ufsStruct -> ufs, 1, fileId );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_add_mapping_file_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, "testArea" );
    ASSERT_UFS_NO_ERROR( areaId );

    status = ufsAddMapping( ufsStruct -> ufs, areaId, 1 );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsGetDirectory                                                            */
static void test_ufs_get_directory_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;
    id = ufsGetDirectory( NULL , "test" );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetDirectory( ufsStruct -> ufs , NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_get_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1;

    ufsStruct = *state;
    id0 = ufsAddDirectory( ufsStruct -> ufs , "test" );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsGetDirectory( ufsStruct -> ufs , "test" );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_equal( id0, id1 );
}

static void test_ufs_get_directory_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsGetDirectory( ufsStruct -> ufs , "test" );
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsGetFile                                                                 */
static void test_ufs_get_file_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;
    id = ufsGetFile( NULL, 1, "test" );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetFile( ufsStruct -> ufs, -1, "test" );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetFile( ufsStruct -> ufs, 1, NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );
}

static void test_ufs_get_file( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, dirId, id1;

    ufsStruct = *state;

    dirId = ufsAddDirectory( ufsStruct -> ufs, "testDir" );
    ASSERT_UFS_NO_ERROR( dirId );

    id0 = ufsAddFile( ufsStruct -> ufs, dirId, "testFile" );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsGetFile( ufsStruct -> ufs, dirId, "testFile" );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_equal( id0, id1 );
}

static void test_ufs_get_file_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id, dirId;

    ufsStruct = *state;

    dirId = ufsAddDirectory( ufsStruct -> ufs, "testDir" );
    ASSERT_UFS_NO_ERROR( dirId );

    id = ufsGetFile( ufsStruct -> ufs, dirId, "testFile" );
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
}

static void test_ufs_get_file_directory_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsGetFile( ufsStruct -> ufs, 1, "testFile" );
    ASSERT_UFS_ERROR( id, UFS_DIRECTORY_DOES_NOT_EXIST );
}

static void test_ufs_get_file_exists_in_different_directory( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1, dirId0, dirId1;

    ufsStruct = *state;

    dirId0 = ufsAddDirectory( ufsStruct -> ufs, "testDir0" );
    ASSERT_UFS_NO_ERROR( dirId0 );

    dirId1 = ufsAddDirectory( ufsStruct -> ufs, "testDir1" );
    ASSERT_UFS_NO_ERROR( dirId1 );

    id0 = ufsAddFile( ufsStruct -> ufs, dirId0, "testFile" );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsGetFile( ufsStruct -> ufs, dirId1, "testFile" );
    ASSERT_UFS_ERROR( id1, UFS_DOES_NOT_EXIST );
}
/* ########################################################################## */

/* ufsGetArea                                                                 */
static void test_ufs_get_area_bad_args( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsGetArea( NULL, "test" );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

    id = ufsGetArea( ufsStruct -> ufs, NULL );
    ASSERT_UFS_ERROR( id, UFS_BAD_CALL );

}

static void test_ufs_get_area( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id0, id1;

    ufsStruct = *state;

    id0 = ufsAddArea( ufsStruct -> ufs, "test" );
    ASSERT_UFS_NO_ERROR( id0 );

    id1 = ufsGetArea( ufsStruct -> ufs, "test" );
    ASSERT_UFS_NO_ERROR( id1 );

    assert_int_equal( id0, id1 );
}

static void test_ufs_get_area_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsIdentifierType id;

    ufsStruct = *state;

    id = ufsGetArea( ufsStruct -> ufs, "test" );
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
    ufsStatusType status0, status1, areaId, dirId, fileId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, "testArea" );
    ASSERT_UFS_NO_ERROR( areaId );

    dirId = ufsAddDirectory( ufsStruct -> ufs, "testDirectory" );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddFile( ufsStruct -> ufs, dirId, "testFile" );
    ASSERT_UFS_NO_ERROR( fileId );

    status0 = ufsAddMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status0 );

    status1 = ufsProbeMapping( ufsStruct -> ufs, areaId, fileId );
    ASSERT_UFS_STATUS_NO_ERROR( status1 );

}

static void test_ufs_probe_mapping_area_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, dirId, fileId;

    ufsStruct = *state;

    dirId = ufsAddDirectory( ufsStruct -> ufs, "testDirectory" );
    ASSERT_UFS_NO_ERROR( dirId );

    fileId = ufsAddFile( ufsStruct -> ufs, dirId, "testFile" );
    ASSERT_UFS_NO_ERROR( fileId );

    status = ufsProbeMapping( ufsStruct -> ufs, 1, fileId );
    ASSERT_UFS_STATUS( status, UFS_DOES_NOT_EXIST );

}

static void test_ufs_probe_mapping_file_does_not_exist( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType status, areaId;

    ufsStruct = *state;

    areaId = ufsAddArea( ufsStruct -> ufs, "testArea" );
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

static const struct CMUnitTest image_tests[] = {

    cmocka_unit_test( test_ufs_init ),

    /* ufsAddDirectory tests.                                                 */
    cmocka_unit_test_setup_teardown( test_ufs_add_directory_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_directory, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_directory_duplicate, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsAddFile tests.                                                      */
    cmocka_unit_test_setup_teardown( test_ufs_add_file_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_file, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_file_duplicate, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_file_no_directory, ufsGetInstance, ufsCleanup ),
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
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_duplicate, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_area_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_add_mapping_file_does_not_exist, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsGetDirectory tests.                                                 */
    cmocka_unit_test_setup_teardown( test_ufs_get_directory_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_directory, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_directory_does_not_exist, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsGetFile tests.                                                      */
    cmocka_unit_test_setup_teardown( test_ufs_get_file_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file_directory_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_file_exists_in_different_directory, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsGetArea tests.                                                      */
    cmocka_unit_test_setup_teardown( test_ufs_get_area_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_area, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_area_does_not_exist, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* ufsProbeMapping                                                        */
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping_area_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping_file_does_not_exist, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_probe_mapping_mapping_does_not_exist, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

};

int main( void ) {
    return cmocka_run_group_tests( image_tests, NULL, NULL );
}

#else

int main( void ) {
    return 0;
}

#endif /* UFS_TEST_DISABLE */
