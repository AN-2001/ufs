/******************************************************************************\
*  utils.h                                                                     *
*                                                                              *
*  Contains common testing utilities.                                          *
*                                                                              *
*              Written by A.N.                                  11-01-2026     *
*                                                                              *
\******************************************************************************/


#ifndef UFS_TESTING
#error "Attempting to use test utility functions outside of tests, utilities aren't safe for proper use."
#endif

#ifndef UFS_TEST_UTILS_H
#define UFS_TEST_UTILS_H

#include <stdbool.h>
#include "ufs_core.h"
#include "ufs_trie.h"

#define ASSERT_UFS_ERROR( returnVal, err ) \
    do { \
        assert_true( ( returnVal ) < 0 ); \
        assert_int_equal( errorNo, ( err ) ); \
    } while ( 0 )

#define ASSERT_UFS_STATUS( returnVal, err ) \
    do { \
        assert_false( ( returnVal ) ); \
        assert_int_equal( errorNo, ( err ) ); \
    } while ( 0 )

#define ASSERT_UFS_NO_ERROR( returnVal ) \
    do { \
        assert_true( ( returnVal ) >= 0 ); \
        assert_int_equal( errorNo, UFS_NO_ERROR ); \
    } while ( 0 )

#define ASSERT_UFS_STATUS_NO_ERROR( returnVal ) \
    do { \
        assert_true( ( returnVal ) ); \
        assert_int_equal( errorNo, UFS_NO_ERROR ); \
    } while ( 0 )

struct ufsTestUfsStateStruct {
    ufsType ufs;
};

struct ufsTestUfsTrieStateStruct {
    ufsTrieType trie;
};

int ufsGetInstance( void **state );
int ufsCleanup( void **state );

int ufsTrieGetInstance( void **state );
int ufsTrieCleanup( void **state );

#endif /* UFS_TEST_UTILS_H */
