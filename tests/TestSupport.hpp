#pragma once

#include "engine/AclTypes.hpp"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QVariant>

#include <vector>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#endif

namespace AclTestSupport {

class IsolatedTestAppDataGuard {
public:
    IsolatedTestAppDataGuard() {
        QStandardPaths::setTestModeEnabled(true);
        m_hadOriginalLocalAppData = qEnvironmentVariableIsSet("LOCALAPPDATA");
        if (m_hadOriginalLocalAppData) {
            m_originalLocalAppData = qgetenv("LOCALAPPDATA");
        }
        if (m_tempDir.isValid()) {
            qputenv("LOCALAPPDATA", QDir::toNativeSeparators(m_tempDir.path()).toUtf8());
        }
    }

    ~IsolatedTestAppDataGuard() {
        if (m_hadOriginalLocalAppData) {
            qputenv("LOCALAPPDATA", m_originalLocalAppData);
        } else {
            qunsetenv("LOCALAPPDATA");
        }
        QStandardPaths::setTestModeEnabled(false);
    }

    IsolatedTestAppDataGuard(const IsolatedTestAppDataGuard&) = delete;
    IsolatedTestAppDataGuard& operator=(const IsolatedTestAppDataGuard&) = delete;

    bool isValid() const { return m_tempDir.isValid(); }
    QString rootPath() const { return m_tempDir.path(); }
    QString dbFilePath(const QString& name = QStringLiteral("test_snapshots.db")) const {
        return QDir(m_tempDir.path()).filePath(name);
    }

private:
    QTemporaryDir m_tempDir;
    bool m_hadOriginalLocalAppData = false;
    QByteArray m_originalLocalAppData;
};

class RegistryContextMenuGuard {
public:
    static constexpr const char* kShellKey = "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\ACLAuditor";
    static constexpr const char* kBgKey    = "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\ACLAuditor";

    RegistryContextMenuGuard() {
        captureKey(QString::fromLatin1(kShellKey), m_shellSnapshot);
        captureKey(QString::fromLatin1(kBgKey), m_bgSnapshot);
    }

    ~RegistryContextMenuGuard() {
        restoreKey(QString::fromLatin1(kShellKey), m_shellSnapshot);
        restoreKey(QString::fromLatin1(kBgKey), m_bgSnapshot);
    }

    RegistryContextMenuGuard(const RegistryContextMenuGuard&) = delete;
    RegistryContextMenuGuard& operator=(const RegistryContextMenuGuard&) = delete;

private:
    struct KeySnapshot {
        bool hadAny = false;
        QMap<QString, QVariant> values;
    };

    static void captureKey(const QString& regPath, KeySnapshot& out) {
        QSettings settings(regPath, QSettings::NativeFormat);
        const QStringList keys = settings.allKeys();
        out.hadAny = !keys.isEmpty();
        for (const QString& k : keys) {
            out.values.insert(k, settings.value(k));
        }
    }

    static void restoreKey(const QString& regPath, const KeySnapshot& snap) {
        QSettings settings(regPath, QSettings::NativeFormat);
        settings.clear();
        if (snap.hadAny) {
            for (auto it = snap.values.cbegin(); it != snap.values.cend(); ++it) {
                settings.setValue(it.key(), it.value());
            }
        }
        settings.sync();
    }

    KeySnapshot m_shellSnapshot;
    KeySnapshot m_bgSnapshot;
};

class NtfsTestSandbox {
public:
    NtfsTestSandbox() = default;

    ~NtfsTestSandbox() {
        // Restore permissive DACLs on any hardened paths before QTemporaryDir deletes them
        for (auto it = m_trackedPaths.rbegin(); it != m_trackedPaths.rend(); ++it) {
            resetDacl(it->toStdWString());
        }
    }

    NtfsTestSandbox(const NtfsTestSandbox&) = delete;
    NtfsTestSandbox& operator=(const NtfsTestSandbox&) = delete;

    bool isValid() const { return m_tempDir.isValid(); }
    QString rootPath() const { return QDir::toNativeSeparators(m_tempDir.path()); }
    std::wstring rootPathW() const { return rootPath().toStdWString(); }

