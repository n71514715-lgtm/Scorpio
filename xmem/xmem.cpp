#include "pch.h"
#include <Windows.h>
#include <cstdlib>
#include <cstdint>

// Scorpio - Xbox One Translation Layer
// xmem.dll - Xbox Memory Manager Stub

extern "C" {

    __declspec(dllexport) void* XMemAlloc(
        size_t size,
        unsigned __int64 attributes)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemAlloc intercepted!\n");
        return malloc(size);
    }

    __declspec(dllexport) void XMemFree(
        void* pointer,
        unsigned __int64 attributes)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemFree intercepted!\n");
        free(pointer);
    }

    __declspec(dllexport) size_t XMemSize(
        void* pointer,
        unsigned __int64 attributes)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemSize intercepted!\n");
        return _msize(pointer);
    }
    __declspec(dllexport) void* XMemAllocatePhysicalPages(
        SIZE_T size,
        UINT32 flags)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemAllocatePhysicalPages stubbed\n");
        return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }

    __declspec(dllexport) void XMemFreePhysicalPages(
        void* pAddress,
        SIZE_T size)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemFreePhysicalPages stubbed\n");
        VirtualFree(pAddress, 0, MEM_RELEASE);
    }

    __declspec(dllexport) void* XMemMapPhysicalPages(
        void* pPhysical,
        SIZE_T size,
        UINT32 flags)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemMapPhysicalPages stubbed\n");
        return pPhysical;
    }
    __declspec(dllexport) void XMemGetWorkingSetStatistics(
        void* pStats)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemGetWorkingSetStatistics stubbed\n");
        if (pStats) memset(pStats, 0, 64);
    }

    __declspec(dllexport) void XMemResetWorkingSetStatistics()
    {
        OutputDebugStringA("[Scorpio/xmem] XMemResetWorkingSetStatistics stubbed\n");
    }

    __declspec(dllexport) HRESULT XMemCreateHeap(
        SIZE_T initialSize,
        UINT32 flags,
        void** ppHeap)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemCreateHeap stubbed\n");
        if (ppHeap) *ppHeap = HeapCreate(0, initialSize, 0);
        return S_OK;
    }

    __declspec(dllexport) HRESULT XMemDestroyHeap(
        void* pHeap)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemDestroyHeap stubbed\n");
        if (pHeap) HeapDestroy(pHeap);
        return S_OK;
    }
    __declspec(dllexport) void* XMemVirtualAlloc(
        void* lpAddress,
        SIZE_T dwSize,
        UINT32 flAllocationType,
        UINT32 flProtect,
        UINT32 flags)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemVirtualAlloc stubbed\n");
        return VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
    }

    __declspec(dllexport) BOOL XMemVirtualFree(
        void* lpAddress,
        SIZE_T dwSize,
        UINT32 dwFreeType)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemVirtualFree stubbed\n");
        return VirtualFree(lpAddress, dwSize, dwFreeType);
    }

    __declspec(dllexport) BOOL XMemVirtualProtect(
        void* lpAddress,
        SIZE_T  dwSize,
        UINT32  flNewProtect,
        UINT32* lpflOldProtect)
    {
        OutputDebugStringA("[Scorpio/xmem] XMemVirtualProtect stubbed\n");
        return VirtualProtect(lpAddress, dwSize, flNewProtect, (PDWORD)lpflOldProtect);
    }
    
} // extern "C"
