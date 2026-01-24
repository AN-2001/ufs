#include "ufs_core.h"

const char *ufsStatusStrings[ UFS_NUM_ERRORS ] = {
#define UFS_X( name, val ) #name, 
    UFS_STATUS_LIST
#undef UFS_X
};

ufsStatusType ufsErrno = UFS_NO_ERROR;
