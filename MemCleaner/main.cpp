#include <iostream>
#include "memory_cleaner.hpp"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage:\n";
        std::cout << "  cleaner --process <ProcessName.exe> <string_to_clean>\n";
        std::cout << "  cleaner --service <ServiceName> <string_to_clean>\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string target = argv[2];
    std::string stringToErase = argv[3];

    MemoryCleaner cleaner;

    if (!cleaner.EnableDebugPrivilege()) {
        std::cerr << "Failed to enable debug privileges.\n";
        return 1;
    }

    DWORD pid = 0;

    if (mode == "--process") {
        pid = cleaner.FindProcessIdByName(target);
    } else if (mode == "--service") {
        pid = cleaner.FindProcessIdByService(target);
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    if (pid == 0) {
        std::cerr << "Failed to find process ID for target: " << target << "\n";
        return 1;
    }

    if (!cleaner.CleanStringFromProcess(pid, stringToErase)) {
        std::cerr << "No matching string found or failed to clean.\n";
        return 1;
    }

    std::cout << "String cleaned from memory successfully.\n";
    return 0;
}
