#include "ServiceAuditor.hpp"
#include "NtfsSecurityUtils.hpp"
#include "RiskScanner.hpp"

#include <winsvc.h>
#include <algorithm>
#include <cwctype>

#ifdef _MSC_VER
#pragma comment(lib, "advapi32.lib")
#endif

namespace AclAuditor::Engine {

std::wstring ServiceAuditor::extractExecutablePath(const std::wstring& rawImagePath) {
    if (rawImagePath.empty()) return L"";

    std::wstring trimmed = rawImagePath;
    size_t firstNonSpace = trimmed.find_first_not_of(L" \t");
    if (firstNonSpace != std::wstring::npos) {
        trimmed = trimmed.substr(firstNonSpace);
    }

    if (trimmed.empty()) return L"";

    // If enclosed in quotes: e.g. "C:\Program Files\App\srv.exe" -arg
    if (trimmed.front() == L'\"') {
        size_t secondQuote = trimmed.find(L'\"', 1);
        if (secondQuote != std::wstring::npos) {
            return trimmed.substr(1, secondQuote - 1);
        }
    }

    // If unquoted, find the space before arguments or test existing files
    size_t spacePos = trimmed.find(L' ');
    if (spacePos != std::wstring::npos) {
        // Check if full path up to space exists
        std::wstring candidate = trimmed.substr(0, spacePos);
        if (GetFileAttributesW(candidate.c_str()) != INVALID_FILE_ATTRIBUTES) {
            return candidate;
        }
    }

    // Fallback: check if path ends with .exe
    size_t exePos = trimmed.rfind(L".exe");
    if (exePos != std::wstring::npos) {
        return trimmed.substr(0, exePos + 4);
    }

    return trimmed;
}

bool ServiceAuditor::hasUnquotedSpaces(const std::wstring& rawImagePath) {
    if (rawImagePath.empty()) return false;

    std::wstring trimmed = rawImagePath;
    size_t first = trimmed.find_first_not_of(L" \t");
    if (first != std::wstring::npos) trimmed = trimmed.substr(first);

    // If starts with quote, it is safely quoted
    if (trimmed.front() == L'\"') return false;

    // Check if it has spaces before .exe
    size_t exePos = trimmed.find(L".exe");
    if (exePos != std::wstring::npos) {
        std::wstring preExe = trimmed.substr(0, exePos);
        return preExe.find(L' ') != std::wstring::npos;
    }

    return false;
}

bool ServiceAuditor::isBinaryWritableByNonAdmin(const std::wstring& binaryPath) {
    if (binaryPath.empty()) return false;

    SecurityDescriptorInfo sec;
    if (!NtfsSecurityUtils::getFileSecurityInfo(binaryPath, sec)) {
        return false;
    }

    for (const auto& ace : sec.dacl) {
        if (ace.type == AccessType::Allow) {
            if (ace.trustee.sidString == L"S-1-1-0" || ace.trustee.sidString == L"S-1-5-32-545" || ace.trustee.sidString == L"S-1-5-11") {
                if (ace.canWriteData || ace.canAppendData || ace.modify || ace.fullControl || ace.write) {
                    return true;
                }
            }
        }
    }

    return false;
}

ServiceSecurityFinding ServiceAuditor::auditSingleService(const std::wstring& serviceName) {
    ServiceSecurityFinding finding;
    finding.serviceName = serviceName;

    SC_HANDLE hSCM = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCM) return finding;

