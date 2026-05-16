#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>

// Scorpio - Xbox One Translation Layer
// v0.0.3 - PE Header Parser + Xbox API Detection

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

// NEW - Parse PE header and find Xbox imports
void ParsePEHeader(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return;

    // Read DOS header
    IMAGE_DOS_HEADER dosHeader;
    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));

    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        std::cout << "[Scorpio] Not a valid PE file!" << std::endl;
        return;
    }

    // Jump to PE header
    file.seekg(dosHeader.e_lfanew);

    // Read NT headers
    IMAGE_NT_HEADERS64 ntHeaders;
    file.read(reinterpret_cast<char*>(&ntHeaders), sizeof(ntHeaders));

    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        std::cout << "[Scorpio] Invalid PE signature!" << std::endl;
        return;
    }

    std::cout << "\n[Scorpio] ===== PE HEADER INFO =====" << std::endl;
    std::cout << "[Scorpio] Machine type: ";
    switch (ntHeaders.FileHeader.Machine) {
    case IMAGE_FILE_MACHINE_AMD64:
        std::cout << "x64 (Xbox One compatible)" << std::endl;
        break;
    case IMAGE_FILE_MACHINE_I386:
        std::cout << "x86" << std::endl;
        break;
    default:
        std::cout << "Unknown: " << ntHeaders.FileHeader.Machine << std::endl;
    }

    std::cout << "[Scorpio] Number of sections: "
        << ntHeaders.FileHeader.NumberOfSections << std::endl;
    std::cout << "[Scorpio] Image base: 0x"
        << std::hex << ntHeaders.OptionalHeader.ImageBase << std::dec << std::endl;
    std::cout << "[Scorpio] Entry point: 0x"
        << std::hex << ntHeaders.OptionalHeader.AddressOfEntryPoint
        << std::dec << std::endl;
    std::cout << "[Scorpio] Size of image: "
        << ntHeaders.OptionalHeader.SizeOfImage << " bytes" << std::endl;

    // Read sections
    std::cout << "\n[Scorpio] ===== SECTIONS =====" << std::endl;
    std::vector<IMAGE_SECTION_HEADER> sections(ntHeaders.FileHeader.NumberOfSections);
    file.read(reinterpret_cast<char*>(sections.data()),
        ntHeaders.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

    for (const auto& section : sections) {
        char name[9] = {};
        memcpy(name, section.Name, 8);
        std::cout << "[Scorpio] Section: " << name
            << " | Size: " << section.SizeOfRawData
            << " | VA: 0x" << std::hex << section.VirtualAddress
            << std::dec << std::endl;
    }

    // Read entire file for import parsing
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> fileData(fileSize);
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
    file.close();

    // Parse imports
    std::cout << "\n[Scorpio] ===== IMPORTED DLLs =====" << std::endl;

    // Xbox specific DLLs we're looking for
    std::vector<std::string> xboxAPIs = {
        "xgameruntime", "xboxservices", "gameruntime",
        "xaudio", "xinput", "xg_", "durango",
        "era", "xdk", "xbox"
    };

    IMAGE_DATA_DIRECTORY importDir =
        ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (importDir.VirtualAddress == 0) {
        std::cout << "[Scorpio] No import table found!" << std::endl;
        return;
    }

    // Convert RVA to file offset using sections
    auto RvaToOffset = [&](uint32_t rva) -> uint32_t {
        for (const auto& section : sections) {
            if (rva >= section.VirtualAddress &&
                rva < section.VirtualAddress + section.SizeOfRawData) {
                return rva - section.VirtualAddress + section.PointerToRawData;
            }
        }
        return 0;
        };

    uint32_t importOffset = RvaToOffset(importDir.VirtualAddress);
    if (importOffset == 0 || importOffset >= fileData.size()) {
        std::cout << "[Scorpio] Could not locate import table!" << std::endl;
        return;
    }

    // Walk import descriptors
    IMAGE_IMPORT_DESCRIPTOR* importDesc =
        reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(fileData.data() + importOffset);

    bool foundXboxAPI = false;
    while (importDesc->Name != 0) {
        uint32_t nameOffset = RvaToOffset(importDesc->Name);
        if (nameOffset == 0 || nameOffset >= fileData.size()) break;

        std::string dllName = reinterpret_cast<char*>(fileData.data() + nameOffset);
        std::cout << "[Scorpio] Imports from: " << dllName << std::endl;

        // Check if it's an Xbox specific API
        std::string dllLower = dllName;
        for (auto& c : dllLower) c = tolower(c);

        for (const auto& xboxAPI : xboxAPIs) {
            if (dllLower.find(xboxAPI) != std::string::npos) {
                std::cout << "  ^^^ XBOX ONE API DETECTED! - Scorpio needs to implement this!" << std::endl;
                foundXboxAPI = true;
                break;
            }
        }
        importDesc++;
    }

    if (foundXboxAPI) {
        std::cout << "\n[Scorpio] Xbox One APIs found - translation layer needed!" << std::endl;
    }
    else {
        std::cout << "\n[Scorpio] No Xbox specific APIs detected" << std::endl;
    }
}