    QString filePath(const QString& relative) const {
        return QDir::toNativeSeparators(QDir(m_tempDir.path()).filePath(relative));
    }

    std::wstring filePathW(const QString& relative) const {
        return filePath(relative).toStdWString();
    }

    bool createSubdir(const QString& relative) {
        return QDir(m_tempDir.path()).mkpath(relative);
    }

    bool createFile(const QString& relative, const QByteArray& content) {
        const QString target = QDir(m_tempDir.path()).filePath(relative);
        QFileInfo info(target);
        if (!QDir().mkpath(info.absolutePath())) {
            return false;
        }
        QFile f(target);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        const qint64 written = f.write(content);
        f.close();
        return written == content.size();
    }

    void trackPathForCleanup(const QString& fullPath) {
        if (!m_trackedPaths.contains(fullPath)) {
            m_trackedPaths.append(fullPath);
        }
    }

#if defined(_WIN32)
    bool applySddl(const std::wstring& path, const std::wstring& sddl, bool protectInheritance = true) {
        PSECURITY_DESCRIPTOR pSd = nullptr;
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                sddl.c_str(), SDDL_REVISION_1, &pSd, nullptr) || !pSd) {
            return false;
        }

        BOOL daclPresent = FALSE;
        BOOL daclDefaulted = FALSE;
        PACL pDacl = nullptr;
        bool ok = false;
        if (GetSecurityDescriptorDacl(pSd, &daclPresent, &pDacl, &daclDefaulted) && daclPresent) {
            SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;
            secInfo |= protectInheritance ? PROTECTED_DACL_SECURITY_INFORMATION
                                          : UNPROTECTED_DACL_SECURITY_INFORMATION;
            DWORD err = SetNamedSecurityInfoW(
                const_cast<LPWSTR>(path.c_str()),
                SE_FILE_OBJECT,
                secInfo,
                nullptr,
                nullptr,
                pDacl,
                nullptr);
            ok = (err == ERROR_SUCCESS);
            if (ok) {
                trackPathForCleanup(QString::fromStdWString(path));
            }
        }
        LocalFree(pSd);
        return ok;
    }

    static bool resetDacl(const std::wstring& path) {
        PSECURITY_DESCRIPTOR pSd = nullptr;
        const wchar_t* resetSddl = L"D:AR(A;OICI;FA;;;BA)(A;OICI;FA;;;SY)(A;OICI;FA;;;CO)";
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                resetSddl, SDDL_REVISION_1, &pSd, nullptr) || !pSd) {
            return false;
        }
        BOOL daclPresent = FALSE;
        BOOL daclDefaulted = FALSE;
        PACL pDacl = nullptr;
        bool ok = false;
        if (GetSecurityDescriptorDacl(pSd, &daclPresent, &pDacl, &daclDefaulted) && daclPresent) {
            DWORD err = SetNamedSecurityInfoW(
                const_cast<LPWSTR>(path.c_str()),
                SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION,
                nullptr,
                nullptr,
                pDacl,
                nullptr);
            ok = (err == ERROR_SUCCESS);
        }
        LocalFree(pSd);
        return ok;
    }
#endif

private:
    QTemporaryDir m_tempDir;
    QStringList m_trackedPaths;
};

inline AclAuditor::Engine::SidInfo makeSid(
    const std::wstring& sid,
    const std::wstring& account,
    const std::wstring& domain,
    AclAuditor::Engine::PrincipalCategory category) {
    AclAuditor::Engine::SidInfo info;
    info.sidString = sid;
    info.accountName = account;
    info.domainName = domain;
    info.category = category;
    info.isOrphaned = (category == AclAuditor::Engine::PrincipalCategory::OrphanedSid);
    info.isWellKnown = (category == AclAuditor::Engine::PrincipalCategory::WellKnown ||
                        category == AclAuditor::Engine::PrincipalCategory::ServiceAccount);
    return info;
}

