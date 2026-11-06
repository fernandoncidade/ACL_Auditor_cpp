#include "NtfsSecurityUtils.hpp"

#include <aclapi.h>
#include <sddl.h>
#include <accctrl.h>
#include <bcrypt.h>
#include <shlwapi.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <mutex>

#ifdef _MSC_VER
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "shlwapi.lib")
#endif

namespace AclAuditor::Engine {

// Thread-safe SID and Account resolution cache to eliminate repetitive Windows LSA/SAM RPC queries
static std::unordered_map<std::wstring, SidInfo> s_sidCache;
static std::mutex s_sidCacheMutex;

// Well-known SIDs
static const wchar_t* SID_EVERYONE = L"S-1-1-0";
static const wchar_t* SID_AUTHENTICATED_USERS = L"S-1-5-11";
static const wchar_t* SID_BUILTIN_USERS = L"S-1-5-32-545";
static const wchar_t* SID_BUILTIN_ADMINS = L"S-1-5-32-544";
static const wchar_t* SID_SYSTEM = L"S-1-5-18";
static const wchar_t* SID_LOCAL_SERVICE = L"S-1-5-19";
static const wchar_t* SID_NETWORK_SERVICE = L"S-1-5-20";
static const wchar_t* SID_ANONYMOUS = L"S-1-5-7";
static const wchar_t* SID_INTERACTIVE = L"S-1-5-4";

bool NtfsSecurityUtils::enableSecurityPrivileges() {
    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    const wchar_t* privilegesToEnable[] = {
        SE_BACKUP_NAME,
        SE_RESTORE_NAME,
        SE_SECURITY_NAME,
        SE_TAKE_OWNERSHIP_NAME
    };

    for (const auto* privName : privilegesToEnable) {
        LUID luid;
        if (LookupPrivilegeValueW(nullptr, privName, &luid)) {
            TOKEN_PRIVILEGES tp;
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr);
        }
    }

    CloseHandle(hToken);
    return true;
}

bool NtfsSecurityUtils::isProcessElevated() {
    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return false;
    }

    TOKEN_ELEVATION elevation;
    DWORD cbSize = sizeof(TOKEN_ELEVATION);
    bool isElevated = false;

    if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize)) {
        isElevated = (elevation.TokenIsElevated != 0);
    }

    CloseHandle(hToken);
    return isElevated;
}

