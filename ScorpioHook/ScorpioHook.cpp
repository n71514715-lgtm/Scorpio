#include "pch.h"
#include <Windows.h>
#include <string>

// ?? NT types ??????????????????????????????????????????????????
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef LONG NTSTATUS;
#define NT_SUCCESS(s) ((NTSTATUS)(s) >= 0)

typedef NTSTATUS(NTAPI* PFN_LdrLoadDll)(
    PWCHAR          PathToFile,
    ULONG* Flags,
    UNICODE_STRING* ModuleFileName,
    HANDLE* ModuleHandle
    );

// ?? Xbox DLL list ?????????????????????????????????????????????
static const wchar_t* XBOX_DLLS[] = {
    L"xgameruntime.dll", L"gameinput.dll",        L"xg_x.dll",
    L"d3d12_x.dll",      L"xmem.dll",             L"acphal.dll",
    L"xfrontpaneldisplay.dll", nullptr
};

static wchar_t  g_gameDir[MAX_PATH] = {};
static void* g_ldrFunc = nullptr;
#define         HOOK_BYTES 14
static BYTE     g_origBytes[HOOK_BYTES] = {};

// ?? Helpers ???????????????????????????????????????????????????
static const wchar_t* BaseName(const wchar_t* path) {
    const wchar_t* s = wcsrchr(path, L'\\');
    return s ? s + 1 : path;
}

static bool IsXboxName(const wchar_t* fname) {
    wchar_t lower[MAX_PATH] = {};
    wcsncpy_s(lower, fname, _TRUNCATE);
    for (int j = 0; lower[j]; j++) lower[j] = towlower(lower[j]);
    for (int i = 0; XBOX_DLLS[i]; i++)
        if (wcscmp(lower, XBOX_DLLS[i]) == 0) return true;
    return false;
}

static void Repatch();   // forward declare

// ?? Hooked LdrLoadDll ?????????????????????????????????????????
static NTSTATUS NTAPI HookedLdrLoadDll(
    PWCHAR          PathToFile,
    ULONG* Flags,
    UNICODE_STRING* ModuleFileName,
    HANDLE* ModuleHandle)
{
    // Restore original bytes so we can call the real function
    DWORD old;
    VirtualProtect(g_ldrFunc, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_ldrFunc, g_origBytes, HOOK_BYTES);
    VirtualProtect(g_ldrFunc, HOOK_BYTES, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_ldrFunc, HOOK_BYTES);

    PFN_LdrLoadDll realFn = (PFN_LdrLoadDll)g_ldrFunc;
    NTSTATUS result;

    if (ModuleFileName && ModuleFileName->Buffer && ModuleFileName->Length > 0) {
        int chars = ModuleFileName->Length / sizeof(wchar_t);
        wchar_t nameBuf[MAX_PATH] = {};
        if (chars < MAX_PATH) {
            wcsncpy_s(nameBuf, ModuleFileName->Buffer, chars);
            const wchar_t* fname = BaseName(nameBuf);
            if (IsXboxName(fname)) {
                wchar_t fullPath[MAX_PATH];
                swprintf_s(fullPath, L"%s\\%s", g_gameDir, fname);
                UNICODE_STRING newName;
                newName.Buffer = fullPath;
                newName.Length = (USHORT)(wcslen(fullPath) * sizeof(wchar_t));
                newName.MaximumLength = newName.Length + sizeof(wchar_t);
                ULONG noFlags = 0;
                result = realFn(nullptr, &noFlags, &newName, ModuleHandle);
                if (NT_SUCCESS(result)) {
                    OutputDebugStringW((std::wstring(
                        L"[ScorpioHook] Redirected: ") + fname + L"\n").c_str());
                    Repatch();
                    return result;
                }
                OutputDebugStringW((std::wstring(
                    L"[ScorpioHook] Redirect FAILED: ") + fname + L"\n").c_str());
            }
        }
    }

    result = realFn(PathToFile, Flags, ModuleFileName, ModuleHandle);
    Repatch();
    return result;
}

// ?? Re-install the hook ???????????????????????????????????????
static void Repatch() {
    BYTE jmp[HOOK_BYTES] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    *(ULONG_PTR*)(jmp + 6) = (ULONG_PTR)HookedLdrLoadDll;
    DWORD old;
    VirtualProtect(g_ldrFunc, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_ldrFunc, jmp, HOOK_BYTES);
    VirtualProtect(g_ldrFunc, HOOK_BYTES, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_ldrFunc, HOOK_BYTES);
}

// ?? Install hook ??????????????????????????????????????????????
static bool InstallLdrHook() {
    g_ldrFunc = (void*)GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "LdrLoadDll");
    if (!g_ldrFunc) {
        OutputDebugStringA("[ScorpioHook] LdrLoadDll not found!\n");
        return false;
    }

    DWORD old;
    VirtualProtect(g_ldrFunc, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_origBytes, g_ldrFunc, HOOK_BYTES);   // save originals

    BYTE jmp[HOOK_BYTES] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    *(ULONG_PTR*)(jmp + 6) = (ULONG_PTR)HookedLdrLoadDll;
    memcpy(g_ldrFunc, jmp, HOOK_BYTES);

    VirtualProtect(g_ldrFunc, HOOK_BYTES, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_ldrFunc, HOOK_BYTES);

    OutputDebugStringA("[ScorpioHook] LdrLoadDll hook installed!\n");
    return true;
}

// ?? DllMain ???????????????????????????????????????????????????
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        // Find game directory from EXE path
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        wchar_t* slash = wcsrchr(exePath, L'\\');
        if (slash) { *slash = L'\0'; wcscpy_s(g_gameDir, exePath); }

        OutputDebugStringW((std::wstring(
            L"[ScorpioHook] Game dir: ") + g_gameDir + L"\n").c_str());

        // Install LdrLoadDll hook — intercepts ALL DLL loads including
        // those using LOAD_LIBRARY_SEARCH_SYSTEM32
        InstallLdrHook();
    }
    return TRUE;
}