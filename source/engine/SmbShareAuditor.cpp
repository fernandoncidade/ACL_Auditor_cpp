#include "SmbShareAuditor.hpp"
#include "NtfsSecurityUtils.hpp"
#include "RiskScanner.hpp"

#include <lm.h>
#include <algorithm>

#ifdef _MSC_VER
#pragma comment(lib, "netapi32.lib")
#endif

namespace AclAuditor::Engine {

bool SmbShareAuditor::isAdministrativeShare(const std::wstring& shareName) {
    if (shareName.empty()) return false;
    if (shareName.back() == L'$') return true; // C$, D$, ADMIN$, IPC$, PRINT$
    return false;
}

std::vector<SmbShareFinding> SmbShareAuditor::auditShares() {
    std::vector<SmbShareFinding> findings;

    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    DWORD resumeHandle = 0;
    PSHARE_INFO_502 pBuf502 = nullptr;

    NET_API_STATUS res = NetShareEnum(
        nullptr,
        502,
        (LPBYTE*)&pBuf502,
        MAX_PREFERRED_LENGTH,
        &entriesRead,
        &totalEntries,
        &resumeHandle
    );

    if (res == NERR_Success && pBuf502) {
        for (DWORD i = 0; i < entriesRead; ++i) {
            SHARE_INFO_502& info = pBuf502[i];
            SmbShareFinding sf;
            sf.shareName = info.shi502_netname ? info.shi502_netname : L"";
            sf.localPath = info.shi502_path ? info.shi502_path : L"";
            sf.comment = info.shi502_remark ? info.shi502_remark : L"";
            sf.shareType = info.shi502_type;

            // Skip default IPC$ share
            if (sf.shareName == L"IPC$") continue;

            // Parse share security descriptor if present
            if (info.shi502_security_descriptor) {
                PACL pDacl = nullptr;
                BOOL daclPresent = FALSE;
                BOOL daclDefaulted = FALSE;

                if (GetSecurityDescriptorDacl(info.shi502_security_descriptor, &daclPresent, &pDacl, &daclDefaulted) && daclPresent && pDacl) {
                    sf.shareAcl = NtfsSecurityUtils::parseAcl(pDacl, sf.shareName, false);
                }
            }

            // Summarize share ACL
            for (const auto& ace : sf.shareAcl) {
                if (ace.type == AccessType::Allow && ace.trustee.sidString == L"S-1-1-0") {
                    sf.isEveryoneAllowed = true;
                    if (ace.fullControl || ace.modify) {
                        sf.isEveryoneFullControl = true;
                    }
                }
            }

            // Check underlying NTFS directory
            if (!sf.localPath.empty()) {
                SecurityDescriptorInfo ntfsSec;
                if (NtfsSecurityUtils::getFileSecurityInfo(sf.localPath, ntfsSec)) {
                    bool ntfsAllowsEveryoneWrite = false;
                    for (const auto& ace : ntfsSec.dacl) {
                        if (ace.type == AccessType::Allow && (ace.trustee.sidString == L"S-1-1-0" || ace.trustee.sidString == L"S-1-5-11")) {
                            if (ace.canWriteData || ace.modify || ace.fullControl) {
                                ntfsAllowsEveryoneWrite = true;
                                break;
                            }
                        }
                    }

                    // Flag risk if share allows Everyone Full Control / Modify
                    if (sf.isEveryoneFullControl) {
                        RiskFinding rf;
                        rf.severity = isAdministrativeShare(sf.shareName) ? RiskSeverity::Medium : RiskSeverity::Critical;
                        rf.category = RiskCategory::SmbExposure;
                        rf.affectedPath = sf.localPath.empty() ? sf.shareName : sf.localPath;
                        rf.affectedPrincipal = L"Everyone";
                        rf.title = L"Compartilhamento SMB com Acesso Total para 'Everyone'";
                        rf.description = L"O compartilhamento de rede '" + sf.shareName +
                                         L"' concede permissões de gravação ou Controle Total ao grupo Everyone no nível de compartilhamento.";
                        rf.technicalDetail = L"Caminho local: " + sf.localPath;
                        rf.remediationAdvice = L"Restrinja a permissão do compartilhamento para 'Authenticated Users' ou grupos específicos e garanta que as permissões NTFS locais sejam estritas.";
                        rf.icaclsCommand = L"# Ajuste permissao de compartilhamento via powershell: Grant-SmbShareAccess -Name \"" + sf.shareName + L"\" ...";
                        rf.powershellCommand = L"Revoke-SmbShareAccess -Name '" + sf.shareName + L"' -AccountName 'Everyone' -Force";
                        sf.risks.push_back(rf);
                    }

                    if (sf.isEveryoneAllowed && ntfsAllowsEveryoneWrite && !isAdministrativeShare(sf.shareName)) {
                        sf.isNtfsMismatch = true;
                        RiskFinding rf;
                        rf.severity = RiskSeverity::High;
                        rf.category = RiskCategory::SmbExposure;
                        rf.affectedPath = sf.localPath;
                        rf.affectedPrincipal = L"Everyone / Users";
                        rf.title = L"Exposição Crítica de Rede: Compartilhamento e Pasta NTFS Abertos para Gravação";
                        rf.description = L"Tanto o compartilhamento SMB quanto o diretório NTFS local permitem gravação sem restrição para usuários comuns e convidados de rede.";
                        rf.technicalDetail = L"Compartilhamento: " + sf.shareName + L", Pasta: " + sf.localPath;
                        rf.remediationAdvice = L"Aplique o princípio do menor privilégio, limitando a gravação apenas a equipes autorizadas.";
                        rf.icaclsCommand = L"icacls \"" + sf.localPath + L"\" /remove \"Everyone\"";
                        rf.powershellCommand = L"# Aplicar ACL segura no compartilhamento e pasta local";
                        sf.risks.push_back(rf);
                    }
                }
            }

            findings.push_back(sf);
        }
        NetApiBufferFree(pBuf502);
    } else {
        // Fallback to level 2 if 502 wasn't permitted
        PSHARE_INFO_2 pBuf2 = nullptr;
        res = NetShareEnum(nullptr, 2, (LPBYTE*)&pBuf2, MAX_PREFERRED_LENGTH, &entriesRead, &totalEntries, &resumeHandle);
        if (res == NERR_Success && pBuf2) {
            for (DWORD i = 0; i < entriesRead; ++i) {
                SmbShareFinding sf;
                sf.shareName = pBuf2[i].shi2_netname ? pBuf2[i].shi2_netname : L"";
                sf.localPath = pBuf2[i].shi2_path ? pBuf2[i].shi2_path : L"";
                sf.comment = pBuf2[i].shi2_remark ? pBuf2[i].shi2_remark : L"";
                sf.shareType = pBuf2[i].shi2_type;

                if (sf.shareName == L"IPC$") continue;

                if (!sf.localPath.empty()) {
                    SecurityDescriptorInfo ntfsSec;
                    if (NtfsSecurityUtils::getFileSecurityInfo(sf.localPath, ntfsSec)) {
                        for (const auto& ace : ntfsSec.dacl) {
                            if (ace.type == AccessType::Allow && ace.trustee.sidString == L"S-1-1-0" && (ace.canWriteData || ace.fullControl)) {
                                RiskFinding rf;
                                rf.severity = RiskSeverity::High;
                                rf.category = RiskCategory::SmbExposure;
                                rf.affectedPath = sf.localPath;
                                rf.affectedPrincipal = L"Everyone";
                                rf.title = L"Pasta Compartilhada em Rede Aberta para Gravação Local";
                                rf.description = L"A pasta compartilhada '" + sf.shareName + L"' possui permissões NTFS permissivas para Everyone.";
                                rf.technicalDetail = L"Caminho: " + sf.localPath;
                                rf.remediationAdvice = L"Remova o grupo Everyone das permissões NTFS do diretório compartilhado.";
                                sf.risks.push_back(rf);
                            }
                        }
                    }
                }

                findings.push_back(sf);
            }
            NetApiBufferFree(pBuf2);
        }
    }

    return findings;
}

SmbShareFinding SmbShareAuditor::auditSingleShare(const std::wstring& shareName) {
    auto all = auditShares();
    for (const auto& s : all) {
        if (_wcsicmp(s.shareName.c_str(), shareName.c_str()) == 0) {
            return s;
        }
    }
    SmbShareFinding empty;
    empty.shareName = shareName;
    return empty;
}

} // namespace AclAuditor::Engine
