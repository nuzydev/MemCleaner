#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

class MemoryCleaner {
public:
    bool EnableDebugPrivilege();
    DWORD FindProcessIdByName(const std::string& processName);
    DWORD FindProcessIdByService(const std::string& serviceName);
    bool CleanStringFromProcess(DWORD processId, const std::string& targetString);

private:
    std::vector<LPVOID> SearchMemoryRegions(DWORD processId, const std::string& searchString);
};
