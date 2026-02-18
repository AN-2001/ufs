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


    ufsStruct = malloc( sizeof( *ufsStruct ) );
    if (!ufsStruct) {
        return -1;
    }

    ufsStruct -> ufs = ufsInit();
    if ( !ufsStruct -> ufs ) {
        printf("Encountered ufs error: %s\n", ufsStatusStrings[ ufsErrno ] );
        return -1;
    }

    *state = ufsStruct;
    return 0;
}

int ufsCleanup( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;

    ufsStruct = *state;

    if ( ufsStruct -> ufs ) {
        ufsDestroy( ufsStruct -> ufs );
        if (ufsErrno) {
            printf("Encountered ufs error: %s\n", ufsStatusStrings[ ufsErrno ] );
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


    ufsTrieStruct = malloc( sizeof( *ufsTrieStruct ) );
    if (!ufsTrieStruct) {
        return -1;
    }

    ufsTrieStruct -> trie = ufsTrieInit();
    if ( !ufsTrieStruct -> trie ) {
        printf("Encountered ufs error: %s\n", ufsStatusStrings[ ufsErrno ] );
        return -1;
    }

    *state = ufsTrieStruct;
    return 0;
}

int ufsTrieCleanup( void **state )
{
    struct ufsTestUfsTrieStateStruct *ufsTrieStruct;

    ufsTrieStruct = *state;

    if ( ufsTrieStruct -> trie ) {
        ufsTrieDestroy( ufsTrieStruct -> trie );
        if (ufsErrno) {
            printf("Encountered ufs error: %s\n", ufsStatusStrings[ ufsErrno ] );
            return -1;
        }
    }

    free( ufsTrieStruct );
    *state = NULL;
    return 0;
}
