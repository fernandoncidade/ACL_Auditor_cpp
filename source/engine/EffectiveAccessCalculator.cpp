#include "EffectiveAccessCalculator.hpp"
#include "NtfsSecurityUtils.hpp"

#include <lm.h>
#include <sddl.h>
#include <authz.h>
#include <algorithm>
#include <sstream>

#ifdef _MSC_VER
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "authz.lib")
#endif

namespace AclAuditor::Engine {

std::vector<std::wstring> EffectiveAccessCalculator::enumerateSystemUsers() {
    std::vector<std::wstring> users;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    DWORD resumeHandle = 0;
    USER_INFO_0* pBuf = nullptr;

    NET_API_STATUS nStatus = NetUserEnum(
        nullptr,
        0,
        FILTER_NORMAL_ACCOUNT,
        (LPBYTE*)&pBuf,
        MAX_PREFERRED_LENGTH,
        &entriesRead,
        &totalEntries,
        &resumeHandle
    );

    if (nStatus == NERR_Success && pBuf) {
        for (DWORD i = 0; i < entriesRead; ++i) {
            users.push_back(pBuf[i].usri0_name);
        }
        NetApiBufferFree(pBuf);
    }

    // Include well-known accounts
    users.push_back(L"Everyone");
    users.push_back(L"Authenticated Users");
    users.push_back(L"SYSTEM");
    users.push_back(L"Administrators");
    users.push_back(L"Users");

    std::sort(users.begin(), users.end());
    users.erase(std::unique(users.begin(), users.end()), users.end());

    return users;
}

std::vector<std::wstring> EffectiveAccessCalculator::enumerateSystemGroups() {
    std::vector<std::wstring> groups;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    LOCALGROUP_INFO_0* pBuf = nullptr;

    NET_API_STATUS nStatus = NetLocalGroupEnum(
        nullptr,
        0,
        (LPBYTE*)&pBuf,
        MAX_PREFERRED_LENGTH,
        &entriesRead,
        &totalEntries,
        nullptr
    );

    if (nStatus == NERR_Success && pBuf) {
        for (DWORD i = 0; i < entriesRead; ++i) {
            groups.push_back(pBuf[i].lgrpi0_name);
        }
        NetApiBufferFree(pBuf);
    }

    return groups;
}

std::vector<SidInfo> EffectiveAccessCalculator::enumerateIdentityGroups(const std::wstring& accountOrSid) {
    std::vector<SidInfo> groupList;

    SidInfo targetPrincipal = NtfsSecurityUtils::resolveAccountOrSidString(accountOrSid);
    if (targetPrincipal.category == PrincipalCategory::OrphanedSid) {
        return groupList; // Orphaned SID has no active group memberships
    }

    // Always belongs to Everyone (S-1-1-0)
    SidInfo everyoneInfo = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-1-0");
    groupList.push_back(everyoneInfo);

    // If it is not Anonymous, also belongs to Authenticated Users (S-1-5-11)
    if (targetPrincipal.sidString != L"S-1-5-7") {
        SidInfo authUsers = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-5-11");
        groupList.push_back(authUsers);

        // Standard user token also contains INTERACTIVE (S-1-5-4)
        SidInfo interactive = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-5-4");
        groupList.push_back(interactive);
    }

    // Standard local interactive users implicitly belong to BUILTIN\Users (S-1-5-32-545)
    if (targetPrincipal.category == PrincipalCategory::User) {
        SidInfo builtinUsers = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-5-32-545");
        groupList.push_back(builtinUsers);

        // Query NetUserGetLocalGroups for actual local memberships
        LPLOCALGROUP_USERS_INFO_0 pGroups = nullptr;
        DWORD entriesRead = 0;
        DWORD totalEntries = 0;

        NET_API_STATUS status = NetUserGetLocalGroups(
            nullptr,
            targetPrincipal.accountName.c_str(),
            0,
            LG_INCLUDE_INDIRECT,
            (LPBYTE*)&pGroups,
            MAX_PREFERRED_LENGTH,
            &entriesRead,
            &totalEntries
        );

        if (status == NERR_Success && pGroups) {
            for (DWORD i = 0; i < entriesRead; ++i) {
                if (pGroups[i].lgrui0_name) {
                    SidInfo gInfo = NtfsSecurityUtils::resolveAccountOrSidString(pGroups[i].lgrui0_name);
                    groupList.push_back(gInfo);
                }
            }
            NetApiBufferFree(pGroups);
        }
    }

    return groupList;
}

EffectiveChainStep EffectiveAccessCalculator::buildChainStepForAce(
    const AceEntry& ace,
    const SidInfo& targetPrincipal,
    const std::vector<SidInfo>& userGroups,
    const std::wstring& objectPath
) {
    (void)objectPath;
    (void)userGroups;
    EffectiveChainStep step;
    step.accessType = ace.type;
    step.isInherited = ace.isInherited;
    step.originPath = ace.inheritedFromPath;
    step.accessGranted = ace.getRightsSummary();

    // Check if directly granted or via group
    if (ace.trustee.sidString == targetPrincipal.sidString) {
        step.title = L"Atribuição Direta: " + targetPrincipal.getDisplayName();
        step.relationship = L"Permissão concedida diretamente ao usuário/objeto";
    } else {
        // Find which group matched
        std::wstring groupName = ace.trustee.getDisplayName();
        step.title = L"Via Grupo: " + groupName;

        if (ace.trustee.isWellKnown) {
            step.relationship = L"Grupo universal do Windows NT do qual o usuário é membro implícito";
        } else {
            step.relationship = L"O usuário pertence a este grupo de segurança";
        }
    }

    // Add child nodes for the visual explanation tree
    EffectiveChainStep inheritanceStep;
    if (ace.isInherited) {
        inheritanceStep.title = L"Origem da Regra: Herdada";
        inheritanceStep.relationship = ace.inheritedFromPath.empty()
            ? L"Herdada da pasta pai"
            : L"Herdada de: " + ace.inheritedFromPath;
        inheritanceStep.originPath = ace.inheritedFromPath;
    } else {
        inheritanceStep.title = L"Origem da Regra: Explícita";
        inheritanceStep.relationship = L"Definida explicitamente neste objeto (não herdada)";
    }
    inheritanceStep.accessType = ace.type;

    EffectiveChainStep permissionStep;
    permissionStep.title = (ace.type == AccessType::Deny ? L"Direito NEGADO: " : L"Direito CONCEDIDO: ") + ace.getRightsSummary();
    permissionStep.relationship = NtfsSecurityUtils::formatAccessMaskDetailed(ace.accessMask, false);
    permissionStep.accessGranted = ace.getRightsSummary();
    permissionStep.accessType = ace.type;
    permissionStep.isDecisive = true;

    inheritanceStep.children.push_back(permissionStep);
    step.children.push_back(inheritanceStep);

    return step;
}

EffectiveAccessResult EffectiveAccessCalculator::calculateEffectiveAccess(
    const std::wstring& objectPath,
    const std::wstring& targetUserOrGroup
) {
    EffectiveAccessResult result;
    result.targetQuery = targetUserOrGroup;
    result.targetPrincipal = NtfsSecurityUtils::resolveAccountOrSidString(targetUserOrGroup);

    // Retrieve DACL
    SecurityDescriptorInfo secInfo;
    if (!NtfsSecurityUtils::getFileSecurityInfo(objectPath, secInfo)) {
        result.finalVerdictSummary = L"Falha ao obter descritor de segurança para o caminho especificado.";
        return result;
    }

    // Collect all applicable SIDs (User SID + all Group SIDs)
    std::set<std::wstring> applicableSids;
    applicableSids.insert(result.targetPrincipal.sidString);

    std::vector<SidInfo> groups = enumerateIdentityGroups(targetUserOrGroup);
    for (const auto& g : groups) {
        if (!g.sidString.empty()) {
            applicableSids.insert(g.sidString);
        }
    }

    // Windows NT DACL Evaluation Algorithm:
    // 1. Explicit Deny ACEs take precedence.
    // 2. Explicit Allow ACEs accumulate granted rights.
    // 3. Inherited Deny ACEs take precedence over Inherited Allow ACEs.
    // 4. Inherited Allow ACEs accumulate remaining granted rights.
    DWORD accumulatedAllow = 0;
    DWORD accumulatedDeny = 0;

    // Step A: Explicit Deny
    for (const auto& ace : secInfo.dacl) {
        if (!ace.isInherited && ace.type == AccessType::Deny && applicableSids.count(ace.trustee.sidString)) {
            accumulatedDeny |= ace.accessMask;
            result.conflictingDenyAces.push_back(ace);
        }
    }

    // Step B: Explicit Allow
    for (const auto& ace : secInfo.dacl) {
        if (!ace.isInherited && ace.type == AccessType::Allow && applicableSids.count(ace.trustee.sidString)) {
            accumulatedAllow |= (ace.accessMask & ~accumulatedDeny);
            result.matchingAces.push_back(ace);
        }
    }

    // Step C: Inherited Deny
    for (const auto& ace : secInfo.dacl) {
        if (ace.isInherited && ace.type == AccessType::Deny && applicableSids.count(ace.trustee.sidString)) {
            accumulatedDeny |= ace.accessMask;
            result.conflictingDenyAces.push_back(ace);
        }
    }

    // Step D: Inherited Allow
    for (const auto& ace : secInfo.dacl) {
        if (ace.isInherited && ace.type == AccessType::Allow && applicableSids.count(ace.trustee.sidString)) {
            accumulatedAllow |= (ace.accessMask & ~accumulatedDeny);
            result.matchingAces.push_back(ace);
        }
    }

    result.effectiveMask = accumulatedAllow;

    // Interpret resulting effective capabilities
    result.canFullControl = ((result.effectiveMask & FILE_ALL_ACCESS) == FILE_ALL_ACCESS);
    result.canModify = result.canFullControl ||
        (((result.effectiveMask & FILE_GENERIC_READ) == FILE_GENERIC_READ) &&
         ((result.effectiveMask & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE) &&
         ((result.effectiveMask & DELETE) == DELETE));
    result.canReadAndExecute = result.canFullControl || ((result.effectiveMask & FILE_GENERIC_EXECUTE) == FILE_GENERIC_EXECUTE);
    result.canRead = result.canFullControl || ((result.effectiveMask & FILE_GENERIC_READ) == FILE_GENERIC_READ);
    result.canWrite = result.canFullControl || ((result.effectiveMask & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE) ||
                      ((result.effectiveMask & FILE_WRITE_DATA) && (result.effectiveMask & FILE_APPEND_DATA));
    result.canDelete = (result.effectiveMask & DELETE) != 0;
    result.canTakeOwnership = (result.effectiveMask & WRITE_OWNER) != 0;
    result.canChangePermissions = (result.effectiveMask & WRITE_DAC) != 0;

    // Build the visual explanation tree
    EffectiveChainStep rootNode;
    rootNode.title = L"Identidade Analisada: " + result.targetPrincipal.getDisplayName();
    rootNode.relationship = L"Alvo da auditoria de acesso efetivo";
    rootNode.accessGranted = result.canFullControl ? L"Controle Total"
                           : result.canModify ? L"Modificar"
                           : result.canReadAndExecute ? L"Leitura e Execução"
                           : result.canWrite ? L"Gravação"
                           : result.canRead ? L"Leitura"
                           : (accumulatedDeny != 0) ? L"Acesso Negado (Deny)" : L"Sem Acesso";

    // Add Deny rules if any
    for (const auto& ace : result.conflictingDenyAces) {
        EffectiveChainStep step = buildChainStepForAce(ace, result.targetPrincipal, groups, objectPath);
        rootNode.children.push_back(step);
    }

    // Add Allow rules
    for (const auto& ace : result.matchingAces) {
        EffectiveChainStep step = buildChainStepForAce(ace, result.targetPrincipal, groups, objectPath);
        rootNode.children.push_back(step);
    }

    result.explanationTree.push_back(rootNode);

    // Build human-friendly verdict text
    result.finalVerdictSummary = generateExplanatoryText(result);

    return result;
}

std::wstring EffectiveAccessCalculator::generateExplanatoryText(const EffectiveAccessResult& result) {
    std::wstringstream ss;

    ss << L"Identidade: " << result.targetPrincipal.getDisplayName() << L"\n";

    if (!result.conflictingDenyAces.empty()) {
        ss << L"[ATENÇÃO - REGRA DE NEGAÇÃO ATIVA] Foram encontradas "
           << result.conflictingDenyAces.size()
           << L" regras de 'Acesso Negado' (Deny) aplicáveis que sobrepõem permissões de concessão.\n";
    }

    if (result.canFullControl) {
        ss << L"Veredito: CONTROLE TOTAL. O usuário possui direitos administrativos totais sobre o objeto.\n";
    } else if (result.canModify) {
        ss << L"Veredito: MODIFICAR. O usuário pode ler, gravar, alterar atributos e excluir arquivos neste local.\n";
    } else if (result.canReadAndExecute) {
        ss << L"Veredito: LEITURA E EXECUÇÃO. O usuário pode listar pastas, ler arquivos e executar programas.\n";
    } else if (result.canWrite) {
        ss << L"Veredito: GRAVAÇÃO. O usuário pode criar novos arquivos ou anexar dados.\n";
    } else if (result.canRead) {
        ss << L"Veredito: SOMENTE LEITURA. O usuário pode visualizar o conteúdo, mas não pode alterar ou executar.\n";
    } else {
        ss << L"Veredito: SEM ACESSO. Nenhuma permissão de concessão aplicável foi identificada na lista de controle de acesso.\n";
    }

    ss << L"\nCadeia de autorização:\n";
    if (result.matchingAces.empty() && result.conflictingDenyAces.empty()) {
        ss << L" • Nenhum ACE na DACL corresponde a este usuário ou a nenhum dos seus grupos.\n";
    } else {
        for (const auto& ace : result.matchingAces) {
            ss << L" • Concedido via [" << ace.trustee.getDisplayName() << L"]: "
               << ace.getRightsSummary() << L" (" << ace.getScopeDescription() << L")\n";
        }
        for (const auto& ace : result.conflictingDenyAces) {
            ss << L" • NEGADO via [" << ace.trustee.getDisplayName() << L"]: "
               << ace.getRightsSummary() << L" (" << ace.getScopeDescription() << L")\n";
        }
    }

    return ss.str();
}

} // namespace AclAuditor::Engine
