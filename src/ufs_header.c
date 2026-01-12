/******************************************************************************\
*  ufsHeader.c                                                                 *
*                                                                              *
*  Internal header for describing a ufs header.                                *
*  This also introduces two functions for manipulating headers.                *
*                                                                              *
*              Written by A.N.                                  12-01-2026     *
*                                                                              *
\******************************************************************************/

#include "ufs_header.h"
#include "ufs_defs.h"
#include "ufs_image.h"
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

struct ufsHeaderSizeRequestStruct ufsDefaultSizeRequest = {
    .numFiles = 256,
    .numAreas = 256,
    .numNodes = 512,
    .numStrBytes = 1024
};

static inline uint64_t resolveSize( struct ufsHeaderSizeRequestStruct sizes );
static inline uint64_t roundToBoundary( uint64_t val, uint64_t align );
static inline ufsImagePtr mountHeader( ufsImagePtr img,
        struct ufsHeaderSizeRequestStruct sizes );

ufsImagePtr ufsHeaderInit( const char *path,
                           struct ufsHeaderSizeRequestStruct sizes )
{
    ufsImagePtr ret;
    if (!path || !sizes.numFiles || !sizes.numAreas || !sizes.numNodes || 
            !sizes.numStrBytes ) {
        ufsErrno = UFS_BAD_CALL;
        return NULL;
    }

    /* The assumption is that the path should not exist, we make a new one.   */
    if (access( path, F_OK) == 0) {
        ufsErrno = UFS_BAD_CALL;
        return NULL;
    }

    ret = ufsImageCreate( path, resolveSize( sizes ) );

    /* ufsErrno is already set by ufsImageCreate.                             */
    if (!ret)
        return NULL;

    return ufsHeaderValidate( mountHeader( ret, sizes ) );
}

ufsImagePtr ufsHeaderValidate( ufsImagePtr img )
{
    if (!img) {
        ufsErrno = UFS_BAD_CALL;
        return NULL;
    }

    struct ufsHeaderSizeRequestStruct sizes;
    struct ufsHeaderStruct
        *header = ufsHeaderGet( img );
    uint64_t size, expectedSize,
        minSize = roundToBoundary( sizeof( uint64_t ),
                _Alignof( struct ufsHeaderStruct ) ) +
                 sizeof( struct ufsHeaderStruct );


    size = *(uint64_t*)img;

    if ( size < minSize ) {
        ufsErrno = UFS_IMAGE_TOO_SMALL;
        ufsImageFree( img );
        return NULL;
    }

    if (header -> magicNumber != UFS_MAGIC_NUMBER) {
        ufsErrno = UFS_IMAGE_IS_CORRUPTED;
        ufsImageFree( img );
        return NULL;
    }

    if (header -> version != UFS_VERSION ) {
        ufsErrno = UFS_VERSION_MISMATCH;
        ufsImageFree( img );
        return NULL;
    }

    sizes.numFiles = header -> sizes[ UFS_TYPES_FILE ];
    sizes.numAreas = header -> sizes[ UFS_TYPES_AREA ];
    sizes.numNodes = header -> sizes[ UFS_TYPES_NODE ];
    sizes.numStrBytes = header -> sizes[ UFS_TYPES_STRING ];

    expectedSize = resolveSize( sizes );
    if ( size < expectedSize ) {
        ufsErrno = UFS_IMAGE_BAD_SIZE;
        ufsImageFree( img );
        return NULL;
    }

    ufsErrno = UFS_NO_ERROR;
    return img;
}

struct ufsHeaderStruct *ufsHeaderGet( ufsImagePtr img )
{
    if (!img) {
        ufsErrno = UFS_BAD_CALL;
        return NULL;
    }
    return (struct ufsHeaderStruct*)((uint8_t*)img +
            roundToBoundary( sizeof( uint64_t ), _Alignof(struct ufsHeaderStruct)));
}

static inline ufsImagePtr mountHeader( ufsImagePtr img,
                        struct ufsHeaderSizeRequestStruct sizes )
{
    uint64_t offset;
    struct ufsHeaderStruct
        *header = ufsHeaderGet( img );

    header -> magicNumber = UFS_MAGIC_NUMBER;
    header -> version = UFS_VERSION;

    header -> sizes[ UFS_TYPES_FILE ] = sizes.numFiles;
    header -> sizes[ UFS_TYPES_AREA ] = sizes.numAreas;
    header -> sizes[ UFS_TYPES_NODE ] = sizes.numNodes;
    header -> sizes[ UFS_TYPES_STRING ] = sizes.numStrBytes;

    offset = sizeof( uint64_t );
    offset = roundToBoundary( offset, _Alignof( struct ufsHeaderStruct ) );
    offset += sizeof( struct ufsHeaderStruct );


    offset = roundToBoundary( offset, _Alignof( struct ufsFileStruct ) );
    header -> offsets[ UFS_TYPES_FILE ] = offset;
    offset += sizeof( struct ufsFileStruct ) * sizes.numFiles;

    offset = roundToBoundary( offset, _Alignof( struct ufsAreaStruct ) );
    header -> offsets[ UFS_TYPES_AREA ] = offset;
    offset += sizeof( struct ufsAreaStruct ) * sizes.numAreas;

    offset = roundToBoundary( offset, _Alignof( struct ufsNodeStruct ) );
    header -> offsets[ UFS_TYPES_NODE ] = offset;
    offset += sizeof( struct ufsNodeStruct ) * sizes.numNodes;

    offset = roundToBoundary( offset, _Alignof( char ) );
    header -> offsets[ UFS_TYPES_STRING ] = offset;

    return img;
}

static inline uint64_t resolveSize( struct ufsHeaderSizeRequestStruct sizes )
{
    uint64_t
        pageSize = sysconf( _SC_PAGESIZE  ),
        size = sizeof( uint64_t );

    size = roundToBoundary( size, _Alignof( struct ufsHeaderStruct ) );
    size += sizeof( struct ufsHeaderStruct );

    size = roundToBoundary( size, _Alignof( struct ufsFileStruct ) );
    size += sizeof( struct ufsFileStruct ) * sizes.numFiles;

    size = roundToBoundary( size, _Alignof( struct ufsAreaStruct ) );
    size += sizeof( struct ufsAreaStruct ) * sizes.numAreas;

    size = roundToBoundary( size, _Alignof( struct ufsNodeStruct ) );
    size += sizeof( struct ufsNodeStruct ) * sizes.numNodes;

    size = roundToBoundary( size, _Alignof( char ) );
    size += sizeof( char ) * sizes.numStrBytes;

    size = roundToBoundary( size, pageSize );

    return size;
}

static inline uint64_t roundToBoundary( uint64_t val, uint64_t align )
{
    return (((val) + ((align) - 1)) & ~((align) - 1));
}
