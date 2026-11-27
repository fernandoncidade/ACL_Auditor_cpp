#include "TestSupport.hpp"
#include "engine/RiskScanner.hpp"
#include "engine/SnapshotEngine.hpp"
#include "language/GerenciadorTraducao.hpp"
#include "ui/AboutDialog.hpp"
#include "ui/MainWindow.hpp"
#include "ui/ManualDialog.hpp"
#include "ui/RemediationDialog.hpp"
#include "ui/ReportExportDialog.hpp"
#include "ui/RiskDashboardWidget.hpp"
#include "ui/ServicesSharesWidget.hpp"
#include "ui/SnapshotDiffWidget.hpp"
#include "ui/TableEnhancer.hpp"
#include "ui/ThemeManager.hpp"
#include "ui/VisualExplanationWidget.hpp"
#include "ui/Win32DwmHelper.hpp"

#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QJsonDocument>
#include <QPointer>
#include <QStandardItemModel>
#include <QTableView>
#include <QTableWidget>
#include <QtTest>

using namespace AclAuditor::Engine;
using namespace AclAuditor::UI;
using namespace AclAuditor::Language;

class TestGuiWidgetsDialogsAndMainWindow : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        m_appDataGuard = std::make_unique<AclTestSupport::IsolatedTestAppDataGuard>();
        QVERIFY(m_appDataGuard->isValid());
        // Must initialize SnapshotEngine BEFORE any MainWindow constructs
        QVERIFY(SnapshotEngine::instance().initializeDatabase(
            m_appDataGuard->dbFilePath(QStringLiteral("gui_snapshots.db")).toStdWString()));
        GerenciadorTraducao::instance().inicializar();
    }

    void cleanupTestCase() {
        SnapshotEngine::instance().cleanupThreadConnection();
        m_appDataGuard.reset();
    }

    void testThemeManagerAndDwmHelperOffscreenSafety() {
        ThemeManager& tm = ThemeManager::instance();
        QVERIFY(tm.createDarkPalette().color(QPalette::Window) !=
                tm.createLightPalette().color(QPalette::Window));

        QWidget dummy;
        Win32DwmHelper::applyTitleBarTheme(&dummy, true);
        Win32DwmHelper::applyMicaAndDarkMode(&dummy);
        Win32DwmHelper::applyTitleBarTheme(nullptr, true);
        QVERIFY(!Win32DwmHelper::getProcessMemoryUsageString().isEmpty());

        Q_UNUSED(Win32DwmHelper::isSystemDarkMode());
    }

    void testTableEnhancerAndProxyFilter() {
        QStandardItemModel sourceModel(3, 2);
        sourceModel.setHeaderData(0, Qt::Horizontal, QStringLiteral("Name"));
        sourceModel.setHeaderData(1, Qt::Horizontal, QStringLiteral("Role"));
        sourceModel.setItem(0, 0, new QStandardItem(QStringLiteral("Alice")));
        sourceModel.setItem(0, 1, new QStandardItem(QStringLiteral("Admin")));
        sourceModel.setItem(1, 0, new QStandardItem(QStringLiteral("Bob")));
        sourceModel.setItem(1, 1, new QStandardItem(QStringLiteral("Auditor")));
        sourceModel.setItem(2, 0, new QStandardItem(QStringLiteral("Charlie")));
        sourceModel.setItem(2, 1, new QStandardItem(QStringLiteral("User")));

        QTableView tableView;
        MultiColumnSortFilterProxyModel proxy(&tableView);
        proxy.setSourceModel(&sourceModel);

        TableEnhancer* enhancer = TableEnhancer::enhanceTableView(
            &tableView, &proxy, QStringLiteral("suite_table"), {120, 120});
        QVERIFY(enhancer != nullptr);
        QCOMPARE(proxy.rowCount(), 3);

        proxy.setFilterPattern(QStringLiteral("Auditor"));
        QCOMPARE(proxy.filterPattern(), QStringLiteral("Auditor"));
        QCOMPARE(proxy.rowCount(), 1);
        proxy.setFilterPattern(QString());
        QCOMPARE(proxy.rowCount(), 3);

        TableEnhancer::autoFitColumns(&tableView);
        TableEnhancer::resetDefaultWidths(&tableView, {140, 160});
        TableEnhancer::copyAllToClipboard(&tableView, true);
        const QString clip = QApplication::clipboard()->text();
        QVERIFY(clip.contains(QStringLiteral("Alice")));

        QTableWidget tableWidget(2, 2);
        tableWidget.setItem(0, 0, new QTableWidgetItem(QStringLiteral("Key")));
        tableWidget.setItem(0, 1, new QTableWidgetItem(QStringLiteral("Val")));
        QVERIFY(TableEnhancer::enhanceTableWidget(&tableWidget, QStringLiteral("w"), {100, 100}) != nullptr);
    }

    void testStandAloneWidgetsAndRetranslation() {
        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());
        QVERIFY(sandbox.createSubdir(QStringLiteral("widget_dir")));
        QVERIFY(sandbox.applySddl(
            sandbox.filePathW(QStringLiteral("widget_dir")),
            L"D:P(A;OICI;FA;;;BA)(A;OICI;FR;;;WD)",
            true));

        VisualExplanationWidget visual;
        visual.setTargetObject(sandbox.filePath(QStringLiteral("widget_dir")));
        visual.evaluateAccess();
        visual.applyTheme(true);
        visual.retranslateUi();

        FileSecurityNode sample = AclTestSupport::makeSampleTree(sandbox.filePathW(QStringLiteral("widget_dir")));
        const auto findings = RiskScanner::evaluateTreeRisks(sample);

        RiskDashboardWidget dashboard;
        QSignalSpy spyPlan(&dashboard, &RiskDashboardWidget::openRemediationPlanRequested);
        dashboard.setFindings(findings);
        dashboard.applyTheme(false);
        dashboard.retranslateUi();
        dashboard.clear();
        Q_UNUSED(spyPlan);

        ServicesSharesWidget svcShares;
        svcShares.applyTheme(true);
        svcShares.retranslateUi();

        SnapshotDiffWidget diffWidget;
        diffWidget.refreshSnapshotList();
        diffWidget.applyTheme(true);
        diffWidget.retranslateUi();
    }

    void testDialogsAndReportGeneration() {
        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());

        const FileSecurityNode tree = AclTestSupport::makeSampleTree(sandbox.filePathW(QStringLiteral("report_root")));
        const auto findings = RiskScanner::evaluateTreeRisks(tree);
        QVERIFY(!findings.empty());

        {
            QPointer<ReportExportDialog> dlg = new ReportExportDialog(tree, findings);
            QVERIFY(dlg != nullptr);
            dlg->retranslateUi();

            const QString html = dlg->generateHtmlReport();
            QVERIFY(html.contains(QStringLiteral("<html"), Qt::CaseInsensitive));

            const QString csv = dlg->generateCsvReport();
            QVERIFY(!csv.trimmed().isEmpty());

            const QString jsonStr = dlg->generateJsonReport();
            QJsonParseError jsonErr;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonErr);
            QCOMPARE(jsonErr.error, QJsonParseError::NoError);
            QVERIFY(jsonDoc.isObject());

            const QString pdfOut = sandbox.filePath(QStringLiteral("audit_report.pdf"));
            QVERIFY(dlg->exportToPdf(pdfOut, html));
            QVERIFY(QFileInfo::exists(pdfOut));
            QVERIFY(QFileInfo(pdfOut).size() > 0);

            dlg->close();
            QApplication::processEvents();
            if (dlg) {
                delete dlg.data();
            }
        }

        {
            QPointer<RemediationDialog> dlg = new RemediationDialog(tree.path, findings);
            QVERIFY(dlg != nullptr);
            dlg->retranslateUi();
            dlg->close();
            QApplication::processEvents();
            if (dlg) {
                delete dlg.data();
            }
        }

        {
            QPointer<AboutDialog> dlg = new AboutDialog();
            QVERIFY(dlg != nullptr);
            dlg->applyTheme(true);
            dlg->onIdiomaAlterado(QStringLiteral("en_US"));
            dlg->onIdiomaAlterado(QStringLiteral("pt_BR"));
            dlg->close();
            QApplication::processEvents();
            if (dlg) {
                delete dlg.data();
            }
        }

        {
            QPointer<ManualDialog> dlg = new ManualDialog();
            QVERIFY(dlg != nullptr);
            dlg->applyTheme(false);
            dlg->onIdiomaAlterado(QStringLiteral("en_US"));
            dlg->onIdiomaAlterado(QStringLiteral("pt_BR"));
            dlg->close();
            QApplication::processEvents();
            if (dlg) {
                delete dlg.data();
            }
        }
    }

    void testMainWindowFullLifecycle() {
        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());
        QVERIFY(sandbox.createSubdir(QStringLiteral("mw_audit")));
        QVERIFY(sandbox.createFile(QStringLiteral("mw_audit/script.ps1"), QByteArray("Write-Host 'hi'\n")));

        MainWindow mainWin;
        mainWin.show();
        QApplication::processEvents();

        mainWin.openPathForAudit(sandbox.filePath(QStringLiteral("mw_audit")));
        QApplication::processEvents();

        mainWin.onExplainCurrentObject();
        mainWin.applyTheme(true);
        mainWin.applyTheme(false);

        GerenciadorTraducao::instance().definir_idioma(QStringLiteral("en_US"));
        QApplication::processEvents();
        mainWin.retranslateUi();

        GerenciadorTraducao::instance().definir_idioma(QStringLiteral("pt_BR"));
        QApplication::processEvents();
        mainWin.retranslateUi();

        mainWin.close();
        QApplication::processEvents();
    }

private:
    std::unique_ptr<AclTestSupport::IsolatedTestAppDataGuard> m_appDataGuard;
};

QTEST_MAIN(TestGuiWidgetsDialogsAndMainWindow)
#if !defined(__INTELLISENSE__)
#include "test_08_GuiWidgetsDialogsAndMainWindow.moc"
#endif
