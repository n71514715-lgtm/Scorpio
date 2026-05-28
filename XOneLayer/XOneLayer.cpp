#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>

// Scorpio - Xbox One Translation Layer
// v0.0.9 - Full stub injection + ScorpioHook

void PrintHex(const std::vector<uint8_t>& data, size_t count) {
    for (size_t i = 0; i < count && i < data.size(); i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

struct XVCHeader {
    uint8_t  magic[4];
    uint32_t version;
    uint64_t contentId;
    uint8_t  contentType;
    uint64_t contentSize;
    uint8_t  reserved[491];
};

bool IsXVCFile(const std::vector<uint8_t>& header) {
    return header[0] == 0x43 && header[1] == 0x4F &&
        header[2] == 0x4E && header[3] == 0x54;
}

bool IsPEFile(const std::vector<uint8_t>& header) {
    return header[0] == 'M' && header[1] == 'Z';
}

void ParsePEHeader(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return;

    IMAGE_DOS_HEADER dosHeader;
    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) return;

    file.seekg(dosHeader.e_lfanew);
    IMAGE_NT_HEADERS64 ntHeaders;
    file.read(reinterpret_cast<char*>(&ntHeaders), sizeof(ntHeaders));
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) return;

    std::cout << "\n[Scorpio] ===== PE HEADER INFO =====" << std::endl;
    switch (ntHeaders.FileHeader.Machine) {
    case IMAGE_FILE_MACHINE_AMD64:
        std::cout << "[Scorpio] Machine: x64 (Xbox One)" << std::endl; break;
    default:
        std::cout << "[Scorpio] Machine: " << ntHeaders.FileHeader.Machine << std::endl;
    }
    std::cout << "[Scorpio] Entry point: 0x" << std::hex
        << ntHeaders.OptionalHeader.AddressOfEntryPoint << std::dec << std::endl;

    std::vector<IMAGE_SECTION_HEADER> sections(ntHeaders.FileHeader.NumberOfSections);
    file.read(reinterpret_cast<char*>(sections.data()),
        ntHeaders.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> fileData(fileSize);
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
    file.close();

    std::cout << "\n[Scorpio] ===== IMPORTED DLLs =====" << std::endl;
    std::vector<std::string> xboxAPIs = {
        "xgameruntime", "xboxservices", "gameruntime",
        "xaudio", "xinput", "xg_", "durango", "era", "xdk", "xbox"
    };

    IMAGE_DATA_DIRECTORY importDir =
        ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!importDir.VirtualAddress) return;

    auto RvaToOffset = [&](uint32_t rva) -> uint32_t {
        for (const auto& s : sections)
            if (rva >= s.VirtualAddress && rva < s.VirtualAddress + s.SizeOfRawData)
                return rva - s.VirtualAddress + s.PointerToRawData;
        return 0;
        };

    uint32_t importOffset = RvaToOffset(importDir.VirtualAddress);
    if (!importOffset || importOffset >= fileData.size()) return;

    auto* importDesc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
        fileData.data() + importOffset);

    bool foundXbox = false;
    while (importDesc->Name) {
        uint32_t nameOff = RvaToOffset(importDesc->Name);
        if (!nameOff || nameOff >= fileData.size()) break;
        std::string dllName = reinterpret_cast<char*>(fileData.data() + nameOff);
        std::cout << "[Scorpio] Imports from: " << dllName << std::endl;
        std::string lower = dllName;
        for (auto& c : lower) c = (char)tolower((unsigned char)c);
        for (const auto& api : xboxAPIs) {
            if (lower.find(api) != std::string::npos) {
                std::cout << "  ^^^ XBOX ONE API DETECTED!" << std::endl;
                foundXbox = true; break;
            }
        }
        importDesc++;
    }
    if (foundXbox)
        std::cout << "\n[Scorpio] Xbox One APIs found - translation layer needed!" << std::endl;
}

bool LoadXboxFile(const std::string& filePath) {
    std::cout << "[Scorpio] Loading: " << filePath << std::endl;
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) { std::cout << "[ERROR] Cannot open file!" << std::endl; return false; }
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg(); file.seekg(0, std::ios::beg);
    std::cout << "[Scorpio] File size: " << fileSize / (1024 * 1024) << " MB" << std::endl;
    std::vector<uint8_t> header(512);
    file.read(reinterpret_cast<char*>(header.data()), 512);
    file.close();
    PrintHex(header, 32);
    if (IsXVCFile(header)) std::cout << "[Scorpio] XVC Container detected!" << std::endl;
    else if (IsPEFile(header)) { std::cout << "[Scorpio] PE executable detected!" << std::endl; ParsePEHeader(filePath); }
    else                        std::cout << "[Scorpio] Unknown format" << std::endl;
    return true;
}

// ?? Copy a stub DLL — tries releaseName first, then debugName ??
static void CopyStub(const std::string& exeDir, const std::string& gameDir,
    const std::string& releaseName, const std::string& destName,
    const std::string& label) {
    // Try release filename first (e.g. d3d12x.dll), then debug (d3d12_x.dll)
    std::vector<std::string> srcs = { releaseName };
    if (releaseName != destName) srcs.push_back(destName);
    for (const auto& src : srcs) {
        std::string from = exeDir + "\\" + src;
        std::string to = gameDir + "\\" + destName;
        if (CopyFileA(from.c_str(), to.c_str(), FALSE)) {
            std::cout << "[Scorpio] " << label << " injected!" << std::endl;
            return;
        }
    }
    std::cout << "[Scorpio] WARNING: Could not copy " << label
        << " (Error " << GetLastError() << ")" << std::endl;
}

