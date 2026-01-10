/******************************************************************************\
*  ufs_index.h                                                                 *
*                                                                              *
*  Contains the APIs for interacting with the ufs index.                       *
*                                                                              *
*              Written by A.N.                                  08-01-2026     *
*                                                                              *
\******************************************************************************/

#ifndef UFS_INDEX_H
#define UFS_INDEX_H

#include <stdbool.h>
#include <sys/types.h>
#include "ufs_defs.h"
#include "ufs_image.h"

/******************************************************************************\
* ufsIndexAddArea                                                              *
*                                                                              *
*  Add a new area to the ufs index.                                            *
*                                                                              *
*  possible errors:                                                            *
*    UFS_BAD_CALL: Badly formatted call, either ufsIndex or areaName are       *
*                  malformed.                                                  *
*    UFS_AREA_ALREADY_EXISTS: The given areaName already exists in the system. *
*    UFS_OUT_OF_MEMORY: The index is out of memory, user should resize then    *
*                       retry.                                                 *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsIndex: The opaque ufs index structure.                                  *
*  -areaName: the area name.                                                   *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdType: the internal area id.                                           *
*                                                                              *
\******************************************************************************/
ufsIdType ufsIndexAddArea( ufsImagePtr ufsIndex,
                           const char *areaName );

/******************************************************************************\
* ufsIndexRemoveArea                                                           *
*                                                                              *
*  Given an area id, remove it from the ufs index.                             *
*                                                                              *
*  possible errors:                                                            *
*    UFS_BAD_CALL: Badly formatted call, either ufsIndex or areaName are       *
*                  malformed.                                                  *
*    UFS_AREA_DOES_NOT_EXIST: area id does not point to a valid area.          *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsIndex: The opaque ufs index structure.                                  *
*  -area: The internal area id as returned by ufsIndexAddArea.                 *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call.                                    *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsIndexRemoveArea( ufsImagePtr ufsIndex,
                                  ufsIdType area );

/******************************************************************************\
* ufsIndexFindArea                                                             *
*                                                                              *
*  Given a ufs index and an areaName, find the ufs id of that name.            *
*                                                                              *
*  possible errors:                                                            *
*    UFS_BAD_CALL: Badly formatted call, either ufsIndex or areaName are       *
*                  malformed.                                                  *
*    UFS_AREA_DOES_NOT_EXIST: areaName does not exist in the index.            *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsIndex: The opaque ufs index structure.                                  *
*  -areaName: the area name.                                                   *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdType: the internal id of areaName.                                    *
*                                                                              *
\******************************************************************************/
ufsIdType ufsIndexFindArea( ufsImagePtr ufsIndex,
                            const char *areaName );

/******************************************************************************\
* ufsIndexAddFile                                                              *
*                                                                              *
*  Add a new file to the ufs index.                                            *
*                                                                              *
*  possible errors:                                                            *
*    UFS_BAD_CALL: Badly formatted call, either ufsIndex or fileName are       *
*                  malformed.                                                  *
*    UFS_FILE_ALREADY_EXISTS: The given fileName already exists in the system. *
*    UFS_OUT_OF_MEMORY: The index is out of memory, user should resize then    *
*                       retry.                                                 *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsIndex: The opaque ufs index structure.                                  *
*  -fileName: the file name.                                                   *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdType: the internal file id.                                           *
*                                                                              *
\******************************************************************************/
ufsIdType ufsIndexAddFile( ufsImagePtr ufsIndex,
                           const char *fileName );

/******************************************************************************\
* ufsIndexRemoveFile                                                           *
*                                                                              *
*  Given an file id, remove it from the ufs index.                             *
*                                                                              *
*  possible errors:                                                            *
*    UFS_BAD_CALL: Badly formatted call, either ufsIndex or fileName are       *
*                  malformed.                                                  *
*    UFS_FILE_DOES_NOT_EXIST: file id does not point to a valid file.          *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsIndex: The opaque ufs index structure.                                  *
*  -file: The internal file id as returned by ufsIndexAddFile.                 *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call.                                    *
*                                                                              *
\******************************************************************************/
void ufsIndexRemoveFile( ufsImagePtr ufsImage,
                         ufsIdType file );

/******************************************************************************\
* ufsIndexFindFile                                                             *
*                                                                              *
*  Given a ufs index and a fileName, find the ufs id of that name.             *
*                                                                              *
*  possible errors:                                                            *
*    UFS_BAD_CALL: Badly formatted call, either ufsIndex or areaName are       *
*                  malformed.                                                  *
*    UFS_FILE_DOES_NOT_EXIST: fileName does not exist in the index.            *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsIndex: The opaque ufs index structure.                                  *
*  -fileName: the file name.                                                   *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdType: the internal id of fileName.                                    *
*                                                                              *
\******************************************************************************/
ufsIdType ufsIndexFindFile( ufsImagePtr ufsIndex,
                            const char *fileName );

/******************************************************************************\
* ufsIndexAddMapping                                                           *
*                                                                              *
*  Given an (area, file) pair, add them to the internal structure.             *
*  The relation defined above is defined as "area contains file"               *
*                                                                              *
*  possible errors:                                                            *
*    UFS_BAD_CALL: Badly formatted call, either ufsIndex or area or file are   *
*                  malformed.                                                  *
*    UFS_FILE_DOES_NOT_EXIST: File id does not point to a valid file.          *
*    UFS_AREA_DOES_NOT_EXIST: Area id does not point to a valid file.          *
*    UFS_MAPPING_ALREADY_EXISTS: The defined mapping already exists.           *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsIndex: The opaque ufs index structure.                                  *
*  -area: The internal area id as returned by ufsIndexAddArea.                 *
*  -file: The internal file id as returned by ufsIndexAddFile.                 *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call.                                    *
*                                                                              *
\******************************************************************************/
ufsIdType ufsIndexAddMapping( ufsImagePtr ufsImage,
                              ufsIdType area,
                              ufsIdType file );

/******************************************************************************\
* ufsIndexProbeMapping                                                         *
*                                                                              *
*  Probes the index mapping and searches for an (area, file) pair.             *
*                                                                              *
*  possible errors:                                                            *
*    UFS_BAD_CALL: Badly formatted call, either ufsIndex or area or file are   *
*                  malformed.                                                  *
*    UFS_FILE_DOES_NOT_EXIST: File id does not point to a valid file.          *
*    UFS_AREA_DOES_NOT_EXIST: Area id does not point to a valid file.          *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsIndex: The opaque ufs index structure.                                  *
*  -area: The internal area id as returned by ufsIndexAddArea.                 *
*  -file: The internal file id as returned by ufsIndexAddFile.                 *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -bool: true if the mapping exists, false otherwise.                         *
*                                                                              *
\******************************************************************************/
bool ufsIndexProbeMapping( ufsImagePtr ufsImage,
                           ufsIdType area,
                           ufsIdType file );

#endif /* UFS_INDEX_H */
