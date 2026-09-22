> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 01 — Memória

> **Estado: um tópico medido.** O [01.01](01-alocador-arena/README.md) tem
> número publicado com campanha arquivada; os outros dois não foram iniciados.
> Ver a [rotulagem de estado](../padrao-do-projeto.md#5-rotulagem-de-estado).

## A pergunta do módulo

> a que taxa de alocação o alocador global passa a dominar o p99?

## A pergunta de falha

> o que acontece quando a arena esgota no meio de um lote?

Ela é obrigatória e se responde com **programa**, nunca em prosa — e a
invariante que ela ataca tem teste negativo: a versão que a viola existe, e a
suíte exige que ela falhe.

## O par `std` → `custom`

`new` / `std::allocator` → arena, pool com *free-list*; `std::pmr` como meio-termo

A ordem é fixa: **primeiro o que a linguagem dá.** Nenhum `custom/` se escreve
antes de o `std/` correspondente estar medido e publicado.

## Tópicos previstos

| Tópico | Estado |
|---|---|
| [`01-alocador-arena`](01-alocador-arena/README.md) — alocador de arena | **medido** |
| `02-pool-freelist` — pool com free-list | **não iniciado** |
| `03-pmr-o-meio-termo` — `std::pmr` como meio-termo | **não iniciado** |

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
