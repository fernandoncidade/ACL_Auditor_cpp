#include "source/engine/AclTypes.hpp"
#include "source/engine/NtfsSecurityUtils.hpp"
#include "source/engine/EffectiveAccessCalculator.hpp"
#include "source/engine/RiskScanner.hpp"
#include "source/engine/ServiceAuditor.hpp"
#include "source/engine/SmbShareAuditor.hpp"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

using namespace AclAuditor::Engine;

static std::string toUtf8(const std::wstring& wstr) {
    return QString::fromStdWString(wstr).toUtf8().toStdString();
}

static void printBanner() {
    std::cout << "===============================================================\n";
    std::cout << "  ACL Auditor CLI — Windows NTFS & SMB Security Auditor       \n";
    std::cout << "  Version 2026.11.6.0 | C++20 Win32 Security Engine          \n";
    std::cout << "===============================================================\n\n";
}

static void printHelp() {
    printBanner();
    std::cout << "Usage:\n";
    std::cout << "  acl-auditor-cli.exe <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  scan <path>              Scan directory for NTFS permissions and risks\n";
    std::cout << "  access <path> --user <u> Calculate effective access for user/group on path\n";
    std::cout << "  services                 Audit Windows services for unquoted paths & weak DACLs\n";
    std::cout << "  shares                   Audit SMB network shares and compare with NTFS\n";
    std::cout << "  --version, -v            Show official version\n";
    std::cout << "  --help, -h               Show this help message\n\n";
    std::cout << "Options:\n";
    std::cout << "  --format <text|json>     Output format (default: text)\n";
    std::cout << "  --depth <n>              Scan depth for directory tree (default: 2)\n";
    std::cout << "  --risks-only             Only output detected security vulnerabilities\n";
    std::cout << "  --no-banner              Suppress banner in text mode\n\n";
    std::cout << "Examples:\n";
    std::cout << "  acl-auditor-cli.exe scan \"C:\\inetpub\\wwwroot\" --format json\n";
    std::cout << "  acl-auditor-cli.exe access \"D:\\Finance\\Report.xlsx\" --user \"Everyone\"\n";
    std::cout << "  acl-auditor-cli.exe services --risks-only\n";
}

static void outputTreeJson(const FileSecurityNode& node, QJsonObject& obj) {
    obj[QStringLiteral("path")] = QString::fromStdWString(node.path);
    obj[QStringLiteral("fileName")] = QString::fromStdWString(node.fileName);
    obj[QStringLiteral("isDirectory")] = node.isDirectory;
    obj[QStringLiteral("owner")] = QString::fromStdWString(node.security.owner.getDisplayName());
    obj[QStringLiteral("ownerSid")] = QString::fromStdWString(node.security.owner.sidString);
    obj[QStringLiteral("inheritanceBlocked")] = node.security.isInheritanceBlocked;
    obj[QStringLiteral("riskScore")] = node.riskScore;

    QJsonArray daclArr;
    for (const auto& ace : node.security.dacl) {
        QJsonObject aceObj;
        aceObj[QStringLiteral("trustee")] = QString::fromStdWString(ace.trustee.getDisplayName());
        aceObj[QStringLiteral("trusteeSid")] = QString::fromStdWString(ace.trustee.sidString);
        aceObj[QStringLiteral("type")] = (ace.type == AccessType::Allow) ? QStringLiteral("Allow") : QStringLiteral("Deny");
        aceObj[QStringLiteral("rights")] = QString::fromStdWString(ace.getRightsSummary());
        aceObj[QStringLiteral("inherited")] = ace.isInherited;
        aceObj[QStringLiteral("inheritedFrom")] = QString::fromStdWString(ace.inheritedFromPath);
        daclArr.append(aceObj);
    }
    obj[QStringLiteral("dacl")] = daclArr;

    QJsonArray risksArr;
    for (const auto& r : node.risks) {
        QJsonObject rObj;
        rObj[QStringLiteral("severity")] = QString::fromStdWString(RiskFinding::severityToString(r.severity));
        rObj[QStringLiteral("title")] = QString::fromStdWString(r.title);
        rObj[QStringLiteral("description")] = QString::fromStdWString(r.description);
        rObj[QStringLiteral("affectedTrustee")] = QString::fromStdWString(r.affectedPrincipal);
        rObj[QStringLiteral("remediationIcacls")] = QString::fromStdWString(r.icaclsCommand);
        risksArr.append(rObj);
    }
    obj[QStringLiteral("risks")] = risksArr;

    if (!node.children.empty()) {
        QJsonArray childArr;
        for (const auto& child : node.children) {
            QJsonObject childObj;
            outputTreeJson(child, childObj);
            childArr.append(childObj);
        }
        obj[QStringLiteral("children")] = childArr;
    }
}

