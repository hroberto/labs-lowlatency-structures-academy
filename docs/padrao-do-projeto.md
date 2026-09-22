> 🇧🇷 Português | [🇺🇸 English](padrao-do-projeto.en.md)

# Padrão do projeto — C++ Performance Academy

Este documento define as regras obrigatórias de **método, medição, contrato,
arquitetura, documentação e referências** do C++ Performance Academy.

## O que este documento substitui

Ele é a versão normalizada de [`origem/setup-cpp-performance-academy.md`](origem/setup-cpp-performance-academy.md),
**preservado intacto** para que as divergências entre a intenção original e a
norma vigente sejam auditáveis. As correções aplicadas estão listadas na última
parte, em [Divergências em relação ao documento de origem](#divergências-em-relação-ao-documento-de-origem).

## Os três projetos a montante

| Projeto | O que este projeto toma dele |
|---|---|
| [DPDK-ACADEMY](referencias.md#dpdk-academy) | arquitetura de alta volumetria, custos de mecanismo já medidos, régua de apuração, ferramental de qualidade |
| [EX442-LABS](referencias.md#ex442-labs) | **apenas na etapa final**: a superfície de ajuste do sistema operacional, no plano de integração com otimização de baixo nível (seção 30) |
| este projeto | **linguagem ↔ compilador ↔ código gerado**, e a engenharia de software em volta |

---

# Parte I — Idioma e forma

## 1. Idiomas oficiais

Toda documentação acadêmica existe em **Português do Brasil (PT-BR)** e em
**English (EN)**. O português é o idioma principal de desenvolvimento do
material.

A versão inglesa preserva profundidade técnica, exemplos, tabelas, referências,
conclusões, limitações e resultados experimentais. **Não é versão resumida** —
representa o mesmo conteúdo técnico.

## 2. Convenção de arquivos

Par de idiomas com **o mesmo radical**:

```text
README.md        Português — Brasil
README.en.md     English
```

A regra do radical idêntico não é estética: é o que permite verificar
automaticamente que todo documento tem par. Um par `referencias.md` /
`references.en.md` — como a versão de origem especificava — quebra qualquer
verificador de pareamento, porque os dois arquivos não compartilham prefixo.
O catálogo, portanto, é [`referencias.md`](referencias.md) /
[`referencias.en.md`](referencias.en.md).

Em cada módulo:

```text
trilha/
└── 03-cache-e-memoria/
    ├── README.md
    ├── README.en.md
    ├── src/
    ├── tests/
    ├── benchmarks/
    └── resultados/
```

## 3. Idioma dos identificadores

**Documentação em português; código em inglês.** A mesma regra do projeto irmão.

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

Vale para classes, funções, variáveis, namespaces, testes, nomes de benchmark,
mensagens de log, mensagens de commit e identificadores de CI. O objetivo é
manter o código compatível com o vocabulário da literatura, das ferramentas e da
comunidade internacional de C++.

### Diretórios: a regra por camada

O documento de origem se contradizia aqui — o exemplo de estrutura usava
`trilha/03-cache-memory/`, em inglês, enquanto a regra de identificadores manda
diretório técnico em inglês e o projeto irmão usa português em toda a trilha.

A norma vigente distingue **diretório de currículo** de **diretório de código**:

| Camada | Idioma | Exemplos |
|---|---|---|
| currículo e documentação | português | `trilha/03-cache-e-memoria/`, `docs/`, `resultados/`, `medicoes/` |
| código e build | inglês | `src/`, `tests/`, `benchmarks/`, `lib/measurement/`, `lib/contract/` |

A razão é que o nome do diretório de currículo é **texto do material** — ele
aparece na navegação, na trilha e nos links do documento —, enquanto o nome do
diretório de código é **identificador**, sujeito à regra 3.

## 4. Navegação entre idiomas

Todo documento abre e fecha com a navegação. No documento português:

```markdown
> 🇧🇷 Português | [🇺🇸 English](README.en.md)
```

No inglês:

```markdown
> [🇧🇷 Português](README.md) | 🇺🇸 English
```

Vale para o README principal, para os documentos da trilha **e para os
documentos de `docs/`** — o documento de origem só previa READMEs, e por isso
ele mesmo, que institui o bilinguismo, existia só em português e sem navegação.

## 5. Rotulagem de estado

Todo documento e todo índice declara o estado do que descreve, com estas
etiquetas e sem eufemismo:

| Etiqueta | Significado |
|---|---|
| **não iniciado** | não existe conteúdo |
| **escrito** | documento existe; nenhum código |
| **ambiente medido, sem código** | o ambiente foi registrado; o experimento não existe |
| **código existe, não medido** | compila e roda; nenhum número publicado |
| **medido** | há número publicado com ambiente e programa que o produz |

> **Por que isto é norma e não bom costume.** No projeto irmão, um banner
> "conteúdo não escrito" sobreviveu num documento de 171 linhas medidas, e a
> correção foi transformar a regra em verificador
> (`verificar-autodescricao.py`). Um material que descreve a si mesmo
> incorretamente mente sobre a única coisa que o leitor não pode conferir sem
> ler tudo.

---

# Parte II — Método

## 6. O percurso metodológico

Cada tópico segue, sempre que aplicável:

```text
Problema → Mecanismo → Trade-offs → Implementação → Medição
        → Confronto com a literatura → Análise → Decisão de arquitetura
```

Não queremos `usar técnica X porque ela é rápida`. Queremos:

```text
problema → hipótese → implementação → medição → explicação
        → literatura → conclusão
```

## 7. Classes de afirmação, e a notação obrigatória

Todo conteúdo técnico permite distinguir quatro categorias. O documento de
origem as definia em prosa, sem notação — e sem marca sintática elas não são
localizáveis nem verificáveis. A norma vigente exige **marcador**:

```markdown
> **Medido.** Nesta máquina, a implementação SoA apresentou 18% menos ciclos
> por elemento. [benchmark, ambiente, n execuções]

> **Fato documentado.** O processador transfere dados entre determinados níveis
> da hierarquia de memória em unidades de cache line. [INTEL-ORM rev. X]

> **Inferência.** A redução observada em cache misses provavelmente explica
> parte do ganho de vazão.

> **Questão em aberto.** A diferença foi observada, mas ainda não foi possível
> determinar se é causada pelo prefetcher de hardware ou pela estratégia de
> geração de código do compilador.
```

O que cada classe exige:

| Classe | Exige |
|---|---|
| **Medido** | benchmark, hardware, compilador, flags, dataset, número de execuções, resultado bruto |
| **Fato documentado** | fonte de Nível A ou B, **com versão** |
| **Inferência** | nada além de ser apresentada como análise, nunca como fato |
| **Questão em aberto** | nada |

O projeto precisa poder dizer **não sabemos ainda**. Isso é preferível a
inventar explicação.

## 8. Regra para números

> **Número produzido pelo projeto precisa de programa que produza o número.**

Não escrever `Acesso à memória custa 100 ns`. Preferir:

```text
No ambiente de referência e neste experimento, o acesso medido apresentou
mediana de X ns.
```

apontando para benchmark, script, resultado bruto, hardware e metodologia.

## 9. Regra para afirmações normativas

Toda afirmação normativa identifica a autoridade correspondente.

Ruim: `Uma cache line tem sempre 64 bytes.`

Melhor: `Na microarquitetura usada pelo ambiente de referência, a documentação
do fabricante descreve cache lines de 64 bytes. [INTEL-ORM rev. X]`

Isso evita transformar característica de um hardware em regra universal.

## 10. Orçamento declarado

Otimização sem critério de parada não termina. O projeto irmão resolve isso
abrindo pela pergunta **"quanto tempo existe por unidade de trabalho"**, com o
número derivado de norma — quadro IEEE 802.3 mais intervalo entre quadros, na
taxa do enlace.

Aqui não há taxa de enlace, então o orçamento é **declarado**, e declarado como
constante de compilação:

```cpp
template <std::uint64_t EventsPerSecond>
struct workload_contract
{
    static_assert(EventsPerSecond > 0, "carga nula nao define orcamento");
    static constexpr double budget_ns = 1e9 / static_cast<double>(EventsPerSecond);
    static constexpr bool fits(double measured_ns) noexcept { return measured_ns <= budget_ns; }
};

using feed = workload_contract<5'000'000>;   // 200 ns por evento
static_assert(feed::budget_ns == 200.0);
static_assert(!feed::fits(240.0));
```

Regra: **todo módulo com experimento declara o orçamento antes de medir.** Um
resultado só é "bom" ou "ruim" contra um orçamento; sem ele, só é diferente.

## 11. Protocolo de comparação simétrica

Esta seção existe por causa de um achado do projeto irmão, e é a regra mais
importante da Parte II.

Na comparação entre a versão DPDK e a versão C++23 do mesmo problema, a tabela
inicial media também **a diferença de aquecimento entre os dois braços**, porque
só um deles aquecia:

> Uma comparação que não controla o método mede o método, não o objeto.

E o resultado publicado — C++23 puro de 1,6 a 2,2 vezes mais rápido — é
acompanhado do aviso de que **o teste removeu tudo aquilo pelo qual o DPDK
cobra**: sem NIC não há DMA, sem troca entre núcleos as barreiras do ring não
compram nada, com um núcleo o cache por lcore é indireção pura. *"É como medir o
peso de um cinto de segurança num carro parado."*

A tese central deste projeto (seção 14) tem **exatamente o mesmo modo de
falha**. Um microbenchmark de tabela `constexpr` num laço, com a tabela quente
em L1 e acesso perfeitamente previsível, remove tudo que a versão com
condicional teria custado — e a recíproca também vale.

Portanto, toda comparação entre duas implementações cumpre:

1. **aquecimento simétrico** nos dois braços, declarado no documento;
2. **piso de amostragem**: os dois braços se recusam a publicar abaixo do mínimo
   declarado de operações;
3. **frequência do núcleo** impressa junto do resultado;
4. **declaração do que o benchmark removeu** do cenário, e **a qual braço isso
   favorece**;
5. **separação explícita** entre o que a medição legitimamente mostra e o que
   ela aparenta mostrar (seção 27);
6. **varredura, não ponto único**, quando a variável sob teste interage com a
   hierarquia de memória — ver seção 14.

## 12. Resultados negativos

O projeto publica experimentos em que a otimização não ajudou ou piorou o
desempenho:

```text
prefetch manual → +3,8% de ciclos/op → otimização rejeitada
```

Esse resultado tem valor acadêmico: mostra que a técnica não é universal,
depende do mecanismo e da carga.

> **Otimização rejeitada também é conhecimento.** Os resultados negativos têm
> índice próprio, para serem encontráveis em vez de ficarem enterrados no módulo
> que os produziu.

---

# Parte III — Contratos e código

## 13. As três portas de contrato

Esta é a diretriz central de implementação do projeto: **cada atributo declara
explicitamente seus termos de aceite**, e a validação redundante desaparece do
código otimizado.

A formulação inicial pedia contrato por atributo com os termos de aceite
verificados **somente em modo DEBUG**. O mecanismo dessa intenção é o
[`assert`](referencias.md#cpp-assert), desligado pela definição de `NDEBUG` — e
não o `static_assert`, que o documento de origem nomeava e que tem semântica
diferente:

- `static_assert` é avaliado na compilação, **incondicionalmente** — não existe
  release em que esteja desligado;
- `static_assert` **não vê valor de runtime**: não há como validar com ele o
  número que vem de uma variável de ambiente, de um arquivo de configuração ou
  da linha de comando.

Os dois são necessários, e para coisas diferentes. E o `assert` sozinho também
não basta, por uma razão medida: com `NDEBUG` ele expande para `((void) 0)`, e
não sobra nem a verificação **nem a informação** — a condicional redundante a
jusante volta ao binário de release, que é justamente o que a segunda diretriz
quer eliminar. O mecanismo da porta R é, portanto, **`assert` mais
`[[assume]]`**: a verificação em debug, a informação em release. A tabela medida
está em [CPP-ASSERT](referencias.md#cpp-assert).

Somando as três situações distintas, cada uma com o seu mecanismo:

| Porta | O que valida | Mecanismo | Custo em release |
|---|---|---|---|
| **T** — compilação | termo de aceite sobre valor constante, propriedade de tipo, relação entre constantes, layout, alinhamento | `static_assert`, concept, `requires` | zero, sempre ativo |
| **R** — runtime confiável | valor interno cuja violação é **bug de programador** | `assert` em DEBUG; `[[assume]]` em release | zero |
| **E** — fronteira externa | valor vindo de variável de ambiente, arquivo, CLI | `std::expected` — **sempre ativo** | pago, e deve ser |

### A porta E nunca é compilada fora

Um número malformado numa variável de ambiente, num arquivo de configuração ou
na linha de comando **não é bug de programador**: é dado de ambiente. Se a
validação de entrada externa desaparecesse em release, o programa passaria a
confiar em conteúdo que ninguém controla — isso é furo de correção, não
otimização.

"Evitar condicional de validação desnecessária" vale para T e R. **Para E a
condicional é necessária**, e a norma é: verificação de entrada externa vive em
`adapters/` e nunca é condicionada a `NDEBUG`.

### O termo de aceite é escrito uma vez

O que unifica as três portas é o predicado `constexpr` único:

```cpp
template <std::integral T, T Lo, T Hi>
struct closed_range
{
    static_assert(Lo <= Hi, "dominio invertido: Lo > Hi");
    static constexpr bool accepts(T v) noexcept { return v >= Lo && v <= Hi; }
};
```

consumido pelas três portas: `static_assert(Domain::accepts(V))` quando o valor
é constante, `PERF_EXPECTS(Domain::accepts(v))` quando é interno,
`std::expected` quando vem de fora.

### O mecanismo de runtime, e por que é macro

```cpp
#ifdef NDEBUG
#  define PERF_EXPECTS(expr) [[assume(expr)]]
#else
#  define PERF_EXPECTS(expr) ::perf::contract::check((expr), #expr, \
                                 std::source_location::current())
#endif
```

`[[assume]]` é de **C++23** ([P1774](referencias.md#p1774)); Contracts
(`pre`, `post`, `contract_assert`) é de **C++26**
([P2900](referencias.md#p2900)) e **não está disponível no toolchain de
referência** — medido, ver a entrada do catálogo. A macro é a costura de
migração: quando Contracts existir, o corpo muda e nenhum ponto de chamada é
tocado. É por isso que `.clang-tidy` desliga `cppcoreguidelines-macro-usage`,
com a razão escrita no próprio arquivo.

### A advertência séria

**Predicado falso em `[[assume]]` é comportamento indefinido**, e o otimizador
tem licença para eliminar caminhos inteiros do código. Um contrato errado não
vira assert que falha: vira release silenciosamente miscompilada.

Consequência normativa — a matriz de build tem **três** configurações, não duas.
Ver seção 16.

### O ganho que fecha a diretriz

Se o domínio é `constexpr`, **o teste do domínio é `static_assert`**: custo zero
em runtime, e a violação quebra o build em vez de quebrar a suíte.

```cpp
static_assert(sample_count_domain::accepts(3));
static_assert(!sample_count_domain::accepts(2));
```

Já aplicado em [`lib/contract/measurement_domains.hpp`](../lib/contract/measurement_domains.hpp)
e em [`lib/measurement/statistics.hpp`](../lib/measurement/statistics.hpp).

### Quais domínios vêm primeiro, e por quê

Decisão de escopo, deliberada: **os primeiros domínios concretos são os da
própria régua de apuração** — contagem de amostras, rodadas por amostra,
percentil, alinhamento. Eles são conceituais, autocontidos, e os termos de
aceite deles já existiam em prosa dentro de `statistics.hpp`, implementados como
número solto no meio de uma chamada. Transformá-los em domínio é o exemplo mais
honesto da diretriz: o termo de aceite deixa de ser comentário e passa a ser
verificado.

Domínios de parâmetro de ajuste do sistema operacional **não pertencem à
fundação**. Eles entram na etapa final, com o plano de integração da seção 30.

## 14. Compile-time versus runtime: a tese, não a regra

A segunda diretriz do projeto — elevar lógica sofisticada para evitar
condicional ou laço que possa ser substituído por mecanismo de compilação — **não
é regra de estilo neste projeto. É a tese central, e é medida.**

> Onde, e sob quais condições, mover trabalho de runtime para compile-time
> efetivamente paga?

Como regra de estilo ela seria dogma, e colidiria de frente com a política
editorial (seção 32): um `if` que o preditor acerta com taxa próxima de 100%
custa quase nada, enquanto a tabela `constexpr` que o substitui pode custar um
cache miss — trocando desvio previsto por acesso a `.rodata` fora do L1. Afirmar
o ganho antes de medir é exatamente o `usar técnica X porque ela é rápida` que a
seção 6 proíbe.

### O que é ganho estrutural e não precisa de medição

- `if constexpr` para seleção de política: elimina o desvio e o código morto;
- `if consteval` ([P1938](referencias.md#p1938)) e
  `std::is_constant_evaluated()` para predicado de modo duplo;
- `<bit>`: `has_single_bit`, `bit_width`, `countl_zero`, `popcount` —
  aritmética de alinhamento, tamanho de página e arredondamento sem laço nem
  condicional, com instrução dedicada na maioria dos alvos;
- parâmetro de template não-tipo para **unidade forte** (`Bytes<N>`,
  `Pages<Size>`), que elimina a classe de erro "passei KiB onde se esperava
  páginas";
- `std::expected` na fronteira; `std::span` e `std::mdspan` para leitura sem
  cópia.

### O que é hipótese e exige experimento

- tabela `static constexpr std::array` construída por função `consteval`, no
  lugar de cadeia de `if`;
- hash perfeito congelado em compilação para resolver nome de parâmetro;
- desenrolar de laço e eliminação de desvio por aritmética;
- especialização de template no lugar de despacho dinâmico.

> **`std::ranges` no lugar de laço manual é ganho de legibilidade e
> proveniência, não de velocidade.** É o mesmo laço. Vendê-lo como otimização
> viola a seção 6.

### Varredura, não ponto único

O projeto irmão mediu a curva de lote: ajuda muito até 8, marginalmente até 32,
**regride em 128** — e a forma é a mesma nos dois braços, porque é comportamento
de cache, não da biblioteca.

Isso prevê o comportamento das tabelas `constexpr`: **existe um ótimo, e passar
dele custa.** Portanto todo experimento de tabela contra condicional é uma
**varredura sobre o tamanho do working set**, nunca um ponto único.

### As métricas da tese incluem o custo de compilar

Mover trabalho para compile-time **tem custo**, e ele é real:

| Métrica | Por quê |
|---|---|
| ciclos/op, ns/op | o ganho pretendido |
| cache misses, branch misses | o mecanismo do ganho ou da perda |
| **tempo de compilação** | instanciação de template não é grátis |
| **tamanho do binário** | tabela vai para o binário |
| **tamanho de `.rodata`** | é o que compete pelo cache com os dados |

Um domínio que compila em 40 s e gera 200 KB de tabela para economizar 2 ns num
caminho frio é **otimização rejeitada** — e o projeto só consegue dizer isso se
medir as duas pontas.

## 15. Arquitetura em camadas

```text
lib/
├── measurement/   régua de apuração: relógio, estatística, dica de espera
└── contract/      contrato, domínios, unidades, orçamento

<módulo ou projeto final>/
├── domain/        cálculo puro: unidades, domínios, invariantes
│                  100% constexpr, ZERO I/O, testado por static_assert
├── ports/         interfaces: TopologyReader, TunableStore, CounterSource
├── adapters/      leitura do ambiente: arquivos, variáveis, contadores
├── application/   casos de uso: diagnosticar, propor, aplicar, verificar
└── cli/           apresentação e relatório de evidência
```

A separação não é moda: **o domínio só é `constexpr`-testável porque não tem
I/O.** A decisão de arquitetura e a decisão de desempenho são a mesma decisão
aqui. E a regra da porta E fica verificável por inspeção de diretório —
`std::expected` de validação de ambiente aparece em `adapters/`, não espalhado.

## 16. Toolchain e a matriz de três configurações

| Item | Valor |
|---|---|
| Padrão da linguagem | **C++23** (`cpp_std=c++23`) |
| Compiladores de referência | GCC 15.2.0 e Clang 21.1.8 |
| Build | Meson ≥ 1.1 + Ninja |
| Arquiteturas no escopo | x86-64 e **arm64** |

A escolha de C++23 estrito é o que torna `[[assume]]` o mecanismo de contrato, e
não Contracts — ver seção 13.

### Por que três configurações

Porque `[[assume]]` com predicado falso é comportamento indefinido, e nenhuma
das duas configurações usuais detecta isso:

| Configuração | Contratos | Otimização | O que ela prova |
|---|---|---|---|
| `debug` | verificados | `-O0`/`-Og` | os contratos valem no código não otimizado |
| `release` | `[[assume]]` | `-O2`/`-O3` | é o que se publica e se mede |
| **`release-checked`** | **verificados** | `-O2`/`-O3` | **os contratos valem no código otimizado** |

Sem a terceira, o projeto assume sem nunca verificar o que assumiu. A suíte roda
nas três.

---

# Parte IV — Fontes

## 17. Hierarquia das fontes

| Nível | O que é | Exemplos |
|---|---|---|
| **A** | normativa ou primária | ISO C++, WG21, Intel SDM/ORM, AMD, Arm ARM, Linux Kernel Documentation, GCC, LLVM, Clang, POSIX, IEEE, RFC |
| **B** | literatura acadêmica | ACM, IEEE, USENIX, SOSP, OSDI, ASPLOS, PLDI, PPoPP, ISCA, MICRO, ISMM, ISPASS |
| **C** | literatura técnica reconhecida | Agner Fog, Brendan Gregg, Paul McKenney, Ulrich Drepper, Anthony Williams, Herb Sutter, Maurice Herlihy, Scott Meyers, Chandler Carruth, Andrei Alexandrescu |
| **D** | material complementar | palestras, blogs de engenharia, artigos técnicos de fabricante, implementações de referência |

Nível A tem precedência quando define diretamente o comportamento estudado.
Nível D nunca substitui A quando A existe.

Papers de Nível B devem trazer autor, título, veículo, ano e DOI.

## 18. Fontes que não sustentam afirmação técnica importante

Evitar como fonte principal: Wikipedia, Stack Overflow, posts de rede social,
blogs sem autoria técnica identificável, artigos de SEO, benchmarks sem
metodologia.

Esses materiais ajudam a **localizar** um assunto; não são autoridade final.

### Sobre conteúdo gerado por IA

O documento de origem listava "conteúdo gerado por IA" entre as fontes a evitar.
A regra, como escrita, era ambígua num material que é produzido com assistência
de IA — e uma norma que o próprio projeto viola não é norma.

A formulação vigente diz o que de fato se quer:

> **Toda afirmação técnica rastreia a uma fonte de Nível A–D ou a um experimento
> reproduzível deste repositório. A autoria do texto não é fonte** — nem humana,
> nem automática. Saída de modelo de linguagem não é citável como autoridade;
> texto assistido por modelo cujas afirmações rastreiam a fonte identificada ou
> a experimento é aceitável, e é como este material é escrito.

## 19. Catálogo central

O catálogo é [`referencias.md`](referencias.md) / [`referencias.en.md`](referencias.en.md).

Identificadores são **estáveis**: não mudam se os documentos forem
reorganizados.

## 20. Âncoras

A âncora de cada referência vem do **título da entrada**. `### INTEL-ORM` produz
`#intel-orm` pelo github-slugger, e é isso que `verificar-links.py` confere:

```markdown
[INTEL-ORM](../../docs/referencias.md#intel-orm)
```

O documento de origem exigia também uma tag `<a id="intel-orm"></a>` explícita.
Ela foi **removida da norma**: nada verificava a concordância entre a tag e o
título, e dois identificadores para a mesma seção divergem em silêncio. Foi o
próprio ferramental portado que expôs isso — o verificador só conhece âncoras de
título.

## 21. Referências por módulo

Cada módulo termina com a lista das fontes que o sustentam:

```markdown
## Referências

- [INTEL-ORM](../../docs/referencias.md#intel-orm)
- [AGNER-CPP](../../docs/referencias.md#agner-cpp)
- [LINUX-PERF](../../docs/referencias.md#linux-perf)
```

Isso permite saber imediatamente o que sustenta aquele capítulo. O catálogo
central continua sendo a fonte completa dos metadados.

## 22. Referências versionadas

Quando a referência depende de versão, o documento a identifica: GCC 15,
Clang 21, Linux 6.x, revisão do manual de otimização, geração da microarquitetura.

Evitar `segundo a documentação da Intel`. Preferir `segundo Intel 64 and IA-32
Architectures Optimization Reference Manual, revisão X`.

## 23. Links estáveis

Preferir documentação oficial, DOI, página permanente do fabricante, número de
paper do WG21, RFC, identificador de norma. Evitar link de busca, URL
temporária, mirror desconhecido, blog que copia documentação.

## 24. Relação referência ↔ módulo

| Módulo | Referências principais |
|---|---|
| Benchmarking e método | GOOGLE-BENCHMARK, KALIBERA-JONES-2013, LINUX-PERF, BRENDAN-GREGG-SP |
| Contratos e domínios | CPP-STD, P1774, P2900, P1938, LINUX-DOC, CGROUP-V2 |
| Compile-time versus runtime | CPP-STD, GCC-DOC, CLANG-DOC, LLVM-MCA, UICA, AGNER-INST |
| Cache e memória | INTEL-ORM, AMD-OPT, ARM-NEOVERSE, DREPPER-MEM, AGNER-CPP, DPDK-ACADEMY |
| Layout de dados | INTEL-ORM, AMD-OPT, AGNER-CPP |
| Alocação | CPP-STD, AGNER-CPP, JEMALLOC, TCMALLOC, MIMALLOC |
| Zero-copy | CPP-STD, POSIX, LINUX-DOC |
| Predição de desvio e prefetch | INTEL-ORM, AMD-OPT, AGNER-UARCH |
| Otimização do compilador | GCC-DOC, LLVM-DOC, CLANG-DOC, LLVM-MCA, AGNER-CPP |
| SIMD | INTEL-SDM, INTEL-ORM, AMD-OPT, ARM-ARM |
| Atomics e ordenação de memória | CPP-STD, WG21, BOEHM-ADVE-2008, MCKENNEY-PERFBOOK, WILLIAMS-CCIA |
| Lock-free | CPP-STD, HERLIHY-WING-1990, MICHAEL-SCOTT-1996, MCKENNEY-PERFBOOK |
| NUMA | LINUX-DOC, INTEL-ORM, AMD-OPT, DREPPER-MEM, DPDK-ACADEMY |
| Profiling e contadores | LINUX-PERF, PERF-EVENT-OPEN, YASIN-2014, PMU-TOOLS, BRENDAN-GREGG-SP |
| Microarquitetura | INTEL-ORM, AMD-OPT, AGNER-UARCH, ARM-NEOVERSE |
| Ajuste do sistema — **etapa final** | LINUX-DOC, CGROUP-V2, EX442-LABS, BRENDAN-GREGG-SP |

---

# Parte V — Estrutura do documento

## 25. Estrutura padrão de capítulo

O documento de origem listava 18 seções como "preferenciais", o que deixava
ambíguo o que é exigido — e mandava "Hardware counters" até em capítulo
conceitual, onde é ruído. A norma vigente separa:

| Seção | Estado |
|---|---|
| Título, Objetivos, Problema | **obrigatória** |
| Modelo mental, Fundamentos, Como funciona | **obrigatória** |
| Implementação baseline | obrigatória **se houver código** |
| Experimento, Ambiente de execução, Resultados | obrigatória **se houver medição** |
| Hardware counters | obrigatória **se houver medição de microarquitetura** e PMU disponível |
| **O que esta medição não mostra** | obrigatória **se houver medição** (seção 27) |
| Análise | obrigatória **se houver medição** |
| Confronto com a literatura | obrigatória **se houver experimento relevante** (seção 26) |
| Trade-offs, Quando utilizar, Quando não utilizar | **obrigatória** |
| Limitações | **obrigatória** |
| Exercícios | **obrigatória** |
| Referências, Navegação | **obrigatória** |

A versão inglesa tem exatamente a mesma estrutura conceitual: Objectives,
Problem, Mental model, Fundamentals, How it works, Baseline implementation,
Experiment, Execution environment, Results, Hardware counters, **What this
measurement does not show**, Analysis, Literature comparison, Trade-offs, When
to use, When not to use, Limitations, Exercises, References, Navigation.

## 26. Confronto com a literatura

A pergunta é: **o comportamento medido é compatível com o que as fontes técnicas
descrevem?**

Três resultados possíveis:

```text
medição confirma a literatura
medição diverge da literatura        → investigar o contexto
literatura não prevê o resultado     → registrar como observação experimental
```

A divergência não é escondida. Ela pode ser a parte mais interessante do estudo.

E não se assume que divergência significa literatura errada. Ela pode vir de
hardware, microarquitetura, compilador, flags, SO, kernel, SMT, turbo, governor,
NUMA, dataset, benchmark incorreto ou efeito não controlado. O processo é:

```text
Literatura → Hipótese → Experimento → Resultado → Análise crítica
```

## 27. O que esta medição não mostra

Seção obrigatória em todo documento com medição. Responde:

1. **o que o benchmark removeu** do cenário real;
2. **a qual braço** da comparação essa remoção favorece;
3. **qual conclusão o leitor tiraria** se parasse na tabela;
4. **o que seria necessário** para sustentar a conclusão mais ampla.

O modelo é o documento de alternativa do projeto irmão, que publica uma tabela
favorável a C++23 puro e escreve em seguida *"se você parar de ler aqui, tirará
a conclusão errada"*, explicando que o teste removeu DMA, troca entre núcleos e
pressão de memória — tudo aquilo pelo qual a abstração medida cobra.

> Uma tabela sem esta seção é um número sem escopo. O projeto trata isso como
> defeito de documento, não como estilo.

## 28. Reprodutibilidade

Cada resultado publicado permite responder: qual CPU, qual kernel, qual
compilador, qual versão, quais flags, quantas execuções, qual afinidade, qual nó
NUMA, turbo ativo, SMT ativo, qual governor, qual tamanho de dataset, qual
código gerou o número.

Acrescenta-se, para este projeto: **qual configuração de build** das três da
seção 16, e **o PMU estava disponível?**

Variável não conhecida é documentada como não conhecida.

## 29. Ambiente de referência

Existe `scripts/ambiente.sh`, que **gera** o registro em vez de o autor o
descrever em prosa. A razão está no script de origem: descrição em prosa diverge
entre arquivos sem que ninguém perceba, envelhece em silêncio quando o kernel ou
o compilador mudam, e não diz a quem reproduz o que comparar.

Registra pelo menos: modelo de CPU, microarquitetura quando identificável,
núcleos, threads, SMT, topologia NUMA, topologia de cache, RAM, kernel,
governor, turbo, compilador e versão, Meson, Ninja, perf. Quando relevante:
mitigations, hugepages, isolamento de CPU, frequência, estado térmico.

> **Lição herdada, a ser preservada na porta:** o script de origem **não** faz
> parsing do texto do `lscpu`, porque a saída dele é traduzida — a primeira
> versão devolvia campos vazios em silêncio numa máquina em português, o que é
> irônico num script cuja razão de existir é reprodutibilidade. As formas usadas
> são `lscpu -p=` e o sysfs, independentes de idioma. **Não reescrever essa
> parte.**

---

# Parte VI — Governança

## 30. Divisão de trabalho com os projetos irmãos

O DPDK Academy já cobre, **com medição**, os fundamentos de mecanismo:
orçamento por unidade de trabalho, fronteira usuário/kernel, tradução de
endereço e page walk, TLB, hugepages, cache e localidade, falso
compartilhamento, NUMA e primeiro toque, SMT, polling versus dormir com
comparação de primitivos, DMA e descritores, IOMMU e IOTLB, orçamento do
barramento, e o vocabulário de métricas ancorado em norma.

**Norma: este projeto cita e estende; não reensina.**

| Assunto | Onde vive | O que este projeto acrescenta |
|---|---|---|
| custo de cache miss, TLB, NUMA, syscall | DPDK-ACADEMY | como o modelo de objetos e o layout de C++ o provocam ou o evitam |
| custo de sincronização | DPDK-ACADEMY | `std::atomic`, ordenação de memória, correção linearizável |
| superfície de ajuste do SO | EX442-LABS | **nada, até a etapa final** — ver abaixo |
| geração de código, `constexpr`, template | — | **é o objeto próprio deste projeto** |

### Quando o EX442 entra, e por que não antes

O eixo de ajuste do sistema operacional — `sysctl`, `sysfs`, cgroups,
hugepages, escalonador, writeback — **não aparece na fundação nem nos módulos
conceituais**. Ele entra numa etapa própria, ao fim, na forma de um **plano de
integração** entre o que este projeto ensina e a otimização de baixo nível que o
EX442 exercita.

A razão é de ensino, não de escopo: os dois eixos são grandes, e misturá-los faz
o leitor estudar ajuste de kernel enquanto deveria estar estudando modelo de
objetos, layout e geração de código. Um domínio de `cpu.weight` num módulo sobre
contratos ensina cgroups de carona, e a carona custa a atenção que o conceito
precisava.

Consequência prática, já aplicada: a biblioteca de contratos modela os domínios
da **régua de apuração deste projeto**, não os do sistema operacional. Ver
seção 13.

### "DPDK versus C++" é falsa oposição

Registrado aqui porque é a identidade do projeto, e o documento irmão já o
estabeleceu: programas DPDK **são** escritos em C e C++; a API do DPDK é C,
chamável de C++23 sem intermediário. O que se compara são duas arquiteturas de
gestão de memória e fluxo, e a diferença real é **quem garante** que não há
alocação no caminho quente: no DPDK, o programador; em C++23, o sistema de tipos
e o RAII.

Este projeto **não** é o "anti-DPDK". É a mesma disciplina com a fronteira de
abstração movida.

## 31. Verificação automatizada

Política sem portão não vale. Os verificadores em
[`ferramental/qualidade/`](../ferramental/qualidade/) rodam na suíte e na CI:

| Verificador | Regra |
|---|---|
| `verificar-links.py` | link relativo aponta para arquivo e âncora existentes |
| `verificar-ancoras.py` | âncora de linha aponta para o trecho certo do código |
| `verificar-retratacoes.py` | valor declarado retratado não sobrevive fora do bloco que o retrata |
| `verificar-aritmetica.py` | percentual que o texto torna conferível fecha |

Os quatro vêm do projeto irmão, onde **cada um nasceu de um defeito medido**, e
todos têm autoteste. A razão de portar em vez de escrever está no cabeçalho de
`verificar-retratacoes.py`: no defeito que o originou, quem escreveu a retratação
sabia da regra, tinha acabado de enunciá-la, e ainda assim deixou o valor
derrubado circulando em outras páginas. **Correção manual não escala.**

Faltam ainda, com acoplamento a adaptar: `verificar-promessa.py` (todo programa
citado existe e entra na compilação) e `verificar-autodescricao.py` (o que o
material afirma sobre si corresponde ao disco). Estado: **não portados** —
registrado no [ROADMAP.md](../ROADMAP.md).

## 32. Política editorial

> **Mecanismo antes da otimização.**
>
> **Medição antes da conclusão.**
>
> **Correção antes da performance.**
>
> **Número sem experimento não é resultado.**
>
> **Afirmação normativa sem autoridade identificada não é referência.**
>
> **Benchmark sem ambiente documentado não é reproduzível.**
>
> **Comparação sem método simétrico mede o método.**
>
> **Medição sem escopo declarado é número sem sentido.**
>
> **Diferença observada sem mecanismo conhecido permanece explicitamente como
> hipótese.**
>
> **Otimização rejeitada também é conhecimento.**
>
> **Contrato assumido e não verificado é comportamento indefinido, não
> otimização.**

## 33. Identidade acadêmica

A documentação é profunda o suficiente para conectar:

```text
C++ source → semântica da linguagem → compilador → assembly
          → microarquitetura → cache/memória → sistema operacional → medição
```

O objetivo não é ensinar **como escrever C++ rápido**. É ensinar:

> **como investigar cientificamente por que determinado software C++ apresenta
> determinado comportamento de desempenho.**

Essa filosofia permanece consistente entre DPDK Academy, C++ Performance
Academy e Messaging Academy, formando uma família coerente de estudos sobre
**High Performance Systems Engineering**.

---

## Divergências em relação ao documento de origem

Registro auditável das correções aplicadas sobre
[`origem/setup-cpp-performance-academy.md`](origem/setup-cpp-performance-academy.md), que permanece intacto.

| # | Origem | Correção |
|---|---|---|
| 1 | §11 pedia `referencias.md` / `references.en.md` | radical idêntico: `referencias.md` / `referencias.en.md` (seção 2) |
| 2 | §2 exemplificava `trilha/03-cache-memory/` em inglês; §3 mandava diretório técnico em inglês | regra por camada: currículo em português, código em inglês (seção 3) |
| 3 | hierarquia de títulos misturava `##` e `#` para o mesmo nível | seis partes em `#`, seções em `##` |
| 4 | §1 e §4 instituíam bilinguismo; o documento existia só em PT-BR e sem navegação | navegação obrigatória também em `docs/`; par EN deste documento |
| 5 | §20 citava GCC, LLVM, POSIX, Linux, Intel/AMD como autoridade sem identificador; CPPREFERENCE definida e ausente da lista | identificadores criados no catálogo; tabela da seção 24 cita só ids existentes |
| 6 | §21 dizia "preferencialmente" para 18 seções, e exigia Hardware counters em capítulo conceitual | obrigatório versus condicional explicitado (seção 25) |
| 7 | §6 definia as quatro classes de afirmação sem notação | marcador obrigatório (seção 7) |
| 8 | §10 proibia "conteúdo gerado por IA" sem dizer o que isso implica num material assistido por IA | regra de rastreabilidade (seção 18) |
| 9 | §12 exigia tag `<a id>` além do título | removida: era decoração não verificada; o verificador só conhece âncora de título (seção 20) |
| 10 | a diretriz de contratos nomeava `static_assert` "ativo somente em DEBUG" | o mecanismo dessa intenção é o `assert` (gatilho `NDEBUG`); `static_assert` é incondicional e não vê runtime. **Os dois são usados, para coisas diferentes**, e `assert` sozinho perde a informação em release — três portas T/R/E (seção 13) |
| 11 | a diretriz de evitar condicional/laço seria regra de estilo | promovida a tese medida (seção 14) |
| 12 | nada exigia protocolo de comparação, escopo de medição, orçamento, ou matriz de build | seções 10, 11, 16 e 27 |
| 13 | nada definia divisão de trabalho com os projetos irmãos | seção 30 |
| 14 | nada exigia rotulagem de estado | seção 5 |

---

## Navegação

- [Catálogo de referências](referencias.md)
- [Índice da documentação](README.md)
- [README do repositório](../README.md)
- [Documento de origem, intacto](origem/setup-cpp-performance-academy.md)

> 🇧🇷 Português | [🇺🇸 English](padrao-do-projeto.en.md)
