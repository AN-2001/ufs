/******************************************************************************\
*  ufs.h                                                                       *
*                                                                              *
*  Contains the spec for ufs.                                                  *
*  Different implementations are valid as long as they implement this.         *
*                                                                              *
*              Written by A.N.                                  17-01-2026     *
*                                                                              *
\******************************************************************************/

#ifndef UFS_H
#define UFS_H

/*                                                                            */
/* This is the spec for the ufs "union file system" storage backend.          */
/* The goal of this spec is to define the semantics of how ufs represents     */
/* its internal data, in other words: this is the core of ufs.                */
/* Definitions:                                                               */
/* File: An entity represented by a path on a file-system.                    */
/*                                                                            */
/* Directory: a directory on a file system, semantically it should be thought */
/*            or as a container of files in our context.                      */
/*                                                                            */
/* The distinction between files are directories is needed since directories  */
/* are iterable, files are not.                                               */
/*                                                                            */
/* Storage: a file or a directory.                                            */
/*                                                                            */
/* Area: A set of storage represented by a unique name.                       */
/*       areas DO NOT own said storage, they only project it using a name.    */
/*                                                                            */
/* Mapping: A (area, storage) relation, defined as area projects storage.     */
/*                                                                            */
/* View: a list of areas, in our context it can be UFS_MAX_VIEW_SIZE          */
/*       size max. Semantically this is a union of areas:                     */
/*          Let V = ( A0, A1, ..., An )                                       */
/*          Assume we're attempting to resolve some storage s in V            */
/*          If n == 0: Fail, V definitely does not contain s.                 */
/*          Inductive step:                                                   */
/*          Attempt to resolve s in Ak, if it contains it halt.               */
/*          Otherwise continue to k + 1                                       */
/*          Stop once n = k                                                   */
/*                                                                            */
/* BASE: a unique area that refers to the base external filesystem.           */
/*       Most views will end with BASE as they're supposed to shadow it.      */
/*                                                                            */
/* IdentifierType: A numeric unique identifier to a file, area, or a directory*/
/*                 the identifier must be strictly greater than 0.            */
/*                                                                            */
/* Note: BASE has the unique identifier 0.                                    */
/*                                                                            */
/*                                                                            */
/* StatusType: A status that ufs stores in errno, shows the current status    */
/*             of ufs, its set as a side effect of all ufs functions.         */
/*                                                                            */



#define UFS_MAX_VIEW_SIZE (1024)

#include <stdint.h>
#include <sys/types.h>

enum {
    UFS_NO_ERROR = 0,
    UFS_BAD_CALL,
    UFS_ALREADY_EXISTS,
    UFS_CANNOT_RESOLVE_STORAGE,
    UFS_UNKNOWN_ERROR
};

typedef uint8_t ufsStatusType;
typedef int64_t ufsIdentifierType;

typedef void *ufsType;
typedef ufsStatusType (*ufsDirIter)(const char *path);
typedef ufsIdentifierType ufsViewType[ UFS_MAX_VIEW_SIZE ];

extern ufsStatusType ufsErrno;

