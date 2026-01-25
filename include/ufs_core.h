/******************************************************************************\
*  ufs_core.h                                                                  *
*                                                                              *
*  Contains the spec for ufs core.                                             *
*  Different implementations are valid as long as they implement this.         *
*                                                                              *
*              Written by A.N.                                  17-01-2026     *
*                                                                              *
\******************************************************************************/

#ifndef UFS_CORE_H
#define UFS_CORE_H

/*                                                                            */
/* This is the spec for the ufs "union file system" storage back-end.         */
/* The goal of this spec is to define the semantics of how ufs represents     */
/* its internal data, in other words: this is the core of ufs.                */
/* Definitions:                                                               */
/*                                                                            */
/* Storage: An entity represented by a name.                                  */
/*                                                                            */
/* Directory: Storage that can contain other storage.                         */
/*                                                                            */
/* Note: Directory is sometimes referred to as "parent" in add/get functions. */
/*       In which case it means that the directory will contain the result of */
/*       the add/get.                                                         */
/*                                                                            */
/* ROOT: The root directory of ufs, internally it has the unique name UFS_ST- */
/*       ORAGE_ROOT_NAME, no other directory can be given this name.          */
/*                                                                            */
/* File: Storage that cannot contain other storage.                           */
/*                                                                            */
/* Area: A set of storage represented by a unique name.                       */
/*       areas DO NOT own said storage, they only project it using a name.    */
/*                                                                            */
/* a ufs type: Is either a storage or an area.                                */
/*                                                                            */
/* Mapping: A (area, storage) relation, defined as area projects storage.     */
/*          Mappings are defined as a proper mathematical relation, meaning:  */
/*            * They have set semantics.                                      */
/*            * The same storage can appear with different areas.             */
/*                                                                            */
/* external filesystem: Referred to as external fs in other places in this d- */
/*                      ocument. It Is the file-system that ufs is mounted on */
/*                      top of.                                               */
/*                      Formally it is the filesystem that existed before     */
/*                      running ufsInit.                                      */
/*                                                                            */
/* BASE: a unique area that refers to the base external filesystem.           */
/*       Most views will end with BASE as they're supposed to shadow it.      */
/*       The keyword 'BASE' is a special keyword that cannot be added as an   */
/*       area. It can be used when specifying a view to refer to the external */
/*       filesystem. BASE is guaranteed to be valid after a ufsInit.          */
/*       BASE cannot appear in a mapping, the filesystem semantics of search  */
/*       are that of the external fs, meaning if ufs encounters BASE it shou- */
/*       ld dispatch queries to the external fs.                              */
/*       the external fs references by BASE should be immutable except when   */
/*       calling ufsCollapse on a view that ends with BASE.                   */
/*                                                                            */
/* About storage and mappings: Storage should always exist in a mapping, to   */
/* satisfy this constraint we define two types of mappings:                   */
/*   * An explicit mapping added view ufsAddMapping                           */
/*   * An implicit mapping, if a file does not appear in an explicit mapping  */
/*     then it is implicitly mapped to BASE.                                  */
/*     Note, that the implicit mapping is "logical" in the sense that its a   */
/*     system-wide system assumption that does not need to be stored as state */
/*                                                                            */
/*                                                                            */
/* Area containing storage: we say that an area A contains storage S if the   */
/*                          implicit or explicit mapping (A, s) exists in     */
/*                          ufs.                                              */
/*                                                                            */
/* View: a list of areas, in our context it can be UFS_VIEW_MAX_SIZE          */
/*       size max. Semantically this is a union of areas:                     */
/*          Let V = ( A0, A1, ..., An )                                       */
/*          Assume we're attempting to resolve some storage s in V            */
/*          If n == 0: Fail, V definitely does not contain s.                 */
/*          Inductive step:                                                   */
/*          Attempt to resolve s in Ak, if it contains it halt.               */
/*          Otherwise continue to k + 1                                       */
/*          Stop once n = k                                                   */
/*       Views are not allowed to contain duplicate areas.                    */
/*       BASE is only allowed to exist at the end of an area.                 */
/*       Views are to be terminated with a UFS_VIEW_TERMINATOR or they can    */
/*       extend to UFS_VIEW_MAX_SIZE. Meaning when looking at a view an       */
/*       observer must stop at the FIRST UFS_VIEW_TERMINATOR or until they    */
/*       exhaust all of UFS_VIEW_MAX_SIZE.                                    */
/*                                                                            */
/* Note: ufs views are treated as immutable state given by the user. Ufs onl- */
/*       y reads and validates them, it does not store or mutate them in any  */
/*       way.                                                                 */
/*                                                                            */
/* Directory iteration in the context of views: A directory can be iterated   */
/* over given a view, the semantics of iteration don't take the view order    */
/* into account. As for our uses (readdir) all the operation needs to do      */
/* is compute a set union operation out of all files in the view.             */
/* Formally: Given a view V = ( A1, A2, ..., An )                             */
/* and a directory d, iterating over d in V equates to iteration over the     */
/* file set F = files_in( A1, d ) union  ... union files_in( An, d )          */
/*                                                                            */
/* The directory iterator: The directory iterator is a function that the      */
/* user supplies and implementer must call. For each iteration it contains    */
/*    * The current identifier of the storage.                                */
/*    * The entry position in the iteration.                                  */
/*    * The total number of entries that its iterating over.                  */
/*    * User provided data.                                                   */
/* An iterator can return an error status, it'd halt iteration and set ufsEr- */
/* rno.                                                                       */
/*                                                                            */
/* IdentifierType: A numeric unique identifier to ufs type instance.          */
/*                 ufs areas have their own identifier space while ufs stora- */
/*                 ge shares their space.                                     */
/*                 The identifier must be strictly greater than 0.            */
/*                 Note: it is up to the implementer to deduce the ufs type   */
/*                       of a ufs type, IdentifierType doesn't define a tagg- */
/*                       ing mechanism.                                       */
/*                                                                            */
/* Note: BASE has the unique identifier 0.                                    */
/* Note: ROOT has the unique identifier 0.                                    */
/*                                                                            */
/* StatusType: A status that ufs stores in ufsErrno, shows the current status */
/*             of ufs, its set as a side effect of all ufs functions.         */
/*                                                                            */
/* Applying mapping to an area: Applying mappings to an area A, means that if */
/*                              a view V = [ a, UFS_VIEW_TERMINATOR ] is used */
/*                              for resolution/iteration then those changes   */
/*                              should be observed.                           */
/*                                                                            */
/* collapse semantics: A ufs collapse on a view should take all mappings in   */
/*                     the view and apply them on the last area.              */
/*                     If the last area happens to be BASE the changes are    */
/*                     applied to the external filesystem.                    */
/*                                                                            */
/* About removal semantics: Once a storage or area or explicit mapping is re- */
/* moved in ufs, then the ufs state should be as if that storage or area or   */
/* explicit mapping don't exist.                                              */
/* * A subsequent removal would yield "UFS_DOES_NOT_EXIST".                   */
/* * A get/probe would yield "UFS_DOES_NOT_EXIST".                            */
/* * An add would succeed.                                                    */
/* Note: ufs has a zero tolerance stance on side effects when it comes to     */
/* removal. This can be fully described by the following rules:               */
/* * A directory can't be removed if it contains files, or is references in   */
/*   an explicit mapping.                                                     */
/* * A file can't be removed if it's referenced in an explicit mapping.       */
/* * An area can't be removed if it exists in an explicit mapping.            */
/*                                                                            */
/* Explicit mappings can be removed freely as no other ufs entity depends on  */
/* them.                                                                      */
/* ufs does not view "views" as state, meaning that if an area referenced in  */
/* a view is removed, it should treated as a "does not exist" case.           */
/*                                                                            */
/* Dependency graph for reference (an edge encodes a "depends" relation ):    */
/*                                                                            */
/*                                   area ----                                */
/*                                            \                               */
/*                   directory ----> file ----> explicit mapping              */
/*                             \             /                                */
/*                              -------------                                 */
/*                                                                            */
/*             An edge ( A, B ) means that a type A cannot be deleted         */
/*             if it depends on something in type B.                          */
/*                                                                            */
/* * a directory D depends on file F if F exists inside of D.                 */
/* * a file F depends on explicit mapping M if M = (A, F) for some area A.    */
/* * an area A depends on explicit mapping M if M = (A, S) for some storage   */
/*  S.                                                                        */
/* * a directory D depends on explicit mapping M if M = (A, D) for some area  */
/*   A.                                                                       */
/*                                                                            */
/* Note: Implicit mappings do not place removal constraints, as they are log- */
/* ical and aren't stored as state.                                           */
/*                                                                            */
/* Note: both ROOT and BASE cannot be removed.                                */
/*                                                                            */

