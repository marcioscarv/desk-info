#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <lmcons.h>

#include "system_info.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cwchar>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace deskinfo {
namespace {

std::wstring environmentValue(const wchar_t* name, const wchar_t* fallback = L"N/A") {
    const DWORD needed = GetEnvironmentVariableW(name, nullptr, 0);
    if (needed == 0) return fallback;
    std::wstring value(needed, L'\0');
    GetEnvironmentVariableW(name, value.data(), needed);
    value.resize(wcslen(value.c_str()));
    return value;
}

std::wstring computerName() {
    std::array<wchar_t, MAX_COMPUTERNAME_LENGTH + 1> value{};
    DWORD size = static_cast<DWORD>(value.size());
    return GetComputerNameW(value.data(), &size) ? std::wstring(value.data(), size) : L"N/A";
}

std::wstring userName() {
    std::array<wchar_t, UNLEN + 1> value{};
    DWORD size = static_cast<DWORD>(value.size());
    return GetUserNameW(value.data(), &size) ? std::wstring(value.data()) : L"N/A";
}

std::wstring architectureName() {
    SYSTEM_INFO info{};
    GetNativeSystemInfo(&info);
    switch (info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: return L"AMD64";
        case PROCESSOR_ARCHITECTURE_ARM64: return L"ARM64";
        case PROCESSOR_ARCHITECTURE_INTEL: return L"x86";
        default: return L"Unknown";
    }
}

std::wstring windowsVersion() {
    using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    const auto module = GetModuleHandleW(L"ntdll.dll");
    const auto rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(GetProcAddress(module, "RtlGetVersion"));
    OSVERSIONINFOEXW version{};
    version.dwOSVersionInfoSize = sizeof(version);
    if (!rtlGetVersion || rtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&version)) != 0) {
        return L"Windows";
    }

    std::wstring productName;
    if (version.dwMajorVersion == 10) {
        if (version.wProductType == VER_NT_WORKSTATION) {
            productName = version.dwBuildNumber >= 22000 ? L"Windows 11" : L"Windows 10";
        } else if (version.dwBuildNumber >= 26100) {
            productName = L"Windows Server 2025";
        } else if (version.dwBuildNumber >= 20348) {
            productName = L"Windows Server 2022";
        } else if (version.dwBuildNumber >= 17763) {
            productName = L"Windows Server 2019";
        } else if (version.dwBuildNumber >= 14393) {
            productName = L"Windows Server 2016";
        } else {
            productName = L"Windows Server";
        }
    } else if (version.dwMajorVersion == 6 && version.dwMinorVersion == 3) {
        productName = version.wProductType == VER_NT_WORKSTATION ? L"Windows 8.1" : L"Windows Server 2012 R2";
    } else if (version.dwMajorVersion == 6 && version.dwMinorVersion == 2) {
        productName = version.wProductType == VER_NT_WORKSTATION ? L"Windows 8" : L"Windows Server 2012";
    } else if (version.dwMajorVersion == 6 && version.dwMinorVersion == 1) {
        productName = version.wProductType == VER_NT_WORKSTATION ? L"Windows 7" : L"Windows Server 2008 R2";
    } else {
        productName = L"Windows " + std::to_wstring(version.dwMajorVersion) + L'.' +
                      std::to_wstring(version.dwMinorVersion);
    }

    std::wostringstream output;
    output << productName << L" (Build " << version.dwBuildNumber << L')';
    return output.str();
}

uint64_t fileTimeValue(const FILETIME& value) {
    return (static_cast<uint64_t>(value.dwHighDateTime) << 32) | value.dwLowDateTime;
}

double cpuUsage() {
    static uint64_t previousIdle = 0;
    static uint64_t previousTotal = 0;
    FILETIME idle{}, kernel{}, user{};
    if (!GetSystemTimes(&idle, &kernel, &user)) return 0.0;
    const uint64_t idleNow = fileTimeValue(idle);
    const uint64_t totalNow = fileTimeValue(kernel) + fileTimeValue(user);
    double usage = 0.0;
    if (previousTotal != 0 && totalNow > previousTotal) {
        const auto totalDelta = totalNow - previousTotal;
        const auto idleDelta = idleNow - previousIdle;
        usage = 100.0 * static_cast<double>(totalDelta - std::min(idleDelta, totalDelta)) /
                static_cast<double>(totalDelta);
    }
    previousIdle = idleNow;
    previousTotal = totalNow;
    return usage;
}

