> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 01.02 — Pool com lista de livres

**Estado: medido.** Há número publicado, com ambiente gerado e programa que o
produz, arquivado em
[`bench/medicoes/`](bench/medicoes/historico/2026-09-22-pool-contra-heap/).

## Objetivos

Medir **quanto custa manter a liberação individual** — o que a
[arena](../01-alocador-arena/README.md) não paga porque não oferece — e a partir
de onde o pool compensa o alocador global.

## Problema

O [01.01](../01-alocador-arena/README.md) fechou dizendo onde a arena **não**
serve: quando um objeto precisa sobreviver aos irmãos. Este tópico começa
exatamente daí.

O padrão agora é **rotatividade em regime**: um conjunto de objetos vivos de
tamanho aproximadamente constante, em que cada passo libera um e adquire outro —
e a ordem de liberação **não** é a de aquisição. É o padrão de uma tabela de
registros que entra e sai, e a arena não o atende.

## Modelo mental

O slot livre guarda o índice do próximo slot livre. Adquirir é desempilhar a
cabeça; liberar é empilhar nela.

| | `std::allocator` | pool |
|---|---|---|
| adquirir | procura bloco livre nas estruturas do alocador | lê um índice |
| liberar | devolve ao alocador | escreve um índice |
| teto | não tem | declarado |
| onde os vivos ficam | onde o alocador decidir | contíguos, por construção |

**A lista é LIFO, e isso não é detalhe:** o slot devolvido é o próximo a sair, e
ele acabou de ser tocado — está quente. Uma lista FIFO devolveria o slot mais
frio, e a diferença apareceria no p50.

## Como funciona

A [`spec.hpp`](spec.hpp) deste tópico tem **cinco invariantes**, e a primeira
mudou de forma em relação à do 01.01. Lá, *"cada `acquire` devolve ponteiro
distinto"* valia para sempre, porque o lote não libera nada até o fim. Aqui um
slot liberado **deve** voltar a ser servido — é a invariante 4 —, então dois
ponteiros iguais em momentos diferentes são corretos, e dois ponteiros iguais ao
mesmo tempo são defeito.

A invariante virou *"distintos **enquanto vivos**"*, e o teste acompanha o
conjunto vivo em vez de acumular tudo. Uma cópia do teste do 01.01 passaria aqui
sem verificar nada do que interessa.

E a liberação individual, que lá era invariante **condicional**, aqui está no
`concept`: um braço que não a ofereça **não compila**. A divergência entre os
dois tópicos virou erro de build em vez de nota de rodapé.

## Implementação

O índice da lista tem **32 bits**, e não é ponteiro. O pool tem teto declarado,
então o índice basta — e num pool de 4096 slots a lista inteira ocupa 16 KB e
fica no L1, contra 32 KB em ponteiros.

O braço `std` é o **mesmo** `std::allocator` do 01.01, com a interface que esta
spec exige e nada mais. Reescrevê-lo daria a chance de escrevê-lo diferente, e
dois baselines diferentes no mesmo módulo tornariam os dois tópicos
incomparáveis.

## Experimento

Campanha de **5** execuções, em `release`, **10000** amostras por ponto, **64**
passos de rotatividade por amostra e **64** rodadas de aquecimento iguais nos
dois braços.

**A ordem de liberação é embaralhada por um passo coprimo com o conjunto vivo.**
Liberar sempre o último transformaria o padrão numa pilha, que é o melhor caso
dos dois braços — o alocador devolveria o bloco recém-liberado, e o pool o slot
recém-empilhado. A tabela publicaria o melhor caso com o nome de rotatividade.

A varredura é sobre o **tamanho do conjunto vivo**: 64 nós são 4 KB e cabem no
L1; 4096 são 256 KB e passam do L2 desta máquina.

```bash
./scripts/build-all.sh release
./scripts/arquivar-medicao.sh docs/01-memoria/02-pool-freelist bench-pool pool-contra-heap 5
```

## Ambiente de execução

Gerado, não descrito:
[`ambiente.md`](bench/medicoes/historico/2026-09-22-pool-contra-heap/ambiente.md).

## Resultados

| Métrica | Mediana entre execuções | Amplitude entre execuções | Unidade |
|---|---:|---:|---|
| std::allocator, 64 vivos — p50 | 481,00 | 20,8% | ns |
| pool, 64 vivos — p50 | 161,00 | 19,3% | ns |
| std::allocator, 64 vivos — p99 | 501,00 | 18,0% | ns |
| pool, 64 vivos — p99 | 171,00 | 128,7% | ns |
| std::allocator, 512 vivos — p50 | 421,00 | 21,4% | ns |
| pool, 512 vivos — p50 | 150,00 | 20,0% | ns |
| std::allocator, 512 vivos — p99 | 451,00 | 20,0% | ns |
| pool, 512 vivos — p99 | 151,00 | 26,5% | ns |
| std::allocator, 4096 vivos — p50 | 411,00 | 26,8% | ns |
| pool, 4096 vivos — p50 | 150,00 | 26,7% | ns |
| std::allocator, 4096 vivos — p99 | 431,00 | 23,2% | ns |
| pool, 4096 vivos — p99 | 151,00 | 27,2% | ns |

## O que esta medição não mostra

- **o que o benchmark removeu:** um único fluxo, conjunto vivo de tamanho fixo,
  objeto de tamanho único. O alocador global tem cache por *thread*, e é sob
  contenção que ele piora — aqui ele nunca é contestado;
