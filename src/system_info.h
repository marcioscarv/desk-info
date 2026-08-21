#pragma once

#include <string>
#include <unordered_map>

namespace deskinfo {

using TokenValues = std::unordered_map<std::wstring, std::wstring>;

struct SystemSnapshot {
    std::wstring machineName;
    TokenValues values;
};

SystemSnapshot collectSystemInfo();

} // namespace deskinfo
