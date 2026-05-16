#include "pch.h"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// Scorpio - Xbox One Translation Layer
// xg_x.dll - Xbox Graphics Library Stub

extern "C" {

    __declspec(dllexport) HRESULT XGCreateDevice(
        void** ppDevice)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGCreateDevice intercepted!\n");
        HMODULE d3d12 = LoadLibraryA("d3d12.dll");
        if (!d3d12) return E_FAIL;
        auto createDevice = (decltype(&D3D12CreateDevice))
            GetProcAddress(d3d12, "D3D12CreateDevice");
        if (!createDevice) return E_FAIL;
        OutputDebugStringA("[Scorpio/xg_x] Redirecting to real D3D12!\n");
        return createDevice(nullptr, D3D_FEATURE_LEVEL_12_0,
            __uuidof(ID3D12Device), ppDevice);
    }

    __declspec(dllexport) void XGGetTextureLayout(
        void* pDesc,
        void* pLayout)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGGetTextureLayout stubbed\n");
    }

    __declspec(dllexport) UINT64 XGComputeTextureSizeAndAlignment(
        void* pDesc,
        void* pAlignment)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGComputeTextureSizeAndAlignment stubbed\n");
        return 0;
    }

    __declspec(dllexport) void XGGetGpuHardwareConfiguration(
        void* pConfig)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGGetGpuHardwareConfiguration stubbed\n");
    }

    __declspec(dllexport) HRESULT XGOpenMemoryObjectFromUnicodeString(
        void* pName,
        void** ppMemory)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGOpenMemoryObjectFromUnicodeString stubbed\n");
        return S_OK;
    }

    __declspec(dllexport) void XGGetDeviceContext(
        void* pDevice,
        void** ppContext)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGGetDeviceContext stubbed\n");
    }

} // extern "C"