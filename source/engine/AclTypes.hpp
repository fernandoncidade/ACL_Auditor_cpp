#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace AclAuditor::Engine {

enum class AccessType {
    Allow,
    Deny,
    Audit
};

enum class PrincipalCategory {
    WellKnown,      // Everyone, Authenticated Users, Users, etc.
    User,           // Local or Domain user account
    LocalGroup,     // Local security group (e.g. BUILTIN\Administrators)
    DomainGroup,    // Active Directory domain group
    ServiceAccount, // SYSTEM, LOCAL SERVICE, NETWORK SERVICE
    OrphanedSid,    // Unresolved / deleted account SID (Security Hazard)
    Unknown
};

enum class RiskSeverity {
    Critical,
    High,
    Medium,
    Low,
    Info
};

enum class RiskCategory {
    WorldWritable,          // Everyone / Users / Authenticated Users can write/modify
    BinaryPlanting,         // Executable or script located in user-writable folder
    ExploitableService,     // Service binary path writable or unquoted with spaces
    SmbExposure,            // SMB Share with excessively permissive DACL
    OrphanedPrincipal,      // Orphaned / Deleted SID in ACL
    BrokenInheritance,      // Inheritance blocked (SE_DACL_PROTECTED) causing shadow rules
    ExcessivePrivilege,     // Standard user has WRITE_DAC or WRITE_OWNER
    InconsistentPermission  // Conflicting or redundant ACEs
};

struct SidInfo {
    std::wstring sidString;         // e.g. "S-1-1-0"
    std::wstring accountName;       // e.g. "Everyone" or "Administrador"
    std::wstring domainName;        // e.g. "NT AUTHORITY" or "WORKSPACE"
    PrincipalCategory category{PrincipalCategory::Unknown};
    bool isOrphaned{false};
    bool isWellKnown{false};

    std::wstring getDisplayName() const {
        if (isOrphaned) {
            return L"[Conta Órfã] " + sidString;
        }
        if (domainName.empty()) {
            return accountName.empty() ? sidString : accountName;
        }
        return domainName + L"\\" + accountName;
    }
};

struct AceEntry {
    int index{0};
    SidInfo trustee;
    AccessType type{AccessType::Allow};
    DWORD accessMask{0};
    DWORD aceFlags{0};

    // Interpreted permission flags
    bool fullControl{false};
    bool modify{false};
    bool readAndExecute{false};
    bool read{false};
    bool write{false};
    bool special{false};

    // Specific granular rights
    bool canReadData{false};
    bool canWriteData{false};
    bool canAppendData{false};
    bool canExecute{false};
    bool canDeleteChild{false};
    bool canDelete{false};
    bool canReadAttributes{false};
    bool canWriteAttributes{false};
    bool canReadEA{false};
    bool canWriteEA{false};
    bool canReadPermissions{false};
    bool canWriteDac{false};
    bool canWriteOwner{false};
    bool canSynchronize{false};

    // Inheritance info
    bool isInherited{false};
    bool appliesToCurrent{true};
    bool appliesToSubfolders{false};
    bool appliesToFiles{false};
    bool inheritOnly{false};
    std::wstring inheritedFromPath; // Calculated via GetInheritanceSourceW

    std::wstring getRightsSummary() const {
        if (fullControl) return L"Controle Total";
        if (modify) return L"Modificar";
        if (readAndExecute && write) return L"Leitura, Execução e Gravação";
        if (readAndExecute) return L"Leitura e Execução";
        if (write) return L"Gravação";
        if (read) return L"Somente Leitura";
        return L"Permissões Especiais";
    }

    std::wstring getScopeDescription() const {
        if (isInherited) {
            if (inheritedFromPath.empty()) {
                return L"Herdado da pasta pai";
            }
            return L"Herdado de: " + inheritedFromPath;
        }
        if (appliesToCurrent && appliesToSubfolders && appliesToFiles) {
            return L"Esta pasta, subpastas e arquivos";
        }
        if (appliesToCurrent && appliesToSubfolders) {
            return L"Esta pasta e subpastas";
        }
        if (appliesToCurrent && appliesToFiles) {
            return L"Esta pasta e arquivos";
        }
        if (appliesToSubfolders && appliesToFiles) {
            return L"Apenas subpastas e arquivos";
        }
        return L"Apenas esta pasta";
    }
};

struct SecurityDescriptorInfo {
    SidInfo owner;
    SidInfo group;
    bool hasDacl{false};
    bool isDaclDefaulted{false};
    bool isInheritanceBlocked{false}; // SE_DACL_PROTECTED
    std::vector<AceEntry> dacl;
    std::vector<AceEntry> sacl;
};

