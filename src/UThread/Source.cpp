#include <stdio.h>
#include <windows.h>
#include "UThread.h"


int main() {

	HANDLE ThreadA = UtCreate(NULL, NULL, NULL, "Thread A");
	HANDLE ThreadB = UtCreate(NULL, NULL, NULL, "Thread B");

	

	return 0;
}