# Padrão de documentação, internacionalização e referências

Esta seção define regras obrigatórias para toda a documentação do **C++ Performance Academy**.

O objetivo é manter compatibilidade conceitual e editorial com o **DPDK Academy**, evitando que os dois projetos evoluam com padrões diferentes de rigor técnico.

---

## 1. Idiomas oficiais

Toda documentação acadêmica do projeto deverá existir em:

1. **Português do Brasil — PT-BR**
2. **English — EN**

O português brasileiro será utilizado como idioma principal de desenvolvimento do material acadêmico.

A versão em inglês deverá preservar:

- profundidade técnica;
- exemplos;
- tabelas;
- referências;
- conclusões;
- limitações;
- resultados experimentais.

A versão inglesa não deverá ser uma versão resumida.

Ela deve representar o mesmo conteúdo técnico.

---

## 2. Convenção de arquivos

A convenção recomendada é:

```text
README.md
README.en.md
```

Em cada módulo:

```text
trilha/
└── 03-cache-memory/
    ├── README.md
    ├── README.en.md
    ├── src/
    ├── tests/
    ├── benchmarks/
    └── results/
```

Portanto:

```text
README.md
```

representa:

```text
Português — Brasil
```

e:

```text
README.en.md
```

representa:

```text
English
```

---

# 3. Código permanece em inglês

Assim como no DPDK Academy, a documentação pode estar em português, mas o código deve utilizar inglês.

Exemplo:

```cpp
class MemoryPool;
class EventBuffer;

void process_event();
void measure_latency();
```

Evitar:

```cpp
class PoolMemoria;

void processar_evento();
```

A mesma regra vale para:

```text
classes
functions
variables
namespaces
tests
benchmark names
directories técnicas
log messages
commit messages
CI identifiers
```

O objetivo é manter o código compatível com o vocabulário utilizado pela literatura, ferramentas e comunidade internacional de C++.

---

# 4. Navegação entre idiomas

Todo documento deve possuir no início uma navegação equivalente a:

```markdown
> 🇧🇷 Português | [🇺🇸 English](README.en.md)
```

No documento inglês:

```markdown
> [🇧🇷 Português](README.md) | 🇺🇸 English
```

Isso vale tanto para o README principal quanto para os documentos da trilha.

---

# 5. Método acadêmico

Cada tópico deverá seguir, sempre que aplicável, o mesmo percurso metodológico:

```text
Problema
   ↓
Mecanismo
   ↓
Trade-offs
   ↓
Implementação
   ↓
Medição
   ↓
Confronto com literatura
   ↓
Análise
   ↓
Decisão de arquitetura
```

Em inglês:

```text
Problem
   ↓
Mechanism
   ↓
Trade-offs
   ↓
Implementation
   ↓
Measurement
   ↓
Literature comparison
   ↓
Analysis
   ↓
Architecture decision
```

Esse fluxo é central para o projeto.

Não queremos:

```text
"usar técnica X porque ela é rápida"
```

Queremos:

```text
problema
   ↓
hipótese
   ↓
implementação
   ↓
medição
   ↓
explicação
   ↓
literatura
   ↓
conclusão
```

---

# 6. Classes de afirmação

Todo conteúdo técnico deve permitir distinguir quatro categorias.

## 6.1 Resultado medido

Exemplo:

```text
Nesta máquina, a implementação SoA apresentou
18% menos ciclos por elemento.
```

Essa afirmação precisa estar associada a:

```text
benchmark
hardware
compiler
flags
dataset
número de execuções
resultado bruto
```

---

## 6.2 Fato documentado

Exemplo:

```text
O processador transfere dados entre determinados
níveis da hierarquia de memória em unidades de cache line.
```

Quando relevante, essa afirmação deverá apontar para documentação reconhecida do fabricante ou literatura especializada.

---

## 6.3 Inferência

Exemplo:

```text
A redução observada em cache misses provavelmente explica
parte do ganho de throughput.
```

