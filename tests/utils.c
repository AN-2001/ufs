/******************************************************************************\
*  utils.c                                                                     *
*                                                                              *
*  Contains common testing utilities.                                          *
*                                                                              *
*              Written by A.N.                                  11-01-2026     *
*                                                                              *
\******************************************************************************/

#include "ufs.h"
#include <unistd.h>
#define UFS_TESTING

#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

int ufsGetInstance( void **state )
{
    struct ufsTestUfsStateStruct *ufsStruct;


    ufsStruct = malloc( sizeof( *ufsStruct ) );
    if (!ufsStruct) {
        return -1;
    }

    ufsStruct -> ufs = ufsInit();
    if ( !ufsStruct -> ufs ) {
        printf("Encountered ufs error: %s", ufsStatusStrings[ ufsErrno ] );
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
            printf("Encountered ufs error: %s", ufsStatusStrings[ ufsErrno ] );
            return -1;
        }
    }

    free( ufsStruct );
    *state = NULL;
    return 0;
}
