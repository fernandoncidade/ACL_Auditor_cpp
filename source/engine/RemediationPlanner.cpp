#include "RemediationPlanner.hpp"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <sstream>

namespace AclAuditor::Engine {

RemediationPlan RemediationPlanner::buildPlan(
    const std::wstring& rootPath,
    const std::vector<RiskFinding>& findings,
    bool includeRollbackSafety
) {
    RemediationPlan plan;
    plan.title = L"Plano de Remediação e Endurecimento NTFS - ACL Auditor";
    plan.generatedTimestamp = QDateTime::currentDateTime().toString(Qt::ISODate).toStdWString();
    plan.totalIssuesToRemediate = findings.size();

    std::wstringstream icaclsSs;
    std::wstringstream psSs;
    std::wstringstream mdSs;
    std::wstringstream rollbackSs;

    // Batch / icacls header
    icaclsSs << L"@echo off\n";
    icaclsSs << L"rem ====================================================================\n";
    icaclsSs << L"rem ACL AUDITOR - SCRIPT DE REMEDIACAO NTFS (MODO SOMENTE LEITURA VALIDADO)\n";
    icaclsSs << L"rem Data de Geracao: " << plan.generatedTimestamp << L"\n";
    icaclsSs << L"rem Alvo: " << rootPath << L"\n";
    icaclsSs << L"rem Total de Vulnerabilidades Tratadas: " << findings.size() << L"\n";
    icaclsSs << L"rem ====================================================================\n\n";

    if (includeRollbackSafety) {
        icaclsSs << L"echo [1/3] Criando backup de seguranca das ACLs atuais...\n";
        icaclsSs << L"icacls \"" << rootPath << L"\" /save \"%TEMP%\\acl_auditor_backup.acl\" /t /c /q\n";
        icaclsSs << L"if %ERRORLEVEL% NEQ 0 (\n";
        icaclsSs << L"    echo [ERRO] Falha ao criar backup das ACLs. Abortando operacao.\n";
        icaclsSs << L"    exit /b 1\n";
        icaclsSs << L")\n\n";
        icaclsSs << L"echo [2/3] Aplicando correcoes de endurecimento e higiene...\n";
    }

    // PowerShell header
    psSs << L"#Requires -RunAsAdministrator\n";
    psSs << L"<#\n";
    psSs << L"    .SYNOPSIS\n";
    psSs << L"        Script de Remediacao e Conformidade de Permissoes NTFS gerado pelo ACL Auditor.\n";
    psSs << L"    .DESCRIPTION\n";
    psSs << L"        Corrige diretorios gravaveis por usuarios comuns, remove contas orfas,\n";
    psSs << L"        elimina riscos de binary planting e restaura a heranca segura.\n";
    psSs << L"    .DATE\n";
    psSs << L"        " << plan.generatedTimestamp << L"\n";
    psSs << L"#>\n\n";
    psSs << L"$ErrorActionPreference = 'Stop'\n";
    psSs << L"Write-Host '=====================================================' -ForegroundColor Cyan\n";
    psSs << L"Write-Host '   ACL AUDITOR - PLANO DE MITIGACAO AUTOMATIZADO      ' -ForegroundColor Cyan\n";
    psSs << L"Write-Host '=====================================================' -ForegroundColor Cyan\n\n";

    // Markdown Report header
    mdSs << L"# Plano Técnico de Remediação de Acessos NTFS\n\n";
    mdSs << L"**Data:** " << plan.generatedTimestamp << L"  \n";
    mdSs << L"**Escopo:** `" << rootPath << L"`  \n";
    mdSs << L"**Total de Itens Auditados:** " << findings.size() << L"\n\n";
    mdSs << L"## Resumo Executivo das Ações Corretivas\n\n";
    mdSs << L"Este plano contém as ações técnicas recomendadas para fechar brechas de privilégios excessivos, eliminar vetores de execução indevida e restabelecer a integridade das listas de controle de acesso (DACLs).\n\n";
    mdSs << L"| Severidade | Categoria | Caminho Afetado | Ação Proposta |\n";
    mdSs << L"| :--- | :--- | :--- | :--- |\n";

    // Rollback script header
    rollbackSs << L"@echo off\n";
    rollbackSs << L"rem Script de Restauração (Rollback) - ACL Auditor\n";
    rollbackSs << L"echo Restaurando ACLs a partir do backup...\n";
    rollbackSs << L"icacls \"" << rootPath << L"\\..\" /restore \"%TEMP%\\acl_auditor_backup.acl\"\n";
    rollbackSs << L"echo Concluido.\n";

    int counter = 1;
    for (const auto& f : findings) {
        mdSs << L"| **" << RiskFinding::severityToString(f.severity) << L"** | "
             << f.title << L" | `" << f.affectedPath << L"` | `" << f.remediationAdvice << L"` |\n";

        if (!f.icaclsCommand.empty()) {
            icaclsSs << L"rem Item " << counter << L": " << f.title << L"\n";
            icaclsSs << f.icaclsCommand << L"\n\n";
        }

        if (!f.powershellCommand.empty()) {
            psSs << L"# Item " << counter << L": " << f.title << L"\n";
            psSs << L"Write-Host '[Item " << counter << L"] " << f.title << L"' -ForegroundColor Yellow\n";
            psSs << f.powershellCommand << L"\n\n";
        }

        counter++;
    }

    if (includeRollbackSafety) {
        icaclsSs << L"echo [3/3] Correcoes aplicadas com sucesso.\n";
        icaclsSs << L"echo Para reverter, execute o script de rollback gerado.\n";
        icaclsSs << L"pause\n";

        psSs << L"Write-Host '[CONCLUIDO] Todas as politicas de endurecimento foram aplicadas.' -ForegroundColor Green\n";
    }

    plan.icaclsScript = icaclsSs.str();
    plan.powershellScript = psSs.str();
    plan.markdownReport = mdSs.str();
    plan.rollbackScript = rollbackSs.str();

    return plan;
}

bool RemediationPlanner::exportScriptToFile(const std::wstring& scriptContent, const std::wstring& filePath) {
    QFile file(QString::fromStdWString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << QString::fromStdWString(scriptContent);
    file.close();
    return true;
}

} // namespace AclAuditor::Engine
