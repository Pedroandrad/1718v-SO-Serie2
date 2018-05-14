#pragma once

#include <Windows.h>
#include "UThreadInternal.h"

typedef struct _node {
	PUTHREAD uThread;
	struct _node *Fnode;
	struct _node *Bnode;
} node_t, *Pnode_t;

#include <Windows.h>

/////////////////////////////////////////////////////////////////
//
// CCISEL 
// Initializes the specified doubly linked list.
//
VOID NodeInitializeListHead(Pnode_t ListHead);

// Returns true if the specified list is empty.
//
BOOLEAN NodeIsListEmpty(Pnode_t ListHead);

// Removes the specified entry from the list that
// contains it.
//
BOOL NodeRemoveEntryList(Pnode_t Entry);

// Removes the entry that is at the front of the list.
//
Pnode_t NodeRemoveHeadList(Pnode_t ListHead);

// Removes the entry that is at the tail of the list.
//
Pnode_t NodeRemoveTailList(Pnode_t ListHead);

// Inserts the specified entry at the tail of the list.
//
VOID NodeInsertTailList(Pnode_t ListHead, Pnode_t Entry);

// Inserts the specified entry at the head of the list.
//
VOID NodeInsertHeadList(Pnode_t ListHead, Pnode_t Entry);
