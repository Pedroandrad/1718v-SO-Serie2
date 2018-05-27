#include "../Include/app.h"
#include "../Include/traversedir.h"
#include <thread>

// contexts for traverse folders

// the global context
typedef struct  {
	LPCSTR pathOutFiles;	// The root dir of transformed files
	PLIST_ENTRY resultList;	// A list of matched reference file. 
							// Each reference file contains the list of matching transformed files.
	FLIP_enum_t flipType;	// The flip type to match
	INT errorCode;			// 0(OPER_SUCCESS) means the operation concludes successfully
} MUTATIONS_RESULT_CTX, *PMUTATIONS_RESULT_CTX;

// the per reference file context
typedef struct {
	PMUTATIONS_RESULT_CTX global; // pointer to global context
	PFILE_MAP refMap;			  // the  (transformed) reference file mapping
	LIST_ENTRY fileMatchingList;  // the list for identical transformed files
} REFIMAGE_MUTLIST_CTX, *PREFIMAGE_MUTLIST_CTX;


static VOID InitGlobalCtx(PMUTATIONS_RESULT_CTX ctx, LPCSTR pathOutFiles, 
								 FLIP_enum_t flipType, PLIST_ENTRY result) {
	ctx->pathOutFiles = pathOutFiles;
	ctx->flipType = flipType;
	InitializeListHead(result);
	ctx->resultList = result;
	ctx->errorCode = OPER_SUCCESS; // optimistic initialization
}

static VOID InitRefImageMutListCtx(PREFIMAGE_MUTLIST_CTX ctx, PFILE_MAP refMap, 
									PMUTATIONS_RESULT_CTX global) {
	ctx->refMap = refMap;
	ctx->global = global;
	InitializeListHead(&ctx->fileMatchingList);
}

FORCEINLINE static VOID OperMarkError(PMUTATIONS_RESULT_CTX ctx, INT code) {
	ctx->errorCode = code;
}

FORCEINLINE static BOOL OperHasError(PMUTATIONS_RESULT_CTX ctx) {
	return ctx->errorCode != OPER_SUCCESS;
}


/*
 * Called for each found bmp in transformed files dir
 * Parameters:
 *	filePath - the transformed file path
 *	_ctx     - a pointer to the per ref file context structure
 **/
BOOL ProcessTransformedFile(LPCSTR filePath, LPVOID _ctx) {
	PREFIMAGE_MUTLIST_CTX ctx = (PREFIMAGE_MUTLIST_CTX)_ctx;
	FILE_MAP fileMap;
	
	if (!FileMapOpen(&fileMap, filePath)) {
		OperMarkError(ctx->global, OPER_MAP_ERROR);
		return FALSE;
	}
	 	
	// Insert filename into list if equals to temporary flip result
	if (fileMap.mapSize == ctx->refMap->mapSize &&
		memcmp(ctx->refMap->hView, fileMap.hView, fileMap.mapSize) == 0) {
		PFILENAME_NODE newNode = (PFILENAME_NODE)malloc(sizeof(FILENAME_NODE));
		newNode->filename = _strdup(filePath);
		InsertTailList(&ctx->fileMatchingList, &newNode->link);
	}
	FileMapClose(&fileMap);
	return TRUE;
}

/*
 * Called for each found bmp in reference files dir
 * Parameters:
 *	filePath - the reference file path
 *	_ctx     - a pointer to the global operation context structure
 **/
BOOL BMP_GetFlipsOfRefFile(LPCSTR filePath, LPVOID _ctx) {
	PMUTATIONS_RESULT_CTX global = (PMUTATIONS_RESULT_CTX)_ctx;
	
	printf("BMPUtils_singlethread.BMP_get_flip_reprodutions_of_file: \n"
		"\tFile to analyse = \"%s\"\n"
		"\tDirectory of files to search for reprodutions = \"%s\"\n"
		"\tFlip type = %s\n", filePath, global->pathOutFiles, 
							  global->flipType == FLIP_VERTICALLY ? "Flip vertically" : "Flip horizontally");

								
	// Map file into process address space and create flip view
	FILE_MAP refFileMap, flipFileMap;
	BOOL ret = TRUE;

	// map reference file
	if (!FileMapOpen(&refFileMap, filePath)) {
		OperMarkError(global, OPER_MAP_ERROR);
		return FALSE;
	}

	// do the flip to a temprary map
	if (!FileMapTemp(&flipFileMap, refFileMap.mapSize)) {
		OperMarkError(global, OPER_MAP_ERROR);
		FileMapClose(&refFileMap);
		return FALSE;
	}
		
	// do the selected flip
	BMP_FlipMem((PUCHAR)refFileMap.hView, (PUCHAR)flipFileMap.hView, global->flipType);

	REFIMAGE_MUTLIST_CTX ctx;
	InitRefImageMutListCtx(&ctx, &flipFileMap, global);
	
	// Iterate through pathOutFiles directory and sub directories,
	// invoking de processor (ProcessTransformedFile) for each transformed file found
	if (!TraverseDirTree(global->pathOutFiles, ".bmp", ProcessTransformedFile, &ctx)) {
		if (!OperHasError(global))
			OperMarkError(global, OPER_TRAVERSE_ERROR);
	}
	if (OperHasError(global)) {
		ret = FALSE;
		goto terminate;
	}
		
	if (!IsListEmpty(&ctx.fileMatchingList)) {
		// Accumulate filename list to final result
		PRES_NODE newNode = (PRES_NODE)malloc(sizeof(RES_NODE));
		newNode->filename = _strdup(filePath);
		InitializeListHead(&newNode->files);
		//InsertRangeTailList(&newNode->files, &ctx.fileMatchingList);
		InsertTailList(global->resultList, &newNode->link);
	}
	terminate:
	// Cleanup file resources
	FileMapClose(&refFileMap);
	FileMapClose(&flipFileMap);
	return ret;
}