bool LoadXboxFile(const std::string& filePath) {
    std::cout << "[Scorpio] Attempting to load: " << filePath << std::endl;

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "[ERROR] Cannot open file!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "[Scorpio] File size: " << fileSize << " bytes ("
        << fileSize / (1024 * 1024) << " MB)" << std::endl;

    std::vector<uint8_t> header(512);
    file.read(reinterpret_cast<char*>(header.data()), 512);
    file.close();

    std::cout << "[Scorpio] File header bytes:" << std::endl;
    PrintHex(header, 32);

    if (IsXVCFile(header)) {
        std::cout << "[Scorpio] XVC Container detected!" << std::endl;
        std::cout << "[Scorpio] This is an Xbox One game package!" << std::endl;
        XVCHeader* xvcHeader = reinterpret_cast<XVCHeader*>(header.data());
        std::cout << "[Scorpio] XVC Version: " << xvcHeader->version << std::endl;
        std::cout << "[Scorpio] Content Size: " << xvcHeader->contentSize << " bytes" << std::endl;
    }
    else if (IsPEFile(header)) {
        std::cout << "[Scorpio] Valid PE executable detected!" << std::endl;
        std::cout << "[Scorpio] This looks like an Xbox One executable!" << std::endl;
        ParsePEHeader(filePath);
    }
    else {
        std::cout << "[Scorpio] Unknown file format" << std::endl;
        printf("First 4 bytes: %02X %02X %02X %02X\n",
            header[0], header[1], header[2], header[3]);
    }

    return true;
}bool LaunchGame(const std::string& filePath) {
    std::cout << "\n[Scorpio] ===== LAUNCHING GAME =====" << std::endl;
    std::cout << "[Scorpio] Preparing to launch: " << filePath << std::endl;

    std::string gameDir = filePath.substr(0, filePath.find_last_of("\\/"));
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir = std::string(exePath).substr(0, std::string(exePath).find_last_of("\\/"));
    std::string dllSource = exeDir + "\\d3d12_x.dll";
    std::string dllDest = gameDir + "\\d3d12_x.dll";

    if (CopyFileA(dllSource.c_str(), dllDest.c_str(), FALSE)) {
        std::cout << "[Scorpio] d3d12_x.dll injected into game directory!" << std::endl;
    }
    else {
        std::cout << "[Scorpio] Warning: Could not copy d3d12_x.dll" << std::endl;
    }
    // Copy xg_x.dll
    std::string xgxSource = exeDir + "\\xgx.dll";
    std::string xgxDest = gameDir + "\\xg_x.dll";
    if (CopyFileA(xgxSource.c_str(), xgxDest.c_str(), FALSE)) {
        std::cout << "[Scorpio] xg_x.dll injected!" << std::endl;
    }

    // Copy GameInput.dll
    std::string giSource = exeDir + "\\GameInput.dll";
    std::string giDest = gameDir + "\\GameInput.dll";
    if (CopyFileA(giSource.c_str(), giDest.c_str(), FALSE)) {
        std::cout << "[Scorpio] GameInput.dll injected!" << std::endl;
    }
    // Copy xmem.dll
    std::string xmemSource = exeDir + "\\xmem.dll";
    std::string xmemDest = gameDir + "\\xmem.dll";
    if (CopyFileA(xmemSource.c_str(), xmemDest.c_str(), FALSE)) {
        std::cout << "[Scorpio] xmem.dll injected!" << std::endl;
    }

    // Copy XFrontPanelDisplay.dll
    std::string xfpSource = exeDir + "\\XFrontPanelDisplay.dll";
    std::string xfpDest = gameDir + "\\XFrontPanelDisplay.dll";
    if (CopyFileA(xfpSource.c_str(), xfpDest.c_str(), FALSE)) {
        std::cout << "[Scorpio] XFrontPanelDisplay.dll injected!" << std::endl;
    }
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);

    std::cout << "[Scorpio] Launching game process..." << std::endl;

    if (CreateProcessA(
        filePath.c_str(),
        nullptr, nullptr, nullptr,
        FALSE, 0, nullptr,
        gameDir.c_str(),
        &si, &pi))
    {
        std::cout << "[Scorpio] Game launched! PID: " << pi.dwProcessId << std::endl;
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        std::cout << "[Scorpio] Game exited with code: 0x"
            << std::hex << exitCode << std::dec << std::endl;

        if (exitCode == 0xC0000135) {
            std::cout << "[Scorpio] DIAGNOSIS: Missing DLL!" << std::endl;
        }
        else if (exitCode == 0xC0000005) {
            std::cout << "[Scorpio] DIAGNOSIS: Access Violation!" << std::endl;
        }
        else if (exitCode == 0xC000007B) {
            std::cout << "[Scorpio] DIAGNOSIS: Bad Image Format!" << std::endl;
        }
        else if (exitCode == 0xC0000142) {
            std::cout << "[Scorpio] DIAGNOSIS: DLL Init Failed!" << std::endl;
        }
        else if (exitCode == 0x00000001) {
            std::cout << "[Scorpio] DIAGNOSIS: Generic error - Xbox runtime missing!" << std::endl;
        }
        else if (exitCode == 0x00000000) {
            std::cout << "[Scorpio] Game exited cleanly!" << std::endl;
        }
        else {
            std::cout << "[Scorpio] Unknown exit code - needs investigation" << std::endl;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cout << "[Scorpio] Failed! Error: " << GetLastError() << std::endl;
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "================================" << std::endl;
    std::cout << "   Scorpio v0.0.7" << std::endl;
    std::cout << "   Xbox One Translation Layer" << std::endl;
    std::cout << "   github.com/Scorpio-Xbox" << std::endl;
    std::cout << "================================" << std::endl;

    if (argc < 2) {
        std::cout << "Usage: Scorpio.exe <path to xbox one file>" << std::endl;
        std::cout << "Supports: .xvc game packages and Xbox executables" << std::endl;
        return 1;
    }

    LoadXboxFile(argv[1]); 
    std::cout << "\n[Scorpio] Launch game? (y/n): ";
    char choice;
    std::cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        LaunchGame(argv[1]);
    }

    std::cout << "\n[Scorpio] Done!" << std::endl;
    system("pause");
    return 0;
}