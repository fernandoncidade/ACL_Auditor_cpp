# Guia do Usuário / Operations User Guide — ACL Auditor

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o guia do usuário em português</summary><br>

# ACL Auditor — Manual de Operação e Guia do Usuário

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Plataforma:** Windows 10 / Windows 11 / Windows Server 2022+ (x64)  
**Autor:** Fernando Nillsson Cidade  

---

## 1. Primeiros Passos

### 1.1. Modos de Execução
- **Modo Padrão:** Execute `ACLAuditor.exe`. Permite auditar arquivos pessoais, pastas compartilhadas, documentos de trabalho e programas comuns.
- **Modo Administrador Elevado:** Para inspecionar áreas restritas do sistema operacional (como `C:\System Volume Information` ou partes críticas de `C:\Windows\System32`), inicie a aplicação com o botão direito -> *Executar como Administrador* para habilitar o privilégio `SeSecurityPrivilege`.

---

## 2. Fluxos Principais de Trabalho

### 2.1. Navegador de Permissões NTFS
1. Insira o caminho desejado na barra superior (ex: `D:\Corporativo`) ou clique em **Procurar...** (`Ctrl+O`).
2. Pressione `Enter` ou clique em **Escanear Permissões** (`F5`).
3. A **Árvore Virtualizada** listará instantaneamente pastas e arquivos com:
   - **Objeto:** Nome do arquivo ou pasta com ícone do tipo.
   - **Proprietário:** Conta proprietária resolvida (ex: `CORP\domain.admins` ou `NT AUTHORITY\SYSTEM`).
   - **Status de Herança:** Indicador visual se a herança da pasta pai está ativa ou bloqueada (`SE_DACL_PROTECTED`).
   - **Nível de Risco:** Classificação calculada pela engine heurística (`Conforme`, `Baixo`, `Médio`, `Alto`, `Crítico`).
4. Ao clicar em qualquer item, a tabela à direita exibe todas as entradas de controle de acesso (ACEs), permissões concedidas e origem da herança.

### 2.2. Acesso Efetivo Visual ("Por Que Tem Acesso?")
1. Selecione o arquivo ou pasta na árvore.
2. Clique no botão azul **Por Que Tem Acesso?**.
3. Escolha uma conta na lista de identidades ou digite manualmente o nome da conta ou SID.
4. A engine processa a hierarquia completa de tokens e exibe:
   - **Badges de Veredito:** Status claro de Leitura, Gravação e Execução.
   - **Árvore Causal Explicativa:** Demonstra como os grupos diretos, aninhados e regras herdadas geraram a permissão final.
   - **Parecer Técnico:** Texto em linguagem natural pronto para inclusão em laudos periciais.

### 2.3. Scanner Heurístico de Riscos & Conformidade
1. Acesse a aba **Scanner de Riscos**.
2. Analise as vulnerabilidades encontradas:
   - **Pastas World-Writable:** Diretórios onde usuários comuns possuem permissão de gravação.
   - **Binary Planting / DLL Hijacking:** Arquivos executáveis em diretórios com escrita para usuários comuns.
   - **Privilégios Excessivos:** Contas não-administrativas com `WRITE_DAC` ou `WRITE_OWNER`.
   - **SIDs Órfãos:** Regras órfãs pertencentes a contas ou domínios excluídos.
3. Consulte o **Score de Risco (0 a 100)** calculado para o diretório.

### 2.4. Auditoria de Serviços Windows e Compartilhamentos SMB
1. Acesse a aba **Serviços & Compartilhamentos**.
2. Clique em **Auditar Serviços Windows**:
   - Detecta caminhos de serviço sem aspas com espaços (*Unquoted Service Paths*).
   - Identifica se o executável do serviço pode ser substituído por contas padrão.
3. Clique em **Auditar Compartilhamentos SMB**:
   - Enumera os compartilhamentos de rede ativos no host via `NetShareEnum`.
   - Cruza as permissões do compartilhamento com as DACLs locais da pasta física.

