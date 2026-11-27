#include "TestSupport.hpp"
#include "engine/AclTypes.hpp"
#include "engine/NtfsSecurityUtils.hpp"

#include <QtTest>
#include <windows.h>
#include <sddl.h>

using namespace AclAuditor::Engine;

class TestNtfsSecurityAndCrypto : public QObject {
    Q_OBJECT

private slots:
    void testPrivilegeEnableAndProcessElevation() {
        const bool privResult = NtfsSecurityUtils::enableSecurityPrivileges();
        Q_UNUSED(privResult);
        const bool elevated = NtfsSecurityUtils::isProcessElevated();
        Q_UNUSED(elevated);
        QVERIFY(true);
    }

    void testSidResolutionWellKnownAndOrphaned() {
        {
            PSID pSid = nullptr;
            QVERIFY(ConvertStringSidToSidW(L"S-1-1-0", &pSid));
            const SidInfo info = NtfsSecurityUtils::resolveSid(pSid);
            LocalFree(pSid);

            QCOMPARE(QString::fromStdWString(info.sidString), QStringLiteral("S-1-1-0"));
            QVERIFY(!info.isOrphaned);
            QVERIFY(info.isWellKnown);
            QVERIFY(!info.accountName.empty());
        }

        {
            PSID pSid = nullptr;
            QVERIFY(ConvertStringSidToSidW(L"S-1-5-18", &pSid));
            const SidInfo info = NtfsSecurityUtils::resolveSid(pSid);
            LocalFree(pSid);

            QCOMPARE(QString::fromStdWString(info.sidString), QStringLiteral("S-1-5-18"));
            QCOMPARE(info.category, PrincipalCategory::ServiceAccount);
            QVERIFY(!info.isOrphaned);
        }

        {
            PSID pSid = nullptr;
            QVERIFY(ConvertStringSidToSidW(L"S-1-5-21-987654321-123456789-555555555-99999", &pSid));
            const SidInfo info = NtfsSecurityUtils::resolveSid(pSid);
            LocalFree(pSid);

            QVERIFY(info.isOrphaned);
            QCOMPARE(info.category, PrincipalCategory::OrphanedSid);
            QVERIFY(QString::fromStdWString(info.getDisplayName()).contains(QStringLiteral("S-1-5-21-")));
        }

        {
            const SidInfo nullInfo = NtfsSecurityUtils::resolveSid(nullptr);
            QVERIFY(nullInfo.sidString.empty() || nullInfo.isOrphaned || nullInfo.category == PrincipalCategory::Unknown);
        }
    }

    void testResolveAccountOrSidString() {
        const SidInfo bySid = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-5-11");
        QCOMPARE(QString::fromStdWString(bySid.sidString), QStringLiteral("S-1-5-11"));
        QVERIFY(!bySid.isOrphaned);

        const SidInfo byName = NtfsSecurityUtils::resolveAccountOrSidString(L"SYSTEM");
        QCOMPARE(QString::fromStdWString(byName.sidString), QStringLiteral("S-1-5-18"));

        const SidInfo unknown = NtfsSecurityUtils::resolveAccountOrSidString(L"NONEXISTENT_ACL_AUDITOR_PRINCIPAL_XYZ");
        QCOMPARE(unknown.category, PrincipalCategory::Unknown);
        QVERIFY(!unknown.isOrphaned);
        QCOMPARE(QString::fromStdWString(unknown.sidString), QStringLiteral("NONEXISTENT_ACL_AUDITOR_PRINCIPAL_XYZ"));
    }

    void testRealNtfsDaclParsingAndDirectoryTreeScan() {
        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());
        QVERIFY(sandbox.createSubdir(QStringLiteral("sub_inherited")));
        QVERIFY(sandbox.createSubdir(QStringLiteral("sub_protected")));
        QVERIFY(sandbox.createFile(QStringLiteral("sub_protected/script.ps1"), QByteArray("Write-Output 'ok'\n")));
        QVERIFY(sandbox.createFile(QStringLiteral("sub_inherited/readme.txt"), QByteArray("hello")));