SidInfo NtfsSecurityUtils::resolveSid(PSID pSid) {
    SidInfo info;
    if (!pSid || !IsValidSid(pSid)) {
        info.sidString = L"INVALID_SID";
        info.accountName = L"SID Inválido";
        info.category = PrincipalCategory::Unknown;
        return info;
    }

    LPWSTR pStrSid = nullptr;
    if (ConvertSidToStringSidW(pSid, &pStrSid)) {
        info.sidString = pStrSid;
        LocalFree(pStrSid);
    }

    // Check thread-safe cache
    {
        std::lock_guard<std::mutex> lock(s_sidCacheMutex);
        auto it = s_sidCache.find(info.sidString);
        if (it != s_sidCache.end()) {
            return it->second;
        }
    }

    // Identify well-known SIDs
    if (info.sidString == SID_EVERYONE) {
        info.accountName = L"Everyone (Todos)";
        info.domainName = L"";
        info.category = PrincipalCategory::WellKnown;
        info.isWellKnown = true;
    } else if (info.sidString == SID_AUTHENTICATED_USERS) {
        info.accountName = L"Authenticated Users (Usuários Autenticados)";
        info.domainName = L"NT AUTHORITY";
        info.category = PrincipalCategory::WellKnown;
        info.isWellKnown = true;
    } else if (info.sidString == SID_BUILTIN_USERS) {
        info.accountName = L"Users (Usuários)";
        info.domainName = L"BUILTIN";
        info.category = PrincipalCategory::LocalGroup;
        info.isWellKnown = true;
    } else if (info.sidString == SID_BUILTIN_ADMINS) {
        info.accountName = L"Administrators (Administradores)";
        info.domainName = L"BUILTIN";
        info.category = PrincipalCategory::LocalGroup;
        info.isWellKnown = true;
    } else if (info.sidString == SID_SYSTEM) {
        info.accountName = L"SYSTEM";
        info.domainName = L"NT AUTHORITY";
        info.category = PrincipalCategory::ServiceAccount;
        info.isWellKnown = true;
    } else if (info.sidString == SID_LOCAL_SERVICE) {
        info.accountName = L"LOCAL SERVICE";
        info.domainName = L"NT AUTHORITY";
        info.category = PrincipalCategory::ServiceAccount;
        info.isWellKnown = true;
    } else if (info.sidString == SID_NETWORK_SERVICE) {
        info.accountName = L"NETWORK SERVICE";
        info.domainName = L"NT AUTHORITY";
        info.category = PrincipalCategory::ServiceAccount;
        info.isWellKnown = true;
    } else if (info.sidString == SID_ANONYMOUS) {
        info.accountName = L"ANONYMOUS LOGON";
        info.domainName = L"NT AUTHORITY";
        info.category = PrincipalCategory::WellKnown;
        info.isWellKnown = true;
    } else if (info.sidString == SID_INTERACTIVE) {
        info.accountName = L"INTERACTIVE";
        info.domainName = L"NT AUTHORITY";
        info.category = PrincipalCategory::WellKnown;
        info.isWellKnown = true;
    } else {
        // Query domain and account name from Windows LSA
        wchar_t nameBuffer[256];
        wchar_t domainBuffer[256];
        DWORD nameLen = 256;
        DWORD domainLen = 256;
        SID_NAME_USE sidUse = SidTypeUnknown;

        BOOL ok = LookupAccountSidW(nullptr, pSid, nameBuffer, &nameLen, domainBuffer, &domainLen, &sidUse);
        if (ok) {
            info.accountName = nameBuffer;
            info.domainName = domainBuffer;
            info.isOrphaned = false;

            switch (sidUse) {
                case SidTypeUser:
                    info.category = PrincipalCategory::User;
                    break;
                case SidTypeGroup:
                    info.category = PrincipalCategory::LocalGroup;
                    break;
                case SidTypeDomain:
                case SidTypeAlias:
                    info.category = PrincipalCategory::LocalGroup;
                    break;
                case SidTypeWellKnownGroup:
                    info.category = PrincipalCategory::WellKnown;
                    info.isWellKnown = true;
                    break;
                case SidTypeDeletedAccount:
                    info.category = PrincipalCategory::OrphanedSid;
                    info.isOrphaned = true;
                    break;
                default:
                    info.category = PrincipalCategory::Unknown;
                    break;
            }
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_NONE_MAPPED) {
                info.accountName = L"[Conta Órfã / Deletada]";
                info.category = PrincipalCategory::OrphanedSid;
                info.isOrphaned = true;
            } else {
                info.accountName = info.sidString;
                info.category = PrincipalCategory::Unknown;
            }
        }
    }

    // Cache the resolved info
    {
        std::lock_guard<std::mutex> lock(s_sidCacheMutex);
        s_sidCache[info.sidString] = info;
    }
    return info;
}

