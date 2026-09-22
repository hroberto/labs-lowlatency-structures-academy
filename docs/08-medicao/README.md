> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 08 — Medição

> **Estado: um tópico medido.** O [08.01](01-harness/README.md) tem número
> publicado com campanha arquivada; o 08.02 não foi iniciado.

## A pergunta do módulo

> como o harness evita medir a si mesmo?

Não evita. Ele mede o próprio custo, publica esse número, e só então pode
atribuir o resto à operação sob teste. O que se evita é medir a si mesmo **sem
saber**.

## A pergunta de falha

> quando o *warm-up* esconde a cauda?

## Por que este módulo vem primeiro na ordem de construção

Ele não é o primeiro da ordem de estudo — é o primeiro da ordem de construção, e
a distinção está no [ROADMAP](../../ROADMAP.md). Um tópico de estrutura escrito
antes dele publicaria percentis por operação sem que o piso do instrumento
estivesse medido, e a diferença entre *a estrutura custa 20 ns* e *o instrumento
custa 20 ns* não apareceria em teste nenhum.

Este módulo é **transversal**: não há par `std` → `custom` aqui, porque ele não
compara duas estruturas — ele mede o instrumento com que as outras serão
comparadas. Por isso também não tem *Regra de decisão*: a seção 35 da norma a
exige de quem compara.

## Tópicos

| Tópico | Pergunta | Estado |
|---|---|---|
| [`01-harness`](01-harness/README.md) — o harness medindo a si mesmo | a partir de que latência a medição por operação deixa de descrever a operação? | **medido** |
| `02-perf-e-cache-misses` — contadores de hardware | o que o PMU mostra que o relógio não mostra? | **não iniciado** |

O 08.02 depende de PMU acessível. Na máquina de referência isso exigiu baixar
`kernel.perf_event_paranoid` de 4 para 2 — o menor valor que serve, porque
contador de desempenho é canal lateral. Ver
[`scripts/check-env.sh`](../../scripts/check-env.sh), que diagnostica.

## O que este módulo entrega para os outros

- `lib/measurement/tail.hpp` — percentil exato sobre amostras cruas, com a
  convenção de quantas amostras cada percentil exige;
- `scripts/arquivar-medicao.sh` — a campanha, com repetições e **amplitude entre
  execuções**, que é o que separa a cauda da estrutura da cauda da máquina;
- o **piso por operação** desta máquina, publicado, que todo tópico que medir por
  operação precisa subtrair ou evitar com critério.

## Navegação

- [Documentação e norma](../README.md)
- [README do repositório](../../README.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
