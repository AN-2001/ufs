/******************************************************************************\
*  ufs_trie.h                                                                  *
*                                                                              *
*  Header file for the ufs trie data-structure.                                *
*  Used to efficiently deduplicate during FUSE readdir.                        *
*                                                                              *
*              Written by A.N.                                  02-09-2026     *
*                                                                              *
\******************************************************************************/

#ifndef UFS_TRIE
#define UFS_TRIE

#include <stdbool.h>

typedef void *ufsTrieType;

ufsTrieType ufsTrieInit();

bool ufsTrieAdd( ufsTrieType ufsTrie, const char *str );

bool ufsTrieExists( ufsTrieType ufsTrie, const char *str );

void ufsTrieDestroy( ufsTrieType ufsTrie );

#endif /* UFS_TRIE */
