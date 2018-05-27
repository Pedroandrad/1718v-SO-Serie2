#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "UThread.h"

bool hasWork, hasAnswer;
HANDLE utServer, utClient;

BOOL serverStarted, clientStarted;
BOOL end;

void Client(UT_ARGUMENT arg) {
	printf("Client: begin\n");
	UtDump();
	clientStarted = true;
	if (!serverStarted) {
		printf("Client deactivate\n");
		UtDeactivate(); 
	}
	UtDump();
	hasWork = true;
	hasAnswer = false;
	printf("client: activate server\n");
	UtActivate(utServer);
	if (!hasAnswer) {
		printf("Client deactivate\n");
		UtDeactivate();
	}
	UtDump();
	end = true;
	hasWork = true;
	printf("Client: activate server\n");
	UtActivate(utServer);
	UtDump();
}

void Server(UT_ARGUMENT arg) {
	printf("Server: begin\n");
	UtDump();
	serverStarted = true;
	if (clientStarted) {
		printf("Server: activate client\n");
		UtActivate(utClient);
	}
	UtDump();
	for (;;) {
		if (!hasWork) {
			printf("Server deactivate\n");
			UtDeactivate();
		}
		if (end)
			break;
		hasAnswer = true;
		hasWork = false;
		UtDump();
		printf("Server: activate client\n");
		UtActivate(utClient);
	}
}

BOOL status_test() {

	hasWork = false;
	hasAnswer = false;
	serverStarted = false;
	clientStarted = false;
	end = false;

	printf("begin\n");
	UtInit();
	UtDump();

	utClient = UtCreate(Client, NULL, NULL, "Client");
	utServer = UtCreate(Server, NULL, NULL, "Server");

	UtRun();
	printf("before UtEnd()\n");
	UtDump();
	UtEnd();
	printf("after UtEnd()\n");
	UtDump();

	printf("end\nPress enter key to finish...\n");
	getchar();

	return 0;
}

BOOL multJoin_test() {
	return false;
}

HANDLE uThread1, uThread2, uThread3;
int step;
void thread1(UT_ARGUMENT arg) {
	// 1: blocks thread 1
	if (step == 1) step = 2;
	UtDeactivate();
}

void thread2(UT_ARGUMENT arg) {
	while (step != 2) UtYield();
	// 2: calls terminate for thread 1, waits untill its reactivated to terminate
	UtDump();
	UtTerminateThread(uThread1);
	// 3: activates thread1, terminating it
	UtActivate(uThread1);
	UtDump();
	// 4: yields for thread 3, thread2->status = ready
	step = 3;
	UtYield();
}

void thread3(UT_ARGUMENT arg) {
	while (step != 3) UtYield();
	UtDump();
	// 5: terminates thread2, waits until its the running thread
	UtTerminateThread(uThread2);
	// 6: yields for thread 2, terminating it
	printf("thread2 ended\n");
	UtDump();
	UtYield();
	// 7: terminates itself
	printf("thread3 ended\n");
	UtTerminateThread(uThread3);
}

BOOL terminateThread_test() {
	
	UtInit();
	step = 1;

	uThread1 = UtCreate(thread1, NULL, NULL, "uThread1");
	uThread2 = UtCreate(thread2, NULL, NULL, "uThread2");
	uThread3 = UtCreate(thread3, NULL, NULL, "uThread3");
	UtDump();

	UtRun();

	UtEnd();

	return false;
}

static DWORD factor = 2000;

VOID count_time(UT_ARGUMENT arg) {

	for (DWORD i = 0; i < factor; i++) {
		UtYield();
	}
}

VOID time_test() {

	DWORD start_time, end_time;

	UtInit();

	HANDLE handle_one = UtCreate(count_time, NULL, 0, "Thread_One");
	HANDLE handle_two = UtCreate(count_time, NULL, 0, "Thread_Two");

	start_time = GetTickCount();

	UtRun();

	end_time = GetTickCount();

	UtEnd();

	DWORD elapsed_time = end_time - start_time;
	DWORD total_time = elapsed_time / factor;
	//print result
	printf("time taken per content switch is: %d", total_time);
	getchar();
}

VOID time2_test() {
	return;
}

DWORD _tmain(DWORD argc, PTCHAR argv[]) {

	printf(" 1 : status test\n");
	printf(" 2 : multJoin test\n");
	printf(" 3 : terminateThread test\n");
	printf(" 4 : time test\n");
	printf(" 5 : time2 test\n");
	
	char a = '1';

	while (a != 6) 
	{
		printf("\nnext:");
		int number = getchar();

		switch (number - 48)
		{
		// status test
		case 1: { status_test(); break; }
		
		// mult join test
		case 2: { multJoin_test(); break; }
		
		// terminate thread test
		case 3: { terminateThread_test(); break; }
		
		// tempo de comutação (context switch) de threads na biblioteca UThread
		case 4: { time_test(); break; }

		// tempo de comutação de threads no sistema operativo Windows
		// tempo de comutação entre threads do mesmo processo e entre threads de processos distintos
		case 5: { time2_test(); break; }
		
		case 6: return 0;
		}
	}

	return 0;
}