struct RiskFinding {
    RiskSeverity severity{RiskSeverity::Medium};
    RiskCategory category{RiskCategory::WorldWritable};
    std::wstring title;
    std::wstring description;
    std::wstring affectedPath;
    std::wstring affectedPrincipal;
    std::wstring technicalDetail;
    std::wstring remediationAdvice;
    std::wstring icaclsCommand;
    std::wstring powershellCommand;

    static const wchar_t* severityToString(RiskSeverity s) {
        switch (s) {
            case RiskSeverity::Critical: return L"CRÍTICO";
            case RiskSeverity::High:     return L"ALTO";
            case RiskSeverity::Medium:   return L"MÉDIO";
            case RiskSeverity::Low:      return L"BAIXO";
            case RiskSeverity::Info:     return L"INFO";
        }
        return L"DESCONHECIDO";
    }
};

struct FileSecurityNode {
    std::wstring path;
    std::wstring fileName;
    bool isDirectory{false};
    uint64_t fileSize{0};
    FILETIME lastWriteTime{0, 0};
    SecurityDescriptorInfo security;
    int riskScore{0}; // 0 to 100
    std::vector<RiskFinding> risks;
    std::vector<FileSecurityNode> children;
};

// Explanatory chain node for the "Why does this user have access?" visual tree
struct EffectiveChainStep {
    std::wstring title;                 // e.g. "Usuário: joao.silva" or "Grupo: TI-Suporte"
    std::wstring relationship;          // e.g. "Membro direto", "Grupo aninhado em...", "Regra herdada de..."
    std::wstring originPath;            // e.g. "D:\Departamentos"
    std::wstring accessGranted;         // e.g. "Modificar (Gravar, Ler, Executar, Excluir)"
    AccessType accessType{AccessType::Allow};
    bool isInherited{false};
    bool isDecisive{false};             // This rule decided the final permission
    std::vector<EffectiveChainStep> children;
};

struct EffectiveAccessResult {
    std::wstring targetQuery;
    SidInfo targetPrincipal;
    DWORD effectiveMask{0};
    bool canFullControl{false};
    bool canModify{false};
    bool canReadAndExecute{false};
    bool canRead{false};
    bool canWrite{false};
    bool canDelete{false};
    bool canTakeOwnership{false};
    bool canChangePermissions{false};

    std::wstring finalVerdictSummary;
    std::vector<AceEntry> matchingAces;
    std::vector<AceEntry> conflictingDenyAces;
    std::vector<EffectiveChainStep> explanationTree;
};

struct ServiceSecurityFinding {
    std::wstring serviceName;
    std::wstring displayName;
    std::wstring binaryPath;
    std::wstring startType;
    std::wstring serviceAccount;
    bool isBinaryWritableByNonAdmin{false};
    bool isDirectoryWritableByNonAdmin{false};
    bool isUnquotedPathWithSpaces{false};
    bool isServiceAclPermissive{false};
    std::vector<RiskFinding> risks;
};

struct SmbShareFinding {
    std::wstring shareName;
    std::wstring localPath;
    std::wstring comment;
    DWORD shareType{0};
    bool isEveryoneAllowed{false};
    bool isEveryoneFullControl{false};
    bool isNtfsMismatch{false};
    std::wstring shareDaclSummary;
    std::wstring ntfsDaclSummary;
    std::vector<AceEntry> shareAcl;
    std::vector<RiskFinding> risks;
};

struct SnapshotItemRecord {
    std::wstring path;
    bool isDirectory{false};
    std::wstring owner;
    bool inheritanceBlocked{false};
    std::vector<AceEntry> aces;
    int riskScore{0};
};

struct SnapshotRecord {
    std::wstring id;
    std::wstring timestampIso;
    std::wstring rootPath;
    std::wstring title;
    size_t totalFiles{0};
    size_t totalDirectories{0};
    size_t totalRisks{0};
    std::string sha256Hash;
    std::vector<SnapshotItemRecord> items;
};

enum class DiffChangeType {
    ObjectAdded,
    ObjectRemoved,
    OwnerChanged,
    InheritanceChanged,
    AceAdded,
    AceRemoved,
    RightsEscalated,
    RightsReduced,
    RiskIncreased,
    RiskDecreased
};

struct DiffEntry {
    std::wstring path;
    bool isDirectory{false};
    DiffChangeType changeType{DiffChangeType::AceAdded};
    std::wstring summary;
    std::wstring oldValue;
    std::wstring newValue;
    RiskSeverity severityDelta{RiskSeverity::Info};
};

struct SnapshotDiffResult {
    std::wstring baselineId;
    std::wstring currentId;
    std::wstring baselineTimestamp;
    std::wstring currentTimestamp;
    size_t totalChanges{0};
    size_t totalNewRisks{0};
    std::vector<DiffEntry> changes;
};

} // namespace AclAuditor::Engine
