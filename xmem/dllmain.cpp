#include "pch.h"
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("[Scorpio/xmem] Xbox Memory Manager loaded!\n");
        break;
    case DLL_PROCESS_DETACH:
        OutputDebugStringA("[Scorpio/xmem] Xbox Memory Manager unloaded\n");
        break;
    }
    return TRUE;
}
