> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 06 — Tempo e erros

> **Estado: não iniciado.** Nenhum tópico deste módulo foi escrito, e nenhum
> número foi publicado. Esta página existe para dar o mapa, não para anunciar
> conteúdo — ver a [rotulagem de estado](../padrao-do-projeto.md#5-rotulagem-de-estado).

## A pergunta do módulo

> quanto custa medir, e quanto custa o caminho de erro?

## A pergunta de falha

> o que acontece quando o TSC não é invariante?

Ela é obrigatória e se responde com **programa**, nunca em prosa — e a
invariante que ela ataca tem teste negativo: a versão que a viola existe, e a
suíte exige que ela falhe.

## O par `std` → `custom`

`steady_clock` → `rdtsc` calibrado; `throw` → `std::expected`

A ordem é fixa: **primeiro o que a linguagem dá.** Nenhum `custom/` se escreve
antes de o `std/` correspondente estar medido e publicado.

## Tópicos previstos

| Tópico | Estado |
|---|---|
| `01-chrono-vs-rdtsc` — `chrono` contra `rdtsc` | **não iniciado** |
| `02-exceptions-vs-expected` — exceções contra `expected` | **não iniciado** |

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
