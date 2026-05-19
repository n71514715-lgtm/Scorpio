#include "pch.h"
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("[Scorpio/AcpHal] Xbox Controller HAL loaded!\n");
        OutputDebugStringA("[Scorpio/AcpHal] Stubbing Xbox controller protocol...\n");
        break;
    case DLL_PROCESS_DETACH:
        OutputDebugStringA("[Scorpio/AcpHal] Xbox Controller HAL unloaded\n");
        break;
    }
    return TRUE;
}

