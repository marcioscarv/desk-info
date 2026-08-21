#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace deskinfo {

struct DisplayContent {
    std::vector<std::wstring> lines;
    std::uint32_t mainColorRgb{0x1E90FF};
};

DisplayContent buildDisplayContent();

} // namespace deskinfo
