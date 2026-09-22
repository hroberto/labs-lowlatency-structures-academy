> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 09 — Capstone

> **Estado: não iniciado.** Ver a
> [rotulagem de estado](../padrao-do-projeto.md#5-rotulagem-de-estado).

## A pergunta do módulo

> qual a diferença ponta a ponta, e **qual tópico respondeu por ela**?

A segunda metade é a que torna o capstone diferente de uma demonstração: a
entrega é uma tabela que **atribui** a diferença total a cada tópico da trilha.
Um motor que roda mais rápido sem dizer por causa de quê não fecha nada.

## A pergunta de falha

> o que o motor faz sob rajada acima do orçamento?

## O par `std` → `custom`

Tudo acima, em **duas versões do mesmo motor**: uma construída com o que a
biblioteca padrão entrega, outra com os complementos que cada módulo mediu. O
contrato é o mesmo, a carga é a mesma, a máquina é a mesma.

## Tópico previsto

| Tópico | Estado |
|---|---|
| `motor-de-indice` — o motor completo, alimentado por *feed* sintético reproduzível | **não iniciado** |

O motor é **um índice em memória alimentado por um fluxo de mensagens**: as
mensagens chegam por uma fila, são decodificadas, viram registros, e o índice
responde consultas sobre eles. É o cenário que atravessa a trilha, e ele é
genérico por decisão — ver a divergência 12 da
[norma](../padrao-do-projeto.md#divergências-em-relação-ao-documento-de-origem-da-trilha).

O *feed* é sintético e reproduzível **por decisão**: um gerador com semente
declarada, no repositório, e não um conjunto de dados baixado de algum lugar que
ninguém consegue rebaixar depois.

## A arquitetura em camadas é daqui

A seção 15 da norma atribui `domain/ports/adapters/application/cli` ao
**capstone**, e não a cada tópico. Um tópico que compara duas estruturas não tem
caso de uso nem adaptador: tem duas implementações, uma `spec.hpp` e um
benchmark. Impor as cinco camadas a ele produziria diretório vazio.

## Navegação

- [Documentação e norma](../README.md)
- [README do repositório](../../README.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
