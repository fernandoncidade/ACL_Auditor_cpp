#include "TestSupport.hpp"
#include "engine/RiskScanner.hpp"
#include "engine/SnapshotEngine.hpp"
#include "language/GerenciadorTraducao.hpp"
#include "model/AceTableModel.hpp"
#include "model/AclTreeModel.hpp"
#include "model/DiffTableModel.hpp"
#include "model/RiskTableModel.hpp"
#include "utils/ApplicationPathUtils.hpp"
#include "utils/ExplorerIntegration.hpp"
#include "utils/LogManager.hpp"

#include <QAbstractItemModelTester>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QtTest>

using namespace AclAuditor::Engine;
using namespace AclAuditor::Model;
using namespace AclAuditor::Utils;
using namespace AclAuditor::Language;

class TestModelsUtilsAndI18n : public QObject {
    Q_OBJECT

private slots:
    void testAclTreeModelWithModelTester() {
        AclTreeModel model;
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

        FileSecurityNode tree = AclTestSupport::makeSampleTree(L"C:\\ModelRoot");
        model.setRootNode(tree);

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.columnCount(), 6);

        const QModelIndex rootIdx = model.index(0, 0);
        QVERIFY(rootIdx.isValid());
        QCOMPARE(model.rowCount(rootIdx), 2);
        QVERIFY(model.getNodeAt(rootIdx) != nullptr);
        QVERIFY(!model.data(rootIdx, Qt::DisplayRole).toString().isEmpty());
        QVERIFY(!model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString().isEmpty());

        const QModelIndex childIdx = model.index(0, 0, rootIdx);
        QVERIFY(childIdx.isValid());
        QCOMPARE(model.parent(childIdx), rootIdx);

        model.refreshTranslations();
        model.clear();
        QCOMPARE(model.rowCount(), 0);
    }

    void testAceRiskAndDiffTableModels() {
        FileSecurityNode tree = AclTestSupport::makeSampleTree(L"C:\\TablesRoot");
        const auto findings = RiskScanner::evaluateTreeRisks(tree);
        QVERIFY(!findings.empty());

        {
            AceTableModel aceModel;
            QAbstractItemModelTester tester(&aceModel, QAbstractItemModelTester::FailureReportingMode::QtTest);
            aceModel.setAces(tree.security.dacl);
            QCOMPARE(aceModel.rowCount(), static_cast<int>(tree.security.dacl.size()));
            QVERIFY(aceModel.columnCount() > 0);
            QVERIFY(aceModel.getAceAt(0) != nullptr);
            QVERIFY(aceModel.getAceAt(999) == nullptr);
            QVERIFY(!aceModel.data(aceModel.index(0, 0), Qt::DisplayRole).toString().isEmpty());
            aceModel.refreshTranslations();
            aceModel.clear();
            QCOMPARE(aceModel.rowCount(), 0);
        }

        {
            RiskTableModel riskModel;
            QAbstractItemModelTester tester(&riskModel, QAbstractItemModelTester::FailureReportingMode::QtTest);
            riskModel.setFindings(findings);
            QCOMPARE(riskModel.rowCount(), static_cast<int>(findings.size()));
            QVERIFY(riskModel.getFindingAt(0) != nullptr);
            QCOMPARE(riskModel.getAllFindings().size(), findings.size());
            QVERIFY(!RiskTableModel::formatFindingDescriptionLocalized(findings.front()).isEmpty());
            QVERIFY(!RiskTableModel::formatFindingTechnicalDetailLocalized(findings.front()).isEmpty());
            riskModel.refreshTranslations();
            riskModel.clear();
            QCOMPARE(riskModel.rowCount(), 0);
        }

        {
            FileSecurityNode modified = tree;
            modified.security.isInheritanceBlocked = false;
            SnapshotEngine& snapEngine = SnapshotEngine::instance();
            SnapshotRecord a = snapEngine.createSnapshot(L"A", tree.path, tree, findings);
            SnapshotRecord b = snapEngine.createSnapshot(L"B", modified.path, modified, {});
            SnapshotDiffResult diff = snapEngine.compareSnapshots(a, b);

            DiffTableModel diffModel;
            QAbstractItemModelTester tester(&diffModel, QAbstractItemModelTester::FailureReportingMode::QtTest);
            diffModel.setDiff(diff);
            QCOMPARE(diffModel.rowCount(), static_cast<int>(diff.changes.size()));
            QVERIFY(!diffModel.data(diffModel.index(0, 0), Qt::DisplayRole).toString().isEmpty());
            diffModel.refreshTranslations();
            diffModel.clear();
            QCOMPARE(diffModel.rowCount(), 0);
        }
    }

    void testLogManagerAndExplorerIntegrationWithGuards() {
        AclTestSupport::IsolatedTestAppDataGuard appDataGuard;
        QVERIFY(appDataGuard.isValid());

        LogManager::configure_logging();
        LogManager::info(QStringLiteral("Suite test info message"));
        LogManager::warning(QStringLiteral("Suite test warning message"));
        LogManager::error(QStringLiteral("Suite test error message"));
        LogManager::debug(QStringLiteral("Suite test debug message"));

        const QString logPath = LogManager::getLogFilePath();
        QVERIFY(!logPath.isEmpty());
        QVERIFY(QFileInfo::exists(logPath));

        AclTestSupport::RegistryContextMenuGuard regGuard;
        QVERIFY(ExplorerIntegration::registerContextMenu());
        QVERIFY(ExplorerIntegration::isContextMenuRegistered());
        QVERIFY(ExplorerIntegration::unregisterContextMenu());
        QVERIFY(!ExplorerIntegration::isContextMenuRegistered());
    }

    void testTranslationManagerRuntimeSwitching() {
        AclTestSupport::IsolatedTestAppDataGuard appDataGuard;
        QVERIFY(appDataGuard.isValid());

        GerenciadorTraducao& gt = GerenciadorTraducao::instance();
        gt.inicializar();
        QVERIFY(gt.idiomas_disponiveis().contains(QStringLiteral("pt_BR")));
        QVERIFY(gt.idiomas_disponiveis().contains(QStringLiteral("en_US")));

        QSignalSpy spy(&gt, &GerenciadorTraducao::idioma_alterado);

        QVERIFY(gt.definir_idioma(QStringLiteral("en_US")));
        QCOMPARE(gt.obter_idioma_atual(), QStringLiteral("en_US"));
        QCOMPARE(gt.idiomaAtual(), Idioma::InglesEstadosUnidos);

        gt.mudarIdioma(Idioma::PortuguesBrasil);
        QCOMPARE(gt.obter_idioma_atual(), QStringLiteral("pt_BR"));
        QCOMPARE(gt.idiomaAtual(), Idioma::PortuguesBrasil);

        QVERIFY(!gt.definir_idioma(QStringLiteral("xx_XX")));
        QVERIFY(spy.count() >= 1);
        QVERIFY(!GerenciadorTraducao::trText("Arquivo").isEmpty());
    }
};

QTEST_MAIN(TestModelsUtilsAndI18n)
#if !defined(__INTELLISENSE__)
#include "test_07_ModelsUtilsAndI18n.moc"
#endif
