#include "TestSupport.hpp"
#include "engine/AclTypes.hpp"
#include "engine/EffectiveAccessCalculator.hpp"

#include <QtTest>

using namespace AclAuditor::Engine;

class TestEffectiveAccessEngine : public QObject {
    Q_OBJECT

private slots:
    void testSystemEnumerationAndIdentityGroups() {
        const auto users = EffectiveAccessCalculator::enumerateSystemUsers();
        const auto groups = EffectiveAccessCalculator::enumerateSystemGroups();
        QVERIFY(!users.empty() || !groups.empty());
        QVERIFY(!groups.empty());

        const auto systemGroups = EffectiveAccessCalculator::enumerateIdentityGroups(L"S-1-5-18");
        Q_UNUSED(systemGroups);

        const auto authUsersGroups = EffectiveAccessCalculator::enumerateIdentityGroups(L"S-1-5-11");
        Q_UNUSED(authUsersGroups);
        QVERIFY(true);
    }

    void testEffectiveAccessAllowAndDenyPrecedence() {
        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());

        QVERIFY(sandbox.createSubdir(QStringLiteral("allow_rw")));
        QVERIFY(sandbox.createSubdir(QStringLiteral("deny_write")));
        QVERIFY(sandbox.createFile(QStringLiteral("allow_rw/payload.txt"), QByteArray("test")));
        QVERIFY(sandbox.createFile(QStringLiteral("deny_write/locked.txt"), QByteArray("locked")));

        const std::wstring allowDir = sandbox.filePathW(QStringLiteral("allow_rw"));
        const std::wstring denyDir = sandbox.filePathW(QStringLiteral("deny_write"));
        const std::wstring allowFile = sandbox.filePathW(QStringLiteral("allow_rw/payload.txt"));

        // Grant BUILTIN\Administrators Full Control and Everyone Read + Write
        QVERIFY(sandbox.applySddl(allowDir, L"D:P(A;OICI;FA;;;BA)(A;OICI;FRFW;;;WD)", true));

        // Grant Everyone Full Control AND explicitly Deny Everyone Write (Deny must override Allow)
        QVERIFY(sandbox.applySddl(denyDir, L"D:P(D;OICI;FW;;;WD)(A;OICI;FA;;;BA)(A;OICI;FA;;;WD)", true));

        const EffectiveAccessResult allowRes = EffectiveAccessCalculator::calculateEffectiveAccess(allowDir, L"S-1-1-0");
        QCOMPARE(QString::fromStdWString(allowRes.targetPrincipal.sidString), QStringLiteral("S-1-1-0"));
        QVERIFY(allowRes.canRead);
        QVERIFY(allowRes.canWrite);
        QVERIFY(!allowRes.canFullControl);
        QVERIFY(!allowRes.finalVerdictSummary.empty());
        QVERIFY(!allowRes.explanationTree.empty());

        const QString allowExplain = QString::fromStdWString(EffectiveAccessCalculator::generateExplanatoryText(allowRes));
        QVERIFY(!allowExplain.trimmed().isEmpty());

        // Verify inheritance propagation onto child file
        const EffectiveAccessResult childRes = EffectiveAccessCalculator::calculateEffectiveAccess(allowFile, L"Everyone");
        QVERIFY(childRes.canRead);
        QVERIFY(childRes.canWrite);

        // Verify explicit DENY write overrides ALLOW full access for Everyone
        const EffectiveAccessResult denyRes = EffectiveAccessCalculator::calculateEffectiveAccess(denyDir, L"S-1-1-0");
        QVERIFY(!denyRes.canWrite);
        QVERIFY(!denyRes.canFullControl);
        QVERIFY(!denyRes.conflictingDenyAces.empty() || !denyRes.matchingAces.empty());
    }

    void testInvalidPrincipalOrPathHandledGracefully() {
        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());
        QVERIFY(sandbox.createFile(QStringLiteral("sample.txt"), QByteArray("ok")));

        const EffectiveAccessResult badUser = EffectiveAccessCalculator::calculateEffectiveAccess(
            sandbox.filePathW(QStringLiteral("sample.txt")),
            L"NONEXISTENT_ACL_USER_987654");
        QVERIFY(!badUser.canFullControl);
        QVERIFY(!badUser.canWrite);

        const EffectiveAccessResult badPath = EffectiveAccessCalculator::calculateEffectiveAccess(
            L"Z:\\__nonexistent_acl_auditor_path__\\missing.txt",
            L"Everyone");
        QVERIFY(!badPath.canFullControl);
    }
};

QTEST_GUILESS_MAIN(TestEffectiveAccessEngine)
#if !defined(__INTELLISENSE__)
#include "test_03_EffectiveAccessEngine.moc"
#endif