static int handleScan(const std::wstring& targetPath, int depth, bool jsonFormat, bool risksOnly, bool noBanner) {
    NtfsSecurityUtils::enableSecurityPrivileges();

    if (!noBanner && !jsonFormat) {
        printBanner();
        std::cout << "[*] Scanning target: " << toUtf8(targetPath) << " (Depth: " << depth << ")...\n";
    }

    FileSecurityNode rootNode = NtfsSecurityUtils::scanDirectoryTree(targetPath, depth, true, nullptr);
    auto allRisks = RiskScanner::evaluateTreeRisks(rootNode);
    int overallScore = RiskScanner::calculateRiskScore(allRisks);

    if (jsonFormat) {
        QJsonObject rootObj;
        rootObj[QStringLiteral("targetPath")] = QString::fromStdWString(targetPath);
        rootObj[QStringLiteral("overallRiskScore")] = overallScore;
        rootObj[QStringLiteral("totalRisksFound")] = static_cast<int>(allRisks.size());

        QJsonArray risksArr;
        for (const auto& r : allRisks) {
            QJsonObject rObj;
            rObj[QStringLiteral("severity")] = QString::fromStdWString(RiskFinding::severityToString(r.severity));
            rObj[QStringLiteral("title")] = QString::fromStdWString(r.title);
            rObj[QStringLiteral("path")] = QString::fromStdWString(r.affectedPath);
            rObj[QStringLiteral("trustee")] = QString::fromStdWString(r.affectedPrincipal);
            rObj[QStringLiteral("remediationIcacls")] = QString::fromStdWString(r.icaclsCommand);
            rObj[QStringLiteral("remediationPowershell")] = QString::fromStdWString(r.powershellCommand);
            risksArr.append(rObj);
        }
        rootObj[QStringLiteral("risks")] = risksArr;

        if (!risksOnly) {
            QJsonObject treeObj;
            outputTreeJson(rootNode, treeObj);
            rootObj[QStringLiteral("tree")] = treeObj;
        }

        QJsonDocument doc(rootObj);
        std::cout << doc.toJson(QJsonDocument::Indented).toStdString() << std::endl;
        return (overallScore >= 70) ? 1 : 0;
    }

    std::cout << "[+] Scan completed successfully.\n";
    std::cout << "    - Target Path: " << toUtf8(targetPath) << "\n";
    std::cout << "    - Owner: " << toUtf8(rootNode.security.owner.getDisplayName()) << " (" << toUtf8(rootNode.security.owner.sidString) << ")\n";
    std::cout << "    - Inheritance Blocked: " << (rootNode.security.isInheritanceBlocked ? "YES" : "NO") << "\n";
    std::cout << "    - Total DACL ACE Entries: " << rootNode.security.dacl.size() << "\n";
    std::cout << "    - Total Risks Identified: " << allRisks.size() << "\n";
    std::cout << "    - Overall Risk Score: " << overallScore << " / 100\n\n";

    if (!allRisks.empty()) {
        std::cout << "--- IDENTIFIED SECURITY RISKS ---\n";
        for (size_t i = 0; i < allRisks.size(); ++i) {
            const auto& r = allRisks[i];
            std::cout << "[" << (i + 1) << "] [" << toUtf8(RiskFinding::severityToString(r.severity)) << "] " << toUtf8(r.title) << "\n";
            std::cout << "    Path:    " << toUtf8(r.affectedPath) << "\n";
            if (!r.affectedPrincipal.empty()) {
                std::cout << "    Trustee: " << toUtf8(r.affectedPrincipal) << "\n";
            }
            std::cout << "    Details: " << toUtf8(r.description) << "\n";
            if (!r.icaclsCommand.empty()) {
                std::cout << "    Remediation (icacls): " << toUtf8(r.icaclsCommand) << "\n";
            }
            std::cout << "\n";
        }
    } else {
        std::cout << "[+] No high or critical security risks detected.\n";
    }

    return (overallScore >= 70) ? 1 : 0;
}

