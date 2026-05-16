#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>

// Scorpio - Xbox One Translation Layer
// v0.0.2 - XVC Container Parser

// XVC Header Magic bytes - every Xbox One game file starts with these
#define XVC_MAGIC_0 0x43
#define XVC_MAGIC_1 0x4F
#define XVC_MAGIC_2 0x4E
#define XVC_MAGIC_3 0x54

struct XVCHeader {
    uint8_t  magic[4];        // "CONT" in ASCII
    uint32_t version;         // XVC version
    uint64_t contentId;       // Unique game ID
    uint8_t  contentType;     // Type of content
    uint64_t contentSize;     // Total size of content
    uint8_t  reserved[491];   // Reserved space
};

void PrintHex(const std::vector<uint8_t>& data, size_t count) {
    for (size_t i = 0; i < count && i < data.size(); i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

bool IsXVCFile(const std::vector<uint8_t>& header) {
    // Check for CONT magic
    if (header[0] == XVC_MAGIC_0 &&
        header[1] == XVC_MAGIC_1 &&
        header[2] == XVC_MAGIC_2 &&
        header[3] == XVC_MAGIC_3) {
        return true;
    }
    return false;
}

bool IsPEFile(const std::vector<uint8_t>& header) {
    return header[0] == 'M' && header[1] == 'Z';
}

bool LoadXboxFile(const std::string& filePath) {
    std::cout << "[Scorpio] Attempting to load: " << filePath << std::endl;

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "[ERROR] Cannot open file!" << std::endl;
        return false;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "[Scorpio] File size: " << fileSize << " bytes ("
        << fileSize / (1024 * 1024) << " MB)" << std::endl;

    // Read header
    std::vector<uint8_t> header(512);
    file.read(reinterpret_cast<char*>(header.data()), 512);

    std::cout << "[Scorpio] File header bytes:" << std::endl;
    PrintHex(header, 32);

    // Detect file type
    if (IsXVCFile(header)) {
        std::cout << "[Scorpio] XVC Container detected!" << std::endl;
        std::cout << "[Scorpio] This is an Xbox One game package!" << std::endl;

        // Parse XVC header
        XVCHeader* xvcHeader = reinterpret_cast<XVCHeader*>(header.data());
        std::cout << "[Scorpio] XVC Version: " << xvcHeader->version << std::endl;
        std::cout << "[Scorpio] Content Size: " << xvcHeader->contentSize << " bytes" << std::endl;
        std::cout << "[Scorpio] Content ID: " << xvcHeader->contentId << std::endl;

    }
    else if (IsPEFile(header)) {
        std::cout << "[Scorpio] Valid PE executable detected!" << std::endl;
        std::cout << "[Scorpio] This looks like an Xbox One executable!" << std::endl;
    }
    else {
        std::cout << "[Scorpio] Unknown file format" << std::endl;
        std::cout << "[Scorpio] First 4 bytes: ";
        printf("%02X %02X %02X %02X\n",
            header[0], header[1], header[2], header[3]);
    }

    file.close();
    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "================================" << std::endl;
    std::cout << "   Scorpio v0.0.2" << std::endl;
    std::cout << "   Xbox One Translation Layer" << std::endl;
    std::cout << "   github.com/Scorpio-Xbox" << std::endl;
    std::cout << "================================" << std::endl;

    if (argc < 2) {
        std::cout << "Usage: Scorpio.exe <path to xbox one file>" << std::endl;
        std::cout << "Supports: .xvc game packages and Xbox executables" << std::endl;
        return 1;
    }

    LoadXboxFile(argv[1]);

    std::cout << "\n[Scorpio] Done!" << std::endl;
    system("pause");
    return 0;
}