#define UFS_VIEW_MAX_SIZE (4096)
#define UFS_VIEW_TERMINATOR (-1)
#define UFS_AREA_BASE_NAME ("BASE") 
#define UFS_AREA_BASE_IDENTIFIER (0)
#define UFS_STORAGE_ROOT_NAME ("ROOT") 
#define UFS_STORAGE_ROOT_IDENTIFIER (0)
#define UFS_NAME

#include <stdint.h>
#include <sys/types.h>

#define UFS_STATUS_LIST                                                        \
    UFS_X( UFS_NO_ERROR,                   0 )                                 \
    UFS_X( UFS_ALREADY_EXISTS,             1ULL << 0 )                         \
    UFS_X( UFS_BAD_CALL,                   1ULL << 1 )                         \
    UFS_X( UFS_CANNOT_RESOLVE_STORAGE,     1ULL << 2 )                         \
    UFS_X( UFS_PARENT_DOES_NOT_EXIST,      1ULL << 3 )                         \
    UFS_X( UFS_DIRECTORY_IS_NOT_EMPTY,     1ULL << 4 )                         \
    UFS_X( UFS_DOES_NOT_EXIST,             1ULL << 5 )                         \
    UFS_X( UFS_EXISTS_IN_EXPLICIT_MAPPING, 1ULL << 6 )                         \
    UFS_X( UFS_ILLEGAL_NAME,               1ULL << 7 )                         \
    UFS_X( UFS_INVALID_AREA_IN_VIEW,       1ULL << 8 )                         \
    UFS_X( UFS_MAPPING_DOES_NOT_EXIST,     1ULL << 9 )                         \
    UFS_X( UFS_OUT_OF_MEMORY,              1ULL << 10 )                        \
    UFS_X( UFS_UNKNOWN_ERROR,              1ULL << 11 )                        \
    UFS_X( UFS_VIEW_CONTAINS_DUPLICATES,   1ULL << 12 )                        \
    UFS_X( UFS_PARENT_CANT_BE_FILE,        1ULL << 13 )                        \
    UFS_X( UFS_BASE_IS_NOT_LAST_AREA,      1ULL << 14 ) 

