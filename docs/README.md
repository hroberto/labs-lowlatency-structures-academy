> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# Documentação

## Norma e bibliografia

| Documento | Para quê serve | Estado |
|---|---|---|
| [padrao-do-projeto.md](padrao-do-projeto.md) | **a norma**: método, medição, contrato, arquitetura, documentação, fontes, e o par `std` e `custom` | **escrito** — 36 seções, 7 partes |
| [referencias.md](referencias.md) | catálogo central, com identificadores estáveis | **escrito**, endereços pendentes |
| [plano-estudo.md](plano-estudo.md) | **a ordem de leitura** — diferente da ordem de construção, que é o ROADMAP | **escrito** |
| [origem/setup-cpp-performance-academy.md](origem/setup-cpp-performance-academy.md) | o documento de setup que originou **esta norma**, preservado intacto para auditoria | intacto |
| [origem/setup-lowlatency-structures-academy.md](origem/setup-lowlatency-structures-academy.md) | o documento de setup que originou **a tese e a trilha** deste repositório, preservado intacto | intacto |


## A trilha

Uma árvore só, e é esta. `docs/0N-*` é o módulo — teoria e prática no mesmo
lugar, porque a [seção 25](padrao-do-projeto.md#25-estrutura-padrão-de-capítulo)
já torna *Modelo mental*, *Fundamentos* e *Como funciona* obrigatórias dentro de
cada capítulo. A **ordem de estudo** é outro documento, ainda não escrito.

| Módulo | Pergunta | Estado |
|---|---|---|
| [01 memória](01-memoria/README.md) | a que taxa de alocação o alocador global passa a dominar o p99? | **um tópico medido** |
| [02 layout](02-layout/README.md) | quantos *cache misses* por elemento cada layout custa? | não iniciado |
| [03 contêineres](03-conteineres/README.md) | em que cardinalidade a busca em array plano perde para a árvore? | não iniciado |
| [04 índice de chave densa](04-indice-de-chave-densa/README.md) | qual o custo de encontrar o extremo ocupado em p50/p99/p99,9? | não iniciado |
| [05 filas e concorrência](05-filas-e-concorrencia/README.md) | quando o lock deixa de ser o gargalo e passa a ser a linha de cache? | não iniciado |
| [06 tempo e erros](06-tempo-e-erros/README.md) | quanto custa medir, e quanto custa o caminho de erro? | não iniciado |
| [07 parsing](07-parsing/README.md) | quando a STL moderna já basta, e quando não? | não iniciado |
| [08 medição](08-medicao/README.md) | como o harness evita medir a si mesmo? | **um tópico medido** |
| [09 capstone](09-capstone/README.md) | qual a diferença ponta a ponta, e qual tópico respondeu por ela? | não iniciado |
| [10 código gerado](10-codigo-gerado/README.md) | o que o compilador faz com `custom/` que não faz com `std/`? | não iniciado |

**2 tópicos de 23.** A tabela diz "não iniciado" onde não há conteúdo porque
índice que anuncia o que não existe é o defeito que a
[seção 5](padrao-do-projeto.md#5-rotulagem-de-estado) existe para impedir.

O módulo 08 vem primeiro na **ordem de construção** — ele mede o instrumento
antes de qualquer tópico usá-lo. O módulo 10 veio do projeto absorvido e é
transversal, como o 08.

## Ainda não escrito

| Documento | Para quê servirá |
|---|---|
| `regras-de-decisao.md` | **a tabela consolidada de regras de decisão** (norma, seção 35) — o artefato de referência interna |

> A ordem de estudo e a ordem de construção são documentos diferentes de
> propósito. A de construção é o [ROADMAP.md](../ROADMAP.md), e ele **não** é
> ordem de leitura.

## Por onde começar

1. [Padrão do projeto](padrao-do-projeto.md) — em especial a Parte III, que
   define as duas diretrizes de implementação do projeto.
2. [Catálogo de referências](referencias.md).
3. O [ROADMAP.md](../ROADMAP.md), para saber o que existe e o que não existe.

## Navegação

- [README do repositório](../README.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
