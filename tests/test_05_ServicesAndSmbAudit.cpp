#include "engine/AclTypes.hpp"
#include "engine/ServiceAuditor.hpp"
#include "engine/SmbShareAuditor.hpp"

#include <QtTest>

using namespace AclAuditor::Engine;

class TestServicesAndSmbAudit : public QObject {
    Q_OBJECT

private slots:
    void testServiceAuditorEnumeratesAndInspectsSingleService() {
        const std::vector<ServiceSecurityFinding> services = ServiceAuditor::auditServices();
        QVERIFY2(!services.empty(), "Expected Windows SCM to report at least one local service.");

        for (const auto& svc : services) {
            QVERIFY(!svc.serviceName.empty());
            if (svc.isUnquotedPathWithSpaces) {
                QVERIFY(!svc.binaryPath.empty());
            }
        }

        const ServiceSecurityFinding& first = services.front();
        const ServiceSecurityFinding single = ServiceAuditor::auditSingleService(first.serviceName);
        QCOMPARE(QString::fromStdWString(single.serviceName),
                 QString::fromStdWString(first.serviceName));
        QCOMPARE(QString::fromStdWString(single.binaryPath),
                 QString::fromStdWString(first.binaryPath));
        QCOMPARE(single.isUnquotedPathWithSpaces, first.isUnquotedPathWithSpaces);

        const ServiceSecurityFinding missing = ServiceAuditor::auditSingleService(
            L"ACL_Auditor_Missing_Service_XYZ_000");
        QVERIFY(missing.binaryPath.empty());
        QVERIFY(!missing.isUnquotedPathWithSpaces);
    }

    void testSmbShareAuditorEnumeratesAndInspectsSafely() {
        const std::vector<SmbShareFinding> shares = SmbShareAuditor::auditShares();
        for (const auto& share : shares) {
            QVERIFY(!share.shareName.empty());
        }

        if (!shares.empty()) {
            const SmbShareFinding& first = shares.front();
            const SmbShareFinding single = SmbShareAuditor::auditSingleShare(first.shareName);
            QCOMPARE(QString::fromStdWString(single.shareName),
                     QString::fromStdWString(first.shareName));
            QCOMPARE(QString::fromStdWString(single.localPath),
                     QString::fromStdWString(first.localPath));
        }

        const SmbShareFinding missing = SmbShareAuditor::auditSingleShare(
            L"ACL_Auditor_Missing_Share_XYZ_000");
        QVERIFY(missing.localPath.empty());
        QVERIFY(!missing.isEveryoneFullControl);
    }
};

QTEST_GUILESS_MAIN(TestServicesAndSmbAudit)
#if !defined(__INTELLISENSE__)
#include "test_05_ServicesAndSmbAudit.moc"
#endif
