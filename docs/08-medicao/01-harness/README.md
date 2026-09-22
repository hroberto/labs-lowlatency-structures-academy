> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 08.01 — O harness medindo a si mesmo

**Estado: medido.** Há número publicado, com ambiente gerado e programa que o
produz, arquivado em
[`bench/medicoes/`](bench/medicoes/historico/2026-09-22-piso-do-harness/).

## Objetivos

Medir o custo do instrumento antes de qualquer tópico de estrutura usá-lo, e
estabelecer a partir de que latência a medição por operação deixa de descrever a
operação.

## Problema

A pergunta do módulo é *como o harness evita medir a si mesmo*. A resposta é que
ele não evita: ele mede o próprio custo, publica esse número, e só então pode
atribuir o resto à operação sob teste. O que se evita é medir a si mesmo **sem
saber**.

Isso importa porque a norma publica latência por percentis altos, e percentil
exige uma amostra por operação. Cada amostra carrega duas leituras de relógio, e
uma operação que custe menos que esse par não é distinguível dele.

## Modelo mental

Há dois modos de medir, e a escolha entre eles não é de gosto:

| Modo | Como | O que dá, e o que tira |
|---|---|---|
| lote | um par de leituras para *N* operações | o custo do relógio se dilui; a distribuição desaparece — não há p99 de uma média |
| por operação | um par de leituras por operação | a cauda aparece; o piso do instrumento entra em cada amostra |

A trilha precisa do segundo, porque publica p99 e p99,9. Este tópico existe para
que o preço dele esteja medido antes.

## Como funciona

Três braços, no mesmo programa e no mesmo harness:

1. **ler o relógio** — custo amortizado de uma chamada a `now_ns()`, medido em
   lote sobre as rodadas;
2. **registrar amostra** — o que o coletor de cauda cobra por amostra gravada;
3. **piso por operação** — o intervalo entre duas leituras consecutivas, com
   **nada** no meio. É a menor latência que este harness consegue observar por
   operação.

O terceiro é o que fecha o argumento.

## Implementação

A cauda não usa histograma. O documento de origem da trilha previa
`HdrHistogram_c`, e a decisão mudou depois de duas verificações: ele não está no
WrapDB, e o escritor de log dele arrasta zlib, contra a promessa de nenhuma
dependência além do compilador. Mais importante, o problema que o HDR resolve —
fluxo ilimitado de amostras em espaço constante — não é o de um laboratório que
escolhe quantas operações vai medir.

Então as amostras são guardadas: um buffer pré-alocado de `uint64_t`, e o
percentil sai **exato**, sem precisão a declarar porque não há perda a declarar.
O que se paga é memória proporcional ao número de amostras e uma ordenação
final, ambas fora do caminho quente. Ver
[`lib/measurement/tail.hpp`](../../../lib/measurement/tail.hpp).

`record()` não pode alocar, e é por isso que a capacidade é reservada uma vez: um
crescimento de vetor no meio da medição entraria na amostra seguinte, e o harness
passaria a medir o próprio crescimento atribuindo isso à operação.

## Experimento

Uma campanha de **5** execuções do programa inteiro, em `release`, com **25**
amostras por braço de lote, **200000** rodadas por amostra e **10000** amostras
no braço de cauda — que é o mínimo que sustenta p99,9 pela convenção deste
projeto.

```bash
./scripts/build-all.sh release
./scripts/arquivar-medicao.sh piso-do-harness 5
```

## Ambiente de execução

Gerado, não descrito: [`ambiente.md`](bench/medicoes/historico/2026-09-22-piso-do-harness/ambiente.md)
e o mesmo conteúdo dentro de
[`metadata.json`](bench/medicoes/historico/2026-09-22-piso-do-harness/metadata.json).

O que mais pesa nos números abaixo: AMD Ryzen 9 9900X, SMT ativo, dois domínios
de L3, `governor=powersave` com *boost* ativo, e resolução de `steady_clock` de
**1** ns.

## Resultados

| Métrica | Mediana entre execuções | Amplitude entre execuções | Unidade |
|---|---:|---:|---|
| ler o relógio (mediana) | 16,09 | 4,6% | ns |
| ler o relógio (mínimo) | 16,06 | 0,1% | ns |
| registrar amostra (mediana) | 0,12 | 0,3% | ns |
| piso por operação p50 | 20,00 | 0,0% | ns |
| piso por operação p99 | 21,00 | 0,0% | ns |
| piso por operação p99,9 | 21,00 | 0,0% | ns |
| piso por operação máximo | 21,00 | 10681,0% | ns |

## O que esta medição não mostra

- **o que o benchmark removeu:** não há trabalho entre as duas leituras, nem
  pressão de cache, nem outra *thread* competindo. Um piso medido sob carga real
  é mais alto, e este número é o melhor caso;
- **a qual braço isso favorece:** a nenhum — não há dois braços aqui. Mas
  favorece a impressão de que a medição por operação é mais barata do que ela
  será dentro de um tópico de estrutura;
- **qual conclusão o leitor tiraria se parasse na tabela:** que a medição por
  operação custa 20 ns. Custa ao menos isso, nesta máquina, sem carga;
- **o que seria necessário** para a conclusão mais ampla: repetir o piso dentro
  de cada tópico que use medição por operação, e publicar os dois lado a lado.

## Quando dá errado

Três caminhos de falha, e os três têm verificação que dispara.

**O laço medido desaparece.** Em `-O2` o braço de registro media 0,359 ns; em
`release` passou a medir 0,000 — e zero não é *rápido*, é *não aconteceu*: sem
ninguém observando o estado do coletor, o otimizador removeu o laço inteiro. A
régua trata mediana zero como `below_resolution`, estado legítimo para operação
mais rápida que o relógio, e por isso a tabela saiu plausível. A correção é
observar o estado depois do laço.

