#include "pch.h"
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("[Scorpio/GameInput] Xbox Input system loaded!\n");
        break;
    case DLL_PROCESS_DETACH:
        OutputDebugStringA("[Scorpio/GameInput] Xbox Input system unloaded\n");
        break;
    }
    return TRUE;
}