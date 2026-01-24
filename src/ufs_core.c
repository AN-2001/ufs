/******************************************************************************\
*  ufs_core.c                                                                  *
*                                                                              *
*  Correspdonding .c part of ufs_core.h. Used to link global symbols.          *
*                                                                              *
*              Written by A.N.                                  24-01-2026     *
*                                                                              *
\******************************************************************************/

#include "ufs_core.h"

const char *ufsStatusStrings[ UFS_NUM_ERRORS ] = {
#define UFS_X( name, val ) #name, 
    UFS_STATUS_LIST
#undef UFS_X
};

ufsStatusType ufsErrno = UFS_NO_ERROR;
