#include "ExplorerIntegration.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

namespace AclAuditor::Utils {

static const char* SHELL_KEY = "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\ACLAuditor";
static const char* BG_KEY    = "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\ACLAuditor";

bool ExplorerIntegration::isContextMenuRegistered() {
    QSettings settings(QString::fromLatin1(SHELL_KEY), QSettings::NativeFormat);
    return settings.contains(QStringLiteral("command/."));
}

bool ExplorerIntegration::registerContextMenu() {
    QString exePath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString commandStr = QStringLiteral("\"%1\" \"%2\"").arg(exePath, QStringLiteral("%1"));
    QString commandBgStr = QStringLiteral("\"%1\" \"%2\"").arg(exePath, QStringLiteral("%V"));
    QString menuText = QStringLiteral("Auditar Permissões NTFS com ACL Auditor");

    {
        QSettings settings(QString::fromLatin1(SHELL_KEY), QSettings::NativeFormat);
        settings.setValue(QStringLiteral("."), menuText);
        settings.setValue(QStringLiteral("Icon"), QStringLiteral("\"%1\",0").arg(exePath));
        settings.setValue(QStringLiteral("command/."), commandStr);
    }

    {
        QSettings bgSettings(QString::fromLatin1(BG_KEY), QSettings::NativeFormat);
        bgSettings.setValue(QStringLiteral("."), menuText);
        bgSettings.setValue(QStringLiteral("Icon"), QStringLiteral("\"%1\",0").arg(exePath));
        bgSettings.setValue(QStringLiteral("command/."), commandBgStr);
    }

    return true;
}

bool ExplorerIntegration::unregisterContextMenu() {
    {
        QSettings settings(QString::fromLatin1(SHELL_KEY), QSettings::NativeFormat);
        settings.clear();
    }
    {
        QSettings bgSettings(QString::fromLatin1(BG_KEY), QSettings::NativeFormat);
        bgSettings.clear();
    }
    return true;
}

} // namespace AclAuditor::Utils
