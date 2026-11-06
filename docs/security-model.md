# Modelo de Segurança / Security Model & Threat Analysis — ACL Auditor

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o modelo de segurança em português</summary><br>

# ACL Auditor — Modelo de Segurança e Análise de Ameaças

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Nível de Segurança:** 100% Local-First | Zero Telemetria | Inspeção Somente Leitura  
**Contexto Operacional:** Windows 10 / 11 / Windows Server 2022+  

---

## 1. Modelo de Ameaças e Garantias de Design

O ACL Auditor foi construído especificamente para ambientes corporativos com requisitos estritos de segurança, incluindo o setor financeiro, operadores de infraestrutura crítica e órgãos governamentais.

### 1.1. Garantias Centrais de Segurança
1. **Zero Comunicação Externa:**
   - A aplicação não realiza conexões de rede de entrada ou saída.
   - Não há código de telemetria, análise comportamental, rastreamento ou relatórios automáticos de falhas na base de código.
2. **Inspeção Estritamente Somente Leitura:**
   - Descritores de segurança, DACLs, SACLs, serviços e compartilhamentos SMB são consultados unicamente com direitos de leitura (`READ_CONTROL`, `GENERIC_READ`).
   - Handles de arquivos são abertos com as flags `FILE_READ_ATTRIBUTES` e `FILE_READ_EA`; o conteúdo físico dos arquivos jamais é aberto, lido ou modificado durante as varreduras de ACL.
3. **Remediação Não-Destrutiva com Reversão Garantida:**
   - A aplicação não executa correções diretamente em segundo plano no disco.
   - Os planos de remediação geram scripts transparentes e auditáveis (`.ps1` e `.bat`).
   - Todos os scripts exigem uma etapa preliminar de backup (`icacls <caminho> /save <backup> /t /c`) antes de aplicar qualquer alteração, acompanhados de rotina de restauração (`icacls <caminho> /restore <backup>`).

---

## 2. Avaliação de Controle de Acesso no Windows NT

### 2.1. Algoritmo de Precedência Canônica de ACEs
O Windows NT impõe uma ordem determinística e estrita na avaliação de Discretionary Access Control Lists (DACLs):

```
       [Solicitação de Acesso para Entidade (SIDs de Usuário e Grupos)]
                                      |
                                      v
                 [Verificar ACEs Explícitas de NEGAÇÃO (DENY)] ------> [NEGADO se corresponder]
                                      |
                                      v
                 [Verificar ACEs Explícitas de CONCESSÃO (ALLOW)] ---> [Acumular Direitos]
                                      |
                                      v
                 [Verificar ACEs Herdadas de NEGAÇÃO (DENY)] --------> [NEGADO se corresponder]
                                      |
                                      v
                 [Verificar ACEs Herdadas de CONCESSÃO (ALLOW)] ------> [Acumular Direitos]
                                      |
                                      v
                   [Todos os direitos solicitados foram atendidos?]
                                   /                \
                                (Sim)              (Não)
                                 /                   \
                       [ACESSO CONCEDIDO]      [NEGAÇÃO IMPLÍCITA]
```

### 2.2. Simulação de Acesso Efetivo
O ACL Auditor simula a lógica do Security Reference Monitor (SRM) do Windows:
1. Resolve a conta analisada em seu token completo de segurança (SID principal, grupos de domínio, grupos locais aninhados e aliases universais como `S-1-1-0 Everyone`, `S-1-5-11 Authenticated Users`, `S-1-5-32-545 BUILTIN\Users`).
2. Avalia as regras da DACL na ordem canônica.
3. Constrói a árvore genealógica de acesso rastreando a origem causal de cada permissão:
   ```text
   Identidade: joao.silva (S-1-5-21-100234-...)
    └── Membro do Grupo: TI-Suporte (S-1-5-21-445892-...)
         └── Grupo Aninhado: Operações-Infra
              └── Origem da Regra: Herdada de D:\Departamentos
                   └── Permissão Efetiva: MODIFICAR (FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE)
   ```

---

## 3. Motor Heurístico de Detecção de Riscos

| Identificador do Risco | Heurística de Detecção | Severidade | Referência Mitre ATT&CK |
|---|---|:---:|:---:|
| **Diretório World-Writable** | Direitos de Gravação/Modificação atribuídos a `Everyone`, `BUILTIN\Users` ou `Authenticated Users`. | **Alto** / **Crítico** | T1078, T1036 |
| **Binary Planting / DLL Hijacking** | Extensões executáveis (`.exe`, `.dll`, `.sys`, `.ps1`, `.bat`) em pastas com escrita para usuários comuns. | **Crítico** | T1574.001, T1574.002 |
| **Privilégio Excessivo (WRITE_DAC / WRITE_OWNER)** | Contas não-administrativas com direito de alterar DACLs ou apropriar-se do objeto. | **Crítico** | T1098, T1222 |
| **SIDs Órfãos** | ACEs contendo SIDs não resolvidos de contas ou domínios excluídos. | **Médio** | T1098 |
| **Caminho de Serviço sem Aspas com Espaços** | Caminho executável de serviço com espaços e sem aspas em partição gravável. | **Alto** | T1574.009 |
| **Binário de Serviço Gravável** | Executável de serviço do SCM passível de modificação por usuários comuns. | **Crítico** | T1574.010 |
| **Discrepância SMB vs. NTFS** | Compartilhamento concede Controle Total na rede, mas NTFS local possui permissões conflitantes. | **Médio** / **Alto** | T1021.002 |

