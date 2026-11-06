# Licenciamento e Esclarecimento Comercial / Licensing & Commercial Model — ACL Auditor

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o documento de licenciamento em português</summary><br>

# ACL Auditor — Licenciamento, Termos de Código Aberto e Esclarecimento Comercial

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Licença do Código-Fonte:** GNU Affero General Public License v3 (AGPLv3)  
**Autor e Detentor dos Direitos Autorais:** Fernando Nillsson Cidade  

---

## 1. Visão Geral do Modelo de Licenciamento

O ACL Auditor adota um modelo aberto transparente que concilia a liberdade do ecossistema de software livre copyleft com a distribuição corporativa oficial:

| Componente | Licença | Usos Permitidos |
|---|---|---|
| **Código-Fonte (Repositório Completo)** | [GNU AGPLv3](../LICENSE) | 100% Livre: Uso, estudo, modificação, distribuição e contribuição sob termos copyleft da AGPLv3. |
| **Documentação em `source/assets/`** | Autoria do Projeto (Uso Livre para Desenvolvedores) | Criada para a Microsoft Store; desenvolvedores possuem liberdade total para alterar, excluir, adicionar e adaptar em seus próprios forks e plataformas. |
| **Binários Compilados por Terceiros (via Código)** | GNU AGPLv3 | Totalmente livres para compilar, executar, empacotar e redistribuir sob a responsabilidade do desenvolvedor, cumprindo os termos da AGPLv3. |
| **Binário Oficial Compilado na Microsoft Store** | Edição Oficial do Autor | Proibida a revenda, comercialização de terceiros ou engenharia reversa do binário oficial fornecido pelo autor. |
| **Bibliotecas de Terceiros (Qt, SQLite)** | LGPL v3 / Domínio Público | Cumprem integralmente os requisitos de atribuição de código aberto upstream. |

---

## 2. Direitos de Código Aberto (GNU AGPLv3)

Todo o código-fonte do ACL Auditor — abrangendo a engine C++20 (`ACLAuditorEngine`), o utilitário CLI (`acl-auditor-cli`), os modelos virtuais Qt 6, os widgets de interface, os recursos de internacionalização e os presets do CMake — está licenciado sob a **GNU Affero General Public License v3 (AGPLv3)**:

```text
ACL Auditor — Windows NTFS & SMB Permission Auditor
Copyright (c) 2025-2026 Fernando Nillsson Cidade

Este programa é um software livre: você pode redistribuí-lo e/ou modificá-lo sob os
termos da GNU Affero General Public License conforme publicada pela Free Software
Foundation, seja a versão 3 da Licença, ou (a seu critério) qualquer versão posterior.

Este programa é distribuído na esperança de que seja útil, mas SEM QUALQUER GARANTIA;
sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM.
Veja a GNU Affero General Public License para mais detalhes.

Você deve ter recebido uma cópia da GNU Affero General Public License junto com este
programa. Se não, veja em: <https://www.gnu.org/licenses/>.
```

---

## 3. Documentação em `source/assets/` e Liberdade Total para Desenvolvedores

A documentação institucional e regulatória mantida no diretório `source/assets/` (abrangendo os módulos `ABOUT`, `CLC`, `COPYRIGHT`, `EULA`, `LICENSES`, `NOTICES`, `PRIVACY_POLICY` e `RELEASE`) possui status e finalidade específicos:

1. **Finalidade Original:** Estes documentos foram elaborados por Fernando Nillsson Cidade exclusivamente para viabilizar a publicação, conformidade regulatória e comercialização da aplicação oficial na **Microsoft Store** (cumprindo exigências contratuais de EULA, ADA, privacidade e conformidade LGPD/GDPR para marketplaces).
2. **Autorização Expressa para Desenvolvedores:** O autor **autoriza expressamente e concede total liberdade** para que desenvolvedores, contribuidores e forks realizem quaisquer alterações, exclusões, adições ou modificações gerais nessa documentação e no código associado.
3. **Condição de Responsabilidade:** Os desenvolvedores são livres para adaptar ou remover esses textos em seus próprios forks e distribuições, desde que respeitem as diretrizes, normas e a legislação vigente de seus respectivos países e das plataformas em que decidirem publicar suas próprias compilações.

---

## 4. Diferenciação dos Binários Oficiais Compilados pelo Autor

Há uma distinção jurídica e operacional fundamental entre o **código-fonte** e o **binário oficial compilado pelo autor**:

- **Liberdade Absoluta no Código-Fonte:** O código-fonte integral está aberto e disponível publicamente neste repositório sob a Licença GNU AGPLv3. Qualquer pessoa é livre para inspecionar, auditar, compilar, modificar e criar executáveis próprios a partir do código, respeitadas as condições copyleft da licença.
- **Restrição sobre o Binário Oficial do Autor (Microsoft Store):** Apenas o binário compilado, empacotado, assinado digitalmente e disponibilizado oficialmente pelo autor (**Fernando Nillsson Cidade**), notadamente o produto comercial distribuído na Microsoft Store:
  - **NÃO possui autorização** para ser comercializado, revendido ou redistribuído comercialmente por terceiros como se fosse produto deles;
  - **NÃO possui autorização** para ser submetido a engenharia reversa (prática que, ademais, torna-se totalmente inócua e desnecessária, visto que o código-fonte original completo já é disponibilizado publicamente pelo próprio autor neste repositório).
- **Compilação Própria Permitida:** Caso terceiros desejem comercializar ou distribuir uma solução baseada neste projeto, devem compilar seus próprios binários a partir do código-fonte aberto, gerando seus próprios pacotes e assumindo a responsabilidade legal por suas distribuições, sem utilizar as chaves de assinatura oficial, marca ou pacotes prontos do autor.

