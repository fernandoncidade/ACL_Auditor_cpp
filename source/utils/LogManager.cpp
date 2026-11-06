#include "LogManager.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <iostream>

namespace AclAuditor::Utils {

QString LogManager::s_logFilePath;
QMutex LogManager::s_logMutex;

void LogManager::configure_logging() {
    QMutexLocker locker(&s_logMutex);
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/logs");
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    s_logFilePath = dir.filePath(QStringLiteral("acl_auditor.log"));
}

QString LogManager::getLogFilePath() {
    QMutexLocker locker(&s_logMutex);
    return s_logFilePath;
}

void LogManager::writeLog(const QString& level, const QString& msg) {
    QMutexLocker locker(&s_logMutex);
    if (s_logFilePath.isEmpty()) {
        configure_logging();
    }

    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    QString line = QStringLiteral("[%1] [%2] %3\n").arg(timestamp, level, msg);

    QFile file(s_logFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << line;
        file.close();
    }

    std::wcout << line.toStdWString();
}

void LogManager::info(const QString& msg) {
    writeLog(QStringLiteral("INFO"), msg);
}

void LogManager::warning(const QString& msg) {
    writeLog(QStringLiteral("WARN"), msg);
}

void LogManager::error(const QString& msg) {
    writeLog(QStringLiteral("ERROR"), msg);
}

void LogManager::debug(const QString& msg) {
    writeLog(QStringLiteral("DEBUG"), msg);
}

} // namespace AclAuditor::Utils
