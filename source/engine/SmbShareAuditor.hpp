#pragma once

#include "AclTypes.hpp"
#include <string>
#include <vector>

namespace AclAuditor::Engine {

class SmbShareAuditor {
public:
    // Enumerates all local SMB shares and checks both share-level and NTFS permissions
    static std::vector<SmbShareFinding> auditShares();

    // Inspects a single share
    static SmbShareFinding auditSingleShare(const std::wstring& shareName);

private:
    static bool isAdministrativeShare(const std::wstring& shareName);
};

} // namespace AclAuditor::Engine