---

## 5. Licenças de Terceiros e Higiene de Propriedade Intelectual

1. **Framework Qt (Qt 6.11.1):** Licenciado sob a **GNU LGPL v3**. O ACL Auditor vincula dinamicamente as DLLs do Qt, permitindo substituição das bibliotecas conforme o item 4(d) da LGPL v3.
2. **SQLite3:** Dedicado formalmente ao **Domínio Público**.
3. **SDK de Segurança do Windows:** APIs padrão Win32 (`advapi32`, `authz`, `netapi32`, `bcrypt`, `dwmapi`) fornecidas nativamente pelo sistema operacional.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the licensing document in English</summary><br>

# ACL Auditor — Licensing, Open Source Terms & Commercial Clarification

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Source Code License:** GNU Affero General Public License v3 (AGPLv3)  
**Author & Copyright Owner:** Fernando Nillsson Cidade  

---

## 1. Overview of the Licensing Model

ACL Auditor adopts a transparent open-source model that balances community freedom under copyleft terms with official enterprise distribution:

| Component | License | Permitted Uses |
|---|---|---|
| **Source Code (Full Repository)** | [GNU AGPLv3](../LICENSE) | 100% Free: Study, audit, modify, compile, distribute, and contribute under AGPLv3 copyleft terms. |
| **Documentation in `source/assets/`** | Project Authorship (Free Adaptation) | Created for Microsoft Store; developers have complete freedom to modify, delete, add, and adapt for their own forks and platforms. |
| **Self-compiled Binaries (from Source)** | GNU AGPLv3 | Completely free to compile, run, package, and distribute under developer's responsibility, complying with AGPLv3. |
| **Official Compiled Binary (Microsoft Store)** | Author's Official Release | Third-party commercial resale, redistribution, or reverse engineering of the author's official compiled binary is prohibited. |
| **Third-Party Libraries (Qt, SQLite)** | LGPL v3 / Public Domain | Complies with all upstream open source attribution requirements. |

---

## 2. Source Code Open Source Rights (GNU AGPLv3)

The complete source code of ACL Auditor — including the core C++20 engine (`ACLAuditorEngine`), the CLI tool (`acl-auditor-cli`), the virtualized Qt 6 models, the UI widgets, translation files, and CMake presets — is licensed under the **GNU Affero General Public License v3 (AGPLv3)**:

```text
ACL Auditor — Windows NTFS & SMB Permission Auditor
Copyright (c) 2025-2026 Fernando Nillsson Cidade

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

---

## 3. Documentation in `source/assets/` and Total Developer Freedom

The institutional and regulatory documentation maintained under the `source/assets/` directory (encompassing `ABOUT`, `CLC`, `COPYRIGHT`, `EULA`, `LICENSES`, `NOTICES`, `PRIVACY_POLICY`, and `RELEASE`) carries a specific context:

1. **Original Purpose:** These documents were authored by Fernando Nillsson Cidade exclusively to enable the official certification, compliance, and commercial publishing of the application on the **Microsoft Store** (satisfying mandatory EULA, ADA, and privacy requirements for enterprise app marketplaces).
2. **Express Developer Authorization:** The author **expressly authorizes and grants full freedom** to developers, contributors, and forks to make any changes, deletions, additions, or general modifications to this documentation and associated code.
3. **Compliance Requirement:** Developers are completely free to adapt or remove these assets in their own forks and custom builds, provided they respect the guidelines, standards, and legislation applicable in their respective countries and target distribution platforms.

---

## 4. Clarification on the Author's Official Pre-Compiled Binaries

There is a clear legal and operational distinction between the **open source code** and the **author's official compiled binaries**:

- **Absolute Freedom in Source Code:** The full source code is publicly accessible in this repository under the GNU AGPLv3 License. Anyone is free to audit, compile, modify, build custom executables, and embed the engine into their own solutions in compliance with AGPLv3 copyleft terms.
- **Restrictions on the Author's Official Pre-Compiled Binary (Microsoft Store):** The official binary compiled, packaged, digitally signed, and distributed directly by the author (**Fernando Nillsson Cidade**), specifically the commercial edition sold on the Microsoft Store:
  - **May NOT** be commercially resold, redistributed for profit, or re-hosted by third parties as their own commercial product;
  - **May NOT** be subjected to reverse engineering (which is entirely pointless anyway, since the author already provides the full, unabridged source code openly in this repository).
- **Custom Builds are Welcome:** Third parties wishing to distribute or commercialize solutions derived from this project must compile their own binaries from the open source code, publish them under their own responsibility, and provide their own branding and packaging without misusing the author's official signing identity, trademarks, or pre-packaged store releases.

---

## 5. Third-Party Licenses & IP Hygiene

1. **Qt Framework (Qt 6.11.1):**
   - Licensed under the **GNU LGPL v3**.
   - ACL Auditor dynamically links against Qt shared libraries (`.dll`), allowing users to replace Qt libraries with newer or modified builds as required by LGPL v3 section 4(d).
   - Upstream notices and source code links are preserved in `source/assets/LICENSES/` and `source/assets/NOTICES/`.
2. **SQLite3:**
   - Dedicated in the **Public Domain**.
   - Used for local, offline snapshot persistence.
3. **Windows Security SDK:**
   - Standard Win32 system APIs (`advapi32`, `authz`, `netapi32`, `bcrypt`, `dwmapi`) provided natively by Microsoft Windows.

</details>
