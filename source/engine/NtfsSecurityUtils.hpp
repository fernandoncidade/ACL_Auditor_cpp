#pragma once

#include "AclTypes.hpp"
#include <functional>
#include <string>
#include <vector>

namespace AclAuditor::Engine {

class NtfsSecurityUtils {
public:
    // Enables SeSecurityPrivilege, SeBackupPrivilege, etc. for the current process
    static bool enableSecurityPrivileges();

    // Checks if current process is running with elevated (Administrator) privileges
    static bool isProcessElevated();

    // Resolves a raw PSID into SidInfo (Account name, domain, category, orphaned check)
    static SidInfo resolveSid(PSID pSid);

    // Resolves an account or group name (or SID string) into a SidInfo
    static SidInfo resolveAccountOrSidString(const std::wstring& accountOrSid);

    // Retrieves complete security descriptor for a file, directory, or registry key
    static bool getFileSecurityInfo(const std::wstring& path, SecurityDescriptorInfo& outInfo);

    // Parses a native Win32 PACL into structured AceEntry objects
    static std::vector<AceEntry> parseAcl(PACL pAcl, const std::wstring& objectPath, bool isSacl);

    // Determines the origin folder for inherited ACEs using GetInheritanceSourceW
    static std::vector<std::wstring> getInheritanceOrigins(const std::wstring& objectPath, PACL pAcl);

    // Deep directory scanner that builds a FileSecurityNode tree
    static FileSecurityNode scanDirectoryTree(
        const std::wstring& rootPath,
        int maxDepth,
        bool recurse,
        std::function<void(const std::wstring& currentPath, size_t count)> progressCallback = nullptr
    );

    // Formats raw DWORD mask into detailed binary/hex strings
    static std::wstring formatAccessMaskHex(DWORD mask);
    static std::wstring formatAccessMaskDetailed(DWORD mask, bool isDirectory);

    // Generates SHA-256 hash using Windows CNG (BCrypt) for audit evidence signing
    static std::string computeSha256Hex(const void* data, size_t size);
    static std::string computeSha256Hex(const std::string& text);

    // Normalizes Windows path (e.g., C:/foo to C:\foo, removes trailing slashes)
    static std::wstring normalizePath(const std::wstring& path);

    // Checks if path represents a known critical system directory
    static bool isSystemDirectory(const std::wstring& path);
};

} // namespace AclAuditor::Engine
