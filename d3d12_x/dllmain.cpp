#include "pch.h"
#include <iostream>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("[Scorpio/d3d12_x] Xbox DirectX translation layer loaded!\n");
        break;
    case DLL_PROCESS_DETACH:
        OutputDebugStringA("[Scorpio/d3d12_x] Translation layer unloaded\n");
        break;
    }
    return TRUE;
}
