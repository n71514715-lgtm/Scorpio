#include "pch.h"
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("[Scorpio/XFrontPanel] Xbox Front Panel stubbed!\n");
        break;
    }
    return TRUE;
}