- **a qual braço isso favorece:** ao `std`, de novo. Este é o melhor caso dele;
- **qual conclusão o leitor tiraria se parasse na tabela:** que o pool ganha
  ~3×. É verdade, e é **um décimo** do que a arena ganhava no 01.01 — e essa
  comparação é o assunto;
- **o que seria necessário** para a conclusão mais ampla: conjunto vivo que
  cresce e encolhe, objetos de tamanhos diferentes, e mais de uma *thread*.

## Quando dá errado

**A lista de livres esvazia.** A invariante 5 diz que `acquire()` devolve
`nullptr`, e continua devolvendo — e, depois de devolver os vivos, o pool volta
a servir a capacidade inteira. O teste exige as três coisas.

A capacidade no benchmark é **exatamente** o conjunto vivo, sem folga: o passo
libera antes de adquirir, então nunca há mais de N vivos. Dar folga esconderia o
regime em que a lista fica vazia, que é justamente o deste tópico.

## Análise

**O pool ganha 2,7× a 3,0×, e a comparação que importa não é essa.**

A [arena](../01-alocador-arena/README.md), no lote de 64, ganhava **27,9×**. O
pool, sob rotatividade, ganha **3,0×** — e as duas estruturas fazem quase a
mesma coisa por dentro: uma soma um índice, a outra lê um índice de uma lista.

A diferença entre 27,9× e 3,0× **é o preço da liberação individual**. Não é o
preço de implementá-la — é o preço de o baseline também tê-la. Quando o padrão
permite jogar tudo fora de uma vez, o `std::allocator` precisa devolver bloco a
bloco e a arena não precisa devolver nada; quando o padrão exige devolver um a
um, os dois devolvem um a um, e o que sobra para o pool é a diferença entre ler
um índice e percorrer as estruturas do alocador.

**O custo por passo quase não muda com o conjunto vivo**: 7,52 ns no `std` com
64 vivos, 6,42 com 4096; no pool, 2,52 e 2,34. Era de esperar que 4096 nós — 256
KB, acima do L2 — piorassem os dois, e não pioram. A rotatividade recicla sempre
os mesmos slots, e o conjunto de trabalho **efetivo** é muito menor que o
conjunto vivo. Fica como questão em aberto se o mesmo vale com liberação
aleatória em vez de cíclica.

## Regra de decisão

> **Sob rotatividade, o pool vale ~3× sobre o alocador global, em qualquer
> tamanho de conjunto vivo — e vale o mesmo no p50 e no p99.**

| Campo | Valor |
|---|---|
| estrutura de partida → troca | `std::allocator<node>` → pool com lista de livres |
| variável de decisão | tamanho do conjunto vivo |
| ponto de virada | **nenhum** — o ganho é ~3× de 64 a 4096, e não há tamanho em que o `std` alcance |
| ganho | **3,0×** no p50 e **2,9×** no p99 com 64 vivos; **2,7×** e **2,9×** com 4096 |
| percentil em que aparece | os dois, e na mesma proporção |
| máquina, configuração, commit | ver o [`metadata.json`](bench/medicoes/historico/2026-09-22-pool-contra-heap/metadata.json) |
| **o que a regra não cobre** | conjunto vivo que cresce, objetos de tamanhos diferentes, liberação aleatória, mais de uma *thread* |

**A regra tem um corolário que vale mais que ela:** se o seu padrão permite a
arena, use a arena — os 3× do pool são um décimo dos 27,9× dela. O pool é o que
se usa quando a arena **não** é possível.

## Confronto com a literatura

O achado de que o custo por passo quase não varia com o conjunto vivo é
consistente com o cache por *thread* da glibc: sob rotatividade cíclica, os
blocos recém-liberados voltam imediatamente e nunca saem da estrutura rápida do
alocador. A literatura de *pool allocator* costuma reportar ganhos de uma ordem
de grandeza; este tópico mede **3×**, e a diferença é que a maior parte da
literatura compara com alocação **sem** o padrão de reciclagem, que é o melhor
caso do alocador moderno.

## Trade-offs

| Escolha | Ganho | Preço |
|---|---|---|
| pool | ~3× sob rotatividade, vivos contíguos, teto declarado | 4 bytes de índice por slot; capacidade a dimensionar |
| `std::allocator` | uso geral, sem teto | ~3× mais caro neste padrão |

## Quando utilizar

Quando o conjunto vivo é limitado e conhecido, e os objetos entram e saem
individualmente.

## Quando não utilizar

Quando o lote morre inteiro — aí a [arena](../01-alocador-arena/README.md) é uma
ordem de grandeza melhor. E quando os objetos têm tamanhos diferentes: um pool
por tamanho vira gestão de vários pools, que é onde
[`std::pmr`](../README.md) entra.

## Limitações

Uma máquina, um compilador, um fluxo, objeto de tamanho fixo, conjunto vivo
constante, liberação cíclica e não aleatória.

## Exercícios

1. Troque a lista de LIFO para FIFO e refaça. Quanto do ganho era o slot quente?
2. Libere em ordem aleatória em vez de cíclica. O conjunto de trabalho efetivo
   cresce, e o ganho cai?
3. Meça com dois conjuntos vivos em duas *threads*. Qual braço sente primeiro?

## Referências

- [01.01 — alocador de arena](../01-alocador-arena/README.md)
- [08.01 — o harness medindo a si mesmo](../../08-medicao/01-harness/README.md)
- [Norma do projeto](../../padrao-do-projeto.md), seções 11, 34 e 35

## Navegação

- [Módulo 01](../README.md)
- [Plano de estudo](../../plano-estudo.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
