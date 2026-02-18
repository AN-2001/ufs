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

#include <cmocka.h>

#define UFS_TRIE_TEST( name ) \
    cmocka_unit_test_setup_teardown( name, ufsTrieGetInstance, ufsTrieCleanup )

#define TEST_STR TEST_STR_0
#define TEST_STR_0 ("test_0")
#define TEST_STR_1 ("test_1")

/* ufsTrieInit tests                                                          */
void test_ufs_trie_init( void **state )
{
    ufsTrieType trie;

    trie = ufsTrieInit();

    assert_non_null( trie );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    ufsTrieDestroy( trie );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    ufsTrieDestroy( NULL );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

}
/* ########################################################################## */

/* ufsTrieAdd tests                                                           */
void test_ufs_trie_add_bad_args( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;

    ufsTrieStruct = *state;

    res = ufsTrieAdd( NULL, TEST_STR );
    assert_false( res );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );

    res = ufsTrieAdd( ufsTrieStruct -> trie, NULL );
    assert_false( res );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
}

void test_ufs_trie_add( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;

    ufsTrieStruct = *state;

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR_0 );
    assert_true( res );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR_1 );
    assert_true( res );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );
}

void test_ufs_trie_add_duplicate( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;

    ufsTrieStruct = *state;

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR );
    assert_true( res );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR );
    assert_false( res );
    assert_int_equal( ufsErrno, UFS_ALREADY_EXISTS );

}

/* ########################################################################## */

/* ufsTrieExists tests                                                        */
void test_ufs_trie_exists_bad_args( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;

    ufsTrieStruct = *state;

    res = ufsTrieExists( NULL, TEST_STR );
    assert_false( res );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );

    res = ufsTrieExists( ufsTrieStruct -> trie, NULL );
    assert_false( res );
    assert_int_equal( ufsErrno, UFS_BAD_CALL );
}

void test_ufs_trie_exists_add_then_exists( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;

    ufsTrieStruct = *state;

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR_0 );
    assert_true( res );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    res = ufsTrieAdd( ufsTrieStruct -> trie, TEST_STR_1 );
    assert_true( res );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    res = ufsTrieExists( ufsTrieStruct -> trie, TEST_STR_0 );
    assert_true( res );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

    res = ufsTrieExists( ufsTrieStruct -> trie, TEST_STR_1 );
    assert_true( res );
    assert_int_equal( ufsErrno, UFS_NO_ERROR );

}

void test_ufs_trie_exists_does_not_exist( void **state )
{
    bool res;
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;

    ufsTrieStruct = *state;

    res = ufsTrieExists( ufsTrieStruct -> trie, TEST_STR );
    assert_false( res );
    assert_int_equal( ufsErrno, UFS_DOES_NOT_EXIST );
}

/* ########################################################################## */

/* ufsTrie 1000 strings test.                                                 */
void test_ufs_trie_1000_strings( void **state )
{

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

    /* ufsTrie 1000 strings                                                   */
    UFS_TRIE_TEST( test_ufs_trie_1000_strings )
    /* ====================================================================== */

};

int main( void ) {
    return cmocka_run_group_tests( ufs_test_trie, NULL, NULL );
}

#else

int main( void ) {
    return 0;
}

#endif /* UFS_TEST_DISABLE */
