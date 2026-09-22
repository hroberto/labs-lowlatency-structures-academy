> 🇧🇷 Português | [🇺🇸 English](referencias.en.md)

# Catálogo de referências

Bibliografia central do **Low-Latency Structures Academy**. Os identificadores entre
colchetes são **estáveis**: não mudam se os documentos forem reorganizados, e é
por eles que os módulos citam.

Como citar, de dentro de um módulo:

```markdown
[INTEL-ORM](../../docs/referencias.md#intel-orm)
```

A âncora vem do **título da entrada**, não de uma tag `<a id>`. O GitHub gera o
identificador de uma seção pelo github-slugger, de modo que `### INTEL-ORM`
produz `#intel-orm` sozinho — e é isso que
[`verificar-links.py`](../ferramental/qualidade/) confere. A versão inicial do
padrão deste projeto exigia também uma tag `<a id>` explícita; ela foi removida
porque era **decoração não verificada**: nada checava se a tag e o título
concordavam, e dois identificadores para a mesma seção divergem em silêncio.

## Estado da verificação dos links

> **Nenhum endereço deste catálogo foi aberto e conferido ainda.** As entradas
> trazem autor, título, veículo e ano, que são os metadados de que a citação
> precisa; os endereços foram escritos de memória editorial e estão **pendentes
> de conferência**. Onde o identificador canônico é um DOI, o campo aparece como
> `DOI: pendente` — este projeto não inventa DOI, porque um DOI errado é pior
> que um DOI ausente: ele parece verificável.
>
> Conferir os endereços e preencher os DOIs é tarefa aberta, registrada no
> [ROADMAP.md](../ROADMAP.md).

## Níveis das fontes

| Nível | O que é | Papel |
|---|---|---|
| **A** | normativa ou primária | precedência máxima quando define diretamente o comportamento estudado |
| **B** | literatura acadêmica revisada | mecanismo e método |
| **C** | literatura técnica reconhecida | explicação e interpretação |
| **D** | material complementar | apoio; nunca substitui A quando A existe |

---

# Arquitetura de CPU

### INTEL-SDM

**Intel 64 and IA-32 Architectures Software Developer's Manual** — Nível A

Arquitetura x86, conjunto de instruções, arquitetura de memória, contadores de
desempenho, programação de sistema.

- Versão consultada: **a declarar no primeiro módulo que a citar** (§referências versionadas)
- Endereço: `https://www.intel.com/sdm` — pendente de conferência

### INTEL-ORM

**Intel 64 and IA-32 Architectures Optimization Reference Manual** — Nível A

Microarquitetura, pipeline, cache, prefetch, SIMD, predição de desvio.

- Versão consultada: a declarar
- Endereço: pendente de conferência

### AMD-OPT

**AMD Software Optimization Guides** — Nível A

Microarquitetura Zen, cache, execução, prefetch, SIMD, monitoramento de
desempenho.

Selecionar **sempre** o documento correspondente à geração efetivamente usada no
experimento. "Documentação da AMD" sem geração não identifica autoridade.

- Versão consultada: a declarar por geração
- Endereço: pendente de conferência

### ARM-ARM

**Arm Architecture Reference Manual** — Nível A

Arquitetura AArch64, modelo de memória, ordenação, barreiras.

Está no catálogo porque **arm64 está no escopo declarado deste projeto**. A
primeira versão do padrão listava apenas Intel e AMD enquanto o projeto irmão
declarava suporte a arm64 — divergência entre promessa e bibliografia.

- Endereço: pendente de conferência

### ARM-NEOVERSE

**Arm Neoverse Software Optimization Guides** — Nível A

Otimização por núcleo Neoverse; equivalente do INTEL-ORM e do AMD-OPT para os
alvos arm64 de servidor.

- Endereço: pendente de conferência

---

# C++: padrão e evolução

### CPP-STD

**ISO/IEC 14882 — Programming Languages — C++** — Nível A

