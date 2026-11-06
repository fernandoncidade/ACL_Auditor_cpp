<!-- Multilanguage OBSERVACAO.md for ACL_Auditor -->

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> |
  <a href="#enus">🇺🇸 English (US)</a> |
  <a href="#tree">🌳 Árvore de Arquivos / File Tree</a>
</p>

---

<h2 id="ptbr">🇧🇷 Português (BR)</h2>

<details>
<summary>Clique para expandir / recolher a Observação Técnica em português</summary>

# ACL AUDITOR - OBSERVAÇÃO TÉCNICA E POSICIONAMENTO ESTRATÉGICO

Versão documentada: `2026.11.6.0`  
Data do snapshot técnico: `6 de novembro de 2026`  
Repositório: `ACL_Auditor_cpp`  
Autor identificado no projeto: `Fernando Nillsson Cidade`  
Licença do repositório: `GNU AGPLv3 (GNU Affero General Public License v3)`  

## 1. Estado atual do projeto e Posicionamento

O repositório atual do **ACL Auditor** consolida uma plataforma desktop corporativa **nativa em C++20 com Qt 6.11.1** compilada via CMake e Ninja. O produto transcende a categoria de simples visualizador de listas de permissões ou utilitários legados de linha de comando (como `icacls` e `accesschk`), posicionando-se como uma **Plataforma Local de Auditoria, Conformidade e Explicabilidade de Acessos NTFS** (*Local NTFS Access Compliance, Explanation & Audit Platform*).

Sua proposta de valor central responde diretamente à pergunta de governança mais crítica em infraestruturas Windows:
> **“Por que este usuário específico consegue ler, alterar ou executar este arquivo?”**
> *(“Why does this specific user have permission to read, modify, or execute this file?”)*

O produto atua como uma solução soberana **100% Local-First** (sem nuvem, sem telemetria, sem contas e sem dependência de serviços externos), operando estritamente em **modo somente leitura** durante todas as rotinas de análise e diagnóstico.

## 2. Linha Técnica e Destaques da Versão 2026.11.6.0

Esta versão consolida a maturidade técnica da engine e a estabilização completa dos subsistemas de auditoria corporativa:

- **Engine Win32 nativa em C++20 (`ACLAuditorEngine`):** Inspeção de baixo nível via `Advapi32`, `Authz`, `Netapi32` e `BCrypt`, com resolução estrita de descritores de segurança, identidades de domínio e grupos universais;
- **Explicador visual de acesso efetivo (`VisualExplanationWidget`):** Avaliação token-based simulando o Security Reference Monitor (SRM), com árvore genealógica de acesso e parecer técnico em linguagem natural;
- **Scanner heurístico de conformidade (`RiskScanner`):** Categorização determinística de vulnerabilidades (World-Writable, DLL Hijacking / Binary Planting, privilégios excessivos `WRITE_DAC`/`WRITE_OWNER`, contas órfãs e herança quebrada);
- **Auditoria de serviços e compartilhamentos (`ServicesSharesWidget`):** Detecção de *Unquoted Service Paths*, executáveis de serviço graváveis e matriz de discrepâncias entre permissões de compartilhamento SMB e DACLs NTFS locais;
- **Persistência local e diff temporal (`SnapshotEngine`):** Banco local embutido SQLite3 com assinatura de custódia pericial **SHA-256** e identificação automatizada de deriva de conformidade (*compliance drift*);
- **Modo consultoria e relatórios executivos (`ReportExportDialog`):** Exportação corporativa pronta para entrega a clientes em HTML Interativo, PDF Executivo A4, CSV Estruturado (UTF-8 BOM) e JSON para DevSecOps/SIEM;
- **Remediação segura e não-destrutiva (`RemediationDialog`):** Geração de scripts em PowerShell e `icacls` com backup prévio obrigatório (`/save`) e reversão imediata garantida (`/restore`);
- **Integração com o Windows Explorer (`ExplorerIntegration`):** Acionamento via menu de contexto de shell registrado em `HKCU`, sem exigência de elevação administrativa.

## 3. Resumo estrutural consolidado

Contagem validada diretamente na árvore de código-fonte `source/`:

| Camada | `.cpp` | `.hpp` | Observação Técnica |
|---|---:|---:|---|
| `source/engine` | 7 | 8 | Núcleo de segurança Win32, acesso efetivo, scanner de risco, serviços, SMB, SQLite e remediação |
| `source/model` | 4 | 4 | Modelos virtuais `QAbstractItemModel` para árvore NTFS, tabela de ACEs, riscos e deltas de snapshot |
| `source/ui` | 11 | 11 | Janela principal, diálogos de manual, sobre, relatórios, remediação e widgets visuais |
| `source/utils` | 3 | 3 | Integração com Explorer, utilitários de caminho e gerenciador de logs |
| `source/language` | 1 | 1 | Gerenciador central de internacionalização e alternância dinâmica em tempo de execução |
| `source/cli` | 1 | 0 | Utilitário Standalone de Linha de Comando (CLI Headless) para automação e CI/CD (`source/cli/main_cli.cpp`) |
| `source/assets` | 0 | 0 | Documentos embarcados (ABOUT, EULA, LICENSES, NOTICES, PRIVACY_POLICY, RELEASE e ícones) |
| Raiz de `source/` | 1 | 0 | Ponto de entrada secundário e bootstrap de inicialização (`source/main.cpp`) |
| **Total em `source/`** | **28** | **27** | **Base C++20 da aplicação principal e CLI (55 arquivos)** |

Arquivos adicionais fora de `source/`:
- `main.cpp`: Ponto de entrada principal da aplicação executável;
- `mocks/teste.cpp`: Suíte de testes automatizados com 7 fases de validação da engine (localizado no diretório `mocks/`);
- `mocks/`: 4 ferramentas e utilitários auxiliares com 4 arquivos `.cpp` e 3 arquivos `.hpp` (Total geral do repositório: 33 `.cpp` e 30 `.hpp`);
- `docs/`: 7 especificações modulares cobrindo arquitetura, modelo de segurança, conformidade, referência da CLI, licenciamento e guia do usuário;
- `repo/`: Scripts Inno Setup, manifesto MSIX com `AppExecutionAlias` e manuais de ambiente.

## 4. Build, toolchain e dependências reais

Dados validados no arquivo `CMakeLists.txt` e `CMakePresets.json`:

- Requisito de CMake: `cmake_minimum_required(VERSION 3.25)`;
- Padrão de linguagem: `C++20` (com `set(CMAKE_CXX_STANDARD 20)` e `CMAKE_CXX_STANDARD_REQUIRED ON`);
- Recursos de automação Qt: `CMAKE_AUTOMOC ON` habilitado globalmente;
- Compilação otimizada em Release: flags `/O2 /Oi /Ot /fp:fast` no MSVC e `-O3 -ffunction-sections -fdata-sections` no MinGW;
- Configuração de runtime: biblioteca dinâmica multithread (`MultiThreadedDLL` no MSVC).

