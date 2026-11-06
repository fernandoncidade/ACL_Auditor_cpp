#pragma once

#include "AclTypes.hpp"
#include <string>
#include <vector>

namespace AclAuditor::Engine {

struct RemediationPlan {
    std::wstring title;
    std::wstring generatedTimestamp;
    size_t totalIssuesToRemediate{0};
    std::wstring backupScript;
    std::wstring icaclsScript;
    std::wstring powershellScript;
    std::wstring markdownReport;
    std::wstring rollbackScript;
};

class RemediationPlanner {
public:
    // Generates a comprehensive, safe remediation plan for a list of findings
    static RemediationPlan buildPlan(
        const std::wstring& rootPath,
        const std::vector<RiskFinding>& findings,
        bool includeRollbackSafety = true
    );

    // Exports the generated remediation plan to disk (.ps1 or .bat or .md)
    static bool exportScriptToFile(const std::wstring& scriptContent, const std::wstring& filePath);
};

} // namespace AclAuditor::Engine
