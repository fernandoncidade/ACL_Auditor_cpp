# ACL Auditor — Windows NTFS & SMB Permission Auditor

**Windows NTFS and SMB permission auditor with effective-access analysis, security risk detection, snapshots, and HTML/PDF reports. Built with C++20, Qt 6, and CMake.**

[![Latest Release](https://img.shields.io/badge/Release-v2026.11.6.0-blue.svg)](https://github.com/fernandoncidade/ACL_Auditor_cpp/releases/latest)
[![Microsoft Store](https://img.shields.io/badge/Microsoft_Store-Available-0078D4?logo=microsoft)](https://apps.microsoft.com/detail/9NX5XZLL8WXW)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=c%2B%2B)](CMakeLists.txt)
[![Qt 6.11.1](https://img.shields.io/badge/Qt-6.11.1-41CD52?logo=qt)](CMakeLists.txt)
[![Platform](https://img.shields.io/badge/Platform-Windows_11_%2F_10_%2F_Server-0078D4?logo=windows)](https://microsoft.com)
[![License AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](LICENSE)
[![100% Offline](https://img.shields.io/badge/Security-100%25_Offline-success.svg)](docs/security-model.md)

<p align="center">
  <a href="#english-us">🇺🇸 English</a> •
  <a href="#português-br">🇧🇷 Português</a> •
  <a href="https://apps.microsoft.com/detail/9NX5XZLL8WXW">🛒 Microsoft Store</a> •
  <a href="docs/">📖 Docs</a> •
  <a href="#-quick-start">🚀 Quick Start</a> •
  <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/discussions">💬 Discussions</a> •
  <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/issues">🐛 Issues</a>
</p>

---

<p align="center">
  <img src="docs/images/banner.png" alt="ACL Auditor Banner" width="900" style="border-radius: 8px; box-shadow: 0 4px 16px rgba(0,0,0,0.2);">
</p>

---

## <a id="english-us"></a>🇺🇸 English (US)

<details>
<summary>Clique para expandir / recolher</summary><br>

**ACL Auditor** is an enterprise-grade, native Windows desktop application and CLI tool engineered in **C++20** and **Qt 6.11.1** for systems administrators, SecOps engineers, IT auditors, and cybersecurity consultancies.

Unlike legacy command-line tools (such as `icacls` or `accesschk`) that output raw, unstructured ACL dumps, ACL Auditor is purpose-built to answer the fundamental question in Windows file system governance:

> **“Why does this specific user have permission to read, modify, or execute this file?”**

```text
Identity: john.doe (S-1-5-21-100234-...)
 └── Group Membership: IT-Support (S-1-5-21-445892-...)
      └── Nested Group: Infra-Operations
           └── Rule Inheritance: Inherited from D:\Departments
                └── Effective Access: MODIFY (FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE)
```

### Core Architecture Highlights
- 🔒 **100% Autonomous & Offline:** Operates strictly locally with zero external network connections, zero telemetry, and zero third-party cloud dependencies.
- 🛡️ **Strictly Read-Only Analysis:** File system inspection, service queries, and SMB share audits never modify security descriptors or metadata.
- 🔄 **Safe Remediation with Rollback:** Remediation plans generate auditable scripts enforcing mandatory backups (`icacls /save`) and guaranteed rollback (`icacls /restore`).

---

### 🖼️ Visual Interface Showcase

Below are previews of the running application interface (Windows 11 Dark Theme). Click any thumbnail to view the full-resolution screenshot:

<table>
  <tr>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210219.png" target="_blank">
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210219.png" alt="NTFS Permissions Explorer" width="420"><br>
        <sub><b>1. Virtualized NTFS Permissions Explorer</b><br>Asynchronous directory tree, ownership, inheritance, and raw/decoded ACE rules.</sub>
      </a>
    </td>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210237.png" target="_blank">
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210237.png" alt="Effective Access Explanation Tree" width="420"><br>
        <sub><b>2. Effective Access Explanation Tree</b><br>Token simulation explaining causal justification, group nesting, and inheritance chains.</sub>
      </a>
    </td>
  </tr>
  <tr>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210255.png" target="_blank">
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210255.png" alt="Risk & Compliance Dashboard" width="420"><br>
        <sub><b>3. Risk & Compliance Dashboard</b><br>Vulnerability categorization, world-writable directories, orphaned SIDs, and remediation generator.</sub>
      </a>
    </td>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210318.png" target="_blank">
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210318.png" alt="Windows Services Security Audit" width="420"><br>
        <sub><b>4. Windows Services Security Audit</b><br>Detection of unquoted service paths, writable service executables, and privilege escalation risks.</sub>
      </a>
    </td>
  </tr>
  <tr>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210348.png" target="_blank">
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210348.png" alt="Network Shares (SMB) Audit" width="420"><br>
        <sub><b>5. Network Shares (SMB) Audit</b><br>SMB share enumeration correlating share DACLs with NTFS permissions to find permission conflicts.</sub>
      </a>
    </td>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210501.png" target="_blank">
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-16%20210501.png" alt="Temporal Comparison (Snapshots)" width="420"><br>
        <sub><b>6. Temporal Comparison (Snapshots)</b><br>Baseline vs. current snapshot diff engine, forensic change detection, and signed evidence export.</sub>
      </a>
    </td>
  </tr>
</table>

---

### Feature Matrix

| Capability / Feature | Status | Description |
|---|:---:|---|
| **Virtualized NTFS Permissions Explorer** | ✅ | Asynchronous virtual tree capable of rendering volumes with 200,000+ files smoothly. |
| **Visual Effective Access Explainer** | ✅ | Token simulation resolving primary, local, domain, and universal SIDs (`Everyone`, `Authenticated Users`). |
| **Heuristic Risk & Compliance Scanner** | ✅ | Flags World-Writable folders, Binary Planting, DLL Hijacking, excessive `WRITE_DAC`/`WRITE_OWNER` & orphaned SIDs. |
| **Windows Services & SMB Shares Audit** | ✅ | Uncovers Unquoted Service Paths, writable service executables, and share DACL vs. NTFS discrepancies. |
| **Historical Snapshots & Temporal Diff** | ✅ | Embedded SQLite3 storage with SHA-256 forensic hashing and automated compliance drift detection. |
| **Multi-Format Executive Reporting** | ✅ | Interactive HTML dashboards, formatted executive PDF (A4 with signature lines), CSV, and SIEM JSON. |
| **Non-Destructive Remediation Planner** | ✅ | Generates ready-to-run `.ps1` and `.bat` (`icacls`) scripts with guaranteed rollback. |
| **Windows Explorer Shell Integration** | ✅ | Right-click context menu integration registered in `HKCU` without requiring admin rights. |
| **Standalone Headless CLI (`acl-auditor-cli`)** | ✅ | Scriptable console tool for pipelines, DevOps, and headless server environments. |
| **Windows 11 Native Theme & Dark Mode** | ✅ | DWM Mica accent rendering and dynamic runtime bilingual translation (EN / PT). |

---

### 🚀 Quick Start

#### Requirements:
- Windows 11 / Windows 10 / Windows Server 2022+ (x64)
- CMake 3.21+
- Qt 6.x (Core, Gui, Widgets, Sql, PrintSupport)
- Visual Studio 2022 (MSVC v143) or MinGW GCC 13+
- Ninja build tool (recommended)

#### Build with CMake Presets:

```powershell
# 1. Clone the repository
git clone https://github.com/fernandoncidade/ACL_Auditor_cpp.git
cd ACL_Auditor_cpp

# 2. Configure and build with MSVC (Visual Studio 2022)
cmake --preset host-qt6-msvc-release
cmake --build --preset host-qt6-msvc-release

# 3. Run automated test suite
ctest --preset host-qt6-msvc-release --output-on-failure
```

#### Build with MinGW:

```powershell
cmake --preset host-qt6-mingw-release
cmake --build --preset host-qt6-mingw-release
ctest --preset host-qt6-mingw-release --output-on-failure
```

---

### 💻 Standalone CLI Usage (`acl-auditor-cli.exe`)

The headless CLI tool enables automated security checks inside CI/CD pipelines and administrative scripts:

```powershell
# 1. Scan directory for NTFS security risks (JSON output for automation)
acl-auditor-cli.exe scan "C:\inetpub\wwwroot" --format json > report.json

# 2. Check effective access for a user on a sensitive file
acl-auditor-cli.exe access "D:\Finance\Payroll.xlsx" --user "CORP\Accounting"

# 3. Audit Windows services for Unquoted Service Paths
acl-auditor-cli.exe services --risks-only

# 4. Audit SMB network shares
acl-auditor-cli.exe shares
```

> **Tip:** When installed via Microsoft Store or MSIX package with `AppExecutionAlias`, the command `acl-auditor-cli` can be invoked globally from any PowerShell or Command Prompt window without modifying `%PATH%`.

See the [CLI Reference](docs/cli-reference.md) for complete options and pipeline exit codes.

---

### 📚 Modular Documentation

- 🏛️ [System Architecture](docs/architecture.md) — Engine design, virtualized models, and Win32 security APIs.
- 🛡️ [Security Model & Threat Analysis](docs/security-model.md) — Canonical ACE precedence, read-only guarantees, and risk heuristics.
- 📋 [Regulatory Compliance Mappings](docs/compliance.md) — CIS Benchmarks, NIST SP 800-53, ISO/IEC 27001, and GDPR/LGPD.
- ⚖️ [Licensing & Commercial Clarification](docs/licensing.md) — GNU AGPLv3 open-source terms, Microsoft Store packaging, and rights grant.
- 📖 [User Operations Guide](docs/user-guide.md) — End-user instructions, scanning workflows, snapshots, and reports.
- ⚙️ [CLI Reference](docs/cli-reference.md) — Command syntax, JSON schemas, and automation parameters.
- 📦 [Release Notes v2026.11.6.0](docs/release-v2026.11.6.0.md) — Hashes (SHA-256), release assets, and changelog.

---

### ⚖️ Open Source License & Commercial Distribution

The source code in this repository is 100% licensed under the **[GNU Affero General Public License v3 (AGPLv3)](LICENSE)**.

> **Rights Holder Statement:**  
> I, **Fernando Nillsson Cidade**, as the sole creator and copyright owner of the **ACL Auditor** project, license the source code of this software under the terms of the GNU AGPLv3, granting any person or organization the freedom to use, study, copy, modify, and redistribute it under the same copyleft terms.

- **Open Source (GNU AGPLv3):** The full codebase is free to fork, build, study, and adapt in accordance with the AGPLv3.
- **Documentation in `source/assets/`:** Created specifically for the Microsoft Store release. Developers and forks have complete freedom to alter, delete, add, or customize this documentation for their own platforms and target markets, provided they comply with their local laws and platform regulations.
- **Official Microsoft Store Edition:** The official pre-compiled and digitally signed binary distributed by the author on the [Microsoft Store](https://apps.microsoft.com/detail/9NX5XZLL8WXW) is a turnkey commercial package. Third parties are **not authorized** to commercially resell, redistribute, or reverse-engineer the author's official compiled binaries (anyone wishing to distribute builds may compile their own binaries from the open source code).
- Complete details: [docs/licensing.md](docs/licensing.md).

Version: v2026.11.6.0<br>
Technical revision date: November 6, 2026<br>
Author: Fernando Nillsson Cidade<br>

</details>

---

## <a id="português-br"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher</summary><br>

**ACL Auditor** é uma aplicação corporativa nativa para Windows e utilitário CLI de alto desempenho, desenvolvida em **C++20** e **Qt 6.11.1** para administradores de rede, equipes de SecOps, peritos forenses, auditores e consultorias de governança de TI.

Diferente de utilitários legados de linha de comando (como `icacls` ou `accesschk`), que apenas geram despejos brutos e desestruturados de ACLs, o ACL Auditor foi concebido para responder à dúvida fundamental na gestão de privilégios do sistema de arquivos Windows:

> **“Por que este usuário específico possui permissão para ler, modificar ou executar este arquivo?”**

```text
Identidade: joao.silva (S-1-5-21-100234-...)
 └── Associação a Grupos: Suporte-TI (S-1-5-21-445892-...)
      └── Grupo Aninhado: Operacoes-Infra
           └── Herança de Regras: Herdado de D:\Departamentos
                └── Acesso Efetivo: MODIFICAR (FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE)
```

### Pilares Arquiteturais Principais
- 🔒 **100% Autônomo e Offline:** Opera estritamente no host local, sem conexões externas à rede, sem telemetria e sem dependências de nuvem.
- 🛡️ **Análise Estritamente Somente Leitura:** Varreduras no sistema de arquivos, consultas de serviços e auditorias SMB nunca alteram descritores de segurança ou metadados.
- 🔄 **Remediação Segura com Rollback:** Os planos de remediação geram scripts auditáveis com backup prévio obrigatório (`icacls /save`) e script de reversão imediata (`icacls /restore`).

---

### 🖼️ Galeria Visual da Interface

Abaixo estão as capturas de tela do aplicativo em execução (Tema Escuro nativo do Windows 11). Clique em qualquer miniatura para abrir a imagem em resolução máxima:

<table>
  <tr>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20210931.png" target="_blank">
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20210931.png" alt="Explorador de Permissões NTFS" width="420"><br>
        <sub><b>1. Explorador Virtualizado de Permissões NTFS</b><br>Navegação assíncrona por pastas, proprietário, herança de regras e detalhamento de ACEs com máscaras hexadecimais.</sub>
      </a>
    </td>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20210943.png" target="_blank">
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20210943.png" alt="Árvore Explicativa de Acesso Efetivo" width="420"><br>
        <sub><b>2. Árvore Explicativa de Acesso Efetivo</b><br>Simulação de token explicando causalidade, grupos aninhados e cadeia hierárquica de resolução.</sub>
      </a>
    </td>
  </tr>
  <tr>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20210954.png" target="_blank">
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20210954.png" alt="Dashboard de Riscos e Conformidade" width="420"><br>
        <sub><b>3. Painel de Riscos e Conformidade</b><br>Categorização de severidade, pastas com escrita pública, SIDs órfãos e geração de plano de remediação.</sub>
      </a>
    </td>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20211005.png" target="_blank">
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20211005.png" alt="Auditoria de Serviços do Windows" width="420"><br>
        <sub><b>4. Auditoria de Serviços do Windows</b><br>Detecção de caminhos sem aspas (Unquoted Service Paths), binários inseguros e vetores de elevação de privilégios.</sub>
      </a>
    </td>
  </tr>
  <tr>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20211019.png" target="_blank">
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20211019.png" alt="Auditoria de Compartilhamentos de Rede (SMB)" width="420"><br>
        <sub><b>5. Auditoria de Compartilhamentos de Rede (SMB)</b><br>Enumeração de compartilhamentos correlacionando DACLs de rede com NTFS para identificar brechas de acesso.</sub>
      </a>
    </td>
    <td align="center" width="50%">
      <a href="https://github.com/fernandoncidade/ACL_Auditor_cpp/blob/main/repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20211032.png" target="_blank">
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-16%20211032.png" alt="Comparação Temporal (Snapshots)" width="420"><br>
        <sub><b>6. Comparação Temporal (Snapshots)</b><br>Motor forense de comparação temporal (baseline vs. atual), detecção de alterações e exportação de evidências.</sub>
      </a>
    </td>
  </tr>
</table>

---

### Matriz de Recursos

| Recurso / Funcionalidade | Status | Descrição |
|---|:---:|---|
| **Explorador Virtualizado de Permissões NTFS** | ✅ | Árvore virtual assíncrona capaz de navegar em volumes com mais de 200.000 arquivos sem congelar a interface. |
| **Explicador Visual de Acesso Efetivo** | ✅ | Simulação de token resolvendo SIDs primários, locais, de domínio e universais (`Todos`, `Usuários Autenticados`). |
| **Varredura Heurística de Riscos e Conformidade** | ✅ | Identifica pastas com escrita aberta (World-Writable), Binary Planting, DLL Hijacking, `WRITE_DAC`/`WRITE_OWNER` excessivos e SIDs órfãos. |
| **Auditoria de Serviços do Windows e Compartilhamentos SMB** | ✅ | Detecta Unquoted Service Paths, binários de serviços vulneráveis e discrepâncias entre permissões de compartilhamento e NTFS. |
| **Snapshots Históricos e Comparação Temporal (Diff)** | ✅ | Armazenamento SQLite3 embutido com hash forense SHA-256 e detecção automatizada de desvio de conformidade (*drift*). |
| **Relatórios Executivos Multiformato** | ✅ | Painéis HTML interativos, relatórios executivos em PDF (A4 formatado com campos de assinatura), CSV e JSON para SIEM. |
| **Planejador de Remediação Não Destrutivo** | ✅ | Gera scripts prontos em `.ps1` e `.bat` (`icacls`) com backup obrigatório e script de reversão imediata (*rollback*). |
| **Integração com o Menu do Windows Explorer** | ✅ | Menu de contexto pelo botão direito registrado em `HKCU`, sem necessidade de privilégios de administrador. |
| **Utilitário CLI Standalone Headless (`acl-auditor-cli`)** | ✅ | Ferramenta de linha de comando para automação em pipelines de CI/CD, DevOps e ambientes de servidor sem interface gráfica. |
| **Tema Nativo do Windows 11 e Modo Escuro** | ✅ | Suporte a estilos visuais modernos com realce DWM Mica e alternância dinâmica de idioma (Inglês / Português). |

---

### 🚀 Início Rápido

#### Requisitos:
- Windows 11 / Windows 10 / Windows Server 2022+ (x64)
- CMake 3.21+
- Qt 6.x (Core, Gui, Widgets, Sql, PrintSupport)
- Visual Studio 2022 (MSVC v143) ou MinGW GCC 13+
- Ninja build tool (recomendado)

#### Compilação com CMake Presets:

```powershell
# 1. Clonar o repositório
git clone https://github.com/fernandoncidade/ACL_Auditor_cpp.git
cd ACL_Auditor_cpp

# 2. Configurar e compilar com MSVC (Visual Studio 2022)
cmake --preset host-qt6-msvc-release
cmake --build --preset host-qt6-msvc-release

# 3. Executar a suíte de testes automatizados
ctest --preset host-qt6-msvc-release --output-on-failure
```

#### Compilação com MinGW:

```powershell
cmake --preset host-qt6-mingw-release
cmake --build --preset host-qt6-mingw-release
ctest --preset host-qt6-mingw-release --output-on-failure
```

---

### 💻 Uso da Linha de Comando Standalone (`acl-auditor-cli.exe`)

A ferramenta CLI headless permite verificações automatizadas de segurança dentro de esteiras de CI/CD e rotinas administrativas:

```powershell
# 1. Realizar varredura de diretório em busca de riscos NTFS (saída JSON para automação)
acl-auditor-cli.exe scan "C:\inetpub\wwwroot" --format json > relatorio.json

# 2. Consultar o acesso efetivo de um usuário específico em um arquivo sensível
acl-auditor-cli.exe access "D:\Financeiro\Folha.xlsx" --user "CORP\Contabilidade"

# 3. Auditar serviços do Windows em busca de caminhos sem aspas (Unquoted Service Paths)
acl-auditor-cli.exe services --risks-only

# 4. Auditar compartilhamentos de rede SMB
acl-auditor-cli.exe shares
```

> **Dica:** Quando instalado via pacote MSIX da Microsoft Store com `AppExecutionAlias`, o utilitário pode ser executado diretamente em qualquer janela do PowerShell, CMD ou Windows Terminal digitando apenas `acl-auditor-cli`.

Consulte a [Referência da CLI](docs/cli-reference.md) para opções completas e códigos de saída para pipelines.

---

### 📚 Documentação Técnica Modular

- 🏛️ [Arquitetura do Sistema](docs/architecture.md) — Design do motor, modelos virtualizados e integração com as APIs de segurança do Win32.
- 🛡️ [Modelo de Segurança e Análise de Ameaças](docs/security-model.md) — Precedência canônica de ACEs, garantias de somente leitura e heurísticas de risco.
- 📋 [Mapeamento de Conformidade Regulatória](docs/compliance.md) — Benchmarks CIS, NIST SP 800-53, ISO/IEC 27001 e GDPR/LGPD.
- ⚖️ [Licenciamento e Esclarecimento Comercial](docs/licensing.md) — Termos da Licença GNU AGPLv3, empacotamento para a Microsoft Store e concessão de direitos.
- 📖 [Guia de Operação do Usuário](docs/user-guide.md) — Instruções para o usuário final, fluxos de varredura, snapshots e relatórios.
- ⚙️ [Referência da Linha de Comando](docs/cli-reference.md) — Sintaxe dos comandos, esquemas JSON e parâmetros para automação.
- 📦 [Notas de Versão v2026.11.6.0](docs/release-v2026.11.6.0.md) — Hashes forenses (SHA-256), binários de lançamento e changelog.
- 📄 [Manual do Usuário em Português](MANUAL.md) & [Observações Técnicas](OBSERVACAO.md) — Documentação estendida em língua portuguesa.

---

### ⚖️ Licença de Código Aberto e Distribuição Comercial

O código-fonte deste repositório é 100% licenciado sob a **[GNU Affero General Public License v3 (AGPLv3)](LICENSE)**.

> **Declaração do Titular dos Direitos:**  
> Eu, **Fernando Nillsson Cidade**, na qualidade de único criador e detentor dos direitos autorais do projeto **ACL Auditor**, licencio o código-fonte deste software sob os termos da GNU AGPLv3, concedendo a qualquer pessoa ou organização a liberdade de usar, estudar, copiar, modificar e redistribuir o software sob os mesmos termos copyleft da licença.

- **Código Aberto (GNU AGPLv3):** O código-fonte integral é livre para bifurcação (fork), compilação, estudo e adaptação sob os termos da AGPLv3.
- **Documentação em `source/assets/`:** Desenvolvida especificamente para a publicação na Microsoft Store. Desenvolvedores e mantenedores de forks têm total liberdade para alterar, excluir, adicionar ou personalizar essa documentação para seus próprios ecossistemas e plataformas, desde que respeitem as diretrizes e leis de seus respectivos países.
- **Edição Oficial da Microsoft Store:** O binário pré-compilado e assinado digitalmente distribuído oficialmente pelo autor na [Microsoft Store](https://apps.microsoft.com/detail/9NX5XZLL8WXW) é um pacote comercial pronto para uso. Terceiros **não possuem autorização** para comercializar, redistribuir ou realizar engenharia reversa sobre os binários oficiais compilados pelo autor (qualquer pessoa ou organização que deseje distribuir binários pode compilá-los livremente a partir do código-fonte aberto).
- Detalhes completos: [docs/licensing.md](docs/licensing.md).

Versão: v2026.11.6.0<br>
Data técnica desta revisão: 6 de novembro de 2026<br>
Autor: Fernando Nillsson Cidade<br>

</details>

---

## 🤝 Contributing & Community / Contribuição e Comunidade

> [!NOTE]
> **Sobre o Desenvolvimento e Contribuições Externas / Project Maintenance & Contributions:**  
> 🇧🇷 *Este projeto é desenvolvido primariamente como uma iniciativa pessoal e educacional pelo autor, que é iniciante e está em contínuo aprendizado em programação. Sugestões, issues e PRs são bem-vindos, mas a análise técnica e incorporação de código ocorrem voluntariamente no ritmo do autor. Caso não haja resposta imediata ou merge, não reaja com alarde; a licença GNU AGPLv3 garante total liberdade para você evoluir o projeto em seu próprio fork.*  
> 🇺🇸 *This project is developed primarily as a personal and educational initiative by the author, who is a beginner and in continuous software engineering learning. Suggestions, issues, and PRs are welcome, but technical reviews and upstream merges occur voluntarily at the author's own pace. If there is no immediate response or merge, please do not be alarmed; the GNU AGPLv3 license guarantees complete freedom to evolve the project in your own fork.*

Contributions are welcome! / Contribuições são muito bem-vindas!  
Please read our [Contributing Guide](CONTRIBUTING.md) and [Code of Conduct](CODE_OF_CONDUCT.md). / Por favor, consulte nosso [Guia de Contribuição](CONTRIBUTING.md) e [Código de Conduta](CODE_OF_CONDUCT.md).

- 💡 Have an idea or question? Open a topic in [GitHub Discussions](https://github.com/fernandoncidade/ACL_Auditor_cpp/discussions)! / Tem uma ideia ou dúvida? Abra um tópico no [GitHub Discussions](https://github.com/fernandoncidade/ACL_Auditor_cpp/discussions)!
- 🐛 Found a bug? Use our [Bug Report Template](https://github.com/fernandoncidade/ACL_Auditor_cpp/issues/new?template=bug_report.md). / Encontrou um problema? Utilize nosso [Modelo de Relato de Bug](https://github.com/fernandoncidade/ACL_Auditor_cpp/issues/new?template=bug_report.md).
- 🛡️ Security vulnerability? Report privately via [GitHub Security Advisories](https://github.com/fernandoncidade/ACL_Auditor_cpp/security/advisories/new). / Vulnerabilidade de segurança? Reporte com privacidade via [GitHub Security Advisories](https://github.com/fernandoncidade/ACL_Auditor_cpp/security/advisories/new).
