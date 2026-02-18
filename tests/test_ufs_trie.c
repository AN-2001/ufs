/******************************************************************************\
*  test_ufs_trie.c                                                             *
*                                                                              *
*  Unit tests for ufs trie.                                                    *
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
#include <ufs_trie.h>
#include "utils.h"

#include <cmocka.h>

/* ########################################################################## */

static const struct CMUnitTest ufs_test_trie[] = {

};

int main( void ) {
    return cmocka_run_group_tests( ufs_test_trie, NULL, NULL );
}

#else

int main( void ) {
    return 0;
}

#endif /* UFS_TEST_DISABLE */