Componentes Qt 6.11.1 exigidos pela aplicação:
- `Qt6::Core`: Estrutura fundamental, tipos primitivos, sistema de eventos e I/O;
- `Qt6::Gui`: Gerenciamento de janelas, renderização gráfica e integração com DWM;
- `Qt6::Widgets`: Elementos visuais da interface desktop corporativa;
- `Qt6::Sql`: Camada de persistência local com o driver embutido `QSQLITE`;
- `Qt6::Network`: Resolução de identificadores de rede e suporte a transportes locais;
- `Qt6::Pdf`: Renderização vetorial e suporte à exportação de relatórios;
- `Qt6::Svg`: Renderização nítida de ícones vetoriais escaláveis.

Bibliotecas nativas do sistema operacional Windows vinculadas:
- `Advapi32`: Manipulação de Security Descriptors, DACLs, SACLs, tokens e SIDs (`GetNamedSecurityInfoW`, `LookupAccountSidW`);
- `Authz`: Avaliação e simulação de contextos de segurança de clientes;
- `Netapi32`: Enumeração de compartilhamentos de rede SMB via `NetShareGetInfo`;
- `BCrypt`: Geração criptográfica de hashes forenses SHA-256 via Windows CNG API;
- `Dwmapi`: Integração com o Desktop Window Manager (efeito Mica e tema escuro imersivo no Windows 11);
- `User32`: Consultas de ambiente de desktop e propriedades de janelas nativas.

## 5. Diretórios auxiliares fora de `source/`

Além do código-fonte principal em `source/`, o repositório mantém estrutura profissional de apoio e automação:

- `mocks/`: Contém os utilitários de suporte `find_VersionEditor`, `CompileTranslationsGUI` e `CompileTranslationsTerminal`, com `CMakeLists.txt` e compilação isolada;
- `repo/`: Scripts de empacotamento Inno Setup (`ACL_Auditor.iss`, `ACL_Auditor_MinGW.iss`, `ACL_Auditor_MSVC.iss`) e manuais técnicos de ambiente;
- `cmake/`: Módulos de deploy (`deploy_optional_qt_binary.cmake`), geração de ícone de recurso (`windows_app_icon.rc.in`) e scripts auxiliares;
- `.vscode/`: Configurações de workspace, tarefas automatizadas (`tasks.json`) e definições de IntelliSense (`c_cpp_properties.json`);
- `build/`: Pastas de compilação isoladas por compilador (`build_ACLAuditor_mingw`, `build_ACLAuditor_msvc`, `build_Mocks_mingw`, `build_Mocks_msvc`).

## 6. Ponto de entrada e bootstrap

Fluxo de inicialização auditado no código:

1. `main.cpp`:
   - Ponto de entrada canônico que repassa imediatamente o controle para `AclAuditor::runApplication(argc, argv)`.

2. `source/main.cpp`:
   - Configura políticas de escalonamento High-DPI (`Qt::HighDpiExecutionPolicy::PassThrough`);
   - Instancia a `QApplication` definindo nome corporativo (`ACLAuditor`), organização e versão oficial;
   - Inicializa o subsistema de log `LogManager` para registro em arquivo local e console;
   - Carrega o `GerenciadorTraducao`, detectando o idioma do sistema operacional e aplicando o catálogo `.qm` correspondente;
   - Processa argumentos de linha de comando (`--scan`, `--snapshot`, `--report`, `--version`, `--help`);
   - Instancia a `MainWindow`, aplica os efeitos DWM Mica / Tema Escuro e exibe a interface ao operador.

## 7. Estrutura atual por camada

A aplicação é estritamente segmentada em responsabilidades desacopladas:

- `source/engine`: Implementa as rotinas de baixo nível com Win32 Security, cálculos matemáticos de acesso efetivo, heurísticas de risco e persistência SQLite;
- `source/model`: Fornece os modelos `AclTreeModel`, `AceTableModel`, `RiskTableModel` e `DiffTableModel`, integrando a engine à visualização gráfica;
- `source/ui`: Contém a interface do usuário, incluindo a janela principal, os cartões de métricas, o painel de serviços/SMB e os diálogos especializados;
- `source/utils`: Encapsula o gerenciamento de logs, cálculo de caminhos da aplicação e scripts de integração com o shell do Windows;
- `source/language`: Provê a infraestrutura de internacionalização dinâmica, permitindo alternar idiomas em tempo real sem reinício;
- `source/assets`: Armazena os textos legais, termos de privacidade, avisos de terceiros e a coleção completa de ícones da aplicação.

## 8. Arquitetura funcional e Engine Win32

O motor de auditoria opera diretamente sobre as estruturas de segurança do kernel Windows NT:

- **Descritor de Segurança (Security Descriptor):** Recuperado via `GetNamedSecurityInfoW` com flags `OWNER_SECURITY_INFORMATION` e `DACL_SECURITY_INFORMATION`;
- **Gerenciamento RAII de Memória:** O ponteiro `PSECURITY_DESCRIPTOR` alocado pelo Windows é obrigatoriamente liberado via `LocalFree` em destrutores RAII;
- **Rastreamento de Herança:** Utiliza `GetInheritanceSourceW` para identificar o caminho exato do contêiner pai que originou cada entrada herdada;
- **Resolução de Identidades:** Mapeia SIDs binários para representações textuais (`ConvertSidToStringSidW`) e nomes amigáveis de domínio (`LookupAccountSidW`);
- **Modo Somente Leitura:** Todas as chamadas de API solicitam apenas o direito de acesso `READ_CONTROL`, impedindo qualquer alteração no sistema de arquivos.

## 9. Navegador de Permissões e Modelo Virtualizado

O navegador de permissões provê exploração visual fluida de volumes locais e caminhos de rede:

- **Carregamento sob Demanda (`fetchMore`):** Os nós de pastas e arquivos são expandidos de forma assíncrona, evitando travamentos de interface;
- **Exibição Hierárquica:** Cada item exibe nome, tipo, proprietário resolvido, status de herança (`SE_DACL_PROTECTED`) e pontuação de risco calculada;
- **Tabela Detalhada de ACEs:** Apresenta o tipo da regra (Allow/Deny), identidade, máscara de acesso, direitos concedidos e pasta de origem da herança.

## 10. Acesso Efetivo e Explicador Visual

O explicador visual responde de maneira formal e inteligível sobre a cadeia de permissões do usuário:

- **Expansão de Tokens:** Avalia o usuário selecionado, seus grupos primários, locais, de domínio e grupos universais (`Everyone`, `Authenticated Users`);
- **Precedência Canônica de ACEs:** Aplica a regra estrita de que regras Deny explícitas superam regras Allow, e regras explícitas superam herdadas;
- **Árvore de Causalidade:** Renderiza graficamente a linhagem completa: Identidade -> Associação de Grupo -> Pasta de Origem -> Direito Efetivo;
- **Parecer Técnico em Linguagem Natural:** Gera um texto explicativo em parágrafo estruturado pronto para anexação em laudos periciais.

