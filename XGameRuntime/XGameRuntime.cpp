#include "pch.h"
#include <Windows.h>
#include <string>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        OutputDebugStringA("[XGameRuntime] DLL LOADED INTO PROCESS!\n");
        char path[MAX_PATH];
        GetModuleFileNameA(hModule, path, MAX_PATH);
        OutputDebugStringA(("[XGameRuntime] Loaded from: "
            + std::string(path) + "\n").c_str());
    }
    return TRUE;
}

extern "C" {

    __declspec(dllexport) HRESULT XGameRuntimeInitialize() {
        OutputDebugStringA("[XGameRuntime] XGameRuntimeInitialize called!\n");
        return S_OK;  // SUCCESS
    }

    __declspec(dllexport) void XGameRuntimeUninitialize() {
        OutputDebugStringA("[XGameRuntime] XGameRuntimeUninitialize called!\n");
    }

    __declspec(dllexport) HRESULT XGameGetXboxTitleId(UINT32* pTitleId) {
        OutputDebugStringA("[XGameRuntime] XGameGetXboxTitleId called!\n");
        if (pTitleId) *pTitleId = 0x12345678;
        return S_OK;
    }

    __declspec(dllexport) HRESULT XStoreCreateContext(void* pUser, void** ppContext) {
        OutputDebugStringA("[XGameRuntime] XStoreCreateContext called!\n");
        if (ppContext) *ppContext = nullptr;
        return S_OK;
    }

    __declspec(dllexport) void XStoreCloseContextHandle(void* context) {
        OutputDebugStringA("[XGameRuntime] XStoreCloseContextHandle called!\n");
    }

    __declspec(dllexport) HRESULT XUserAddAsync(UINT32 options, void** ppAsync) {
        OutputDebugStringA("[XGameRuntime] XUserAddAsync called!\n");
        if (ppAsync) *ppAsync = nullptr;
        return S_OK;
    }

    __declspec(dllexport) HRESULT XUserGetId(void* user, UINT64* pId) {
        OutputDebugStringA("[XGameRuntime] XUserGetId called!\n");
        if (pId) *pId = 0;
        return S_OK;
    }

    __declspec(dllexport) HRESULT XSystemGetDeviceType(UINT32* pDeviceType) {
        OutputDebugStringA("[XGameRuntime] XSystemGetDeviceType called!\n");
        if (pDeviceType) *pDeviceType = 0;
        return S_OK;
    }

} // extern "C"