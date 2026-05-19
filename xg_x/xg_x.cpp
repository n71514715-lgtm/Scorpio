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
    __declspec(dllexport) HRESULT XGCreateTextureComputer(
        void* pDevice,
        void** ppComputer)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGCreateTextureComputer stubbed\n");
        if (ppComputer) *ppComputer = nullptr;
        return S_OK;
    }

    __declspec(dllexport) HRESULT XGCreateTextureFetcher(
        void* pDevice,
        void** ppFetcher)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGCreateTextureFetcher stubbed\n");
        if (ppFetcher) *ppFetcher = nullptr;
        return S_OK;
    }

    __declspec(dllexport) HRESULT XGCreateSurfaceComputer(
        void* pDevice,
        void** ppComputer)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGCreateSurfaceComputer stubbed\n");
        if (ppComputer) *ppComputer = nullptr;
        return S_OK;
    }
    __declspec(dllexport) HRESULT XGComputeOptimalTileMode(
        void* pDesc,
        void* pTileMode)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGComputeOptimalTileMode stubbed\n");
        if (pTileMode) memset(pTileMode, 0, 16);
        return S_OK;
    }
    __declspec(dllexport) HRESULT XGCreateTexture2DComputer(
        void* pDevice, void** ppComputer)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGCreateTexture2DComputer stubbed\n");
        if (ppComputer) *ppComputer = nullptr;
        return S_OK;
    }

    __declspec(dllexport) HRESULT XGCreateTexture3DComputer(
        void* pDevice, void** ppComputer)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGCreateTexture3DComputer stubbed\n");
        if (ppComputer) *ppComputer = nullptr;
        return S_OK;
    }

    __declspec(dllexport) HRESULT XGCreateTextureCubeComputer(
        void* pDevice, void** ppComputer)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGCreateTextureCubeComputer stubbed\n");
        if (ppComputer) *ppComputer = nullptr;
        return S_OK;
    }

    __declspec(dllexport) HRESULT XGCreateTextureArrayComputer(
        void* pDevice, void** ppComputer)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGCreateTextureArrayComputer stubbed\n");
        if (ppComputer) *ppComputer = nullptr;
        return S_OK;
    }

    __declspec(dllexport) void XGSetTextureHeader(
        void* pDesc, void* pHeader)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGSetTextureHeader stubbed\n");
    }

    __declspec(dllexport) void XGGetTextureDesc(
        void* pHeader, UINT32 mip, void* pDesc)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGGetTextureDesc stubbed\n");
    }

    __declspec(dllexport) UINT32 XGComputeTextureSize(
        void* pHeader)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGComputeTextureSize stubbed\n");
        return 0;
    }

    __declspec(dllexport) HRESULT XGComputeSurfaceInfo(
        void* pDesc, void* pInfo)
    {
        OutputDebugStringA("[Scorpio/xg_x] XGComputeSurfaceInfo stubbed\n");
        return S_OK;
    }
    
    

} // extern "C"