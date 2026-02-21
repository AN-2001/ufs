/******************************************************************************\
*  test_ufs_trie.c                                                             *
*                                                                              *
*  Unit tests for ufs trie.                                                    *
*                                                                              *
*              Written by A.N.                                  18-02-2026     *
*                                                                              *
\******************************************************************************/

#include "ufs_core.h"
#define UFS_TESTING

#ifndef UFS_TEST_DISABLE

#include <memory.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ufs_trie.h>
#include "utils.h"
#include <time.h>

#include <cmocka.h>

#define UFS_TRIE_TEST( name ) \
    cmocka_unit_test_setup_teardown( name, ufsTrieGetInstance, ufsTrieCleanup )

#define TEST_STR TEST_STR_0
#define TEST_STR_0 ("test_0")
#define TEST_STR_1 ("test_1")

/* ufsTrieInit tests                                                          */
void test_ufs_trie_init( void **state )
{
    ufsStatusType errorNo;
    ufsTrieType trie;

    trie = ufsTrieInit( &errorNo );

    assert_non_null( trie );
    assert_int_equal( errorNo, UFS_NO_ERROR );

    ufsTrieDestroy( trie, &errorNo );
    assert_int_equal( errorNo, UFS_NO_ERROR );

    ufsTrieDestroy( NULL, &errorNo );
    assert_int_equal( errorNo, UFS_NO_ERROR );

}
/* ########################################################################## */

/* ufsTrieAdd tests                                                           */
void test_ufs_trie_add_bad_args( void **state )
{
    bool res;
    ufsStatusType errorNo;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;

    ufsTrieStruct = *state;

    res = ufsTrieAdd( NULL, TEST_STR, &errorNo );
    assert_false( res );
    assert_int_equal( errorNo, UFS_BAD_CALL );

    res = ufsTrieAdd( ufsTrieStruct -> trie, NULL, &errorNo );
    assert_false( res );
    assert_int_equal( errorNo, UFS_BAD_CALL );
}

void test_ufs_trie_add( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;
    ufsStatusType errorNo;

    ufsTrieStruct = *state;

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR_0, &errorNo );
    assert_true( res );
    assert_int_equal( errorNo, UFS_NO_ERROR );

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR_1, &errorNo );
    assert_true( res );
    assert_int_equal( errorNo, UFS_NO_ERROR );
}

void test_ufs_trie_add_duplicate( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;
    ufsStatusType errorNo;

    ufsTrieStruct = *state;

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR, &errorNo );
    assert_true( res );
    assert_int_equal( errorNo, UFS_NO_ERROR );

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR, &errorNo );
    assert_false( res );
    assert_int_equal( errorNo, UFS_ALREADY_EXISTS );

}

/* ########################################################################## */

/* ufsTrieExists tests                                                        */
void test_ufs_trie_exists_bad_args( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;
    ufsStatusType errorNo;

    ufsTrieStruct = *state;

    res = ufsTrieExists( NULL, TEST_STR, &errorNo );
    assert_false( res );
    assert_int_equal( errorNo, UFS_BAD_CALL );

    res = ufsTrieExists( ufsTrieStruct -> trie, NULL, &errorNo );
    assert_false( res );
    assert_int_equal( errorNo, UFS_BAD_CALL );
}

void test_ufs_trie_exists_add_then_exists( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;
    ufsStatusType errorNo;

    ufsTrieStruct = *state;

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR_0, &errorNo );
    assert_true( res );
    assert_int_equal( errorNo, UFS_NO_ERROR );

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR_1, &errorNo );
    assert_true( res );
    assert_int_equal( errorNo, UFS_NO_ERROR );

    res = ufsTrieExists( ufsTrieStruct -> trie, TEST_STR_0, &errorNo );
    assert_true( res );
    assert_int_equal( errorNo, UFS_NO_ERROR );

    res = ufsTrieExists( ufsTrieStruct -> trie, TEST_STR_1, &errorNo );
    assert_true( res );
    assert_int_equal( errorNo, UFS_NO_ERROR );

}

void test_ufs_trie_exists_does_not_exist( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;
    ufsStatusType errorNo;

    ufsTrieStruct = *state;

    res = ufsTrieExists( ufsTrieStruct -> trie, TEST_STR, &errorNo );
    assert_false( res );
    assert_int_equal( errorNo, UFS_DOES_NOT_EXIST );
}

/* ########################################################################## */

/* ufsTrie 4096 strings test.                                                 */
void test_ufs_trie_4096_strings( void **state )
{
    int i, j;
    char strings[4096][256];
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;
    bool res;
    ufsStatusType errorNo;

    memset( strings, 0, sizeof( strings ) );

    for ( i = 0; i < 4096; i ++ )
        for ( j = 0; ( rand() % 1000 ) < 950 && j < 255; j++ )
            strings[ i ][ j ] = 'a' + rand() % ( 'z' - 'a' + 1 );

    ufsTrieStruct = *state;

    for ( i = 0; i < 4096; i++ ) { 
        if ( ufsTrieExists( ufsTrieStruct -> trie, strings [ i ], &errorNo ) ) {
            assert_int_equal( errorNo, UFS_NO_ERROR );
            continue;
        }

        assert_int_equal( errorNo, UFS_DOES_NOT_EXIST );
        res = ufsTrieAdd( ufsTrieStruct -> trie, strings [ i ], &errorNo );
        assert_true( res );
        assert_int_equal( errorNo, UFS_NO_ERROR );
    }

    for ( i = 0; i < 4096; i++ ) { 
        res = ufsTrieExists( ufsTrieStruct -> trie, strings [ i ], &errorNo );
        assert_true( res );
        assert_int_equal( errorNo, UFS_NO_ERROR );
    }
}

/* ########################################################################## */

static const struct CMUnitTest ufs_test_trie[] = {
    cmocka_unit_test( test_ufs_trie_init ),

    /* ufsTrieAdd                                                             */
    UFS_TRIE_TEST( test_ufs_trie_add_bad_args ),
    UFS_TRIE_TEST( test_ufs_trie_add ),
    UFS_TRIE_TEST( test_ufs_trie_add_duplicate ),
    /* ====================================================================== */

    /* ufsTrieExists                                                          */
    UFS_TRIE_TEST( test_ufs_trie_exists_bad_args ),
    UFS_TRIE_TEST( test_ufs_trie_exists_add_then_exists ),
    UFS_TRIE_TEST( test_ufs_trie_exists_does_not_exist ),
    /* ====================================================================== */

    /* ufsTrie 4096 strings                                                   */
    UFS_TRIE_TEST( test_ufs_trie_4096_strings )
    /* ====================================================================== */

};

int main( void ) {
    time_t seed;

    seed = time( NULL );
    // seed = 1771532713;
    printf( "SEED IS %lu\n", seed );

    srand( seed );
    return cmocka_run_group_tests( ufs_test_trie, NULL, NULL );
}

#else

int main( void ) {
    return 0;
}

#endif /* UFS_TEST_DISABLE */
