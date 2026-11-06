# MANUAL DO USUÁRIO E GUIA DE CONFORMIDADE NTFS — ACL AUDITOR
# USER MANUAL AND NTFS COMPLIANCE GUIDE — ACL AUDITOR

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Ambiente Tecnológico / Tech Stack:** C++20 | Qt 6.11.1 | Win32 Security API | SQLite3  
**Plataforma / Platform:** Windows 11 / Windows 10 / Windows Server 2022+ (x64)  
**Autor / Author:** Fernando Nillsson Cidade  

---

## Sumário / Table of Contents

- [PARTE 1: MANUAL DE INSTRUÇÕES (PORTUGUÊS - BR)](#parte-1-manual-de-instruções-português---br)
  - [1. Introdução & Arquitetura de Permissões NTFS](#1-introdução--arquitetura-de-permissões-ntfs)
  - [2. Navegador de Permissões NTFS & Árvore Virtual](#2-navegador-de-permissões-ntfs--árvore-virtual)
  - [3. Acesso Efetivo & Explicador Visual ("Por Que Tem Acesso?")](#3-acesso-efetivo--explicador-visual-por-que-tem-acesso)
  - [4. Scanner de Riscos & Matriz de Conformidade](#4-scanner-de-riscos--matriz-de-conformidade)
  - [5. Auditoria de Serviços Windows & Compartilhamentos SMB](#5-auditoria-de-serviços-windows--compartilhamentos-smb)
  - [6. Snapshots Históricos & Comparação Temporal Diff (SQLite)](#6-snapshots-históricos--comparação-temporal-diff-sqlite)
  - [7. Modo Consultoria & Geração de Relatórios Executivos](#7-modo-consultoria--geração-de-relatórios-executivos)
  - [8. Remediação Segura, Scripts Automatizados e Garantia de Rollback](#8-remediação-segura-scripts-automatizados-e-garantia-de-rollback)
  - [9. Integração com o Windows Explorer (Menu de Contexto)](#9-integração-com-o-windows-explorer-menu-de-contexto)
  - [10. Linha de Comando (CLI), Atalhos & FAQ](#10-linha-de-comando-cli-atalhos--faq)
- [PART 2: OPERATIONS MANUAL & COMPLIANCE GUIDE (ENGLISH - US)](#part-2-operations-manual--compliance-guide-english---us)
  - [1. Overview & NTFS Security Architecture](#1-overview--ntfs-security-architecture)
  - [2. NTFS Permissions Explorer & Virtual Tree](#2-ntfs-permissions-explorer--virtual-tree)
  - [3. Effective Access & Visual Explainer ("Why Does It Have Access?")](#3-effective-access--visual-explainer-why-does-it-have-access)
  - [4. Risk & Compliance Scanner](#4-risk--compliance-scanner)
  - [5. Windows Services & SMB Network Shares Audit](#5-windows-services--smb-network-shares-audit)
  - [6. Historical Snapshots & Temporal Diff Engine (SQLite)](#6-historical-snapshots--temporal-diff-engine-sqlite)
  - [7. Consulting Mode & Executive Multi-Format Reporting](#7-consulting-mode--executive-multi-format-reporting)
  - [8. Safe Remediation, Automated Scripts & Rollback Guarantee](#8-safe-remediation-automated-scripts--rollback-guarantee)
  - [9. Windows Explorer Shell Integration](#9-windows-explorer-shell-integration)
  - [10. Command Line Interface (CLI), Shortcuts & FAQ](#10-command-line-interface-cli-shortcuts--faq)

---

# PARTE 1: MANUAL DE INSTRUÇÕES (PORTUGUÊS - BR)

## 1. Introdução & Arquitetura de Permissões NTFS

O **ACL Auditor** é uma solução para inspeção, visualização, auditoria e geração de relatórios de segurança de sistemas de arquivos NTFS no ambiente Windows.

A aplicação foi construída em **C++20** utilizando as APIs de segurança de baixo nível do Windows (`Advapi32.dll`, `Authz.dll`, `Netapi32.dll`, `BCrypt.dll`) e interface em **Qt 6.11.1**, operando em **Modo 100% Autônomo, Local e Somente Leitura**.

### 1.1. Estrutura do Descritor de Segurança (Security Descriptor)
Todo objeto securitário no Windows possui um **Descritor de Segurança** composto por:
- **Owner (Proprietário):** SID da conta ou grupo proprietário. O proprietário possui o direito intrínseco de alterar permissões (`WRITE_DAC`), mesmo se a DACL negar seu acesso explícito.
- **DACL (Discretionary Access Control List):** Lista ordenada de entradas de controle de acesso (ACEs). Cada ACE concede (`ACCESS_ALLOWED_ACE_TYPE`) ou nega (`ACCESS_DENIED_ACE_TYPE`) direitos específicos a um SID.
- **SACL (System Access Control List):** Controla a geração de registros de auditoria no log de eventos de segurança do Windows.
- **Flags de Controle:**
  - `SE_DACL_PRESENT`: Indica a presença de uma lista de controle discricionária.
  - `SE_DACL_PROTECTED`: Indica que a herança da pasta pai foi bloqueada intencionalmente.

### 1.2. Algoritmo de Precedência Canônica de ACEs
O Windows NT avalia as entradas na seguinte ordem obrigatória:
1. **Regras Explícitas de NEGAÇÃO (DENY):** Têm prioridade absoluta.
2. **Regras Explícitas de CONCESSÃO (ALLOW).**
3. **Regras Herdadas de NEGAÇÃO (DENY):** Avaliadas do pai mais próximo até a raiz.
4. **Regras Herdadas de CONCESSÃO (ALLOW).**
5. **Negação Implícita (Default Deny):** Se nenhuma regra conceder o acesso solicitado, o acesso é sumariamente bloqueado.

---

## 2. Navegador de Permissões NTFS & Árvore Virtual

O Navegador de Permissões é a interface principal para inspeção visual da estrutura de arquivos e diretórios:

1. **Seleção de Caminho:**
   - Digite o caminho absoluto (ex: `C:\inetpub`, `D:\Dados_Empresa`) na barra de endereços superior ou clique em **Procurar...** (`Ctrl+O`).
   - Pressione **Enter** ou clique no botão **Escanear Permissões** (`F5`).
2. **Árvore Hierárquica Virtualizada:**
   - A árvore virtual carrega os nós sob demanda, garantindo alto desempenho mesmo em diretórios com centenas de milhares de arquivos.
   - Cada linha exibe:
     - **Objeto:** Nome do arquivo ou pasta com ícone do tipo.
     - **Proprietário:** Nome da conta proprietária resolvida pelo SAM ou Active Directory.
     - **Status de Herança:** Indicador visual se as permissões são herdadas da pasta pai ou se a herança está bloqueada (`SE_DACL_PROTECTED`).
     - **Nível de Risco:** Classificação calculada pela engine heurística (`Conforme`, `Baixo`, `Médio`, `Alto`, `Crítico`).
3. **Tabela de Entradas ACE:**
   - Ao clicar em qualquer nó da árvore, a tabela detalhada à direita apresenta todas as ACEs ativas:
     - **Tipo:** `Permitir (Allow)` em verde ou `Negar (Deny)` em vermelho.
     - **Entidade / SID:** Nome amigável e SID em formato SDDL textual.
     - **Direitos:** Controle Total (`GENERIC_ALL`), Modificar, Leitura e Execução, Gravação, ou Permissões Especiais.
     - **Origem da Herança:** Caminho absoluto da pasta pai que gerou a regra (recuperado via API `GetInheritanceSourceW`).
     - **Máscara Hexadecimal:** Código binário de 32 bits (ex: `0x001F01FF`).

---

## 3. Acesso Efetivo & Explicador Visual ("Por Que Tem Acesso?")

A funcionalidade mais importante do ACL Auditor é responder visual e formalmente à pergunta:

> **“Por que este usuário consegue ler, alterar ou executar este arquivo?”**

### 3.1. Operação do Explicador Visual
1. Selecione o arquivo ou pasta desejada na árvore e clique no botão azul **Por Que Tem Acesso?**.
2. Na aba de Acesso Efetivo, selecione a conta de usuário na lista de identidades locais/domínio ou digite manualmente o nome da conta ou SID.
3. Clique em **Explicar Cadeia de Acesso Efetivo**.
4. A engine do ACL Auditor realiza:
   - Resolução completa de todos os grupos primários, locais, globais e universais aos quais o usuário pertence;
   - Inclusão dos SIDs especiais do Windows: `Everyone` (S-1-1-0), `Authenticated Users` (S-1-5-11) e `BUILTIN\Users` (S-1-5-32-545);
   - Cálculo do acesso efetivo simulando o token de segurança no Security Reference Monitor (SRM).

### 3.2. Painel de Veredito e Árvore Causal
- **Badges de Veredito:** Apresentam o veredito instantâneo para os 5 direitos fundamentais:
  - `LEITURA` | `GRAVAÇÃO` | `EXECUÇÃO` | `EXCLUSÃO` | `ALTERAÇÃO DE PERMISSÕES`
- **Árvore de Linhagem de Acesso:**
  ```text
  Identidade Analisada: joao.silva (S-1-5-21-100234-...)
   └── Membro do Grupo: TI-Suporte (S-1-5-21-445892-...)
        └── Grupo Aninhado: Operações-Infra
             └── Origem da Regra: Herdada de D:\Departamentos
                  └── Permissão Concedida: MODIFICAR (FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE)
  ```
- **Parecer Técnico em Linguagem Natural:** Gera um texto explicativo claro, pronto para ser copiado e anexado a laudos periciais de conformidade.

---

## 4. Scanner de Riscos & Matriz de Conformidade

O Scanner de Riscos varre o sistema de arquivos aplicando heurísticas de segurança alinhadas aos padrões **CIS Microsoft Windows Benchmarks** e **NIST SP 800-53**:

| Código de Risco | Severidade | Descrição do Risco | Vetor de Ataque / Conformidade |
| :--- | :--- | :--- | :--- |
| **WORLD_WRITABLE** | **Crítico** | Pastas do sistema ou dados com permissão de escrita para `Everyone` ou `Users`. | Permite a qualquer usuário local criar ou substituir arquivos. |
| **BINARY_PLANTING** | **Crítico** | Executáveis (`.exe`), bibliotecas (`.dll`) ou scripts (`.bat`, `.ps1`) em pastas graváveis por usuários comuns. | **DLL Hijacking** e substituição de binários para elevação de privilégios. |
| **EXCESSIVE_CONTROL** | **Alto** | Contas não administrativas com direitos `WRITE_DAC` ou `WRITE_OWNER`. | Permite a um usuário comum assumir posse ou alterar a ACL para conceder a si mesmo Controle Total. |
| **ORPHANED_ACCOUNT** | **Médio** | SIDs não resolvidos (contas deletadas do Active Directory ou SAM local). | Falha de higiene de diretório e risco de colisão de SIDs em florestas legadas. |
| **BROKEN_INHERITANCE** | **Médio** | Pastas com flag `SE_DACL_PROTECTED` que bloqueiam a governança da pasta raiz. | Criação de "bolsões" clandestinos fora das políticas corporativas. |
| **SENSITIVE_EXPOSURE** | **Alto** | Pastas com palavras-chave sensíveis (`RH`, `Financeiro`, `Backup`, `Senhas`) abertas a grupos amplos. | Violação da LGPD / GDPR e do Princípio do Menor Privilégio. |

---

## 5. Auditoria de Serviços Windows & Compartilhamentos SMB

### 5.1. Auditoria de Serviços do Windows (SCM)
Clique em **Auditar Serviços do Windows** para enumerar todos os serviços registrados no Service Control Manager:
- **Unquoted Service Paths:** Localiza serviços com espaços no caminho do executável sem aspas (ex: `C:\Program Files\App Folder\service.exe`). O Windows tenta executar `C:\Program.exe` antes do executável real, permitindo elevação direta para `NT AUTHORITY\SYSTEM`.
- **Binários de Serviço Graváveis:** Identifica serviços cujos arquivos executáveis podem ser modificados por contas sem privilégios de administrador.

### 5.2. Auditoria de Compartilhamentos de Rede SMB
Clique em **Auditar Compartilhamentos de Rede (SMB)** para inspecionar os compartilhamentos do servidor:
- **Permissões do Compartilhamento (Share ACLs):** Identifica compartilhamentos com `Everyone` com Controle Total.
- **Matriz Cruzada SMB x NTFS:** Revela discrepâncias onde o compartilhamento de rede é configurado como somente leitura, mas a pasta NTFS local subjacente permite escrita irrestrita, expondo o servidor a ataques locais.

---

## 6. Snapshots Históricos & Comparação Temporal Diff (SQLite)

O ACL Auditor possui uma base de dados local autônoma em **SQLite3** armazenada em `%LOCALAPPDATA%/ACLAuditor/ACLAuditor/acl_auditor_snapshots.db`:

1. **Captura de Snapshot:**
   - Clique em **Salvar Snapshot** (`Ctrl+S`).
   - A engine armazena o mapa completo de arquivos, DACLs, SIDs e proprietários, calculando um hash criptográfico **SHA-256** do estado de segurança para garantia de custódia pericial.
2. **Comparação Temporal (Diff):**
   - Na aba *Comparação Temporal*, selecione dois snapshots (ex: *Baseline Inicial* vs *Auditoria Atual*).
   - Clique em **Comparar Snapshots (Calcular Diff)**.
   - O sistema classifica todas as alterações:
     - `Novo Acesso Concedido`: Contas adicionadas à DACL.
     - `Acesso Revogado`: Permissões removidas.
     - `Elevação de Privilégio`: Contas comuns que receberam direitos de gravação ou controle total.
     - `Alteração de Proprietário`: Troca de conta dona do objeto.
     - `Bloqueio de Herança`: Alterações na flag de herança.

---

## 7. Modo Consultoria & Geração de Relatórios Executivos

O ACL Auditor foi planejado para entrega formal de resultados a clientes corporativos e diretorias:

1. Acesse o menu **Arquivo → Exportar Relatório Executivo...** (`Ctrl+E`).
2. **Formatos de Saída:**
   - **HTML Interativo:** Relatório autossuficiente com gráficos de severidade, tabelas dinâmicas com busca e visualização da árvore de riscos.
   - **PDF Executivo:** Documento diagramado em formato A4 para impressão e assinatura formal, contendo sumário executivo, matriz de riscos e recomendações.
   - **CSV Tabular:** Arquivo codificado em UTF-8 com BOM para importação direta no Excel, Power BI ou bancos analíticos.
   - **JSON Estruturado:** Exportação completa de todos os nós para integração em esteiras DevSecOps e SIEMs.
3. **Personalização Institucional:**
   - Informe o Nome da Consultoria, Nome do Cliente Auditado, Nome do Auditor Responsável e anexe o arquivo de logotipo da consultoria para estampar o cabeçalho.

---

## 8. Remediação Segura, Scripts Automatizados e Garantia de Rollback

O ACL Auditor respeita estritamente o princípio de **Não-Destrutividade**:
- Nenhuma permissão é alterada em tempo real pela aplicação;
- O botão **Gerar Plano de Mitigação** produz scripts completos e auditáveis;
- **Script de Backup Prévio Obrigatório:** Utiliza o comando `icacls <caminho> /save <arquivo_backup> /t /c` antes de qualquer alteração;
- **Script de Correção:** Executa os ajustes recomendados em PowerShell ou `icacls`;
- **Script de Rollback (Restauração Imediata):** Utiliza `icacls <caminho_pai> /restore <arquivo_backup>` para reverter imediatamente todas as permissões ao estado exato anterior caso ocorra qualquer instabilidade.

---

## 9. Integração com o Windows Explorer (Menu de Contexto)

Para agilizar o fluxo de trabalho de administradores de sistemas:
- Acesse **Ferramentas → Integrar ao Windows Explorer**;
- O registro é efetuado na chave do usuário atual (`HKCU\Software\Classes\Directory\shell\AclAuditor`), **não exigindo direitos de administrador**;
- No Windows Explorer, basta clicar com o botão direito sobre qualquer pasta e selecionar **Auditar Permissões com ACL Auditor**.

---

## 10. Linha de Comando (CLI), Atalhos & FAQ

### 10.1. Linha de Comando (CLI)

O ACL Auditor disponibiliza duas formas complementares de execução via linha de comando: o **utilitário standalone de terminal (`acl-auditor-cli.exe`)** para automação sem interface gráfica e os **argumentos diretos do executável gráfico (`ACLAuditor.exe`)**.

#### 10.1.1. Utilitário Standalone de Console (`acl-auditor-cli.exe`)
Ideal para servidores sem interface gráfica (*Windows Server Core*), contêineres, esteiras de CI/CD (GitHub Actions), scripts PowerShell e integração com plataformas SIEM:

```cmd
acl-auditor-cli.exe <comando> [opções]
```

> **Dica Windows Terminal / MSIX:** Em pacotes distribuídos com `AppExecutionAlias` (Microsoft Store), o utilitário pode ser executado diretamente de qualquer pasta digitando apenas `acl-auditor-cli`.

**Comandos Suportados:**
- `scan <caminho>`: Executa a auditoria recursiva de permissões NTFS, detectando vulnerabilidades e calculando o escore de risco (0 a 100).
- `access <caminho> --user <usuario>`: Calcula e explica o acesso efetivo real de uma conta ou grupo sobre o arquivo ou diretório.
- `services`: Audita todos os serviços do Windows no Service Control Manager (SCM), identificando caminhos sem aspas (*Unquoted Service Paths*) e binários graváveis.
- `shares`: Inspeciona compartilhamentos de rede SMB locais e permissões atribuídas a grupos amplos (*Everyone* / *Todos*).
- `--version, -v`: Exibe a versão oficial compilada.
- `--help, -h`: Exibe o guia rápido de sintaxe e opções.

**Opções Globais:**
- `--format <text|json>`: Define o formato de saída (padrão: `text`). O formato `json` emite dados estruturados ideais para automação e pipes.
- `--depth <n>`: Define a profundidade máxima na árvore de diretórios (padrão: `2`).
- `--risks-only`: Filtra a saída para reportar exclusivamente vulnerabilidades e comandos de remediação.
- `--no-banner`: Suprime o banner inicial no modo texto para geração de logs limpos.

**Exemplos Práticos:**
```powershell
# Varredura completa em formato JSON para ingestão automatizada
acl-auditor-cli scan "C:\inetpub\wwwroot" --format json

# Verificação de acesso efetivo de um grupo ou usuário específico
acl-auditor-cli access "D:\Financeiro\Folha.xlsx" --user "Everyone"

# Auditoria de serviços reportando apenas itens vulneráveis
acl-auditor-cli services --risks-only

# Varredura profunda com profundidade 4 e exibição apenas de vulnerabilidades
acl-auditor-cli scan "D:\Corporativo" --depth 4 --risks-only
```

**Códigos de Retorno (Exit Codes):**
- `0`: Execução concluída com sucesso e sem riscos críticos identificados.
- `1`: Vulnerabilidades críticas detectadas ou pontuação de risco elevada (escore geral >= 70).
- `3`: Parâmetros ausentes ou sintaxe de comando inválida.

#### 10.1.2. Executável Gráfico com Argumentos (`ACLAuditor.exe`)
```cmd
ACLAuditor.exe [opções] [caminho]

Parâmetros:
  caminho                     Abre a interface gráfica já iniciando a auditoria no caminho informado.
  --scan <diretorio>          Inicia a auditoria direta no diretório especificado.
  --snapshot                  Captura e salva um snapshot do diretório informado no SQLite local.
  --report <formato> <saida>  Exporta relatório executivo (formatos: html, pdf, csv, json).
  --help                      Exibe a sintaxe da linha de comando.
  --version                   Exibe a versão oficial instalada.
```

### 10.2. Atalhos de Teclado
| Atalho | Ação |
| :--- | :--- |
| `F5` | Escanear / Atualizar permissões do diretório atual |
| `Ctrl+O` | Abrir diálogo de seleção de diretório |
| `Ctrl+E` | Exportar Relatório Executivo |
| `Ctrl+S` | Salvar Snapshot de Conformidade |
| `Ctrl+F` | Focar na caixa de pesquisa rápida |
| `F1` | Abrir este Manual de Instruções |
| `Ctrl+Q` | Encerrar a aplicação |

### 10.3. Resolução de Dúvidas Frequentes (FAQ)
- **Por que algumas pastas do sistema retornam "Acesso Negado"?**  
  Diretórios protegidos do sistema (como `C:\System Volume Information`) requerem a execução da aplicação com elevação de administrador para habilitar o privilégio `SeSecurityPrivilege`.
- **A aplicação envia dados para a nuvem?**  
  Não. O ACL Auditor é 100% autônomo, offline e não possui código de telemetria ou envio de dados.

---

# PART 2: OPERATIONS MANUAL & COMPLIANCE GUIDE (ENGLISH - US)

## 1. Overview & NTFS Security Architecture

**ACL Auditor** is an enterprise-grade visual analyzer and compliance engine for Windows NTFS file systems.

Engineered in native **C++20** utilizing low-level Windows Security APIs (`Advapi32.dll`, `Authz.dll`, `Netapi32.dll`, `BCrypt.dll`) with a **Qt 6.11.1** desktop interface, the tool operates in a **100% autonomous, local, and read-only mode**.

### 1.1. Security Descriptor Architecture
Every securable object in Windows NT is governed by a **Security Descriptor (SD)** composed of:
- **Owner SID:** Security Identifier representing the owning user or group. The owner retains the inherent right to modify permissions (`WRITE_DAC`), even if explicit DACL rules deny access.
- **DACL (Discretionary Access Control List):** An ordered list of Access Control Entries (ACEs) that explicitly grant (`ALLOW`) or reject (`DENY`) specific rights to SIDs.
- **SACL (System Access Control List):** Directs the kernel's audit generation for security event logs.
- **Control Flags:**
  - `SE_DACL_PRESENT`: Indicates an active discretionary access control list.
  - `SE_DACL_PROTECTED`: Indicates that inheritance propagation from the parent container has been explicitly blocked.

### 1.2. Canonical ACE Precedence Algorithm
Windows NT enforces canonical ACE ordering as follows:
1. **Explicit DENY ACEs:** Evaluated first with absolute veto authority.
2. **Explicit ALLOW ACEs.**
3. **Inherited DENY ACEs:** Evaluated from closest parent container up to drive root.
4. **Inherited ALLOW ACEs.**
5. **Implicit Deny:** If no ACE grants the requested privilege, access is summarily denied.

---

## 2. NTFS Permissions Explorer & Virtual Tree

The Permissions Explorer serves as the primary visual workspace for inspecting storage volumes, directory hierarchies, and files:

1. **Target Directory Selection:**
   - Enter any absolute folder path (e.g., `C:\inetpub`, `D:\CorporateData`) or click **Browse...** (`Ctrl+O`).
   - Press **Enter** or click **Scan Directory** (`F5`).
2. **Virtualized Hierarchical Tree:**
   - The tree model employs virtualized on-demand loading, maintaining responsive rendering across massive directory hierarchies.
   - Each entry displays:
     - **Object Name:** File or directory name with type indicator icon.
     - **Owner:** Account resolved from local SAM or Active Directory.
     - **Inheritance Status:** Indicates whether rights are inherited or blocked (`SE_DACL_PROTECTED`).
     - **Calculated Risk Rating:** Dynamic heuristic rating (`Compliant`, `Low`, `Medium`, `High`, `Critical`).
3. **Detailed ACE Inspection Table:**
   - Selecting any node reveals all active ACE rules in the right panel:
     - **Type:** `Allow` (Green) or `Deny` (Red).
     - **Identity / SID:** Friendly name and raw string SID.
     - **Rights:** Full Control (`GENERIC_ALL`), Modify, Read & Execute, Write, or Special Permissions.
     - **Inheritance Source:** Exact parent directory where the permission originated (retrieved via `GetInheritanceSourceW`).
     - **Hex Mask:** Low-level Win32 32-bit mask (e.g., `0x001F01FF`).

---

## 3. Effective Access & Visual Explainer ("Why Does It Have Access?")

The cornerstone capability of ACL Auditor is answering with mathematical certainty:

> **“Why does this user have permission to read, modify, or execute this file?”**

### 3.1. Visual Explainer Workflow
1. Select any target file or folder in the tree and click **Why Does It Have Access?**.
2. In the Effective Access tab, pick an identity from the account selector or type any username, local group, or SID.
3. Click **Explain Effective Access Chain**.
4. The calculation engine performs:
   - Complete resolution of primary, local, domain, and universal group memberships;
   - Inclusion of Windows well-known SIDs: `Everyone` (S-1-1-0), `Authenticated Users` (S-1-5-11), and `BUILTIN\Users` (S-1-5-32-545);
   - Token simulation against the Security Reference Monitor (SRM) evaluation rules.

### 3.2. Verdict Badges & Causal Hierarchy
- **Verdict Badges:** Real-time status for the 5 fundamental access rights:
  - `READ` | `WRITE` | `EXECUTE` | `DELETE` | `CHANGE PERMISSIONS`
- **Causal Lineage Tree:**
  ```text
  Analyzed Identity: john.doe (S-1-5-21-100234-...)
   └── Group Membership: IT-Support (S-1-5-21-445892-...)
        └── Nested Group: Infra-Operations
             └── Rule Inheritance: Inherited from D:\Departments
                  └── Granted Permission: MODIFY (FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE)
  ```
- **Natural Language Technical Report:** Executive technical verdict ready for compliance audit records.

---

## 4. Risk & Compliance Scanner

The Risk Scanner audits the directory hierarchy against **CIS Microsoft Windows Benchmarks** and **NIST SP 800-53**:

| Risk Code | Severity | Description | Threat Vector / Compliance |
| :--- | :--- | :--- | :--- |
| **WORLD_WRITABLE** | **Critical** | System or corporate folders granting write access to `Everyone` or `Users`. | Allows any local user to place malicious files or achieve persistence. |
| **BINARY_PLANTING** | **Critical** | Executables (`.exe`), DLLs (`.dll`), or scripts (`.bat`, `.ps1`) in writable paths. | **DLL Hijacking** and binary replacement for local privilege escalation. |
| **EXCESSIVE_CONTROL** | **High** | Standard accounts granted `WRITE_DAC` or `WRITE_OWNER` rights. | Allows a non-admin account to seize ownership or grant itself Full Control. |
| **ORPHANED_ACCOUNT** | **Medium** | Unresolved SIDs originating from deleted domain or local user accounts. | Directory hygiene violation and potential SID reuse exposure in legacy forests. |
| **BROKEN_INHERITANCE** | **Medium** | Folders with `SE_DACL_PROTECTED` flag blocking governance rules. | Creates unmanaged permission silos outside enterprise oversight. |
| **SENSITIVE_EXPOSURE** | **High** | Directories with sensitive naming patterns (`HR`, `Finance`, `Backup`, `Passwords`) accessible by broad groups. | Regulatory violation of GDPR / LGPD and Principle of Least Privilege. |

---

## 5. Windows Services & SMB Network Shares Audit

### 5.1. Windows Services Audit (SCM)
Click **Audit Windows Services** to inspect services registered in the Service Control Manager:
- **Unquoted Service Paths:** Flags service binary paths containing spaces and missing enclosing quotation marks. Windows attempts running `C:\Program.exe` prior to the legitimate service executable, granting immediate escalation to `NT AUTHORITY\SYSTEM`.
- **Writable Service Binaries:** Identifies service executables writable by standard user accounts.

### 5.2. SMB Network Shares Audit
Click **Audit Network Shares (SMB)** to evaluate all SMB shares:
- **Share Level ACLs:** Identifies shares granting Full Control to `Everyone`.
- **SMB vs NTFS Discrepancy Matrix:** Detects mismatches where a network share is configured as read-only, but the underlying NTFS folder is world-writable, exposing the server to local compromise.

---

## 6. Historical Snapshots & Temporal Diff Engine (SQLite)

ACL Auditor includes an embedded local **SQLite3** database stored at `%LOCALAPPDATA%/ACLAuditor/ACLAuditor/acl_auditor_snapshots.db`:

1. **Snapshot Capture:**
   - Click **Save Snapshot** (`Ctrl+S`).
   - The engine records the complete tree state, DACLs, SIDs, and owners, computing a cryptographic **SHA-256** hash for forensic custody.
2. **Temporal Diff:**
   - Under the *Temporal Comparison* tab, select a Baseline Snapshot and a Target Snapshot.
   - Click **Compare Snapshots (Calculate Diff)**.
   - The engine categorizes all changes:
     - `New Access Granted`: Newly added accounts.
     - `Access Revoked`: Removed permissions or accounts.
     - `Privilege Escalation`: Standard accounts upgraded to write or full control.
     - `Owner Change`: Alterations in object ownership.
     - `Inheritance Toggle`: Blocking or unblocking of inheritance propagation.

---

## 7. Consulting Mode & Executive Multi-Format Reporting

Designed for professional services delivery and corporate audit reporting:

1. Navigate to **File → Export Executive Report...** (`Ctrl+E`).
2. **Output Formats:**
   - **Interactive HTML:** Responsive web dashboard with interactive charts, filterable risk tables, and full tree visual maps.
   - **Executive PDF:** Formatted A4 document with cover page, executive summary, risk tables, and formal sign-off sections.
   - **Tabular CSV:** UTF-8 BOM encoded tabular dataset for Excel, Power BI, or SIEM platforms.
   - **Structured JSON:** Machine-readable payload for automated DevSecOps pipelines.
3. **Corporate Branding:**
   - Customize Consultancy Name, Client Name, Lead Auditor Name, and attach your company PNG/JPEG logo for cover page rendering.

---

## 8. Safe Remediation, Automated Scripts & Rollback Guarantee

ACL Auditor strictly enforces the **Non-Destructive Principle**:
- The software **never modifies file permissions directly or automatically**;
- Clicking **Generate Mitigation Plan** produces transparent, auditable scripts;
- **Mandatory Pre-Backup:** Executes `icacls <path> /save <backup_file> /t /c` before applying changes;
- **Fix Script:** Applies recommended hardening via PowerShell or `icacls`;
- **Instant Rollback Script:** Executes `icacls <parent_path> /restore <backup_file>` to immediately revert permissions in case of application incompatibilities.

---

## 9. Windows Explorer Shell Integration

To optimize administrator daily workflow:
- Select **Tools → Integrate with Windows Explorer**;
- Writes shell registry keys to Current User (`HKCU\Software\Classes\Directory\shell\AclAuditor`) — **no administrator privileges required**;
- Right-click any directory in Windows Explorer and choose **Audit Permissions with ACL Auditor** to begin instant analysis.

---

## 10. Command Line Interface (CLI), Shortcuts & FAQ

### 10.1. Command Line Interface (CLI)

ACL Auditor delivers two complementary command-line operational modes: the **headless standalone console utility (`acl-auditor-cli.exe`)** for unattended automation and the **direct arguments of the graphical executable (`ACLAuditor.exe`)**.

#### 10.1.1. Standalone Headless Console Utility (`acl-auditor-cli.exe`)
Engineered for headless servers (*Windows Server Core*), Docker Windows containers, CI/CD pipelines (GitHub Actions), PowerShell automation, and SIEM data collection:

```cmd
acl-auditor-cli.exe <command> [options]
```

> **Windows Terminal / MSIX Tip:** When installed via MSIX packages supporting `AppExecutionAlias` (Microsoft Store), the utility is immediately globally accessible by running `acl-auditor-cli` from any terminal.

**Supported Commands:**
- `scan <path>`: Recursively audits NTFS permissions, discovers security vulnerabilities, and evaluates risk rating (0 to 100).
- `access <path> --user <trustee>`: Calculates and explains true effective access rights for any local or domain account/group.
- `services`: Audits the Windows Service Control Manager (SCM) for Unquoted Service Paths and user-writable service executables.
- `shares`: Inspects local SMB network shares and flags excessive permissions granted to broad groups (*Everyone*).
- `--version, -v`: Prints the official release version.
- `--help, -h`: Displays command-line syntax and option reference.

**Global Options:**
- `--format <text|json>`: Specifies output serialization format (default: `text`). The `json` format outputs structured schemas for pipeline integration.
- `--depth <n>`: Configures directory recursion depth limit (default: `2`).
- `--risks-only`: Restricts reporting strictly to identified security vulnerabilities and remediation scripts.
- `--no-banner`: Suppresses the ASCII banner in text mode for clean logging.

**Practical Examples:**
```powershell
# Comprehensive directory scan exported as formatted JSON
acl-auditor-cli scan "C:\inetpub\wwwroot" --format json

# Calculate effective permissions for a specific security principal
acl-auditor-cli access "D:\Finance\Payroll.xlsx" --user "Everyone"

# Audit Windows services reporting only high/critical vulnerabilities
acl-auditor-cli services --risks-only

# Deep scan with depth 4 focusing strictly on risk findings
acl-auditor-cli scan "D:\CorporateData" --depth 4 --risks-only
```

**Process Exit Codes:**
- `0`: Execution completed successfully with no critical security findings.
- `1`: High/critical security vulnerabilities detected or risk score elevated (overall score >= 70).
- `3`: Missing mandatory parameters or invalid syntax.

#### 10.1.2. Graphical Executable CLI Arguments (`ACLAuditor.exe`)
```cmd
ACLAuditor.exe [options] [path]

Arguments:
  path                        Launches GUI and begins scan on specified path.
  --scan <directory>          Initiates direct audit on target directory.
  --snapshot                  Captures and saves a baseline snapshot to local SQLite.
  --report <format> <output>  Generates executive report (formats: html, pdf, csv, json).
  --help                      Displays CLI usage information.
  --version                   Prints official installed version.
```

### 10.2. Keyboard Shortcuts
| Shortcut | Action |
| :--- | :--- |
| `F5` | Scan / Refresh current directory permissions |
| `Ctrl+O` | Open directory selection dialog |
| `Ctrl+E` | Export Executive Report |
| `Ctrl+S` | Save Compliance Snapshot |
| `Ctrl+F` | Focus quick search bar |
| `F1` | Open this Operations Manual |
| `Ctrl+Q` | Exit application |

### 10.3. Frequently Asked Questions (FAQ)
- **Why do certain system directories report "Access Denied"?**  
  Restricted system folders (e.g., `C:\System Volume Information`) require launching ACL Auditor as Administrator to enable the `SeSecurityPrivilege` token.
- **Does the application connect to external network servers?**  
  No. ACL Auditor operates 100% locally and autonomously. It performs zero telemetry and sends no data externally.

---
*ACL Auditor • Operations Manual & NTFS Compliance Guide • © 2026 Fernando Nillsson Cidade*