enum {
#define UFS_X( name, val ) name = val,
    UFS_STATUS_LIST
#undef UFS_X
    UFS_NUM_ERRORS
};

extern const char *ufsStatusStrings[ UFS_NUM_ERRORS ];

typedef uint64_t ufsStatusType;
typedef int64_t ufsIdentifierType;

typedef void *ufsType;
typedef ufsStatusType (*ufsDirIter)( ufsIdentifierType storage,
                                     uint64_t currEntry,
                                     uint64_t numEntries,
                                     void *userData);
typedef ufsIdentifierType ufsViewType[ UFS_VIEW_MAX_SIZE ];

extern ufsStatusType ufsErrno;

/******************************************************************************\
* ufsInit                                                                      *
*                                                                              *
*  Initialise a ufs and return it.                                             *
*  The function will initialise both ROOT and BASE and get them to a state wh- *
*  ere users can use them.                                                     *
*  Meaning, after ufsInit, both ROOT and BASE can be referenced as intended.   *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_OUT_OF_MEMORY: The system is out of memory and can't create ufs.      *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsType: a new ufs instance.                                               *
*                                                                              *
\******************************************************************************/
ufsType ufsInit();

/******************************************************************************\
* ufsDestroy                                                                   *
*                                                                              *
*  Destroys a given ufs.                                                       *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_UNKNOWN_ERROR: Should not return errors, so any error should be       *
*                       unknown.                                               *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, can be NULL, in which case this is a no-op.         *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -void.                                                                      *
*                                                                              *
\******************************************************************************/
void ufsDestroy( ufsType ufs );

