/******************************************************************************\
*  ufs_header_test.c                                                           *
*                                                                              *
*  Tests for ufs headers.                                                      *
*                                                                              *
*              Written by A.N.                                  11-01-2026     *
*                                                                              *
\******************************************************************************/

#include <stdint.h>
#define UFS_TESTING

#include <fcntl.h>
#include <sys/types.h>
#include <memory.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "ufs_defs.h"
#include "ufs_header.h"
#include "ufs_image.h"
#include <unistd.h>
#include "utils.h"

#include <cmocka.h>

/* ----- ufs_header tests ----                                                */

static void test_ufs_header_init_bad_arg( void **state ) {
    struct ufsHeaderSizeRequestStruct badSize;
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    memset( &badSize, 0, sizeof( badSize ) );

    ufsImagePtr img = ufsHeaderInit( NULL, ufsDefaultSizeRequest );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );

    img = ufsHeaderInit( fn -> name, badSize );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
    assert_true( access( fn -> name , F_OK ) != 0 );
}

static void test_ufs_header_init( void **state ) {
    struct ufsHeaderStruct *header;
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    ufsImagePtr img = ufsHeaderInit( fn -> name, ufsDefaultSizeRequest );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    header = ufsHeaderGet( img );

    assert_true( header -> magicNumber == UFS_MAGIC_NUMBER );
    assert_true( header -> version >= 1 );

    assert_true( header -> sizes[ UFS_TYPES_FILE ] == ufsDefaultSizeRequest.numFiles );
    assert_true( header -> sizes[ UFS_TYPES_AREA ] == ufsDefaultSizeRequest.numAreas );
    assert_true( header -> sizes[ UFS_TYPES_NODE ] == ufsDefaultSizeRequest.numNodes );
    assert_true( header -> sizes[ UFS_TYPES_STRING ] == ufsDefaultSizeRequest.numStrBytes );

    ufsImageFree( img );
}

static void test_ufs_header_init_exists( void **state ) {
    struct ufsHeaderStruct *header;
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    ufsImagePtr img = ufsHeaderInit( fn -> name, ufsDefaultSizeRequest );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    ufsImageFree( img );

    img = ufsImageOpen( fn -> name );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    header = ufsHeaderGet( img );
    assert_true( header -> magicNumber == UFS_MAGIC_NUMBER );
    assert_true( header -> version >= 1 );

    assert_true( header -> sizes[ UFS_TYPES_FILE ] == ufsDefaultSizeRequest.numFiles );
    assert_true( header -> sizes[ UFS_TYPES_AREA ] == ufsDefaultSizeRequest.numAreas );
    assert_true( header -> sizes[ UFS_TYPES_NODE ] == ufsDefaultSizeRequest.numNodes );
    assert_true( header -> sizes[ UFS_TYPES_STRING ] == ufsDefaultSizeRequest.numStrBytes );

    ufsImageFree( img );
}

static void test_ufs_header_validate_bad_arg( void **state ) {
    assert_null( ufsHeaderValidate( NULL ) );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
}

static void test_ufs_header_validate( void **state ) {
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    ufsImagePtr img = ufsHeaderInit( fn -> name, ufsDefaultSizeRequest );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    assert_non_null( ufsHeaderValidate( img ) );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );
    ufsImageFree( img );
}

static void test_ufs_header_validate_corrupted_magic_number( void **state ) {
    (void) state;
    struct ufsHeaderStruct *header;
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    ufsImagePtr img = ufsHeaderInit( fn -> name, ufsDefaultSizeRequest );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    header = ufsHeaderGet( img );
    header -> magicNumber = 123;

    assert_null( ufsHeaderValidate( img ) );
}

static void test_ufs_header_validate_bad_version( void **state ) {
    struct ufsHeaderStruct *header;
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    ufsImagePtr img = ufsHeaderInit( fn -> name, ufsDefaultSizeRequest );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    header = ufsHeaderGet( img );
    header -> version = 0;

    assert_null( ufsHeaderValidate( img ) );
}

static void test_ufs_header_validate_random_file( void **state ) {
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    if ( ftruncate( fn -> fd, 1024 ) < 0 ) {
        perror( "ftruncate" );
        fail_msg( "Could not truncate temp file" );
    }

    ufsImagePtr img = ufsImageOpen( fn -> name );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    assert_null( ufsHeaderValidate( img ) );
    assert_int_equal( ufsErrno, UFS_IMAGE_IS_CORRUPTED );
}

static void test_ufs_header_validate_too_small( void **state ) {
    struct ufsTestUtilsFileNameStruct *fn;
    uint64_t
        badSize = sizeof( struct ufsHeaderStruct );

    fn = *state;

    if ( ftruncate( fn -> fd, badSize ) < 0 ) {
        perror( "ftruncate" );
        fail_msg( "Could not truncate temp file" );
    }

    ufsImagePtr img = ufsImageOpen( fn -> name );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    assert_null( ufsHeaderValidate( img ) );
    assert_int_equal( ufsErrno, UFS_IMAGE_TOO_SMALL );
}

static void test_ufs_header_validate_bad_size( void **state ) {
    struct ufsTestUtilsFileNameStruct *fn;
    uint64_t
        badSize = sizeof( struct ufsHeaderStruct ) * 4;

    fn = *state;

    ufsImagePtr img = ufsHeaderInit( fn -> name, ufsDefaultSizeRequest );
    assert_non_null( img );

    ufsImageFree( img );

    fn -> fd = open( fn -> name, O_RDWR );

    if (ftruncate( fn -> fd, badSize ) < 0) {
        perror( "ftruncate" );
        fail_msg( "Could not truncate temp file." );
    }

    img = ufsImageOpen( fn -> name );
    assert_non_null( img );
    assert_null( ufsHeaderValidate( img ) );
    assert_int_equal( ufsErrno, UFS_IMAGE_BAD_SIZE );
}

static void test_ufs_header_get_bad_arg( void **state ) {
    assert_null( ufsHeaderGet( NULL ) );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
}

static const struct CMUnitTest header_tests[] = {
    cmocka_unit_test_setup_teardown(test_ufs_header_init_bad_arg, getFileNameSetup, cleanUpTeardown ),
    cmocka_unit_test_setup_teardown(test_ufs_header_init, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_header_init_exists, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test(test_ufs_header_validate_bad_arg),
    cmocka_unit_test_setup_teardown(test_ufs_header_validate, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_header_validate_bad_version, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_header_validate_corrupted_magic_number, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_header_validate_random_file, getFileSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_header_validate_too_small, getFileSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_header_validate_bad_size, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test(test_ufs_header_get_bad_arg),
};

int main(void) {
    return cmocka_run_group_tests(header_tests, NULL, NULL);
}
