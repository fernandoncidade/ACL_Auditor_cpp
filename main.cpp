// ACL Auditor - Analisador Visual de Permissões e Conformidade NTFS para Windows
#include "source/ui/MainWindow.hpp"
#include "source/ui/ThemeManager.hpp"
#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/LogManager.hpp"
#include "source/language/GerenciadorTraducao.hpp"

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

    AclAuditor::Language::GerenciadorTraducao::instance().inicializar();

    AclAuditor::UI::MainWindow mainWindow;
    mainWindow.show();

    AclAuditor::UI::ThemeManager::instance().initialize(&mainWindow);

    if (argc > 1) {
        QString initialDir = QString::fromLocal8Bit(argv[1]);
        if (!initialDir.isEmpty() && QDir(initialDir).exists()) {
            mainWindow.openPathForAudit(initialDir);
        }
    }

    return app.exec();
}
