#include <windows.h>
#include <stdio.h>
#include <string>

#define DLL_PATH "Codes.dll"

BOOL dllInjector(const char* dllpath, DWORD pID);

int main(int argc, char** argv)
{   
    // Create Process SUSPENDED
    PROCESS_INFORMATION pi;
    STARTUPINFOA Startup;
    ZeroMemory(&Startup, sizeof(Startup));
    ZeroMemory(&pi, sizeof(pi));

    if (argc < 2) {
        //printf("Usage: %s prog_name \n", argv[0]);
        return 1;
    }

    std::string lpCmdArgs = std::string(argv[1]) + " ROBBER_CAPTURED whatever";
    
    if (CreateProcessA(NULL, (LPSTR)lpCmdArgs.c_str(), NULL, NULL, NULL, CREATE_SUSPENDED, NULL, NULL, &Startup, &pi) == FALSE)
    {
        //printf("couldnt open process %s\n", lpCmdArgs.c_str());
        return 1;
    }

    if (!(dllInjector(DLL_PATH, pi.dwProcessId))) {
        //printf("couldnt inject dll");
        return 1;
    }

    Sleep(1000); // Let the DLL finish loading
    ResumeThread(pi.hThread);
    //printf("Injected dll successfully\n");

    return 0;
}

BOOL dllInjector(const char* dllpath, DWORD pID)
{
    HANDLE pHandle;
    LPVOID remoteString;
    LPVOID remoteLoadLib;

    pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

    if (!pHandle) {
        printf("couldnt open proccess with perms\n");
        return false;
    }        

    remoteLoadLib = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

    remoteString = (LPVOID)VirtualAllocEx(pHandle, NULL, strlen(DLL_PATH) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(pHandle, (LPVOID)remoteString, dllpath, strlen(dllpath), NULL);
    if (NULL == CreateRemoteThread(pHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)remoteLoadLib, (LPVOID)remoteString, NULL, NULL)) {
        return false;
    }
    CloseHandle(pHandle);

    return true;
}