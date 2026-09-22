> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 07 — Parsing

> **Estado: não iniciado.** Nenhum tópico deste módulo foi escrito, e nenhum
> número foi publicado. Esta página existe para dar o mapa, não para anunciar
> conteúdo — ver a [rotulagem de estado](../padrao-do-projeto.md#5-rotulagem-de-estado).

## A pergunta do módulo

> quando a STL moderna já basta, e quando não?

## A pergunta de falha

> o que o *decoder* faz com mensagem truncada?

Ela é obrigatória e se responde com **programa**, nunca em prosa — e a
invariante que ela ataca tem teste negativo: a versão que a viola existe, e a
suíte exige que ela falhe.

## O par `std` → `custom`

`std::string` / `stringstream` → `string_view` + `from_chars` → *decoder* binário sem cópia

A ordem é fixa: **primeiro o que a linguagem dá.** Nenhum `custom/` se escreve
antes de o `std/` correspondente estar medido e publicado.

## Tópicos previstos

| Tópico | Estado |
|---|---|
| `01-string-vs-string-view` — `string` contra `string_view` | **não iniciado** |
| `02-decoder-zero-copy` — *decoder* sem cópia | **não iniciado** |

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
