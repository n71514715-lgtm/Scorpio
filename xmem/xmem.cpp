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

} // extern "C"
