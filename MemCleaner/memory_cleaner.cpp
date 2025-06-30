#include "memory_cleaner.hpp"
#include <iostream>
#include <algorithm>
#include <winsvc.h>

bool MemoryCleaner::EnableDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return false;

    if (!LookupPrivilegeValueA(nullptr, SE_DEBUG_NAME, &luid)) {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL adjusted = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
    CloseHandle(hToken);
    return adjusted && GetLastError() == ERROR_SUCCESS;
}

DWORD MemoryCleaner::FindProcessIdByName(const std::string& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 entry = { sizeof(entry) };
    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        return 0;
    }

    DWORD pid = 0;
    do {
        if (_stricmp(entry.szExeFile, processName.c_str()) == 0) {
            pid = entry.th32ProcessID;
            break;
        }
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return pid;
}

DWORD MemoryCleaner::FindProcessIdByService(const std::string& serviceName) {
    SC_HANDLE scm = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm)
        return 0;

    SC_HANDLE service = OpenServiceA(scm, serviceName.c_str(), SERVICE_QUERY_STATUS);
    if (!service) {
        CloseServiceHandle(scm);
        return 0;
    }

    SERVICE_STATUS_PROCESS status;
    DWORD bytesNeeded;
    BOOL ok = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                                   reinterpret_cast<LPBYTE>(&status), sizeof(status), &bytesNeeded);

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    return ok ? status.dwProcessId : 0;
}

std::vector<LPVOID> MemoryCleaner::SearchMemoryRegions(DWORD processId, const std::string& searchString) {
    std::vector<LPVOID> matches;
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!hProcess) return matches;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    LPBYTE currentAddr = (LPBYTE)sysInfo.lpMinimumApplicationAddress;
    LPBYTE maxAddr = (LPBYTE)sysInfo.lpMaximumApplicationAddress;

    std::string lowerSearch = searchString;
    std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

    while (currentAddr < maxAddr) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(hProcess, currentAddr, &mbi, sizeof(mbi)) == 0)
            break;

        if (mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_NOACCESS) && !(mbi.Protect & PAGE_GUARD)) {
            std::vector<char> buffer(mbi.RegionSize);
            SIZE_T bytesRead;

            if (ReadProcessMemory(hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
                std::string region(buffer.begin(), buffer.end());
                std::transform(region.begin(), region.end(), region.begin(), ::tolower);

                size_t pos = region.find(lowerSearch);
                while (pos != std::string::npos) {
                    matches.push_back((LPBYTE)mbi.BaseAddress + pos);
                    pos = region.find(lowerSearch, pos + 1);
                }
            }
        }

        currentAddr += mbi.RegionSize;
    }

    CloseHandle(hProcess);
    return matches;
}

bool MemoryCleaner::CleanStringFromProcess(DWORD processId, const std::string& targetString) {
    HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    if (!hProcess)
        return false;

    auto addresses = SearchMemoryRegions(processId, targetString);
    if (addresses.empty()) {
        CloseHandle(hProcess);
        return false;
    }

    std::string nulls(targetString.size(), '\0');
    bool success = true;

    for (const auto& addr : addresses) {
        if (!WriteProcessMemory(hProcess, addr, nulls.c_str(), targetString.size(), nullptr)) {
            std::cerr << "Failed to write at: 0x" << std::hex << (uintptr_t)addr << "\n";
            success = false;
        }
    }

    CloseHandle(hProcess);
    return success;
}
