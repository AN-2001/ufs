/******************************************************************************\
*  ufs_image.h                                                                 *
*                                                                              *
*  Contains the definitions for the ufs image.                                 *
*                                                                              *
*              Written by A.N.                                  08-01-2026     *
*                                                                              *
\******************************************************************************/

/* Notes:                                                                    */
/* Note that a ufsImage by itself does not hold any semantics.                */
/* The intention is usf_image only manages low level management of memory     */
/* images.                                                                    */
/* Regardless, we put some meta-data (the size) in images.                    */

#ifndef UFS_IMAGE_H
#define UFS_IMAGE_H

#include <stdbool.h>
#include <sys/types.h>
#include "ufs_defs.h"

typedef void *ufsImagePtr;

/******************************************************************************\
* ufsImageOpen                                                                 *
*                                                                              *
*  Opens an existing ufs image and returns it.                                 *
*                                                                              *
*  Possible errors:                                                            *
*    On error, will return NULL and set ufsErrno to one of the following:      *
*    * UFS_DOES_NOT_EXIST: Ufs image does not exist( bad filepath... )         *
*    * UFS_IMAGE_TOO_SMALL: The loaded image is too small.                     *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -filePath: The path of the image file.                                      *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsImagePtr: The opened ufs image.                                         *
*                                                                              *
\******************************************************************************/
ufsImagePtr ufsImageOpen( const char *filePath );

/******************************************************************************\
* ufsImageCreate                                                               *
*                                                                              *
*  Creates a file backed memory region that would fit size bytes.             .*
*                                                                              *
*  Possible errors:                                                            *
*    On error, will return NULL and set ufsErrno to one of the following:      *
*    * UFS_CANT_CREATE_FILE: Ufs failed to create filePath.                    *
*    * UFS_BAD_CALL: The size request is bad or the filepath is NULL.          *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -filePath: The path of the image file.                                      *
*  -size: The size of the ufs image in bytes.                                  *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsImagePtr: The allocated memory.                                         *
*                                                                              *
\******************************************************************************/
ufsImagePtr ufsImageCreate( const char *filePath, uint64_t size );

/******************************************************************************\
* ufsImageSync                                                                 *
*                                                                              *
*  Syncs the in-memory ufs image with the backing file.                        *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -img: the ufs image.                                                        *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -bool: true on success, false otherwise.                                    *
*                                                                              *
\******************************************************************************/
bool ufsImageSync( ufsImagePtr image );

/******************************************************************************\
* ufsImageFree                                                                 *
*                                                                              *
*  Invalidates the memory pointed to by image.                                 *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -image: the ufs image.                                                      *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -void.                                                                      *
*                                                                              *
\******************************************************************************/
void ufsImageFree( ufsImagePtr image );

#endif /* UFS_IMAGE_H */
