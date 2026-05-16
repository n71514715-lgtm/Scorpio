#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <d3d12.h>

// Scorpio - Xbox One Translation Layer
// d3d12_x.dll - Xbox DirectX 12 Translation Stub

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

extern "C" {

    // Xbox D3D12CreateDevice ? redirect to real Windows D3D12CreateDevice
    __declspec(dllexport) HRESULT D3D12XboxCreateDevice(
        IUnknown* pAdapter,
        D3D_FEATURE_LEVEL minimumFeatureLevel,
        REFIID riid,
        void** ppDevice)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] D3D12XboxCreateDevice intercepted!\n");
        return D3D12CreateDevice(pAdapter, minimumFeatureLevel, riid, ppDevice);
    }

    // Xbox present ? stub for now
    __declspec(dllexport) void D3D12XboxPresentX(
        void* pCommandQueue,
        void* pPresentDesc)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] D3D12XboxPresentX intercepted!\n");
    }

    // Xbox GPU memory ? stub
    __declspec(dllexport) void D3D12XboxSetGpuMemoryConfiguration(
        void* pConfig)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] GPU memory config stubbed\n");
    }

} // extern "C"s