bool LaunchGame(const std::string& filePath) {
    std::cout << "\n[Scorpio] ===== LAUNCHING GAME =====" << std::endl;
    std::cout << "[Scorpio] Preparing to launch: " << filePath << std::endl;

    std::string gameDir = filePath.substr(0, filePath.find_last_of("\\/"));
    char selfBuf[MAX_PATH];
    GetModuleFileNameA(NULL, selfBuf, MAX_PATH);
    std::string exeDir = std::string(selfBuf)
        .substr(0, std::string(selfBuf).find_last_of("\\/"));

    // Inject stub DLLs — Release builds use different names so we try both
    CopyStub(exeDir, gameDir, "d3d12x.dll", "d3d12_x.dll", "d3d12_x.dll");
    CopyStub(exeDir, gameDir, "xgx.dll", "xg_x.dll", "xg_x.dll");
    CopyStub(exeDir, gameDir, "GameInput.dll", "GameInput.dll", "GameInput.dll");
    CopyStub(exeDir, gameDir, "xmem.dll", "xmem.dll", "xmem.dll");
    CopyStub(exeDir, gameDir, "XFrontPanelDisplay.dll", "XFrontPanelDisplay.dll", "XFrontPanelDisplay.dll");
    CopyStub(exeDir, gameDir, "AcpHal.dll", "AcpHal.dll", "AcpHal.dll");
    CopyStub(exeDir, gameDir, "XGameRuntime.dll", "XGameRuntime.dll", "XGameRuntime.dll");

    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);

    std::cout << "[Scorpio] Launching game process..." << std::endl;

    if (!CreateProcessA(filePath.c_str(), nullptr, nullptr, nullptr,
        FALSE, CREATE_SUSPENDED, nullptr,
        gameDir.c_str(), &si, &pi)) {
        std::cout << "[Scorpio] Launch failed! Error: " << GetLastError() << std::endl;
        return false;
    }

    std::cout << "[Scorpio] Game launched (suspended) PID: " << pi.dwProcessId << std::endl;

    // Inject ScorpioHook.dll before the game runs anything
    std::string hookPath = exeDir + "\\ScorpioHook.dll";
    bool hookOk = false;
    LPVOID mem = VirtualAllocEx(pi.hProcess, nullptr,
        hookPath.size() + 1,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (mem) {
        WriteProcessMemory(pi.hProcess, mem, hookPath.c_str(),
            hookPath.size() + 1, nullptr);
        HANDLE hThread = CreateRemoteThread(pi.hProcess, nullptr, 0,
            (LPTHREAD_START_ROUTINE)GetProcAddress(
                GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
            mem, 0, nullptr);
        if (hThread) {
            WaitForSingleObject(hThread, 5000);
            CloseHandle(hThread);
            hookOk = true;
            std::cout << "[Scorpio] ScorpioHook.dll injected - hooks active!" << std::endl;
        }
        else {
            std::cout << "[Scorpio] WARNING: Hook inject failed! Error: "
                << GetLastError() << std::endl;
        }
        VirtualFreeEx(pi.hProcess, mem, 0, MEM_RELEASE);
    }

    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    std::cout << "[Scorpio] Game exited with code: 0x"
        << std::hex << code << std::dec
        << " (" << code << ")" << std::endl;

    if (code == 0xC0000135) std::cout << "[Scorpio] DIAGNOSIS: Missing DLL!" << std::endl;
    else if (code == 0xC0000005) std::cout << "[Scorpio] DIAGNOSIS: Access Violation!" << std::endl;
    else if (code == 0xC000007B) std::cout << "[Scorpio] DIAGNOSIS: Bad Image Format!" << std::endl;
    else if (code == 0xC0000142) std::cout << "[Scorpio] DIAGNOSIS: DLL Init Failed!" << std::endl;
    else if (code == 0xC0000139) std::cout << "[Scorpio] DIAGNOSIS: Entry point not found!" << std::endl;
    else if (code == 0x00000001) std::cout << "[Scorpio] DIAGNOSIS: exit(1) - check DebugView for stack trace!" << std::endl;
    else if (code == 0x00000000) std::cout << "[Scorpio] Game exited cleanly!" << std::endl;
    else                         std::cout << "[Scorpio] Unknown exit code - check DebugView" << std::endl;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "================================" << std::endl;
    std::cout << "   Scorpio v0.0.9" << std::endl;
    std::cout << "   Xbox One Translation Layer" << std::endl;
    std::cout << "================================" << std::endl;

    if (argc < 2) {
        std::cout << "Usage: XOneLayer.exe <path to Terraria.exe>" << std::endl;
        return 1;
    }

    LoadXboxFile(argv[1]);

    std::cout << "\n[Scorpio] Launch game? (y/n): ";
    char choice; std::cin >> choice;
    if (choice == 'y' || choice == 'Y') LaunchGame(argv[1]);

    std::cout << "\n[Scorpio] Done!" << std::endl;
    system("pause");
    return 0;
}