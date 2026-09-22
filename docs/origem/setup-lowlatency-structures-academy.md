# Low-Latency Structures Academy — documento de setup

**Repositório:** `hroberto/labs-lowlatency-structures-academy`
**Subtítulo:** *Beyond the Standard Library*
**Descrição do repositório (EN):** Where the C++ standard library stops and the hot path begins. Every published number has a program that produces it, measured on a named machine. Portuguese prose, English code.
**Topics:** `cpp23` `data-structures` `low-latency` `lock-free` `cache-friendly` `memory-allocation` `benchmarking` `performance-engineering` `systems-programming` `meson` `portuguese` `learning-resources`

Este documento é a fonte de verdade para iniciar o repositório. Ele diz o que o
projeto é, o que herda do [DPDK Academy](https://github.com/hroberto/labs-dpdk-academy),
o que muda, e em que ordem construir. Depois de o repositório existir, o
conteúdo daqui migra para `README.md`, `ROADMAP.md`, `CLAUDE.md` e
`docs/00-visao-geral/`, e este arquivo deixa de ser referência.

---

## 1. A tese

> Cada tópico pega o mecanismo que a linguagem entrega, mede o ponto exato em que
> ele deixa de servir, e constrói o complemento — no mesmo harness, com o mesmo
> contrato, na mesma máquina.

Não é "reinventar a STL" nem um catálogo de otimizações. É um estudo
experimental de **onde a biblioteca padrão para**, com três resultados
possíveis por tópico, e todos os três se publicam:

| Resultado | O que o leitor aprende |
|---|---|
| a STL moderna basta | qual recurso usar, e por que a alternativa artesanal não compensa |
| a STL basta até um volume ou percentil | a **regra de decisão**: em que ponto trocar |
| a STL não serve no caminho crítico | o mecanismo que a substitui, e o que ele custa em troca |

A diferença para um repositório de otimizações está na terceira coluna: o
leitor sai sabendo *quando*, não só *como*.

### 1.1 Três leitores, um material

O repositório serve a três usos ao mesmo tempo, e o método é o mesmo para os
três — o que muda é só o ponto de entrada:

- **base acadêmica** — trilha progressiva, de memória a capstone, para quem
  quer aprender engenharia de desempenho em C++ moderno;
- **referência interna** — a tabela de regras de decisão e os números por
  percentil são o que se consulta ao projetar um sistema real;
- **influência na área** — cada tópico publicado é uma afirmação verificável
  com programa, e é esse tipo de afirmação que circula.

### 1.2 O que este projeto não é

- não é curso de algoritmos: complexidade assintótica é premissa, não assunto;
- não é *academy* de mercado financeiro: o livro de ofertas aparece como
  **cenário recorrente** (como o servidor de market data no DPDK Academy),
  não como objeto de estudo;
- não publica estratégia, sinal ou lógica de decisão de nenhum sistema —
  só infraestrutura;
- não compara compiladores, nem bibliotecas de terceiros entre si: o par é
  sempre `std` versus `custom`, no mesmo contrato.

---

## 2. O que herda do DPDK Academy

Herança explícita, item a item. O que não está aqui não é herdado por padrão.

### 2.1 Método e regras editoriais

- **O percurso de todo tópico:** problema → mecanismo → trade-offs →
  implementação → medição → confronto com a literatura → análise → decisão de
  arquitetura. Sem template de seções obrigatórias; o percurso é o que se
  confere.
- **Afirmação numérica precisa de programa que a produza.** Folclore
  ("`std::map` é lento", "alocação custa dezenas de nanossegundos") é
  verificado, e quando não se sustenta, o material se corrige e registra a
  correção.
- **Afirmação normativa precisa nomear quem a define.** ISO/IEC 14882, a
  documentação do compilador, o manual do fabricante da CPU, um paper com
  DOI. Wikipédia não é fonte.
- **Como ler os números:** a convenção na linguagem, não em etiquetas —
  "nesta máquina, medimos" é medição; "a documentação diz" é fato; "ou seja"
  é inferência; "fica em aberto" é pendência.
- **Desempenho não é previsibilidade.** Latência publicada por percentis
  (mediana, p75, p99, p99,9), nunca só por média, sempre com dispersão.
- **Toda seção tem "Limitações"** (o que os números não autorizam concluir) e
  **"Quando dá errado"** (o que o sistema faz fora do caminho feliz),
  respondida com experimento executável, nunca em prosa.
- **Recusas registradas:** FMEA/FTA, template de 16 seções, marcação
  FATO/MEDIÇÃO/INFERÊNCIA por frase — recusados lá pelas mesmas razões, e
  recusados aqui.

### 2.2 Estrutura e ferramental

- **Tópico autocontido:** documento, código, testes e alternativa no mesmo
  diretório.
- **Meson + Ninja**, `cpp_std=c++23`, `warning_level=3`,
  `buildtype=debugoptimized` como padrão e um perfil `release` para os
  benchmarks publicados.
- **Dependências de teste fixadas por hash** em `subprojects/*.wrap`, com
  `.wraplock`.
- **Testes em níveis:** L1 (lógica pura, GoogleTest, rápido) e L2 (o binário
  como o leitor o executa, via script). L3 fica reservado para experimentos
  que exigem privilégio (hugepages, isolamento de core).
- **Scripts com o mesmo vocabulário:** `check-env.sh`, `ambiente.sh`,
  `build-all.sh`, `test-all.sh`, com os testes dos próprios scripts em
  `scripts/tests/`.
- **Ambiente de medição gerado, não descrito:** `scripts/ambiente.sh
  --markdown` produz a tabela que entra em cada documento com número.
- **`ferramental/qualidade/`:** verificadores de âncora de linha, de
  autodescrição (contagens no texto conferidas contra o disco), de paridade
  de idioma, rodando na suíte `l1+docs` e no pre-commit.
- **CI:** imagem fixa (`ubuntu-24.04`), ação fixada por SHA, `permissions:
  contents: read`, gatilho só em `main` e `pull_request`.
- **Commits assinados**, `main` exigindo assinatura verificada, `SECURITY.md`
  declarando que o ativo protegido é a procedência do conteúdo.
- **`CITATION.cff`**, licença MIT, `.clang-format` e `.clang-tidy` iguais aos
  do DPDK Academy no ponto de partida.

### 2.3 Idioma

- Prosa em **português**; identificadores, saída de programa, mensagens de
  log e nomes de arquivo de código em **inglês**. Nomes de caso de GoogleTest
  em português, por serem prosa.
- **Paridade PT/EN desde o primeiro tópico** — a lição do DPDK Academy foi que
  adiar custa mais do que manter. `README.en.md` par de cada `README.md`,
  verificado por máquina: nenhum número publicado em inglês pode faltar no
  português, e nenhum bloco de código em `.en.md` pode conter português.

---

## 3. O que muda em relação ao DPDK Academy

| No DPDK Academy | Aqui | Por quê |
|---|---|---|
| dependência de ambiente (`libdpdk` via pkg-config) | **nenhuma dependência de sistema além do compilador** | o objeto é a linguagem; qualquer máquina Linux com GCC 14+ ou Clang 18+ roda tudo |
| a alternativa mora em `alternativas/cpp23/` | a alternativa é **o próprio tópico**: `std/` e `custom/` lado a lado | a comparação não é apêndice, é a tese |
| DPDK é protagonista, C++23 é contraponto | `std` é o ponto de partida, `custom` é o complemento | ordem invertida de propósito: primeiro o que a linguagem dá |
| privilégio aparece no meio da trilha (VFIO) | privilégio só em L3, sempre opcional | nada do material didático exige root |
| um cenário de rede (servidor de market data) | um cenário de estrutura: o **livro de ofertas** | é o exemplo que junta memória, layout, contêiner, fila e tempo num objeto só |
| HDR Histogram ausente | **HdrHistogram_c** fixado por wrap, sob o harness próprio | percentis altos precisam de histograma com precisão declarada |

Uma seção nova, obrigatória em todo tópico que compara: **"Regra de decisão"**
— o volume, a cardinalidade ou o percentil a partir do qual a troca compensa,
com o número que sustenta a regra. É o artefato de referência interna do
projeto, e a tabela consolidada delas vive em `docs/regras-de-decisao.md`.

---

## 4. Estrutura do repositório

```
labs-lowlatency-structures-academy/
├── README.md / README.en.md
├── ROADMAP.md / ROADMAP.en.md
├── CLAUDE.md
├── SECURITY.md · LICENSE · CITATION.cff
├── meson.build · meson.options
├── .clang-format · .clang-tidy · .gitattributes · .gitignore
├── .github/workflows/ci.yml · .github/dependabot.yml
├── subprojects/
│   ├── gtest.wrap
│   ├── hdr_histogram.wrap
│   └── .wraplock
├── scripts/
│   ├── check-env.sh · ambiente.sh · build-all.sh · test-all.sh
│   ├── preparar-hugepages.sh · isolar-core.sh        (L3, opcionais)
│   ├── lib-*.sh
│   └── tests/
├── ferramental/
│   ├── bench/            harness: HDR, warm-up, pinning, metadata de execução
│   └── qualidade/        verificadores (âncoras, autodescrição, idioma, paridade)
├── docs/
│   ├── 00-visao-geral/   o que é, método, como ler os números, ambiente
│   ├── 01-memoria/       teoria: hierarquia, TLB, alocação, fragmentação
│   ├── 02-layout/        teoria: cache line, alinhamento, SoA/AoS, false sharing
│   ├── 03-concorrencia/  teoria: modelo de memória C++, atomics, ordering
│   ├── regras-de-decisao.md
│   └── plano-estudo.md
└── trilha/
    ├── README.md
    ├── 01-memoria/
    │   ├── 01-alocador-arena/
    │   ├── 02-pool-freelist/
    │   └── 03-pmr-o-meio-termo/
    ├── 02-layout/
    │   ├── 01-soa-vs-aos/
    │   ├── 02-false-sharing/
    │   └── 03-small-buffer/
    ├── 03-conteineres/
    │   ├── 01-vector-vs-list/
    │   ├── 02-flat-map-vs-map/
    │   └── 03-open-addressing-vs-unordered/
    ├── 04-livro-de-ofertas/
    │   ├── 01-map-de-precos/
    │   ├── 02-array-plano-com-bitmap/
    │   └── 03-hash-de-ordens/
    ├── 05-filas-e-concorrencia/
    │   ├── 01-mutex-queue/
    │   ├── 02-spsc-lock-free/
    │   ├── 03-mpsc/
    │   └── 04-wakeup-vs-busy-poll/
    ├── 06-tempo-e-erros/
    │   ├── 01-chrono-vs-rdtsc/
    │   └── 02-exceptions-vs-expected/
    ├── 07-parsing/
    │   ├── 01-string-vs-string-view/
    │   └── 02-decoder-zero-copy/
    ├── 08-medicao/
    │   ├── 01-harness/
    │   └── 02-perf-e-cache-misses/
    └── 09-capstone/
        └── order-book-engine/
```

Pastas `docs/0N-*` carregam a teoria; `trilha/0N-*` a prática. Um tópico da
trilha sem módulo de teoria vira receita; um módulo de teoria sem tópico vira
leitura — a tabela em `trilha/README.md` liga os dois, como no DPDK Academy.

---

## 5. Anatomia de um tópico

```
trilha/04-livro-de-ofertas/02-array-plano-com-bitmap/
├── README.md / README.en.md
├── contract.hpp          interface comum: o que std/ e custom/ precisam cumprir
├── std/
│   └── book_map.hpp      baseline com std::map<Price, Level>
├── custom/
│   └── book_flat.hpp     array plano indexado por tick + bitmap de níveis ocupados
├── bench/
│   ├── bench_book.cpp    os dois, no mesmo harness, mesma carga
│   └── medicoes/         saída arquivada: metadata, CSV, tabela .md
├── tests/
│   ├── l1_contract.cpp   GoogleTest parametrizado: o contrato vale para os dois
│   └── l2_run.sh         roda o benchmark como o leitor rodaria
└── meson.build
```

Regras:

1. **`contract.hpp` é a lei.** Toda implementação passa nos mesmos testes L1,
   parametrizados por tipo. Se `custom/` precisa de um contrato diferente,
   isso é um achado, e o documento diz por quê.
2. **Nenhum número no `README.md` sem arquivo em `bench/medicoes/`** que o
   produza, com a metadata do `ambiente.sh` ao lado.
3. **Teste negativo quando houver invariante:** a versão que viola a
   invariante (por exemplo, `book_flat_no_bitmap`) existe e a suíte exige que
   ela falhe — verificação que nunca falhou é indistinguível de uma que nunca
   dispara.
4. **Seções fixas no README**, em qualquer ordem que o texto pedir: o
   percurso do método, *Quando dá errado*, *Limitações*, *Regra de decisão*.

---

## 6. Trilha — módulos e a pergunta de cada um

| Módulo | Par `std` → `custom` | Pergunta que o tópico responde | Pergunta de falha |
|---|---|---|---|
| 01 memória | `new` / `std::allocator` → arena, pool com free-list; `std::pmr` como meio-termo | a que taxa de alocação o alocador global passa a dominar o p99? | o que acontece quando a arena esgota no meio de um lote? |
| 02 layout | `struct` de campos → SoA; padding manual → `hardware_destructive_interference_size` | quantos cache misses por elemento cada layout custa, medidos? | o que dois contadores na mesma linha fazem com dois núcleos? |
| 03 contêineres | `std::list` → `std::vector`; `std::map` → flat sorted; `std::unordered_map` → open addressing | em que cardinalidade a busca em array plano perde para a árvore? | o que o rehash faz com a cauda? |
| 04 livro de ofertas | `std::map<Price,Level>` → array por tick + bitmap; `unordered_map` de ordens → hash sem alocação por nó | qual o custo de *best bid/ask* em p50/p99/p99,9 sob carga real de mensagens? | o que acontece com preço fora da faixa do array? |
| 05 filas e concorrência | `std::queue` + `mutex` → SPSC → MPSC; `condition_variable` → busy-poll com `_mm_pause` | quando o lock deixa de ser o gargalo e passa a ser a linha de cache? | o que a fila faz quando enche — e quem descobre primeiro? |
| 06 tempo e erros | `steady_clock` → `rdtsc` calibrado; `throw` → `std::expected` | quanto custa medir, e quanto custa o caminho de erro? | o que acontece quando o TSC não é invariante? |
| 07 parsing | `std::string` / `stringstream` → `string_view` + `from_chars` → decoder binário sem cópia | quando a STL moderna já basta, e quando não? | o que o decoder faz com mensagem truncada? |
| 08 medição | — (transversal) | como o harness evita medir a si mesmo? | quando o warm-up esconde a cauda? |
| 09 capstone | tudo acima, em duas versões do mesmo motor | qual a diferença ponta a ponta, e qual tópico respondeu por ela? | o que o motor faz sob rajada acima do orçamento? |

O módulo 07 existe em parte para publicar o resultado incômodo: em vários
casos `string_view` + `from_chars` já é a resposta, e o decoder artesanal não
compensa. Publicar isso é o que dá credibilidade ao resto.

### 6.1 O cenário que atravessa os módulos

O **livro de ofertas** (order book) é o exemplo recorrente: precisa de
alocação previsível (01), layout compacto (02), estrutura de busca por preço
(03, 04), fila de entrada (05), carimbo de tempo (06) e decodificação de
mensagem (07). O capstone é o motor completo, alimentado por um *feed*
sintético reproduzível — nunca por dado proprietário de bolsa.

É embasamento, não camisa de força: quando um conceito fica mais claro com
outro exemplo, usa-se o outro exemplo.

---

## 7. Stack e requisitos

| Item | Escolha | Observação |
|---|---|---|
| Linguagem | C++23 | `<expected>`, `<print>`, `<span>`, `<ranges>`, `hardware_*_interference_size` |
| Compiladores | GCC 14+ **e** Clang 18+ | os dois na CI; onde divergirem, o documento registra |
| Build | Meson ≥ 1.1 + Ninja | dois perfis: `debugoptimized` (padrão) e `release` (números publicados) |
| Testes | GoogleTest 1.17.0 | wrap fixado por hash, herdado do DPDK Academy |
| Histograma | HdrHistogram_c | wrap fixado por hash; precisão declarada em cada tabela |
| Harness | `ferramental/bench/` próprio | warm-up, pinning, repetições, metadata de execução, saída CSV + Markdown |
| Profiling | `perf stat`, `perf record`; eBPF/bpftrace opcional | só a partir do módulo 08 |
| Sanitizers | ASan, UBSan, TSan | perfil `sanitize` no Meson; TSan obrigatório no módulo 05 |
| Plataforma | Linux x86_64; arm64 compila, sem números publicados | `rdtsc` e `_mm_pause` têm ramo por arquitetura |

**O que a máquina precisa ter:** compilador, Meson, Ninja, `perf` (opcional).
Nada mais. `./scripts/check-env.sh` diagnostica.

**O que se pressupõe do leitor:** C++ intermediário (templates, RAII, `std::`
básico), Linux por linha de comando, noção de thread. **Não se pressupõe** e
o material ensina: hierarquia de cache, TLB, modelo de memória do C++,
atomics e ordering, alocadores, layout de memória, percentis e como medi-los.

---

## 8. Reprodutibilidade — o que cada medição registra

Todo arquivo em `bench/medicoes/` vem acompanhado de `metadata.json` gerado
pelo harness:

- saída completa de `scripts/ambiente.sh` (CPU, topologia, domínios de L3,
  governor, turbo, mitigações, kernel e linha de comando, hugepages);
- compilador, versão, flags efetivas, perfil de build, hash do commit;
- parâmetros do experimento: tamanho, cardinalidade, repetições, warm-up,
  núcleo(s) usados, se houve isolamento;
- precisão do histograma e faixa configurada;
- data e duração.

Publica-se mediana, p75, p99, p99,9 e máximo, com o número de amostras. Média
aparece só quando o texto explica por que ela importa naquele caso.

---

## 9. `CLAUDE.md` — o papel do assistente no repositório

Herda a estrutura do DPDK Academy (postura cética, medir antes de afirmar,
separar correção/segurança/desempenho, padrões só quando resolvem problema
real). O que muda é o foco da seção *project-specific*:

- o par `std` → `custom` é a unidade de trabalho; nunca sugerir `custom` sem
  o `std` medido primeiro;
- toda proposta de estrutura declara o contrato antes do código;
- toda afirmação de desempenho vem com o cenário, a carga, o percentil e a
  máquina;
- a **regra de decisão** é entregável, não comentário;
- o modelo de memória do C++ é citado por cláusula do padrão quando o texto
  afirma algo sobre ordering;
- cenário de mercado financeiro é ilustração; nenhuma lógica de estratégia
  entra no repositório.

---

## 10. Roadmap

| Etapa | Entrega | Critério de pronto |
|---|---|---|
| 0 — esqueleto | estrutura, build, CI verde nos dois compiladores, scripts, verificadores herdados, `docs/00-visao-geral/` | `test-all.sh` passa em máquina limpa; paridade PT/EN verificada |
| 1 — harness | `ferramental/bench/` com HDR, metadata e saída Markdown; módulo 08.01 | um benchmark trivial produz tabela publicável e `metadata.json` |
| 2 — memória e layout | módulos 01 e 02 completos | cada tópico com `std/`, `custom/`, L1 parametrizado, medição arquivada, regra de decisão |
| 3 — contêineres e livro | módulos 03 e 04 | idem; `docs/regras-de-decisao.md` iniciado |
| 4 — filas e tempo | módulos 05 e 06 | TSan limpo; teste negativo em cada invariante de fila |
| 5 — parsing e medição | módulos 07 e 08.02 | inclui o resultado em que a STL basta |
| 6 — capstone | motor de livro em duas versões, feed sintético, comparação ponta a ponta | tabela atribuindo a diferença total a cada tópico |
| 7 — extração | `ferramental/bench/` vira repositório próprio, consumido por este, pelo DPDK Academy e pelo Messaging Academy | os três compilam contra a mesma versão fixada |

A etapa 7 é o ponto em que os três repositórios passam a compartilhar
instrumento de medição — e, com isso, a comparabilidade de números entre eles.

---

## 11. Primeiros comandos (Etapa 0)

Ordem sugerida para o bootstrap com Claude Code, cada passo com commit
assinado:

1. `git init`, `LICENSE` (MIT), `.gitattributes`, `.gitignore`, `.clang-format`
   e `.clang-tidy` copiados do DPDK Academy.
2. `meson.build` raiz com `cpp_std=c++23`, perfis `debugoptimized`, `release`
   e `sanitize`; `meson.options` para pinning e repetições padrão.
3. `subprojects/gtest.wrap` (copiado) e `subprojects/hdr_histogram.wrap`
   (versão e hash fixados no momento da criação); `.wraplock`.
4. `scripts/check-env.sh`, `ambiente.sh`, `build-all.sh`, `test-all.sh` —
   adaptados: sem DPDK, sem NIC, com detecção de `perf`, governor e turbo.
5. `ferramental/qualidade/` — verificadores de âncora, autodescrição e idioma,
   portados; suíte `l1+docs` registrada no Meson.
6. `.github/workflows/ci.yml` com matriz GCC 14 / Clang 18, ação fixada por SHA.
7. `docs/00-visao-geral/README.md` + `.en.md` — seções 1, 2, 3 e 4 deste
   documento reescritas para o leitor.
8. `README.md`, `README.en.md`, `ROADMAP.md`, `ROADMAP.en.md`, `CLAUDE.md`,
   `SECURITY.md`, `CITATION.cff`.
9. Um tópico mínimo em `trilha/08-medicao/01-harness/` que compila, roda e
   arquiva uma medição trivial — para provar o ciclo inteiro antes de escrever
   conteúdo.
10. Repositório remoto, `description` e `topics`, proteção de `main` com
    assinatura verificada.

O critério de "esqueleto pronto" é o mesmo do DPDK Academy: nenhum documento
publicado como esqueleto sem se declarar esqueleto na primeira seção.