/******************************************************************************\
* ufsAddDirectory                                                              *
*                                                                              *
*  Adds a directory to ufs.                                                    *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_ALREADY_EXISTS: The directory already exists.                         *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -name: The name of the directory, must not be NULL.                         *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the new directory.             *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsAddDirectory( ufsType ufs,
                                   const char *name );

/******************************************************************************\
* ufsAddFile                                                                   *
*                                                                              *
*  Adds a file to ufs.                                                         *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_ALREADY_EXISTS: The file already exists.                              *
*   -UFS_DOES_NOT_EXIST: The specified directory does not exist.               *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -directory: The directory that contains this file, must be greater than 0.  *
*  -name: The name of the file, must not be NULL.                              *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the new file.                  *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsAddFile( ufsType ufs,
                              ufsIdentifierType directory,     
                              const char *name );

/******************************************************************************\
* ufsAddArea                                                                   *
*                                                                              *
*  Adds a area to ufs.                                                         *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_ALREADY_EXISTS: The area already exists.                              *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -name: The name of the area, must not be NULL.                              *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the new area.                  *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsAddArea( ufsType ufs,
                              const char *name );

/******************************************************************************\
* ufsGetDirectory                                                              *
*                                                                              *
*  Retrieves a directory's unique identifier from ufs.                         *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The directory does not exist in ufs.                  *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -name: The name of the directory, must not be NULL.                         *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the existing directory.        *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsGetDirectory( ufsType ufs,
                                   const char *name );

/******************************************************************************\
* ufsGetFile                                                                   *
*                                                                              *
*  Retrieves a file's unique identifier from ufs.                              *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The file does not exist in ufs.                       *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -name: The name of the file, must not be NULL.                              *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the existing file.             *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsGetFile( ufsType ufs,
                              const char *name );

/******************************************************************************\
* ufsGetArea                                                                   *
*                                                                              *
*  Retrieves a area's unique identifier from ufs.                              *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The area does not exist in ufs.                       *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -name: The name of the area, must not be NULL.                              *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the existing area.             *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsGetArea( ufsType ufs,
                              const char *name );

/******************************************************************************\
* ufsRemoveDirectory                                                           *
*                                                                              *
*  Removes a directory from ufs.                                               *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The directory does not exist in ufs.                  *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -directory: the directory's unique identifier, must be greater than 0.      *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, errno is also set.                 *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsRemoveDirectory( ufsType ufs,
                                  ufsIdentifierType directory );

/******************************************************************************\
* ufsRemoveFile                                                                *
*                                                                              *
*  Removes a file from ufs.                                                    *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The file does not exist in ufs.                       *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -file: the file's unique identifier, must be greater than 0.                *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, errno is also set.                 *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsRemoveFile( ufsType ufs,
                             ufsIdentifierType file );

/******************************************************************************\
* ufsRemoveArea                                                                *
*                                                                              *
*  Removes a area from ufs.                                                    *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The area does not exist in ufs.                       *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -area: the area's unique identifier, must be greater than 0.                *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, errno is also set.                 *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsRemoveArea( ufsType ufs,
                             ufsIdentifierType area );

/******************************************************************************\
* ufsAddMapping                                                                *
*                                                                              *
*  Adds a ufs mapping in the form of (area, storage).                          *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The area or the storage do not exist in ufs.          *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -area: the area's unique identifier, must be greater than 0.                *
*  -storage: the storage's unique identifier, must be greater than 0.          *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, errno is also set.                 *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsAddAMapping( ufsType ufs,
                              ufsIdentifierType area,
                              ufsIdentifierType storage );

/******************************************************************************\
* ufsResolveStorageInView                                                      *
*                                                                              *
*  Given storage and a view, resolve the storage over the view.                *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The storage does not exist in ufs.                    *
*   -UFS_CANNOT_RESOLVE_STORAGE: Could not resolve storage in the view.        *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -view: The view to use.                                                     *
*  -storage: the storage's unique identifier, must be greater than 0.          *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the first area that contains   *
*                      the storage.                                            *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsResolveStorageInView( ufsType ufs,
                                           ufsViewType view,
                                           ufsIdentifierType storage );

/******************************************************************************\
* ufsIterateDirInView                                                          *
*                                                                              *
*  Given a directory and a view, iterate over the directory in the context     *
*  of that view.                                                               *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The directory does not exist in ufs.                  *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -view: The view to use.                                                     *
*  -directory: The directory's unique identifier, must be greater than 0.      *
*  -iterator: The iterator function to apply, must not be NULL.                *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, errno is also set.                 *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsIterateDirInView( ufsType ufs,
                                   ufsViewType view,
                                   ufsIdentifierType directory,
                                   ufsDirIter iterator );

#endif /* UFS_H */
