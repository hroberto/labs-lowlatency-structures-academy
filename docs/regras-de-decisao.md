> 🇧🇷 Português | [🇺🇸 English](regras-de-decisao.en.md)

# Regras de decisão

> **Estado: 3 regras.** Uma por tópico que compara. Esta página é o artefato de
> **referência interna** do projeto — o que se consulta ao projetar um sistema,
> e não ao estudar.

Cada linha aqui sai de uma campanha arquivada, e cada campanha tem programa,
ambiente gerado e *commit*. Nenhuma regra é opinião com aparência de referência:
a coluna **campanha** é onde se confere.

## Como ler esta tabela

O **ponto de virada** é um número — volume, cardinalidade, taxa de chegada ou
percentil —, nunca um adjetivo. Onde ele está vazio, a troca vale em todo o
intervalo medido, e o intervalo está dito.

A coluna que decide o uso é a **última**. Uma regra sem o limite dela não é
regra: é slogan. Antes de aplicar qualquer linha, leia o que ela não cobre.

**Todos os números são desta máquina** — AMD Ryzen 9 9900X, `governor` em
`powersave`, um fluxo, GCC 15.2 em `release`. O que transfere entre máquinas é a
**forma** da curva e o mecanismo; o ponto de virada, não necessariamente.

## As regras

| # | Partida → troca | Variável | Ponto de virada | Ganho | Campanha |
|---|---|---|---|---|---|
| [01.01](01-memoria/01-alocador-arena/README.md) | `std::allocator` → arena | alocações por lote | **64** | 27,9× no p50, 24,0× no p99 | [2026-09-22](01-memoria/01-alocador-arena/bench/medicoes/historico/2026-09-22-arena-contra-heap/) |
| [01.02](01-memoria/02-pool-freelist/README.md) | `std::allocator` → pool com lista de livres | tamanho do conjunto vivo | — (vale de 64 a 4096) | 3,0× no p50 e 2,9× no p99 | [2026-09-22](01-memoria/02-pool-freelist/bench/medicoes/historico/2026-09-22-pool-contra-heap/) |
| [01.03](01-memoria/03-pmr-o-meio-termo/README.md) | `std::allocator` → `std::pmr::monotonic_buffer_resource` | alocações por lote | **64** | 25,0× no p50; a arena artesanal acrescenta 1,59× | [2026-09-22](01-memoria/03-pmr-o-meio-termo/bench/medicoes/historico/2026-09-22-pmr-contra-arena/) |

## O que cada regra **não** cobre

| # | Fora do alcance |
|---|---|
| 01.01 | lote de tamanho variável, objetos de tamanhos diferentes, mais de uma *thread*, e qualquer uso que precise liberar objeto individual |
| 01.02 | conjunto vivo que cresce, objetos de tamanhos diferentes, liberação aleatória em vez de cíclica, mais de uma *thread* |
| 01.03 | vários tipos de objeto, `pmr` usado com contêineres em vez de `allocate` direto, escolha de recurso em tempo de execução, mais de uma *thread* |

Três dos três dizem **mais de uma *thread***. Não é coincidência: o alocador
global tem cache por *thread*, e é sob contenção que ele piora. Todas as três
regras foram medidas no **melhor caso do baseline**, e portanto são
conservadoras — sob contenção, a troca tende a valer mais, não menos.

## O que as três juntas dizem

Lidas em sequência, elas contam algo que nenhuma diz sozinha:

**O ganho da arena não é o ganho de escrever uma arena.** O 01.01 mediu 27,9×
sobre `std::allocator`, e o 01.03 mostrou que `std::pmr` — que já vem na
linguagem — entrega 25,0× dos mesmos 27,9×. O que o código artesanal acrescenta
é **1,59×**, e não uma ordem de grandeza.

**A maior parte do ganho vem de desistir da liberação individual, e não de
escrever código.** O 01.02 mede o pool, que faz por dentro quase o mesmo que a
arena — ler um índice em vez de somar um — e ganha **3,0×** em vez de 27,9×. A
diferença entre os dois números é o preço de o *baseline* também ter de liberar
um a um.

Daí a ordem prática: **veja se o seu padrão permite jogar tudo fora de uma vez.**
Se permitir, `std::pmr` resolve a maior parte. Se não permitir, o pool dá 3× e
não há como fugir disso sem mudar o padrão.

## Navegação

- [Documentação e norma](README.md)
- [Plano de estudo](plano-estudo.md)

> 🇧🇷 Português | [🇺🇸 English](regras-de-decisao.en.md)
