#pragma once

#include <QWidget>
#include <QString>

namespace AclAuditor::UI {

class Win32DwmHelper {
public:
    static bool isSystemDarkMode();
    static void applyTitleBarTheme(QWidget* widget, bool isDark);
    static void applyMicaAndDarkMode(QWidget* widget);
    static QString getProcessMemoryUsageString();
};

} // namespace AclAuditor::UI