## 11. Scanner Heurístico de Riscos e Conformidade

O módulo `RiskScanner` avalia o sistema contra regras de conformidade corporativa e segurança:

- **Pastas World-Writable:** Detecta diretórios de sistema ou dados graváveis por `Everyone`, `Users` ou `Authenticated Users`;
- **Binary Planting & DLL Hijacking:** Identifica arquivos executáveis (`.exe`, `.dll`, `.sys`, `.bat`, `.ps1`) situados em pastas graváveis por usuários comuns;
- **Controle Excessivo de Permissões:** Mapeia contas não administrativas com direitos de alteração de DACL (`WRITE_DAC`) ou apropriação (`WRITE_OWNER`);
- **Higiene de Contas Órfãs:** Localiza SIDs que não correspondem a nenhuma conta existente no Active Directory ou SAM local;
- **Herança Quebrada:** Sinaliza contêineres com herança bloqueada que isolam diretórios das políticas corporativas da raiz.

## 12. Auditoria de Serviços Windows e Compartilhamentos SMB

Módulo especializado em auditoria de vetores comuns de escalada de privilégios e exposição em rede:

- **Auditoria do Service Control Manager (SCM):** Conecta via `OpenSCManagerW` e analisa serviços registrados com `EnumServicesStatusExW`;
- **Detecção de Unquoted Service Paths:** Identifica serviços com espaços no caminho do executável sem aspas delimitadoras;
- **Binários de Serviço Graváveis:** Detecta arquivos executáveis de serviços que podem ser modificados por contas sem privilégios administrativos;
- **Auditoria de Compartilhamentos SMB:** Inspeciona permissões de rede via `NetShareGetInfo` e cruza os dados com a DACL da pasta local subjacente.

## 13. Persistência Local e Snapshots SQLite

O gerenciamento de dados históricos utiliza um banco de dados local SQLite3 integrado:

- **Localização Segura:** O arquivo é criado em `%LOCALAPPDATA%/ACLAuditor/ACLAuditor/acl_auditor_snapshots.db`;
- **Esquema Relacional:** Estrutura otimizada composta pelas tabelas `snapshots`, `snapshot_itens` e `snapshot_aces`;
- **Cadeia Forense de Custódia:** Cada snapshot recebe um hash de integridade **SHA-256** calculado via Windows CNG (`BCrypt`);
- **Mecanismo de Diff Temporal:** Compara dois snapshots e reporta novos acessos, permissões revogadas, elevações e trocas de propriedade.

## 14. Modo Consultoria e Geração de Relatórios

Projetado para prestadores de serviços de auditoria e equipes corporativas de conformidade:

- **HTML Interativo:** Dashboard autossuficiente e responsivo com tabelas filtráveis e gráficos dinâmicos de severidade;
- **PDF Executivo:** Documento diagramado em padrão A4 para impressão, com sumário executivo, matriz de riscos e espaço para assinatura;
- **CSV Estruturado:** Exportação tabular completa com cabeçalho e codificação UTF-8 BOM para abertura imediata no Microsoft Excel;
- **JSON Estruturado:** Carga de dados completa para automação de testes contínuos em esteiras DevSecOps e ingestão em SIEMs;
- **Identidade Visual:** Permite configurar nome da consultoria, cliente auditado, auditor responsável e anexar o logotipo corporativo.

## 15. Remediação Segura e Garantia de Rollback

O ACL Auditor estabelece um compromisso formal com a estabilidade do sistema auditado:

- **Operação Não-Destrutiva:** O sistema nunca executa correções diretamente na árvore de arquivos sem a confirmação expressa do operador;
- **Geração de Scripts Auditáveis:** O diálogo de remediação gera scripts em lote (`.bat` com `icacls`) e scripts PowerShell (`.ps1`);
- **Backup Prévio Obrigatório:** Todos os scripts de correção incluem o comando de salvamento prévio das DACLs originais via `icacls /save`;
- **Script de Rollback Imediato:** É gerado concomitantemente um script de reversão que executa `icacls /restore` para restaurar o estado original.

## 16. Integração com o Windows Explorer

Agilidade para a rotina diária de administração e resposta a incidentes:

- **Registro em Nível de Usuário:** O atalho é gravado em `HKCU\Software\Classes\Directory\shell\AclAuditor`, dispensando privilégios de administrador;
- **Comando de Acionamento:** Adiciona a opção *Auditar Permissões com ACL Auditor* no menu de contexto de pastas;
- **Parâmetro de Inicialização:** O Explorer repassa o caminho selecionado diretamente como argumento para abertura imediata no aplicativo.

## 17. Internacionalização e Localização

Arquitetura de internacionalização nativa com suporte bilíngue completo:

- **Gerenciador Central (`GerenciadorTraducao`):** Implementado como singleton para carregamento de catálogos compilados `.qm`;
- **Idiomas Suportados:** Suporte integral a Português do Brasil (`pt_BR`) e Inglês dos Estados Unidos (`en_US`);
- **Troca Dinâmica em Tempo de Execução:** O idioma pode ser alternado instantaneamente nos menus ou diálogos sem necessidade de reiniciar o executável;
- **Propagação de Eventos:** Emite o sinal `QEvent::LanguageChange` para que todos os widgets ativos repintem seus textos imediatamente.

## 18. Assets e Conteúdos Embarcados

A aplicação mantém acervo documental completo empacotado no diretório `source/assets/`:

- `ABOUT/`: Contém os arquivos `ABOUT_pt_BR.txt`, `ABOUT_en_US.txt`, `History_APP_pt_BR.txt` e `History_APP_en_US.txt`;
- `CLC/`: Contrato de Licenciamento Comercial em português e inglês (`CLC_pt_BR.txt`, `CLC_en_US.txt`);
- `COPYRIGHT/`: Aviso oficial de Direitos Autorais e Marca Registrada em ambos os idiomas;
- `EULA/`: Termos do Contrato de Licença de Usuário Final em conformidade com as normas da Microsoft Store e legislação brasileira;
- `LICENSES/`: Textos integrais de todas as licenças de código aberto e dependências de terceiros (incluindo LGPL v3 e SQLite Public Domain);
- `NOTICES/`: Atribuições legais de direitos autorais de componentes de terceiros em ambos os idiomas;
- `PRIVACY_POLICY/`: Políticas de privacidade garantindo arquitetura Local-First, ausência de telemetria e conformidade com LGPD/GDPR;
- `RELEASE/`: Notas de versão detalhadas das edições lançadas;
- `icons/`: Logotipo e ícones institucionais do ACL Auditor em 11 resoluções distintas (de 71x71 até 3840x2160).

## 19. Utilitários Auxiliares e Mocks

O subdiretório `mocks/` fornece ferramentas especializadas para o ciclo de desenvolvimento e manutenção:

