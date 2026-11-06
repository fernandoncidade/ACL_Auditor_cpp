#include "ApplicationPathUtils.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

namespace AclAuditor::Utils {

QString ApplicationPathUtils::getApplicationDirPath() {
    return QCoreApplication::applicationDirPath();
}

QString ApplicationPathUtils::getAssetsDirPath() {
    QString appDir = getApplicationDirPath();

    // Check sibling assets/
    QString p1 = appDir + QStringLiteral("/assets");
    if (QDir(p1).exists()) return p1;

    // Check parent source/assets (development build)
    QString p2 = appDir + QStringLiteral("/../../source/assets");
    if (QDir(p2).exists()) return QDir(p2).canonicalPath();

    return p1;
}

QString ApplicationPathUtils::getTranslationsDirPath() {
    QString appDir = getApplicationDirPath();
    QString p1 = appDir + QStringLiteral("/translations");
    if (QDir(p1).exists()) return p1;

    QString p2 = appDir + QStringLiteral("/../../source/language/translations");
    if (QDir(p2).exists()) return QDir(p2).canonicalPath();

    return p1;
}

QIcon ApplicationPathUtils::getAppIcon() {
    const QStringList iconFiles = {
        QStringLiteral("acl_auditor.ico"),
        QStringLiteral("acl_auditor.png"),
        QStringLiteral("acl_auditor_300-300.png")
    };

    const QStringList candidateDirs = {
        getAssetsDirPath() + QStringLiteral("/icons"),
        getApplicationDirPath() + QStringLiteral("/assets/icons"),
        getApplicationDirPath() + QStringLiteral("/icons"),
        getApplicationDirPath()
    };

    for (const auto& dirPath : candidateDirs) {
        for (const auto& file : iconFiles) {
            QString fullPath = dirPath + QStringLiteral("/") + file;
            if (QFile::exists(fullPath)) {
                QIcon icon(fullPath);
                if (!icon.isNull()) {
                    return icon;
                }
            }
        }
    }

    return QIcon();
}

} // namespace AclAuditor::Utils
