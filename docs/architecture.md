# Arquitetura do Sistema / System Architecture — ACL Auditor

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher a arquitetura em português</summary><br>

# ACL Auditor — Arquitetura de Sistema e Especificações Técnicas

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Padrão:** C++20 | Qt 6.11.1 | Win32 Security API | SQLite3  
**Plataforma Alvo:** Windows 11 / Windows 10 / Windows Server 2022+ (x64)  
**Autor:** Fernando Nillsson Cidade  

---

## 1. Filosofia Arquitetural

O ACL Auditor foi projetado desde o início como uma solução de auditoria de segurança **100% Local-First, estritamente somente leitura e de alto desempenho** para o ecossistema Windows:

```
+-------------------------------------------------------------------------+
|                           CAMADA DE APRESENTAÇÃO                        |
|                     (Qt 6.11.1 Widgets, DWM 11 Dark / Mica)             |
|                                                                         |
|  MainWindow  |  VisualExplanationWidget  |  RiskDashboardWidget         |
|  ServicesSharesWidget  |  SnapshotDiffWidget  |  ReportExportDialog     |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                            MODELOS VIRTUAIS                             |
|                       (Qt Model/View QAbstractItemModel)                |
|                                                                         |
|    AclTreeModel   |   AceTableModel   |   RiskTableModel   | DiffModel  |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                             MOTOR CENTRAL                               |
|                        (ACLAuditorEngine - C++20)                       |
|                                                                         |
|  +------------------------+  +---------------------------------------+  |
|  |  NtfsSecurityUtils     |  |  EffectiveAccessCalculator            |  |
|  |  - GetNamedSecurityInfo|  |  - Simulação Authz                    |  |
|  |  - Ordem Canônica ACEs |  |  - Expansão de Grupos de Token / SIDs|  |
|  |  - Resolução SDDL      |  |  - Árvore Causal Explicativa          |  |
|  +------------------------+  +---------------------------------------+  |
|  +------------------------+  +---------------------------------------+  |
|  |  RiskScanner           |  |  SnapshotEngine                       |  |
|  |  - Pastas World-Writable| |  - Persistência Embutida SQLite3      |  |
|  |  - Binary Planting/DLL |  |  - Assinatura Hash SHA-256 (CNG)      |  |
|  |  - Privilégios Excessivos| - Motor de Diff Temporal e Deriva     |  |
|  +------------------------+  +---------------------------------------+  |
|  +------------------------+  +---------------------------------------+  |
|  |  ServiceAuditor & SMB  |  |  RemediationPlanner                   |  |
|  |  - Caminhos sem aspas  |  |  - Scripts Seguros PowerShell (.ps1)  |  |
|  |  - DACL Compartilhamento| |  - Rotinas icacls /save e /restore    |  |
|  +------------------------+  +---------------------------------------+  |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                         APIS NATIVAS DO WINDOWS                         |
|                                                                         |
|      advapi32.dll  |  authz.dll  |  netapi32.dll  |  bcrypt.dll         |
|      shlwapi.dll   |  dwmapi.dll |  kernel32.dll  |  user32.dll         |
+-------------------------------------------------------------------------+
```

---

## 2. Decomposição em Camadas

### 2.1. Núcleo de Segurança C++20 (`source/engine/`)
Compilado em uma biblioteca estática independente (`ACLAuditorEngine`), sem acoplamento com bibliotecas gráficas:
- **`AclTypes.hpp`:** Definições dos tipos de domínio para SIDs, ACEs, descritores de segurança, severidade de riscos e snapshots.
- **`NtfsSecurityUtils`:** Wrappers Win32 de baixo nível para recuperação de descritores de segurança (`GetNamedSecurityInfoW`), DACLs, resolução de SIDs (`LookupAccountSidW`), habilitação de privilégios (`SeSecurityPrivilege`) e hash criptográfico SHA-256 via Windows CNG (`BCryptCreateHash`).
- **`EffectiveAccessCalculator`:** Avalia o acesso efetivo combinando simulação de tokens com a ordem de precedência canônica do Windows NT, construindo a árvore genealógica de acesso.
- **`RiskScanner`:** Motor heurístico de conformidade que calcula o score de risco (0 a 100) e detecta vulnerabilidades críticas (pastas World-Writable, Binary Planting, DLL Hijacking, `WRITE_DAC` e contas órfãs).
- **`ServiceAuditor`:** Inspeciona o Service Control Manager (`OpenSCManagerW`, `EnumServicesStatusExW`) detectando caminhos sem aspas com espaços e binários modificáveis por usuários comuns.
- **`SmbShareAuditor`:** Audita compartilhamentos SMB via `NetShareEnum` (nível 502) e correlaciona permissões de rede com as DACLs NTFS locais.
- **`SnapshotEngine`:** Gerencia a persistência no banco SQLite3 embutido, assinaturas SHA-256 e análise de deltas temporais.
- **`RemediationPlanner`:** Gera scripts auditáveis em PowerShell e batch com backup prévio obrigatório (`icacls /save`) e script de reversão imediata (`icacls /restore`).

