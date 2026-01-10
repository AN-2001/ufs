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

/* Increment on every index update, used to validate compatibility.           */
#define UFS_INDEX_VERSION (1) 

/* contains the word ufs followed by 0, sanity check for corruption.          */
#define UFS_MAGIC_NUMBER (0x75667300)
#define UFS_DIRECTORY (".ufs")
#define UFS_IMAGE_FILE UFS_DIRECTORY ("ufs_index")

#define UFS_NO_ERROR (0)
#define UFS_IMAGE_DOES_NOT_EXIST (-1)
#define UFS_IMAGE_IS_CORRUPTED (-2)
#define UFS_VERSION_MISMATCH (-3)
#define UFS_BAD_CALL (-4)
#define UFS_AREA_ALREADY_EXISTS (-5)
#define UFS_OUT_OF_MEMORY (-6)
#define UFS_AREA_DOES_NOT_EXIST (-7)
#define UFS_FILE_ALREADY_EXISTS (-8)
#define UFS_FILE_DOES_NOT_EXIST (-9)
#define UFS_MAPPING_ALREADY_EXISTS (-10)
#define UFS_CANT_CREATE_FILE (-11)
#define UFS_UNKNOWN_ERROR (-12)
#define UFS_IMAGE_TOO_SMALL (-12)
#define UFS_IMAGE_COULD_NOT_SYNC (-13)

typedef int64_t ufsIdType;

/* Errors are defined as negative, so that functions that could return        */
/* ufsIdType could also return errors.                                        */
typedef int64_t ufsStatusType;

struct ufsSizeRequestStruct {
    uint64_t fileCount;
    uint64_t areaCount;
    /* We'll assume it's | files X areas |, still lets expose it for the      */
    /* future.                                                                */
    uint64_t bstNodeCount; 
};

extern struct ufsSizeRequestStruct ufsDefaultSizeRequest;

extern ufsStatusType ufsErrno;

#endif /* UFS_DEFS_H */
