> [🇧🇷 Português](README.md) | 🇺🇸 English

# 05 — Queues and concurrency

> **State: not started.** No topic in this module has been written, and no
> number has been published. This page exists to give the map, not to announce
> content — see [state labelling](../padrao-do-projeto.en.md#5-state-labelling).

## The module's question

> when does the lock stop being the bottleneck and the cache line start?

## The failure question

> what does the queue do when it fills — and who finds out first?

It is mandatory and it is answered with a **program**, never in prose — and the
invariant it attacks has a negative test: the version that violates it exists,
and the suite requires it to fail.

## The `std` → `custom` pair

`std::queue` + `mutex` → SPSC → MPSC; `condition_variable` → busy-poll with `_mm_pause`

The order is fixed: **the language's offering first.** No `custom/` is written
before the corresponding `std/` has been measured and published.

## Planned topics

| Topic | State |
|---|---|
| `01-mutex-queue` — a mutex queue | **not started** |
| `02-spsc-lock-free` — lock-free SPSC | **not started** |
| `03-mpsc` — MPSC | **not started** |
| `04-wakeup-vs-busy-poll` — waking up versus busy-poll | **not started** |

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
