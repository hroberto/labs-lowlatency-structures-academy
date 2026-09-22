> 🇧🇷 Português | [🇺🇸 English](plano-estudo.en.md)

# Plano de estudo — a ordem de leitura

> **Estado: escrito.** A ordem existe; **1 dos 23 tópicos** existe. Cada módulo
> declara o próprio estado no índice dele, e esta página não os anuncia como se
> estivessem prontos.

Este documento é a **ordem de leitura**. Não é a ordem de construção, que é o
[ROADMAP](../ROADMAP.md) — e a distinção existe porque no projeto a montante duas
numerações concorrentes conviveram no mesmo README e "nível 5" passou a
significar duas coisas diferentes conforme o documento.

As duas ordens **divergem de propósito**: o módulo 08 é o primeiro a ser
construído, porque mede o instrumento que todos os outros usam, e é o penúltimo
a ser lido, porque só interessa a quem já quer medir alguma coisa.

## O que se pressupõe, e o que não

**Pressupõe-se:** C++ intermediário — *template*, RAII, o `std::` do dia a dia —,
Linux por linha de comando, e noção de *thread*.

**Não se pressupõe, e o material ensina:** hierarquia de cache, TLB, modelo de
memória do C++, *atomics* e ordenação, alocadores, layout de memória, percentis
e como medi-los.

Complexidade assintótica é **premissa, não assunto**. Este material não ensina
algoritmos: ele mede o que a constante escondida por trás do O-grande custa nesta
máquina.

## Antes de qualquer módulo

| Passo | Por quê |
|---|---|
| [README do repositório](../README.md) | o que o projeto é, e o que ele não é |
| [`./scripts/check-env.sh`](../scripts/check-env.sh) | diz se a sua máquina compila o que a trilha usa, sondando recurso em vez de confiar na versão |
| [Norma, seções 6 a 9](padrao-do-projeto.md#6-o-percurso-metodológico) | o percurso, as classes de afirmação, e a regra de que número precisa de programa |
| [Norma, seção 28](padrao-do-projeto.md#28-reprodutibilidade) | como ler os números: percentil, amplitude entre execuções, e o que o CCD tem a ver com isso |

Quem for só **consultar** — e não estudar — tem um atalho: a tabela de
`regras-de-decisao.md`, ainda não escrita, é o artefato de referência. Ela diz
*em que ponto trocar*, sem o caminho que levou até lá.

## A ordem

### Primeiro, o instrumento — [08 medição](08-medicao/README.md)

Fora de ordem, e de propósito: ler o [08.01](08-medicao/01-harness/README.md)
antes de tudo custa quinze minutos e evita a leitura errada de **todos** os
números que vêm depois. Ele responde por que um p50 de 20 ns pode ser o relógio,
e não a estrutura.

Quem não quiser esse desvio pode voltar a ele quando o primeiro número
incomodar. Ele vai incomodar.

### 1. [Memória](01-memoria/README.md) → 2. [Layout](02-layout/README.md)

Onde o dado **mora** antes de onde ele é **organizado**. Alocação vem primeiro
porque o custo de layout só aparece depois que o de alocação para de dominar —
medir SoA contra AoS sob pressão de alocador mede o alocador.

### 3. [Contêineres](03-conteineres/README.md) → 4. [Livro de ofertas](04-livro-de-ofertas/README.md)

O módulo 03 dá as estruturas genéricas; o 04 é o mesmo problema com **um domínio
que restringe**, e a restrição é o que permite bater a estrutura genérica. Ler o
04 antes do 03 faz o array plano parecer mágica, em vez de parecer o que ele é:
uma troca que só cabe porque o preço tem faixa e passo.

### 5. [Filas e concorrência](05-filas-e-concorrencia/README.md) → 6. [Tempo e erros](06-tempo-e-erros/README.md)

O 05 é o primeiro módulo em que **dois núcleos** aparecem, e por isso é o
primeiro que precisa do modelo de memória do C++ citado por cláusula. O 06 vem
depois porque `rdtsc` só interessa a quem já mediu o suficiente para o custo de
`steady_clock` incomodar.

### 7. [Parsing](07-parsing/README.md)

O módulo do **resultado incômodo**: em vários casos `string_view` com
`from_chars` já é a resposta, e o decodificador artesanal não compensa.
Deliberadamente perto do fim — quem chegou até aqui já viu seis módulos em que a
alternativa compensava, e é isso que dá peso ao caso em que ela não compensa.

### 8. [Medição, parte 2](08-medicao/README.md) e 9. [Código gerado](10-codigo-gerado/README.md)

Os dois transversais, e os dois melhores **depois** de existir o que medir. O
08.02 abre o PMU; o 10 pergunta o que o compilador fez com o `custom/` que ele
não fez com o `std/`.

### 10. [Capstone](09-capstone/README.md)

O motor completo, em duas versões, com a tabela que **atribui** a diferença
ponta a ponta a cada módulo anterior. Ele não é leitura: é a conferência de que
os nove anteriores somam.

## Três formas de usar este material

| Leitor | Caminho |
|---|---|
| **estudar** | a ordem acima, inteira, com o 08.01 na frente |
| **consultar ao projetar** | `regras-de-decisao.md` e, do módulo que interessa, só *Regra de decisão* e *Limitações* |
| **reproduzir** | `check-env.sh`, depois `bench/medicoes/` do tópico e o `metadata.json` ao lado — a máquina dele está descrita ali, não em prosa |

## Navegação

- [Documentação e norma](README.md)
- [Ordem de construção](../ROADMAP.md)

> 🇧🇷 Português | [🇺🇸 English](plano-estudo.en.md)
