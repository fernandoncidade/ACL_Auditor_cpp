#pragma once

#include "AclTypes.hpp"
#include <string>
#include <vector>
#include <set>
#include <map>

namespace AclAuditor::Engine {

class EffectiveAccessCalculator {
public:
    // Resolves all direct, nested, and implicit group memberships for an account
    static std::vector<SidInfo> enumerateIdentityGroups(const std::wstring& accountOrSid);

    // Calculates the effective access mask and generates the full explanation tree
    static EffectiveAccessResult calculateEffectiveAccess(
        const std::wstring& objectPath,
        const std::wstring& targetUserOrGroup
    );

    // Generates a human-friendly paragraph explaining the verdict
    static std::wstring generateExplanatoryText(const EffectiveAccessResult& result);

    // Enumerates all local users on the system (for easy dropdown selection in GUI)
    static std::vector<std::wstring> enumerateSystemUsers();

    // Enumerates all local groups on the system
    static std::vector<std::wstring> enumerateSystemGroups();

private:
    static bool isSidInSet(const std::wstring& sid, const std::set<std::wstring>& sids);
    static EffectiveChainStep buildChainStepForAce(
        const AceEntry& ace,
        const SidInfo& targetPrincipal,
        const std::vector<SidInfo>& userGroups,
        const std::wstring& objectPath
    );
};

} // namespace AclAuditor::Engine
