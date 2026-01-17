/******************************************************************************\
*  ufs_image_test.c                                                            *
*                                                                              *
*  Tests for ufs images.                                                       *
*                                                                              *
*              Written by A.N.                                  10-01-2026     *
*                                                                              *
\******************************************************************************/

#define UFS_TEST_DISABLE

#ifndef UFS_TEST_DISABLE

#define UFS_TESTING

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
#include "ufs_defs.h"
#include "ufs_image.h"
#include <unistd.h>
#include "utils.h"

#include <cmocka.h>

#define NEW_IMAGE_PATH_BAD ("/cant_create_here")
#define TEST_SIZE (128)
#define SMALL_TEST_SIZE (4)


/* ----- ufs_image tests ----                                                 */

static void test_ufs_image_open_bad_args(void **state) {
    (void) state;
    ufsImagePtr img = ufsImageOpen( NULL );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
}

static void test_ufs_image_open_does_not_exist(void **state) {
    (void) state;
    ufsImagePtr img = ufsImageOpen( "does_not_exist" );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_IMAGE_DOES_NOT_EXIST );
}

static void test_ufs_image_open_exists(void **state) {
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state; 
    if (ftruncate( fn -> fd, TEST_SIZE ) < 0) {
        perror( "ftruncate" );
        fail_msg( "Could not truncate the temp file..." );
    }

    close( fn -> fd );
    fn -> fd = -1;

    ufsImagePtr img = ufsImageOpen( fn -> name );
    assert_non_null( img );

    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    ufsImageFree( img );
}

static void test_ufs_image_open_too_small(void **state) {
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state; 

    if (ftruncate( fn -> fd, SMALL_TEST_SIZE ) < 0) {
        perror( "ftruncate" );
        fail_msg( "Could not truncate the temp file..." );
    }
    close( fn -> fd );
    fn -> fd = -1;

    ufsImagePtr img = ufsImageOpen( fn -> name );
    assert_null( img );

    assert_int_equal( ufsErrno, UFS_IMAGE_TOO_SMALL );
}

static void test_ufs_image_create_bad_args(void **state) {
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    ufsImagePtr img = ufsImageCreate( fn -> name, SMALL_TEST_SIZE );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );

    /* Make sure it didn't create the file.                                   */
    assert_false( access( fn -> name, F_OK ) == 0 );

    img = ufsImageCreate( NULL, TEST_SIZE );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
}

static void test_ufs_image_create_default_size(void **state) {
    struct stat sb;
    char *byte;
    struct ufsTestUtilsFileNameStruct *fn;
    uint64_t *size, i;
    long pageSize = sysconf( _SC_PAGESIZE );

    if (pageSize < 0) {
        perror( "sysconf" );
        fail_msg( "Could not retrieve page size." );
        return;
    }

    fn = *state;

    ufsImagePtr img = ufsImageCreate( fn -> name, TEST_SIZE );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    assert_true( access( fn -> name, F_OK ) == 0 );

    size = img;
    assert_true( *size == TEST_SIZE );

    /* Could be made faster but size is tiny, it doesn't matter.              */
    byte = (char*)(img) + sizeof( *size ); 
    for (i = 0; i < *size; i++)
        assert_true( byte[i] == '\0' );

    fn -> fd = open( fn -> name, O_RDONLY );
    if ( fstat( fn -> fd, &sb ) < 0 ) {
        perror( "fstat" );
        ufsImageFree( img );
        fail_msg( "Could not stat the mapped file" ); 
        return;
    }

    assert_int_equal( *size, sb.st_size );

                          
    ufsImageFree( img );
}

static void test_ufs_image_create_cant_create_file(void **state) {
    (void) state;

    ufsImagePtr img = ufsImageCreate( NEW_IMAGE_PATH_BAD, TEST_SIZE );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_CANT_CREATE_FILE );
}

static void test_ufs_image_sync_bad_arg(void **state) {
    assert_false( ufsImageSync( NULL ) );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
}

static void test_ufs_image_sync(void **state) {
    char payload[] = "hello world";
    char buff[ 128 ];
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    ufsImagePtr img = ufsImageCreate( fn -> name, 128 );

    char * mem = (char*)(img) + 9;
    
    memcpy( mem, payload, sizeof(payload));

    assert_true( ufsImageSync( img ) );

    fn -> fd = open( fn -> name, O_RDONLY );

    if ( fn -> fd < 0 ){
        perror( "open" );
        fail_msg( "Failed to open image file after syncing" );
        return;
    }
    read( fn -> fd, buff, sizeof( buff ) );

    assert_string_equal( buff + 9, payload );
}

static void test_ufs_image_free_bad_arg(void **state) {
    ufsImageFree( NULL );

    /* This is not defined as an error.                                       */
    assert_int_equal( ufsErrno, UFS_NO_ERROR );
}

static const struct CMUnitTest image_tests[] = {
    cmocka_unit_test(test_ufs_image_open_bad_args),
    cmocka_unit_test(test_ufs_image_open_does_not_exist),
    cmocka_unit_test_setup_teardown(test_ufs_image_open_exists, getFileSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_image_open_too_small, getFileSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_image_create_bad_args, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test_setup_teardown(test_ufs_image_create_default_size, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test(test_ufs_image_create_cant_create_file),
    cmocka_unit_test(test_ufs_image_sync_bad_arg),
    cmocka_unit_test_setup_teardown(test_ufs_image_sync, getFileNameSetup, cleanUpTeardown),
    cmocka_unit_test(test_ufs_image_free_bad_arg),
};

int main(void) {
    return cmocka_run_group_tests(image_tests, NULL, NULL);
}

#else

int main(void) {
    return 0;
}

#endif /* UFS_TEST_DISABLE */
