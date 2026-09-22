> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 01.03 — `std::pmr` como meio-termo

**Estado: medido.** Há número publicado, com ambiente gerado e programa que o
produz, arquivado em
[`bench/medicoes/`](bench/medicoes/historico/2026-09-22-pmr-contra-arena/).

## Objetivos

Responder se **a própria biblioteca padrão já oferece o mecanismo** que os dois
tópicos anteriores escreveram à mão — e, se oferece, quanto do ganho ela
entrega.

## Problema

Os tópicos [01.01](../01-alocador-arena/README.md) e
[01.02](../02-pool-freelist/README.md) compararam `std::allocator` com código
artesanal, e o artesanal ganhou 27,9× e 3,0×. Mas `std::allocator` **não é a
única coisa que a linguagem oferece**: desde o C++17 existe
`std::pmr::monotonic_buffer_resource`, que é uma arena.

Se ele entregar o mesmo, os dois tópicos anteriores continuam certos e ganham
uma ressalva cara: eles mediram o custo de `std::allocator`, não o custo do que
a linguagem oferece.

**A ordem é esta de propósito.** O meio-termo só se avalia depois de conhecidos
os dois extremos — caso contrário, "a  é boa" não tem contra o quê.

## Modelo mental

`monotonic_buffer_resource` aloca somando um ponteiro dentro de um buffer que se
dá a ele; `release()` devolve tudo de uma vez. É o desenho do 01.01, escrito
pelo comitê.

A diferença de dentro é que ele é **polimórfico**: cada `allocate` passa por
despacho virtual e por um caminho genérico de tamanho e alinhamento, porque ele
serve a qualquer tipo. A arena do 01.01 serve a um tipo só e sabe disso em
tempo de compilação.

## Como funciona

A spec é **a mesma do 01.01** — [`spec.hpp`](spec.hpp) apenas diz isso em voz
alta. O L1 roda as sete invariantes sobre os **três** braços, e é onde a
afirmação vira verificação: se a `pmr_arm` cumprisse uma lei ligeiramente
diferente, o arquivo não compilaria.

E o resultado do tópico aparece na saída do teste antes de aparecer na tabela: a
`pmr_arm` pula **exatamente o mesmo caso** que a arena artesanal — invariante 7,
liberação individual. A biblioteca padrão oferece o mesmo mecanismo, com as
mesmas renúncias.

## Implementação

**A reserva é `null_memory_resource`, e isso é a invariante 6.** Por padrão,
quando o buffer acaba, o `monotonic_buffer_resource` pede mais ao recurso de
reserva — que é o alocador global. Isso é conveniente e destrói o teto: no meio
de um lote, sem aviso, a alocação volta a custar o que custava.

Com `null_memory_resource`, o esgotamento vira `std::bad_alloc`, que este braço
converte em `nullptr` para cumprir a spec. É a diferença entre *arena com teto*
e *arena que às vezes vira heap*, e é uma decisão de configuração que quase não
aparece nos exemplos de `pmr`.

### Um defeito que a invariante 6 pegou

A primeira versão reservava `capacity * sizeof(node) + alignof(node)` bytes,
"para o alinhamento". Só que `alignof(node)` é 64 e `sizeof(node)` é 64: a folga
era **exatamente um nó**, e o braço servia `capacity + 1`.

O teste acusou — *"serviu além da capacidade declarada"* —, e é literalmente
para isso que ele existe. Um teto que não é o teto declarado é pior que teto
nenhum: o dimensionamento do lote passa a ser feito contra um número que o
código não respeita.

## Experimento

Campanha de **5** execuções, em `release`, **10000** amostras por ponto, **64**
lotes de aquecimento iguais nos três braços, e **a mesma carga do 01.01** —
mesmos tamanhos de lote, mesmo objeto, mesma forma de amostra. Sem isso a
pergunta deste tópico não teria com o que ser respondida.

O braço `std::allocator` aparece de novo, e não por simetria: ele é a **âncora**
que diz se estes números e os do 01.01 vieram da mesma máquina no mesmo estado.

