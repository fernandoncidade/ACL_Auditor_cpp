#pragma once

#include <QString>
#include <QIcon>

namespace AclAuditor::Utils {

class ApplicationPathUtils {
public:
    static QString getApplicationDirPath();
    static QString getAssetsDirPath();
    static QString getTranslationsDirPath();
    static QIcon getAppIcon();
};

} // namespace AclAuditor::Utils
