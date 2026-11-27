#include "utils/ApplicationPathUtils.hpp"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QPrinter>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QtTest>

class TestFrameworkAndPlugins : public QObject {
    Q_OBJECT

private slots:
    void testQtRuntimeEnvironmentAndPlatform() {
        QVERIFY(QCoreApplication::instance() != nullptr);
        QVERIFY(!QStyleFactory::keys().isEmpty());
        QCOMPARE(QGuiApplication::platformName(), QStringLiteral("offscreen"));

        const QList<QByteArray> formats = QImageReader::supportedImageFormats();
        QVERIFY(formats.contains("png"));
        QVERIFY(formats.contains("ico"));
    }

    void testQtPluginDirectoriesPresent() {
        const QDir pluginsDir(QStringLiteral(ACL_AUDITOR_QT_PLUGINS_DIR));
        QVERIFY2(pluginsDir.exists(), qPrintable(pluginsDir.absolutePath()));

        QVERIFY(QFileInfo::exists(pluginsDir.filePath(QStringLiteral("sqldrivers/qsqlite.dll"))));
        QVERIFY(QFileInfo::exists(pluginsDir.filePath(QStringLiteral("platforms/qwindows.dll"))));
        QVERIFY(QFileInfo::exists(pluginsDir.filePath(QStringLiteral("platforms/qoffscreen.dll"))));
        QVERIFY(QFileInfo::exists(pluginsDir.filePath(QStringLiteral("styles/qmodernwindowsstyle.dll"))));
        QVERIFY(QFileInfo::exists(pluginsDir.filePath(QStringLiteral("imageformats/qico.dll"))));
    }

    void testSqliteDriverExecution() {
        QVERIFY(QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE")));

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString dbPath = tempDir.filePath(QStringLiteral("plugin_probe.db"));
        const QString connName = QStringLiteral("test_01_sqlite_probe");

        {
            QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
            db.setDatabaseName(dbPath);
            QVERIFY2(db.open(), qPrintable(db.lastError().text()));

            QSqlQuery q(db);
            QVERIFY2(q.exec(QStringLiteral("CREATE TABLE kv (k TEXT PRIMARY KEY, v INTEGER NOT NULL)")),
                     qPrintable(q.lastError().text()));
            QVERIFY(q.prepare(QStringLiteral("INSERT INTO kv (k, v) VALUES (?, ?)")));
            q.addBindValue(QStringLiteral("probe"));
            q.addBindValue(42);
            QVERIFY2(q.exec(), qPrintable(q.lastError().text()));

            QVERIFY(q.exec(QStringLiteral("SELECT v FROM kv WHERE k = 'probe'")));
            QVERIFY(q.next());
            QCOMPARE(q.value(0).toInt(), 42);
            db.close();
        }

        QSqlDatabase::removeDatabase(connName);
        QVERIFY(QFileInfo::exists(dbPath));
    }

    void testPrintSupportPdfEngine() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString pdfPath = tempDir.filePath(QStringLiteral("printer_probe.pdf"));

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(pdfPath);

        QVERIFY(printer.isValid());
        QCOMPARE(printer.outputFormat(), QPrinter::PdfFormat);
    }

    void testRuntimeAssetsAndTranslationsDeployedLocally() {
        const QString assetsDir = AclAuditor::Utils::ApplicationPathUtils::getAssetsDirPath();
        const QString transDir = AclAuditor::Utils::ApplicationPathUtils::getTranslationsDirPath();

        QVERIFY(QDir(assetsDir).exists());
        QVERIFY(QFileInfo::exists(assetsDir + QStringLiteral("/icons/acl_auditor.ico")));
        QVERIFY(QFileInfo::exists(assetsDir + QStringLiteral("/icons/acl_auditor.png")));
        QVERIFY(QDir(transDir).exists());
        QVERIFY(QFileInfo::exists(transDir + QStringLiteral("/aclauditor_pt_BR.qm")));
        QVERIFY(QFileInfo::exists(transDir + QStringLiteral("/aclauditor_en_US.qm")));

        const QIcon appIcon = AclAuditor::Utils::ApplicationPathUtils::getAppIcon();
        QVERIFY(!appIcon.isNull());
    }
};

QTEST_MAIN(TestFrameworkAndPlugins)
#if !defined(__INTELLISENSE__)
#include "test_01_FrameworkAndPlugins.moc"
#endif