SidInfo NtfsSecurityUtils::resolveAccountOrSidString(const std::wstring& accountOrSid) {
    // Check thread-safe cache first
    {
        std::lock_guard<std::mutex> lock(s_sidCacheMutex);
        auto it = s_sidCache.find(accountOrSid);
        if (it != s_sidCache.end()) {
            return it->second;
        }
    }

    SidInfo result;
    if (accountOrSid.rfind(L"S-1-", 0) == 0) {
        PSID pSid = nullptr;
        if (ConvertStringSidToSidW(accountOrSid.c_str(), &pSid)) {
            result = resolveSid(pSid);
            LocalFree(pSid);
            std::lock_guard<std::mutex> lock(s_sidCacheMutex);
            s_sidCache[accountOrSid] = result;
            return result;
        }
    }

    // Try LookupAccountNameW
    wchar_t sidBuffer[SECURITY_MAX_SID_SIZE];
    DWORD sidLen = SECURITY_MAX_SID_SIZE;
    wchar_t refDomain[256];
    DWORD refDomainLen = 256;
    SID_NAME_USE use = SidTypeUnknown;

    if (LookupAccountNameW(nullptr, accountOrSid.c_str(), (PSID)sidBuffer, &sidLen, refDomain, &refDomainLen, &use)) {
        result = resolveSid((PSID)sidBuffer);
        std::lock_guard<std::mutex> lock(s_sidCacheMutex);
        s_sidCache[accountOrSid] = result;
        return result;
    }

    SidInfo fallback;
    fallback.accountName = accountOrSid;
    fallback.sidString = accountOrSid;
    fallback.category = PrincipalCategory::Unknown;

    std::lock_guard<std::mutex> lock(s_sidCacheMutex);
    s_sidCache[accountOrSid] = fallback;
    return fallback;
}

std::vector<std::wstring> NtfsSecurityUtils::getInheritanceOrigins(const std::wstring& objectPath, PACL pAcl) {
    std::vector<std::wstring> origins;
    if (!pAcl) return origins;

    // GetInheritanceSourceW resolves origin paths for inherited ACEs
    PINHERITED_FROMW pInheritedFrom = (PINHERITED_FROMW)LocalAlloc(LPTR, pAcl->AceCount * sizeof(INHERITED_FROMW));
    if (!pInheritedFrom) return origins;

    DWORD res = GetInheritanceSourceW(
        const_cast<LPWSTR>(objectPath.c_str()),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        TRUE, // Container
        nullptr,
        0,
        pAcl,
        nullptr,
        nullptr,
        pInheritedFrom
    );

    if (res == ERROR_SUCCESS) {
        for (WORD i = 0; i < pAcl->AceCount; ++i) {
            if (pInheritedFrom[i].AncestorName) {
                origins.push_back(pInheritedFrom[i].AncestorName);
            } else {
                origins.push_back(L"");
            }
        }
        FreeInheritedFromArray(pInheritedFrom, pAcl->AceCount, nullptr);
    } else {
        LocalFree(pInheritedFrom);
    }

    return origins;
}