```bash
./scripts/build-all.sh release
./scripts/arquivar-medicao.sh docs/01-memoria/03-pmr-o-meio-termo bench-pmr pmr-contra-arena 5
```

## Ambiente de execução

Gerado, não descrito:
[`ambiente.md`](bench/medicoes/historico/2026-09-22-pmr-contra-arena/ambiente.md).

## Resultados

| Métrica | Mediana entre execuções | Amplitude entre execuções | Unidade |
|---|---:|---:|---|
| std::allocator, lote 64 — p50 | 1674,00 | 26,9% | ns |
| arena artesanal, lote 64 — p50 | 60,00 | 16,7% | ns |
| std::pmr, lote 64 — p50 | 90,00 | 33,3% | ns |
| std::allocator, lote 512 — p50 | 14758,00 | 1,0% | ns |
| arena artesanal, lote 512 — p50 | 371,00 | 2,4% | ns |
| std::pmr, lote 512 — p50 | 591,00 | 1,7% | ns |
| std::allocator, lote 512 — p99 | 20710,10 | 56,5% | ns |
| arena artesanal, lote 512 — p99 | 381,00 | 2,6% | ns |
| std::pmr, lote 512 — p99 | 601,00 | 0,2% | ns |
| std::allocator, lote 4096 — p50 | 88829,00 | 1,6% | ns |
| arena artesanal, lote 4096 — p50 | 3526,00 | 70,4% | ns |
| std::pmr, lote 4096 — p50 | 5020,00 | 71,6% | ns |

O lote 8 ficou **fora da tabela**: ali os três braços marcam 2,50 ns por nó, que
é o piso do instrumento medido em
[08.01](../../08-medicao/01-harness/README.md). Publicar a linha convidaria a
comparar números que descrevem o relógio.

## O que esta medição não mostra

- **o que o benchmark removeu:** um único tipo de objeto, tamanho fixo, um
  fluxo. A `pmr` existe para o caso em que os tipos são muitos e o alocador é
  escolhido em tempo de execução — e esse caso **não** está medido aqui;
- **a qual braço isso favorece:** à **arena artesanal**. Ela é especializada num
  tipo, e este benchmark tem um tipo só. A vantagem dela encolheria num programa
  com dez tipos diferentes, onde a arena viraria dez arenas;
- **qual conclusão o leitor tiraria se parasse na tabela:** que a arena
  artesanal vale a pena. Vale 1,5×, e a tabela não diz o que custa mantê-la;
- **o que seria necessário** para a conclusão mais ampla: vários tipos de
  objeto, e o `pmr` usado como ele foi desenhado — com contêineres recebendo o
  recurso, e não com chamadas diretas a `allocate`.

## Quando dá errado

**O buffer acaba no meio do lote.** Com `null_memory_resource` de reserva, o
`allocate` lança `std::bad_alloc` e o braço devolve `nullptr` — teto declarado,
e continua declarado. **Sem** essa configuração, o mesmo caso silenciosamente
aloca do heap e a medição publicaria um número que mistura os dois caminhos.

O caminho de exceção está no laço quente e não custa nada enquanto não dispara:
o modelo do GCC e do Clang em x86-64 é *zero-cost*. Quando dispara, o lote já
acabou.

## Análise

**A biblioteca padrão já oferece o mecanismo, e entrega a maior parte do
ganho.** No lote 512, o `std::allocator` custa 28,82 ns por nó, a `pmr` custa
1,15 e a arena artesanal custa 0,72 — **25,0×** e **39,8×** sobre o baseline.

Dito de outro modo: **a arena artesanal é 1,59× mais rápida que a `pmr`**, e não
40× como a leitura apressada do 01.01 poderia sugerir. Os 27,9× que o 01.01
publicou não são o ganho de "escrever uma arena" — são o ganho de "não usar
`std::allocator` para este padrão", e a `pmr` captura a maior parte dele sem
uma linha de código artesanal.

