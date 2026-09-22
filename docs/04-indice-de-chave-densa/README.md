> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 04 — Índice de chave densa

> **Estado: não iniciado.** Nenhum tópico deste módulo foi escrito, e nenhum
> número foi publicado. Esta página existe para dar o mapa, não para anunciar
> conteúdo — ver a [rotulagem de estado](../padrao-do-projeto.md#5-rotulagem-de-estado).

## A pergunta do módulo

> qual o custo de encontrar o **extremo ocupado** em p50/p99/p99,9, sob uma
> carga real de atualizações?

## A pergunta de falha

> o que acontece com chave fora da faixa do array?

Ela é obrigatória e se responde com **programa**, nunca em prosa — e a
invariante que ela ataca tem teste negativo: a versão que a viola existe, e a
suíte exige que ela falhe.

## O que é uma chave densa, e por que ela muda a estrutura

Uma chave é **densa** quando o domínio dela é discreto, limitado e conhecido: um
identificador que anda de um em um dentro de uma faixa, e não um inteiro
qualquer de 64 bits. Nesse caso o índice não precisa de árvore nem de hash — a
chave **é** o índice, e o que sobra é descobrir quais posições estão ocupadas.

É a mesma restrição que o [módulo 03](../03-conteineres/README.md) não tem. Lá a
estrutura é genérica porque a chave é qualquer coisa; aqui a estrutura ganha
porque a chave é pouca coisa. O módulo existe para medir **quanto** essa
restrição vale, e a partir de que ponto.

O módulo 03 vem antes de propósito: ler este primeiro faz o array plano parecer
mágica, em vez de parecer o que ele é — uma troca que só cabe porque a chave tem
faixa e passo.

## O par `std` → `custom`

`std::map<Key, Value>` → array por chave + bitmap de ocupação; `unordered_map`
de registros → hash sem alocação por nó

A ordem é fixa: **primeiro o que a linguagem dá.** Nenhum `custom/` se escreve
antes de o `std/` correspondente estar medido e publicado.

## Tópicos previstos

| Tópico | Estado |
|---|---|
| `01-map-de-chaves` — `std::map` como índice | **não iniciado** |
| `02-array-plano-com-bitmap` — array por chave + bitmap de ocupação | **não iniciado** |
| `03-hash-de-registros` — hash sem alocação por nó | **não iniciado** |

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