std::vector<AceEntry> NtfsSecurityUtils::parseAcl(PACL pAcl, const std::wstring& objectPath, bool isSacl) {
    (void)isSacl;
    std::vector<AceEntry> entries;
    if (!pAcl) return entries;

    std::vector<std::wstring> inheritanceOrigins = getInheritanceOrigins(objectPath, pAcl);

    ACL_SIZE_INFORMATION sizeInfo;
    if (!GetAclInformation(pAcl, &sizeInfo, sizeof(sizeInfo), AclSizeInformation)) {
        return entries;
    }

    for (DWORD i = 0; i < sizeInfo.AceCount; ++i) {
        LPVOID pAce = nullptr;
        if (!GetAce(pAcl, i, &pAce)) continue;

        ACE_HEADER* pHeader = reinterpret_cast<ACE_HEADER*>(pAce);
        AceEntry entry;
        entry.index = static_cast<int>(i);
        entry.aceFlags = pHeader->AceFlags;

        PSID pSid = nullptr;
        DWORD mask = 0;

        if (pHeader->AceType == ACCESS_ALLOWED_ACE_TYPE) {
            ACCESS_ALLOWED_ACE* pAllowed = reinterpret_cast<ACCESS_ALLOWED_ACE*>(pAce);
            mask = pAllowed->Mask;
            pSid = reinterpret_cast<PSID>(&pAllowed->SidStart);
            entry.type = AccessType::Allow;
        } else if (pHeader->AceType == ACCESS_DENIED_ACE_TYPE) {
            ACCESS_DENIED_ACE* pDenied = reinterpret_cast<ACCESS_DENIED_ACE*>(pAce);
            mask = pDenied->Mask;
            pSid = reinterpret_cast<PSID>(&pDenied->SidStart);
            entry.type = AccessType::Deny;
        } else if (pHeader->AceType == SYSTEM_AUDIT_ACE_TYPE) {
            SYSTEM_AUDIT_ACE* pAudit = reinterpret_cast<SYSTEM_AUDIT_ACE*>(pAce);
            mask = pAudit->Mask;
            pSid = reinterpret_cast<PSID>(&pAudit->SidStart);
            entry.type = AccessType::Audit;
        } else {
            // Callback or other ACE type
            continue;
        }

        entry.accessMask = mask;
        entry.trustee = resolveSid(pSid);

        // Inheritance flags
        entry.isInherited = (pHeader->AceFlags & INHERITED_ACE) != 0;
        if (i < inheritanceOrigins.size() && !inheritanceOrigins[i].empty()) {
            entry.inheritedFromPath = inheritanceOrigins[i];
        }

        bool oi = (pHeader->AceFlags & OBJECT_INHERIT_ACE) != 0;
        bool ci = (pHeader->AceFlags & CONTAINER_INHERIT_ACE) != 0;
        bool io = (pHeader->AceFlags & INHERIT_ONLY_ACE) != 0;

        entry.appliesToCurrent = !io;
        entry.appliesToFiles = oi;
        entry.appliesToSubfolders = ci;
        entry.inheritOnly = io;

        // Interpret granular rights
        entry.canReadData = (mask & FILE_READ_DATA) != 0;
        entry.canWriteData = (mask & FILE_WRITE_DATA) != 0;
        entry.canAppendData = (mask & FILE_APPEND_DATA) != 0;
        entry.canExecute = (mask & FILE_EXECUTE) != 0;
        entry.canDeleteChild = (mask & FILE_DELETE_CHILD) != 0;
        entry.canDelete = (mask & DELETE) != 0;
        entry.canReadAttributes = (mask & FILE_READ_ATTRIBUTES) != 0;
        entry.canWriteAttributes = (mask & FILE_WRITE_ATTRIBUTES) != 0;
        entry.canReadEA = (mask & FILE_READ_EA) != 0;
        entry.canWriteEA = (mask & FILE_WRITE_EA) != 0;
        entry.canReadPermissions = (mask & READ_CONTROL) != 0;
        entry.canWriteDac = (mask & WRITE_DAC) != 0;
        entry.canWriteOwner = (mask & WRITE_OWNER) != 0;
        entry.canSynchronize = (mask & SYNCHRONIZE) != 0;

        // Broad categories
        if ((mask & FILE_ALL_ACCESS) == FILE_ALL_ACCESS || (mask & GENERIC_ALL) == GENERIC_ALL) {
            entry.fullControl = true;
            entry.modify = true;
            entry.readAndExecute = true;
            entry.read = true;
            entry.write = true;
        } else {
            DWORD modifyMask = FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | DELETE;
            if ((mask & modifyMask) == modifyMask) {
                entry.modify = true;
            }
            if ((mask & FILE_GENERIC_READ) == FILE_GENERIC_READ) {
                entry.read = true;
            }
            if ((mask & FILE_GENERIC_EXECUTE) == FILE_GENERIC_EXECUTE) {
                entry.readAndExecute = true;
            }
            if ((mask & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE ||
                ((mask & FILE_WRITE_DATA) && (mask & FILE_APPEND_DATA))) {
                entry.write = true;
            }
            if (!entry.fullControl && !entry.modify && !entry.readAndExecute && !entry.read && !entry.write) {
                entry.special = true;
            }
        }

        entries.push_back(entry);
    }

    return entries;
}

bool NtfsSecurityUtils::getFileSecurityInfo(const std::wstring& path, SecurityDescriptorInfo& outInfo) {
    PSECURITY_DESCRIPTOR pSD = nullptr;
    PSID pOwner = nullptr;
    PSID pGroup = nullptr;
    PACL pDacl = nullptr;
    PACL pSacl = nullptr;

    SECURITY_INFORMATION secInfo = OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION |
                                   PROTECTED_DACL_SECURITY_INFORMATION;

    DWORD res = GetNamedSecurityInfoW(
        path.c_str(),
        SE_FILE_OBJECT,
        secInfo,
        &pOwner,
        &pGroup,
        &pDacl,
        &pSacl,
        &pSD
    );

    if (res != ERROR_SUCCESS || !pSD) {
        return false;
    }

    if (pOwner) {
        outInfo.owner = resolveSid(pOwner);
    }
    if (pGroup) {
        outInfo.group = resolveSid(pGroup);
    }

    // Inspect control flags (SE_DACL_PROTECTED = inheritance blocked)
    SECURITY_DESCRIPTOR_CONTROL control = 0;
    DWORD revision = 0;
    if (GetSecurityDescriptorControl(pSD, &control, &revision)) {
        outInfo.isInheritanceBlocked = (control & SE_DACL_PROTECTED) != 0;
    }

    outInfo.hasDacl = (pDacl != nullptr);
    if (pDacl) {
        outInfo.dacl = parseAcl(pDacl, path, false);
    }

    LocalFree(pSD);
    return true;
}

FileSecurityNode NtfsSecurityUtils::scanDirectoryTree(
    const std::wstring& rootPath,
    int maxDepth,
    bool recurse,
    std::function<void(const std::wstring& currentPath, size_t count)> progressCallback
) {
    static size_t scanCounter = 0;
    if (maxDepth == 0) scanCounter = 0;

    std::wstring normalized = normalizePath(rootPath);
    FileSecurityNode node;
    node.path = normalized;

    size_t lastSlash = normalized.find_last_of(L"\\/");
    node.fileName = (lastSlash != std::wstring::npos) ? normalized.substr(lastSlash + 1) : normalized;

    DWORD attrs = GetFileAttributesW(normalized.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        node.isDirectory = (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    // Fetch security
    getFileSecurityInfo(normalized, node.security);
    scanCounter++;

    if (progressCallback) {
        progressCallback(normalized, scanCounter);
    }

    if (!node.isDirectory || !recurse || maxDepth >= 10) {
        return node;
    }

    std::wstring searchPattern = normalized + L"\\*";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) {
                continue;
            }

            std::wstring childPath = normalized + L"\\" + findData.cFileName;
            bool isSubDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

            FileSecurityNode childNode;
            childNode.path = childPath;
            childNode.fileName = findData.cFileName;
            childNode.isDirectory = isSubDir;
            childNode.fileSize = (static_cast<uint64_t>(findData.nFileSizeHigh) << 32) | findData.nFileSizeLow;
            childNode.lastWriteTime = findData.ftLastWriteTime;

            getFileSecurityInfo(childPath, childNode.security);
            scanCounter++;

            if (progressCallback && (scanCounter % 10 == 0)) {
                progressCallback(childPath, scanCounter);
            }

            // Recurse into directories up to maxDepth limit
            if (isSubDir && maxDepth < 4) {
                FileSecurityNode subTree = scanDirectoryTree(childPath, maxDepth + 1, recurse, progressCallback);
                childNode.children = std::move(subTree.children);
            }

            node.children.push_back(std::move(childNode));

            // Protection against excessive memory during deep traversal
            if (node.children.size() > 5000) {
                break;
            }

        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    return node;
}

std::wstring NtfsSecurityUtils::formatAccessMaskHex(DWORD mask) {
    std::wstringstream ss;
    ss << L"0x" << std::uppercase << std::setfill(L'0') << std::setw(8) << std::hex << mask;
    return ss.str();
}

std::wstring NtfsSecurityUtils::formatAccessMaskDetailed(DWORD mask, bool isDirectory) {
    std::vector<std::wstring> rights;
    if ((mask & FILE_ALL_ACCESS) == FILE_ALL_ACCESS) {
        return L"Controle Total (Full Control)";
    }

    if (isDirectory) {
        if (mask & FILE_LIST_DIRECTORY)   rights.push_back(L"Listar Diretório / Ler Dados");
        if (mask & FILE_ADD_FILE)         rights.push_back(L"Criar Arquivos / Gravar Dados");
        if (mask & FILE_ADD_SUBDIRECTORY) rights.push_back(L"Criar Subpastas");
        if (mask & FILE_TRAVERSE)         rights.push_back(L"Atravessar Pastas / Executar");
        if (mask & FILE_DELETE_CHILD)     rights.push_back(L"Excluir Subpastas e Arquivos");
    } else {
        if (mask & FILE_READ_DATA)        rights.push_back(L"Ler Dados");
        if (mask & FILE_WRITE_DATA)       rights.push_back(L"Gravar Dados");
        if (mask & FILE_APPEND_DATA)      rights.push_back(L"Acrescentar Dados");
        if (mask & FILE_EXECUTE)          rights.push_back(L"Executar Arquivo");
    }

    if (mask & DELETE)                    rights.push_back(L"Excluir");
    if (mask & READ_CONTROL)              rights.push_back(L"Ler Permissões");
    if (mask & WRITE_DAC)                 rights.push_back(L"Alterar Permissões (WRITE_DAC)");
    if (mask & WRITE_OWNER)               rights.push_back(L"Apropriar-se (WRITE_OWNER)");
    if (mask & SYNCHRONIZE)               rights.push_back(L"Sincronizar");

    if (rights.empty()) {
        return formatAccessMaskHex(mask);
    }

    std::wstring result;
    for (size_t i = 0; i < rights.size(); ++i) {
        if (i > 0) result += L", ";
        result += rights[i];
    }
    return result;
}

std::string NtfsSecurityUtils::computeSha256Hex(const void* data, size_t size) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    std::string hexResult;

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) == 0) {
        DWORD cbHashObject = 0;
        DWORD cbData = 0;
        if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0) == 0) {
            std::vector<BYTE> hashObject(cbHashObject);
            if (BCryptCreateHash(hAlg, &hHash, hashObject.data(), cbHashObject, nullptr, 0, 0) == 0) {
                BCryptHashData(hHash, (PBYTE)data, static_cast<ULONG>(size), 0);
                BYTE hash[32];
                if (BCryptFinishHash(hHash, hash, 32, 0) == 0) {
                    std::stringstream ss;
                    for (int i = 0; i < 32; ++i) {
                        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
                    }
                    hexResult = ss.str();
                }
                BCryptDestroyHash(hHash);
            }
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }

    return hexResult;
}

std::string NtfsSecurityUtils::computeSha256Hex(const std::string& text) {
    return computeSha256Hex(text.data(), text.size());
}

std::wstring NtfsSecurityUtils::normalizePath(const std::wstring& path) {
    std::wstring p = path;
    std::replace(p.begin(), p.end(), L'/', L'\\');
    while (p.length() > 3 && (p.back() == L'\\' || p.back() == L'/')) {
        p.pop_back();
    }
    return p;
}

bool NtfsSecurityUtils::isSystemDirectory(const std::wstring& path) {
    wchar_t winDir[MAX_PATH];
    if (GetWindowsDirectoryW(winDir, MAX_PATH) > 0) {
        std::wstring w(winDir);
        if (_wcsnicmp(path.c_str(), w.c_str(), w.length()) == 0) {
            return true;
        }
    }

    wchar_t progFiles[MAX_PATH];
    if (GetEnvironmentVariableW(L"ProgramFiles", progFiles, MAX_PATH) > 0) {
        std::wstring pf(progFiles);
        if (_wcsnicmp(path.c_str(), pf.c_str(), pf.length()) == 0) {
            return true;
        }
    }

    return false;
}

} // namespace AclAuditor::Engine
