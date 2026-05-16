#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>

// Scorpio - Xbox One Translation Layer
// Our goal: Load and run Xbox One executables on PC

// First step: Read and validate an Xbox One executable file
// Xbox One games are essentially Windows PE files in a special container

void PrintHex(const std::vector<uint8_t>& data, size_t count) {
    for (size_t i = 0; i < count && i < data.size(); i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

bool LoadXboxExecutable(const std::string& filePath) {
    std::cout << "[Scorpio] Attempting to load: " << filePath << std::endl;

    // Open the file
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "[ERROR] Cannot open file!" << std::endl;
        return false;
    }

    // Read first 64 bytes - the file header
    std::vector<uint8_t> header(64);
    file.read(reinterpret_cast<char*>(header.data()), 64);

    std::cout << "[Scorpio] File header bytes:" << std::endl;
    PrintHex(header, 64);

    // Check for Windows PE signature (MZ) - Xbox One executables are PE files
    if (header[0] == 'M' && header[1] == 'Z') {
        std::cout << "[Scorpio] Valid PE executable detected!" << std::endl;
        std::cout << "[Scorpio] This looks like an Xbox One executable!" << std::endl;
    }
    else {
        std::cout << "[Scorpio] Unknown file format - might be a container file" << std::endl;
    }

    file.close();
    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "================================" << std::endl;
    std::cout << "   Scorpio v0.0.1" << std::endl;
    std::cout << "   Xbox One Translation Layer" << std::endl;
    std::cout << "   github.com/Scorpio-Xbox" << std::endl;
    std::cout << "================================" << std::endl;

    if (argc < 2) {
        std::cout << "Usage: Scorpio.exe <path to xbox one executable>" << std::endl;
        std::cout << "Drag and drop an Xbox One game file onto this exe to test!" << std::endl;
        return 1;
    }

    LoadXboxExecutable(argv[1]);

    std::cout << "\n[Scorpio] Done!" << std::endl;
    system("pause");
    return 0;
}