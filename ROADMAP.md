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
- [x] **delta da absorção na norma** — 33 seções e 6 partes viraram 36 e 7:
      seção 30 reconciliada, seção 11 promovida a portão de publicação do par,
      seção 16 sem arm64 e com dois perfis de sanitizer, seções 28 e 29 com CCD,
      precisão de histograma, variação entre execuções e a máquina nomeada, e a
      **Parte VII** nova — par `std` e `custom` (34), regra de decisão (35),
      eixo do código gerado (36)
- [x] tabela de divergências em relação ao setup desta trilha: 12 itens, todos
      verificados nesta máquina antes de entrarem na norma
- [x] colisão de nome de `contract.hpp`: a lei do tópico é `spec.hpp` (seção 34)
- [x] colisão de caminho, e a **regra por trás dela invertida**: o nome é
      `bench/medicoes/`, e o histórico de campanha passa a ser **versionado**. O
      `.gitignore` herdado excluía saída bruta; num projeto cujo ativo é a
      procedência do número, isso remove a evidência. Vale a prática do DPDK
      Academy: `medicoes/historico/<data>-<campanha>/`, ambiente ao lado, uma
      saída por repetição
- [x] o eixo `linguagem ↔ compilador ↔ código gerado` virou a seção 36, e o
      módulo transversal `10-codigo-gerado` está previsto na trilha
- [ ] prefixo `CPP_ACADEMY_*` das variáveis de ambiente da régua de apuração:
      mantido na absorção, decidir se acompanha a identidade nova
- [ ] `pre-commit.sh` e CI executando a matriz de três configurações
- [ ] verificador de **par de idiomas**: a regra do radical idêntico (seção 2 da
      norma) existe para ser verificável, e hoje não é

## Etapa 2 — Ambiente · **parcial**

- [x] `scripts/ambiente.sh`, portado **sem reescrever o parsing**: a versão a
      montante usa `lscpu -p=` e sysfs porque parsing do texto do `lscpu`
      devolve campo vazio em silêncio em máquina com locale traduzido — e esta
      máquina tem. Três modos: texto, `--markdown`, `--json`
- [x] registro de disponibilidade do PMU (`perf_event_paranoid`)
- [x] registro de quais configurações de build existem na árvore
- [x] CCD e *SMT sibling* de cada CPU lógica, com uma linha por CPU
- [x] `scripts/check-env.sh` — diagnóstico com sonda de C++23 real (compila
      `<expected>`, `<print>`, `<span>` e `hardware_destructive_interference_size`
      em vez de olhar a versão do compilador), e cada aviso dizendo o que deixa
      de ser possível
- [x] `scripts/build-all.sh`, `scripts/test-all.sh` e `scripts/lib-configuracoes.sh`,
      com a matriz de configurações num lugar só
- [x] autotestes na suíte `l1+scripts`: braço sintético de dois domínios de L3
      para o agrupamento, e braço de controle que esconde o Meson do `PATH` e
      exige que o `check-env.sh` acuse
- [ ] **`perf_event_paranoid=4` nesta máquina bloqueia contador de hardware.**
      O módulo de `perf` e *cache misses* não mede sem `sysctl
      kernel.perf_event_paranoid=0`, que é mudança de sistema e fica como
      decisão declarada, não como pré-requisito silencioso
- [ ] `governor=powersave` na máquina de referência: a campanha precisa declarar
      se fixa `performance` antes de medir, ou publica a variação que o governor
      introduz

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
