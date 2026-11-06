#include "ui/MainWindow.hpp"
#include "ui/ThemeManager.hpp"
#include "utils/ApplicationPathUtils.hpp"
#include "utils/LogManager.hpp"
#include "language/GerenciadorTraducao.hpp"

#include <QApplication>
#include <QIcon>
#include <QDir>

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shobjidl.h>
#endif

int main(int argc, char* argv[]) {
#if defined(Q_OS_WIN)
    SetCurrentProcessExplicitAppUserModelID(L"ACLAuditor.NTFS.Security.v1");
#endif

    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ACLAuditor"));
app.setApplicationVersion(QStringLiteral("2026.11.6.0"));
    app.setOrganizationName(QStringLiteral("ACLAuditor"));

    AclAuditor::Utils::LogManager::configure_logging();
    AclAuditor::Utils::LogManager::info(QStringLiteral("=== Iniciando ACL Auditor v2026.11.6.0 ==="));

    const QIcon appIcon = AclAuditor::Utils::ApplicationPathUtils::getAppIcon();
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

    // Initialize translation manager
    AclAuditor::Language::GerenciadorTraducao::instance().inicializar();

    AclAuditor::UI::MainWindow mainWindow;
    mainWindow.show();

    // Initialize dynamic theme manager
    AclAuditor::UI::ThemeManager::instance().initialize(&mainWindow);

    // If a folder path was provided in command line (e.g. from Windows Explorer context menu)
    if (argc > 1) {
        QString initialDir = QString::fromLocal8Bit(argv[1]);
        if (!initialDir.isEmpty() && QDir(initialDir).exists()) {
            mainWindow.openPathForAudit(initialDir);
        }
    }

    return app.exec();
}
