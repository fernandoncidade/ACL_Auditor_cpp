#pragma once

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QPalette>
#include <QWidget>

namespace AclAuditor::UI {

class ThemeManager : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

public:
    static ThemeManager& instance();

    void initialize(QWidget* mainWindow);
    bool isDark() const { return m_lastAppliedDark; }
    void refreshTheme();

    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

    QPalette createDarkPalette() const;
    QPalette createLightPalette() const;

signals:
    void themeChanged(bool isDark);

private:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() override;

    void updateTheme(bool forceNotify = false);

    QWidget* m_mainWindow{nullptr};
    class QTimer* m_pollTimer{nullptr};
    bool m_lastAppliedDark{false};
    bool m_initialized{false};
};

} // namespace AclAuditor::UI
