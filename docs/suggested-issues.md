# Sugestões de Issues Iniciais / Starter Issues — ACL Auditor

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher as sugestões de issues em português</summary><br>

# Issues Recomendadas para Engajamento da Comunidade

Abaixo estão 7 templates de issues prontas para você publicar no GitHub em [Issues -> New Issue](https://github.com/fernandoncidade/ACL_Auditor_cpp/issues/new), facilitando a colaboração de desenvolvedores e administradores Windows:

---

### Issue 1: Expansão Recursiva de Grupos de Domínio do Active Directory
- **Título:** `[FEATURE] Support recursive Active Directory domain group expansion`
- **Labels:** `enhancement`, `help wanted`, `active-directory`
- **Conteúdo:**
```markdown
### Resumo
Atualmente, o `EffectiveAccessCalculator` resolve grupos locais e conhecidos (Everyone, Authenticated Users, BUILTIN\Users) e grupos de domínio diretos.

### Melhoria Proposta
Adicionar resolução recursiva de grupos globais e universais aninhados do Active Directory via `NetUserGetLocalGroups` e consultas ADSI / LDAP quando conectado a um domínio.

### Critérios de Aceite
- [ ] Resolução de hierarquias de grupos aninhados em múltiplos níveis.
- [ ] Fallback gracioso para grupos locais se o Controlador de Domínio estiver inacessível.
- [ ] Inclusão de mock de testes para grupos de domínio.
```

---

### Issue 2: Agendamento de Varreduras e Auditoria em Segundo Plano
- **Título:** `[FEATURE] Support scheduled scans via Windows Task Scheduler integration`
- **Labels:** `enhancement`, `automation`
- **Conteúdo:**
```markdown
### Resumo
Administradores corporativos frequentemente precisam de varreduras agendadas (ex: baselines semanais) para detectar acúmulo de privilégios.

### Melhoria Proposta
Adicionar comando no CLI ou opção na interface gráfica para registrar uma tarefa no Agendador de Tarefas do Windows que execute `acl-auditor-cli.exe scan <caminho> --snapshot` automaticamente.

### Critérios de Aceite
- [ ] Comando: `acl-auditor-cli.exe schedule --path "D:\Dados" --interval daily --time 02:00`
- [ ] Criação de Tarefa Agendada do Windows em `\ACLAuditor\DailyScan`.
- [ ] Salvamento automático do snapshot no SQLite local (`%LOCALAPPDATA%/ACLAuditor/acl_auditor_snapshots.db`).
```

---

### Issue 3: Otimização de Virtualização para Árvores com Mais de 1 Milhão de Arquivos
- **Título:** `[PERF] Optimize memory and caching for file trees with > 1,000,000 objects`
- **Labels:** `performance`, `c++20`
- **Conteúdo:**
```markdown
### Resumo
O modelo virtualizado opera com alta performance em centenas de milhares de arquivos, mas diretórios gigantescos com mais de 1 milhão de objetos podem causar picos de memória.

### Melhoria Proposta
- Implementar cache LRU para nós filhos de `FileSecurityNode` no `AclTreeModel`.
- Pré-carregamento assíncrono em lotes para a área visível do viewport.

### Critérios de Aceite
- [ ] Consumo de memória abaixo de 150 MB em volumes com mais de 1.000.000 de arquivos.
- [ ] Rolagem fluida na árvore virtual sem micro-travamentos.
```

---

### Issue 4: Adicionar Traduções para Alemão e Espanhol
- **Título:** `[I18N] Add German (de_DE) and Spanish (es_ES) language translations`
- **Labels:** `i18n`, `good first issue`
- **Conteúdo:**
```markdown
### Resumo
O ACL Auditor suporta atualmente Inglês (`en_US`) e Português (`pt_BR`). A adição de Alemão e Espanhol ampliará a adoção nos mercados corporativos europeu e latino-americano.

### Etapas de Implementação
1. Adicionar `aclauditor_de_DE.ts` e `aclauditor_es_ES.ts` em `source/language/translations/`.
2. Traduzir strings da interface e mensagens de conformidade.
3. Atualizar alvos de compilação de tradução no `CMakeLists.txt`.
```

---

### Issue 5: Automação de Releases (ZIP Portátil e Instalador) no GitHub Actions
- **Título:** `[CI/CD] Automate GitHub Releases artifact generation on version tags`
- **Labels:** `ci`, `devops`
- **Conteúdo:**
```markdown
### Resumo
Ao enviar uma tag de versão (ex: `v2026.11.6.0`), o GitHub Actions deve compilar o projeto, executar testes, empacotar o instalador `ACLAuditor-x64-Setup.exe` (via Inno Setup) e o arquivo `ACLAuditor-x64-Portable.zip`, calcular hashes SHA-256 e criar uma Release no GitHub.

### Critérios de Aceite
- [ ] Disparo automático em tags `v*`.
- [ ] Execução da compilação CMake e suíte de testes.
- [ ] Geração de arquivo `checksums.txt` com hashes SHA-256.
- [ ] Criação de release com os binários anexados.
```

---

### Issue 6: Adicionar Exportação CSV ao Utilitário CLI
- **Título:** `[CLI] Add structured CSV export option to acl-auditor-cli`
- **Labels:** `cli`, `good first issue`
- **Conteúdo:**
```markdown
### Resumo
O utilitário CLI standalone (`acl-auditor-cli.exe`) suporta saída em texto e JSON. A adição de exportação direta em CSV (`--format csv`) facilitará a importação de dados no Excel e scripts PowerShell.

### Critérios de Aceite
- [ ] `acl-auditor-cli.exe scan "C:\inetpub" --format csv > resultados.csv`
- [ ] Colunas: Path, IsDirectory, Owner, InheritanceBlocked, RiskScore, RiskTitle, Severity, Remediation.
- [ ] Formatação UTF-8 BOM para compatibilidade com o Microsoft Excel.
```

---

### Issue 7: Módulo PowerShell Oficial (`AclAuditor.psm1`)
- **Título:** `[FEATURE] Create PowerShell module wrapper for ACL Auditor CLI`
- **Labels:** `powershell`, `sysadmin`
- **Conteúdo:**
```markdown
### Resumo
Administradores Windows utilizam intensamente o PowerShell. O fornecimento de um módulo oficial empacotando o `acl-auditor-cli.exe` tornará a automação ainda mais nativa.

### Cmdlets Propostos
- `Get-AclAuditorReport -Path "C:\Dados"`
- `Test-AclAuditorAccess -Path "C:\Dados" -User "Everyone"`
- `Get-AclAuditorServiceRisk`
- `Get-AclAuditorShareRisk`
```

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the suggested issues in English</summary><br>

# Starter Issues for ACL Auditor Community

Here are 7 ready-to-use GitHub Issue templates for community contributors. You can create these issues directly on your repository under [Issues -> New Issue](https://github.com/fernandoncidade/ACL_Auditor_cpp/issues/new).

---

### Issue 1: Add Active Directory Domain Group Recursive Expansion
- **Title:** `[FEATURE] Support recursive Active Directory domain group expansion`
- **Labels:** `enhancement`, `help wanted`, `active-directory`
- **Body:**
```markdown
### Summary
Currently, `EffectiveAccessCalculator` evaluates local and well-known groups (Everyone, Authenticated Users, BUILTIN\Users) and direct domain groups.

### Proposed Improvement
Add recursive expansion for nested Active Directory domain global and universal security groups using `NetUserGetLocalGroups` and ADSI / LDAP queries when connected to a domain-joined machine.

### Acceptance Criteria
- [ ] Resolves nested group membership across multi-level Active Directory hierarchies.
- [ ] Gracefully falls back to direct groups if DC is unreachable or machine is not domain-joined.
- [ ] Includes unit test mock for nested AD groups.
```

---

### Issue 2: Support Scheduled Scans & Automated Background Audit
- **Title:** `[FEATURE] Support scheduled scans via Windows Task Scheduler integration`
- **Labels:** `enhancement`, `automation`
- **Body:**
```markdown
### Summary
Enterprise administrators often require recurring scans (e.g. daily or weekly baselines) to detect privilege creep.

### Proposed Improvement
Add a CLI switch or GUI option to register a recurring Windows Scheduled Task that runs `acl-auditor-cli.exe scan <path> --snapshot` automatically.

### Acceptance Criteria
- [ ] Command: `acl-auditor-cli.exe schedule --path "D:\Data" --interval daily --time 02:00`
- [ ] Creates Windows Scheduled Task under `\ACLAuditor\DailyScan`.
- [ ] Saves point-in-time snapshot directly into `%LOCALAPPDATA%/ACLAuditor/acl_auditor_snapshots.db`.
```

---

### Issue 3: Virtualization Tuning for Multi-Million Object Trees
- **Title:** `[PERF] Optimize memory and caching for file trees with > 1,000,000 objects`
- **Labels:** `performance`, `c++20`
- **Body:**
```markdown
### Summary
The virtualized model handles directories with hundreds of thousands of objects well, but very deep trees with over a million files can cause memory spikes.

### Proposed Improvement
- Introduce an LRU cache for `FileSecurityNode` children in `AclTreeModel`.
- Implement background chunked pre-fetching for visible viewport items.

### Acceptance Criteria
- [ ] Memory footprint remains below 150 MB when browsing volumes with >1,000,000 objects.
- [ ] No UI micro-stutters during rapid scrolling in virtual tree.
```

---

### Issue 4: Add German and Spanish Translations
- **Title:** `[I18N] Add German (de_DE) and Spanish (es_ES) language translations`
- **Labels:** `i18n`, `good first issue`
- **Body:**
```markdown
### Summary
ACL Auditor currently supports English (`en_US`) and Portuguese (`pt_BR`). Adding German and Spanish will significantly expand adoption in European and Latin American enterprise markets.

### Implementation Steps
1. Add `aclauditor_de_DE.ts` and `aclauditor_es_ES.ts` to `source/language/translations/`.
2. Translate UI strings and compliance summaries.
3. Update `CMakeLists.txt` translation generation targets.
```

---

### Issue 5: Automated GitHub Actions Release Packaging for Portable ZIP & Setup
- **Title:** `[CI/CD] Automate GitHub Releases artifact generation on version tags`
- **Labels:** `ci`, `devops`
- **Body:**
```markdown
### Summary
When a new version tag (e.g., `v2026.11.6.0`) is pushed, GitHub Actions should automatically build the project, run tests, package `ACLAuditor-x64-Setup.exe` (via Inno Setup) and `ACLAuditor-x64-Portable.zip`, compute SHA-256 hashes, and draft a GitHub Release.

### Acceptance Criteria
- [ ] Trigger on `v*` tags.
- [ ] Runs CMake build + tests.
- [ ] Generates SHA-256 checksums in `checksums.txt`.
- [ ] Creates GitHub Release with assets attached.
```

---

### Issue 6: Add CSV and JSON Export to CLI Tool
- **Title:** `[CLI] Add structured CSV export option to acl-auditor-cli`
- **Labels:** `cli`, `good first issue`
- **Body:**
```markdown
### Summary
The standalone CLI (`acl-auditor-cli.exe`) supports human-readable text and JSON. Adding direct CSV export (`--format csv`) will allow administrators to pipe scan results directly into Excel and PowerShell reporting scripts.

### Acceptance Criteria
- [ ] `acl-auditor-cli.exe scan "C:\inetpub" --format csv > results.csv`
- [ ] Columns: Path, IsDirectory, Owner, InheritanceBlocked, RiskScore, RiskTitle, Severity, Remediation.
- [ ] Formatted with UTF-8 BOM for Microsoft Excel compatibility.
```

---

### Issue 7: Interactive PowerShell Module Wrapper (`AclAuditor.psm1`)
- **Title:** `[FEATURE] Create PowerShell module wrapper for ACL Auditor CLI`
- **Labels:** `powershell`, `sysadmin`
- **Body:**
```markdown
### Summary
Windows sysadmins frequently use PowerShell for automation. Providing an official PowerShell module wrapping `acl-auditor-cli.exe` would make integration effortless.

### Proposed Cmdlets
- `Get-AclAuditorReport -Path "C:\Data"`
- `Test-AclAuditorAccess -Path "C:\Data" -User "Everyone"`
- `Get-AclAuditorServiceRisk`
- `Get-AclAuditorShareRisk`
```

</details>