/******************************************************************************\
* ufsAddDirectory                                                              *
*                                                                              *
*  Adds a directory to ufs.                                                    *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_ILLEGAL_NAME: An illegal directory name (e.g ROOT) was provided.      *
*   -UFS_PARENT_CANT_BE_FILE: Parent represents a file instead of a directory. *
*   -UFS_ALREADY_EXISTS: The directory already exists.                         *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -parent: The directory that contains this directory, must be non-negative.  *
*  -name: The name of the directory, must not be NULL.                         *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the new directory.             *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsAddDirectory( ufsType ufs,
                                   ufsIdentifierType parent,
                                   const char *name );

/******************************************************************************\
* ufsAddFile                                                                   *
*                                                                              *
*  Adds a file to ufs.                                                         *
*                                                                              *
*  Note that file names are not global, a directory asks as the scope for      *
*  the name. Meaning that the same name may be reused under different directo- *
*  ies.                                                                        *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_ALREADY_EXISTS: The file already exists.                              *
*   -UFS_PARENT_DOES_NOT_EXIST: The specified directory does not exist.        *
*   -UFS_ILLEGAL_NAME: An illegal file name (e.g ROOT) was provided.           *
*   -UFS_PARENT_CANT_BE_FILE: Parent represents a file instead of a directory. *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -parent: The directory that contains this file, must be non-negative.       *
*  -name: The name of the file, must not be NULL.                              *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the new file.                  *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsAddFile( ufsType ufs,
                              ufsIdentifierType parent,     
                              const char *name );

/******************************************************************************\
* ufsAddArea                                                                   *
*                                                                              *
*  Adds a area to ufs.                                                         *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_ALREADY_EXISTS: The area already exists.                              *
*   -UFS_ILLEGAL_NAME: An illegal area name (e.e "BASE") was provided.         *
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
* ufsAddMapping                                                                *
*                                                                              *
*  Adds a ufs mapping in the form of (area, storage).                          *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The area or the storage do not exist in ufs.          *
*   -UFS_ALREADY_EXISTS: The mapping already exists in ufs.                    *
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
*  -ufsStatusType: The status of this call, ufsErrno is also set.              *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsAddMapping( ufsType ufs,
                             ufsIdentifierType area,
                             ufsIdentifierType storage );

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
*  -parent: The directory that contains this directory, must be non-negative.  *
*  -name: The name of the directory, must not be NULL.                         *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the existing directory.        *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsGetDirectory( ufsType ufs,
                                   ufsIdentifierType parent,
                                   const char *name );

/******************************************************************************\
* ufsGetFile                                                                   *
*                                                                              *
*  Retrieves a file's unique identifier from ufs.                              *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The specified file does not exist.                    *
*   -UFS_PARENT_DOES_NOT_EXIST: The specified parent does not exist.           *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -parent: The directory that contains this file, must be non-negative.       *
*  -name: The name of the file, must not be NULL.                              *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsIdentifierType: The unique identifier of the existing file.             *
*                      If a negative value is returned, check ufsErrno.        *
*                                                                              *
\******************************************************************************/
ufsIdentifierType ufsGetFile( ufsType ufs,
                              ufsIdentifierType parent,
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
* ufsProbeMapping                                                              *
*                                                                              *
*  Probes ufs for a mapping.                                                   *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The area or the storage do not exist in ufs.          *
*   -UFS_MAPPING_DOES_NOT_EXIST: The mapping does not exist.                   *
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
*  -ufsStatusType: The status of this call, ufsErrno is also set.              *
*  Note, if the mapping does not exist UFS_MAPPING_DOES_NOT_EXIST is returned  *
*  and is set in ufsErrno, this is not a traditional error as it's the result  *
*  of the query.                                                               *
*  If the mapping exists UFS_NO_ERROR is returned and set in ufsErrno.         *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsProbeMapping( ufsType ufs,
                               ufsIdentifierType area,
                               ufsIdentifierType storage );

/******************************************************************************\
* ufsRemoveDirectory                                                           *
*                                                                              *
*  Removes a directory from ufs.                                               *
*  A directory must be empty before being removed, an empty directory is a     *
*  directory that does not contain any files globally across ufs.              *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The directory does not exist in ufs.                  *
*   -UFS_DIRECTORY_IS_NOT_EMPTY: The directory is not empty and can't be       *
*                                removed.                                      *
*   -UFS_ILLEGAL_NAME: An illegal directory name (e.g ROOT) was provided.      *
*   -UFS_EXISTS_IN_EXPLICIT_MAPPING: The directory is referenced in an explic- *
*                                    it mapping and cannot be removed.         *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -parent: The directory that contains this directory, must be non-negative.  *
*  -directory: the directory's unique identifier, must be greater than 0.      *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, ufsErrno is also set.              *
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
*   -UFS_EXISTS_IN_EXPLICIT_MAPPING: The file is referenced in an explicit ma- *
*                                    pping and cannot be removed.              *
*   -UFS_ILLEGAL_NAME: An illegal file name (e.g ROOT) was provided.           *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -file: the file's unique identifier, must be greater than 0.                *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, ufsErrno is also set.              *
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
*   -UFS_EXISTS_IN_EXPLICIT_MAPPING: The area is referenced in an explicit ma- *
*                                    pping and cannot be removed.              *
*   -UFS_ILLEGAL_NAME: An illegal area name (e.e "BASE") was provided.         *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -area: the area's unique identifier, must be greater than 0.                *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, ufsErrno is also set.              *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsRemoveArea( ufsType ufs,
                             ufsIdentifierType area );

/******************************************************************************\
* ufsRemoveMapping                                                             *
*                                                                              *
*  Removes an (area, storage) mapping from ufs.                                *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The (area, storage) mapping does not exist in ufs.    *
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
*  -ufsStatusType: The status of this call, ufsErrno is also set.              *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsRemoveMapping( ufsType ufs,
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
*   -UFS_VIEW_CONTAINS_DUPLICATES: The view contains duplicate areas.          *
*   -UFS_INVALID_AREA_IN_VIEW: The view contains a non-existent area.          *
*   -UFS_BASE_IS_NOT_LAST_AREA: BASE was used but was not the last area in th- *
*                               e view.                                        *
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
*   -UFS_VIEW_CONTAINS_DUPLICATES: The view contains duplicate areas.          *
*   -UFS_INVALID_AREA_IN_VIEW: The view contains a non-existent area.          *
*   -UFS_BASE_IS_NOT_LAST_AREA: BASE was used but was not the last area in th- *
*                               e view.                                        *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -view: The view to use.                                                     *
*  -directory: The directory's unique identifier, must be greater than 0.      *
*  -iterator: The iterator function to apply, must not be NULL.                *
*  -userData: The user's data, can be NULL.                                    *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, ufsErrno is also set.              *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsIterateDirInView( ufsType ufs,
                                   ufsViewType view,
                                   ufsIdentifierType directory,
                                   ufsDirIter iterator,
                                   void *userData );

/******************************************************************************\
* ufsCollapse                                                                  *
*                                                                              *
*  Collapses all mappings in a ufs view into the last area in the view.        *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The directory does not exist in ufs.                  *
*   -UFS_VIEW_CONTAINS_DUPLICATES: The view contains duplicate areas.          *
*   -UFS_INVALID_AREA_IN_VIEW: The view contains a non-existent area.          *
*   -UFS_BASE_IS_NOT_LAST_AREA: BASE was used but was not the last area in th- *
*                               e view.                                        *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufs: The ufs instance, must not be NULL.                                   *
*  -view: The view to use.                                                     *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -ufsStatusType: The status of this call, ufsErrno is also set.              *
*                                                                              *
\******************************************************************************/
ufsStatusType ufsCollapse( ufsType ufs,
                           ufsViewType view );

#endif /* UFS_CORE_H */
