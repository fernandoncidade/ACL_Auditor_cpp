#pragma once

#include "AclTypes.hpp"
#include <string>
#include <vector>

namespace AclAuditor::Engine {

class ServiceAuditor {
public:
    // Enumerates all installed Windows services and evaluates their security configuration
    static std::vector<ServiceSecurityFinding> auditServices();

    // Inspects a single service by service name
    static ServiceSecurityFinding auditSingleService(const std::wstring& serviceName);

private:
    static std::wstring extractExecutablePath(const std::wstring& rawImagePath);
    static bool hasUnquotedSpaces(const std::wstring& rawImagePath);
    static bool isBinaryWritableByNonAdmin(const std::wstring& binaryPath);
};

} // namespace AclAuditor::Engine
