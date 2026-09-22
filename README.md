> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# Low-Latency Structures Academy

*Beyond the Standard Library.* Onde a biblioteca padrão de C++ para e o caminho
crítico começa. Todo número publicado tem um programa que o produz, medido numa
máquina nomeada.

## Estado do projeto

> **Esqueleto.** Este documento é um esqueleto e se declara como tal. A
> identidade pública do projeto — tese, leitores, o que o projeto não é — está
> em [`docs/origem/setup-lowlatency-structures-academy.md`](docs/origem/setup-lowlatency-structures-academy.md)
> e ainda **não** foi reescrita para o leitor. **Nenhum número de desempenho foi
> publicado.**

| Componente | Estado |
|---|---|
| [Padrão do projeto](docs/padrao-do-projeto.md) | **escrito** — 36 seções, 7 partes; delta da absorção aplicado |
| [Catálogo de referências](docs/referencias.md) | **escrito** — endereços pendentes de conferência |
| [Ferramental de qualidade](ferramental/qualidade/) | **código existe, não medido** — 5 verificadores, autotestes passando |
| [Régua de apuração](lib/measurement/) | **código existe, não medido** — compila nas três configurações |
| [Biblioteca de contratos](lib/contract/) | **código existe, não medido** — três portas, suíte nas três configurações |
| [`scripts/`](scripts/) | **código existe, não medido** — `check-env.sh`, `ambiente.sh` (texto, markdown, JSON), `build-all.sh`, `test-all.sh`, `arquivar-medicao.sh`, com autotestes na suíte |
| [CI](.github/workflows/ci.yml) e [pre-commit](ferramental/qualidade/pre-commit.sh) | **código existe, não medido** — consistência antes de build; matriz GCC 14 × Clang 18 × três configurações |
| [Trilha de módulos](trilha/README.md) | **um tópico medido** — [08.01, o harness medindo a si mesmo](trilha/08-medicao/01-harness/README.md); os outros oito módulos não iniciados |
| Capstone (motor de livro de ofertas) | **não iniciado** |

As etiquetas de estado são normativas — ver
[padrão do projeto, seção 5](docs/padrao-do-projeto.md#5-rotulagem-de-estado).

## A tese

Cada tópico pega o mecanismo que a linguagem entrega, mede o ponto exato em que
ele deixa de servir, e constrói o complemento — no mesmo harness, com o mesmo
contrato, na mesma máquina. O par `std` → `custom` é a unidade de trabalho, e a
**regra de decisão** — o volume, a cardinalidade ou o percentil a partir do qual
a troca compensa — é o entregável.

## Governança

| Documento | Para quê |
|---|---|
| [Norma do projeto](docs/padrao-do-projeto.md) | método, medição, contrato, arquitetura, fontes |
| [Política de segurança](SECURITY.md) | o ativo protegido é a **procedência do conteúdo** |
| [Como citar](CITATION.cff) | se você usar uma regra de decisão, um programa ou um número |
| [CLAUDE.md](CLAUDE.md) | instruções para o assistente; o par `std` e `custom` é a unidade de trabalho |

## Origem

Este repositório absorveu a fundação do `labs_cpp_performance_tuning_academy`
(norma, régua de apuração, biblioteca de contratos, verificadores), que não foi
publicado. Os dois documentos de setup estão preservados intactos em
[`docs/origem/`](docs/origem/).

## Navegação

- [Documentação e norma](docs/README.md)
- [Ordem de construção](ROADMAP.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