static int handleAccess(const std::wstring& targetPath, const std::wstring& user, bool jsonFormat, bool noBanner) {
    NtfsSecurityUtils::enableSecurityPrivileges();

    if (!noBanner && !jsonFormat) {
        printBanner();
        std::cout << "[*] Calculating effective access on: " << toUtf8(targetPath) << " for user: " << toUtf8(user) << "...\n\n";
    }

    EffectiveAccessResult result = EffectiveAccessCalculator::calculateEffectiveAccess(targetPath, user);

    if (jsonFormat) {
        QJsonObject obj;
        obj[QStringLiteral("targetPath")] = QString::fromStdWString(targetPath);
        obj[QStringLiteral("userQuery")] = QString::fromStdWString(user);
        obj[QStringLiteral("resolvedAccount")] = QString::fromStdWString(result.targetPrincipal.getDisplayName());
        obj[QStringLiteral("resolvedSid")] = QString::fromStdWString(result.targetPrincipal.sidString);

        QJsonObject rightsObj;
        rightsObj[QStringLiteral("canRead")] = result.canRead;
        rightsObj[QStringLiteral("canWrite")] = result.canWrite;
        rightsObj[QStringLiteral("canReadAndExecute")] = result.canReadAndExecute;
        rightsObj[QStringLiteral("canModify")] = result.canModify;
        rightsObj[QStringLiteral("canFullControl")] = result.canFullControl;
        rightsObj[QStringLiteral("canDelete")] = result.canDelete;
        rightsObj[QStringLiteral("canTakeOwnership")] = result.canTakeOwnership;
        rightsObj[QStringLiteral("canChangePermissions")] = result.canChangePermissions;
        obj[QStringLiteral("rights")] = rightsObj;

        obj[QStringLiteral("finalVerdict")] = QString::fromStdWString(result.finalVerdictSummary);

        QJsonDocument doc(obj);
        std::cout << doc.toJson(QJsonDocument::Indented).toStdString() << std::endl;
        return 0;
    }

    std::cout << "Target:   " << toUtf8(targetPath) << "\n";
    std::cout << "Trustee:  " << toUtf8(result.targetPrincipal.getDisplayName()) << " (" << toUtf8(result.targetPrincipal.sidString) << ")\n\n";
    std::cout << "Effective Access Rights:\n";
    std::cout << "  - Read:               " << (result.canRead ? "ALLOWED [YES]" : "DENIED [NO]") << "\n";
    std::cout << "  - Write:              " << (result.canWrite ? "ALLOWED [YES]" : "DENIED [NO]") << "\n";
    std::cout << "  - Read & Execute:     " << (result.canReadAndExecute ? "ALLOWED [YES]" : "DENIED [NO]") << "\n";
    std::cout << "  - Modify:             " << (result.canModify ? "ALLOWED [YES]" : "DENIED [NO]") << "\n";
    std::cout << "  - Full Control:       " << (result.canFullControl ? "ALLOWED [YES]" : "DENIED [NO]") << "\n";
    std::cout << "  - Change Permissions: " << (result.canChangePermissions ? "ALLOWED [YES]" : "DENIED [NO]") << "\n";
    std::cout << "  - Take Ownership:     " << (result.canTakeOwnership ? "ALLOWED [YES]" : "DENIED [NO]") << "\n\n";

    if (!result.finalVerdictSummary.empty()) {
        std::cout << "Verdict Explanation:\n  " << toUtf8(result.finalVerdictSummary) << "\n";
    }

    return 0;
}

