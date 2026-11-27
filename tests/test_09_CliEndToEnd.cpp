#include "TestSupport.hpp"
#include "engine/RiskScanner.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QtTest>

class TestCliEndToEnd : public QObject {
    Q_OBJECT

private:
    struct CliResult {
        int exitCode = -1;
        QByteArray stdOut;
        QByteArray stdErr;
    };

    static CliResult runCli(const QStringList& args) {
        CliResult res;
        QProcess proc;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        const QString qtBin = QDir::toNativeSeparators(QStringLiteral(ACL_AUDITOR_QT_BIN_DIR));
        const QString appBin = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
        env.insert(QStringLiteral("PATH"), appBin + QLatin1Char(';') + qtBin + QLatin1Char(';') + env.value(QStringLiteral("PATH")));
        proc.setProcessEnvironment(env);

        proc.start(QString::fromUtf8(ACL_AUDITOR_CLI_EXECUTABLE), args);
        if (!proc.waitForStarted(10000)) {
            return res;
        }
        if (!proc.waitForFinished(30000)) {
            proc.kill();
            proc.waitForFinished(3000);
            return res;
        }
        res.exitCode = proc.exitCode();
        res.stdOut = proc.readAllStandardOutput();
        res.stdErr = proc.readAllStandardError();
        return res;
    }

private slots:
    void testVersionHelpAndArgumentErrors() {
        const CliResult ver = runCli({QStringLiteral("--version")});
        QCOMPARE(ver.exitCode, 0);
        QVERIFY(QString::fromUtf8(ver.stdOut).contains(QStringLiteral("ACL Auditor CLI")));

        const CliResult help = runCli({QStringLiteral("--help")});
        QCOMPARE(help.exitCode, 0);
        const QString helpText = QString::fromUtf8(help.stdOut);
        QVERIFY(helpText.contains(QStringLiteral("scan")));
        QVERIFY(helpText.contains(QStringLiteral("access")));
        QVERIFY(helpText.contains(QStringLiteral("services")));
        QVERIFY(helpText.contains(QStringLiteral("shares")));

        const CliResult unknown = runCli({QStringLiteral("invalid-command")});
        QCOMPARE(unknown.exitCode, 3);
        QVERIFY(!unknown.stdErr.trimmed().isEmpty());

        const CliResult missingScanPath = runCli({QStringLiteral("scan")});
        QCOMPARE(missingScanPath.exitCode, 3);

        const CliResult missingAccessUser = runCli({QStringLiteral("access"), QStringLiteral("C:\\Windows")});
        QCOMPARE(missingAccessUser.exitCode, 3);
    }

    void testScanAndAccessJsonContracts() {
        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());
        QVERIFY(sandbox.createSubdir(QStringLiteral("cli_target")));
        QVERIFY(sandbox.createFile(QStringLiteral("cli_target/run.bat"), QByteArray("@echo off\n")));
        const std::wstring targetW = sandbox.filePathW(QStringLiteral("cli_target"));
        QVERIFY(sandbox.applySddl(targetW, L"D:P(A;OICI;FA;;;BA)(A;OICI;FRFW;;;WD)", true));

        const QString targetPath = sandbox.filePath(QStringLiteral("cli_target"));

        // scan --format json (includes tree)
        const CliResult scanFull = runCli({
            QStringLiteral("scan"),
            targetPath,
            QStringLiteral("--format"), QStringLiteral("json"),
            QStringLiteral("--depth"), QStringLiteral("2")
        });
        QVERIFY(scanFull.exitCode == 0 || scanFull.exitCode == 1);

        QJsonParseError errFull;
        const QJsonDocument docFull = QJsonDocument::fromJson(scanFull.stdOut, &errFull);
        QCOMPARE(errFull.error, QJsonParseError::NoError);
        QVERIFY(docFull.isObject());
        const QJsonObject objFull = docFull.object();
        QVERIFY(objFull.contains(QStringLiteral("targetPath")));
        QVERIFY(objFull.contains(QStringLiteral("overallRiskScore")));
        QVERIFY(objFull.contains(QStringLiteral("risks")));
        QVERIFY(objFull.contains(QStringLiteral("tree")));

        // scan --format json --risks-only (omits tree)
        const CliResult scanRisks = runCli({
            QStringLiteral("scan"),
            targetPath,
            QStringLiteral("--format"), QStringLiteral("json"),
            QStringLiteral("--risks-only")
        });
        QVERIFY(scanRisks.exitCode == 0 || scanRisks.exitCode == 1);
        const QJsonObject objRisks = QJsonDocument::fromJson(scanRisks.stdOut).object();
        QVERIFY(objRisks.contains(QStringLiteral("risks")));
        QVERIFY(!objRisks.contains(QStringLiteral("tree")));

        // access --user S-1-1-0 --format json (SID works across localized Windows locales)
        const CliResult accessRes = runCli({
            QStringLiteral("access"),
            targetPath,
            QStringLiteral("--user"), QStringLiteral("S-1-1-0"),
            QStringLiteral("--format"), QStringLiteral("json")
        });
        QCOMPARE(accessRes.exitCode, 0);
        const QJsonObject objAccess = QJsonDocument::fromJson(accessRes.stdOut).object();
        QCOMPARE(objAccess.value(QStringLiteral("resolvedSid")).toString(), QStringLiteral("S-1-1-0"));
        QVERIFY(objAccess.value(QStringLiteral("rights")).toObject().value(QStringLiteral("canRead")).toBool());
        QVERIFY(objAccess.value(QStringLiteral("rights")).toObject().value(QStringLiteral("canWrite")).toBool());
    }

    void testServicesAndSharesJsonOutput() {
        const CliResult svcRes = runCli({
            QStringLiteral("services"),
            QStringLiteral("--format"), QStringLiteral("json"),
            QStringLiteral("--risks-only")
        });
        QCOMPARE(svcRes.exitCode, 0);
        const QJsonDocument svcDoc = QJsonDocument::fromJson(svcRes.stdOut);
        QVERIFY(svcDoc.isObject());
        QVERIFY(svcDoc.object().value(QStringLiteral("services")).isArray());

        const CliResult shareRes = runCli({
            QStringLiteral("shares"),
            QStringLiteral("--format"), QStringLiteral("json")
        });
        QCOMPARE(shareRes.exitCode, 0);
        const QJsonDocument shareDoc = QJsonDocument::fromJson(shareRes.stdOut);
        QVERIFY(shareDoc.isObject());
        QVERIFY(shareDoc.object().value(QStringLiteral("shares")).isArray());
    }
};

QTEST_GUILESS_MAIN(TestCliEndToEnd)
#if !defined(__INTELLISENSE__)
#include "test_09_CliEndToEnd.moc"
#endif
