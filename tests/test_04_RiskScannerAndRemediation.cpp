#include "TestSupport.hpp"
#include "engine/AclTypes.hpp"
#include "engine/RemediationPlanner.hpp"
#include "engine/RiskScanner.hpp"

#include <QDir>
#include <QFile>
#include <QtTest>

using namespace AclAuditor::Engine;

class TestRiskScannerAndRemediation : public QObject {
    Q_OBJECT

private slots:
    void testHeuristicsPredicates() {
        QVERIFY(RiskScanner::isExecutableOrScript(L"C:\\Tools\\run.exe"));
        QVERIFY(RiskScanner::isExecutableOrScript(L"C:\\Scripts\\deploy.PS1"));
        QVERIFY(RiskScanner::isExecutableOrScript(L"C:\\Bin\\mod.dll"));
        QVERIFY(RiskScanner::isExecutableOrScript(L"init.bat"));
        QVERIFY(!RiskScanner::isExecutableOrScript(L"notes.txt"));
        QVERIFY(!RiskScanner::isExecutableOrScript(L"noext"));

        QVERIFY(RiskScanner::isSensitiveName(L"C:\\Dados\\Financeiro_2026"));
        QVERIFY(RiskScanner::isSensitiveName(L"passwords.xlsx"));
        QVERIFY(RiskScanner::isSensitiveName(L"backup_secrets"));
        QVERIFY(!RiskScanner::isSensitiveName(L"public_icons"));
    }

    void testAllNodeAndTreeRiskHeuristics() {
        // Construct comprehensive synthetic node triggering every heuristic
        FileSecurityNode root = AclTestSupport::makeSampleTree(L"C:\\Financeiro_Backup");
        root.fileName = L"Financeiro_Backup";

        // Add an orphaned SID ACE and an ExcessivePrivilege ACE (non-admin WRITE_DAC / WRITE_OWNER)
        AceEntry orphanAce = AclTestSupport::makeAce(
            L"S-1-5-21-1111-2222-3333-4444", L"", L"",
            PrincipalCategory::OrphanedSid, AccessType::Allow, FILE_GENERIC_READ, false);
        root.security.dacl.push_back(orphanAce);

        AceEntry excessiveAce = AclTestSupport::makeAce(
            L"S-1-5-32-545", L"Users", L"BUILTIN",
            PrincipalCategory::WellKnown, AccessType::Allow, FILE_GENERIC_READ | WRITE_DAC | WRITE_OWNER, false);
        root.security.dacl.push_back(excessiveAce);

        const std::vector<RiskFinding> rootFindings = RiskScanner::evaluateNodeRisks(root);
        QVERIFY(!rootFindings.empty());

        bool hasWorldWritable = false;
        bool hasOrphaned = false;
        bool hasBrokenInheritance = false;
        bool hasExcessivePrivilege = false;
        for (const auto& f : rootFindings) {
            if (f.category == RiskCategory::WorldWritable) hasWorldWritable = true;
            if (f.category == RiskCategory::OrphanedPrincipal) hasOrphaned = true;
            if (f.category == RiskCategory::BrokenInheritance) hasBrokenInheritance = true;
            if (f.category == RiskCategory::ExcessivePrivilege) hasExcessivePrivilege = true;
        }
        QVERIFY(hasWorldWritable);
        QVERIFY(hasOrphaned);
        QVERIFY(hasBrokenInheritance);
        QVERIFY(hasExcessivePrivilege);

        // Tree evaluation should also detect BinaryPlanting on deploy.ps1
        const std::vector<RiskFinding> treeFindings = RiskScanner::evaluateTreeRisks(root);
        QVERIFY(treeFindings.size() >= rootFindings.size());
        bool hasBinaryPlanting = false;
        for (const auto& f : treeFindings) {
            if (f.category == RiskCategory::BinaryPlanting) {
                hasBinaryPlanting = true;
            }
            QVERIFY(!f.title.empty());
            QVERIFY(!f.affectedPath.empty());
        }
        QVERIFY(hasBinaryPlanting);

        QCOMPARE(RiskScanner::calculateRiskScore({}), 0);
        const int score = RiskScanner::calculateRiskScore(treeFindings);
        QVERIFY(score > 0);
        QVERIFY(score <= 100);
    }

    void testRemediationCommandsAndPlannerExport() {
        RiskFinding f;
        f.category = RiskCategory::OrphanedPrincipal;
        f.severity = RiskSeverity::Medium;
        f.affectedPath = L"C:\\Target\\data";
        f.affectedPrincipal = L"S-1-5-21-1-2-3-999";
        RiskScanner::generateRemediationCommands(f);
        QVERIFY(QString::fromStdWString(f.icaclsCommand).contains(QStringLiteral("icacls")));
        QVERIFY(QString::fromStdWString(f.powershellCommand).contains(QStringLiteral("Get-Acl")));

        FileSecurityNode tree = AclTestSupport::makeSampleTree(L"C:\\AuditTarget");
        const auto findings = RiskScanner::evaluateTreeRisks(tree);
        QVERIFY(!findings.empty());

        const RemediationPlan withRollback = RemediationPlanner::buildPlan(tree.path, findings, true);
        QCOMPARE(withRollback.totalIssuesToRemediate, findings.size());
        QVERIFY(!withRollback.title.empty());
        QVERIFY(!withRollback.icaclsScript.empty());
        QVERIFY(!withRollback.powershellScript.empty());
        QVERIFY(!withRollback.markdownReport.empty());
        QVERIFY(!withRollback.rollbackScript.empty());
        QVERIFY(QString::fromStdWString(withRollback.icaclsScript).contains(QStringLiteral("icacls")));
        QVERIFY(QString::fromStdWString(withRollback.powershellScript).contains(QStringLiteral("Set-Acl")));

        QVERIFY(QString::fromStdWString(withRollback.icaclsScript).contains(QStringLiteral("acl_auditor_backup.acl")));

        const RemediationPlan noRollback = RemediationPlanner::buildPlan(tree.path, findings, false);
        QCOMPARE(noRollback.totalIssuesToRemediate, findings.size());
        QVERIFY(!QString::fromStdWString(noRollback.icaclsScript).contains(QStringLiteral("acl_auditor_backup.acl")));
        QVERIFY(!noRollback.rollbackScript.empty());

        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());

        const std::wstring ps1Out = sandbox.filePathW(QStringLiteral("remediate.ps1"));
        const std::wstring batOut = sandbox.filePathW(QStringLiteral("remediate.bat"));
        const std::wstring mdOut = sandbox.filePathW(QStringLiteral("remediate.md"));

        QVERIFY(RemediationPlanner::exportScriptToFile(withRollback.powershellScript, ps1Out));
        QVERIFY(RemediationPlanner::exportScriptToFile(withRollback.icaclsScript, batOut));
        QVERIFY(RemediationPlanner::exportScriptToFile(withRollback.markdownReport, mdOut));

        QVERIFY(QFile::exists(QString::fromStdWString(ps1Out)));
        QVERIFY(QFile::exists(QString::fromStdWString(batOut)));
        QVERIFY(QFile::exists(QString::fromStdWString(mdOut)));
        QVERIFY(QFileInfo(QString::fromStdWString(mdOut)).size() > 0);
    }
};

QTEST_GUILESS_MAIN(TestRiskScannerAndRemediation)
#if !defined(__INTELLISENSE__)
#include "test_04_RiskScannerAndRemediation.moc"
#endif
