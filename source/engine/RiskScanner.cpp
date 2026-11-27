#include "RiskScanner.hpp"
#include "NtfsSecurityUtils.hpp"

#include <algorithm>
#include <cwctype>

namespace AclAuditor::Engine {

bool RiskScanner::isBroadGroup(const SidInfo& trustee) {
    if (trustee.sidString == L"S-1-1-0") return true;      // Everyone
    if (trustee.sidString == L"S-1-5-11") return true;     // Authenticated Users
    if (trustee.sidString == L"S-1-5-32-545") return true; // BUILTIN\Users
    if (trustee.sidString == L"S-1-5-7") return true;      // Anonymous
    return false;
}

bool RiskScanner::isAdministrativePrincipal(const SidInfo& trustee) {
    if (trustee.sidString == L"S-1-5-32-544") return true; // BUILTIN\Administrators
    if (trustee.sidString == L"S-1-5-18") return true;     // SYSTEM
    if (trustee.accountName == L"Administrador" || trustee.accountName == L"Administrator") return true;
    return false;
}

bool RiskScanner::isExecutableOrScript(const std::wstring& path) {
    size_t dotPos = path.find_last_of(L'.');
    if (dotPos == std::wstring::npos) return false;

    std::wstring ext = path.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    return (ext == L".exe" || ext == L".dll" || ext == L".sys" ||
            ext == L".bat" || ext == L".cmd" || ext == L".ps1" ||
            ext == L".vbs" || ext == L".msi" || ext == L".com" ||
            ext == L".scr" || ext == L".cpl" || ext == L".ocx");
}

bool RiskScanner::isSensitiveName(const std::wstring& name) {
    std::wstring lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    const wchar_t* keywords[] = {
        L"financeir", L"contabil", L"salari", L"rh", L"folha",
        L"secret", L"confidenc", L"senha", L"password", L"backup",
        L"contrato", L"juridic", L"diretoria", L"gestao", L"privado",
        L"pessoal", L"credito", L"token", L"chave", L"key"
    };

    for (const auto* kw : keywords) {
        if (lower.find(kw) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

void RiskScanner::generateRemediationCommands(RiskFinding& finding) {
    std::wstring p = finding.affectedPath;

    switch (finding.category) {
        case RiskCategory::WorldWritable:
        case RiskCategory::BinaryPlanting: {
            finding.icaclsCommand = L"icacls \"" + p + L"\" /remove:g \"" + finding.affectedPrincipal + L"\" /t /c /l /q";
            finding.powershellCommand =
                L"$acl = Get-Acl -LiteralPath '" + p + L"'; "
                L"$rule = $acl.Access | Where-Object { $_.IdentityReference -match '" + finding.affectedPrincipal + L"' }; "
                L"if ($rule) { $acl.RemoveAccessRuleAll($rule); Set-Acl -LiteralPath '" + p + L"' -AclObject $acl }";
            break;
        }
        case RiskCategory::OrphanedPrincipal: {
            finding.icaclsCommand = L"icacls \"" + p + L"\" /remove \"" + finding.affectedPrincipal + L"\"";
            finding.powershellCommand =
                L"$acl = Get-Acl -LiteralPath '" + p + L"'; "
                L"$orphaned = $acl.Access | Where-Object { $_.IdentityReference.Value -eq '" + finding.affectedPrincipal + L"' }; "
                L"if ($orphaned) { $acl.RemoveAccessRuleAll($orphaned); Set-Acl -LiteralPath '" + p + L"' -AclObject $acl }";
            break;
        }
        case RiskCategory::BrokenInheritance: {
            finding.icaclsCommand = L"icacls \"" + p + L"\" /inheritance:e";
            finding.powershellCommand =
                L"$acl = Get-Acl -LiteralPath '" + p + L"'; "
                L"$acl.SetAccessRuleProtection($false, $true); "
                L"Set-Acl -LiteralPath '" + p + L"' -AclObject $acl";
            break;
        }
        case RiskCategory::ExcessivePrivilege: {
            finding.icaclsCommand = L"icacls \"" + p + L"\" /deny \"" + finding.affectedPrincipal + L":(WDAC,WO)\"";
            finding.powershellCommand =
                L"# Recomenda-se remover o direito de alteracao de permissoes (WriteDac) e apropriacao (WriteOwner)";
            break;
        }
        default: {
            finding.icaclsCommand = L"icacls \"" + p + L"\" /reset /t";
            finding.powershellCommand = L"# Revisao manual recomendada para conformidade";
            break;
        }
    }
}

std::vector<RiskFinding> RiskScanner::evaluateNodeRisks(const FileSecurityNode& node) {
    std::vector<RiskFinding> findings;

    // Check 1: Orphaned SIDs (Deleted accounts in ACL)
    for (const auto& ace : node.security.dacl) {
        if (ace.trustee.isOrphaned) {
            RiskFinding f;
            f.severity = RiskSeverity::Medium;
            f.category = RiskCategory::OrphanedPrincipal;
            f.affectedPath = node.path;
            f.affectedPrincipal = ace.trustee.sidString;
            f.title = L"Conta Órfã / SID Deletado na ACL";
            f.description = L"Foi identificado um SID na lista de controle de acesso correspondente a uma conta excluída ou inexistente: " + ace.trustee.sidString;
            f.technicalDetail = L"Tipo: " + std::wstring(ace.type == AccessType::Allow ? L"Concedido (Allow)" : L"Negado (Deny)") +
                                L", Máscara: " + NtfsSecurityUtils::formatAccessMaskHex(ace.accessMask);
            f.remediationAdvice = L"Remova o SID órfão da lista de controle de acesso para restaurar a higiene do diretório e evitar reutilização acidental de SID.";
            generateRemediationCommands(f);
            findings.push_back(f);
        }
    }

    // Check 2: Pastas graváveis por usuários comuns (World-Writable)
    if (node.isDirectory) {
        bool isSysDir = NtfsSecurityUtils::isSystemDirectory(node.path);

        for (const auto& ace : node.security.dacl) {
            if (ace.type == AccessType::Allow && isBroadGroup(ace.trustee)) {
                bool hasWrite = ace.canWriteData || ace.canAppendData || (ace.accessMask & FILE_ADD_FILE) ||
                                (ace.accessMask & FILE_ADD_SUBDIRECTORY) || ace.write || ace.fullControl || ace.modify;

                if (hasWrite) {
                    RiskFinding f;
                    f.severity = isSysDir ? RiskSeverity::Critical : RiskSeverity::High;
                    f.category = RiskCategory::WorldWritable;
                    f.affectedPath = node.path;
                    f.affectedPrincipal = ace.trustee.getDisplayName();
                    f.title = isSysDir ? L"Diretório de Sistema Gravável por Usuários Comuns (CRÍTICO)"
                                       : L"Diretório com Permissão Excessiva de Gravação para Grupos Globais";
                    f.description = L"O grupo amplo '" + ace.trustee.getDisplayName() +
                                    L"' possui permissão de gravação/criação de arquivos nesta pasta. Isso permite que qualquer usuário padrão modifique ou plante arquivos.";
                    f.technicalDetail = L"Direitos detectados: " + ace.getRightsSummary() + L" (" +
                                        NtfsSecurityUtils::formatAccessMaskDetailed(ace.accessMask, true) + L")";
                    f.remediationAdvice = L"Substitua a permissão de gravação por permissão de Leitura e Execução para usuários comuns, reservando a gravação apenas para contas administrativas.";
                    generateRemediationCommands(f);
                    findings.push_back(f);
                }
            }
        }
    }

    // Check 3: Executáveis em locais modificáveis (Binary Planting / DLL Hijacking)
    if (!node.isDirectory && isExecutableOrScript(node.path)) {
        for (const auto& ace : node.security.dacl) {
            if (ace.type == AccessType::Allow && isBroadGroup(ace.trustee)) {
                bool canModifyBinary = ace.canWriteData || ace.canAppendData || ace.modify || ace.fullControl || ace.write;
                if (canModifyBinary) {
                    RiskFinding f;
                    f.severity = RiskSeverity::Critical;
                    f.category = RiskCategory::BinaryPlanting;
                    f.affectedPath = node.path;
                    f.affectedPrincipal = ace.trustee.getDisplayName();
                    f.title = L"Vulnerabilidade de Binary Planting / Substituição de Executável";
                    f.description = L"O executável ou script '" + node.fileName +
                                    L"' pode ser sobrescrito ou modificado pelo grupo '" + ace.trustee.getDisplayName() +
                                    L"'. Um atacante ou malware pode substituir este binário para elevar privilégios.";
                    f.technicalDetail = L"Arquivo: " + node.path + L", Permissão: " + ace.getRightsSummary();
                    f.remediationAdvice = L"Revogue direitos de gravação para usuários comuns no executável e garanta que apenas Administradores ou SYSTEM possam modificá-lo.";
                    generateRemediationCommands(f);
                    findings.push_back(f);
                }
            }
        }
    }

    // Check 4: Permissões excessivas de controle (WRITE_DAC ou WRITE_OWNER para não-administradores)
    for (const auto& ace : node.security.dacl) {
        if (ace.type == AccessType::Allow && !isAdministrativePrincipal(ace.trustee)) {
            if (ace.canWriteDac || ace.canWriteOwner) {
                RiskFinding f;
                f.severity = RiskSeverity::High;
                f.category = RiskCategory::ExcessivePrivilege;
                f.affectedPath = node.path;
                f.affectedPrincipal = ace.trustee.getDisplayName();
                f.title = L"Privilégio Excessivo de Modificação de ACL (WRITE_DAC / WRITE_OWNER)";
                f.description = L"A conta ou grupo '" + ace.trustee.getDisplayName() +
                                L"' possui direitos para alterar permissões (WRITE_DAC) ou apropriar-se do objeto (WRITE_OWNER). Isso permite que essa conta conceda a si mesma Controle Total.";
                f.technicalDetail = L"Máscara: " + NtfsSecurityUtils::formatAccessMaskHex(ace.accessMask);
                f.remediationAdvice = L"Remova os direitos WRITE_DAC e WRITE_OWNER para contas não administrativas.";
                generateRemediationCommands(f);
                findings.push_back(f);
            }
        }
    }

    // Check 5: Herança bloqueada (SE_DACL_PROTECTED)
    if (node.security.isInheritanceBlocked) {
        RiskFinding f;
        f.severity = RiskSeverity::Low;
        f.category = RiskCategory::BrokenInheritance;
        f.affectedPath = node.path;
        f.affectedPrincipal = L"Herança";
        f.title = L"Herança de Permissões Desativada (Permissões Explícitas Isoladas)";
        f.description = L"A pasta está configurada como protegida (SE_DACL_PROTECTED). Ela não herda mais permissões do diretório pai, podendo ocultar ou despadronizar o modelo de governança.";
        f.technicalDetail = L"Flag SE_DACL_PROTECTED detectada no Security Descriptor.";
        f.remediationAdvice = L"Verifique se o bloqueio de herança é intencional. Caso contrário, reative a herança automática para manter a conformidade.";
        generateRemediationCommands(f);
        findings.push_back(f);
    }

    // Check 6: Dados confidenciais expostos a 'Everyone'
    if (isSensitiveName(node.fileName)) {
        for (const auto& ace : node.security.dacl) {
            if (ace.type == AccessType::Allow && (ace.trustee.sidString == L"S-1-1-0" || ace.trustee.sidString == L"S-1-5-11")) {
                RiskFinding f;
                f.severity = RiskSeverity::High;
                f.category = RiskCategory::ExcessivePrivilege;
                f.affectedPath = node.path;
                f.affectedPrincipal = ace.trustee.getDisplayName();
                f.title = L"Dados Sensíveis / Confidenciais Acessíveis por Grupos Globais";
                f.description = L"O objeto '" + node.fileName + L"' possui indícios de conter dados sensíveis ou corporativos restritos, porém concede acesso a '" +
                                ace.trustee.getDisplayName() + L"'.";
                f.technicalDetail = L"Permissão concedida: " + ace.getRightsSummary();
                f.remediationAdvice = L"Restrinja o acesso exclusivamente aos grupos de departamento pertinentes (ex.: RH, Financeiro, Gestores) e remova Everyone.";
                generateRemediationCommands(f);
                findings.push_back(f);
            }
        }
    }

    return findings;
}

std::vector<RiskFinding> RiskScanner::evaluateTreeRisks(const FileSecurityNode& rootNode) {
    std::vector<RiskFinding> allFindings = evaluateNodeRisks(rootNode);

    for (const auto& child : rootNode.children) {
        std::vector<RiskFinding> childFindings = evaluateTreeRisks(child);
        allFindings.insert(allFindings.end(), childFindings.begin(), childFindings.end());
    }

    return allFindings;
}

int RiskScanner::calculateRiskScore(const std::vector<RiskFinding>& findings) {
    if (findings.empty()) return 0;

    int score = 0;
    for (const auto& f : findings) {
        switch (f.severity) {
            case RiskSeverity::Critical: score += 30; break;
            case RiskSeverity::High:     score += 18; break;
            case RiskSeverity::Medium:   score += 10; break;
            case RiskSeverity::Low:      score += 4;  break;
            case RiskSeverity::Info:     score += 1;  break;
        }
    }

    return std::min(100, score);
}

} // namespace AclAuditor::Engine