### 2.5. Snapshots Históricos & Comparação Temporal Diff
1. Para criar uma linha de base, clique em **Salvar Snapshot** (`Ctrl+S`).
2. O snapshot é gravado no banco SQLite local com assinatura de integridade SHA-256.
3. Para comparar modificações ao longo do tempo:
   - Abra a aba **Snapshots & Diff**.
   - Selecione o Snapshot A (Baseline) e o Snapshot B (Atual).
   - Clique em **Comparar Snapshots**.
   - Analise novos arquivos criados, acessos revogados e elevações de privilégio.

### 2.6. Geração de Relatórios Executivos
1. Clique em **Exportar Relatório** (`Ctrl+E`).
2. Escolha o formato desejado:
   - **HTML Interativo:** Dashboard web responsivo com filtros dinâmicos.
   - **PDF Executivo:** Documento formatado em A4 com sumário executivo e espaço para assinatura.
   - **CSV Estruturado:** Arquivo tabular com UTF-8 BOM para abertura no Microsoft Excel.
   - **JSON Completo:** Formato estruturado para integração com SIEMs e automações.
3. Personalize com o nome da consultoria, cliente, auditor e logotipo.

### 2.7. Remediação Segura e Rollback Imediato
1. Clique em **Plano de Remediação** sobre qualquer risco encontrado.
2. Visualize o script gerado em PowerShell (`.ps1`) ou Batch (`icacls`).
3. O plano segue a filosofia não-destrutiva de três etapas:
   - **Passo 1 (Backup):** `icacls "Caminho" /save "Backup.acl" /t /c`
   - **Passo 2 (Correção):** Aplica a regra de menor privilégio recomendada.
   - **Passo 3 (Script de Rollback):** `icacls "Caminho" /restore "Backup.acl"` gerado automaticamente.

### 2.8. Integração com o Windows Explorer
1. Nas opções da aplicação, marque a opção de integração com o Explorer.
2. No Windows Explorer, basta clicar com o botão direito sobre qualquer pasta e selecionar **Auditar Permissões com ACL Auditor**.

---

## 3. Atalhos de Teclado

| Atalho | Ação |
|:---:|---|
| `F5` | Atualizar / Escanear diretório atual |
| `Ctrl+O` | Abrir diálogo de seleção de pasta |
| `Ctrl+E` | Exportar Relatório Executivo |
| `Ctrl+S` | Salvar Snapshot de Segurança |
| `Ctrl+F` | Focar no campo de busca rápida |
| `F1` | Abrir Manual de Instruções |
| `Ctrl+Q` | Encerrar a aplicação |

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the user guide in English</summary><br>

# ACL Auditor — End-User Operations Guide

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Platform:** Windows 10 / Windows 11 / Windows Server 2022+ (x64)  
**Author:** Fernando Nillsson Cidade  

---

## 1. Getting Started

### 1.1. Launching ACL Auditor
- **Standard Mode:** Run `ACLAuditor.exe`. This allows complete auditing of user files, network shares, personal directories, and standard program files.
- **Elevated Administrator Mode:** To inspect protected system areas (such as `C:\System Volume Information` or restricted parts of `C:\Windows\System32`), launch ACL Auditor as Administrator. This enables the `SeSecurityPrivilege` and `SeBackupPrivilege` user rights.

---

## 2. Core Workflows

### 2.1. Auditing NTFS Permissions
1. Enter the target folder path in the address bar (e.g., `D:\CorporateData`) or click **Browse...** (`Ctrl+O`).
2. Press `Enter` or click **Scan Permissions** (`F5`).
3. The **Virtualized Tree** immediately lists folders and files:
   - **Object:** Displays file/folder name with associated system icon.
   - **Owner:** Resolved account name (e.g., `CORP\domain.admins` or `NT AUTHORITY\SYSTEM`).
   - **Inheritance:** Indicates whether DACL inheritance is active or blocked (`SE_DACL_PROTECTED`).
   - **Risk Level:** Instant heuristic severity (`Compliant`, `Low`, `Medium`, `High`, `Critical`).
