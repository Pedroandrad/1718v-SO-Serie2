#pragma once

typedef BOOL (*ACTION)(LPCSTR fileName, LPVOID arg);

BOOL TraverseDirTree(LPCSTR dir, LPCSTR extension, ACTION  action, LPVOID arg);

typedef struct _Info {
	LIST_ENTRY Link;
	CHAR *buffer;
	LPCSTR dir;
	LPCSTR extension;
	ACTION action;
	LPVOID arg;
} Info, *PInfo;

BOOL myTraverse(LPCSTR dir, LPCSTR extension, ACTION  action, LPVOID arg, PLIST_ENTRY head);