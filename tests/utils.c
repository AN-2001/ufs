/******************************************************************************\
*  utils.c                                                                     *
*                                                                              *
*  Contains common testing utilities.                                          *
*                                                                              *
*              Written by A.N.                                  11-01-2026     *
*                                                                              *
\******************************************************************************/

#define UFS_TESTING

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ufs_core.h"
#include "ufs_trie.h"
#include <unistd.h>
#include "utils.h"

int ufsGetInstance( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType errorNo;

    ufsStruct = malloc( sizeof( *ufsStruct ) );
    if (!ufsStruct) {
        return -1;
    }

    ufsStruct -> ufs = ufsInit( &errorNo );
    if ( !ufsStruct -> ufs ) {
        printf("Encountered ufs error: %s\n", ufsStatusStrings[ errorNo ] );
        return -1;
    }

    *state = ufsStruct;
    return 0;
}

int ufsCleanup( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;
    ufsStatusType errorNo;

    ufsStruct = *state;

    if ( ufsStruct -> ufs ) {
        ufsDestroy( ufsStruct -> ufs, &errorNo );
        if ( errorNo ) {
            printf("Encountered ufs error: %s\n", ufsStatusStrings[ errorNo ] );
            return -1;
        }
    }

    free( ufsStruct );
    *state = NULL;
    return 0;
}

int ufsTrieGetInstance( void **state )
{
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;
    ufsStatusType errorNo;

    ufsTrieStruct = malloc( sizeof( *ufsTrieStruct ) );
    if (!ufsTrieStruct) {
        return -1;
    }

    ufsTrieStruct -> trie = ufsTrieInit( &errorNo );
    if ( !ufsTrieStruct -> trie ) {
        printf("Encountered ufs error: %s\n", ufsStatusStrings[ errorNo ] );
        return -1;
    }

    *state = ufsTrieStruct;
    return 0;
}

int ufsTrieCleanup( void **state )
{
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;
    ufsStatusType errorNo;

    ufsTrieStruct = *state;

    if ( ufsTrieStruct -> trie ) {
        ufsTrieDestroy( ufsTrieStruct -> trie, &errorNo );
        if ( errorNo ) {
            printf("Encountered ufs error: %s\n", ufsStatusStrings[ errorNo ] );
            return -1;
        }
    }

    free( ufsTrieStruct );
    *state = NULL;
    return 0;
}
