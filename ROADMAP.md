> 🇧🇷 Português | [🇺🇸 English](ROADMAP.en.md)

# Roadmap do projeto

**Este documento é a ordem de CONSTRUÇÃO do material. Não é ordem de leitura.**

A ordem de estudo canônica será a da trilha, em `docs/plano-estudo.md`, **ainda
não escrito**. A distinção existe porque no projeto a montante duas numerações
concorrentes conviveram no mesmo README e "nível 5" passou a significar duas
coisas diferentes conforme o documento.

> **Esqueleto.** Este documento é um esqueleto e se declara como tal. As etapas
> abaixo da 2 vêm de
> [`docs/origem/setup-lowlatency-structures-academy.md`](docs/origem/setup-lowlatency-structures-academy.md)
> e ainda não foram reconciliadas com a norma absorvida.

## Etapa 1 — Absorção da fundação · **parcial**

O `labs_cpp_performance_tuning_academy` — C++23, mesma máquina, mesmo método,
trilha não iniciada, **zero commits e sem remoto** — foi absorvido por este
repositório em 2026-09-21. Veio o que já era executável:

- [x] `git init`, licença MIT, `.gitignore`, `.gitattributes`, `.clang-format`,
      `.clang-tidy`
- [x] os **dois** documentos de origem preservados intactos em
      [`docs/origem/`](docs/origem/)
- [x] [norma](docs/padrao-do-projeto.md): 33 seções, 6 partes, com as 14
      divergências em relação à sua própria origem registradas de forma auditável
- [x] [catálogo de referências](docs/referencias.md) com identificadores estáveis
- [x] `lib/measurement/` — régua de apuração em C++23, *header-only* de
      propósito: uma chamada através de fronteira de tradução mediria a chamada
      junto com a operação
- [x] `lib/contract/` — as três portas T/R/E, e `verificar-suposicao.py`, que
      confere no assembly que a pré-condição da porta R chega ao otimizador
- [x] 5 verificadores, cada um com o **seu autoteste** registrado ao lado
- [x] `meson.build` com a matriz de três configurações e o `-Werror=assume`
      condicional
- [x] suíte verde nas três configurações **nesta** máquina (GCC 15.2.0):
      `debug` e `release-checked` com 11 OK + 1 falha esperada,
      `release` com 11 OK + 1 pulado
- [ ] **delta da absorção na norma** — seção 30 (dois projetos a montante, não
      três), seção 11 promovida a definição do par `std` → `custom`, seção nova
      para a regra de decisão, seção 29 com a máquina de dois CCDs
- [ ] tabela de divergências em relação ao setup desta trilha: `.wraplock`
      inexistente, HdrHistogram fora do WrapDB, dois perfis de sanitizer,
      `docs/0N-*/medicoes/`, ABI C do harness, CCD e SMT na metadata, arm64 sem
      máquina, TSC não invariante não reproduzível aqui
- [ ] colisão de nome de `contract.hpp`: a lei do tópico passa a ser `spec.hpp`
- [ ] colisão de caminho: o `.gitignore` herdado ignora `resultados/**`, e o setup
      desta trilha arquiva em `bench/medicoes/` — um dos dois nomes sai
- [ ] prefixo `CPP_ACADEMY_*` das variáveis de ambiente da régua de apuração:
      mantido na absorção, decidir se acompanha a identidade nova
- [ ] o eixo `linguagem ↔ compilador ↔ código gerado`, que era objeto próprio do
      projeto absorvido, ganha módulo transversal `10-codigo-gerado`
- [ ] `pre-commit.sh` e CI executando a matriz de três configurações
- [ ] verificador de **par de idiomas**: a regra do radical idêntico (seção 2 da
      norma) existe para ser verificável, e hoje não é

## Etapa 2 — Ambiente

- [ ] `scripts/ambiente.sh`, portado **sem reescrever o parsing**: a versão a
      montante usa `lscpu -p=` e sysfs porque parsing do texto do `lscpu`
      devolve campo vazio em silêncio em máquina com locale traduzido — e esta
      máquina tem
- [ ] registro de disponibilidade do PMU (`perf_event_paranoid`)
- [ ] registro de qual das três configurações produziu o número
- [ ] CCD e *SMT sibling* do núcleo usado, não só o id

## Etapa 3 — Harness e o ciclo completo

- [ ] `trilha/08-medicao/01-harness/`: um tópico mínimo que compila, roda e
      arquiva uma medição trivial com `metadata.json` — para provar o ciclo
      inteiro **antes** de escrever conteúdo
- [ ] decisão HdrHistogram versus amostras cruas em buffer pré-alocado
- [ ] `docs/regras-de-decisao.md` com esquema de dados e verificador que confira
      cada linha contra um `metadata.json` existente

## Etapas 4 em diante — a trilha

Os nove módulos, o cenário do livro de ofertas e o capstone estão descritos em
[`docs/origem/setup-lowlatency-structures-academy.md`](docs/origem/setup-lowlatency-structures-academy.md),
seções 4 a 6, e entram aqui quando reconciliados com a norma.

## Navegação

- [README do repositório](README.md)
- [Documentação e norma](docs/README.md)

> 🇧🇷 Português | [🇺🇸 English](ROADMAP.en.md)
