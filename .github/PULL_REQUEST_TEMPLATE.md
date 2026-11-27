# Modelo de Pull Request / Pull Request Template

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

> [!NOTE]
> **Aviso ao Colaborador / Contributor Notice:**  
> 🇧🇷 *Este repositório é um projeto pessoal mantido pelo autor em ritmo próprio de aprendizado. Pull Requests são bem-vindos, mas a análise e incorporação de código ocorrem de forma voluntária e sem compromisso de prazo. Caso não haja manifestação ou merge, sinta-se plenamente livre para manter e utilizar suas melhorias em seu próprio fork sob a licença MIT.*  
> 🇺🇸 *This repository is a personal project maintained by the author at their own learning pace. Pull Requests are welcome, but review and merging are carried out voluntarily without any guaranteed timeframe. If there is no response or merge, please feel free to maintain and use your improvements in your own fork under the MIT license.*

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o template em português</summary><br>

## Descrição
Descreva brevemente a finalidade deste PR e qual problema ele resolve.

## Tipo de Alteração
- [ ] Correção de bug (alteração sem quebra que corrige um problema)
- [ ] Nova funcionalidade (alteração sem quebra que adiciona funcionalidade)
- [ ] Melhoria no CLI / Automação
- [ ] Otimização de desempenho / Refatoração
- [ ] Atualização de documentação
- [ ] Atualização de localização / tradução

## Testes Realizados
Descreva os testes executados para validar suas alterações:
- [ ] Compilado com MSVC (`cmake --preset host-qt6-msvc-release`)
- [ ] Compilado com MinGW (`cmake --preset host-qt6-mingw-release`)
- [ ] Testes automatizados executados com sucesso (`ctest --output-on-failure`)
- [ ] Testes manuais no Windows 10/11:
  - Detalhes: 

## Conformidade Arquitetural e de Segurança
- [ ] O código da engine em `source/engine/` NÃO possui dependência de `QtWidgets`.
- [ ] Todas as operações de inspeção de arquivos permanecem estritamente **somente leitura**.
- [ ] Nenhuma conexão de rede ou telemetria foi introduzida.
- [ ] Qualquer nova lógica de remediação gera comandos de backup e rollback.

## Issues Relacionadas
Corrige #(número da issue)

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the template in English</summary><br>

## Description
Briefly describe the purpose of this PR and what problem it solves.

## Type of Change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] CLI / Automation improvement
- [ ] Performance optimization / Refactoring
- [ ] Documentation update
- [ ] Localization / Translation update

## Testing Performed
Describe the tests you ran to verify your changes:
- [ ] Built with MSVC (`cmake --preset host-qt6-msvc-release`)
- [ ] Built with MinGW (`cmake --preset host-qt6-mingw-release`)
- [ ] Automated tests passed (`ctest --output-on-failure`)
- [ ] Manual testing on Windows 10/11:
  - Details: 

## Architectural & Security Compliance
- [ ] Engine code in `source/engine/` does NOT depend on `QtWidgets`.
- [ ] File and directory inspection operations remain strictly **read-only**.
- [ ] No telemetry or network sockets were introduced.
- [ ] Any new remediation logic includes backup and rollback capabilities.

## Related Issues
Fixes #(issue number)

</details>
