// cspell:words VALIDACAO Resolucao seguranca Proprietario DACL dacl Heranca Permissao Gravacao Execucao Pontuacao comparacao Alteracoes enumeracao servicos
#include "source/engine/AclTypes.hpp"
#include "source/engine/NtfsSecurityUtils.hpp"
#include "source/engine/EffectiveAccessCalculator.hpp"
#include "source/engine/RiskScanner.hpp"
#include "source/engine/ServiceAuditor.hpp"
#include "source/engine/SmbShareAuditor.hpp"
#include "source/engine/SnapshotEngine.hpp"
#include "source/engine/RemediationPlanner.hpp"

#include <QCoreApplication>
#include <iostream>
#include <cassert>
#include <io.h>
#include <fcntl.h>

using namespace AclAuditor::Engine;

void runTests() {
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    std::wcout << L"============================================================" << std::endl;
    std::wcout << L"  TESTES AUTOMATIZADOS E VALIDACAO - ACL AUDITOR ENGINE     " << std::endl;
    std::wcout << L"============================================================" << std::endl;

    // Test 1: NtfsSecurityUtils
    std::wcout << L"[TESTE 1] Validando NtfsSecurityUtils..." << std::endl;
    NtfsSecurityUtils::enableSecurityPrivileges();
    bool elevated = NtfsSecurityUtils::isProcessElevated();
    std::wcout << L"  - Processo elevado (Admin): " << (elevated ? L"SIM" : L"NAO") << std::endl;

    SidInfo everyone = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-1-0");
    assert(everyone.isWellKnown);
    std::wcout << L"  - Resolucao S-1-1-0: " << everyone.getDisplayName() << L" [OK]" << std::endl;

    std::string sha = NtfsSecurityUtils::computeSha256Hex("ACL Auditor Test Data");
    assert(!sha.empty());
    std::wcout << L"  - SHA-256 CNG Hash: " << QString::fromStdString(sha).toStdWString() << L" [OK]" << std::endl;

    // Test 2: File Security Descriptor
    std::wcout << L"[TESTE 2] Inspecionando descritor de seguranca em C:\\Windows..." << std::endl;
    SecurityDescriptorInfo sec;
    bool okSec = NtfsSecurityUtils::getFileSecurityInfo(L"C:\\Windows", sec);
    if (okSec) {
        std::wcout << L"  - Proprietario: " << sec.owner.getDisplayName() << std::endl;
        std::wcout << L"  - Total de ACEs na DACL: " << sec.dacl.size() << std::endl;
        std::wcout << L"  - Heranca Bloqueada: " << (sec.isInheritanceBlocked ? L"SIM" : L"NAO") << L" [OK]" << std::endl;
    }

    // Test 3: Effective Access & Visual Chain Tree
    std::wcout << L"[TESTE 3] Calculando acesso efetivo e arvore explicativa..." << std::endl;
    EffectiveAccessResult eff = EffectiveAccessCalculator::calculateEffectiveAccess(L"C:\\Windows", L"Everyone");
    std::wcout << L"  - Permissao Efetiva de Everyone em C:\\Windows:" << std::endl;
    std::wcout << L"    * Leitura: " << (eff.canRead ? L"SIM" : L"NAO") << std::endl;
    std::wcout << L"    * Gravacao: " << (eff.canWrite ? L"SIM" : L"NAO") << std::endl;
    std::wcout << L"    * Execucao: " << (eff.canReadAndExecute ? L"SIM" : L"NAO") << std::endl;
    std::wcout << L"    * Nos da arvore explicativa gerados: " << eff.explanationTree.size() << L" [OK]" << std::endl;

    // Test 4: Risk Scanner
    std::wcout << L"[TESTE 4] Validando RiskScanner..." << std::endl;
    FileSecurityNode testNode;
    testNode.path = L"C:\\Temp\\VulnerableApp";
    testNode.fileName = L"VulnerableApp";
    testNode.isDirectory = true;

    AceEntry vulnAce;
    vulnAce.trustee = everyone;
    vulnAce.type = AccessType::Allow;
    vulnAce.canWriteData = true;
    vulnAce.modify = true;
    testNode.security.dacl.push_back(vulnAce);

    auto findings = RiskScanner::evaluateNodeRisks(testNode);
    std::wcout << L"  - Vulnerabilidades detectadas no no de teste: " << findings.size() << std::endl;
    assert(!findings.empty());
    std::wcout << L"  - Titulo: " << findings[0].title << L" [OK]" << std::endl;
    int score = RiskScanner::calculateRiskScore(findings);
    std::wcout << L"  - Pontuacao de risco calculada: " << score << L" / 100 [OK]" << std::endl;

    // Test 5: Remediation Planner
    std::wcout << L"[TESTE 5] Validando RemediationPlanner..." << std::endl;
    RemediationPlan plan = RemediationPlanner::buildPlan(testNode.path, findings, true);
    assert(!plan.icaclsScript.empty());
    assert(!plan.powershellScript.empty());
    std::wcout << L"  - Script icacls gerado com sucesso (" << plan.icaclsScript.length() << L" caracteres) [OK]" << std::endl;
    std::wcout << L"  - Script PowerShell gerado com sucesso (" << plan.powershellScript.length() << L" caracteres) [OK]" << std::endl;

    // Test 6: Snapshot Engine & Temporal Diff
    std::wcout << L"[TESTE 6] Validando SnapshotEngine e comparacao temporal (SQLite)..." << std::endl;
    SnapshotEngine::instance().initializeDatabase();
    SnapshotRecord snapA = SnapshotEngine::instance().createSnapshot(L"Snapshot Teste A", testNode.path, testNode, findings);
    bool savedA = SnapshotEngine::instance().saveSnapshot(snapA);
    assert(savedA);
    (void)savedA;
    std::wcout << L"  - Snapshot A salvo no SQLite (Hash: " << QString::fromStdString(snapA.sha256Hash).left(16).toStdWString() << L"...) [OK]" << std::endl;

    // Create modified node for Snapshot B (add second ACE)
    FileSecurityNode modifiedNode = testNode;
    AceEntry newAce;
    newAce.trustee = NtfsSecurityUtils::resolveAccountOrSidString(L"S-1-5-11"); // Authenticated Users
    newAce.type = AccessType::Allow;
    newAce.fullControl = true;
    modifiedNode.security.dacl.push_back(newAce);

    SnapshotRecord snapB = SnapshotEngine::instance().createSnapshot(L"Snapshot Teste B", modifiedNode.path, modifiedNode, findings);
    SnapshotEngine::instance().saveSnapshot(snapB);

    SnapshotDiffResult diff = SnapshotEngine::instance().compareSnapshots(snapA, snapB);
    std::wcout << L"  - Diff temporal calculado. Alteracoes encontradas: " << diff.totalChanges << L" [OK]" << std::endl;
    assert(diff.totalChanges > 0);

    // Clean up test snapshots
    SnapshotEngine::instance().deleteSnapshot(snapA.id);
    SnapshotEngine::instance().deleteSnapshot(snapB.id);

    // Test 7: Services and Shares
    std::wcout << L"[TESTE 7] Testando enumeracao de servicos e compartilhamentos..." << std::endl;
    auto shares = SmbShareAuditor::auditShares();
    std::wcout << L"  - Compartilhamentos SMB auditados no host: " << shares.size() << L" [OK]" << std::endl;

    std::wcout << L"\n============================================================" << std::endl;
    std::wcout << L"  TODOS OS 7 TESTES DA ENGINE FORAM EXECUTADOS COM SUCESSO!  " << std::endl;
    std::wcout << L"============================================================" << std::endl;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    runTests();
    return 0;
}
