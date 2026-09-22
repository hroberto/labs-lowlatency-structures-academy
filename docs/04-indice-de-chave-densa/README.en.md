> [🇧🇷 Português](README.md) | 🇺🇸 English

# 04 — Dense-key index

> **State: not started.** No topic in this module has been written, and no
> number has been published. This page exists to give the map, not to announce
> content — see [state labelling](../padrao-do-projeto.en.md#5-state-labelling).

## The module's question

> what does finding the **occupied extreme** cost at p50/p99/p99.9, under a real
> update load?

## The failure question

> what happens with a key outside the array's range?

It is mandatory and it is answered with a **program**, never in prose — and the
invariant it attacks has a negative test: the version that violates it exists,
and the suite requires it to fail.

## What a dense key is, and why it changes the structure

A key is **dense** when its domain is discrete, bounded and known: an identifier
that moves one step at a time inside a range, not an arbitrary 64-bit integer.
In that case the index needs neither a tree nor a hash — the key **is** the
index, and what is left is finding out which positions are occupied.

It is the same constraint [module 03](../03-conteineres/README.en.md) does not
have. There the structure is generic because the key can be anything; here the
structure wins because the key is very little. This module exists to measure
**how much** that constraint is worth, and from what point.

Module 03 comes first on purpose: reading this one first makes the flat array
look like magic, instead of what it is — a trade that only fits because the key
has a range and a step.

## The `std` → `custom` pair

`std::map<Key, Value>` → an array by key + an occupancy bitmap; an
`unordered_map` of records → a hash with no per-node allocation

The order is fixed: **the language's offering first.** No `custom/` is written
before the corresponding `std/` has been measured and published.

## Planned topics

| Topic | State |
|---|---|
| `01-map-de-chaves` — `std::map` as an index | **not started** |
| `02-array-plano-com-bitmap` — an array by key + an occupancy bitmap | **not started** |
| `03-hash-de-registros` — a hash with no per-node allocation | **not started** |

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
