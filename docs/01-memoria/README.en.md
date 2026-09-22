> [🇧🇷 Português](README.md) | 🇺🇸 English

# 01 — Memory

> **State: module complete.** All three topics have published numbers with
> archived campaigns, and the three decision rules are in
> [`regras-de-decisao.md`](../regras-de-decisao.en.md). See [state labelling](../padrao-do-projeto.en.md#5-state-labelling).

## The module's question

> at what allocation rate does the global allocator start dominating p99?

## The failure question

> what happens when the arena runs out mid-batch?

It is mandatory and it is answered with a **program**, never in prose — and the
invariant it attacks has a negative test: the version that violates it exists,
and the suite requires it to fail.

## The `std` → `custom` pair

`new` / `std::allocator` → arena, pool with a free list; `std::pmr` as the middle ground

The order is fixed: **the language's offering first.** No `custom/` is written
before the corresponding `std/` has been measured and published.

## Planned topics

| Topic | State |
|---|---|
| [`01-alocador-arena`](01-alocador-arena/README.en.md) — arena allocator | **measured** |
| [`02-pool-freelist`](02-pool-freelist/README.en.md) — pool with a free list | **measured** |
| [`03-pmr-o-meio-termo`](03-pmr-o-meio-termo/README.en.md) — `std::pmr` as the middle ground | **measured** |

Directories are born with their first real file. An empty directory is a
promise, and a promise is what section 5 of the standard exists to prevent.

## Theory

The theory this module needs lives **here and in its topics**, not in a parallel
tree: section 25 of the standard already makes *Mental model*, *Fundamentals*
and *How it works* mandatory in every chapter.

Whatever is a hardware or operating-system mechanism **already measured** — the
cost of a cache miss, TLB, NUMA, false sharing — is **cited**, not re-taught: it
belongs to the [DPDK Academy](../referencias.en.md#dpdk-academy), under section
30 of the standard. What this module adds is what the C++ structure does with
that cost.

## Navigation

- [Documentation and standard](../README.en.md)
- [Repository README](../../README.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
