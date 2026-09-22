> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 01.01 — Alocador de arena contra o alocador global

**Estado: medido.** Há número publicado, com ambiente gerado e programa que o
produz, arquivado em
[`bench/medicoes/`](bench/medicoes/historico/2026-09-22-arena-contra-heap/).

## Objetivos

Medir a que taxa de alocação o alocador global passa a dominar o custo de um
lote, e a que ponto a troca por uma arena compensa — com o número que sustenta
a regra.

## Problema

O padrão do caminho quente deste projeto é **um lote nasce, é usado, e morre
inteiro**: mensagens chegam, viram objetos, são processadas, somem. Nesse
padrão, `new`/`delete` paga duas coisas de que o lote não precisa — encontrar
um bloco livre para cada objeto, e devolver cada um separadamente.

A pergunta não é se dá para fazer melhor. É **a partir de onde** faz diferença.

## Modelo mental

| | `std::allocator` | arena |
|---|---|---|
| alocar | procura bloco livre, atualiza estruturas do alocador | soma um índice |
| liberar | devolve objeto por objeto | zera o índice |
| teto | não tem; lança quando a memória acaba | declarado, e devolve `nullptr` |
| liberar um só | **sim** | **não** |

As duas últimas linhas são a troca inteira. Tudo o que a arena ganha vem de ter
desistido da liberação individual.

## Como funciona

A [`spec.hpp`](spec.hpp) declara **sete invariantes**. Cinco valem para os dois
braços; duas valem só para quem as honra, e cada braço diz em `traits` quais são
as suas. O teste L1 é parametrizado pela spec e consulta os `traits`: a
invariante que um braço não honra vira caso **pulado com motivo**, nunca caso
ausente.

Isso não é frouxidão da spec — é o assunto do tópico. Exigir liberação
individual dos dois produziria uma spec que só a `std::allocator` cumpre, e a
comparação viraria *"a arena é uma `std::allocator` pior"*.

## Implementação

O braço [`std/`](std/heap_arm.hpp) usa `std::allocator<node>`, e não `new`
direto: é a forma que a biblioteca padrão oferece a quem quer trocar de alocador
depois. O braço [`custom/`](custom/arena_arm.hpp) reserva o buffer **uma vez**,
no construtor, fora de qualquer medição — uma arena que crescesse sob demanda
chamaria o alocador global no meio do lote, e passaria a medir, de vez em
quando, aquilo de que ela existe para fugir.

**Nenhum dos dois guarda a lista de ponteiros.** Quem guarda é o chamador, igual
para os dois. Se cada braço guardasse os seus, o `std` pagaria também o custo de
manter a lista, e a tabela publicaria como "custo de alocar" algo que é custo de
escriturar (norma, seção 11).

O objeto tem **64 bytes** e `alignas(64)`: uma linha de cache exata. Menor que
isso, dois objetos vizinhos dividiriam linha e o tópico passaria a medir também
falso compartilhamento, que é assunto do módulo 02.

## Experimento

Campanha de **5** execuções, em `release`, com **10000** amostras por ponto e
**64** lotes de aquecimento — iguais nos dois braços, porque sem isso a primeira
alocação de cada um paga a primeira falta de página do seu buffer e a tabela
publicaria a diferença de aquecimento com o nome da estrutura.

Cada amostra é **um lote completo**: adquirir L nós, escrever em cada um,
devolver todos. O relógio é lido duas vezes por lote, e não por objeto — o piso
por operação desta máquina é de **20** ns ([08.01](../../08-medicao/01-harness/README.md))
e uma alocação custa da ordem de 25 ns; medir objeto a objeto publicaria uma
tabela em que metade do número é o relógio.

**Varredura, e não ponto único**: o tamanho do lote é a variável.

```bash
./scripts/build-all.sh release
./scripts/arquivar-medicao.sh docs/01-memoria/01-alocador-arena bench-arena arena-contra-heap 5
```

## Ambiente de execução

Gerado, não descrito:
[`ambiente.md`](bench/medicoes/historico/2026-09-22-arena-contra-heap/ambiente.md)
e o mesmo conteúdo dentro de
[`metadata.json`](bench/medicoes/historico/2026-09-22-arena-contra-heap/metadata.json).

## Resultados

| Métrica | Mediana entre execuções | Amplitude entre execuções | Unidade |
|---|---:|---:|---|
| std::allocator, lote 8 — p50 | 60,00 | 33,3% | ns |
| arena, lote 8 — p50 | 20,00 | 50,0% | ns |
| std::allocator, lote 64 — p50 | 1673,00 | 26,4% | ns |
| arena, lote 64 — p50 | 60,00 | 50,0% | ns |
| std::allocator, lote 64 — p99 | 1703,00 | 145,3% | ns |
| arena, lote 64 — p99 | 71,00 | 28,2% | ns |
| std::allocator, lote 512 — p50 | 14828,00 | 3,9% | ns |
| arena, lote 512 — p50 | 371,00 | 5,1% | ns |
| std::allocator, lote 512 — p99 | 18966,00 | 35,6% | ns |
| arena, lote 512 — p99 | 481,00 | 22,9% | ns |
| std::allocator, lote 4096 — p50 | 88388,00 | 1,2% | ns |
| arena, lote 4096 — p50 | 3576,00 | 82,1% | ns |
| std::allocator, lote 4096 — p99 | 91704,30 | 12,0% | ns |
| arena, lote 4096 — p99 | 4458,00 | 62,5% | ns |

## O que esta medição não mostra

- **o que o benchmark removeu:** um único fluxo, sem outra *thread* disputando o
  alocador. É o melhor caso do `std::allocator`, e favorece **ele**: o alocador
  global tem cache por *thread*, e é sob contenção que ele piora;