- **`CompileTranslationsGUI`:** Interface visual intuitiva para compilação e teste imediato de arquivos de tradução `.ts` para binários `.qm`;
- **`CompileTranslationsTerminal`:** Versão em linha de comando para compilação automatizada de traduções em scripts e rotinas de CI/CD;
- **`find_VersionEditor`:** Utilitário com interface gráfica e CLI para validação e sincronização atômica de versão e data em 19 arquivos monitorados (código C++, documentação, licenças, contratos e instaladores Inno Setup);
- **Isolamento de Build:** As ferramentas auxiliares possuem diretórios de compilação próprios (`build_Mocks_mingw` e `build_Mocks_msvc`).

## 20. Fluxo Principal de Execução Observado

Sequência cronológica da operação típica do ACL Auditor:

1. **Inicialização e Diagnóstico:** Inicialização da aplicação, detecção do idioma do sistema e aplicação do tema visual Windows 11;
2. **Seleção de Alvo:** Entrada de caminho absoluto ou seleção de pasta via menu de contexto do Windows Explorer;
3. **Varredura e Virtualização:** Carregamento assíncrono dos nós do sistema de arquivos e população do modelo `AclTreeModel`;
4. **Inspeção de DACL:** Seleção de nó na árvore e exibição tabular detalhada de todas as regras de permissão no `AceTableModel`;
5. **Explicabilidade:** Cálculo de acesso efetivo simulando o token do usuário e renderização da árvore causal de herança;
6. **Análise de Riscos:** Execução do scanner heurístico e consolidação das vulnerabilidades encontradas no painel de riscos;
7. **Auditoria de Serviços e Rede:** Varredura de executáveis do SCM e cruzamento com as permissões dos compartilhamentos SMB;
8. **Custódia Forense:** Gravação do snapshot no banco SQLite com cálculo de assinatura SHA-256;
9. **Entrega de Resultados:** Exportação do laudo técnico em HTML, PDF, CSV ou JSON e geração de scripts de mitigação com rollback.

## 21. Pontos Técnicos Importantes para Manutenção

Recomendações técnicas para a evolução e sustentabilidade do código-fonte:

- **Elevação de Privilégios:** Ao inspecionar contêineres restritos do Windows, certifique-se de executar com o token `SeSecurityPrivilege` habilitado;
- **Liberação de Handles Nativos:** Descritores de segurança retornados por APIs Win32 devem ser sempre gerenciados com padrões RAII ou `LocalFree`;
- **Isolamento de Presets:** Mantenha a separação rígida entre as builds da aplicação principal e as builds das ferramentas auxiliares em `mocks/`;
- **Auditoria de Versão:** Antes de cada liberação de versão, execute o utilitário `find_VersionEditor --check .` para garantir conformidade em todos os 19 arquivos.

## 22. Conclusão

O **ACL Auditor** consolida-se como uma solução de alta performance em C++20, pronta para operação em cenários corporativos exigentes. Sua arquitetura sem dependências em nuvem, aliada à explicabilidade visual de permissões e à remediação segura com garantia de rollback, estabelece um novo padrão em governança e auditoria de sistemas de arquivos Windows.

</details>

---

<h2 id="enus">🇺🇸 English (US)</h2>

<details>
<summary>Click to expand / collapse the Technical Observation in English</summary>

# ACL AUDITOR - TECHNICAL OBSERVATION AND STRATEGIC POSITIONING

Documented version: `2026.11.6.0`  
Technical snapshot date: `November 6, 2026`  
Repository: `ACL_Auditor_cpp`  
Identified author in the project: `Fernando Nillsson Cidade`  
Repository license: `GNU AGPLv3 (GNU Affero General Public License v3)`  

## 1. Current Project State and Positioning

The current repository of **ACL Auditor** consolidates an enterprise desktop platform **native in C++20 with Qt 6.11.1** compiled via CMake and Ninja. The product transcends the narrow category of simple permission list viewers or legacy command-line utilities (such as `icacls` and `accesschk`), positioning itself as a **Local NTFS Access Compliance, Explanation & Audit Platform**.

Its core value proposition directly answers the most critical governance question across Windows enterprise infrastructures:
> **“Why does this specific user have permission to read, modify, or execute this file?”**
> *(“Por que este usuário específico consegue ler, alterar ou executar este arquivo?”)*

The product operates as a sovereign **100% Local-First** solution (no cloud, no telemetry, no accounts, and no external service dependencies), functioning strictly in **read-only mode** throughout all inspection and diagnostic routines.

## 2. Technical Line and Highlights of Version 2026.11.6.0

This release consolidates the technical maturity of the engine and the full stabilization of enterprise audit subsystems:

- **Native C++20 Win32 engine (`ACLAuditorEngine`):** Low-level inspection via `Advapi32`, `Authz`, `Netapi32`, and `BCrypt`, with strict resolution of security descriptors, domain identities, and universal groups;
- **Visual effective access explainer (`VisualExplanationWidget`):** Token-based evaluation simulating the Security Reference Monitor (SRM), featuring a causal lineage tree and natural language technical reports;
- **Heuristic compliance scanner (`RiskScanner`):** Deterministic risk classification (World-Writable, DLL Hijacking / Binary Planting, excessive privileges `WRITE_DAC`/`WRITE_OWNER`, orphaned accounts, and broken inheritance);
- **Services and network shares audit (`ServicesSharesWidget`):** Detection of *Unquoted Service Paths*, writable service binaries, and cross-matrix discrepancies between SMB share permissions and local NTFS DACLs;
- **Local persistence and temporal diff (`SnapshotEngine`):** Embedded local SQLite3 database with forensic custody **SHA-256** signatures and automated detection of compliance drift;
- **Consulting mode and executive reports (`ReportExportDialog`):** Enterprise-grade export ready for client delivery in Interactive HTML, Executive A4 PDF, Structured CSV (UTF-8 BOM), and JSON for DevSecOps/SIEM;
- **Safe and non-destructive remediation (`RemediationDialog`):** Generates PowerShell and `icacls` scripts with mandatory pre-backup (`/save`) and guaranteed immediate rollback (`/restore`);
- **Windows Explorer shell integration (`ExplorerIntegration`):** Shell context menu activation registered in `HKCU`, requiring zero administrative elevation for installation.

## 3. Consolidated Structural Summary

Directly validated file metrics across the active `source/` codebase tree:

| Layer | `.cpp` | `.hpp` | Technical Scope |
|---|---:|---:|---|
| `source/engine` | 7 | 8 | Core Win32 security engine, effective access, risk scanner, services, SMB, SQLite, and remediation |
| `source/model` | 4 | 4 | Virtual `QAbstractItemModel` implementations for NTFS tree, ACE table, risk findings, and snapshot diffs |
| `source/ui` | 11 | 11 | Main application window, manuals, about dialog, reporting, remediation, and visual graph widgets |
| `source/utils` | 3 | 3 | Windows Explorer integration, application path resolution, and centralized logging subsystem |
| `source/language` | 1 | 1 | Central internationalization manager orchestrating dynamic runtime language toggling |
| `source/cli` | 1 | 0 | Standalone Headless Command Line Interface for automated CI/CD and DevOps (`source/cli/main_cli.cpp`) |
| `source/assets` | 0 | 0 | Embedded documentation (ABOUT, EULA, LICENSES, NOTICES, PRIVACY_POLICY, RELEASE, and app icons) |
| Root of `source/` | 1 | 0 | Secondary entry point and application initialization bootstrap (`source/main.cpp`) |
| **Total in `source/`** | **28** | **27** | **Main application & CLI C++20 codebase (55 total files)** |

Additional source files located outside `source/`:
- `main.cpp`: Primary entry point delegating execution to the application initialization bootstrap;
- `mocks/teste.cpp`: Automated regression and validation test suite with 7 comprehensive engine phases (located in `mocks/`);
- `mocks/`: 4 auxiliary developer tools/targets comprising 4 `.cpp` and 3 `.hpp` files (Repository grand total: 33 `.cpp` and 30 `.hpp`);
- `docs/`: 7 modular specifications covering architecture, security model, compliance mappings, CLI reference, licensing, and user guides;
- `repo/`: Inno Setup packaging scripts, MSIX AppxManifest with `AppExecutionAlias`, and technical compilation guides.

## 4. Build, Toolchain, and Actual Dependencies

Validated specifications from `CMakeLists.txt` and `CMakePresets.json` files:

- CMake version requirement: `cmake_minimum_required(VERSION 3.25)`;
- Language standard specification: `C++20` (enforced via `set(CMAKE_CXX_STANDARD 20)` and `CMAKE_CXX_STANDARD_REQUIRED ON`);
- Qt compilation automation: `CMAKE_AUTOMOC ON` enabled globally across targets;
- Optimized Release compiler flags: `/O2 /Oi /Ot /fp:fast` on MSVC and `-O3 -ffunction-sections -fdata-sections` on MinGW;
- Runtime library linking: Multithreaded dynamic runtime configuration (`MultiThreadedDLL` on MSVC).

Required Qt 6.11.1 framework modules:
- `Qt6::Core`: Fundamental runtime mechanisms, primitive types, event loop, and file I/O;
- `Qt6::Gui`: Desktop window management, hardware-accelerated rendering, and DWM integration;
- `Qt6::Widgets`: Enterprise user interface components and desktop control layouts;
- `Qt6::Sql`: Local database persistence subsystem utilizing the embedded `QSQLITE` driver;
- `Qt6::Network`: Network identifier resolution and local inter-process transport support;
- `Qt6::Pdf`: Vectorized document rendering and high-fidelity reporting export support;
- `Qt6::Svg`: Crisp high-DPI rendering for scalable application iconography.

Linked native Microsoft Windows system libraries:
- `Advapi32`: Security Descriptor management, DACLs, SACLs, tokens, and SIDs (`GetNamedSecurityInfoW`, `LookupAccountSidW`);
- `Authz`: Authorization client context simulation and token-based evaluation;
- `Netapi32`: Network Management API for SMB share inspection via `NetShareGetInfo`;
- `BCrypt`: Cryptographic SHA-256 evidence hashing utilizing the native Windows CNG API;
- `Dwmapi`: Windows Desktop Window Manager integration (Mica backdrop effect and immersive dark mode on Windows 11);
- `User32`: Windows desktop environment interrogation and native window attribute control.

## 5. Auxiliary Directories Outside `source/`

Beyond the core application code in `source/`, the repository hosts modular maintenance and deployment assets:

- `mocks/`: Hosts auxiliary developer tools `find_VersionEditor`, `CompileTranslationsGUI`, and `CompileTranslationsTerminal`, with dedicated CMake configuration;
- `repo/`: Inno Setup deployment packaging scripts (`ACL_Auditor.iss`, `ACL_Auditor_MinGW.iss`, `ACL_Auditor_MSVC.iss`) and technical environment guides;
- `cmake/`: Deployment modules (`deploy_optional_qt_binary.cmake`), Windows application resource template (`windows_app_icon.rc.in`), and scripts;
- `.vscode/`: Workspace settings, automated build/test tasks (`tasks.json`), and IntelliSense compiler definitions (`c_cpp_properties.json`);
- `build/`: Isolated build artifact output folders (`build_ACLAuditor_mingw`, `build_ACLAuditor_msvc`, `build_Mocks_mingw`, `build_Mocks_msvc`).

## 6. Entry Point and Bootstrap

Audited execution bootstrap sequence in application code:

1. `main.cpp`:
   - Canonical executable entry point immediately delegating execution to `AclAuditor::runApplication(argc, argv)`.

2. `source/main.cpp`:
   - Configures High-DPI execution scaling behavior (`Qt::HighDpiExecutionPolicy::PassThrough`);
   - Instantiates `QApplication` configuring application identity (`ACLAuditor`), organization, and official release version;
   - Initializes centralized logging via `LogManager` directing diagnostics to local rotating files and console outputs;
   - Initializes `GerenciadorTraducao`, interrogating host operating system locale and binding the matching compiled `.qm` catalog;
   - Evaluates command-line interface arguments (`--scan`, `--snapshot`, `--report`, `--version`, `--help`);
   - Instantiates `MainWindow`, applies DWM Mica / Immersive Dark Mode visual styling, and displays GUI to the user.

## 7. Current Layered Architecture

The application architecture strictly segregates core technical responsibilities across isolated layers:

- `source/engine`: Encapsulates low-level Win32 Security APIs, mathematical effective access computation, risk heuristics, and SQLite storage;
- `source/model`: Supplies virtualized `AclTreeModel`, `AceTableModel`, `RiskTableModel`, and `DiffTableModel` bridging engine data to GUI views;
- `source/ui`: Hosts desktop user interface screens, metric cards, services/SMB inspection views, and modal dialogs;
- `source/utils`: Encapsulates rotating diagnostic logging, application path resolution, and Windows Explorer shell integration;
- `source/language`: Implements centralized internationalization services enabling dynamic real-time language switching without restart;
- `source/assets`: Houses embedded legal terms, user privacy documentation, third-party notices, and complete multi-resolution icons.

## 8. Functional Architecture and Win32 Engine

The core auditing engine operates directly against native Windows NT kernel security objects:

- **Security Descriptor Architecture:** Retrieved via `GetNamedSecurityInfoW` using `OWNER_SECURITY_INFORMATION` and `DACL_SECURITY_INFORMATION` flags;
- **RAII Memory Management:** Security descriptor memory buffers allocated by Windows are safely deallocated via `LocalFree` inside RAII wrappers;
- **Inheritance Propagation Tracking:** Invokes `GetInheritanceSourceW` to identify the ancestor container directory where inherited ACEs originated;
- **Identity and SID Resolution:** Translates binary SIDs into standard SDDL strings (`ConvertSidToStringSidW`) and friendly domain names (`LookupAccountSidW`);
- **Strict Read-Only Access:** All Win32 API calls request solely `READ_CONTROL` query rights, guaranteeing that target file systems remain unmodified.

