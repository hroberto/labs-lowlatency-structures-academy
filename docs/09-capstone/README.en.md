> [🇧🇷 Português](README.md) | 🇺🇸 English

# 09 — Capstone

> **State: not started.** See
> [state labelling](../padrao-do-projeto.en.md#5-state-labelling).

## The module's question

> what is the end-to-end difference, and **which topic accounted for it**?

The second half is what makes the capstone different from a demo: the
deliverable is a table that **attributes** the total difference to each topic in
the track. An engine that runs faster without saying what made it faster closes
nothing.

## The failure question

> what does the engine do under a burst above the budget?

## The `std` → `custom` pair

Everything above, in **two versions of the same engine**: one built with what the
standard library ships, the other with the complements each module measured. The
contract is the same, the load is the same, the machine is the same.

## Planned topic

| Topic | State |
|---|---|
| `order-book-engine` — the full engine, fed by a reproducible synthetic feed | **not started** |

The feed is synthetic and reproducible **by decision**: no proprietary exchange
data enters this repository, and no strategy, signal or decision logic — only
infrastructure.

## The layered architecture belongs here

Section 15 of the standard assigns `domain/ports/adapters/application/cli` to the
**capstone**, not to every topic. A topic comparing two structures has neither a
use case nor an adapter: it has two implementations, one `spec.hpp` and one
benchmark. Imposing the five layers on it would produce empty directories.

## Navigation

- [Documentation and standard](../README.en.md)
- [Repository README](../../README.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
