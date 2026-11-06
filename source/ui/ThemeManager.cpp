#include "ThemeManager.hpp"
#include "Win32DwmHelper.hpp"

#include <QApplication>
#include <QTimer>
#include <QStyleFactory>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace AclAuditor::UI {

ThemeManager& ThemeManager::instance() {
    static ThemeManager inst;
    return inst;
}

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
}

ThemeManager::~ThemeManager() {
    if (m_initialized) {
        qApp->removeNativeEventFilter(this);
    }
}

void ThemeManager::initialize(QWidget* mainWindow) {
    m_mainWindow = mainWindow;
    if (!m_initialized) {
        qApp->installNativeEventFilter(this);

        m_pollTimer = new QTimer(this);
        connect(m_pollTimer, &QTimer::timeout, this, [this]() {
            bool currentDark = Win32DwmHelper::isSystemDarkMode();
            if (currentDark != m_lastAppliedDark) {
                updateTheme(true);
            }
        });
        m_pollTimer->start(1000); // Check every 1 second for OS theme change
        m_initialized = true;
    }

    updateTheme(true);
}

void ThemeManager::refreshTheme() {
    updateTheme(true);
}

void ThemeManager::updateTheme(bool forceNotify) {
    bool dark = Win32DwmHelper::isSystemDarkMode();
    if (dark != m_lastAppliedDark || forceNotify) {
        m_lastAppliedDark = dark;

        QPalette pal = dark ? createDarkPalette() : createLightPalette();
        qApp->setPalette(pal);

        if (m_mainWindow) {
            Win32DwmHelper::applyTitleBarTheme(m_mainWindow, dark);
            m_mainWindow->setPalette(pal);
            m_mainWindow->update();
        }

        emit themeChanged(dark);
    }
}

bool ThemeManager::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
    Q_UNUSED(eventType);
    Q_UNUSED(result);
#if defined(Q_OS_WIN)
    MSG* msg = static_cast<MSG*>(message);
    if (msg && (msg->message == WM_SETTINGCHANGE || msg->message == 0x031A /*WM_THEMECHANGED*/)) {
        updateTheme(false);
    }
#endif
    return false;
}

QPalette ThemeManager::createDarkPalette() const {
    QPalette pal;
    // Premium dark palette (Windows 11 dark slate & graphite)
    QColor windowBg(32, 33, 36);
    QColor baseBg(24, 25, 27);
    QColor text(240, 242, 245);
    QColor secondaryText(170, 175, 185);
    QColor accent(0, 120, 215);
    QColor buttonBg(45, 48, 52);
    QColor border(60, 64, 70);

    pal.setColor(QPalette::Window, windowBg);
    pal.setColor(QPalette::WindowText, text);
    pal.setColor(QPalette::Base, baseBg);
    pal.setColor(QPalette::AlternateBase, QColor(28, 30, 33));
    pal.setColor(QPalette::ToolTipBase, QColor(40, 42, 46));
    pal.setColor(QPalette::ToolTipText, text);
    pal.setColor(QPalette::Text, text);
    pal.setColor(QPalette::Button, buttonBg);
    pal.setColor(QPalette::ButtonText, text);
    pal.setColor(QPalette::BrightText, Qt::red);
    pal.setColor(QPalette::Link, QColor(80, 160, 255));
    pal.setColor(QPalette::Highlight, accent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::PlaceholderText, secondaryText);

    return pal;
}

QPalette ThemeManager::createLightPalette() const {
    QPalette pal;
    // Clean, modern light palette
    QColor windowBg(248, 249, 250);
    QColor baseBg(255, 255, 255);
    QColor text(33, 37, 41);
    QColor secondaryText(108, 117, 125);
    QColor accent(0, 102, 204);
    QColor buttonBg(235, 238, 242);

    pal.setColor(QPalette::Window, windowBg);
    pal.setColor(QPalette::WindowText, text);
    pal.setColor(QPalette::Base, baseBg);
    pal.setColor(QPalette::AlternateBase, QColor(241, 243, 245));
    pal.setColor(QPalette::ToolTipBase, Qt::white);
    pal.setColor(QPalette::ToolTipText, text);
    pal.setColor(QPalette::Text, text);
    pal.setColor(QPalette::Button, buttonBg);
    pal.setColor(QPalette::ButtonText, text);
    pal.setColor(QPalette::BrightText, Qt::red);
    pal.setColor(QPalette::Link, accent);
    pal.setColor(QPalette::Highlight, accent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::PlaceholderText, secondaryText);

    return pal;
}

} // namespace AclAuditor::UI