Fonte normativa para tempo de vida de objeto, modelo de memória, atomics,
sequenciamento, comportamento indefinido, semântica da linguagem.

Este projeto declara **C++23** como alvo. Ver a seção de toolchain do
[padrão do projeto](padrao-do-projeto.md).

- Endereço: pendente de conferência

### WG21

**ISO C++ WG21 Papers** — Nível A (com ressalva)

Origem de recursos, racional de projeto, alterações no modelo de memória,
propostas para C++23 e C++26.

> **Ressalva obrigatória:** um paper do WG21 **não** deve ser interpretado
> automaticamente como recurso incorporado ao padrão. É necessário verificar o
> status — e, neste projeto, verificar também o suporte no toolchain, porque
> "adotado no padrão" e "disponível no compilador" são coisas diferentes.

- Endereço: `https://wg21.link/` — pendente de conferência

### P2900

**Contracts for C++** — Nível A (proposta)

`pre`, `post` e `contract_assert`. Adotado para **C++26**; **não existe em
C++23**.

> **Medido nesta máquina em 16/09/2026:** GCC 15.2.0 rejeita `int f(int x)
> pre(x>0)` mesmo com `-std=c++2c -fcontracts`, com erro `expected initializer
> before 'pre'`. É por isso que o mecanismo de contrato deste projeto usa
> `[[assume]]` de C++23 atrás de macro, e não Contracts: não é preferência de
> estilo, é o que o toolchain oferece. Ver a seção das três portas de contrato
> no [padrão do projeto](padrao-do-projeto.md).

- Endereço: `https://wg21.link/p2900` — pendente de conferência

### P1774

**Portable assumptions** — Nível A

Origem de `[[assume]]`, incorporado em **C++23**.

> **Medido nesta máquina em 16/09/2026:** `[[assume(x > 0)]]` compila em GCC
> 15.2.0 e em Clang 21.1.8 com `-std=c++23 -Wall -Wextra`.

- Endereço: `https://wg21.link/p1774` — pendente de conferência

### P1938

**`if consteval`** — Nível A

Incorporado em C++23. Permite ao predicado de contrato ter comportamento
distinto em avaliação constante e em runtime.

> **Medido nesta máquina em 16/09/2026:** compila em GCC 15.2.0 com `-std=c++23`.

- Endereço: `https://wg21.link/p1938` — pendente de conferência

### CPPREFERENCE

**cppreference** — Nível C

Excelente referência operacional e índice para recursos da linguagem e da
biblioteca.

> `cppreference != ISO C++ Standard`. Em discussão normativa, CPP-STD ou WG21
> têm precedência.

- Endereço: `https://en.cppreference.com/` — pendente de conferência

### CPP-ASSERT

**`assert` — `<cassert>` / `<assert.h>`** — Nível A (norma) e C (referência operacional)

A macro de asserção, desligada pela definição de `NDEBUG`. **É a origem do
desenho da porta R deste projeto**: `PERF_EXPECTS` é `assert` mais `[[assume]]`,
e herda dele o gatilho `NDEBUG` e o comportamento em avaliação constante.

> **Medido nesta máquina em 16/09/2026, GCC 15.2.0, `-O2`:**
>
> | configuração | `assert` sozinho | `assert` + `[[assume]]` |
> |---|---|---|
> | sem `NDEBUG` | verificado; caminho morto eliminado | idem |
> | com `NDEBUG` | **não** verificado; caminho morto **presente** | não verificado; eliminado |
>
> Com `NDEBUG`, `assert` expande para `((void) 0)` e não sobra nem a
> verificação nem a informação — a condicional redundante volta ao binário de
> release, que é exatamente o que a segunda diretriz do projeto quer eliminar.
> Por isso o mecanismo tem as duas metades.
>
> Também conferido: `assert` é macro de **um** argumento e recusa vírgula
> dentro de `<>` (`macro 'assert' passed 2 arguments, but takes just 1`), o que
> é a razão de `PERF_EXPECTS` ser variádico; e um `assert` falso em função
> `constexpr` já quebra o build, por alcançar `__assert_fail`, que não é
> `constexpr`.

