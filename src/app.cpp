#include "app.h"

#include "template_renderer.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace deskinfo {
namespace {

constexpr wchar_t kWindowClass[] = L"DeskInfoNativeWindow";
constexpr wchar_t kMutexName[] = L"DeskInfo_SingleInstance_Mutex";
constexpr UINT_PTR kTimerId = 1;
constexpr UINT kUpdateIntervalMs = 1000;
constexpr int kGapX = 30;
constexpr int kGapY = 30;
constexpr int kPadding = 15;
constexpr COLORREF kTransparentColor = RGB(1, 0, 1);
constexpr COLORREF kTextColor = RGB(255, 255, 255);
constexpr COLORREF kOutlineColor = RGB(0, 0, 0);

struct AppState {
    HFONT regularFont{};
    HFONT titleFont{};
    std::vector<std::wstring> lines;
    COLORREF mainColor{RGB(30, 144, 255)};
    RECT windowRect{};
};

COLORREF toColorRef(std::uint32_t rgb) {
    return RGB(
        static_cast<BYTE>((rgb >> 16) & 0xFF),
        static_cast<BYTE>((rgb >> 8) & 0xFF),
        static_cast<BYTE>(rgb & 0xFF));
}

bool isSeparator(const std::wstring& line) {
    return !line.empty() && std::all_of(line.begin(), line.end(), [](wchar_t ch) {
        return ch == L'-' || ch == L' ';
    });
}

void refresh(HWND window, AppState& state) {
    auto content = buildDisplayContent();
    state.lines = std::move(content.lines);
    state.mainColor = toColorRef(content.mainColorRgb);
    HDC dc = GetDC(window);
    int width = 0;
    int height = kPadding * 2;
    for (size_t index = 0; index < state.lines.size(); ++index) {
        SelectObject(dc, index == 0 ? state.titleFont : state.regularFont);
        SIZE size{};
        GetTextExtentPoint32W(
            dc,
            state.lines[index].c_str(),
            static_cast<int>(state.lines[index].size()),
            &size);
        width = std::max(width, static_cast<int>(size.cx));
        height += size.cy + 1;
    }
    ReleaseDC(window, dc);
    width += kPadding * 2 + 4;

    MONITORINFO monitor{sizeof(monitor)};
    GetMonitorInfoW(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor);
    const int x = monitor.rcWork.right - width - kGapX;
    const int y = monitor.rcWork.top + kGapY;
    const RECT nextRect{x, y, x + width, y + height};
    if (!EqualRect(&state.windowRect, &nextRect)) {
        SetWindowPos(
            window,
            HWND_BOTTOM,
            x,
            y,
            width,
            height,
            SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER);
        state.windowRect = nextRect;
    }
    InvalidateRect(window, nullptr, FALSE);
}

void paint(HWND window, const AppState& state) {
    PAINTSTRUCT paintInfo{};
    HDC dc = BeginPaint(window, &paintInfo);
    RECT area{};
    GetClientRect(window, &area);

    const int width = area.right - area.left;
    const int height = area.bottom - area.top;
    HDC bufferDc = CreateCompatibleDC(dc);
    HBITMAP bufferBitmap = CreateCompatibleBitmap(dc, width, height);
    const HGDIOBJ previousBitmap = SelectObject(bufferDc, bufferBitmap);

    HBRUSH background = CreateSolidBrush(kTransparentColor);
    FillRect(bufferDc, &area, background);
    DeleteObject(background);
    SetBkMode(bufferDc, TRANSPARENT);

    int y = kPadding;
    for (size_t index = 0; index < state.lines.size(); ++index) {
        const HFONT font = index == 0 ? state.titleFont : state.regularFont;
        SelectObject(bufferDc, font);
        TEXTMETRICW metrics{};
        GetTextMetricsW(bufferDc, &metrics);
        const auto& line = state.lines[index];
        SetTextColor(bufferDc, kOutlineColor);
        for (const POINT offset : {POINT{-1, 0}, POINT{1, 0}, POINT{0, -1}, POINT{0, 1}}) {
            TextOutW(
                bufferDc,
                kPadding + offset.x,
                y + offset.y,
                line.c_str(),
                static_cast<int>(line.size()));
        }
        SetTextColor(bufferDc, (index == 0 || isSeparator(line)) ? state.mainColor : kTextColor);
        TextOutW(bufferDc, kPadding, y, line.c_str(), static_cast<int>(line.size()));
        y += metrics.tmHeight + 1;
    }

    BitBlt(dc, 0, 0, width, height, bufferDc, 0, 0, SRCCOPY);
    SelectObject(bufferDc, previousBitmap);
    DeleteObject(bufferBitmap);
    DeleteDC(bufferDc);
    EndPaint(window, &paintInfo);
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<AppState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    switch (message) {
        case WM_NCCREATE: {
            const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
            return TRUE;
        }
        case WM_CREATE:
            SetTimer(window, kTimerId, kUpdateIntervalMs, nullptr);
            refresh(window, *reinterpret_cast<AppState*>(GetWindowLongPtrW(window, GWLP_USERDATA)));
            return 0;
        case WM_TIMER:
            if (wParam == kTimerId && state) refresh(window, *state);
            return 0;
        case WM_PAINT:
            if (state) paint(window, *state);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_DESTROY:
            KillTimer(window, kTimerId);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(window, message, wParam, lParam);
    }
}

} // namespace

int runApplication(HINSTANCE instance) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HANDLE mutex = CreateMutexW(nullptr, FALSE, kMutexName);
    if (!mutex || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (mutex) CloseHandle(mutex);
        return 0;
    }

    AppState state;
    state.regularFont = CreateFontW(
        -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Consolas");
    state.titleFont = CreateFontW(
        -20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Consolas");

    WNDCLASSEXW windowClass{sizeof(windowClass)};
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(1));
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = CreateSolidBrush(kTransparentColor);
    windowClass.lpszClassName = kWindowClass;
    RegisterClassExW(&windowClass);

    HWND window = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kWindowClass, L"DeskInfo", WS_POPUP, 0, 0, 1, 1,
        nullptr, nullptr, instance, &state);
    if (!window) {
        DeleteObject(state.regularFont);
        DeleteObject(state.titleFont);
        CloseHandle(mutex);
        return 1;
    }
    SetLayeredWindowAttributes(window, kTransparentColor, 255, LWA_COLORKEY);
    ShowWindow(window, SW_SHOWNOACTIVATE);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    DeleteObject(state.regularFont);
    DeleteObject(state.titleFont);
    CloseHandle(mutex);
    return static_cast<int>(message.wParam);
}

} // namespace deskinfo
