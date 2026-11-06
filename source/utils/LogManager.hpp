#pragma once

#include <QString>
#include <QMutex>

namespace AclAuditor::Utils {

class LogManager {
public:
    static void configure_logging();
    static void info(const QString& msg);
    static void warning(const QString& msg);
    static void error(const QString& msg);
    static void debug(const QString& msg);

    static QString getLogFilePath();

private:
    static void writeLog(const QString& level, const QString& msg);
    static QString s_logFilePath;
    static QMutex s_logMutex;
};

} // namespace AclAuditor::Utils
