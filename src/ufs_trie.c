/******************************************************************************\
*  ufs_trie.c                                                                  *
*                                                                              *
*  Implementation for the ufs trie data structure.                             *
*                                                                              *
*              Written by A.N.                                  16-02-2026     *
*                                                                              *
\******************************************************************************/

#include <ufs_trie.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#define UFS_TRIE_TOTAL_CHARS (256)
#define UFS_STACK_SIZE (512)

typedef struct ufsTrieNodeStruct ufsTrieNode;

typedef struct ufsTrieEdgeStruct {
    const char *str;
    uint64_t start, len;
    ufsTrieNode *Node;
} ufsTrieEdge;

struct ufsTrieNodeStruct {
    bool isTerminal;
    char *terminalStr;
    ufsTrieEdge edges[ UFS_TRIE_TOTAL_CHARS ];
};

ufsTrieType ufsTrieInit()
{
    ufsTrieNode *root;

    root = malloc( sizeof( *root ) );
    if ( !root )
        return NULL;
    memset( root, 0, sizeof( *root ) );
    return root;
}

bool ufsTrieAdd( ufsTrieType ufsTrie, const char *str );

bool ufsTrieExists( ufsTrieType ufsTrie, const char *str );

void ufsTrieDestroy( ufsTrieType ufsTrie )
{
    if ( !ufsTrie )
        return;
    ufsTrieNode *stack[ UFS_STACK_SIZE ],
                *current,
                **tmp;
    uint64_t i,
             stackTop = 0;

    stack[ stackTop++ ] = ufsTrie;
    while ( stackTop ) {
        current = stack[ stackTop - 1 ];

        for ( i = 0; i < UFS_TRIE_TOTAL_CHARS; i++ ) {
            tmp = &current -> edges[ i ].Node;
            if ( *tmp ) {
                stack[ stackTop++ ] = *tmp;
                *tmp = NULL;
            }
        }

        /* stack top would only move, this condition boils down to:           */
        /* Did this node have children prior to the above loop?               */
        if ( stack[ stackTop - 1 ] == current ) {
            free( current -> terminalStr );
            free( current );
            stackTop--;
        }

    }

}