### 2.2. Modelos de Dados Virtualizados (`source/model/`)
Projetados para suportar volumes corporativos com centenas de milhares de arquivos sem travamento da interface:
- **`AclTreeModel`:** Modelo hierárquico virtualizado com carregamento sob demanda (lazy loading).
- **`AceTableModel`:** Formata e exibe cada entrada ACE com decodificação de máscaras e herança.
- **`RiskTableModel`:** Organiza e categoriza os riscos detectados por criticidade.
- **`DiffTableModel`:** Apresenta deltas comparativos entre snapshots.

### 2.3. Camada de Apresentação (`source/ui/`)
- Desenvolvida em Qt 6.11.1 Widgets com suporte nativo ao Modo Escuro e efeitos Mica do Windows 11 (`Win32DwmHelper`, `ThemeManager`).
- Alternância dinâmica de idiomas (Português BR / Inglês US) em tempo de execução sem reiniciar a aplicação (`GerenciadorTraducao`).

---

## 3. Armazenamento Local

- **Banco de Dados Local:** SQLite3 embutido.
- **Localização:** `%LOCALAPPDATA%\ACLAuditor\ACLAuditor\acl_auditor_snapshots.db`
- **Tabelas Principais:** `snapshots`, `snapshot_nodes` e `snapshot_findings`.

---

## 4. Métricas de Desempenho

- **Consumo de Memória:** Menor que 85 MB RAM em árvores com mais de 200.000 objetos.
- **Taxa de Processamento:** Até 15.000 descritores de segurança por segundo em discos NVMe.
- **Tráfego de Rede:** 0 bytes transmitidos externamente durante toda a execução.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the architecture in English</summary><br>

# ACL Auditor — System Architecture & Technical Specifications

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Standard:** C++20 | Qt 6.11.1 | Win32 Security API | SQLite3  
**Target Platform:** Windows 11 / Windows 10 / Windows Server 2022+ (x64)  
**Author:** Fernando Nillsson Cidade  

---

## 1. Architectural Philosophy

ACL Auditor is engineered from the ground up as a **high-performance, Local-First, strictly read-only** Windows security auditing solution:

```
+-------------------------------------------------------------------------+
|                              PRESENTATION LAYER                         |
|                     (Qt 6.11.1 Widgets, DWM 11 Dark / Mica)             |
|                                                                         |
|  MainWindow  |  VisualExplanationWidget  |  RiskDashboardWidget         |
|  ServicesSharesWidget  |  SnapshotDiffWidget  |  ReportExportDialog     |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                              VIRTUALIZED MODELS                         |
|                       (Qt Model/View QAbstractItemModel)                |
|                                                                         |
|    AclTreeModel   |   AceTableModel   |   RiskTableModel   | DiffModel  |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                              CORE ENGINE                                |
|                        (ACLAuditorEngine - C++20)                       |
|                                                                         |
|  +------------------------+  +---------------------------------------+  |
|  |  NtfsSecurityUtils     |  |  EffectiveAccessCalculator            |  |
|  |  - GetNamedSecurityInfo|  |  - Authz Simulation                   |  |
|  |  - Canonical Ordering  |  |  - Token & SID Group Expansion        |  |
|  |  - SDDL / Inheritance  |  |  - Causal Explanation Tree           |  |
|  +------------------------+  +---------------------------------------+  |
|  +------------------------+  +---------------------------------------+  |
|  |  RiskScanner           |  |  SnapshotEngine                       |  |
|  |  - World-Writable      |  |  - SQLite3 Embedded Storage           |  |
|  |  - Binary Planting/DLL |  |  - SHA-256 Forensic Baseline Hashing  |  |
|  |  - Excessive Privilege |  |  - Temporal Delta Diff Engine         |  |
|  +------------------------+  +---------------------------------------+  |
|  +------------------------+  +---------------------------------------+  |
|  |  ServiceAuditor & SMB  |  |  RemediationPlanner                   |  |
|  |  - Unquoted Paths      |  |  - Non-destructive PowerShell (.ps1)  |  |
|  |  - Share DACL vs NTFS  |  |  - icacls /save & /restore rollback   |  |
|  +------------------------+  +---------------------------------------+  |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                          NATIVE WINDOWS OS APIS                         |
|                                                                         |
|      advapi32.dll  |  authz.dll  |  netapi32.dll  |  bcrypt.dll         |
|      shlwapi.dll   |  dwmapi.dll |  kernel32.dll  |  user32.dll         |
+-------------------------------------------------------------------------+
```

