#include "pch.h"
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("[Scorpio/xg_x] Xbox Graphics library loaded!\n");
        OutputDebugStringA("[Scorpio/xg_x] Intercepting Xbox GPU calls...\n");
        break;
    case DLL_PROCESS_DETACH:
        OutputDebugStringA("[Scorpio/xg_x] Xbox Graphics library unloaded\n");
        break;
    }
    return TRUE;
}
