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

BOOL terminateThread_test() {
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
