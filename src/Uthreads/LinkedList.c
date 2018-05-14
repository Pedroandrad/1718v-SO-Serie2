#include <Windows.h>
#include "LinkedList.h"

/////////////////////////////////////////////////////////////////
//
// CCISEL 
// Initializes the specified doubly linked list.
//
VOID NodeInitializeListHead(Pnode_t ListHead) {
	ListHead->Fnode = ListHead->Bnode = ListHead;
}

// Returns true if the specified list is empty.
//
BOOLEAN NodeIsListEmpty(Pnode_t ListHead) {
	return (BOOLEAN)(ListHead->Fnode == ListHead);
}

// Removes the specified entry from the list that
// contains it.
//
BOOL NodeRemoveEntryList(Pnode_t Entry) {
	Pnode_t Bnode;
	Pnode_t Fnode;

	Fnode = Entry->Fnode;
	Bnode = Entry->Bnode;
	Bnode->Fnode = Fnode;
	Fnode->Bnode = Bnode;

	return Fnode == Bnode;
}

// Removes the entry that is at the front of the list.
//
Pnode_t NodeRemoveHeadList(Pnode_t ListHead) {
	Pnode_t Fnode;
	Pnode_t Entry;

	Entry = ListHead->Fnode;
	Fnode = Entry->Fnode;
	ListHead->Fnode = Fnode;
	Fnode->Bnode = ListHead;

	return Entry;
}

// Removes the entry that is at the tail of the list.
//
Pnode_t NodeRemoveTailList(Pnode_t ListHead) {
	Pnode_t Bnode;
	Pnode_t Entry;

	Entry = ListHead->Bnode;
	Bnode = Entry->Bnode;
	ListHead->Bnode = Bnode;
	Bnode->Fnode = ListHead;
	return Entry;
}

// Inserts the specified entry at the tail of the list.
//
VOID NodeInsertTailList(Pnode_t ListHead, Pnode_t Entry){
	Pnode_t Bnode;

	Bnode = ListHead->Bnode;
	Entry->Fnode = ListHead;
	Entry->Bnode = Bnode;
	Bnode->Fnode = Entry;
	ListHead->Bnode = Entry;
}

// Inserts the specified entry at the head of the list.
//
VOID NodeInsertHeadList(Pnode_t ListHead, Pnode_t Entry) {
	Pnode_t Fnode;

	Fnode = ListHead->Fnode;
	Entry->Fnode = Fnode;
	Entry->Bnode = ListHead;
	Fnode->Bnode = Entry;
	ListHead->Fnode = Entry;
}