std::wstring fixed(double value, int precision = 1) {
    std::wostringstream output;
    output << std::fixed << std::setprecision(precision) << value;
    return output.str();
}

std::wstring networkAdapters() {
    ULONG size = 15000;
    std::vector<unsigned char> storage(size);
    auto* addresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(storage.data());
    ULONG result = GetAdaptersAddresses(
        AF_INET,
        GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
        nullptr,
        addresses,
        &size);
    if (result == ERROR_BUFFER_OVERFLOW) {
        storage.resize(size);
        addresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(storage.data());
        result = GetAdaptersAddresses(
            AF_INET,
            GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
            nullptr,
            addresses,
            &size);
    }

    std::wstring output = L"Network adapter:";
    bool found = false;
    if (result == NO_ERROR) {
        for (auto* adapter = addresses; adapter; adapter = adapter->Next) {
            if (adapter->OperStatus != IfOperStatusUp || adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
            for (auto* entry = adapter->FirstUnicastAddress; entry; entry = entry->Next) {
                if (!entry->Address.lpSockaddr || entry->Address.lpSockaddr->sa_family != AF_INET) continue;
                wchar_t ip[INET_ADDRSTRLEN]{};
                const auto* ipv4 = reinterpret_cast<const sockaddr_in*>(entry->Address.lpSockaddr);
                if (InetNtopW(AF_INET, const_cast<IN_ADDR*>(&ipv4->sin_addr), ip, INET_ADDRSTRLEN)) {
                    output += L"\n\u2022  ";
                    output += adapter->FriendlyName ? adapter->FriendlyName : L"Adapter";
                    output += L": ";
                    output += ip;
                    found = true;
                    break;
                }
            }
        }
    }
    return found ? output : output + L"\n  Local: N/A";
}

struct DiskValues {
    std::wstring details;
    std::wstring cUsage = L"N/A";
};

DiskValues diskValues() {
    DiskValues result;
    const DWORD mask = GetLogicalDrives();
    for (int index = 0; index < 26; ++index) {
        if ((mask & (1u << index)) == 0) continue;
        const std::wstring root{static_cast<wchar_t>(L'A' + index), L':', L'\\'};
        const UINT type = GetDriveTypeW(root.c_str());
        if (type != DRIVE_FIXED && type != DRIVE_REMOVABLE) continue;
        ULARGE_INTEGER freeBytes{}, totalBytes{};
        if (!GetDiskFreeSpaceExW(root.c_str(), &freeBytes, &totalBytes, nullptr) || totalBytes.QuadPart == 0) continue;
        if (!result.details.empty()) result.details += L'\n';
        result.details += L"Free Space (" + root + L"): " +
                          fixed(freeBytes.QuadPart / 1073741824.0) + L"G of " +
                          fixed(totalBytes.QuadPart / 1073741824.0) + L'G';
        if (index == 2) {
            const double used = 100.0 * (1.0 - static_cast<double>(freeBytes.QuadPart) /
                                               static_cast<double>(totalBytes.QuadPart));
            result.cUsage = fixed(used) + L'%';
        }
    }
    if (result.details.empty()) result.details = L"Free Space (C:\\): N/A of N/A";
    return result;
}

} // namespace

SystemSnapshot collectSystemInfo() {
    const std::wstring machine = computerName();
    std::wstring domain = environmentValue(L"USERDOMAIN");
    if (_wcsicmp(domain.c_str(), machine.c_str()) == 0) domain = L"WORKGROUP";
    const std::wstring user = userName();
    MEMORYSTATUSEX memory{sizeof(memory)};
    GlobalMemoryStatusEx(&memory);
    const auto disks = diskValues();

    return {
        machine,
        {
            {L"nodename", machine},
            {L"machine_domain", domain},
            {L"ip_address", networkAdapters()},
            {L"user_name", user},
            {L"logon_domain", user + L"@" + domain},
            {L"os_version", windowsVersion()},
            {L"system_type", architectureName()},
            {L"cpu_usage", fixed(cpuUsage()) + L'%'},
            {L"mem_used_gb", fixed((memory.ullTotalPhys - memory.ullAvailPhys) / 1073741824.0) + L'G'},
            {L"mem_total_gb", fixed(memory.ullTotalPhys / 1073741824.0) + L'G'},
            {L"disk_c_usage", disks.cUsage},
            {L"disk_info", disks.details},
        },
    };
}

} // namespace deskinfo
