# Guia de Contribuição / Contributing Guide — ACL Auditor

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher as instruções em português</summary><br>

# Contribuindo com o ACL Auditor

Antes de tudo, agradecemos seu interesse em contribuir com o **ACL Auditor**! Seu apoio torna a auditoria de segurança do Windows mais transparente, robusta e acessível para todos.

> [!NOTE]
> **Sobre o Desenvolvimento, Suporte e Contribuições Externas:**  
> Este projeto é concebido, desenvolvido e mantido primariamente como uma iniciativa pessoal e educacional pelo autor, que é iniciante e está em processo contínuo de aprendizado em programação.  
> O envio de sugestões, abertura de *Issues* e submissão de *Pull Requests (PRs)* são sempre bem-vindos! No entanto, ressalta-se que a análise de código, interação em tópicos técnicos e incorporação de contribuições são realizadas no próprio ritmo do autor e de forma estritamente voluntária. Caso um chamado ou PR demore para ser respondido ou não venha a ser incorporado à branch principal, por favor não reaja com alarde ou frustração. Conforme estabelecido pela licença GNU AGPLv3, você possui total liberdade para criar um *Fork* independente deste repositório e evoluir suas próprias ideias e versões.

---

## 1. Código de Conduta

Ao participar deste projeto, você concorda em cumprir nosso [Código de Conduta](CODE_OF_CONDUCT.md). Trate todos os colaboradores com respeito, cortesia e profissionalismo.

---

## 2. Formas de Contribuir

Você pode colaborar com o ACL Auditor de várias maneiras:
- **Relatar Bugs:** Submeta relatórios de bugs claros e reproduzíveis.
- **Sugerir Funcionalidades:** Proponha novas ideias, checagens de conformidade ou melhorias de fluxo de trabalho.
- **Melhorar a Documentação:** Esclareça instruções de compilação, documente cenários do Active Directory ou corrija pontuações.
- **Enviar Pull Requests:** Implemente correções, novas heurísticas de risco, melhorias no CLI ou traduções.
- **Traduções (i18n):** Ajude a localizar o ACL Auditor para idiomas adicionais (como Alemão, Espanhol ou Francês).

---

## 3. Configuração do Ambiente de Desenvolvimento

### 3.1. Pré-requisitos
- **Sistema Operacional:** Windows 10 / 11 x64 ou Windows Server 2022+
- **Compilador:**
  - Microsoft Visual C++ (MSVC) do Visual Studio 2022 (toolset v143) ou Build Tools 2022
  - Ou MinGW GCC 13+ (x86_64-w64-mingw32)
- **CMake:** Versão 3.21 ou superior
- **Qt:** Qt 6.x (Core, Gui, Widgets, Sql, PrintSupport)
- **Ferramenta de Build:** Ninja (recomendado)

### 3.2. Compilando com Presets do CMake
Disponibilizamos presets pré-configurados no arquivo `CMakePresets.json`:

```powershell
# Clonar o repositório
git clone https://github.com/fernandoncidade/ACL_Auditor_cpp.git
cd ACL_Auditor_cpp

# Configurar e compilar com MSVC (Visual Studio 2022)
cmake --preset host-qt6-msvc-release
cmake --build --preset host-qt6-msvc-release

# Executar a suíte de testes automatizados
ctest --preset host-qt6-msvc-release --output-on-failure
```

Ou com MinGW:

```powershell
cmake --preset host-qt6-mingw-release
cmake --build --preset host-qt6-mingw-release
ctest --preset host-qt6-mingw-release --output-on-failure
```

### 3.3. Compilação Manual (sem Presets)
Caso o Qt esteja instalado em um diretório personalizado, aponte a variável `CMAKE_PREFIX_PATH`:

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/msvc2022_64"
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

---

## 4. Padrões de Código e Arquitetura

- **Padrão da Linguagem:** C++20 (`set(CMAKE_CXX_STANDARD 20)`). Utilize recursos modernos de C++ (RAII, smart pointers, structured binding, concepts onde vantajoso).
- **Integridade da Arquitetura:**
  - O núcleo de segurança em `source/engine/` deve permanecer estritamente independente de Widgets do Qt (`QtWidgets`), dependendo apenas de `QtCore`, `QtSql` e APIs Win32.
  - Todas as operações de inspeção de arquivos devem ser **estritamente somente leitura**. Nunca solicite handles de escrita em alvos durante varreduras.
  - Qualquer lógica de remediação deve obrigatoriamente gerar backup prévio (`icacls /save`) e script de reversão imediata (`icacls /restore`).
- **Codificação de Arquivos:** UTF-8 sem BOM (exceto onde ferramentas específicas do Windows exijam), com indentação de 4 espaços.
- **Convenções de Nomenclatura:**
  - Classes e Estruturas: `PascalCase` (ex: `EffectiveAccessCalculator`, `AceEntry`).
  - Métodos e Funções: `camelCase` (ex: `calculateEffectiveAccess`).
  - Funções Win32: Utilizar explicitamente as versões de caracteres largos `W` (ex: `GetNamedSecurityInfoW`).

---

## 5. Como Submeter um Pull Request

1. Faça um **Fork** do repositório e crie uma branch a partir de `main`:
   ```bash
   git checkout -b feature/minha-melhoria
   ```
