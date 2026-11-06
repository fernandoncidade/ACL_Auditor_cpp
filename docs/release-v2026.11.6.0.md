# Notas de Lançamento / Release Notes — v2026.11.6.0

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher as notas de versão em português</summary><br>

# ACL Auditor — Release v2026.11.6.0

**ACL Auditor v2026.11.6.0**

**Tag de Versão:** `v2026.11.6.0`  
**Data de Publicação:** 6 de novembro de 2026  
**Autor:** Fernando Nillsson Cidade  
**Download Direto:** [Baixar Última Versão no GitHub](https://github.com/fernandoncidade/ACL_Auditor_cpp/releases/latest) | [Instalar via Microsoft Store](https://apps.microsoft.com/detail/9NX5XZLL8WXW)  

---

## 1. Artefatos de Instalação e Hashes Criptográficos

Valide a integridade do pacote baixado antes da execução utilizando o cmdlet nativo do PowerShell:

```powershell
Get-FileHash -Path ".\ACLAuditor-v2026.11.6.0-x64-Setup.exe" -Algorithm SHA256
```

| Nome do Artefato | Formato | Arquitetura Alvo | Descrição |
|---|---|:---:|---|
| `ACLAuditor-v2026.11.6.0-x64-Setup.exe` | Instalador Inno Setup | x64 (MSVC / MinGW) | Instalador gráfico completo com integração ao Windows Explorer e atalhos na Área de Trabalho. |
| `ACLAuditor-v2026.11.6.0-x64-Portable.zip` | Arquivo ZIP Portátil | x64 | Versão portátil sem instalação. Descompacte e execute em qualquer máquina sem privilégios de administrador. |
| `Source code (zip)` | Arquivo ZIP | Qualquer | Código-fonte completo em C++20 e Qt 6. |
| `Source code (tar.gz)` | Arquivo Tarball | Qualquer | Código-fonte completo em C++20 e Qt 6. |

---

## 2. Destaques da Versão

- **Explicador Visual de Acesso Efetivo:** Responde formalmente *"Por que este usuário específico consegue acessar este arquivo?"*, simulando o token de segurança, grupos diretos, grupos universais (`Everyone`, `Authenticated Users`) e herança da pasta raiz.
- **Scanner Heurístico de Riscos:** Identifica diretórios World-Writable, vetores de Binary Planting / DLL Hijacking (`.exe`, `.dll`, `.sys`, `.ps1`), privilégios excessivos (`WRITE_DAC`, `WRITE_OWNER`) e SIDs órfãos.
- **Auditoria de Serviços Windows e Compartilhamentos SMB:** Inspeciona o Service Control Manager em busca de Unquoted Service Paths e binários com permissões fracas; audita permissões de rede SMB cruzando-as com as DACLs NTFS locais.
- **Snapshots Locais em SQLite e Diff Temporal:** Snapshots com assinatura SHA-256 armazenados em `%LOCALAPPDATA%/ACLAuditor`. Comparador de deltas automatizado para detectar elevações de privilégio e desvios de conformidade.
- **Relatórios Executivos Multi-Formato:** Dashboards em HTML interativo, PDFs executivos em A4 com sumário técnico e espaço para assinatura, planilhas CSV estruturadas (UTF-8 BOM) e JSON para SIEM.
- **Remediação Não-Destrutiva com Rollback Garantido:** Scripts em PowerShell e batch com backup obrigatório (`icacls /save`) e script de restauração imediata (`icacls /restore`).
- **Integração com o Windows Explorer:** Menu de contexto registrado em `HKCU`, dispensando elevação de administrador para instalação.

---

## 3. Requisitos de Sistema

- **Sistema Operacional:** Windows 11, Windows 10, Windows Server 2022 / 2019 (64-bit x64).
- **Privilégios:** Modo de usuário padrão para arquivos comuns e compartilhamentos; privilégios de Administrador (`SeSecurityPrivilege`) para diretórios protegidos do sistema (como `C:\System Volume Information`).
- **Memória:** Mínimo de 256 MB RAM (consumo típico < 85 MB).
- **Espaço em Disco:** ~55 MB após instalado.

---

## 4. Problemas Conhecidos e Soluções

- **Diretórios Protegidos do Sistema:** Acessar `C:\System Volume Information` sem privilégios de administrador retorna o erro Win32 `ERROR_ACCESS_DENIED`. Solução: Clique com o botão direito no `ACLAuditor.exe` e selecione *Executar como Administrador*.
- **Latência em Compartilhamentos de Rede WAN:** Varreduras em compartilhamentos remotos através de conexões de alta latência podem demandar mais tempo. Solução: Execute a ferramenta CLI localmente no servidor de arquivos.

---

## 5. Instruções de Desinstalação

- **Versão com Instalador:** Abra Configurações do Windows -> **Aplicativos** -> **Aplicativos Instalados**, localize **ACL Auditor** e clique em **Desinstalar**. Alternativamente, execute `unins000.exe` na pasta de instalação.
- **Versão Portátil:** Basta excluir a pasta descompactada.
- **Remoção do Menu de Contexto do Explorer:** Se ativado, abra o ACL Auditor, desmarque a opção nas configurações ou execute:
  ```powershell
  Remove-Item -Recurse -Force "HKCU:\Software\Classes\Directory\shell\AclAuditor"
  ```

---

## 6. Termos de Licenciamento e Distribuição do Binário

- **Código-Fonte Aberto (Licença GNU AGPLv3):** O código integral desta versão é livre para estudo, modificação e compilação própria sob os termos copyleft da AGPLv3.
- **Documentação em `source/assets/`:** Desenvolvida para a Microsoft Store; desenvolvedores possuem liberdade total para alterar, excluir, adicionar e adaptar para seus próprios projetos e plataformas.
- **Restrição sobre o Binário Oficial do Autor:** O binário pré-compilado e assinado oficialmente pelo autor (**Fernando Nillsson Cidade**), notadamente a edição comercial distribuída na Microsoft Store, **não possui autorização** para comercialização ou revenda por terceiros, tampouco para engenharia reversa.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the release notes in English</summary><br>

# ACL Auditor — Release v2026.11.6.0

**ACL Auditor v2026.11.6.0**

**Release Tag:** `v2026.11.6.0`  
**Publication Date:** November 6, 2026  
**Author:** Fernando Nillsson Cidade  
**Direct Download:** [Download Latest Release from GitHub](https://github.com/fernandoncidade/ACL_Auditor_cpp/releases/latest) | [Install via Microsoft Store](https://apps.microsoft.com/detail/9NX5XZLL8WXW)  

---

## 1. Release Assets & Cryptographic Hashes

Verify your downloaded package before running installation scripts using PowerShell's built-in `Get-FileHash`:

```powershell
Get-FileHash -Path ".\ACLAuditor-v2026.11.6.0-x64-Setup.exe" -Algorithm SHA256
```

| Asset Name | Format | Target Architecture | Description |
|---|---|:---:|---|
| `ACLAuditor-v2026.11.6.0-x64-Setup.exe` | Inno Setup Installer | x64 (MSVC / MinGW) | Full graphical installer with Explorer shell integration and desktop shortcuts. |
| `ACLAuditor-v2026.11.6.0-x64-Portable.zip` | Portable ZIP | x64 | Zero-install standalone archive. Unzip and run anywhere without admin rights. |
| `Source code (zip)` | ZIP Archive | Any | Complete C++20 / Qt 6 source code. |
| `Source code (tar.gz)` | Tarball | Any | Complete C++20 / Qt 6 source code. |

---

## 2. Release Highlights

- **Visual Causal Effective Access Explainer:** Instantly answers *"Why does this user have permission?"* by evaluating the Windows token hierarchy, direct groups, nested/universal groups (`Everyone`, `Authenticated Users`), and DACL inheritance origin.
- **Automated Heuristic Risk Scanner:** Pinpoints World-Writable folders, Binary Planting / DLL Hijacking vectors (`.exe`, `.dll`, `.sys`, `.ps1`), excessive privileges (`WRITE_DAC`, `WRITE_OWNER`), and orphaned SIDs.
- **Windows Services & SMB Network Shares Auditor:** Scans Service Control Manager for Unquoted Service Paths and weak binary DACLs; audits SMB share permissions against underlying NTFS DACLs.
- **Local SQLite Snapshots & Temporal Diff:** Cryptographic SHA-256 fingerprinted snapshots saved locally in `%LOCALAPPDATA%/ACLAuditor`. Automated delta comparison highlights privilege escalations and compliance drift.
- **Multi-Format Executive Reporting:** Interactive HTML dashboards, formatted executive PDF (A4 with signature lines), CSV, and SIEM JSON.
- **Non-Destructive Remediation with Safe Rollback:** Generates `.ps1` and `.bat` remediation scripts enforcing preliminary backups (`icacls /save`) and guaranteed instant rollback routines (`icacls /restore`).
- **Windows Explorer Integration:** Right-click context menu integration registered in `HKCU`, requiring no administrator elevation.

---

## 3. System Requirements

- **Operating System:** Windows 11, Windows 10, Windows Server 2022 / 2019 (64-bit x64).
- **Permissions:** Standard user privileges for user files, personal folders, and network shares; Administrator elevation with `SeSecurityPrivilege` required for protected OS folders (e.g., `C:\System Volume Information`).
- **Memory:** Minimum 256 MB RAM (typical footprint < 85 MB).
- **Disk Space:** ~55 MB installed.

---

## 4. Known Issues & Workarounds

- **Protected System Folders:** Accessing `C:\System Volume Information` without Administrator elevation returns Win32 `ERROR_ACCESS_DENIED`. Workaround: Right-click `ACLAuditor.exe` and select *Run as administrator*.
- **Large Network Latency:** Scanning network shares over high-latency WAN connections may take longer. Workaround: Use the localized directory scan or run the headless CLI directly on the file server.

---

## 5. Uninstallation Instructions

- **Installer Version:** Open Windows Settings -> **Apps** -> **Installed Apps**, locate **ACL Auditor**, and click **Uninstall**. Alternatively, run `unins000.exe` in the application install directory.
- **Portable Version:** Simply delete the extracted folder.
- **Explorer Context Menu Removal:** If enabled, launch ACL Auditor, uncheck *Explorer Context Menu Integration* in Settings, or run:
  ```powershell
  Remove-Item -Recurse -Force "HKCU:\Software\Classes\Directory\shell\AclAuditor"
  ```

---

## 6. Licensing & Binary Distribution Terms

- **Open Source Code (GNU AGPLv3 License):** The complete source code for this release is freely available for study, modification, and custom builds under AGPLv3 copyleft terms.
- **Documentation in `source/assets/`:** Authored for the Microsoft Store; developers have complete freedom to modify, delete, add, and adapt it for their own platforms and countries.
- **Restriction on the Author's Official Binaries:** The official binary compiled, digitally signed, and packaged by the author (**Fernando Nillsson Cidade**), specifically on the Microsoft Store, **may NOT** be commercially resold, redistributed by third parties, or reverse-engineered.

</details>