4. Select any item to view all individual Access Control Entries (ACEs) in the right-hand panel, including Trustee, Access Type (`Allow` / `Deny`), granted permissions mask, and inheritance source.

### 2.2. Visual Effective Access ("Why Does It Have Access?")
1. Select any target file or folder in the tree.
2. Click the blue button: **Why Does It Have Access?**.
3. Select an account from the local/domain user list, or type a username / SID manually.
4. The engine analyzes the full token hierarchy and renders:
   - **Verdict Badges:** Clear Read / Write / Execute visual statuses.
   - **Causal Explanation Tree:** Traces how group membership and folder inheritance combined to produce the final access rights.
   - **Technical Opinion:** Formal narrative summary suitable for audit and forensic documentation.

### 2.3. Heuristic Risk & Compliance Scanner
1. Switch to the **Risk Scanner** tab.
2. Review detected vulnerabilities categorized by severity:
   - **World-Writable Folders:** Directories where unprivileged users can create files.
   - **Binary Planting / DLL Hijacking:** Executable extensions located in user-writable paths.
   - **Excessive Rights:** Non-admin accounts possessing `WRITE_DAC` or `WRITE_OWNER`.
   - **Orphaned SIDs:** Leftover access rules from deleted accounts.
3. View the calculated composite **Risk Score (0–100)**.

### 2.4. Windows Services & SMB Shares Audit
1. Navigate to the **Services & Shares** tab.
2. Click **Scan Windows Services**:
   - Detects unquoted service executable paths with spaces.
   - Verifies whether service binaries can be replaced by non-admin users.
3. Click **Scan SMB Shares**:
   - Enumerates local network shares via `NetShareEnum`.
   - Cross-references share-level permissions against local NTFS DACLs to highlight discrepancies.

### 2.5. Historical Snapshots & Temporal Diff
1. To capture a point-in-time baseline, click **Save Snapshot** (`Ctrl+S`).
2. The snapshot is saved to local SQLite with an integrity SHA-256 fingerprint.
3. To compare changes over time:
   - Go to the **Snapshots & Diff** tab.
   - Select Baseline Snapshot (Snapshot A) and Target Snapshot (Snapshot B).
   - Click **Compare Snapshots**.
   - Review added files, revoked permissions, and privilege escalation events.

### 2.6. Generating Executive Reports
1. Click **Export Report** (`Ctrl+E`).
2. Select your desired output format:
   - **Interactive HTML:** Responsive web dashboard with live search and severity filtering.
   - **Executive PDF:** Formatted A4 document with executive summary, charts, and signature lines.
   - **Structured CSV:** Comma-separated data with UTF-8 BOM for Microsoft Excel.
   - **Full JSON:** Raw machine-readable data for SIEM and automation pipelines.
3. Add custom branding: Consultancy Name, Client Name, Lead Auditor, and Logo.

### 2.7. Non-Destructive Remediation & Safe Rollback
1. Click **Remediation Plan** on any finding or audited folder.
2. Review the generated PowerShell (`.ps1`) or Batch (`icacls`) script.
3. Notice the mandatory two-phase safety mechanism:
   - **Step 1 (Backup):** `icacls "Path" /save "Backup.acl" /t /c`
   - **Step 2 (Remediation):** Applies the least-privilege correction.
   - **Step 3 (Rollback Script):** `icacls "Path" /restore "Backup.acl"` generated automatically.

### 2.8. Windows Explorer Integration
1. In the application settings/tools, enable **Windows Explorer Integration**.
2. Right-click any folder in Windows Explorer and click **Audit Permissions with ACL Auditor**.
3. Registered in `HKCU`, requiring no administrator elevation to install.

---

## 3. Keyboard Shortcuts

| Shortcut | Description |
|:---:|---|
| `F5` | Refresh / Scan current directory |
| `Ctrl+O` | Open folder selection dialog |
| `Ctrl+E` | Export Executive Report |
| `Ctrl+S` | Save Security Snapshot |
| `Ctrl+F` | Focus quick search filter |
| `F1` | Open User Manual |
| `Ctrl+Q` | Quit application |

</details>
