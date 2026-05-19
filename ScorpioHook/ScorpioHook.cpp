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
    PWCHAR, ULONG*, UNICODE_STRING*, HANDLE*);

typedef FARPROC(WINAPI* PFN_GetProcAddress)(HMODULE, LPCSTR);

// ?? Xbox DLL list ?????????????????????????????????????????????
static const wchar_t* XBOX_DLLS[] = {
    L"xgameruntime.dll", L"gameinput.dll",  L"xg_x.dll",
    L"d3d12_x.dll",      L"xmem.dll",       L"acphal.dll",
    L"xfrontpaneldisplay.dll", nullptr
};

static wchar_t g_gameDir[MAX_PATH] = {};
static void* g_ldrFunc = nullptr;
#define        HOOK_BYTES 14
static BYTE    g_origBytes[HOOK_BYTES] = {};

static PFN_GetProcAddress g_origGetProcAddress = nullptr;

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

// ?? IAT patcher ???????????????????????????????????????????????
static void PatchIAT(HMODULE hMod, const char* dll, const char* fn,
    void* hook, void** ppOrig) {
    if (!hMod) return;
    BYTE* base = (BYTE*)hMod;
    __try {
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

// ?? Hooked GetProcAddress — logs xgameruntime lookups ?????????
static FARPROC WINAPI HookedGetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    FARPROC result = g_origGetProcAddress(hModule, lpProcName);

    if (hModule && lpProcName && ((ULONG_PTR)lpProcName > 0xFFFF)) {
        wchar_t modPath[MAX_PATH] = {};
        GetModuleFileNameW(hModule, modPath, MAX_PATH);
        const wchar_t* modName = BaseName(modPath);
        wchar_t lower[MAX_PATH] = {};
        wcsncpy_s(lower, modName, _TRUNCATE);
        for (int j = 0; lower[j]; j++) lower[j] = towlower(lower[j]);

        if (wcsstr(lower, L"xgameruntime") || wcsstr(lower, L"gameinput")) {
            if (result) {
                OutputDebugStringA((std::string("[ScorpioHook] GetProcAddress HIT: ")
                    + lpProcName + "\n").c_str());
            }
            else {
                OutputDebugStringA((std::string("[ScorpioHook] GetProcAddress MISS: ")
                    + lpProcName + " <-- MISSING!\n").c_str());
            }
        }
    }
    return result;
}

// ?? Forward declare Repatch ???????????????????????????????????
static void Repatch();

// ?? Hooked LdrLoadDll ?????????????????????????????????????????
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

// ?? Re-install LdrLoadDll hook ????????????????????????????????
static void Repatch() {
    BYTE jmp[HOOK_BYTES] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    *(ULONG_PTR*)(jmp + 6) = (ULONG_PTR)HookedLdrLoadDll;
    DWORD old;
    VirtualProtect(g_ldrFunc, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_ldrFunc, jmp, HOOK_BYTES);
    VirtualProtect(g_ldrFunc, HOOK_BYTES, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_ldrFunc, HOOK_BYTES);
}

// ?? Install LdrLoadDll hook ???????????????????????????????????
static bool InstallLdrHook() {
    g_ldrFunc = (void*)GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "LdrLoadDll");
    if (!g_ldrFunc) {
        OutputDebugStringA("[ScorpioHook] LdrLoadDll not found!\n");
        return false;
    }
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

// ?? DllMain ???????????????????????????????????????????????????
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        wchar_t* slash = wcsrchr(exePath, L'\\');
        if (slash) { *slash = L'\0'; wcscpy_s(g_gameDir, exePath); }

        OutputDebugStringW((std::wstring(
            L"[ScorpioHook] Game dir: ") + g_gameDir + L"\n").c_str());

        // Hook LdrLoadDll to redirect Xbox DLL loads
        InstallLdrHook();

        // Hook GetProcAddress to see what xgameruntime functions Terraria needs
        HMODULE hGame = GetModuleHandleW(nullptr);
        PatchIAT(hGame, "kernel32.dll", "GetProcAddress",
            (void*)HookedGetProcAddress, (void**)&g_origGetProcAddress);
        if (!g_origGetProcAddress)
            g_origGetProcAddress = GetProcAddress;

        OutputDebugStringA("[ScorpioHook] GetProcAddress hook installed!\n");
    }
    return TRUE;
}