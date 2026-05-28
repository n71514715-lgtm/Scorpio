#include "pch.h"
#include <Windows.h>
#include <string>
#include <stdio.h>

extern "C" USHORT WINAPI RtlCaptureStackBackTrace(ULONG, ULONG, PVOID*, PULONG);

// ?? NT types ??????????????????????????????????????????????????
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef LONG NTSTATUS;
#define NT_SUCCESS(s) ((NTSTATUS)(s) >= 0)

typedef NTSTATUS(NTAPI* PFN_LdrLoadDll)(PWCHAR, ULONG*, UNICODE_STRING*, HANDLE*);
typedef FARPROC(WINAPI* PFN_GetProcAddress)(HMODULE, LPCSTR);
typedef void    (WINAPI* PFN_ExitProcess)(UINT);
typedef BOOL(WINAPI* PFN_TerminateProcess)(HANDLE, UINT);
typedef void    (*PFN_exit)(int);

// ?? Xbox DLL list ?????????????????????????????????????????????
static const wchar_t* XBOX_DLLS[] = {
    L"gameinput.dll",  L"xg_x.dll",
    L"d3d12_x.dll",    L"xmem.dll",       L"acphal.dll",
    L"xfrontpaneldisplay.dll", nullptr
};

static wchar_t g_gameDir[MAX_PATH] = {};
static void* g_ldrFunc = nullptr;
#define        HOOK_BYTES 14
static BYTE    g_origBytes[HOOK_BYTES] = {};

static PFN_GetProcAddress   g_origGPA = nullptr;
static PFN_ExitProcess      g_origExitProcess = nullptr;
static PFN_TerminateProcess g_origTerminateProcess = nullptr;
static PFN_exit             g_origExit = nullptr;

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

// ?? Stack trace ???????????????????????????????????????????????
static void PrintStack(const char* reason) {
    char header[128];
    sprintf_s(header, "[ScorpioHook] *** %s *** stack:\n", reason);
    OutputDebugStringA(header);
    void* stack[32] = {};
    USHORT n = RtlCaptureStackBackTrace(1, 32, stack, nullptr);
    for (USHORT i = 0; i < n; i++) {
        char mod[MAX_PATH] = "?";
        HMODULE hm = nullptr;
        if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR)stack[i], &hm))
            GetModuleFileNameA(hm, mod, MAX_PATH);
        char line[640];
        sprintf_s(line, "[ScorpioHook]   [%02u] %p  %s\n", (UINT)i, stack[i], mod);
        OutputDebugStringA(line);
    }
}