---

## 4. Integridade Criptográfica e Cadeia de Custódia

Para assegurar validade pericial e suporte probatório em auditorias:
- Cada snapshot gera um hash **CNG SHA-256** contemplando a totalidade das permissões, donos e estruturas inspecionadas.
- Garante a comprovação irrefutável de que o laudo emitido reflete o estado fático do sistema na data do exame.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the security model in English</summary><br>

# ACL Auditor — Security Model & Threat Analysis

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Security Level:** 100% Local-First | Zero Telemetry | Read-Only Inspection  
**Operating Context:** Windows 10 / 11 / Windows Server 2022+  

---

## 1. Threat Model & Design Guarantees

ACL Auditor is specifically designed for security-conscious enterprise environments, including defense contractors, banking institutions, and critical infrastructure operators.

### 1.1. Core Security Guarantees
1. **Zero External Communications:**
   - The application makes no outgoing or incoming network connections.
   - No telemetry, analytics, crash reporting, or telemetry beacons exist in the codebase.
2. **Strictly Read-Only Inspection:**
   - Security descriptors, DACLs, SACLs, services, and SMB shares are queried exclusively with read privileges (`READ_CONTROL`, `GENERIC_READ`).
   - File handles are opened with `FILE_READ_ATTRIBUTES` and `FILE_READ_EA` flags; file contents are never accessed or altered during ACL scans.
3. **Safe Remediation with Guaranteed Rollback:**
   - The application does not apply changes directly in the background.
   - Remediation plans generate transparent, auditable scripts (`.ps1` and `.bat`).
   - All scripts enforce a mandatory backup step (`icacls <path> /save <backup_file> /t /c`) before executing modifications, alongside a generated rollback script (`icacls <path> /restore <backup_file>`).

---

## 2. Windows NT Access Control Evaluation

### 2.1. Canonical ACE Precedence Algorithm
Windows NT enforces a strict deterministic ordering when evaluating Discretionary Access Control Lists (DACLs):

```
       [Request Access for Trustee (User + Group SIDs)]
                              |
                              v
                 [Check Explicit DENY ACEs] -----------------> [DENIED if Match]
                              |
                              v
                 [Check Explicit ALLOW ACEs] ---------------> [Accumulate Rights]
                              |
                              v
                 [Check Inherited DENY ACEs] ---------------> [DENIED if Match]
                              |
                              v
                 [Check Inherited ALLOW ACEs] --------------> [Accumulate Rights]
                              |
                              v
             [Are all requested rights satisfied?]
                    /                   \
                 (Yes)                  (No)
                  /                       \
        [ACCESS GRANTED]             [DEFAULT DENY]
```

### 2.2. Effective Access Simulation
ACL Auditor simulates the Windows Security Reference Monitor (SRM) by:
1. Resolving the target user account into its full security token, including primary SID, domain group SIDs, nested local group SIDs, and well-known aliases (`S-1-1-0 Everyone`, `S-1-5-11 Authenticated Users`, `S-1-5-32-545 BUILTIN\Users`).
2. Iterating through DACL entries according to canonical precedence.
3. Generating a causal explanation tree tracing the precise origin of granted or denied permissions:
   ```text
   Identity: john.doe (S-1-5-21-100234-...)
    └── Group Membership: IT-Support (S-1-5-21-445892-...)
         └── Nested Group: Infra-Operations
              └── Rule Inheritance: Inherited from D:\Departments
                   └── Effective Access: MODIFY (FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE)
   ```

---

## 3. Heuristic Risk Detection Engine

ACL Auditor inspects file systems and services for classic high-impact Windows vulnerabilities:

| Risk Identifier | Detection Heuristic | Severity | Mitre ATT&CK Reference |
|---|---|:---:|:---:|
| **World-Writable Directory** | Write/Modify permissions granted to `Everyone`, `BUILTIN\Users`, or `Authenticated Users`. | **High** / **Critical** | T1078, T1036 |
| **Binary Planting / DLL Hijacking** | Executable file extensions (`.exe`, `.dll`, `.sys`, `.ps1`, `.bat`) located in directories with standard user write permissions. | **Critical** | T1574.001, T1574.002 |
| **Excessive Privilege (WRITE_DAC / WRITE_OWNER)** | Non-administrative trustees granted permission to modify DACLs or take ownership of sensitive system files. | **Critical** | T1098, T1222 |
| **Orphaned SIDs** | ACEs containing unresolvable SIDs left behind by deleted users or obsolete Active Directory domains. | **Medium** | T1098 |
| **Unquoted Service Paths** | Windows service executable paths containing spaces without surrounding quotation marks located on writable partitions. | **High** | T1574.009 |
| **Writable Service Binaries** | Service binaries registered in the SCM that can be modified or overwritten by standard user accounts. | **Critical** | T1574.010 |
| **SMB vs. NTFS Discrepancy** | Share-level DACL allows Write/Full Control, but underlying NTFS folder DACL is misconfigured or inconsistent. | **Medium** / **High** | T1021.002 |

---

## 4. Cryptographic Integrity & Chain of Custody

To guarantee compliance and forensic defensibility:
- Snapshots calculate a **CNG SHA-256** cryptographic hash over all collected permissions and structure metadata.
- Hashed snapshots allow verifiable integrity checks over time, ensuring evidence submitted in audits has not been tampered with.

</details>
