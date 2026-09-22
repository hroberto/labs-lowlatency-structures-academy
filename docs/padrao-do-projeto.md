> 🇧🇷 Português | [🇺🇸 English](padrao-do-projeto.en.md)

# Padrão do projeto — Low-Latency Structures Academy

Este documento define as regras obrigatórias de **método, medição, contrato,
arquitetura, documentação e referências** do Low-Latency Structures Academy.

> **Esta norma foi absorvida, e a absorção está registrada.** Ela nasceu no
> `labs_cpp_performance_tuning_academy` — norma escrita, fundação executável,
> trilha não iniciada, nenhum commit — e passou a reger este repositório, que
> tinha a trilha e não tinha fundação. O que a absorção acrescentou está nas
> seções [34](#34-o-par-std-e-custom-como-unidade-de-trabalho),
> [35](#35-regra-de-decisão) e [36](#36-o-eixo-do-código-gerado), e o que ela
> corrigiu está na segunda tabela de divergências.

## O que este documento substitui

Ele é a versão normalizada de [`origem/setup-cpp-performance-academy.md`](origem/setup-cpp-performance-academy.md),
**preservado intacto** para que as divergências entre a intenção original e a
norma vigente sejam auditáveis. As correções aplicadas estão listadas na última
parte, em [Divergências em relação ao documento de origem](#divergências-em-relação-ao-documento-de-origem).

## Os projetos a montante

| Projeto | O que este projeto toma dele |
|---|---|
| [DPDK-ACADEMY](referencias.md#dpdk-academy) | arquitetura de alta volumetria, custos de mecanismo já medidos, régua de apuração, ferramental de qualidade |
| [EX442-LABS](referencias.md#ex442-labs) | **apenas na etapa final**: a superfície de ajuste do sistema operacional, no plano de integração com otimização de baixo nível (seção 30) |
| C++ Performance Academy | **absorvido**: esta norma, `lib/measurement/`, `lib/contract/`, os verificadores — e o eixo `linguagem ↔ compilador ↔ código gerado`, que virou a seção 36 |
| este projeto | **onde a biblioteca padrão para**: o par `std` e `custom` medido no mesmo contrato, e a regra de decisão que sai dele |

O terceiro não é mais um projeto a montante no sentido dos outros dois: ele não
existe separado deste. Está listado porque a norma que você está lendo é dele, e
porque a seção 30 legislava sobre este repositório antes de ele ter fundação.

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

Em cada tópico da trilha:

```text
trilha/
└── 04-livro-de-ofertas/
    └── 02-array-plano-com-bitmap/
        ├── README.md
        ├── README.en.md
        ├── spec.hpp            a lei do topico: interface e traits dos dois bracos
        ├── std/                o baseline que a linguagem entrega
        ├── custom/             o complemento, sob o mesmo contrato
        ├── bench/
        │   └── medicoes/       saida arquivada: metadata, CSV, tabela
        ├── tests/
        └── meson.build
```

O documento de origem desta norma exemplificava `src/` e `resultados/`, que
pressupõem um tópico com **uma** implementação. Aqui o tópico tem **duas**, e a
estrutura declara isso: `std/` e `custom/` lado a lado, não a alternativa num
apêndice. A razão do nome `spec.hpp`, e não `contract.hpp`, está na seção 34.

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
mensagens de log e identificadores de CI. O objetivo é manter o código
compatível com o vocabulário da literatura, das ferramentas e da comunidade
internacional de C++.

### Onde a fronteira passa, e por que ela precisava ser dita

A regra acima não dizia onde o **inglês** para, e a omissão produziu deriva
medida: o primeiro programa de medição deste repositório nasceu com
`custo_do_relogio()`, 35 mensagens em português e a flag `--braco-de-controle`.
Nada disso contraria a letra da regra; contraria o que ela quis dizer.

| Camada | Idioma | Exemplos |
|---|---|---|
| identificadores de C++ | inglês | `clock_read_cost`, `tail_collector`, `sample_count` |
| **saída de programa** | inglês | `"reading the clock"`, `"CONTRACT VIOLATED (precondition)"`, cabeçalho de tabela |
| **flags de linha de comando** | inglês | `--csv`, `--violate`, `--control-arm` |
| **variáveis de ambiente** | inglês, com prefixo do INSTRUMENTO | `HARNESS_SAMPLES`, `HARNESS_ROUNDS`, `HARNESS_TAIL_SAMPLES` |
| **esquema de dado emitido por programa** | inglês | o CSV `arm,metric,value,unit` |
| comentário de código | português | é prosa, e prosa é português |
| script de `scripts/` e de `ferramental/` | português | é automação de quem estuda e de quem publica, e o nome do arquivo já é português |
| registro emitido por script | português | `ambiente.json`, `metadata.json` — estrutura em português, com identificadores de métrica em inglês dentro |
| mensagem de commit | **português** | ver abaixo |

**O prefixo de variável de ambiente nomeia o instrumento, não a academia.**
`HARNESS_*` e não `CPP_ACADEMY_*`: a régua de apuração é candidata a
repositório próprio, consumido por este projeto, pelo DPDK Academy e pelo
Messaging Academy (seção 33). Uma régua compartilhada que lê `CPP_ACADEMY_*`
num repositório e `DPDK_ACADEMY_*` noutro não é compartilhada — é copiada.

**A mensagem de commit é a exceção, e ela é deliberada.** O documento de origem
desta norma listava "mensagens de commit" entre os itens em inglês, e o
histórico deste repositório e do DPDK Academy é inteiro em português. A regra
segue a prática, e não o contrário: o assunto do commit é o **achado** — o que
quebrou, o que aquilo produziu, e qual verificação o teria pegado —, e achado é
prosa. Prosa é português.

### Diretórios: a regra por camada

O documento de origem se contradizia aqui — o exemplo de estrutura usava
`trilha/03-cache-memory/`, em inglês, enquanto a regra de identificadores manda
diretório técnico em inglês e o projeto irmão usa português em toda a trilha.

A norma vigente distingue **diretório de currículo** de **diretório de código**:

| Camada | Idioma | Exemplos |
|---|---|---|
| currículo e documentação | português | `trilha/01-memoria/`, `docs/`, `medicoes/` |
| código e build | inglês | `std/`, `custom/`, `bench/`, `tests/`, `lib/measurement/`, `lib/contract/` |

A razão é que o nome do diretório de currículo é **texto do material** — ele
aparece na navegação, na trilha e nos links do documento —, enquanto o nome do
diretório de código é **identificador**, sujeito à regra 3.

### O arquivo de medição é versionado, e tem um nome só

O diretório de saída arquivada é **`bench/medicoes/`**, uma vez. O `.gitignore`
herdado ignorava `resultados/**`, e o setup da trilha arquivava em
`bench/medicoes/`: dois nomes para a mesma coisa, e o segundo entrava no
histórico do git porque o primeiro era o ignorado.

A colisão de nome era o defeito menor. O maior era a **regra**: o `.gitignore`
herdado declarava que saída bruta de medição não se versiona, porque "difere em
cada host e inflaria o histórico". Isso contradiz a regra da seção 8 desta norma
e a prática do DPDK Academy, que versiona cada campanha em
`medicoes/historico/<AAAA-MM-DD>-<campanha>/`, com o ambiente ao lado e **uma
saída por repetição** (`r0`, `r1`, `r2`, `r3`).

A regra vigente é a do DPDK Academy, e a razão é o que este projeto publica:

> Um p99,9 cuja campanha não está na árvore não é auditável depois. O ativo do
> projeto é a **procedência** do número, e procedência que mora fora do
> repositório não é procedência.

Portanto: o histórico de campanha **é versionado**. O que entra é o registro
auditável, e não tudo o que a campanha produz:

| Arquivo | Versionado | Por quê |
|---|---|---|
| `metadata.json` | **sim** | parâmetros, build, *commit*, ambiente embutido, e **a série completa por execução** de cada métrica |
| `tabela.md` / `tabela.en.md` | **sim** | é o que entra nos dois READMEs do tópico |
| `ambiente.md` | **sim** | a coleta legível, que se confere a olho |
| `r*.csv` | não | intermediário: medido que **todas** as métricas já trazem `por_execucao` no metadata |
| `ambiente.json` | não | duplica o `.md`, e o metadata já embute o ambiente inteiro |

Guardar o mesmo dado duas vezes não aumenta procedência; aumenta a chance de as
duas cópias divergirem. Quem mediu continua com a saída crua na máquina; quem
audita tem, no `metadata.json`, a mesma série que ela contém.

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

**Este protocolo é o portão de publicação do par `std` e `custom`** (seção 34).
Não é um cuidado a mais: é a diferença entre medir a estrutura e medir o
harness. O modo de falha aqui é o mesmo de lá — um `custom/` que aquece e um
`std/` que não aquece publicam a diferença de aquecimento com o nome da
estrutura.

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

<tópico da trilha>/
├── spec.hpp       a lei: interface comum e traits de invariante (seção 34)
├── std/           o baseline que a linguagem entrega
├── custom/        o complemento, sob a mesma spec
├── bench/         os dois braços no mesmo harness, mesma carga
└── tests/         L1 parametrizado pela spec, L2 como o leitor executa

<capstone>/
├── domain/        cálculo puro: unidades, domínios, invariantes
│                  100% constexpr, ZERO I/O, testado por static_assert
├── ports/         interfaces: FeedReader, BookSink, ClockSource
├── adapters/      leitura do ambiente: arquivos, variáveis, contadores
├── application/   casos de uso: montar o livro, medir, relatar
└── cli/           apresentação e relatório de evidência
```

As camadas `domain/ports/adapters/application/cli` são do **capstone**, não de
cada tópico. Um tópico que compara duas estruturas não tem caso de uso nem
adaptador: tem duas implementações, uma spec e um benchmark. Impor as cinco
camadas a ele produziria diretório vazio, que é autodescrição falsa (seção 5).

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
| Arquiteturas no escopo | **x86-64 apenas** |
| Dependências de sistema | **nenhuma além do compilador**, do Meson e do Ninja |

A escolha de C++23 estrito é o que torna `[[assume]]` o mecanismo de contrato, e
não Contracts — ver seção 13.

**arm64 saiu do escopo, e a saída é a correção de uma promessa sem verificador.**
O documento de origem da trilha dizia "arm64 compila, sem números publicados".
Não existe máquina arm64 neste projeto, e nenhuma suíte compila para arm64 — uma
arquitetura "no escopo" que nada exercita é autodescrição falsa (seção 5). Os
ramos por arquitetura de `rdtsc` e de `_mm_pause` continuam existindo em
`lib/measurement/`, porque são corretude de código portável, não promessa de
plataforma. arm64 volta ao escopo no dia em que houver máquina ou *runner* de CI
que o compile.

### Os sanitizers são dois perfis, não um

O documento de origem da trilha previa "um perfil `sanitize` com ASan, UBSan e
TSan". **ASan e TSan não coexistem no mesmo binário** — o próprio compilador
recusa. São dois perfis, e a distinção tem consequência de método:

| Perfil | Ferramenta | Onde é obrigatório |
|---|---|---|
| `sanitize-address` | ASan + UBSan | todo tópico com alocador, arena ou pool próprios |
| `sanitize-thread` | TSan | todo tópico do módulo de filas e concorrência |

TSan não entende ordenação relaxada de algoritmo *lock-free* escrito à mão: ele
reporta o que observou, e o que ele **não** reporta não é prova de correção. A
suíte usa TSan como filtro, e o argumento de correção continua vindo da norma
citada por cláusula (seção 9), não da ausência de aviso.

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
| **Regra de decisão** | obrigatória **se o tópico compara** (seção 35) |
| **Quando dá errado** | obrigatória **se houver invariante**, e respondida com experimento |
| Análise | obrigatória **se houver medição** |
| Confronto com a literatura | obrigatória **se houver experimento relevante** (seção 26) |
| Trade-offs, Quando utilizar, Quando não utilizar | **obrigatória** |
| Limitações | **obrigatória** |
| Exercícios | **obrigatória** |
| Referências, Navegação | **obrigatória** |

A versão inglesa tem exatamente a mesma estrutura conceitual: Objectives,
Problem, Mental model, Fundamentals, How it works, Baseline implementation,
Experiment, Execution environment, Results, Hardware counters, **What this
measurement does not show**, Analysis, Literature comparison, **Decision rule**,
**When it goes wrong**, Trade-offs, When to use, When not to use, Limitations,
Exercises, References, Navigation.

*Quando dá errado* não é a mesma seção que *Limitações*: limitação é o que os
números não autorizam concluir; *quando dá errado* é o que o sistema faz fora do
caminho feliz — a arena que esgota no meio de um lote, a fila que enche, o preço
fora da faixa do array, a mensagem truncada. A regra herdada do DPDK Academy é
que ela se responde com programa, nunca em prosa, e que **a invariante tem teste
negativo**: a versão que a viola existe e a suíte exige que ela falhe.
Verificação que nunca falhou é indistinguível de verificação que nunca dispara.

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

E acrescenta-se o que a máquina de referência obriga a acrescentar:

| Registro | Por que ele, e não só o que se costuma registrar |
|---|---|
| **CCD do núcleo, e o par SMT** | a máquina tem dois domínios de L3; um par de núcleos no mesmo CCD e um par em CCDs diferentes dão resultados distintos para a **mesma** estrutura. "Núcleo 3" não identifica o experimento; "núcleo 3, CCD 0, par SMT 15" identifica |
| **precisão e faixa do histograma** | um p99,9 publicado sem a precisão do instrumento é um número sem barra de erro |
| **variação entre execuções** | a campanha roda o experimento inteiro mais de uma vez, e publica a variação. Uma tabela de percentis de uma única execução não distingue a cauda da estrutura da cauda da máquina |

A última é herança direta de um defeito do DPDK Academy: o critério de validade
estava publicado e a calibração dele não estava em lugar nenhum. Aqui a
calibração é entregável da campanha, não nota de rodapé.

Variável não conhecida é documentada como não conhecida.

## 29. Ambiente de referência

O registro é **gerado** por `scripts/ambiente.sh`, em vez de descrito em prosa
pelo autor. Estado do script neste repositório: **código existe, não medido** —
ele coleta em três modos (`texto`, `--markdown`, `--json`), e o `--json` é o que
alimenta o `metadata.json` de cada medição. A razão está no script de origem: descrição em prosa diverge
entre arquivos sem que ninguém perceba, envelhece em silêncio quando o kernel ou
o compilador mudam, e não diz a quem reproduz o que comparar.

Registra pelo menos: modelo de CPU, microarquitetura quando identificável,
núcleos, threads, SMT, topologia NUMA, topologia de cache **com os domínios de
L3 e o CCD de cada núcleo**, RAM, kernel, governor, turbo, compilador e versão,
Meson, Ninja, perf, `perf_event_paranoid`, e se o TSC é invariante
(`constant_tsc`, `nonstop_tsc`). Quando relevante: mitigations, hugepages,
isolamento de CPU, frequência, estado térmico.

**A máquina de referência deste projeto** é um AMD Ryzen 9 9900X — 12 núcleos,
24 *threads*, SMT ativo, **dois domínios de L3**, um nó NUMA, TSC invariante. As
duas consequências que atravessam a trilha: o par de núcleos escolhido é
variável do experimento, não detalhe de execução (seção 28); e a pergunta de
falha "o que acontece quando o TSC não é invariante" **não é reproduzível
aqui** — fica registrada como questão em aberto (seção 7), não como experimento
prometido.

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
| geração de código, `constexpr`, template | aqui, seção 36 | veio do projeto absorvido, e virou módulo transversal |
| **onde a estrutura da biblioteca padrão para** | aqui | **é o objeto próprio deste projeto** — seções 34 e 35 |

**Esta seção legislava sobre este repositório antes de ele existir.** Ela foi
escrita no projeto absorvido, e atribui ao DPDK Academy cache, TLB, NUMA e falso
compartilhamento — que são o assunto dos módulos de memória e de layout daqui. A
regra que resolve não mudou, e é a que abre a seção: **cita e estende; não
reensina.** Na prática, um módulo daqui não reapresenta o custo do *cache miss*:
ele cita o número já medido no DPDK Academy e mede o que a estrutura de C++ faz
com esse custo. Quando o número citado não serve — outra máquina, outro
compilador, outra carga —, o módulo remede e diz por que o citado não servia.

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
| `verificar-suposicao.py` | a pré-condição da porta R chega ao otimizador, conferida no assembly, com braço de controle |
| `verificar-paridade.py` | o par de idiomas existe, a navegação aponta para ele, a estrutura é a mesma e nenhum número publicado falta num dos idiomas |

Os quatro primeiros vêm do DPDK Academy, onde **cada um nasceu de um defeito
medido**; o quinto nasceu no projeto absorvido, de um defeito dele — o Clang
descartava a suposição em silêncio. O sexto nasceu aqui, e nasceu desta seção:
ela registrava que a regra do radical idêntico existia para ser verificável e
não era verificada. Todos têm autoteste. A razão de portar em vez de escrever está no cabeçalho de
`verificar-retratacoes.py`: no defeito que o originou, quem escreveu a retratação
sabia da regra, tinha acabado de enunciá-la, e ainda assim deixou o valor
derrubado circulando em outras páginas. **Correção manual não escala.**

**Faltam doze, e a conta é para ser lida.** O DPDK Academy tem 16 verificadores
em `ferramental/qualidade/`, dentro de 23 programas de qualidade e 6.827 linhas.
Quatro deles foram portados: faltam doze. Os dois que mais importam, com o
acoplamento já medido, são `verificar-promessa.py` (todo programa citado existe e
entra na compilação, 18 pontos de acoplamento) e `verificar-autodescricao.py` (o
que o material afirma sobre si corresponde ao disco, 838 linhas e 34 pontos) —
este último é o que sustenta a rotulagem de estado da seção 5, e enquanto ele não
existir a seção 5 é promessa de autor, não portão. Falta também o **gerador de
gráficos**, porque dispersão de percentil alto se lê em gráfico e a paridade
PT/EN vale para as imagens. Estado: **não portados** — registrado no
[ROADMAP.md](../ROADMAP.md).

A ordem de portar é por necessidade real, não em bloco: portar 6.800 linhas
antes de existir conteúdo a verificar é promessa sem código, que é o que esta
norma proíbe.

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

e a cadeia deste projeto acrescenta um elo, que é o seu:

```text
o que a biblioteca padrão entrega → o ponto medido em que ela para
                                  → o complemento, sob o mesmo contrato
                                  → a regra de decisão
```

O objetivo não é ensinar **como escrever C++ rápido**. É ensinar:

> **como investigar cientificamente por que determinado software C++ apresenta
> determinado comportamento de desempenho** — e, com isso, **quando** trocar o
> mecanismo que a linguagem dá, não só como.

Essa filosofia permanece consistente entre DPDK Academy, este projeto e
Messaging Academy, formando uma família coerente de estudos sobre **High
Performance Systems Engineering**. A comparabilidade entre os três depende de
uma régua de apuração compartilhada, e é por isso que `lib/` é tratado como
candidato a repositório próprio desde já: a fronteira tem de ser **ABI C**, ou a
extração nunca acontece — o DPDK Academy é escrito em C, e um harness em
`<expected>` e template não é consumível de lá.

---

# Parte VII — O par std e custom

Esta parte não existia na norma absorvida. Ela é o que este repositório tem e o
projeto de origem não tinha: a trilha, e a unidade de trabalho dela.

## 34. O par std e custom como unidade de trabalho

> Um tópico não é uma estrutura. É **um par de estruturas sob a mesma lei**,
> medido no mesmo harness, na mesma máquina, com o mesmo contrato.

A ordem é fixa e não é preferência: **primeiro o que a linguagem dá.** Nenhum
`custom/` se escreve antes de o `std/` correspondente estar medido e publicado.
Um complemento proposto contra um baseline não medido é opinião com código.

Três resultados são possíveis por tópico, e **todos os três se publicam**:

| Resultado | O que o leitor aprende |
|---|---|
| a biblioteca padrão basta | qual recurso usar, e por que a alternativa artesanal não compensa |
| basta até um volume ou percentil | a regra de decisão: em que ponto trocar (seção 35) |
| não serve no caminho crítico | o mecanismo que a substitui, e o que ele custa em troca |

O segundo e o terceiro são os esperados. **O primeiro é o que dá crédito aos
outros dois**, e por isso tem índice próprio junto com os resultados negativos
da seção 12: em vários casos `string_view` com `from_chars` já é a resposta, e o
decodificador artesanal não compensa.

### `spec.hpp` é a lei, e `contract.hpp` é outra coisa

Duas palavras colidiam. Nesta norma, **contrato** é o mecanismo das três portas
da seção 13 — `PERF_EXPECTS`, `[[assume]]`, `lib/contract/`. No documento de
origem da trilha, "contrato" era a interface comum que `std/` e `custom/`
precisam cumprir. Dois conceitos, um nome, no mesmo repositório.

O nome da interface do tópico é **`spec.hpp`**. O de `lib/contract/` fica, porque
é o que `-Werror=assume` e `verificar-suposicao.py` já nomeiam.

### A divergência de contrato é achado, e é declarada em código

`std::map<Price, Level>` e um array plano indexado por *tick* **não** cumprem a
mesma lei: diferem em ordem de iteração, estabilidade de referência e domínio de
preço. Isso não é defeito do tópico, é o assunto dele.

Regra: a `spec.hpp` declara as invariantes, e **cada braço declara em `traits`
quais delas honra**. Os testes L1 são parametrizados pela spec e consultam os
traits; uma invariante que um braço não honra é um teste **pulado com motivo**,
não um teste ausente. A divergência fica no código, verificável, em vez de na
prosa, onde envelhece sem que ninguém perceba.

> Uma spec que os dois braços cumprem sem ressalva costuma ser sinal de que a
> spec foi escrita fraca o suficiente para caber nos dois.

## 35. Regra de decisão

Seção obrigatória em todo tópico que compara, e **é o artefato de referência
interna do projeto** — o que se consulta ao projetar um sistema real, e não ao
estudar.

A regra responde a uma pergunta só: **a partir de que ponto a troca compensa?**
O ponto é um número — volume, cardinalidade, taxa de chegada, percentil —, não
um adjetivo.

Cada regra publicada declara, no mínimo:

| Campo | Exemplo |
|---|---|
| a estrutura de partida e a de troca | `std::map<Price, Level>` → array plano por *tick* com bitmap |
| a variável de decisão e o ponto de virada | cardinalidade de níveis ativos, acima de N |
| o percentil em que a virada aparece | aparece em p99,9 antes de aparecer em p50 |
| a máquina, a configuração de build e o *commit* | ambiente gerado, não descrito (seção 29) |
| o que a regra **não** cobre | a carga, o padrão de acesso e a faixa fora dos quais ela não vale |

A tabela consolidada vive em `docs/regras-de-decisao.md`, e **cada linha dela
aponta para um `metadata.json` existente**. A tabela é verificada por máquina,
como as demais afirmações numéricas da seção 8: uma regra de decisão sem
medição arquivada é opinião com aparência de referência, e é o tipo de defeito
mais caro que este projeto pode publicar, porque é o que alguém usaria para
decidir.

> Uma regra de decisão que não diz onde deixa de valer não é regra: é slogan.

## 36. O eixo do código gerado

Objeto próprio do projeto absorvido, preservado aqui como **módulo transversal**
em vez de diluído nos outros. A pergunta dele é a que os módulos de estrutura
não fazem:

> **o que o compilador faz com `custom/` que ele não faz com `std/`?**

O que este eixo traz, e o que ele não é:

- traz `constexpr`, template, `[[assume]]` e o deslocamento de trabalho para
  tempo de compilação como **hipótese medida**, nunca como regra de estilo
  (seção 14);
- traz a varredura em vez do ponto único, porque a variável sob teste interage
  com a hierarquia de memória;
- traz o custo de compilar entre as métricas;
- **não** é comparação entre compiladores, nem catálogo de *flags*: o par
  continua sendo `std` e `custom`, e o compilador é o instrumento, não o objeto.

O verificador que sustenta este eixo já existe: `verificar-suposicao.py` compila
uma sonda e confere no assembly, com braço de controle, que a pré-condição chega
ao otimizador. Ele nasceu de um defeito medido — o Clang descartava a suposição
em silêncio —, e é a prova de que neste eixo a afirmação sobre código gerado se
verifica lendo código gerado.

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

## Divergências em relação ao documento de origem da trilha

Segunda tabela, e segundo documento de origem:
[`origem/setup-lowlatency-structures-academy.md`](origem/setup-lowlatency-structures-academy.md),
também preservado intacto. As correções abaixo foram verificadas nesta máquina
antes de entrar na norma.

| # | Origem | Correção |
|---|---|---|
| 1 | §2.2 dizia que as dependências de teste vêm "fixadas por hash em `subprojects/*.wrap`, com `.wraplock`" | o `.wraplock` **não existe** no DPDK Academy, e em Meson `.wraplock` é o lock de diretório usado durante o download, não um lockfile de dependências a versionar. O pin por hash já está no `.wrap`; a menção sai |
| 2 | §7 previa "HdrHistogram_c, wrap fixado por hash" | **não está no WrapDB** — consulta ao `releases.json` em 2026-09-21: 367 projetos, nenhum de histograma. O wrap tem de ser escrito à mão, com `meson.build` próprio, porque o projeto é CMake; e o *log writer* dele arrasta zlib, o que colide com "nenhuma dependência além do compilador" (seção 16). Decisão registrada no ROADMAP, etapa do harness |
| 3 | §7 previa "um perfil `sanitize` com ASan, UBSan e TSan" | ASan e TSan não coexistem: dois perfis (seção 16) |
| 4 | §2.2 descrevia o ferramental de qualidade como "verificadores de âncora, autodescrição e paridade" | são 23 programas e 6.827 linhas no DPDK Academy; portar é a maior tarefa do projeto, não um item de checklist (seção 31) |
| 5 | §4 dava a `docs/0N-*` só teoria, sem diretório de código | no DPDK Academy os programas que produzem número de teoria moram em `docs/0N-*/medicoes/`. Sem isso, ou a teoria é prosa sem programa — o que a seção 8 proíbe — ou o diretório falta. Fica `docs/0N-*/medicoes/` |
| 6 | §7 punha arm64 no escopo, "compila, sem números publicados" | não há máquina nem *runner*; arm64 sai do escopo até haver um (seção 16) |
| 7 | §6 prometia o experimento "o que acontece quando o TSC não é invariante" | a máquina de referência tem `constant_tsc` e `nonstop_tsc`: não é reproduzível aqui, e fica como questão em aberto (seções 7 e 29) |
| 8 | §8 listava o que cada medição registra, sem CCD, sem precisão de histograma e sem variação entre execuções | os três entram, e o terceiro é herança de um defeito do DPDK Academy: critério de validade publicado com a calibração em lugar nenhum (seção 28) |
| 9 | §10 previa a extração de `ferramental/bench/` como etapa final, consumida também pelo DPDK Academy | o DPDK Academy é C; a fronteira tem de ser ABI C **desde já**, ou a extração não acontece (seção 33) |
| 10 | §5 chamava de `contract.hpp` a interface comum de `std/` e `custom/` | colisão com o mecanismo das três portas; a lei do tópico é `spec.hpp` (seção 34) |
| 11 | §4 previa `ferramental/bench/` para o harness | o harness absorvido é `lib/measurement/`, e `lib/` é a fronteira de extração. `ferramental/` fica só para qualidade e gráficos (seção 15) |
| 12 | §1.2 dizia que o livro de ofertas é "cenário recorrente, não objeto de estudo", e a trilha lhe dá um módulo inteiro mais o capstone | pendente de decisão editorial, registrada no ROADMAP: ou o README assume o livro como protagonista, ou o módulo 04 encolhe |

## Navegação

- [Catálogo de referências](referencias.md)
- [Índice da documentação](README.md)
- [README do repositório](../README.md)
- [Documento de origem desta norma, intacto](origem/setup-cpp-performance-academy.md)
- [Documento de origem da trilha, intacto](origem/setup-lowlatency-structures-academy.md)

> 🇧🇷 Português | [🇺🇸 English](padrao-do-projeto.en.md)
