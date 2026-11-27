#pragma once

#include "AclTypes.hpp"
#include <string>
#include <vector>

namespace AclAuditor::Engine {

class RiskScanner {
public:
    // Analyzes a single node and returns all detected security vulnerabilities
    static std::vector<RiskFinding> evaluateNodeRisks(const FileSecurityNode& node);

    // Recursively scans an entire tree and aggregates all findings
    static std::vector<RiskFinding> evaluateTreeRisks(const FileSecurityNode& rootNode);

    // Calculates an overall security risk score (0 to 100, where 0 = safe, 100 = critical danger)
    static int calculateRiskScore(const std::vector<RiskFinding>& findings);

    // Checks if a file extension represents an executable or script
    static bool isExecutableOrScript(const std::wstring& path);

    // Checks if a folder name hints at confidential or sensitive organizational data
    static bool isSensitiveName(const std::wstring& name);

    // Formats remediation guidance and generates ready-to-run icacls / PowerShell commands
    static void generateRemediationCommands(RiskFinding& finding);

private:
    static bool isBroadGroup(const SidInfo& trustee);
    static bool isAdministrativePrincipal(const SidInfo& trustee);
};

} // namespace AclAuditor::Engine
