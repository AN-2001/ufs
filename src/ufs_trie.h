/******************************************************************************\
*  ufs_trie.h                                                                  *
*                                                                              *
*  Header file for the ufs trie data-structure.                                *
*  Used to efficiently deduplicate during FUSE readdir.                        *
*                                                                              *
*              Written by A.N.                                  02-09-2026     *
*                                                                              *
\******************************************************************************/

/*                                                                            */
/* The trie is designed in the following manner:                              */
/* * Paths are compressed, as long as there's no fork a long path will be     */
/*   collapsed into a single node.                                            */
/* * Strings are allocated and stored inside of terminal nodes.               */
/* * Edges do not own any strings, the strings they hold are pointers to str- */
/*   ings in terminal nodes.                                                  */
/* * Strings are heavily reused, if for example a string is a prefix of anot- */
/*   her, then that string is not allocated.                                  */
/* * Also, if a prefix of a new string already exists, then only the chars a- */
/*   fter the prefix are allocated.                                           */
/*                                                                            */
/* These functions set ufsErrno on error.                                     */
/*                                                                            */
/* This trie is NOT threadsafe, as its designed to be used in the context of  */
/* a single function and freed after. It does not persist as state.           */
/*                                                                            */


#ifndef UFS_TRIE
#define UFS_TRIE

#include <stdbool.h>
#include <stdint.h>
#include "ufs_core.h"

typedef void *ufsTrieType;

/******************************************************************************\
* ufsTrieInit                                                                  *
*                                                                              *
*  Initialize a new empty ufs trie.                                            *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_OUT_OF_MEMORY: The system is out of memory.                           *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  ufsTrieType: the new trie, can be NULL if out of memory.                    *
*                                                                              *
\******************************************************************************/
ufsTrieType ufsTrieInit();

/******************************************************************************\
* ufsTrieAdd                                                                   *
*                                                                              *
*  Adds a new string into a ufs trie.                                          *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_ALREADY_EXISTS: The string already exists.                            *
*   -UFS_OUT_OF_MEMORY: The system is out of memory.                           *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsTrie: the UFS trie to use.                                              *
*  -str: the new string to be added.                                           *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -bool: true on success, false otherwise.                                    *
*                                                                              *
\******************************************************************************/
bool ufsTrieAdd( ufsTrieType ufsTrie, const char *str );

/******************************************************************************\
* ufsTrieAdd                                                                   *
*                                                                              *
*  Checks whether a string exists inside a given ufs trie.                     *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_BAD_CALL: The function received bad arguments.                        *
*   -UFS_DOES_NOT_EXIST: The string does not exist.                            *
*   -UFS_UNKNOWN_ERROR: Any error not specified above.                         *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsTrie: The UFS trie to use.                                              *
*  -str: The string to check.                                                  *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -bool: true if str exists in ufsTrie, false otherwise.                      *
*                                                                              *
\******************************************************************************/
bool ufsTrieExists( ufsTrieType ufsTrie, const char *str );

/******************************************************************************\
* ufsTrieDestroy                                                               *
*                                                                              *
*  Frees the memory used by ufsTrie, if ufsTrie is NULL this is a noop.        *
*                                                                              *
*  Possible errors:                                                            *
*   -UFS_UNKNOWN_ERROR: Any error not specified here.                          *
*                                                                              *
* Parameters                                                                   *
*                                                                              *
*  -ufsTrie: The UFS trie to use.                                              *
*                                                                              *
* Return                                                                       *
*                                                                              *
*  -void.                                                                      *
*                                                                              *
\******************************************************************************/
void ufsTrieDestroy( ufsTrieType ufsTrie );

#endif /* UFS_TRIE */