## 9. Permissions Explorer and Virtualized Model

The permissions explorer provides smooth visual navigation across extensive local storage volumes and remote network shares:

- **On-Demand Loading (`fetchMore`):** Directory and file nodes are resolved and populated asynchronously, preventing GUI thread freezing;
- **Hierarchical Tree Inspection:** Every node displays object name, type, resolved owner identity, inheritance flag (`SE_DACL_PROTECTED`), and risk score;
- **Detailed ACE Table Inspection:** Displays rule type (Allow/Deny), resolved identity, access mask, granted privileges, and inheritance source.

## 10. Effective Access and Visual Explainer

The visual explanation engine produces mathematically verifiable, natural-language verdicts on user access authority:

- **Security Token Expansion:** Evaluates target user identity, primary group, local security groups, domain groups, and universal SIDs (`Everyone`, `Authenticated Users`);
- **Canonical ACE Precedence:** Adheres strictly to canonical ordering rules where explicit Deny rules override Allow, and explicit rules override inherited ones;
- **Causal Lineage Graph:** Renders the complete authorization ancestry: Target Identity -> Group Membership -> Originating Folder -> Effective Right;
- **Natural Language Technical Report:** Synthesizes structured technical paragraphs suitable for inclusion in forensic audit documentation.

## 11. Heuristic Risk Scanner and Compliance

The `RiskScanner` subsystem validates directory permissions against corporate baseline security benchmarks:

- **World-Writable Directories:** Flags system, software, or corporate data directories granting write rights to `Everyone`, `Users`, or `Authenticated Users`;
- **Binary Planting & DLL Hijacking:** Identifies executable binaries (`.exe`, `.dll`, `.sys`, `.bat`, `.ps1`) located inside user-writable directories;
- **Excessive Permission Control:** Detects non-administrative user accounts granted DACL modification (`WRITE_DAC`) or ownership (`WRITE_OWNER`) rights;
- **Orphaned Account Hygiene:** Locates unresolved SIDs remaining from accounts previously deleted from Active Directory or local SAM databases;
- **Broken Inheritance Silos:** Highlights containers configured with `SE_DACL_PROTECTED` isolating child directories from root corporate governance policies.

## 12. Windows Services and SMB Shares Audit

Dedicated inspection subsystem assessing high-risk privilege escalation and lateral network vectors:

- **Service Control Manager (SCM) Audit:** Connects via `OpenSCManagerW` and queries all active services via `EnumServicesStatusExW`;
- **Unquoted Service Path Detection:** Identifies services with executable paths containing spaces and lacking surrounding quotation marks;
- **Writable Service Binaries:** Identifies service binary executables that can be modified by non-privileged standard user accounts;
- **SMB Network Shares Audit:** Queries share configurations via `NetShareGetInfo` and correlates findings against underlying local NTFS folder DACLs.

## 13. Local Persistence and SQLite Snapshots

Historical baseline compliance tracking is managed by an integrated local SQLite3 repository:

- **Secure Storage Location:** The database file is initialized at `%LOCALAPPDATA%/ACLAuditor/ACLAuditor/acl_auditor_snapshots.db`;
- **Relational Schema:** Optimized normalized database structure composed of `snapshots`, `snapshot_itens`, and `snapshot_aces` tables;
- **Forensic Custody Integrity:** Every captured snapshot is stamped with a cryptographic **SHA-256** hash calculated via Windows CNG (`BCrypt`);
- **Temporal Diff Engine:** Compares arbitrary snapshot pairs, isolating newly granted accesses, revoked rights, privilege escalations, and owner changes.

## 14. Consulting Mode and Report Generation

Engineered for cybersecurity consultants, Managed Service Providers (MSP), and corporate compliance auditors:

- **Interactive HTML Dashboard:** Standalone, responsive web report featuring filterable risk tables and dynamic severity breakdown charts;
- **Executive PDF Document:** Print-ready A4 document including executive overview, complete risk matrices, and formal sign-off sections;
- **Structured Tabular CSV:** Complete dataset export formatted with UTF-8 BOM encoding for direct ingestion into Microsoft Excel and analytics tools;
- **Machine-Readable JSON:** Full structured data payload for automated continuous compliance pipelines in DevSecOps and SIEM ingestion;
- **Consultancy Custom Branding:** Supports customizing consultancy name, audited client entity, lead auditor, and attaching corporate logo images.

## 15. Safe Remediation and Rollback Guarantee

ACL Auditor maintains an unwavering engineering commitment to the operational stability of target hosts:

- **Non-Destructive Operations:** The application never alters file permissions or system configurations without explicit human confirmation;
- **Auditable Script Generation:** The remediation planner generates transparent Windows Command Prompt (`.bat` with `icacls`) and PowerShell (`.ps1`) scripts;
- **Mandatory Pre-Change Backup:** All mitigation scripts include automated pre-change DACL state backups using `icacls /save`;
- **Guaranteed Instant Rollback:** Automatically generates an accompanying rollback script executing `icacls /restore` to revert permissions immediately.

## 16. Windows Explorer Shell Integration

Accelerating system administrator daily workflows and rapid incident triage:

- **Per-User Registry Registration:** The integration key is recorded in `HKCU\Software\Classes\Directory\shell\AclAuditor`, requiring zero admin elevation;
- **Context Menu Command:** Adds a convenient *Audit Permissions with ACL Auditor* option to Windows Explorer folder context menus;
- **Automated Startup Parameter:** Windows Explorer automatically forwards the selected folder path as a command-line argument for instant scanning.

## 17. Internationalization and Localization

Native internationalization framework providing complete, seamless bilingual capabilities:

- **Central Translation Manager (`GerenciadorTraducao`):** Implemented as a thread-safe singleton loading precompiled binary `.qm` catalogs;
- **Supported Locales:** Complete native translations for Brazilian Portuguese (`pt_BR`) and American English (`en_US`);
- **Dynamic Runtime Switching:** The application language can be toggled on-the-fly from top menus or dialogs without restarting the process;
- **Event Propagation:** Emits `QEvent::LanguageChange` across the widget hierarchy, instructing all active components to refresh their display strings.

## 18. Embedded Assets and Documentation

The repository packages a comprehensive set of legal and operational documentation under `source/assets/`:

- `ABOUT/`: Contains `ABOUT_pt_BR.txt`, `ABOUT_en_US.txt`, `History_APP_pt_BR.txt`, and `History_APP_en_US.txt` files;
- `CLC/`: Commercial Licensing Agreement terms in both Portuguese and English (`CLC_pt_BR.txt`, `CLC_en_US.txt`);
- `COPYRIGHT/`: Official Copyright and Trademark Notices in both supported languages;
- `EULA/`: End User License Agreement terms strictly compliant with Microsoft Store standards and local regulations;
- `LICENSES/`: Complete verbatim license texts for all open-source libraries and third-party dependencies (including LGPL v3 and SQLite Public Domain);
- `NOTICES/`: Formal third-party legal attributions and copyright notices in both supported languages;
- `PRIVACY_POLICY/`: Privacy policies ensuring a Local-First architecture, complete absence of telemetry, and full GDPR/LGPD compliance;
- `RELEASE/`: Exhaustive release notes and architectural changelogs across published project versions;
- `icons/`: Comprehensive official branding and application icons across 11 distinct pixel resolutions (from 71x71 up to 3840x2160).

## 19. Auxiliary Utilities and Mocks

The `mocks/` subsystem houses dedicated tooling for the development and maintenance lifecycle:

- **`CompileTranslationsGUI`:** Intuitive graphical user interface for real-time compilation and validation of `.ts` files into binary `.qm` catalogs;
- **`CompileTranslationsTerminal`:** Headless command-line utility for automated translation catalog compilation inside scripts and CI/CD pipelines;
- **`find_VersionEditor`:** GUI and CLI utility for automated auditing, validation, and atomic synchronization of release versions across 19 tracked files (C++ code, documentation, licenses, contracts, and Inno Setup installers);
- **Isolated Build Pipelines:** Auxiliary tools utilize their own dedicated build environments (`build_Mocks_mingw` and `build_Mocks_msvc`).

## 20. Main Observed Execution Flow

Chronological workflow of typical ACL Auditor operational engagements:

1. **Bootstrap and Diagnostics:** Application startup, system language detection, log file initialization, and Windows 11 visual styling application;
2. **Target Path Selection:** Operator inputs target path directly or initiates scan via Windows Explorer context menu;
3. **Scanning and Virtualization:** Asynchronous traversal of file system hierarchy, creating and populating the virtual `AclTreeModel`;
4. **DACL Inspection:** Selection of nodes in the virtual tree, populating detailed tabular ACE representations inside `AceTableModel`;
5. **Access Explainability:** Calculation of effective access simulating user token groups and rendering the visual causal authorization lineage;
6. **Risk Analysis:** Execution of the heuristic compliance scanner and consolidation of identified vulnerabilities onto the risk dashboard;
7. **Services and Network Audit:** Enumeration of SCM services, unquoted path analysis, and correlation with active SMB network shares;
8. **Forensic Evidence Custody:** Persistence of security snapshot inside local SQLite database stamped with SHA-256 cryptographic signature;
9. **Deliverable Generation:** Export of formal audit deliverables in HTML, PDF, CSV, or JSON, alongside mitigation scripts and rollback plans.

## 21. Important Technical Points for Maintenance

Core engineering guidelines ensuring codebase sustainability and reliable operation:

- **Privilege Elevation:** When inspecting restricted operating system directories, launch the application with `SeSecurityPrivilege` enabled;
- **Native Handle Lifecycle:** Security descriptors and Win32 pointers must strictly be reclaimed via RAII wrappers or `LocalFree`;
- **Build Preset Isolation:** Preserve the architectural separation between primary application build trees and auxiliary mock targets in `mocks/`;
- **Version Synchronization:** Before tagging new releases, execute `find_VersionEditor --check .` to guarantee 100% synchronization across all 19 files.

## 22. Conclusion

**ACL Auditor** stands as a robust, high-performance C++20 enterprise platform engineered for demanding corporate environments. Its zero-cloud architecture, coupled with visual permission explainability and non-destructive remediation with guaranteed rollback, establishes a premier standard for Windows file system governance and compliance auditing.

</details>

---

<h2 id="tree">🌳 Estrutura da Árvore de Diretórios e Arquivos / Directory and File Tree Structure</h2>

<details>
<summary>Clique para expandir / recolher a Árvore de Arquivos / Click to expand / collapse File Tree</summary>