    SC_HANDLE hService = OpenServiceW(hSCM, serviceName.c_str(), SERVICE_QUERY_CONFIG | READ_CONTROL);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return finding;
    }

    DWORD bytesNeeded = 0;
    QueryServiceConfigW(hService, nullptr, 0, &bytesNeeded);
    if (bytesNeeded > 0) {
        std::vector<BYTE> buffer(bytesNeeded);
        LPQUERY_SERVICE_CONFIGW pConfig = reinterpret_cast<LPQUERY_SERVICE_CONFIGW>(buffer.data());

        if (QueryServiceConfigW(hService, pConfig, bytesNeeded, &bytesNeeded)) {
            finding.displayName = pConfig->lpDisplayName ? pConfig->lpDisplayName : L"";
            finding.binaryPath = pConfig->lpBinaryPathName ? pConfig->lpBinaryPathName : L"";
            finding.serviceAccount = pConfig->lpServiceStartName ? pConfig->lpServiceStartName : L"";

            switch (pConfig->dwStartType) {
                case SERVICE_AUTO_START:   finding.startType = L"Automático"; break;
                case SERVICE_BOOT_START:   finding.startType = L"Boot"; break;
                case SERVICE_DEMAND_START: finding.startType = L"Manual"; break;
                case SERVICE_DISABLED:     finding.startType = L"Desativado"; break;
                case SERVICE_SYSTEM_START: finding.startType = L"Sistema"; break;
                default:                   finding.startType = L"Outro"; break;
            }

            // Check unquoted path with spaces
            if (hasUnquotedSpaces(finding.binaryPath)) {
                finding.isUnquotedPathWithSpaces = true;
                RiskFinding rf;
                rf.severity = RiskSeverity::High;
                rf.category = RiskCategory::ExploitableService;
                rf.affectedPath = finding.binaryPath;
                rf.affectedPrincipal = finding.serviceName;
                rf.title = L"Caminho de Serviço Não Entre Aspas com Espaços (Unquoted Service Path)";
                rf.description = L"O serviço '" + finding.serviceName + L"' possui caminho executável sem aspas: " + finding.binaryPath +
                                 L". O Windows tentará executar binários intermediários nos espaços ao iniciar o serviço.";
                rf.technicalDetail = L"Tipo de inicialização: " + finding.startType;
                rf.remediationAdvice = L"Envolva o caminho do executável entre aspas duplas no registro do Windows ou via sc.exe config.";
                rf.icaclsCommand = L"# Ajuste de registro: sc config \"" + finding.serviceName + L"\" binPath= \"\\\"" + finding.binaryPath + L"\\\"\"";
                rf.powershellCommand = L"# Ajuste via PowerShell: Set-ItemProperty -Path 'HKLM:\\System\\CurrentControlSet\\Services\\" + finding.serviceName + L"' -Name ImagePath -Value '\"" + finding.binaryPath + L"\"'";
                finding.risks.push_back(rf);
            }

            // Check binary file permissions
            std::wstring cleanBinary = extractExecutablePath(finding.binaryPath);
            if (!cleanBinary.empty()) {
                if (isBinaryWritableByNonAdmin(cleanBinary)) {
                    finding.isBinaryWritableByNonAdmin = true;
                    RiskFinding rf;
                    rf.severity = RiskSeverity::Critical;
                    rf.category = RiskCategory::ExploitableService;
                    rf.affectedPath = cleanBinary;
                    rf.affectedPrincipal = finding.serviceName;
                    rf.title = L"Executável de Serviço Gravável por Usuários Comuns (Elevação de Privilégios)";
                    rf.description = L"O binário do serviço '" + cleanBinary + L"' permite modificação por usuários comuns ou Everyone. "
                                     L"Um atacante pode substituir este executável para obter execução como SYSTEM.";
                    rf.technicalDetail = L"Conta do serviço: " + finding.serviceAccount;
                    rf.remediationAdvice = L"Remova imediatamente permissões de gravação de usuários comuns no executável e restaure a propriedade para Administradores.";
                    rf.icaclsCommand = L"icacls \"" + cleanBinary + L"\" /grant:r \"Administrators:(F)\" \"SYSTEM:(F)\" /remove \"Users\" \"Everyone\"";
                    rf.powershellCommand = L"# Executar icacls ou redefinir ACL do arquivo binario do servico";
                    finding.risks.push_back(rf);
                }
            }
        }
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return finding;
}

std::vector<ServiceSecurityFinding> ServiceAuditor::auditServices() {
    std::vector<ServiceSecurityFinding> findings;

    SC_HANDLE hSCM = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) return findings;

    DWORD bytesNeeded = 0;
    DWORD servicesReturned = 0;
    DWORD resumeHandle = 0;

    EnumServicesStatusExW(
        hSCM,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        nullptr,
        0,
        &bytesNeeded,
        &servicesReturned,
        &resumeHandle,
        nullptr
    );

    if (bytesNeeded > 0) {
        std::vector<BYTE> buffer(bytesNeeded);
        LPENUM_SERVICE_STATUS_PROCESSW pServices = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESSW>(buffer.data());

        if (EnumServicesStatusExW(
            hSCM,
            SC_ENUM_PROCESS_INFO,
            SERVICE_WIN32,
            SERVICE_STATE_ALL,
            buffer.data(),
            bytesNeeded,
            &bytesNeeded,
            &servicesReturned,
            &resumeHandle,
            nullptr
        )) {
            for (DWORD i = 0; i < servicesReturned; ++i) {
                std::wstring sName = pServices[i].lpServiceName;
                ServiceSecurityFinding sf = auditSingleService(sName);
                if (!sf.serviceName.empty()) {
                    findings.push_back(sf);
                }
            }
        }
    }

    CloseServiceHandle(hSCM);
    return findings;
}

} // namespace AclAuditor::Engine
