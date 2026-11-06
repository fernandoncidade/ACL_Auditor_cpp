#include "Win32DwmHelper.hpp"

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwmapi.h>
#include <psapi.h>

#ifdef _MSC_VER
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "psapi.lib")
#endif
#endif

namespace AclAuditor::UI {

bool Win32DwmHelper::isSystemDarkMode() {
#if defined(Q_OS_WIN)
    HKEY hKey;
    LONG res = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0,
        KEY_READ,
        &hKey
    );
    if (res == ERROR_SUCCESS) {
        DWORD appsUseLightTheme = 1;
        DWORD cbData = sizeof(DWORD);
        res = RegQueryValueExW(
            hKey,
            L"AppsUseLightTheme",
            nullptr,
            nullptr,
            reinterpret_cast<LPBYTE>(&appsUseLightTheme),
            &cbData
        );
        RegCloseKey(hKey);
        if (res == ERROR_SUCCESS) {
            return (appsUseLightTheme == 0);
        }
    }
#endif
    return false;
}

void Win32DwmHelper::applyTitleBarTheme(QWidget* widget, bool isDark) {
#if defined(Q_OS_WIN)
    if (!widget) return;
    HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    if (!hwnd) return;

    BOOL value = isDark ? TRUE : FALSE;
    // DWMWA_USE_IMMERSIVE_DARK_MODE = 20
    DwmSetWindowAttribute(hwnd, 20, &value, sizeof(value));

    // Try Mica backdrop: DWMWA_SYSTEMBACKDROP_TYPE = 38 (2 = Mica)
    int backdrop = 2;
    DwmSetWindowAttribute(hwnd, 38, &backdrop, sizeof(backdrop));
#else
    Q_UNUSED(widget);
    Q_UNUSED(isDark);
#endif
}

void Win32DwmHelper::applyMicaAndDarkMode(QWidget* widget) {
    applyTitleBarTheme(widget, isSystemDarkMode());
}

QString Win32DwmHelper::getProcessMemoryUsageString() {
#if defined(Q_OS_WIN)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        double mb = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        return QStringLiteral("%1 MB").arg(mb, 0, 'f', 1);
    }
#endif
    return QStringLiteral("N/A");
}

} // namespace AclAuditor::UI
