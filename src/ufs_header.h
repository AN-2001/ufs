/******************************************************************************\
*  ufsHeader.h                                                                 *
*                                                                              *
*  Internal header for describing a ufs header.                                *
*  This also introduces two functions for manipulating headers.                *
*                                                                              *
*              Written by A.N.                                  12-01-2026     *
*                                                                              *
\******************************************************************************/

#ifndef UFS_HEADER_H
#define UFS_HEADER_H

#include <stdint.h>
#include "ufs_defs.h"
#include "ufs_image.h"

struct ufsFileStruct {
    uint8_t isOwend;
    uint64_t strOffset;
};

struct ufsAreaStruct {
    uint8_t isOwend;
    uint64_t strOffset;
};

struct ufsNodeStruct {
    uint8_t isOwend;
    ufsIdType left, right;
    ufsIdType key[2];
    uint8_t numKeys;
};

struct ufsHeaderStruct {
    uint32_t magicNumber;
    uint32_t version;

    uint64_t sizes[ UFS_TYPES_COUNT ],
             offsets[ UFS_TYPES_COUNT ];
};

struct ufsHeaderSizeRequestStruct {
    uint64_t numFiles;
    uint64_t numAreas;
    uint64_t numNodes;
    uint64_t numStrBytes;
};

extern struct ufsHeaderSizeRequestStruct ufsDefaultSizeRequest;

/******************************************************************************\
* ufsHeaderInit                                                                *
*                                                                              *
*  Creates a new ufs image at path and initializes a ufs header.               *
*  The image size is determined by sizes, the resulting size is guaranteed     *
*  to conform to it.                                                           *
*  If path already exists a new header will not be created.                    *
*  Possible errors:                                                            *
*    UFS_BAD_CALL: if path or sizes are invalid.                               *
*    All errors of ufsImageCreate                                              *
*    All erors of ufsHeaderValidate                                            *
*                                                                              *
*  NOTE: this function will sync the image after writing the header.           *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -path: The path to the new ufs image.                                       *
*  -sizes: The size specification for the ufs image.                           *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsImagePtr: a new ufs image with the header mounted.                      *
*                                                                              *
\******************************************************************************/
ufsImagePtr ufsHeaderInit( const char *path,
                           struct ufsHeaderSizeRequestStruct sizes );

/******************************************************************************\
* ufsHeaderValidate                                                            *
*                                                                              *
*  Validates a ufs image and checks that it contains a valid header.           *
*  The checks are as follows:                                                  *
*    1.The magic number is set correctly.                                      *
*    2.The version of the header is compatible with this client.               *
*    3.The image is large enough to atleast contain the header.                *
*    4.The image conforms to the sizes specified in the header.                *
*                                                                              *
*  Possible Errors:                                                            *
*    UFS_BAD_CALL: If the input image is badly formed.                         *
*    UFS_IMAGE_TOO_SMALL: If the image is too small to contain a header.       *
*    UFS_IMAGE_IS_CORRUPTED: If the magic number is corrupted.                 *
*    UFS_VERSION_MISMATCH: If the version in the image does not match the      *
*                          client.                                             *
*    UFS_IMAGE_BAD_SIZE: If the image does not conform to the size spec in the *
*                         header.                                              *
*                                                                              *
*  NOTE: If the image happens to be invalid, this function will free it.       *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -img: a ufs image.                                                          *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsImagePtr: The same ufs image if valid, NULL otherwise.                  *
*                                                                              *
\******************************************************************************/
ufsImagePtr ufsHeaderValidate( ufsImagePtr img );

/******************************************************************************\
* ufsHeaderGet                                                                 *
*                                                                              *
*  Assuming img is valid, this function gets the header portion out of it.     *
*                                                                              *
*  Possible Errors:                                                            *
*   UFS_BAD_CALL: if img is NULL.                                              *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -img: the ufs image.                                                        *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -struct ufsHeaderStruct*: The header portion inside img, NULL if img is bad *
*                                                                              *
\******************************************************************************/
struct ufsHeaderStruct *ufsHeaderGet( ufsImagePtr img );

#endif /* UFS_HEADER_H */