// ?? IAT patcher ???????????????????????????????????????????????
static void PatchIAT(HMODULE hMod, const char* dll, const char* fn,
    void* hook, void** ppOrig) {
    if (!hMod) return;
    __try {
        BYTE* base = (BYTE*)hMod;
        auto* dos = (IMAGE_DOS_HEADER*)base;
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return;
        auto* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
        auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        if (!dir.VirtualAddress) return;
        auto* desc = (IMAGE_IMPORT_DESCRIPTOR*)(base + dir.VirtualAddress);
        for (; desc->Name; desc++) {
            if (_stricmp((char*)(base + desc->Name), dll) != 0) continue;
            auto* orig = (IMAGE_THUNK_DATA*)(base + desc->OriginalFirstThunk);
            auto* thunk = (IMAGE_THUNK_DATA*)(base + desc->FirstThunk);
            for (; orig->u1.AddressOfData; orig++, thunk++) {
                if (IMAGE_SNAP_BY_ORDINAL(orig->u1.Ordinal)) continue;
                auto* ibn = (IMAGE_IMPORT_BY_NAME*)(base + orig->u1.AddressOfData);
                if (_stricmp((char*)ibn->Name, fn) != 0) continue;
                if (ppOrig && !*ppOrig) *ppOrig = (void*)thunk->u1.Function;
                DWORD old;
                VirtualProtect(&thunk->u1.Function, sizeof(ULONG_PTR),
                    PAGE_READWRITE, &old);
                thunk->u1.Function = (ULONG_PTR)hook;
                VirtualProtect(&thunk->u1.Function, sizeof(ULONG_PTR),
                    old, &old);
                return;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

// ?? Hook functions ????????????????????????????????????????????
static void HookedExit(int code) {
    char msg[64];
    sprintf_s(msg, "exit(%d)", code);
    PrintStack(msg);
    g_origExit(code);
}

static void WINAPI HookedExitProcess(UINT code) {
    char msg[64];
    sprintf_s(msg, "ExitProcess(%u)", code);
    PrintStack(msg);
    g_origExitProcess(code);
}

static BOOL WINAPI HookedTerminateProcess(HANDLE hProcess, UINT code) {
    if (hProcess == GetCurrentProcess() || hProcess == (HANDLE)-1) {
        char msg[64];
        sprintf_s(msg, "TerminateProcess(self,%u)", code);
        PrintStack(msg);
    }
    return g_origTerminateProcess(hProcess, code);
}

static FARPROC WINAPI HookedGetProcAddress(HMODULE hMod, LPCSTR lpName) {
    FARPROC result = g_origGPA(hMod, lpName);
    if (hMod && lpName && (ULONG_PTR)lpName > 0xFFFF) {
        wchar_t path[MAX_PATH] = {};
        GetModuleFileNameW(hMod, path, MAX_PATH);
        wchar_t low[MAX_PATH] = {};
        wcsncpy_s(low, BaseName(path), _TRUNCATE);
        for (int j = 0; low[j]; j++) low[j] = towlower(low[j]);
        if (wcsstr(low, L"xgameruntime") || wcsstr(low, L"d3d12") ||
            wcsstr(low, L"xg_x") || wcsstr(low, L"xgx") ||
            wcsstr(low, L"gameinput")) {
            char line[512];
            sprintf_s(line, result
                ? "[ScorpioHook] GPA HIT:  [%ws] %s\n"
                : "[ScorpioHook] GPA MISS: [%ws] %s  <-- MISSING!\n",
                BaseName(path), lpName);
            OutputDebugStringA(line);
        }
    }
    return result;
}

// ?? LdrLoadDll hook ???????????????????????????????????????????
static void Repatch();

static NTSTATUS NTAPI HookedLdrLoadDll(
    PWCHAR PathToFile, ULONG* Flags,
    UNICODE_STRING* ModuleFileName, HANDLE* ModuleHandle)
{
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

static void Repatch() {
    BYTE jmp[HOOK_BYTES] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    *(ULONG_PTR*)(jmp + 6) = (ULONG_PTR)HookedLdrLoadDll;
    DWORD old;
    VirtualProtect(g_ldrFunc, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_ldrFunc, jmp, HOOK_BYTES);
    VirtualProtect(g_ldrFunc, HOOK_BYTES, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_ldrFunc, HOOK_BYTES);
}

static bool InstallLdrHook() {
    g_ldrFunc = (void*)GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrLoadDll");
    if (!g_ldrFunc) { OutputDebugStringA("[ScorpioHook] LdrLoadDll not found!\n"); return false; }
    DWORD old;
    VirtualProtect(g_ldrFunc, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_origBytes, g_ldrFunc, HOOK_BYTES);
    BYTE jmp[HOOK_BYTES] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    *(ULONG_PTR*)(jmp + 6) = (ULONG_PTR)HookedLdrLoadDll;
    memcpy(g_ldrFunc, jmp, HOOK_BYTES);
    VirtualProtect(g_ldrFunc, HOOK_BYTES, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_ldrFunc, HOOK_BYTES);
    OutputDebugStringA("[ScorpioHook] LdrLoadDll hook installed!\n");
    return true;
}

// ?? DoInstallHooks — plain C function so __try works in DllMain ??
static void DoInstallHooks() {
    // Get game directory
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    wchar_t* slash = wcsrchr(exePath, L'\\');
    if (slash) { *slash = L'\0'; wcscpy_s(g_gameDir, exePath); }

    wchar_t msg[MAX_PATH + 64];
    wcscpy_s(msg, L"[ScorpioHook] Game dir: ");
    wcscat_s(msg, g_gameDir);
    wcscat_s(msg, L"\n");
    OutputDebugStringW(msg);

    // Hook LdrLoadDll to redirect Xbox DLL loads
    InstallLdrHook();

    // Hook Terraria.exe's IAT
    HMODULE hGame = GetModuleHandleW(nullptr);

    PatchIAT(hGame, "kernel32.dll", "GetProcAddress",
        (void*)HookedGetProcAddress, (void**)&g_origGPA);
    if (!g_origGPA) g_origGPA = GetProcAddress;

    PatchIAT(hGame, "kernel32.dll", "ExitProcess",
        (void*)HookedExitProcess, (void**)&g_origExitProcess);
    if (!g_origExitProcess) g_origExitProcess = ExitProcess;

    PatchIAT(hGame, "kernel32.dll", "TerminateProcess",
        (void*)HookedTerminateProcess, (void**)&g_origTerminateProcess);
    if (!g_origTerminateProcess) g_origTerminateProcess = TerminateProcess;

    // Hook exit() — Terraria uses api-ms-win-crt-runtime-l1-1-0.dll
    PatchIAT(hGame, "api-ms-win-crt-runtime-l1-1-0.dll", "exit",
        (void*)HookedExit, (void**)&g_origExit);
    if (!g_origExit) g_origExit = exit;

    OutputDebugStringA("[ScorpioHook] All hooks installed!\n");
}

// ?? DllMain ???????????????????????????????????????????????????
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        __try {
            DoInstallHooks();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            OutputDebugStringA("[ScorpioHook] EXCEPTION in DllMain!\n");
        }
    }
    return TRUE;
}