- Norma: [CPP-STD](#cpp-std), cláusula sobre `<cassert>`
- Referência operacional: `https://en.cppreference.com/w/cpp/error/assert` — pendente de conferência

---

# Compiladores

### GCC-DOC

**GCC documentation** — Nível A

Opções de otimização, atributos, builtins, comportamento dependente de
implementação.

A primeira versão do padrão citava "GCC" como autoridade na tabela de
referências por módulo sem lhe dar identificador. Esta entrada corrige isso.

- Versão: **GCC 15.2.0** na máquina de referência
- Endereço: `https://gcc.gnu.org/onlinedocs/` — pendente de conferência

### LLVM-DOC

**LLVM documentation** — Nível A

Representação intermediária, passes de otimização, modelo de custo.

- Endereço: pendente de conferência

### CLANG-DOC

**Clang documentation** — Nível A

Opções, atributos, diagnósticos, extensões.

- Versão: **Clang 21.1.8** na máquina de referência
- Endereço: pendente de conferência

### LLVM-MCA

**llvm-mca — Machine Code Analyzer** — Nível A

Análise estática de vazão e de porta de execução sobre código já gerado.
Instrumento para a tese de compile-time: permite comparar duas gerações de
código sem executar nenhuma delas.

- Endereço: pendente de conferência

### UICA

Abel, A.; Reineke, J. — **uiCA: Accurate Throughput Prediction of Basic Blocks
on Recent Intel Microarchitectures** — Nível B

Predição de vazão de bloco básico. Complementa e frequentemente corrige o
llvm-mca em microarquiteturas recentes.

- DOI: pendente

---

# Sistema operacional e Linux

### LINUX-DOC

**Linux Kernel Documentation** — Nível A

Autoridade para os domínios dos parâmetros ajustáveis: `vm.*`, `net.*`,
escalonador, writeback, hugepages, NUMA.

> **Autoridade obrigatória para qualquer domínio que descreva um ajuste do
> kernel**, e **com a versão do kernel**: os limites mudaram entre versões, e
> `vm.swappiness` é o caso conhecido. Domínios desse tipo pertencem à etapa
> final (ver [EX442-LABS](#ex442-labs)); a biblioteca de contratos da fundação
> modela os domínios da própria régua de apuração.

- Endereço: `https://www.kernel.org/doc/html/latest/` — pendente de conferência

### CGROUP-V2

**Control Group v2 — Linux Kernel Documentation** — Nível A

Interface unificada, `cpu.weight`, `cpu.max`, `memory.max`, `io.weight`,
`cpuset`.

Autoridade direta dos domínios de cgroup que o EX442 exercita.

- Endereço: pendente de conferência

### LINUX-PERF

**Linux perf / perf_events documentation** — Nível A

Contadores de hardware e de software, amostragem, PMU, `perf stat`,
`perf record`, `perf report`.

- Endereço: pendente de conferência

### PERF-EVENT-OPEN

**`perf_event_open(2)`** — Nível A

`perf` não é apenas uma ferramenta externa: existe um subsistema do kernel
expondo eventos de desempenho, e a cadeia é

```text
PMU da CPU → perf_event do Linux → perf(1) → medição
```

Importa aqui porque o projeto final lê contadores **pela chamada**, sem depender
do `perf(1)` no caminho.

> **Advertência operacional que a primeira versão do padrão não trazia:** o
> acesso ao PMU depende de `perf_event_paranoid` e frequentemente **não existe**
> em máquina virtual ou contêiner. Documento que publica contador de hardware
> precisa declarar que o PMU estava disponível.

- Endereço: `https://man7.org/linux/man-pages/man2/perf_event_open.2.html` — pendente de conferência

### PMU-TOOLS

Kleen, A. — **pmu-tools / toplev** — Nível C

Implementação da análise top-down sobre contadores do Linux. Instrumento de
YASIN-2014.

- Endereço: pendente de conferência

### POSIX

**IEEE Std 1003.1 — POSIX** — Nível A

Interfaces do sistema usadas fora do padrão C++: memória compartilhada,
mapeamento, afinidade, sinais.

Também sem identificador na primeira versão do padrão, apesar de citada na
tabela por módulo.

- Endereço: pendente de conferência

---

# Engenharia de desempenho

### AGNER-CPP

Fog, A. — **Optimizing Software in C++** — Nível C

Otimização em C++, comportamento do compilador, CPU, SIMD, desvios, estruturas
de dados, geração de código.

- Endereço: `https://www.agner.org/optimize/` — pendente de conferência

### AGNER-INST

Fog, A. — **Instruction Tables** — Nível C

Latência, vazão e portas de execução por instrução e por microarquitetura.

Entrada **distinta** de AGNER-CPP: são manuais diferentes, e a primeira versão
do padrão tratava o autor como uma referência só.

- Endereço: pendente de conferência

### AGNER-UARCH

Fog, A. — **The Microarchitecture of Intel, AMD and VIA CPUs** — Nível C

Pipeline, execução fora de ordem, predição de desvio por microarquitetura.

- Endereço: pendente de conferência

### BRENDAN-GREGG-SP

Gregg, B. — **Systems Performance: Enterprise and the Cloud** — Nível C

Metodologia de desempenho, profiling, observabilidade, análise de CPU, latência,
método USE, desempenho no Linux.

- Endereço: pendente de conferência

### DREPPER-MEM

Drepper, U. — **What Every Programmer Should Know About Memory** — Nível C

Hierarquia de memória, cache, TLB, prefetch, NUMA, padrões de acesso.

- Endereço: pendente de conferência

### MCKENNEY-PERFBOOK

McKenney, P. E. — **Is Parallel Programming Hard, And, If So, What Can You Do
About It?** — Nível C

Custo de sincronização, ordenação de memória, RCU, escalabilidade. A Tabela de
custos de operação é ponto de confronto já usado pelo projeto irmão.

- Endereço: `https://arxiv.org/abs/1701.00854` — pendente de conferência

---

# Método de medição e benchmarking

### GOOGLE-BENCHMARK

**Google Benchmark — User Guide** — Nível D

Metodologia de microbenchmark, fixtures, temporização, iterações, contadores,
configuração.

> Google Benchmark **mede código**. Não substitui metodologia científica.

- Endereço: pendente de conferência

### KALIBERA-JONES-2013

Kalibera, T.; Jones, R. — **Rigorous Benchmarking in Reasonable Time** —
ISMM 2013 — Nível B

Tratamento rigoroso de repetição, aquecimento, autocorrelação e intervalo de
confiança em benchmark de software.

> **É a autoridade da régua de apuração deste projeto.** O
> `lib/measurement/statistics.hpp` declara explicitamente que **não** faz
> inferência formal — não há intervalo de confiança nem teste de hipótese,
> porque as amostras de microbenchmark não são independentes nem normalmente
> distribuídas. Esta referência é o que o leitor deve procurar quando precisar
> do tratamento que aquele arquivo declina fazer.

- DOI: pendente

### YASIN-2014

Yasin, A. — **A Top-Down Method for Performance Analysis and Counters
Architecture** — ISPASS 2014 — Nível B

Base da Top-down Microarchitecture Analysis (TMA): atribuição de slots de
emissão a retirada útil, mau especulação, limite de front-end e limite de
back-end.

> **É o método que falta para a seção "Hardware counters" fazer sentido.** Sem
> ele, contador de hardware vira lista de números sem modelo de atribuição.

- DOI: pendente

---

# Concorrência e correção

### HERLIHY-WING-1990

Herlihy, M.; Wing, J. — **Linearizability: A Correctness Condition for
Concurrent Objects** — ACM TOPLAS, 1990 — Nível B

Referência fundamental para estruturas concorrentes, programação lock-free,
filas, correção, linearizabilidade.

> Desempenho nunca substitui correção. Uma fila extremamente rápida e incorreta
> não constitui otimização válida.

- DOI: pendente

### MICHAEL-SCOTT-1996

Michael, M. M.; Scott, M. L. — **Simple, Fast, and Practical Non-Blocking and
Blocking Concurrent Queue Algorithms** — PODC 1996 — Nível B

Algoritmo canônico de fila concorrente; par obrigatório de HERLIHY-WING-1990 no
módulo lock-free.

- DOI: pendente

### BOEHM-ADVE-2008

Boehm, H.-J.; Adve, S. V. — **Foundations of the C++ Concurrency Memory Model** —
PLDI 2008 — Nível B

Racional do modelo de memória de C++. Os módulos de atomics e de ordenação de
memória apontavam apenas para CPP-STD e WG21, que dizem **o que** o modelo é e
não **por que** ele é assim.

- DOI: pendente

### WILLIAMS-CCIA

Williams, A. — **C++ Concurrency in Action** — Nível C

Uso prático de atomics, ordenação, primitivas e padrões concorrentes em C++
moderno. Citado como nome na primeira versão do padrão, sem identificador.

- Endereço: pendente de conferência

---

# Alocação de memória

### JEMALLOC

**jemalloc** — Nível D

Arenas, classes de tamanho, cache por thread, fragmentação.

### TCMALLOC

**TCMalloc** — Nível D

Cache por thread, transferência em lote, páginas.

### MIMALLOC

**mimalloc** — Nível D

Listas livres fragmentadas, localidade, comportamento em carga concorrente.

> Estas três entram porque o módulo de alocação apontava apenas para CPP-STD e
> AGNER-CPP — e o **mecanismo** de um alocador moderno não está em nenhum dos
> dois. São Nível D: descrevem implementações, não normas.

---

# Projetos irmãos

### DPDK-ACADEMY

**labs-dpdk-academy** — Roberto, H. M.

`https://github.com/hroberto/labs-dpdk-academy`

Estudo de arquitetura de alta volumetria e baixa latência com DPDK. **É a
autoridade deste projeto para custo de mecanismo de hardware e de sistema
operacional já medido**: orçamento por unidade de trabalho, fronteira
usuário/kernel, tradução de endereço e TLB, hugepages, cache e localidade, falso
compartilhamento, NUMA e política de primeiro toque, SMT, polling versus dormir,
DMA e descritores, IOMMU, orçamento do barramento.

Ver a divisão de trabalho declarada no
[padrão do projeto](padrao-do-projeto.md). Este projeto **cita e estende**; não
reensina.

### EX442-LABS

**labs-EX442** — Roberto, H. M.

Laboratório de ajuste de desempenho no RHEL 10: `sysctl`, `sysfs`, TuneD,
cgroups, prioridade e política de escalonamento, hugepages, overcommit,
swappiness, NUMA, SYSV shm, escalonador de I/O, writeback, buffers de rede por
BDP, PCP, eBPF.

> **Esta referência só é citável na etapa final.** O eixo de ajuste do sistema
> operacional não entra na fundação nem nos módulos conceituais: ele aparece
> numa etapa própria, ao fim, na forma de um plano de integração entre este
> projeto e a otimização de baixo nível. A razão é de ensino — misturar os dois
> eixos faz estudar ajuste de kernel no lugar de modelo de objetos, layout e
> geração de código. Ver a seção 30 do
> [padrão do projeto](padrao-do-projeto.md).

O ciclo de experimento adotado aqui — baseline, hipótese, uma alteração, nova
medição, verificação — vem dele, e esse sim vale desde o começo.

---

## Navegação

- [Padrão do projeto](padrao-do-projeto.md)
- [Índice da documentação](README.md)
- [README do repositório](../README.md)

> 🇧🇷 Português | [🇺🇸 English](referencias.en.md)