static int handleServices(bool jsonFormat, bool risksOnly, bool noBanner) {
    NtfsSecurityUtils::enableSecurityPrivileges();

    if (!noBanner && !jsonFormat) {
        printBanner();
        std::cout << "[*] Auditing Windows Service Control Manager (SCM)...\n\n";
    }

    auto services = ServiceAuditor::auditServices();

    if (jsonFormat) {
        QJsonObject root;
        QJsonArray arr;
        for (const auto& s : services) {
            if (risksOnly && !s.isUnquotedPathWithSpaces && !s.isBinaryWritableByNonAdmin && s.risks.empty()) {
                continue;
            }
            QJsonObject sObj;
            sObj[QStringLiteral("serviceName")] = QString::fromStdWString(s.serviceName);
            sObj[QStringLiteral("displayName")] = QString::fromStdWString(s.displayName);
            sObj[QStringLiteral("binaryPath")] = QString::fromStdWString(s.binaryPath);
            sObj[QStringLiteral("serviceAccount")] = QString::fromStdWString(s.serviceAccount);
            sObj[QStringLiteral("isUnquotedPathWithSpaces")] = s.isUnquotedPathWithSpaces;
            sObj[QStringLiteral("isBinaryWritableByNonAdmin")] = s.isBinaryWritableByNonAdmin;
            arr.append(sObj);
        }
        root[QStringLiteral("services")] = arr;
        QJsonDocument doc(root);
        std::cout << doc.toJson(QJsonDocument::Indented).toStdString() << std::endl;
        return 0;
    }

    int riskCount = 0;
    for (const auto& s : services) {
        bool hasRisk = s.isUnquotedPathWithSpaces || s.isBinaryWritableByNonAdmin;
        if (risksOnly && !hasRisk) continue;

        if (hasRisk) riskCount++;
        std::cout << "Service: " << toUtf8(s.serviceName) << " (" << toUtf8(s.displayName) << ")\n";
        std::cout << "  Binary:   " << toUtf8(s.binaryPath) << "\n";
        std::cout << "  Account:  " << toUtf8(s.serviceAccount) << "\n";
        if (s.isUnquotedPathWithSpaces) {
            std::cout << "  [!] VULNERABILITY: Unquoted Service Path with spaces!\n";
        }
        if (s.isBinaryWritableByNonAdmin) {
            std::cout << "  [!] CRITICAL: Service binary writable by standard users!\n";
        }
        std::cout << "\n";
    }

    std::cout << "[+] Audited " << services.size() << " services. Vulnerable services found: " << riskCount << "\n";
    return (riskCount > 0) ? 1 : 0;
}

