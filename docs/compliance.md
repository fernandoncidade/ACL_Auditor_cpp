# Mapeamento de Conformidade / Compliance Mapping — ACL Auditor

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o guia de conformidade em português</summary><br>

# ACL Auditor — Mapeamento de Conformidade Regulatória

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Frameworks:** CIS Benchmarks | NIST SP 800-53 | ISO/IEC 27001 | GDPR / LGPD  

---

## 1. Visão Geral de Conformidade

Empresas e órgãos públicos são obrigados por lei, normas setoriais e frameworks de cibersegurança a aplicar o Princípio do Menor Privilégio, eliminar heranças inseguras e documentar revisões periódicas de acesso em sistemas de arquivos. O ACL Auditor atende diretamente a essas exigências.

---

## 2. Mapeamento dos Frameworks

### 2.1. Benchmarks CIS para Microsoft Windows
- **CIS Seção 2.2 — Atribuição de Direitos de Usuário:**
  - Audita contas detentoras de privilégios como `SeSecurityPrivilege`, `SeTakeOwnershipPrivilege` e `SeBackupPrivilege`.
- **CIS Seção 18.9 — Listas de Controle de Acesso (ACLs) do Sistema:**
  - Exige que contas não-administrativas (`Users`, `Authenticated Users`, `Everyone`) não possuam direitos de gravação ou modificação na raiz do sistema (`C:\Windows`), pastas de programas (`C:\Program Files`) e binários de serviços.
  - O motor `RiskScanner` sinaliza automaticamente essas violações como riscos críticos.

### 2.2. NIST SP 800-53 (Revisão 5)
- **AC-6 — Menor Privilégio (Least Privilege):**
  - "A organização emprega o princípio do menor privilégio, permitindo apenas acessos estritamente necessários para o cumprimento das funções designadas."
  - *Papel do ACL Auditor:* O cálculo de Acesso Efetivo visual evidencia permissões excessivas ou acumuladas via grupos aninhados antes que sejam exploradas.
- **AC-3 — Aplicação de Controle de Acesso (Access Enforcement):**
  - Garante a segregação de funções e bloqueio de heranças indevidas.
- **AU-2 — Auditoria e Evidência Forense:**
  - Snapshots com assinatura criptográfica SHA-256 fornecem evidência de custódia imutável entre períodos de auditoria.

### 2.3. ISO/IEC 27001:2022
- **Controle A.9.1 — Requisitos de Negócio para Controle de Acesso:**
  - Verificação de políticas de acesso em compartilhamentos de rede e servidores de arquivos.
- **Controle A.9.2.5 — Revisão Periódica dos Direitos de Acesso:**
  - Comparação temporal diff entre snapshots históricos para identificar acúmulo clandestino de permissões (*permission creep*).
- **Controle A.9.4.5 — Controle de Acesso ao Código-Fonte de Programas:**
  - Identificação de diretórios de repositórios ou scripts com permissão de escrita para usuários comuns.

### 2.4. LGPD (Lei nº 13.709/2018) & GDPR (Regulamento UE 2016/679)
- **LGPD Art. 46 / GDPR Artigo 32 — Segurança do Tratamento:**
  - Exige medidas técnicas para proteger dados pessoais contra acessos não autorizados.
  - *Papel do ACL Auditor:* Identifica diretórios corporativos confidenciais (`RH`, `Financeiro`, `Folha`, `Prontuários`, `Backup`) expostos a usuários não autorizados devido a herança quebrada ou ACEs excessivamente amplas.

---

## 3. Entregáveis Executivos de Auditoria

O ACL Auditor gera artefatos prontos para auditorias internas e externas:
1. **Relatório Executivo em PDF:** Diagramado em A4 com sumário técnico, matriz de risco e espaço formal para assinatura do auditor e do cliente.
2. **Dashboard Interativo em HTML:** Painel web responsivo com filtros dinâmicos por severidade e entidade.
3. **Planilha Estruturada CSV (UTF-8 BOM):** Pronta para abertura imediata no Microsoft Excel e Power BI.
4. **JSON Completo:** Formato estruturado para integração com pipelines DevSecOps e ingestão em SIEMs (Splunk, Microsoft Sentinel, Elastic).