A diferença de 1,59× tem causa conhecida: o `monotonic_buffer_resource` é
polimórfico — despacho virtual por chamada, e caminho genérico de tamanho e
alinhamento. A arena do 01.01 serve um tipo só e sabe disso em compilação.

No lote 8 os três coincidem em 2,50 ns por nó, porque os três estão no piso do
instrumento. **A razão de 1,00× ali não é um empate: é a ausência de medição.**

## Regra de decisão

> **Use `std::pmr::monotonic_buffer_resource` com reserva `null_memory_resource`.
> A arena artesanal só se justifica se 1,6× for a diferença entre caber e não
> caber no orçamento — e isso precisa estar medido, não suposto.**

| Campo | Valor |
|---|---|
| estrutura de partida → troca | `std::allocator<node>` → `std::pmr::monotonic_buffer_resource` |
| variável de decisão | alocações por lote |
| ponto de virada | **64**, o mesmo do 01.01 — abaixo disso a medição não distingue nenhum dos três do instrumento |
| ganho sobre o baseline | **25,0×** no p50 do lote 512 |
| o que a arena artesanal acrescenta | **1,59×** sobre a `pmr`, no mesmo ponto |
| percentil em que aparece | os dois; no lote 512, o p99 da `pmr` é 601 ns contra 20710 do `std::allocator` |
| máquina, configuração, commit | ver o [`metadata.json`](bench/medicoes/historico/2026-09-22-pmr-contra-arena/metadata.json) |
| **o que a regra não cobre** | vários tipos de objeto, `pmr` usado com contêineres, escolha de recurso em tempo de execução, mais de uma *thread* |

**Este é um resultado em que a biblioteca padrão basta**, e ele vale tanto
quanto os dois anteriores: publicar só os casos em que o artesanal ganha
produziria um material que confirma o que o autor já achava.

## Confronto com a literatura

O material sobre `pmr` costuma apresentá-lo como ferramenta de conveniência —
trocar o alocador sem trocar o tipo — e raramente publica o custo do despacho
polimórfico contra uma arena especializada. Este tópico mede **1,59×** nesta
máquina, para um tipo só.

Fica em aberto quanto desse fator é despacho virtual e quanto é o caminho
genérico de alinhamento. Separar os dois exige olhar o código gerado, que é o
módulo [10](../../10-codigo-gerado/README.md).

## Trade-offs

| Escolha | Ganho | Preço |
|---|---|---|
| `std::pmr` | 25× sobre o baseline, sem código artesanal; serve a qualquer tipo | 1,59× mais lento que a arena especializada; a reserva **precisa** ser configurada |
| arena artesanal | mais 1,59× | um tipo por arena; código para manter |
| `std::allocator` | uso geral | 25× mais caro neste padrão |

## Quando utilizar

Sempre que o padrão for lote que morre inteiro — que é quando o 01.01 se
aplica. A `pmr` é o ponto de partida; a arena artesanal é otimização com número
que a justifique.

## Quando não utilizar

Quando os objetos entram e saem individualmente: aí nem a `pmr` nem a arena
servem, e o caminho é o [pool do 01.02](../02-pool-freelist/README.md).

## Limitações

Uma máquina, um compilador, um tipo de objeto, um fluxo. A `pmr` foi usada por
chamada direta a `allocate`, e não através de contêiner — que é o uso para o
qual ela foi desenhada.

## Exercícios

1. Tire o `null_memory_resource` e deixe a reserva padrão. Qual percentil
   denuncia o lote que estourou o buffer?
2. Meça com quatro tipos de objeto de tamanhos diferentes. A vantagem de 1,59×
   da arena sobrevive?
3. Use a `pmr` através de um `std::pmr::vector`. O custo por nó muda?

## Referências

- [01.01 — alocador de arena](../01-alocador-arena/README.md)
- [01.02 — pool com lista de livres](../02-pool-freelist/README.md)
- [Norma do projeto](../../padrao-do-projeto.md), seções 11, 34 e 35

## Navegação

- [Módulo 01](../README.md)
- [Plano de estudo](../../plano-estudo.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