Isso deve ser apresentado como análise, não como fato comprovado.

---

## 6.4 Questão em aberto

Exemplo:

```text
A diferença foi observada, mas ainda não foi possível
determinar se é causada pelo hardware prefetcher ou pela
estratégia de code generation do compilador.
```

O projeto deve permitir dizer:

> Não sabemos ainda.

Isso é preferível a inventar uma explicação.

---

# 7. Regra para números

Uma afirmação quantitativa produzida pelo projeto deve possuir um experimento reproduzível.

Não escrever:

```text
Acesso à memória custa 100 ns.
```

Preferir:

```text
No ambiente de referência e neste experimento,
o acesso medido apresentou mediana de X ns.
```

e apontar para:

```text
benchmark
script
resultado bruto
hardware
metodologia
```

Regra:

> **Número produzido pelo projeto precisa de programa que produza o número.**

---

# 8. Regra para afirmações normativas

Toda afirmação normativa precisa identificar a autoridade correspondente.

Exemplo ruim:

```text
Uma cache line tem sempre 64 bytes.
```

Exemplo melhor:

```text
Na microarquitetura utilizada pelo ambiente de referência,
a documentação do fabricante descreve cache lines de 64 bytes.
```

Isso evita transformar características de determinado hardware em regras universais.

---

# 9. Hierarquia das fontes

As fontes serão classificadas em níveis.

## Nível A — Fonte normativa ou primária

Preferência máxima.

Exemplos:

```text
ISO C++ Standard
WG21 papers

Intel Software Developer Manual
Intel Optimization Reference Manual

AMD Processor Programming Reference
AMD Software Optimization Guide

Linux Kernel Documentation

GCC documentation
LLVM documentation
Clang documentation

POSIX
IEEE
RFC
```

Essas fontes devem ser utilizadas sempre que definirem diretamente o comportamento estudado.

---

## Nível B — Literatura acadêmica

Exemplos:

```text
ACM
IEEE
USENIX
SOSP
OSDI
ASPLOS
PLDI
PPoPP
ISCA
MICRO
```

Papers acadêmicos devem preferencialmente possuir:

```text
autor
título
evento ou periódico
ano
DOI
```

---

## Nível C — Literatura técnica reconhecida

Exemplos:

```text
Agner Fog
Brendan Gregg
Martin Thompson
Herb Sutter
Anthony Williams
Maurice Herlihy
Scott Meyers
Chandler Carruth
Andrei Alexandrescu
```

Livros e materiais técnicos reconhecidos são particularmente úteis para explicação e interpretação.

---

## Nível D — Material complementar

Pode incluir:

```text
conference talks
engineering blogs
vendor engineering articles
technical presentations
```

Essas fontes podem complementar a literatura principal.

Não devem substituir uma fonte normativa quando ela existir.

---

# 10. Fontes que não devem sustentar afirmações técnicas importantes

Evitar como fonte principal:

```text
Wikipedia
Stack Overflow
posts de redes sociais
blogs sem autoria técnica identificável
conteúdo gerado por IA
artigos SEO
benchmarks sem metodologia
```

Esses materiais podem ajudar a localizar um assunto.

Não devem ser utilizados como autoridade final.

---

# 11. Catálogo central de referências

O projeto deverá possuir:

```text
docs/
├── referencias.md
└── references.en.md
```

`referencias.md` será o catálogo PT-BR.

`references.en.md` será a versão inglesa.

As referências devem receber identificadores estáveis.

Exemplo:

```text
[INTEL-SDM]
[INTEL-ORM]
[AMD-OPT]
[CPP-STD]
[WG21]
[LINUX-PERF]
[GOOGLE-BENCHMARK]
[AGNER-CPP]
[HERLIHY-WING-1990]
[BRENDAN-GREGG-SP]
```

Esses identificadores não devem mudar caso os documentos sejam reorganizados.

---

# 12. Âncoras

Cada referência deverá possuir uma âncora Markdown previsível.

Exemplo:

