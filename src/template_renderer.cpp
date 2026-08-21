#include "template_renderer.h"

#include "system_info.h"

#include <windows.h>

#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace deskinfo {
namespace {

constexpr wchar_t kSeparator[] = L"--------------------------------------------------";
constexpr std::uint32_t kDefaultMainColorRgb = 0x1E90FF;

const char kDefaultConfig[] =
    "Main_color: #1E90FF\n"
    "Machine Domain:   {machine_domain}\n"
    "{ip_address}\n"
    "{SEPARATOR}\n"
    "User Name:        {user_name}\n"
    "Logon Domain:     {logon_domain}\n"
    "{SEPARATOR}\n"
    "OS Version:       {os_version}\n"
    "System Type:      {system_type}\n"
    "{SEPARATOR}\n"
    "CPU Usage:        {cpu_usage}\n"
    "Memory:           {mem_used_gb} / {mem_total_gb}\n"
    "Disk Usage (C:\\): {disk_c_usage}\n"
    "{disk_info}\n";

struct TemplateConfig {
    std::vector<std::wstring> lines;
    std::uint32_t mainColorRgb{kDefaultMainColorRgb};
};

constexpr int hexDigitValue(wchar_t digit) {
    if (digit >= L'0' && digit <= L'9') return digit - L'0';
    if (digit >= L'a' && digit <= L'f') return digit - L'a' + 10;
    if (digit >= L'A' && digit <= L'F') return digit - L'A' + 10;
    return -1;
}

constexpr std::uint32_t parseHexColor(std::wstring_view value) {
    const bool shortFormat = value.size() == 4 || value.size() == 5;
    const bool longFormat = value.size() == 7 || value.size() == 9;
    if ((!shortFormat && !longFormat) || value.front() != L'#') return kDefaultMainColorRgb;

    for (size_t index = 1; index < value.size(); ++index) {
        if (hexDigitValue(value[index]) < 0) return kDefaultMainColorRgb;
    }

    if (shortFormat) {
        const auto red = static_cast<std::uint32_t>(hexDigitValue(value[1]) * 17);
        const auto green = static_cast<std::uint32_t>(hexDigitValue(value[2]) * 17);
        const auto blue = static_cast<std::uint32_t>(hexDigitValue(value[3]) * 17);
        return (red << 16) | (green << 8) | blue;
    }

    std::uint32_t color = 0;
    for (size_t index = 1; index <= 6; ++index) {
        color = (color << 4) | static_cast<std::uint32_t>(hexDigitValue(value[index]));
    }
    return color;
}

static_assert(parseHexColor(L"#19F") == 0x1199FF);
static_assert(parseHexColor(L"#19F8") == 0x1199FF);
static_assert(parseHexColor(L"#1E90FF") == 0x1E90FF);
static_assert(parseHexColor(L"#ff0000") == 0xFF0000);
static_assert(parseHexColor(L"#b3003bff") == 0xB3003B);
static_assert(parseHexColor(L"invalid") == kDefaultMainColorRgb);

std::wstring_view trim(std::wstring_view value) {
    while (!value.empty() && std::iswspace(value.front())) value.remove_prefix(1);
    while (!value.empty() && std::iswspace(value.back())) value.remove_suffix(1);
    return value;
}

bool equalsIgnoreCase(std::wstring_view left, std::wstring_view right) {
    if (left.size() != right.size()) return false;
    for (size_t index = 0; index < left.size(); ++index) {
        if (std::towlower(left[index]) != std::towlower(right[index])) return false;
    }
    return true;
}

std::filesystem::path executableDirectory() {
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
}

std::wstring fromUtf8(const std::string& text) {
    if (text.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (length <= 0) return {};
    std::wstring result(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), length);
    return result;
}

TemplateConfig loadTemplate() {
    const auto path = executableDirectory() / L"config.cfg";
    if (!std::filesystem::exists(path)) {
        std::ofstream created(path, std::ios::binary);
        created << kDefaultConfig;
    }
    std::ifstream input(path, std::ios::binary);
    std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (bytes.size() >= 3 && static_cast<unsigned char>(bytes[0]) == 0xEF &&
        static_cast<unsigned char>(bytes[1]) == 0xBB && static_cast<unsigned char>(bytes[2]) == 0xBF) {
        bytes.erase(0, 3);
    }
    TemplateConfig config;
    std::wistringstream stream(fromUtf8(bytes));
    for (std::wstring line; std::getline(stream, line);) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (line.empty()) continue;

        const size_t separator = line.find(L':');
        if (separator != std::wstring::npos &&
            equalsIgnoreCase(trim(std::wstring_view(line).substr(0, separator)), L"Main_color")) {
            config.mainColorRgb = parseHexColor(trim(std::wstring_view(line).substr(separator + 1)));
            continue;
        }
        config.lines.push_back(std::move(line));
    }
    return config;
}

std::wstring replaceTokens(std::wstring line, const TokenValues& values) {
    for (const auto& [key, value] : values) {
        const std::wstring token = L"{" + key + L"}";
        size_t position = 0;
        while ((position = line.find(token, position)) != std::wstring::npos) {
            line.replace(position, token.size(), value);
            position += value.size();
        }
    }
    return line;
}

} // namespace

DisplayContent buildDisplayContent() {
    auto snapshot = collectSystemInfo();
    snapshot.values.emplace(L"SEPARATOR", kSeparator);
    auto config = loadTemplate();

    std::vector<std::wstring> output{snapshot.machineName, kSeparator};
    for (auto line : config.lines) {
        line = replaceTokens(std::move(line), snapshot.values);
        std::wistringstream expanded(line);
        for (std::wstring part; std::getline(expanded, part);) {
            if (!part.empty() && part.back() == L'\r') part.pop_back();
            output.push_back(std::move(part));
        }
    }
    return {std::move(output), config.mainColorRgb};
}

} // namespace deskinfo
