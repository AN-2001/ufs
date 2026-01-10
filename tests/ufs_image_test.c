/******************************************************************************\
*  ufs_image_test.c                                                            *
*                                                                              *
*  Tests for ufs images.                                                       *
*                                                                              *
*              Written by A.N.                                  10-01-2026     *
*                                                                              *
\******************************************************************************/

#include <fcntl.h>
#include <memory.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ufs_defs.h"
#include "ufs_image.h"
#include <unistd.h>

#include <cmocka.h>

#define NEW_IMAGE_PATH ("/tmp/ufs_test_img")
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
    (void) state;

    char template[] = "/tmp/ufsTempXXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        fail_msg( "Could not create a temp file..." );
        return;
    }

    ftruncate( fd, 8 );
    close(fd);

    ufsImagePtr img = ufsImageOpen( template );
    assert_non_null( img );

    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    ufsImageFree( img );
    unlink( template );
}

static void test_ufs_image_open_too_small(void **state) {
    (void) state;

    char template[] = "/tmp/ufsTempXXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        fail_msg( "Could not create a temp file..." );
        return;
    }

    ftruncate( fd, 2 );
    close(fd);

    ufsImagePtr img = ufsImageOpen( template );
    assert_null( img );

    assert_int_equal( ufsErrno, UFS_IMAGE_TOO_SMALL );

    unlink( template );
}

static void test_ufs_image_create_bad_args(void **state) {
    (void) state;
    char template[] = "/tmp/ufsTempXXXXXXX";
    char *name;

    name = mktemp( template );
    if ( !name || name[0] == '\0' ) {
        perror( "mktemp" );
        fail_msg( "Could not create temp file name" );
        return;
    }

    ufsImagePtr img = ufsImageCreate( name, SMALL_TEST_SIZE );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );

    /* Make sure it didn't create the file.                                   */
    assert_false( access( name, F_OK ) == 0 );

    img = ufsImageCreate( NULL, TEST_SIZE );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
    unlink( name );
}

static void test_ufs_image_create_default_size(void **state) {
    (void) state;
    char template[] = "/tmp/ufsTempXXXXXXX";
    char *name;
    int fd;
    struct stat sb;
    uint64_t *size;
    long pageSize = sysconf( _SC_PAGESIZE );

    if (pageSize < 0) {
        perror( "sysconf" );
        fail_msg( "Could not retrieve page size." );
        return;
    }

    name = mktemp( template );
    if ( !name || name[0] == '\0' ) {
        perror( "mktemp" );
        fail_msg( "Could not create temp file name" );
        return;
    }

    ufsImagePtr img = ufsImageCreate( name, TEST_SIZE );
    assert_non_null( img );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    assert_true( access( name, F_OK ) == 0 );

    size = img;
    assert_true( *size == TEST_SIZE );

    fd = open( name, O_RDONLY );
    if ( fstat( fd, &sb ) < 0 ) {
        perror( "fstat" );
        ufsImageFree( img );
        unlink( name );
        close( fd );
        
        fail_msg( "Could not stat the mapped file" ); 
        return;
    }

    assert_int_equal( *size, sb.st_size );

                          
    ufsImageFree( img );
    unlink( name );
    close( fd );
}

static void test_ufs_image_create_cant_create_file(void **state) {
    (void) state;

    ufsImagePtr img = ufsImageCreate( NEW_IMAGE_PATH_BAD, TEST_SIZE );
    assert_null( img );
    assert_int_equal( ufsErrno, UFS_CANT_CREATE_FILE );
}

static void test_ufs_image_sync(void **state) {
    (void) state;

    char payload[] = "hello world";
    char buff[ 128 ];
    char template[] = "/tmp/ufsTempXXXXXXX";
    char *name;
    int fd;

    name = mktemp( template );
    if ( !name || name[0] == '\0' ) {
        perror( "mktemp" );
        fail_msg( "Could not create temp file name" );
        return;
    }

    ufsImagePtr img = ufsImageCreate( name, 128 );

    char * mem = (char*)(img) + 9;
    
    memcpy( mem, payload, sizeof(payload));

    assert_true( ufsImageSync( img ) );
    ufsImageFree( img );

    fd = open( name, O_RDONLY );

    if ( fd < 0 ){
        perror( "open" );
        fail_msg( "Failed to open image file after syncing" );
        return;
    }
    read( fd, buff, sizeof( buff ) );

    assert_string_equal( buff + 9, payload );
    close( fd );
}

static const struct CMUnitTest image_tests[] = {
    cmocka_unit_test(test_ufs_image_open_bad_args),
    cmocka_unit_test(test_ufs_image_open_does_not_exist),
    cmocka_unit_test(test_ufs_image_open_exists),
    cmocka_unit_test(test_ufs_image_open_too_small),
    cmocka_unit_test(test_ufs_image_create_bad_args),
    cmocka_unit_test(test_ufs_image_create_default_size),
    cmocka_unit_test(test_ufs_image_create_cant_create_file),
    cmocka_unit_test(test_ufs_image_sync),
};

int main(void) {
    return cmocka_run_group_tests(image_tests, NULL, NULL);
}