static int handleShares(bool jsonFormat, bool noBanner) {
    NtfsSecurityUtils::enableSecurityPrivileges();

    if (!noBanner && !jsonFormat) {
        printBanner();
        std::cout << "[*] Auditing local SMB network shares...\n\n";
    }

    auto shares = SmbShareAuditor::auditShares();

    if (jsonFormat) {
        QJsonObject root;
        QJsonArray arr;
        for (const auto& s : shares) {
            QJsonObject sObj;
            sObj[QStringLiteral("shareName")] = QString::fromStdWString(s.shareName);
            sObj[QStringLiteral("localPath")] = QString::fromStdWString(s.localPath);
            sObj[QStringLiteral("comment")] = QString::fromStdWString(s.comment);
            sObj[QStringLiteral("isEveryoneAllowed")] = s.isEveryoneAllowed;
            sObj[QStringLiteral("isEveryoneFullControl")] = s.isEveryoneFullControl;
            arr.append(sObj);
        }
        root[QStringLiteral("shares")] = arr;
        QJsonDocument doc(root);
        std::cout << doc.toJson(QJsonDocument::Indented).toStdString() << std::endl;
        return 0;
    }

    for (const auto& s : shares) {
        std::cout << "Share: " << toUtf8(s.shareName) << "\n";
        std::cout << "  Path:    " << toUtf8(s.localPath) << "\n";
        std::cout << "  Comment: " << toUtf8(s.comment) << "\n";
        if (s.isEveryoneFullControl) {
            std::cout << "  [!] WARNING: 'Everyone' has FULL CONTROL on this share!\n";
        } else if (s.isEveryoneAllowed) {
            std::cout << "  [*] 'Everyone' has access permissions.\n";
        }
        std::cout << "\n";
    }

    std::cout << "[+] Total SMB shares audited: " << shares.size() << "\n";
    return 0;
}

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ACLAuditorCLI"));
app.setApplicationVersion(QStringLiteral("2026.11.6.0"));

    if (argc < 2) {
        printHelp();
        return 0;
    }

    QString cmdArg = QString::fromLocal8Bit(argv[1]);
    if (cmdArg == QStringLiteral("--help") || cmdArg == QStringLiteral("-h") || cmdArg == QStringLiteral("/?") || cmdArg == QStringLiteral("help")) {
        printHelp();
        return 0;
    }

    if (cmdArg == QStringLiteral("--version") || cmdArg == QStringLiteral("-v") || cmdArg == QStringLiteral("version")) {
        std::cout << "ACL Auditor CLI v2026.11.6.0\n";
        return 0;
    }

    std::wstring targetPath;
    std::wstring targetUser;
    int depth = 2;
    bool jsonFormat = false;
    bool risksOnly = false;
    bool noBanner = false;

    for (int i = 2; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QStringLiteral("--format") && i + 1 < argc) {
            QString fmt = QString::fromLocal8Bit(argv[++i]).toLower();
            if (fmt == QStringLiteral("json")) jsonFormat = true;
        } else if (arg == QStringLiteral("--depth") && i + 1 < argc) {
            depth = QString::fromLocal8Bit(argv[++i]).toInt();
            if (depth < 0) depth = 0;
        } else if (arg == QStringLiteral("--user") && i + 1 < argc) {
            targetUser = QString::fromLocal8Bit(argv[++i]).toStdWString();
        } else if (arg == QStringLiteral("--risks-only")) {
            risksOnly = true;
        } else if (arg == QStringLiteral("--no-banner")) {
            noBanner = true;
        } else if (targetPath.empty() && !arg.startsWith(QStringLiteral("--"))) {
            targetPath = arg.toStdWString();
        }
    }

    if (cmdArg == QStringLiteral("scan")) {
        if (targetPath.empty()) {
            std::cerr << "[ERROR] Missing required <path> argument for 'scan' command.\n";
            std::cerr << "Run 'acl-auditor-cli.exe --help' for usage.\n";
            return 3;
        }
        return handleScan(targetPath, depth, jsonFormat, risksOnly, noBanner);
    } else if (cmdArg == QStringLiteral("access")) {
        if (targetPath.empty() || targetUser.empty()) {
            std::cerr << "[ERROR] Both <path> and --user <user> are required for 'access' command.\n";
            std::cerr << "Example: acl-auditor-cli.exe access \"C:\\Folder\" --user \"Everyone\"\n";
            return 3;
        }
        return handleAccess(targetPath, targetUser, jsonFormat, noBanner);
    } else if (cmdArg == QStringLiteral("services")) {
        return handleServices(jsonFormat, risksOnly, noBanner);
    } else if (cmdArg == QStringLiteral("shares")) {
        return handleShares(jsonFormat, noBanner);
    } else {
        std::cerr << "[ERROR] Unknown command: " << toUtf8(cmdArg.toStdWString()) << "\n";
        printHelp();
        return 3;
    }
}
