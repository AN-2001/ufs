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
    ASSERT_UFS_ERROR( id, UFS_DOES_NOT_EXIST );
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

    /* ufsGetDirectory tests.                                                 */
    cmocka_unit_test_setup_teardown( test_ufs_get_directory_bad_args, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_directory, ufsGetInstance, ufsCleanup ),
    cmocka_unit_test_setup_teardown( test_ufs_get_directory_does_not_exist, ufsGetInstance, ufsCleanup ),
    /* ====================================================================== */

    /* TODO: add "add then get" tests.                                        */
};

int main( void ) {
    return cmocka_run_group_tests( image_tests, NULL, NULL );
}

#else

int main( void ) {
    return 0;
}

#endif /* UFS_TEST_DISABLE */
