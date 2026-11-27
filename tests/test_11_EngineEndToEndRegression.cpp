#include "TestSupport.hpp"
#include "engine/AclTypes.hpp"
#include "engine/EffectiveAccessCalculator.hpp"
#include "engine/NtfsSecurityUtils.hpp"
#include "engine/RemediationPlanner.hpp"
#include "engine/RiskScanner.hpp"
#include "engine/ServiceAuditor.hpp"
#include "engine/SmbShareAuditor.hpp"
#include "engine/SnapshotEngine.hpp"

#include <QtTest>

using namespace AclAuditor::Engine;

/**
 * @brief End-to-end 7-phase engine regression suite migrated from mocks/teste.cpp.
 */
class TestEngineEndToEndRegression : public QObject {
    Q_OBJECT

private:
    static FileSecurityNode makeVulnerableTestNode() {
        const SidInfo everyone = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-1-0");

        FileSecurityNode testNode;
        testNode.path = L"C:\\Temp\\VulnerableApp";
        testNode.fileName = L"VulnerableApp";
        testNode.isDirectory = true;

        AceEntry vulnAce;
        vulnAce.trustee = everyone;
        vulnAce.type = AccessType::Allow;
        vulnAce.canWriteData = true;
        vulnAce.modify = true;
        testNode.security.dacl.push_back(vulnAce);
        return testNode;
    }

private slots:
    void phase01_ntfsSecurityUtilsAndCrypto() {
        NtfsSecurityUtils::enableSecurityPrivileges();
        (void)NtfsSecurityUtils::isProcessElevated();

        const SidInfo everyone = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-1-0");
        QVERIFY(everyone.isWellKnown);
        QVERIFY(!everyone.getDisplayName().empty());

        const std::string sha = NtfsSecurityUtils::computeSha256Hex("ACL Auditor Test Data");
        QCOMPARE(sha.size(), size_t{64});
    }

    void phase02_windowsDirectorySecurityDescriptor() {
        SecurityDescriptorInfo sec;
        QVERIFY(NtfsSecurityUtils::getFileSecurityInfo(L"C:\\Windows", sec));
        QVERIFY(!sec.owner.getDisplayName().empty());
        QVERIFY(!sec.dacl.empty());
    }

    void phase03_effectiveAccessAndExplanationTree() {
        const EffectiveAccessResult effByName =
            EffectiveAccessCalculator::calculateEffectiveAccess(L"C:\\Windows", L"Everyone");
        QVERIFY(!effByName.targetPrincipal.sidString.empty());
        QVERIFY(!effByName.explanationTree.empty());
        QVERIFY(!effByName.finalVerdictSummary.empty());
        (void)effByName.canRead;
        (void)effByName.canWrite;
        (void)effByName.canReadAndExecute;

        const EffectiveAccessResult effBySid =
            EffectiveAccessCalculator::calculateEffectiveAccess(L"C:\\Windows", L"S-1-1-0");
        QCOMPARE(QString::fromStdWString(effBySid.targetPrincipal.sidString), QStringLiteral("S-1-1-0"));
        QVERIFY(!effBySid.explanationTree.empty());
        QVERIFY(!effBySid.finalVerdictSummary.empty());
    }

    void phase04_riskScannerOnSyntheticVulnerableNode() {
        const FileSecurityNode testNode = makeVulnerableTestNode();
        const auto findings = RiskScanner::evaluateNodeRisks(testNode);
        QVERIFY(!findings.empty());
        QVERIFY(!findings[0].title.empty());

        const int score = RiskScanner::calculateRiskScore(findings);
        QVERIFY(score > 0 && score <= 100);
    }

    void phase05_remediationPlannerScripts() {
        const FileSecurityNode testNode = makeVulnerableTestNode();
        const auto findings = RiskScanner::evaluateNodeRisks(testNode);
        QVERIFY(!findings.empty());

        const RemediationPlan plan = RemediationPlanner::buildPlan(testNode.path, findings, true);
        QVERIFY(!plan.icaclsScript.empty());
        QVERIFY(!plan.powershellScript.empty());
    }

    void phase06_snapshotEnginePersistenceAndTemporalDiff() {
        AclTestSupport::IsolatedTestAppDataGuard appDataGuard;
        QVERIFY(appDataGuard.isValid());

        SnapshotEngine& engine = SnapshotEngine::instance();
        QVERIFY(engine.initializeDatabase(
            appDataGuard.dbFilePath(QStringLiteral("engine_e2e_regression.db")).toStdWString()));

        const FileSecurityNode testNode = makeVulnerableTestNode();
        const auto findings = RiskScanner::evaluateNodeRisks(testNode);
        QVERIFY(!findings.empty());

        const SnapshotRecord snapA =
            engine.createSnapshot(L"Snapshot Teste A", testNode.path, testNode, findings);
        QCOMPARE(snapA.sha256Hash.size(), size_t{64});
        QVERIFY(engine.saveSnapshot(snapA));

        FileSecurityNode modifiedNode = testNode;
        AceEntry newAce;
        newAce.trustee = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-5-11");
        newAce.type = AccessType::Allow;
        newAce.fullControl = true;
        modifiedNode.security.dacl.push_back(newAce);

        const SnapshotRecord snapB =
            engine.createSnapshot(L"Snapshot Teste B", modifiedNode.path, modifiedNode, findings);
        QVERIFY(engine.saveSnapshot(snapB));

        const SnapshotDiffResult diff = engine.compareSnapshots(snapA, snapB);
        QVERIFY(diff.totalChanges > 0);

        QVERIFY(engine.deleteSnapshot(snapA.id));
        QVERIFY(engine.deleteSnapshot(snapB.id));
        engine.cleanupThreadConnection();
    }

    void phase07_servicesAndSmbSharesEnumeration() {
        const auto services = ServiceAuditor::auditServices();
        QVERIFY(!services.empty());

        const auto shares = SmbShareAuditor::auditShares();
        for (const auto& share : shares) {
            QVERIFY(!share.shareName.empty());
        }
    }
};

QTEST_GUILESS_MAIN(TestEngineEndToEndRegression)
#if !defined(__INTELLISENSE__)
#include "test_11_EngineEndToEndRegression.moc"
#endif