```markdown
<a id="intel-orm"></a>

### INTEL-ORM

Intel — Intel 64 and IA-32 Architectures Optimization Reference Manual.
```

Permitindo:

```markdown
[INTEL-ORM](../../docs/referencias.md#intel-orm)
```

Na versão inglesa:

```markdown
[INTEL-ORM](../../docs/references.en.md#intel-orm)
```

---

# 13. Referências locais por módulo

Cada módulo também deve terminar com:

```markdown
## Referências

- [INTEL-ORM]
- [AGNER-CPP]
- [LINUX-PERF]
```

Isso permite saber imediatamente quais fontes sustentam aquele capítulo.

A bibliografia central continua sendo a fonte completa dos metadados.

---

# 14. Referências fundamentais iniciais

O projeto deverá começar com pelo menos o seguinte conjunto.

## Arquitetura de CPU

### INTEL-SDM

**Intel 64 and IA-32 Architectures Software Developer's Manual**

Utilização:

```text
arquitetura x86
instructions
memory architecture
performance counters
system programming
```

---

### INTEL-ORM

**Intel 64 and IA-32 Architectures Optimization Reference Manual**

Utilização:

```text
microarchitecture
pipeline
cache
prefetch
SIMD
branch prediction
optimization
```

---

### AMD-OPT

**AMD Software Optimization Guides**

Utilização:

```text
Zen microarchitecture
cache
execution
prefetch
SIMD
performance monitoring
```

Sempre selecionar o documento correspondente à geração efetivamente utilizada no experimento.

---

# 15. C++ e otimização

## CPP-STD

**ISO/IEC 14882 — Programming Languages — C++**

Fonte normativa para:

```text
object lifetime
memory model
atomics
sequencing
undefined behavior
language semantics
```

---

## WG21

**ISO C++ WG21 Papers**

Utilização para compreender:

```text
origem de recursos
design rationale
alterações no memory model
C++23
C++26
proposals
```

WG21 paper não deve automaticamente ser interpretado como recurso incorporado ao padrão.

É necessário verificar seu status.

---

## CPPREFERENCE

**cppreference**

Pode ser utilizada como excelente referência operacional e índice para recursos C++.

Porém:

```text
cppreference != ISO C++ Standard
```

Quando houver discussão normativa, o padrão ou WG21 deve possuir precedência.

---

# 16. Performance Engineering

## AGNER-CPP

**Agner Fog — Optimizing Software in C++**

Referência importante para:

```text
C++ optimization
compiler behavior
CPU
SIMD
branching
data structures
code generation
```

---

## BRENDAN-GREGG-SP

**Brendan Gregg — Systems Performance**

Referência importante para:

```text
performance methodology
profiling
observability
CPU analysis
latency
USE method
Linux performance
```

---

# 17. Benchmarking

## GOOGLE-BENCHMARK

**Google Benchmark — User Guide**

Referência para:

```text
microbenchmark methodology
fixtures
timing
iterations
counters
benchmark configuration
```

Google Benchmark mede código.

Ele não substitui metodologia científica.

---

# 18. Linux Performance

## LINUX-PERF

**Linux perf / perf_events documentation**

Referência para:

```text
hardware counters
software counters
sampling
PMU
perf stat
perf record
perf report
```

---

## PERF-EVENT-OPEN

**Linux `perf_event_open()`**

Importante para compreender que `perf` não é apenas uma ferramenta externa.

Existe um subsistema do kernel expondo eventos de performance através de:

```text
perf_event_open()
```

permitindo estudar a relação:

```text
CPU PMU
   ↓
Linux perf_event
   ↓
perf
   ↓
measurement
```

---

# 19. Concorrência

## HERLIHY-WING-1990

Maurice Herlihy e Jeannette Wing:

**Linearizability: A Correctness Condition for Concurrent Objects**

Deve ser referência fundamental no módulo de:

```text
concurrent structures
lock-free programming
queues
correctness
linearizability
```

