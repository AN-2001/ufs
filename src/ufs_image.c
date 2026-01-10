/******************************************************************************\
*  ufs_image.c                                                                 *
*                                                                              *
*  Contains the definitions for the ufs image.                                 *
*                                                                              *
*              Written by A.N.                                  08-01-2026     *
*                                                                              *
\******************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ufs_defs.h"
#include "ufs_image.h"
#include <unistd.h>

struct ufsSizeRequestStruct
        ufsDefaultSizeRequest = (struct ufsSizeRequestStruct){
                            .fileCount = 128,
                            .areaCount = 128,
                            .bstNodeCount = 128 };

ufsStatusType ufsErrno = 0;

ufsImagePtr ufsImageOpen( const char *filePath )
{
    int fd;
    struct stat sb;
    ufsImagePtr ret;
    uint64_t *size;

    if ( !filePath ) {
        ufsErrno = UFS_BAD_CALL;
        return NULL;
    }

    if ( access( filePath, F_OK ) ) {
        ufsErrno = UFS_IMAGE_DOES_NOT_EXIST;
        return NULL;
    }

    fd = open( filePath, O_RDWR );

    if ( fd == -1 ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        perror( "open" );
        return NULL;
    }

    if ( fstat( fd, &sb ) == -1 ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        perror( "fstat" );
        close( fd );
        return NULL;
    }

    if ( sb.st_size < sizeof( uint64_t ) ) {
        ufsErrno = UFS_IMAGE_TOO_SMALL;
        close( fd );
        return NULL;
    }

    ret = mmap( NULL, sb.st_size, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0 );
    if ( ret == MAP_FAILED ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        perror( "mmap" );
        close( fd );
        return NULL;
    }

    size = ret;
    *size = sb.st_size;

    close( fd );
    ufsErrno = UFS_NO_ERROR;
    return ret;
}

ufsImagePtr ufsImageCreate( const char *filePath, uint64_t size )
{
    int fd;
    ufsImagePtr ret;
    uint64_t *sizePtr;

    if ( !filePath || size < sizeof( uint64_t ) ) {
        ufsErrno = UFS_BAD_CALL;
        return NULL;
    }

    fd = open( filePath, O_CREAT | O_RDWR, 0644 );
    if (fd == -1) {
        if (errno == EACCES) 
            ufsErrno = UFS_CANT_CREATE_FILE;
        else
            ufsErrno = UFS_BAD_CALL;
        perror( "open" );
        return NULL;
    }

    if ( ftruncate( fd, size ) != 0 ) {
        perror( "ftruncate" );
        close(fd);
        return NULL;
    }

    ret = mmap( NULL, size, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0 );
    if ( ret == MAP_FAILED ) {
        ufsErrno = UFS_UNKNOWN_ERROR;
        perror( "mmap" );
        close( fd );
        return NULL;
    }

    close( fd );

    sizePtr = ret;
    *sizePtr = size;

    ufsErrno = UFS_NO_ERROR;
    return ret;
}

bool ufsImageSync( ufsImagePtr image )
{
    uint64_t size;

    size = *(uint64_t*)image;

    if ( msync( image, size, MS_SYNC ) < 0 ) {
        perror( "msync" );
        ufsErrno = UFS_IMAGE_COULD_NOT_SYNC;
        return false;
    }

    return true;
}

void ufsImageFree( ufsImagePtr image )
{
    uint64_t size;

    size = *(uint64_t*)image;

    munmap( image, size );
}