inline AclAuditor::Engine::AceEntry makeAce(
    const std::wstring& sid,
    const std::wstring& account,
    const std::wstring& domain,
    AclAuditor::Engine::PrincipalCategory category,
    AclAuditor::Engine::AccessType type,
    DWORD accessMask,
    bool isInherited = false,
    const std::wstring& inheritedFrom = L"") {
    AclAuditor::Engine::AceEntry ace;
    ace.trustee = makeSid(sid, account, domain, category);
    ace.type = type;
    ace.accessMask = accessMask;
    ace.isInherited = isInherited;
    ace.inheritedFromPath = inheritedFrom;
    ace.fullControl = ((accessMask & FILE_ALL_ACCESS) == FILE_ALL_ACCESS);
    ace.modify = ace.fullControl || (((accessMask & (DELETE | FILE_WRITE_DATA)) == (DELETE | FILE_WRITE_DATA)));
    ace.write = ace.fullControl || ((accessMask & (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_GENERIC_WRITE)) != 0);
    ace.readAndExecute = ace.fullControl || ((accessMask & (FILE_EXECUTE | FILE_GENERIC_READ)) == (FILE_EXECUTE | FILE_GENERIC_READ));
    ace.read = ace.fullControl || ((accessMask & (FILE_READ_DATA | FILE_GENERIC_READ)) != 0);
    ace.canReadData = ace.read;
    ace.canWriteData = ace.write;
    ace.canExecute = ace.readAndExecute;
    ace.canDelete = ace.modify;
    ace.canWriteDac = ((accessMask & WRITE_DAC) != 0);
    ace.canWriteOwner = ((accessMask & WRITE_OWNER) != 0);
    ace.appliesToCurrent = true;
    ace.appliesToSubfolders = true;
    ace.appliesToFiles = true;
    return ace;
}

inline AclAuditor::Engine::FileSecurityNode makeSampleTree(const std::wstring& rootPath = L"C:\\TestAuditRoot") {
    using namespace AclAuditor::Engine;

    FileSecurityNode root;
    root.path = rootPath;
    root.fileName = L"TestAuditRoot";
    root.isDirectory = true;
    root.security.hasDacl = true;
    root.security.isInheritanceBlocked = true;
    root.security.owner = makeSid(L"S-1-5-32-544", L"Administrators", L"BUILTIN", PrincipalCategory::LocalGroup);
    root.security.group = makeSid(L"S-1-5-18", L"SYSTEM", L"NT AUTHORITY", PrincipalCategory::ServiceAccount);
    root.security.dacl.push_back(makeAce(
        L"S-1-5-32-544", L"Administrators", L"BUILTIN", PrincipalCategory::LocalGroup,
        AccessType::Allow, FILE_ALL_ACCESS, false));
    root.security.dacl.push_back(makeAce(
        L"S-1-1-0", L"Everyone", L"", PrincipalCategory::WellKnown,
        AccessType::Allow, FILE_GENERIC_READ | FILE_GENERIC_WRITE, false));

    FileSecurityNode childScript;
    childScript.path = rootPath + L"\\deploy.ps1";
    childScript.fileName = L"deploy.ps1";
    childScript.isDirectory = false;
    childScript.security.hasDacl = true;
    childScript.security.owner = makeSid(L"S-1-5-32-544", L"Administrators", L"BUILTIN", PrincipalCategory::LocalGroup);
    childScript.security.dacl.push_back(makeAce(
        L"S-1-5-32-545", L"Users", L"BUILTIN", PrincipalCategory::WellKnown,
        AccessType::Allow, FILE_ALL_ACCESS, true, rootPath));

    FileSecurityNode childSecret;
    childSecret.path = rootPath + L"\\secrets.env";
    childSecret.fileName = L"secrets.env";
    childSecret.isDirectory = false;
    childSecret.security.hasDacl = true;
    childSecret.security.owner = makeSid(L"S-1-5-32-544", L"Administrators", L"BUILTIN", PrincipalCategory::LocalGroup);
    childSecret.security.dacl.push_back(makeAce(
        L"S-1-5-11", L"Authenticated Users", L"NT AUTHORITY", PrincipalCategory::WellKnown,
        AccessType::Allow, FILE_GENERIC_READ, true, rootPath));

    root.children.push_back(childScript);
    root.children.push_back(childSecret);
    return root;
}

} // namespace AclTestSupport