- **a qual braço isso favorece:** ao `std`, nos dois eixos — sem contenção e com
  o lote sempre do mesmo tamanho, que é o padrão que mais ajuda o *free list*;
- **qual conclusão o leitor tiraria se parasse na tabela:** que a arena é ~25 a
  40 vezes mais rápida. É verdade **neste padrão de uso** e diz pouco fora dele;
- **o que seria necessário** para a conclusão mais ampla: lotes de tamanho
  variável, objetos de tamanhos diferentes, e mais de uma *thread*.

## Quando dá errado

**A arena esgota no meio de um lote.** É a pergunta de falha do módulo, e a
resposta está na invariante 6: `acquire()` devolve `nullptr`, e continua
devolvendo — esgotamento não é estado transitório. O braço `std` **não honra
essa invariante**, e o caso dele é pulado com esse motivo escrito: quando a
memória acaba, ele lança.

No benchmark, `nullptr` no meio do lote é **erro de medição**, não caminho a
medir: o programa reprova em vez de publicar um lote incompleto como se fosse um
lote.

## Análise

**O custo por nó do `std::allocator` não é constante, e é isso que responde a
pergunta do módulo.** Ele vai de **7,50** ns no lote 8 para **26,14** ns no lote
64 e **28,96** ns no lote 512. O lote pequeno recicla os mesmos poucos blocos,
que continuam quentes no cache do alocador; a partir de algumas dezenas, cada
alocação passa a tocar bloco novo.

A arena anda no sentido contrário: **2,50** ns por nó no lote 8, **0,72** no lote
512. Ela amortiza — o que ela faz por objeto é somar um índice, e o resto do
custo do lote é a escrita nos objetos, que os dois braços pagam igual.

**No lote 8, a arena está no piso do instrumento.** Os 20,00 ns medidos são
exatamente o piso por operação publicado em [08.01](../../08-medicao/01-harness/README.md).
O número não descreve a arena: descreve o relógio. A linha da tabela fica, com
esta ressalva ao lado — e é a razão de a regra de decisão não começar em 8.

A razão entre os braços vai de **27,9×** no p50 do lote 64 a **40,0×** no lote
512, e cai para **24,7×** no lote 4096, quando o conjunto de trabalho deixa de
caber no cache e os dois braços passam a esperar memória.

## Regra de decisão

> **Acima de ~64 alocações por lote, trocar o alocador global pela arena vale
> uma ordem de grandeza — e o ganho é maior no p99 que no p50.**

| Campo | Valor |
|---|---|
| estrutura de partida → troca | `std::allocator<node>` → arena com teto declarado |
| variável de decisão | alocações por lote |
| ponto de virada | **64** — abaixo disso a medição não distingue a arena do instrumento |
| ganho no ponto de virada | **27,9×** no p50, **24,0×** no p99 |
| percentil em que aparece | os dois; no lote 512 o p99 do `std` é 39,4× o da arena |
| máquina, configuração, commit | ver o [`metadata.json`](bench/medicoes/historico/2026-09-22-arena-contra-heap/metadata.json) |
| **o que a regra não cobre** | lote de tamanho variável, objetos de tamanhos diferentes, mais de uma *thread*, e qualquer uso que precise liberar objeto individual |

A última linha é a que importa mais: **a regra só vale onde o lote morre
inteiro**. Fora disso, a invariante 7 — que a arena não honra — deixa de ser
detalhe e passa a ser impedimento.

## Confronto com a literatura

A forma da curva confere com o que se documenta sobre o alocador da glibc: um
cache por *thread* atende os blocos pequenos e recém-liberados sem trava, e é
por isso que o lote 8 é mais barato **por nó** que o lote 64. O que este tópico
acrescenta não é o mecanismo, é o número desta máquina e o **ponto** em que ele
deixa de bastar.

Fica em aberto se o joelho entre 8 e 64 é o limite do cache por *thread* ou o do
próprio cache de dados. Separar as duas exige contador de hardware, que é o
módulo [08.02](../../08-medicao/README.md).

## Trade-offs

| Escolha | Ganho | Preço |
|---|---|---|
| arena | alocação em tempo constante e devolução em O(1) | sem liberação individual; teto a dimensionar |
| `std::allocator` | uso geral, sem teto, libera individual | custo por objeto que cresce com o lote |

## Quando utilizar

Quando o lote nasce e morre inteiro, e o tamanho máximo é conhecido — que é o
caso do decodificador de mensagens e do livro de ofertas.

## Quando não utilizar

Quando um objeto precisa sobreviver aos irmãos. Aí a arena não serve, e a saída
não é "arena com liberação individual" — isso é um *pool* com lista de livres,
que é o [tópico 01.02](../README.md).

## Limitações

Uma máquina, um compilador, um fluxo, objeto de tamanho fixo. `governor` em
`powersave`. O ponto de virada de 64 é **desta** máquina: em outra, com outro
alocador de sistema, ele se move — e o programa que o encontra está aqui.

## Exercícios

1. Rode a varredura com lotes 16 e 32 e ache o joelho com mais precisão. Ele
   está mais perto de 8 ou de 64?
2. Troque o `node` de 64 para 32 bytes e refaça. O ponto de virada anda?
3. Faça a arena crescer sob demanda em vez de reservar no construtor. Qual
   percentil sente primeiro?

## Referências

- [Norma do projeto](../../padrao-do-projeto.md), seções 11, 34 e 35
- [08.01 — o harness medindo a si mesmo](../../08-medicao/01-harness/README.md)
- [Módulo 01 — memória](../README.md)

## Navegação

- [Módulo 01](../README.md)
- [Plano de estudo](../../plano-estudo.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