2. Realize seus **Commits** com mensagens claras e semânticas:
   - `feat: adiciona exportação CSV no utilitário CLI`
   - `fix: corrige expansão de grupos aninhados no Active Directory`
   - `docs: atualiza instruções de build para MSVC`
3. Execute e valide os testes:
   ```powershell
   ctest --output-on-failure
   ```
4. Envie a branch para o seu fork e abra um **Pull Request** apontando para a branch `main`.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the instructions in English</summary><br>

# Contributing to ACL Auditor

First off, thank you for considering contributing to **ACL Auditor**! Your help makes Windows security auditing more transparent, robust, and accessible for everyone.

> [!NOTE]
> **About Project Development, Support, and External Contributions:**  
> This project is designed, developed, and maintained primarily as a personal and educational initiative by the author, who is a beginner and in an ongoing learning journey in software engineering.  
> Feedback, suggestions, *Issues*, and *Pull Requests (PRs)* are always welcomed from the community! However, please be aware that reviewing technical inquiries, code submissions, and upstream merges is carried out strictly at the author's own pace on a voluntary basis. If a ticket or PR is not answered promptly or is not merged into the mainline branch, please do not react with alarm or frustration. Under the terms of the GNU AGPLv3 License, you have complete freedom to create an independent *Fork* of this repository to evolve your own ideas and customized builds.

---

## 1. Code of Conduct

By participating in this project, you agree to abide by our [Code of Conduct](CODE_OF_CONDUCT.md). Please treat all contributors with respect, courtesy, and professionalism.

---

## 2. Ways to Contribute

You can contribute to ACL Auditor in many ways:
- **Reporting Bugs:** Submit clear, reproducible issue reports.
- **Requesting Features:** Propose ideas, compliance mappings, or workflow improvements.
- **Improving Documentation:** Clarify build instructions, document Active Directory edge cases, or fix typos.
- **Submitting Pull Requests:** Implement bug fixes, new risk heuristics, CLI features, or translations.
- **Translations:** Help localize ACL Auditor into additional languages (e.g., German, Spanish, French).

---

## 3. Development Environment Setup

### 3.1. Prerequisites
- **OS:** Windows 10 / 11 x64 or Windows Server 2022+
- **Compiler:** 
  - Microsoft Visual C++ (MSVC) from Visual Studio 2022 (v143 toolset) or Build Tools 2022
  - Or MinGW GCC 13+ (x86_64-w64-mingw32)
- **CMake:** Version 3.21 or higher
- **Qt:** Qt 6.x (Core, Gui, Widgets, Sql, PrintSupport)
- **Build Tool:** Ninja recommended

### 3.2. Building with CMake Presets
We provide pre-configured presets in `CMakePresets.json`:

```powershell
# Clone the repository
git clone https://github.com/fernandoncidade/ACL_Auditor_cpp.git
cd ACL_Auditor_cpp

# Configure and build with MSVC (Visual Studio 2022)
cmake --preset host-qt6-msvc-release
cmake --build --preset host-qt6-msvc-release

# Run automated tests
ctest --preset host-qt6-msvc-release --output-on-failure
```

Or with MinGW:

```powershell
cmake --preset host-qt6-mingw-release
cmake --build --preset host-qt6-mingw-release
ctest --preset host-qt6-mingw-release --output-on-failure
```

### 3.3. Building without Presets (Manual Configuration)
If Qt is installed in a custom location, point `CMAKE_PREFIX_PATH` to your Qt installation:

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/msvc2022_64"
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

---

## 4. Coding Standards

- **Language Standard:** C++20 (`set(CMAKE_CXX_STANDARD 20)`). Use modern C++ practices (RAII, smart pointers, structured binding, concepts where beneficial).
- **Architecture Integrity:**
  - The security engine in `source/engine/` must remain independent of Qt Widgets (`QtWidgets`). It should only depend on `QtCore`, `QtSql`, and Win32 APIs.
  - All file inspection operations must remain **strictly read-only**. Never take write handles to file targets during scanning.
  - Any remediation logic must support backup (`icacls /save`) and rollback (`icacls /restore`).
- **Formatting:** UTF-8 file encoding without BOM (except where Windows tools specifically require it), 4-space indentation.
- **Naming Conventions:**
  - Classes and Structs: `PascalCase` (e.g., `EffectiveAccessCalculator`, `AceEntry`).
  - Methods and Functions: `camelCase` (e.g., `calculateEffectiveAccess`).
  - Win32 API calls: Explicitly use wide-character `W` functions (e.g., `GetNamedSecurityInfoW`, not `GetNamedSecurityInfoA`).

---

## 5. Submitting a Pull Request

1. **Fork** the repository and create your branch from `main`:
   ```bash
   git checkout -b feature/my-cool-feature
   ```
2. **Commit** your changes with meaningful messages:
   - `feat: add JSON export to CLI tool`
   - `fix: resolve nested group expansion issue`
   - `docs: update troubleshooting guide for MSVC`
3. **Verify** that tests pass:
   ```powershell
   ctest --output-on-failure
   ```
4. **Push** to your fork and submit a Pull Request targeting the `main` branch.

</details>