/*
 *  Recursively traverses the folder with the reference images and 
 *  find matches in transformed files dir
 *	Parameters:
 *		pathRefFiles - the reference files directory
 *		pathOutFiles - the transformed files directory
 *		flipType	 - what transformation to match (FLIP_HORIZONTALLY or FLIP_VERTICALLY)
 *		res			 - sentinel node of result list
 */

int ActiveThreads = 0;
int MaxThreads = 0;

typedef struct _data {
	int threadNumber;
	int sleepTime;
} data, *Pdata;

HANDLE threadEnd;
HANDLE acknowledge;
int threadsNum = 0;

DWORD CALLBACK workerThread(LPVOID param) {
	//Pdata data = (Pdata)param;

	//printf("worker thread %d started : sleeptime = %d\n", data->threadNumber, data->sleepTime);
	//Sleep(data->sleepTime);

	printf("worker thread started\n");

	PInfo info = (PInfo)param;
	info->action(info->dir, info->arg);

	SetEvent(threadEnd);
	//printf("worker thread %d ended\n", data->threadNumber);
	printf("worker thread done\n");

	threadsNum = threadsNum - 1;
	return 0;
}

void manager(PLIST_ENTRY workListHead) {

	MaxThreads = std::thread::hardware_concurrency();

	printf("%s, maximum threads: %d\n\n", "manager starting", MaxThreads);

	int sleepTime = 1000;
	//Pdata data;
	PLIST_ENTRY curr = workListHead->Flink;

	for (; curr != workListHead; curr = curr->Flink) {
		printf("%d\n", threadsNum);
		//data = (Pdata)malloc(sizeof(data));

		sleepTime = (sleepTime == 1000 ? 3000 : 1000);

		//data->sleepTime = sleepTime;
		//data->threadNumber = i;

		if (threadsNum >= MaxThreads) {
			ResetEvent(threadEnd);
			DWORD dwWaitResult = WaitForSingleObject(
				threadEnd, // event handle
				INFINITE);    // indefinite wait
			switch (dwWaitResult)
			{
				// Event object was signaled
			case WAIT_OBJECT_0:
				//
				// TODO: Read from the shared buffer
				//
				break;

				// An error occurred
			default:
				printf("Wait error (%d)\n", GetLastError());
				return;
			}
		}
		PInfo info = CONTAINING_RECORD(workListHead->Flink, Info, Link);
		
		if (threadsNum <= MaxThreads) {
			threadsNum = threadsNum + 1;
			QueueUserWorkItem(workerThread, info, WT_EXECUTEDEFAULT);
		}
	}

	while (threadsNum != 0) {
		printf("%d\n", threadsNum);
		Sleep(1000);
	}

	printf("done\n");
	getchar();
}

INT BMP_GetFlipsOfFilesInRefDir(LPCSTR pathRefFiles, LPCSTR pathOutFiles, FLIP_enum_t flipType, PLIST_ENTRY res) {

	MUTATIONS_RESULT_CTX ctx;
	InitGlobalCtx(&ctx, pathOutFiles, flipType, res);

	// Iterate through pathRefFiles directory and sub directories
	// invoking de processor (BMP_GetFlipsOfRefFile) for each ref file
	LIST_ENTRY workListHead;
	InitializeListHead(&workListHead);

	//if (!TraverseDirTree(pathRefFiles, ".bmp", BMP_GetFlipsOfRefFile, &ctx)) {
	if (!myTraverse(pathRefFiles, ".bmp", BMP_GetFlipsOfRefFile, &ctx, &workListHead)) {
		if (!OperHasError(&ctx))
			OperMarkError(&ctx, OPER_TRAVERSE_ERROR);
	}

	threadEnd = CreateEvent(NULL, FALSE, FALSE, "threadEnd");
	acknowledge = CreateEvent(NULL, FALSE, FALSE, "acknowledge");

	manager(&workListHead);

	return ctx.errorCode;
}