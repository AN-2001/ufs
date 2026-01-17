/******************************************************************************\
*  ufs_defs.h                                                                  *
*                                                                              *
*  Contains basic definitions for ufs, such as error codes, magic numbers,     *
*  preset file paths, etc...                                                   *
*                                                                              *
*              Written by A.N.                                  10-01-2026     *
*                                                                              *
\******************************************************************************/

#ifndef UFS_DEFS_H
#define UFS_DEFS_H

#include <stdint.h>
#include <sys/types.h>

/* Increment on every ufs update, used to validate compatibility.             */
#define UFS_VERSION (1) 

/* contains the word ufs followed by 0, sanity check for corruption.          */
#define UFS_MAGIC_NUMBER (0x00736675)
#define UFS_DIRECTORY (".ufs")
#define UFS_IMAGE_FILE UFS_DIRECTORY ("ufs_index")

enum {
    UFS_NO_ERROR = 0,
    UFS_IMAGE_DOES_NOT_EXIST,
    UFS_IMAGE_IS_CORRUPTED,
    UFS_VERSION_MISMATCH,
    UFS_BAD_CALL,
    UFS_AREA_ALREADY_EXISTS,
    UFS_OUT_OF_MEMORY,
    UFS_AREA_DOES_NOT_EXIST,
    UFS_FILE_ALREADY_EXISTS,
    UFS_FILE_DOES_NOT_EXIST,
    UFS_MAPPING_ALREADY_EXISTS,
    UFS_CANT_CREATE_FILE,
    UFS_UNKNOWN_ERROR,
    UFS_IMAGE_TOO_SMALL,
    UFS_IMAGE_COULD_NOT_SYNC,
    UFS_IMAGE_BAD_SIZE,
};

typedef uint8_t ufsStatusType;
extern ufsStatusType ufsErrno;

enum ufsTyepesEnum {
    UFS_TYPES_FILE = 0,
    UFS_TYPES_AREA,
    UFS_TYPES_NODE,
    UFS_TYPES_STRING,
    UFS_TYPES_COUNT,
};

typedef int64_t ufsIdType;


#endif /* UFS_DEFS_H */