---

## 2. Layer Decomposition

### 2.1. Core Engine (`source/engine/`)
The core engine is compiled into a standalone static library (`ACLAuditorEngine`) that does not depend on graphical user interface components:
- **`AclTypes.hpp`:** Fundamental domain representations for SIDs, ACE entries, security descriptors, risk classifications, snapshot records, and remediation scripts.
- **`NtfsSecurityUtils`:** Low-level Win32 wrappers for security descriptor retrieval (`GetNamedSecurityInfoW`), DACL enumeration, SID-to-name lookup (`LookupAccountSidW`), privilege elevation (`SeSecurityPrivilege`, `SeBackupPrivilege`), and CNG SHA-256 hashing (`BCryptCreateHash`).
- **`EffectiveAccessCalculator`:** Evaluates effective permissions by combining token simulation with canonical ACE evaluation order, constructing a hierarchical explanation tree.
- **`RiskScanner`:** Heuristic analysis engine that computes security scores (0–100) and detects dangerous permission combinations (World-Writable, Binary Planting, DLL Hijacking, excessive `WRITE_DAC` / `WRITE_OWNER`, and orphaned SIDs).
- **`ServiceAuditor`:** Scans the Windows Service Control Manager (`OpenSCManagerW`, `EnumServicesStatusExW`) to detect unquoted executable paths with spaces and writable service binaries.
- **`SmbShareAuditor`:** Audits SMB network shares via `NetShareEnum` (level 502) and correlates share permissions with local NTFS DACLs.
- **`SnapshotEngine`:** Manages snapshot persistence in an embedded SQLite3 database, supporting cryptographic baseline hashing and delta computation.
- **`RemediationPlanner`:** Generates auditable PowerShell and batch (`icacls`) scripts with mandatory pre-change backup (`icacls /save`) and instant rollback (`icacls /restore`).

### 2.2. Virtualized Data Models (`source/model/`)
Designed to handle enterprise file systems with millions of entries without memory exhaustion or UI freezes:
- **`AclTreeModel`:** Virtualized hierarchical tree model implementing lazy on-demand node loading.
- **`AceTableModel`:** Formats and presents individual Access Control Entries with color coding and permission decoding.
- **`RiskTableModel`:** Categorizes and sorts detected vulnerabilities by severity.
- **`DiffTableModel`:** Visualizes snapshot deltas (added, modified, removed permissions, and ownership changes).

### 2.3. Presentation Layer (`source/ui/`)
- Built with Qt 6.11.1 Widgets, styled with native Windows 11 Dark Mode and Mica accent styling (`Win32DwmHelper`, `ThemeManager`).
- Dynamic runtime language switching between English (US) and Portuguese (BR) without application restart (`GerenciadorTraducao`).

---

## 3. Storage Architecture

Snapshots are stored locally in an embedded SQLite database:
- **Path:** `%LOCALAPPDATA%\ACLAuditor\ACLAuditor\acl_auditor_snapshots.db`
- **Schema:**
  - `snapshots`: Metadata, timestamp, root path, node count, risk score, and SHA-256 fingerprint.
  - `snapshot_nodes`: Path, owner SID, inheritance flags, and serialized DACL representations.
  - `snapshot_findings`: Detected risks and severity ratings.

---

## 4. Key Performance Metrics

- **Memory Efficiency:** Lazy loading consumes less than 85 MB RAM when navigating directory trees with >200,000 files.
- **Scanning Throughput:** Win32 direct API calls process up to 15,000 security descriptors per second on modern NVMe storage.
- **Zero Network Traffic:** 0 bytes transmitted over any network socket during operation.

</details>
