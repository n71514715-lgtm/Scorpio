#include "pch.h"
#include <Windows.h>

// Scorpio - Xbox One Translation Layer
// GameInput.dll - Xbox Input System Stub

extern "C" {

    __declspec(dllexport) HRESULT GameInputCreate(
        void** ppGameInput)
    {
        OutputDebugStringA("[Scorpio/GameInput] GameInputCreate intercepted!\n");
        OutputDebugStringA("[Scorpio/GameInput] Stubbing Xbox input system...\n");
        if (ppGameInput) *ppGameInput = nullptr;
        return S_OK;
    }

    __declspec(dllexport) void GameInputGetCurrentReading(
        void* pGameInput,
        int inputKind,
        void* pDevice,
        void** ppReading)
    {
        OutputDebugStringA("[Scorpio/GameInput] GameInputGetCurrentReading stubbed\n");
        if (ppReading) *ppReading = nullptr;
    }

    __declspec(dllexport) void GameInputGetDeviceInfo(
        void* pDevice,
        void** ppInfo)
    {
        OutputDebugStringA("[Scorpio/GameInput] GameInputGetDeviceInfo stubbed\n");
        if (ppInfo) *ppInfo = nullptr;
    }

} // extern "C"