---

### ⚠️ Nota de Isenção Regulatória
O **ACL Auditor** é um utilitário técnico de software para apoio à auditoria, inspeção e diagnóstico de segurança no sistema de arquivos NTFS. A sua utilização e os relatórios por ele gerados não conferem por si sós certificação automática de conformidade nem substituem as políticas internas, processos de governança e controles formais exigidos por entidades reguladoras, legislações ou órgãos certificadores.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the compliance guide in English</summary><br>

# ACL Auditor — Regulatory Compliance Mapping

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Frameworks:** CIS Benchmarks | NIST SP 800-53 | ISO/IEC 27001 | GDPR / LGPD  

---

## 1. Compliance Overview

Enterprises are required by law, regulation, and cybersecurity standards to enforce least privilege access controls, eliminate insecure permission inheritance, and document regular audits of sensitive file systems. ACL Auditor directly maps to the controls outlined below.

---

## 2. Framework Mappings

### 2.1. CIS Microsoft Windows Benchmarks
- **CIS Section 2.2 — User Rights Assignment:**
  - Audits who possesses `SeSecurityPrivilege`, `SeTakeOwnershipPrivilege`, and `SeBackupPrivilege`.
- **CIS Section 18.9 — System Access Control Lists:**
  - Enforces that non-administrative accounts (`Users`, `Authenticated Users`, `Everyone`) have no write access to system root (`C:\Windows`), `C:\Program Files`, and service executable locations.
  - Automatically flags violations via the `RiskScanner` engine.

### 2.2. NIST SP 800-53 (Revision 5)
- **AC-6 — Least Privilege:**
  - "The organization employs the principle of least privilege, allowing only authorized accesses for users (and processes acting on behalf of users) which are necessary to accomplish assigned tasks."
  - *ACL Auditor role:* Visual Effective Access calculation highlights overly broad access grants before they can be exploited.
- **AC-3 — Access Enforcement:**
  - Ensures Discretionary Access Control Lists (DACLs) enforce strict role segregation.
- **AU-2 — Event Logging & Forensic Integrity:**
  - Cryptographic SHA-256 baseline snapshots provide verifiable proof of compliance state across audit periods.

### 2.3. ISO/IEC 27001:2022
- **Control A.9.1 — Business Requirements of Access Control:**
  - Verification that access policies are enforced across network shares and file servers.
- **Control A.9.2.5 — Review of User Access Rights:**
  - Periodic review of file permissions with temporal diff comparisons between historical snapshots to detect permission creep.
- **Control A.9.4.5 — Access Control to Program Source Code:**
  - Identification of world-writable repository directories and unauthorized developer access grants.

### 2.4. GDPR (EU 2016/679) & Brazilian LGPD (Lei nº 13.709/2018)
- **GDPR Article 32 / LGPD Art. 46 — Security of Processing:**
  - Mandates organizational and technical measures to protect Personally Identifiable Information (PII) against unauthorized disclosure or alteration.
  - *ACL Auditor role:* Locates sensitive corporate folders (`HR`, `Finance`, `Payroll`, `Medical`, `Backup`) exposed to non-authorized personnel via inheritance breaks or overly permissive ACEs.

---

## 3. Executive Audit Deliverables

ACL Auditor generates audit artifacts ready for submission to internal and external auditors:
1. **Executive PDF Report:** Includes executive summary, severity scorecard, risk breakdown, and physical signature/sign-off blocks.
2. **Interactive HTML Dashboard:** Standalone, client-facing dashboard with live filtering by severity, trustee, and risk type.
3. **Structured CSV (UTF-8 BOM):** Prepared for immediate pivot table analysis in Microsoft Excel and Power BI.
4. **Machine-Readable JSON:** Directly ingestible into SIEM platforms (Splunk, Microsoft Sentinel, Elastic) and DevSecOps compliance pipelines.

---

### ⚠️ Regulatory Disclaimer
**ACL Auditor** is a technical software utility for NTFS permission inspection, risk assessment, and forensic auditing support. Its use and generated deliverables do not in themselves grant automated compliance certification, nor do they substitute for formal internal policies, organizational governance processes, or security controls required by regulatory authorities, legislation, or accredited certification bodies.

</details>