        const std::wstring protectedDir = sandbox.filePathW(QStringLiteral("sub_protected"));
        // Protected DACL granting Everyone Read and BUILTIN\Administrators Full Access, plus Everyone Deny Write
        QVERIFY(sandbox.applySddl(
            protectedDir,
            L"D:P(D;OICI;FW;;;WD)(A;OICI;FA;;;BA)(A;OICI;FR;;;WD)",
            true));

        SecurityDescriptorInfo secInfo;
        QVERIFY(NtfsSecurityUtils::getFileSecurityInfo(protectedDir, secInfo));
        QVERIFY(secInfo.hasDacl);
        QVERIFY(secInfo.isInheritanceBlocked);
        QVERIFY(!secInfo.owner.sidString.empty());
        QVERIFY(secInfo.dacl.size() >= 3);

        bool foundDenyEveryone = false;
        bool foundAllowEveryoneRead = false;
        for (const auto& ace : secInfo.dacl) {
            if (ace.trustee.sidString == L"S-1-1-0" && ace.type == AccessType::Deny) {
                foundDenyEveryone = true;
            }
            if (ace.trustee.sidString == L"S-1-1-0" && ace.type == AccessType::Allow && ace.read) {
                foundAllowEveryoneRead = true;
            }
        }
        QVERIFY(foundDenyEveryone);
        QVERIFY(foundAllowEveryoneRead);

        int callbackCount = 0;
        const FileSecurityNode tree = NtfsSecurityUtils::scanDirectoryTree(
            sandbox.rootPathW(),
            3,
            true,
            [&callbackCount](const std::wstring& path, int /*total*/) {
                if (!path.empty()) {
                    ++callbackCount;
                }
            });

        QCOMPARE(QString::fromStdWString(tree.path), QDir::toNativeSeparators(sandbox.rootPath()));
        QVERIFY(tree.isDirectory);
        QVERIFY(callbackCount >= 1);
        QVERIFY(tree.children.size() >= 2);
    }

    void testAccessMaskFormattingAndSha256() {
        QCOMPARE(QString::fromStdWString(NtfsSecurityUtils::formatAccessMaskHex(FILE_ALL_ACCESS)),
                 QStringLiteral("0x001F01FF"));
        QVERIFY(!NtfsSecurityUtils::formatAccessMaskDetailed(FILE_GENERIC_READ | FILE_GENERIC_WRITE, true).empty());
        QVERIFY(!NtfsSecurityUtils::formatAccessMaskDetailed(0, false).empty());

        // NIST FIPS 180-4 vectors
        const std::string emptyHash = NtfsSecurityUtils::computeSha256Hex(std::string(""));
        QCOMPARE(QString::fromStdString(emptyHash),
                 QStringLiteral("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));

        const std::string abcHash = NtfsSecurityUtils::computeSha256Hex(std::string("abc"));
        QCOMPARE(QString::fromStdString(abcHash),
                 QStringLiteral("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));

        const char* raw = "abc";
        QCOMPARE(QString::fromStdString(NtfsSecurityUtils::computeSha256Hex(raw, 3)),
                 QString::fromStdString(abcHash));
    }

    void testPathNormalizationAndSystemDirectoryDetection() {
        const std::wstring norm1 = NtfsSecurityUtils::normalizePath(L"C:/Windows/System32/");
        QCOMPARE(QString::fromStdWString(norm1), QStringLiteral("C:\\Windows\\System32"));

        QVERIFY(NtfsSecurityUtils::isSystemDirectory(L"C:\\Windows\\System32"));
        QVERIFY(NtfsSecurityUtils::isSystemDirectory(L"C:\\Windows"));

        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());
        QVERIFY(!NtfsSecurityUtils::isSystemDirectory(sandbox.rootPathW()));
    }
};

QTEST_GUILESS_MAIN(TestNtfsSecurityAndCrypto)
#if !defined(__INTELLISENSE__)
#include "test_02_NtfsSecurityAndCrypto.moc"
#endif