```
```
.gitattributes
.gitignore
CMakeLists.txt
CMakePresets.json
CODE_OF_CONDUCT.md
CONTRIBUTING.md
LICENSE
main.cpp
MANUAL.md
OBSERVACAO.md
rascunho.cpp
README.md
SECURITY.md
```

**Root Path:** `./source`

```
├── 📁 assets
│   ├── 📁 ABOUT
│   │   ├── 📄 ABOUT_en_US.txt
│   │   ├── 📄 ABOUT_pt_BR.txt
│   │   ├── 📄 History_APP_en_US.txt
│   │   └── 📄 History_APP_pt_BR.txt
│   ├── 📁 CLC
│   │   ├── 📄 CLC_en_US.txt
│   │   └── 📄 CLC_pt_BR.txt
│   ├── 📁 COPYRIGHT
│   │   ├── 📄 AVISO DE COPYRIGHT E MARCA REGISTRADA_pt_BR.txt
│   │   └── 📄 COPYRIGHT AND TRADEMARK NOTICE_en_US.txt
│   ├── 📁 EULA
│   │   ├── 📄 EULA_en_US.txt
│   │   └── 📄 EULA_pt_BR.txt
│   ├── 📁 LICENSES
│   │   ├── 📄 Apache License Version 2.0.txt
│   │   ├── 📄 BSD License Clause-2.txt
│   │   ├── 📄 BSD License Clause-3.txt
│   │   ├── 📄 GNU AFFERO GENERAL PUBLIC LICENSE Version3.txt
│   │   ├── 📄 GNU GENERAL PUBLIC LICENSE Version 3.txt
│   │   ├── 📄 GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.txt
│   │   ├── 📄 GNU LESSER GENERAL PUBLIC LICENSE Version 3.txt
│   │   ├── 📄 ISC LICENSE.txt
│   │   ├── 📄 MIT License.txt
│   │   ├── 📄 PYTHON SOFTWARE FOUNDATION LICENSE.txt
│   │   └── 📄 SQLite Is Public Domain.txt
│   ├── 📁 NOTICES
│   │   ├── 📄 NOTICE_en_US.txt
│   │   └── 📄 NOTICE_pt_BR.txt
│   ├── 📁 PRIVACY_POLICY
│   │   ├── 📄 Privacy_Policy_en_US.txt
│   │   └── 📄 Privacy_Policy_pt_BR.txt
│   ├── 📁 RELEASE
│   │   ├── 📄 RELEASE NOTES_en_US.txt
│   │   └── 📄 RELEASE NOTES_pt_BR.txt
│   └── 📁 icons
│       ├── 📄 acl_auditor.ico
│       ├── 🖼️ acl_auditor.png
│       ├── 🖼️ acl_auditor_1080-1080.png
│       ├── 🖼️ acl_auditor_1440-2160.png
│       ├── 🖼️ acl_auditor_150-150.png
│       ├── 🖼️ acl_auditor_1920-1080.png
│       ├── 🖼️ acl_auditor_2160-2160.png
│       ├── 🖼️ acl_auditor_300-300.png
│       ├── 🖼️ acl_auditor_3840-2160.png
│       ├── 🖼️ acl_auditor_584-800.png
│       └── 🖼️ acl_auditor_71-71.png
├── 📁 cli
│   └── ⚡ main_cli.cpp
├── 📁 engine
│   ├── ⚡ AclTypes.hpp
│   ├── ⚡ EffectiveAccessCalculator.cpp
│   ├── ⚡ EffectiveAccessCalculator.hpp
│   ├── ⚡ NtfsSecurityUtils.cpp
│   ├── ⚡ NtfsSecurityUtils.hpp
│   ├── ⚡ RemediationPlanner.cpp
│   ├── ⚡ RemediationPlanner.hpp
│   ├── ⚡ RiskScanner.cpp
│   ├── ⚡ RiskScanner.hpp
│   ├── ⚡ ServiceAuditor.cpp
│   ├── ⚡ ServiceAuditor.hpp
│   ├── ⚡ SmbShareAuditor.cpp
│   ├── ⚡ SmbShareAuditor.hpp
│   ├── ⚡ SnapshotEngine.cpp
│   └── ⚡ SnapshotEngine.hpp
├── 📁 language
│   ├── 📁 translations
│   │   ├── 📄 aclauditor_en_US.qm
│   │   ├── 📄 aclauditor_en_US.ts
│   │   ├── 📄 aclauditor_pt_BR.qm
│   │   └── 📄 aclauditor_pt_BR.ts
│   ├── ⚡ GerenciadorTraducao.cpp
│   └── ⚡ GerenciadorTraducao.hpp
├── 📁 model
│   ├── ⚡ AceTableModel.cpp
│   ├── ⚡ AceTableModel.hpp
│   ├── ⚡ AclTreeModel.cpp
│   ├── ⚡ AclTreeModel.hpp
│   ├── ⚡ DiffTableModel.cpp
│   ├── ⚡ DiffTableModel.hpp
│   ├── ⚡ RiskTableModel.cpp
│   └── ⚡ RiskTableModel.hpp
├── 📁 ui
│   ├── ⚡ AboutDialog.cpp
│   ├── ⚡ AboutDialog.hpp
│   ├── ⚡ MainWindow.cpp
│   ├── ⚡ MainWindow.hpp
│   ├── ⚡ ManualDialog.cpp
│   ├── ⚡ ManualDialog.hpp
│   ├── ⚡ RemediationDialog.cpp
│   ├── ⚡ RemediationDialog.hpp
│   ├── ⚡ ReportExportDialog.cpp
│   ├── ⚡ ReportExportDialog.hpp
│   ├── ⚡ RiskDashboardWidget.cpp
│   ├── ⚡ RiskDashboardWidget.hpp
│   ├── ⚡ ServicesSharesWidget.cpp
│   ├── ⚡ ServicesSharesWidget.hpp
│   ├── ⚡ SnapshotDiffWidget.cpp
│   ├── ⚡ SnapshotDiffWidget.hpp
│   ├── ⚡ TableEnhancer.cpp
│   ├── ⚡ TableEnhancer.hpp
│   ├── ⚡ ThemeManager.cpp
│   ├── ⚡ ThemeManager.hpp
│   ├── ⚡ VisualExplanationWidget.cpp
│   ├── ⚡ VisualExplanationWidget.hpp
│   ├── ⚡ Win32DwmHelper.cpp
│   └── ⚡ Win32DwmHelper.hpp
├── 📁 utils
│   ├── ⚡ ApplicationPathUtils.cpp
│   ├── ⚡ ApplicationPathUtils.hpp
│   ├── ⚡ ExplorerIntegration.cpp
│   ├── ⚡ ExplorerIntegration.hpp
│   ├── ⚡ LogManager.cpp
│   └── ⚡ LogManager.hpp
└── ⚡ main.cpp
```

**Root Path:** `./.github`

```
├── 📁 ISSUE_TEMPLATE
│   ├── 📝 bug_report.md
│   ├── ⚙️ config.yml
│   ├── 📝 feature_request.md
│   └── 📝 question.md
├── 📁 workflows
│   └── ⚙️ update-readme-version.yml
└── 📝 PULL_REQUEST_TEMPLATE.md
```

**Root Path:** `./mocks`

```
├── 📁 icons
│   ├── 📄 acl_auditor.ico
│   ├── 🖼️ acl_auditor.png
│   ├── 🖼️ acl_auditor_1080-1080.png
│   ├── 🖼️ acl_auditor_1440-2160.png
│   ├── 🖼️ acl_auditor_150-150.png
│   ├── 🖼️ acl_auditor_1920-1080.png
│   ├── 🖼️ acl_auditor_2160-2160.png
│   ├── 🖼️ acl_auditor_300-300.png
│   ├── 🖼️ acl_auditor_3840-2160.png
│   ├── 🖼️ acl_auditor_584-800.png
│   └── 🖼️ acl_auditor_71-71.png
├── 📄 CMakeLists.txt
├── ⚡ CompileTranslationsGUI.cpp
├── ⚡ CompileTranslationsGUI.hpp
├── ⚡ CompileTranslationsTerminal.cpp
├── ⚡ CompileTranslationsTerminal.hpp
├── ⚡ find_VersionEditor.cpp
├── ⚡ find_VersionEditor.hpp
└── ⚡ teste.cpp
```

**Root Path:** `./docs`

```
├── 📁 images
│   ├── 🖼️ banner.png
│   └── 🖼️ logo.png
├── 📝 architecture.md
├── 📝 cli-reference.md
├── 📝 compliance.md
├── 📝 licensing.md
├── 📝 release-v2026.11.6.0.md
├── 📝 security-model.md
├── 📝 suggested-issues.md
└── 📝 user-guide.md
```

**Root Path:** `./repo/LISTAGEM`

```
├── 📁 en_US
│   ├── 🖼️ Captura de tela 2026-10-16 210219.png
│   ├── 🖼️ Captura de tela 2026-10-16 210237.png
│   ├── 🖼️ Captura de tela 2026-10-16 210255.png
│   ├── 🖼️ Captura de tela 2026-10-16 210318.png
│   ├── 🖼️ Captura de tela 2026-10-16 210348.png
│   └── 🖼️ Captura de tela 2026-10-16 210501.png
└── 📁 pt_BR
    ├── 🖼️ Captura de tela 2026-10-16 210931.png
    ├── 🖼️ Captura de tela 2026-10-16 210943.png
    ├── 🖼️ Captura de tela 2026-10-16 210954.png
    ├── 🖼️ Captura de tela 2026-10-16 211005.png
    ├── 🖼️ Captura de tela 2026-10-16 211019.png
    └── 🖼️ Captura de tela 2026-10-16 211032.png
```

```
</details>
