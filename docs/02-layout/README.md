> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 02 — Layout de memória

> **Estado: não iniciado.** Nenhum tópico deste módulo foi escrito, e nenhum
> número foi publicado. Esta página existe para dar o mapa, não para anunciar
> conteúdo — ver a [rotulagem de estado](../padrao-do-projeto.md#5-rotulagem-de-estado).

## A pergunta do módulo

> quantos *cache misses* por elemento cada layout custa, medidos?

## A pergunta de falha

> o que dois contadores na mesma linha fazem com dois núcleos?

Ela é obrigatória e se responde com **programa**, nunca em prosa — e a
invariante que ela ataca tem teste negativo: a versão que a viola existe, e a
suíte exige que ela falhe.

## O par `std` → `custom`

`struct` de campos → SoA; *padding* manual → `hardware_destructive_interference_size`

A ordem é fixa: **primeiro o que a linguagem dá.** Nenhum `custom/` se escreve
antes de o `std/` correspondente estar medido e publicado.

## Tópicos previstos

| Tópico | Estado |
|---|---|
| `01-soa-vs-aos` — SoA contra AoS | **não iniciado** |
| `02-false-sharing` — falso compartilhamento | **não iniciado** |
| `03-small-buffer` — *small buffer optimization* | **não iniciado** |

Os diretórios nascem com o primeiro arquivo real. Diretório vazio é promessa,
e promessa é o que a seção 5 da norma existe para impedir.

## Teoria

A teoria de que este módulo precisa mora **aqui e nos seus tópicos**, e não numa
árvore paralela: a seção 25 da norma já torna *Modelo mental*, *Fundamentos* e
*Como funciona* obrigatórias em todo capítulo.

O que for mecanismo de hardware ou de sistema operacional **já medido** — custo
de *cache miss*, TLB, NUMA, falso compartilhamento — se **cita**, não se
reensina: é do [DPDK Academy](../referencias.md#dpdk-academy), pela seção 30 da
norma. O que este módulo acrescenta é o que a estrutura de C++ faz com esse
custo.

## Navegação

- [Documentação e norma](../README.md)
- [README do repositório](../../README.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
