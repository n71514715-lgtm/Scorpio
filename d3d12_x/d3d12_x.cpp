#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <d3d12.h>
#pragma comment(linker, "/export:D3D12SerializeRootSignature=d3d12.D3D12SerializeRootSignature")
#pragma comment(linker, "/export:D3D12SerializeVersionedRootSignature=d3d12.D3D12SerializeVersionedRootSignature")
// Scorpio  -  Xbox One Translation Layer
// d3d12_x.dll - Xbox DirectX 12 Translation Stub
// d3d12_x internally needs XGameRuntime — stub it here too
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
    __declspec(dllexport) HRESULT D3DMapEsramMemory(
        void* pResource,
        void** ppData)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] D3DMapEsramMemory stubbed\n");
        if (ppData) *ppData = nullptr;
        return S_OK;
    }

    __declspec(dllexport) void D3DUnmapEsramMemory(
        void* pResource)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] D3DUnmapEsramMemory stubbed\n");
    }

    __declspec(dllexport) HRESULT D3DAllocateGraphicsMemory(
        SIZE_T size,
        SIZE_T alignment,
        UINT32 flags,
        void** ppMemory)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] D3DAllocateGraphicsMemory stubbed\n");
        if (ppMemory) *ppMemory = _aligned_malloc(size, alignment > 0 ? alignment : 16);
        return S_OK;
    }

    __declspec(dllexport) void D3DFreeGraphicsMemory(
        void* pMemory)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] D3DFreeGraphicsMemory stubbed\n");
        _aligned_free(pMemory);
    }
    __declspec(dllexport) void D3D12XboxSetProcessDebugFlags(
        UINT32 flags)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] D3D12XboxSetProcessDebugFlags stubbed\n");
    }

    

    __declspec(dllexport) void D3D12XboxSetDisplayMode(
        void* pCommandQueue,
        void* pDesc)
    {
        OutputDebugStringA("[Scorpio/d3d12_x] D3D12XboxSetDisplayMode stubbed\n");
    }


    
  // d3d12_x internally needs XGameRuntime — stub it here too
    __declspec(dllexport) HRESULT XGameRuntimeInitialize() { return S_OK; }

    

} // extern "C"s
