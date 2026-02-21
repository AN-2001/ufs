/******************************************************************************\
*  ufs_trie.c                                                                  *
*                                                                              *
*  Implementation for the ufs trie data structure.                             *
*                                                                              *
*              Written by A.N.                                  16-02-2026     *
*                                                                              *
\******************************************************************************/

#include "ufs_core.h"
#include "ufs_utils.h"
#include <ufs_trie.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define UFS_TRIE_TOTAL_CHARS (256)
#define UFS_STACK_SIZE (512)

typedef struct ufsTrieNodeStruct ufsTrieNode;

typedef struct ufsTrieEdgeStruct {
    const char *str;
    uint64_t start, len;
    ufsTrieNode *node;
} ufsTrieEdge;

typedef struct ufsTrieNodeStruct {
    bool isTerminal;
    char *terminalStr;
    ufsTrieEdge edges[ UFS_TRIE_TOTAL_CHARS ];
} ufsTrieNode;

enum ufsTrieType {
    UFS_TRIE_NODE,
    UFS_TRIE_EDGE,
};

typedef struct ufsTrieTraversalResult {
    enum ufsTrieType type;
    int offset, prefixLen;
    union {
        ufsTrieNode *node;
        ufsTrieEdge *edge;
    };
} ufsTrieTraversalResult;

static inline int findLongestCommonPrefixLen( const char *str0,
                                              int len0,
                                              const char *str1,
                                              int len1 )
{
    int i;
    for ( i = 0; i < len0 && i < len1 && str0[i] == str1[i]; i++ );
    return i;
}

static inline ufsTrieTraversalResult traverse( ufsTrieNode *node,
                                               const char *str,
                                               int strLen )
{
    ufsTrieTraversalResult res;
    ufsTrieEdge *edge;

    res.offset = res.prefixLen = 0;

    edge = &node -> edges[ (unsigned char) str[ res.offset ] ];

    while ( edge -> node && res.offset < strLen ) {
         
        res.prefixLen = findLongestCommonPrefixLen( str + res.offset, strLen - res.offset,
                                                edge -> str + edge -> start,
                                                edge -> len );
        res.offset += res.prefixLen;
        if ( res.prefixLen < edge -> len ) {
            res.type = UFS_TRIE_EDGE;
            res.edge = edge;
            return res;
        }

        node = edge -> node;
        edge = &node -> edges[ (unsigned char) str[ res.offset ] ];
    }

    res.type = UFS_TRIE_NODE;
    res.node = node;
    return res;
}

static ufsTrieNode *createNode( const char *str, ufsStatusType *statusNo )
{
    ufsTrieNode *node;

    node = malloc( sizeof( *node ) );
    if ( !node ) {
        SET_STATUS( UFS_OUT_OF_MEMORY );
        return NULL;
    }
    memset( node, 0, sizeof( *node ) );
    if ( str ) {
        node -> isTerminal = true;
        node -> terminalStr = strdup( str );
        if ( !node -> terminalStr ) {
            free( node );
            SET_STATUS( UFS_OUT_OF_MEMORY );
            return NULL;
        }
    }
    SET_STATUS( UFS_NO_ERROR );
    return node;
}

ufsTrieType ufsTrieInit( ufsStatusType *statusNo )
{
    return createNode( NULL, statusNo );
}

static inline bool handleAddAtNode( ufsTrieTraversalResult *res,
                                    const char *str,
                                    int strLen,
                                    ufsStatusType *statusNo )
{
    ufsTrieNode *newNode;
    ufsTrieEdge *edge;
    if ( !str[ res -> offset ] ) {
        if ( res -> node -> isTerminal ) {
            SET_STATUS( UFS_ALREADY_EXISTS );
            return false;
        }
        res -> node -> isTerminal = true;
        SET_STATUS( UFS_NO_ERROR );
        return true;
    }

    edge = &res -> node -> edges[ (unsigned char) str[ res -> offset ] ];
    newNode = createNode( str + res -> offset, statusNo );
    if ( !newNode ) {
        return NULL;
    }

    edge -> str = newNode -> terminalStr;
    edge -> node = newNode;
    edge -> start = 0;
    edge -> len = strLen - res -> offset;

    SET_STATUS( UFS_NO_ERROR );
    return true;
}

