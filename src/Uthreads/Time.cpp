#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "UThread.h"


static DWORD factor = 2000;

VOID count_time(UT_ARGUMENT arg) {
	
	for (DWORD i = 0; i < factor; i++) {
		UtYield();
	}
}


DWORD time_test() { 

	DWORD start_time, end_time;
	
	UtInit();

	HANDLE handle_one = UtCreate(count_time, NULL, 0, "Thread_One");
	HANDLE handle_two = UtCreate(count_time, NULL, 0, "Thread_Two");

	start_time = GetTickCount();
	
	UtRun();

	end_time = GetTickCount();

	DWORD elapsed_time = end_time - start_time;
	DWORD total_time = elapsed_time / factor;
	//print result
	printf("time taken per content switch is: %d", total_time);

	return 0;
}