**O portão de validade não cobre o modo que arquiva.** A primeira versão do
programa conferia a validade **depois** do `return` do modo `--csv`. O modo
texto reprovava; o `--csv`, que é justamente o que a campanha arquiva, saía
zero. Uma campanha foi versionada com procedência completa descrevendo um braço
que não mediu. Ela foi descartada, e o programa passou a conferir antes de
qualquer saída. Hoje o braço de controle (`--control-arm`) força o caso e
o teste L2 exige reprovação nos **dois** modos.

**Publicar percentil que a amostra não sustenta.** p99,9 de 1000 amostras
repousa sobre uma observação, que é o máximo com outro nome. `tail_statistics`
carrega o percentil máximo sustentado, o programa avisa quando a cauda é curta,
e a suíte tem caso que aborta ao pedir publicação sem sustentação.

## Análise

O piso por operação é **20** ns de mediana, e p50, p99 e p99,9 saem **idênticos
nas cinco execuções** — amplitude de 0,0% nos três. O par de leituras custa
aproximadamente o que uma leitura isolada custa em lote, o que é coerente com o
custo estar na própria chamada e não no que ela mede.

O **máximo** é a exceção, e ela é instrutiva. A série por execução é
`[2264, 21, 21, 21, 1864]`: duas das cinco execuções pegaram uma interrupção, e
as outras três não. A amplitude de **10681%** não descreve variação do
instrumento — descreve **quantas amostras isoladas cada execução colheu**. O
máximo é uma amostra, e publicá-lo sem a série ao lado convida à conclusão
errada. Fica na tabela porque esconder a extensão do que se observou é pior que
exibi-la, mas ele não sustenta comparação nenhuma.

### Uma leitura anterior deste tópico foi refutada por mais medição

A primeira campanha arquivada mostrava p99 variando **42,9%** entre execuções, e
o braço do relógio ~26% mais caro em `r0` e `r1`. Concluímos ali que a campanha
tinha **aquecimento**: a máquina levando dois segundos para assentar em
frequência de *boost*.

Esta campanha refuta isso. O desvio do relógio caiu para 4,6% e está em `r3`, não
nas primeiras; os percentis não variaram nada. O que havia era **interferência
esporádica**, que atinge execuções ao acaso — não um transiente de aquecimento,
que atingiria sempre as primeiras.

A conclusão correta é mais modesta e mais útil: nesta máquina o **valor típico do
instrumento é reprodutível**, e o que varia entre execuções é quanta
interferência cada uma colheu. A amplitude entre execuções continua sendo campo
obrigatório — mas ela mede o **estado da máquina durante a campanha**, e não uma
propriedade estável dela. Duas campanhas da mesma medição deram 42,9% e 0,0% no
mesmo p99.

O custo de registrar amostra, **0,12** ns, é da ordem de uma instrução: o
coletor não é o que limita a medição por operação. O relógio é.

## Confronto com a literatura

A ordem de grandeza confere com o que se documenta sobre `clock_gettime` em
Linux x86-64 com `vDSO`: dezenas de nanossegundos, sem entrada no *kernel*. A
literatura de microbenchmark recomenda medição em lote justamente por esse piso,
e a recomendação é consistente com o que se mediu aqui — o que este tópico
acrescenta não é a recomendação, é o número desta máquina, para que os tópicos
seguintes possam subtraí-lo ou evitá-lo com critério.

O que **não** se encontrou na literatura consultada foi tratamento da amplitude
entre execuções de percentil alto como dado publicável. Fica registrado como
questão em aberto: a prática comum é publicar a melhor execução, e ela esconde
justamente o que a tabela acima mostra.

## Trade-offs

| Escolha | Ganho | Preço |
|---|---|---|
| amostras cruas em vez de histograma | percentil exato, zero dependência | memória proporcional às amostras, ordenação final |
| medição por operação | a cauda aparece | piso do instrumento em cada amostra |
| medição em lote | o piso se dilui | não há percentil de uma média |

## Quando utilizar

Medição por operação quando a operação sob teste custa bem acima do piso — como
regra prática desta máquina, acima de algumas centenas de nanossegundos —, e
sempre que a conclusão depender da cauda.

## Quando não utilizar

Quando a operação custa menos que o piso. Aí a saída não é um número menor: é
medir em lote e abandonar a pretensão de publicar percentil daquele braço,
dizendo isso no documento.

## Limitações

Uma máquina, um compilador por campanha, sem isolamento de núcleo e sem fixar o
*governor*. O piso foi medido sem carga concorrente. Nenhum destes números vale
como propriedade de `steady_clock` em geral: valem como propriedade desta
máquina, e é para isso que servem.

## Exercícios

1. Rode a campanha com `governor=performance` e compare a amplitude entre
   execuções do p99. Quanto dela era o *governor*?
2. Fixe o processo num núcleo de cada domínio de L3 e refaça o piso. O CCD
   muda o piso, ou só a cauda?
3. Reduza a cauda para 500 amostras e confira que o aviso de percentil sem
   sustentação aparece. Depois remova o aviso do programa e veja qual teste
   fica vermelho.

## Referências

- [Norma do projeto](../../padrao-do-projeto.md), seções 11, 28, 29 e 35
- [`lib/measurement/tail.hpp`](../../../lib/measurement/tail.hpp)
- [`lib/measurement/clock.hpp`](../../../lib/measurement/clock.hpp)
- [Catálogo de referências](../../referencias.md)

## Navegação

- [Trilha](../../../ROADMAP.md)
- [README do repositório](../../../README.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