static inline bool handleAddAtEdge( ufsTrieTraversalResult *res,
                                    const char *str,
                                    int strLen,
                                    ufsStatusType *statusNo )
{
    ufsTrieNode *newNode, *tmp, *fork;
    ufsTrieEdge  *newEdge0, *newEdge1,
        *edge = res -> edge;

    /* str is a prefix of a string that already exists.                       */
    if ( !str[ res -> offset ] ) {
        tmp = edge -> node;

        newNode = createNode( NULL, statusNo );
        if ( !newNode ) {
            return NULL;
        }

        newNode -> isTerminal = true;
        newEdge0 = &newNode -> edges[ (unsigned char )edge -> str[ edge -> start + res -> prefixLen ] ];

        newEdge0 -> str = edge -> str;
        newEdge0 -> start = edge -> start + res -> prefixLen;
        newEdge0 -> len = edge -> len - res -> prefixLen;
        newEdge0 -> node = tmp;

        edge -> node = newNode;
        edge -> len = res -> prefixLen;

        SET_STATUS( UFS_NO_ERROR );
        return true;

    }

    tmp = edge -> node;

    fork = createNode( NULL, statusNo );
    if (!fork) {
        return NULL;
    }

    newEdge1 = &fork -> edges[ (unsigned char) str[ res -> offset ] ];
    newEdge1 -> node = createNode( str + res -> offset, statusNo );
    if ( !newEdge1 ) {
        free( fork );
        return NULL;
    }

    newEdge1 -> str = newEdge1 -> node -> terminalStr;
    newEdge1 -> start = 0;
    newEdge1 -> len = strLen - res -> offset;

    newEdge0 = &fork -> edges[ (unsigned char) edge -> str[ edge -> start + res -> prefixLen ] ];
    newEdge0 -> str = edge -> str;
    newEdge0 -> start = edge -> start + res -> prefixLen;
    newEdge0 -> len = edge -> len - res -> prefixLen;
    newEdge0 -> node = tmp;

    edge -> len = res -> prefixLen;
    edge -> node = fork;

    SET_STATUS( UFS_NO_ERROR );
    return true;
}

bool ufsTrieAdd( ufsTrieType ufsTrie, const char *str, ufsStatusType *statusNo  )
{
    int strLen;
    ufsTrieTraversalResult res;
    static bool (*handlers[])( ufsTrieTraversalResult*,
                               const char *,
                               int,
                               ufsStatusType *) = {
        handleAddAtNode,
        handleAddAtEdge
    };

    if ( !ufsTrie || !str ) {
        SET_STATUS( UFS_BAD_CALL );
        return false;
    }

    strLen = strlen( str );
    res = traverse( ufsTrie, str, strLen );

    return handlers[ res.type ]( &res, str, strLen, statusNo );
}

bool ufsTrieExists( ufsTrieType ufsTrie, const char *str, ufsStatusType *statusNo )
{
    ufsTrieTraversalResult res;
    if ( !ufsTrie || !str ) {
        SET_STATUS( UFS_BAD_CALL );
        return false;
    }

    res = traverse( ufsTrie, str, strlen( str ) );

    if ( res.type == UFS_TRIE_NODE &&
         res.node -> isTerminal &&
         !str[ res.offset ] ) {
        SET_STATUS( UFS_NO_ERROR );
        return true;
    }

    SET_STATUS( UFS_DOES_NOT_EXIST );
    return false;
}

void ufsTrieDestroy( ufsTrieType ufsTrie, ufsStatusType *statusNo )
{
    ufsTrieNode *stack[ UFS_STACK_SIZE ],
                *current;
    uint64_t i,
             stackTop = 0;

    if ( !ufsTrie ) {
        SET_STATUS( UFS_NO_ERROR );
        return;
    }

    stack[ stackTop++ ] = ufsTrie;
    while ( stackTop ) {
        current = stack[ stackTop - 1 ];
        for ( i = 0; i < UFS_TRIE_TOTAL_CHARS && !current -> edges[ i ].node; i++ );
        if ( i == UFS_TRIE_TOTAL_CHARS ) {
            free( current -> terminalStr );
            free( current );
            stackTop--;
        } else {
            stack[ stackTop++ ] = current -> edges[ i ].node;
            current -> edges[ i ].node = NULL;
        }
    }

    SET_STATUS( UFS_NO_ERROR );
}
