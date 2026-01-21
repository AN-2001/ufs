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

bool ufsTestUtilsGetTmpFileName( struct ufsTestUtilsFileNameStruct *fn )
{
    char template[] = "/tmp/ufsTempXXXXXXX";
    char *name;

    name = mktemp( template );

    if ( !name || name[0] == '\0' ) {
        perror( "mktemp" );
        return false;
    }

    fn -> fd = -1;

    memset( fn -> name, 0, sizeof( char ) * UFS_TEST_UTILS_BUFF_SIZE );
    strcpy( fn -> name, template );
    return true;
}

bool ufsTestUtilsGetTmpFile( struct ufsTestUtilsFileNameStruct *fn )
{
    char template[] = "/tmp/ufsTempXXXXXXX";

    fn -> fd = mkstemp( template );

    if ( fn -> fd < 0 ) {
        perror( "mkstemp" );
        return false;
    }

    memset( fn -> name, 0, sizeof( char ) * UFS_TEST_UTILS_BUFF_SIZE );
    strcpy( fn -> name, template );
    return true;
}

int getFileNameSetup( void **state )
{
    struct ufsTestUtilsFileNameStruct *fn;

    fn = malloc(sizeof(*fn));
    if (!fn)
        return -1;

    if ( !ufsTestUtilsGetTmpFileName( fn ) )
        return -1;

    *state = fn;
    return 0;
}

int getFileSetup( void **state )
{
    struct ufsTestUtilsFileNameStruct *fn;

    fn = malloc(sizeof(*fn));
    if (!fn)
        return -1;

    if ( !ufsTestUtilsGetTmpFile( fn ) )
        return -1;

    *state = fn;
    return 0;
}

int cleanUpTeardown( void **state )
{
    struct ufsTestUtilsFileNameStruct *fn;

    fn = *state;

    if (fn -> fd > 0)
        close( fn -> fd );
    if ( fn -> name[0] )
        unlink( fn -> name );

    free(*state);
    *state = NULL;
    return 0;
}


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