Performance nunca deve substituir correção.

Uma fila extremamente rápida e incorreta não constitui uma otimização válida.

---

# 20. Relação referência ↔ módulo

O projeto deverá manter um mapa como:

| Módulo | Referências principais |
|---|---|
| Benchmarking | GOOGLE-BENCHMARK, LINUX-PERF, BRENDAN-GREGG-SP |
| CPU Architecture | INTEL-SDM, INTEL-ORM, AMD-OPT |
| Cache | INTEL-ORM, AMD-OPT, AGNER-CPP |
| Memory Layout | INTEL-ORM, AMD-OPT, AGNER-CPP |
| Allocation | CPP-STD, AGNER-CPP |
| Zero-Copy | CPP-STD, POSIX, Linux documentation |
| Branch Prediction | INTEL-ORM, AMD-OPT, AGNER-CPP |
| Prefetch | INTEL-ORM, AMD-OPT, AGNER-CPP |
| Compiler Optimization | GCC, LLVM, AGNER-CPP |
| SIMD | INTEL-SDM, INTEL-ORM, AMD-OPT |
| Atomics | CPP-STD, WG21 |
| Memory Ordering | CPP-STD, WG21 |
| Lock-Free | CPP-STD, HERLIHY-WING-1990 |
| NUMA | Linux Kernel, Intel/AMD documentation |
| Profiling | LINUX-PERF, BRENDAN-GREGG-SP |
| Microarchitecture | INTEL-ORM, AMD-OPT |

---

# 21. Estrutura padrão de cada capítulo

Cada documento acadêmico deverá preferencialmente utilizar:

```markdown
# Título

## Objetivos

## Problema

## Modelo mental

## Fundamentos

## Como funciona

## Implementação baseline

## Experimento

## Ambiente de execução

## Resultados

## Hardware counters

## Análise

## Confronto com a literatura

## Trade-offs

## Quando utilizar

## Quando não utilizar

## Limitações

## Exercícios

## Referências

## Navegação
```

A versão inglesa deve possuir exatamente a mesma estrutura conceitual:

```markdown
# Title

## Objectives

## Problem

## Mental model

## Fundamentals

## How it works

## Baseline implementation

## Experiment

## Execution environment

## Results

## Hardware counters

## Analysis

## Literature comparison

## Trade-offs

## When to use

## When not to use

## Limitations

## Exercises

## References

## Navigation
```

---

# 22. Seção obrigatória: confronto com a literatura

Todo experimento relevante deverá possuir:

```markdown
## Confronto com a literatura
```

ou:

```markdown
## Literature comparison
```

A pergunta será:

> O comportamento medido é compatível com o que as fontes técnicas descrevem?

Existem três resultados possíveis:

```text
medição confirma literatura

medição diverge da literatura
        ↓
investigar contexto

literatura não prevê diretamente o resultado
        ↓
registrar como observação experimental
```

A divergência não deve ser escondida.

Ela pode ser justamente a parte mais interessante do estudo.

---

# 23. Exemplo: cache

Suponha que a literatura sugira determinado comportamento ao ultrapassar o tamanho de um nível de cache.

O projeto não deve escrever simplesmente:

```text
Quando passamos do L2, a latência aumenta.
```

O processo correto é:

```text
documentação do processador
        ↓
identificar tamanho/topologia do cache
        ↓
criar benchmark
        ↓
variar working set
        ↓
medir cycles/op
        ↓
medir cache misses
        ↓
identificar transição
        ↓
comparar com documentação
```

---

# 24. Exemplo: zero-copy

Para zero-copy, não devemos partir do pressuposto:

```text
zero-copy é mais rápido.
```

Devemos investigar:

```text
copy
vs
view
vs
move
vs
buffer reuse
vs
scatter/gather
```

Medindo:

```text
latency
throughput
instructions
cycles
cache misses
memory bandwidth
allocations
```

E confrontando o resultado com:

```text
C++ object model
OS APIs
compiler behavior
CPU architecture
```

