# Referência da Linha de Comando / CLI Reference — ACL Auditor

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher a referência do CLI em português</summary><br>

# ACL Auditor — Referência da Linha de Comando (CLI)

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Executáveis:** `ACLAuditor.exe` (Interface gráfica com suporte a parâmetros) | `acl-auditor-cli.exe` (Utilitário de console headless)  

---

## 1. Visão Geral

O ACL Auditor fornece tanto uma aplicação gráfica para desktop quanto um utilitário de console leve (`acl-auditor-cli.exe`), projetado especificamente para scripts PowerShell/Bash, pipelines de CI/CD, DevSecOps e administração remota de servidores Windows.

---

## 2. Aplicação Gráfica Desktop (`ACLAuditor.exe`)

Permite abrir a interface apontando diretamente para um diretório ou executar rotinas automatizadas no terminal:

```cmd
ACLAuditor.exe [opções] [caminho]
```

### Parâmetros Suportados:
| Parâmetro | Descrição | Exemplo |
|---|---|---|
| `[caminho]` | Abre e inicia a auditoria imediatamente no diretório informado. | `ACLAuditor.exe "D:\Departamentos"` |
| `--scan <diretorio>` | Flag explícita para varredura do diretório. | `ACLAuditor.exe --scan "C:\inetpub"` |
| `--snapshot` | Captura e salva o snapshot diretamente no SQLite local. | `ACLAuditor.exe --scan "D:\Dados" --snapshot` |
| `--report <formato> <arquivo>` | Exporta relatório executivo (`html`, `pdf`, `csv`, `json`). | `ACLAuditor.exe --scan "D:\Dados" --report html "C:\Laudos\auditoria.html"` |
| `--help` | Exibe a sintaxe da linha de comando. | `ACLAuditor.exe --help` |
| `--version` | Exibe a versão oficial da aplicação. | `ACLAuditor.exe --version` |

---

## 3. Utilitário Standalone de Console (`acl-auditor-cli.exe`)

A ferramenta CLI opera sem interface gráfica e emite resultados em texto limpo ou em formato JSON:

```cmd
acl-auditor-cli.exe scan <caminho> [opções]
acl-auditor-cli.exe access <caminho> --user <usuario_ou_sid>
acl-auditor-cli.exe services [--risks-only]
acl-auditor-cli.exe shares
```

### Exemplos Práticos:

#### 1. Varrer Diretório em Busca de Riscos e Exportar em JSON:
```powershell
acl-auditor-cli.exe scan "C:\inetpub\wwwroot" --format json > resultado_auditoria.json
```

#### 2. Consultar Acesso Efetivo de uma Conta em um Arquivo Sensível:
```powershell
acl-auditor-cli.exe access "D:\Financeiro\Folha.xlsx" --user "CORP\jsilva"
```
Saída:
```text
Target: D:\Financeiro\Folha.xlsx
Trustee: CORP\jsilva (S-1-5-21-...)
Read: ALLOWED (via Grupo: Contabilidade)
Write: DENIED (Nenhuma regra de escrita presente)
Execute: DENIED
Inheritance Origin: D:\Financeiro
```

#### 3. Auditar Serviços Windows (Unquoted Paths e Permissões Fracas):
```powershell
acl-auditor-cli.exe services --risks-only
```

#### 4. Códigos de Saída (Exit Codes) para Pipelines:
- `0`: Varredura concluída com sucesso; nenhum risco crítico/alto detectado.
- `1`: Risco de segurança Alto ou Crítico identificado.
- `2`: Caminho não encontrado ou acesso negado pelo sistema operacional.
- `3`: Sintaxe de comando inválida.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the CLI reference in English</summary><br>

# ACL Auditor — Command-Line Interface (CLI) Reference

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Executables:** `ACLAuditor.exe` (GUI with CLI parameters) | `acl-auditor-cli.exe` (Headless Console CLI)  

---

## 1. Overview

ACL Auditor provides both a graphical desktop application capable of accepting command-line arguments and a dedicated lightweight headless console tool (`acl-auditor-cli`) designed for automated scripts, CI/CD pipelines, and remote Windows administration.

---

## 2. Desktop Application CLI (`ACLAuditor.exe`)

You can launch the GUI targeting a specific directory or invoke automated actions via the Windows terminal:

```cmd
ACLAuditor.exe [options] [path]
```

### Supported Parameters:
| Argument | Description | Example |
|---|---|---|
| `[path]` | Automatically opens and audits the target directory. | `ACLAuditor.exe "D:\Departments"` |
| `--scan <directory>` | Direct scan flag for the specified path. | `ACLAuditor.exe --scan "C:\inetpub"` |
| `--snapshot` | Saves a snapshot of the scanned folder directly to the local SQLite database. | `ACLAuditor.exe --scan "D:\Data" --snapshot` |
| `--report <format> <file>` | Exports an executive report (`html`, `pdf`, `csv`, `json`). | `ACLAuditor.exe --scan "D:\Data" --report html "C:\Reports\audit.html"` |
| `--help` | Prints syntax and parameters to standard output. | `ACLAuditor.exe --help` |
| `--version` | Displays current application version. | `ACLAuditor.exe --version` |

---

## 3. Standalone CLI Utility (`acl-auditor-cli.exe`)

The headless CLI tool operates without launching the GUI and outputs structured human-readable text or JSON:

```cmd
acl-auditor-cli.exe scan <path> [options]
acl-auditor-cli.exe access <path> --user <username_or_sid>
acl-auditor-cli.exe services
acl-auditor-cli.exe shares
```

### Examples:

#### 1. Scan a Directory for Security Risks and Output JSON:
```powershell
acl-auditor-cli.exe scan "C:\inetpub\wwwroot" --format json > audit_results.json
```

#### 2. Check Effective Access for a Specific User:
```powershell
acl-auditor-cli.exe access "D:\Finance\Ledger.xlsx" --user "CORP\jsmith"
```
Output:
```text
Target: D:\Finance\Ledger.xlsx
Trustee: CORP\jsmith (S-1-5-21-...)
Read: ALLOWED (via Group: Accounting)
Write: DENIED (No write rule present)
Execute: DENIED
Inheritance Origin: D:\Finance
```

#### 3. Audit Local Windows Services for Unquoted Paths and Weak Permissions:
```powershell
acl-auditor-cli.exe services --risks-only
```

#### 4. Automated Exit Codes for CI/CD Pipelines:
- `0`: Scan completed successfully, no critical risks detected.
- `1`: High or Critical severity security risk detected.
- `2`: Target path not found or access denied.
- `3`: Invalid command syntax.

</details>
