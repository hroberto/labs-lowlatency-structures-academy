> [🇧🇷 Português](README.md) | 🇺🇸 English

# Documentation

## Standard and bibliography

| Document | What it is for | State |
|---|---|---|
| [padrao-do-projeto.en.md](padrao-do-projeto.en.md) | **the standard**: method, measurement, contract, architecture, documentation, sources, and the `std` and `custom` pair | **written** — 36 sections, 7 parts |
| [referencias.en.md](referencias.en.md) | central catalogue, with stable identifiers | **written**, addresses pending |
| [origem/setup-cpp-performance-academy.md](origem/setup-cpp-performance-academy.md) | the setup document this **standard** came from, preserved intact for auditing (PT-BR only, by design) | intact |
| [origem/setup-lowlatency-structures-academy.md](origem/setup-lowlatency-structures-academy.md) | the setup document this repository's **thesis and track** came from, preserved intact (PT-BR only, by design) | intact |


## The track

One tree, and this is it. `docs/0N-*` is the module — theory and practice in one
place, because [section 25](padrao-do-projeto.en.md#25-standard-chapter-structure)
already makes *Mental model*, *Fundamentals* and *How it works* mandatory inside
every chapter. The **study order** is a separate document, not yet written.

| Module | Question | State |
|---|---|---|
| [01 memory](01-memoria/README.en.md) | at what allocation rate does the global allocator start dominating p99? | not started |
| [02 layout](02-layout/README.en.md) | how many cache misses per element does each layout cost? | not started |
| [03 containers](03-conteineres/README.en.md) | at what cardinality does a flat-array search lose to the tree? | not started |
| [04 order book](04-livro-de-ofertas/README.en.md) | what does best bid/ask cost at p50/p99/p99.9? | not started |
| [05 queues and concurrency](05-filas-e-concorrencia/README.en.md) | when does the lock stop being the bottleneck and the cache line start? | not started |
| [06 time and errors](06-tempo-e-erros/README.en.md) | what does measuring cost, and what does the error path cost? | not started |
| [07 parsing](07-parsing/README.en.md) | when does the modern STL already suffice, and when not? | not started |
| [08 measurement](08-medicao/README.en.md) | how does the harness avoid measuring itself? | **one measured topic** |
| [09 capstone](09-capstone/README.en.md) | what is the end-to-end difference, and which topic accounted for it? | not started |
| [10 generated code](10-codigo-gerado/README.en.md) | what does the compiler do with `custom/` that it does not do with `std/`? | not started |

**1 topic out of 31.** The table says "not started" where there is no content
because an index announcing what does not exist is the defect
[section 5](padrao-do-projeto.en.md#5-state-labelling) exists to prevent.

Module 08 comes first in the **build order** — it measures the instrument before
any topic uses it. Module 10 came from the absorbed project and is cross-cutting,
like 08.

## Not written yet

| Document | What it will be for |
|---|---|
| `plano-estudo.en.md` | **the canonical study order** — the one the modules will cite |
| `regras-de-decisao.en.md` | **the consolidated decision-rule table** (standard, section 35) — the internal-reference artefact |

> Study order and build order are deliberately different documents. The build
> order is the [ROADMAP.en.md](../ROADMAP.en.md), and it is **not** a reading
> order.

## Where to start

1. [Project standard](padrao-do-projeto.en.md) — especially Part III, which
   defines the project's two implementation directives.
2. [Reference catalogue](referencias.en.md).
3. The [ROADMAP.en.md](../ROADMAP.en.md), to learn what exists and what does not.

## Navigation

- [Repository README](../README.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