---

# 25. Referências versionadas

Sempre que uma referência depender de versão, o documento deve identificá-la.

Exemplo:

```text
GCC 15
Clang 21
Linux 6.x
Intel Optimization Manual revision X
AMD Zen 5 Optimization Guide revision Y
```

Evitar apenas:

```text
segundo a documentação da Intel
```

Preferir:

```text
segundo Intel 64 and IA-32 Architectures
Optimization Reference Manual, revision X
```

---

# 26. Links estáveis

Dar preferência a:

```text
documentação oficial
DOI
página permanente do fabricante
WG21 paper number
RFC
standard identifier
```

Evitar depender de:

```text
links de busca
URLs temporárias
mirrors desconhecidos
blogs que copiam documentação
```

---

# 27. Literatura versus experimento

O projeto não deve tentar provar que a literatura está sempre correta.

Também não deve assumir que uma divergência significa que a literatura está errada.

Uma divergência pode resultar de:

```text
hardware diferente
microarquitetura diferente
compiler diferente
flags diferentes
OS diferente
kernel diferente
SMT
turbo
governor
NUMA
dataset
benchmark incorreto
efeito não controlado
```

Por isso o processo deve ser:

```text
Literatura
     ↓
Hipótese
     ↓
Experimento
     ↓
Resultado
     ↓
Análise crítica
```

---

# 28. Reprodutibilidade

Cada resultado publicado deverá permitir responder:

```text
Em qual CPU?

Em qual kernel?

Qual compilador?

Qual versão?

Quais flags?

Quantas execuções?

Qual afinidade?

Qual NUMA node?

Turbo estava ativo?

SMT estava ativo?

Qual governor?

Qual tamanho do dataset?

Qual código gerou esse número?
```

Quando alguma variável não for conhecida, isso deve estar documentado.

---

# 29. Ambiente de referência

Assim como no DPDK Academy, deverá existir:

```text
scripts/ambiente.sh
```

ou equivalente.

Ele deverá registrar pelo menos:

```text
CPU model
microarchitecture quando identificável
cores
threads
SMT
NUMA topology
cache topology
RAM
kernel
governor
turbo
compiler
compiler version
Meson
Ninja
perf
```

Também poderá registrar:

```text
mitigations
hugepages
CPU isolation
frequency
thermal state
```

quando relevante.

---

# 30. Resultados negativos

O projeto deve publicar experimentos onde a otimização:

```text
não ajudou
```

ou:

```text
piorou a performance
```

Exemplo:

```text
manual prefetch
        ↓
+3.8% de cycles/op
        ↓
otimização rejeitada
```

Esse resultado possui valor acadêmico.

Ele mostra:

> A técnica não é universal; depende do mecanismo e da carga.

---

# 31. Política editorial

O C++ Performance Academy adotará as seguintes regras:

> **Mecanismo antes da otimização.**

> **Medição antes da conclusão.**

> **Correção antes da performance.**

> **Número sem experimento não é resultado.**

> **Afirmação normativa sem autoridade identificada não é referência.**

> **Benchmark sem ambiente documentado não é reproduzível.**

> **Diferença observada sem mecanismo conhecido deve permanecer explicitamente como hipótese.**

> **Otimização rejeitada também é conhecimento.**

---

# 32. Identidade acadêmica

A documentação deverá ser suficientemente profunda para conectar:

```text
C++ source
    ↓
language semantics
    ↓
compiler
    ↓
assembly
    ↓
microarchitecture
    ↓
cache / memory
    ↓
operating system
    ↓
measurement
```

O objetivo não é simplesmente ensinar:

> como escrever C++ rápido.

É ensinar:

> **como investigar cientificamente por que determinado software C++ apresenta determinado comportamento de performance.**

Essa filosofia deve permanecer consistente entre:

```text
DPDK Academy
C++ Performance Academy
Messaging Academy
```

formando uma família coerente de estudos sobre:

> **High Performance Systems Engineering**