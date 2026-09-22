> [🇧🇷 Português](README.md) | 🇺🇸 English

# 01.02 — Pool with a free list

**State: measured.** There is a published number, with a generated environment
and a program that produces it, archived under
[`bench/medicoes/`](bench/medicoes/historico/2026-09-22-pool-contra-heap/).

## Objectives

Measure **what keeping individual freeing costs** — what the
[arena](../01-alocador-arena/README.en.md) does not pay because it does not
offer it — and from where the pool beats the global allocator.

## Problem

[01.01](../01-alocador-arena/README.en.md) closed by saying where the arena does
**not** serve: when an object has to outlive its siblings. This topic starts
exactly there.

The pattern now is **steady-state churn**: a set of live objects of roughly
constant size, where each step frees one and acquires another — and the order of
freeing is **not** the order of acquisition. It is the pattern of a table of
records coming and going, and the arena does not serve it.

## Mental model

The free slot holds the index of the next free slot. Acquiring is popping the
head; freeing is pushing onto it.

| | `std::allocator` | pool |
|---|---|---|
| acquire | searches for a free block in the allocator's structures | reads an index |
| free | returns it to the allocator | writes an index |
| ceiling | none | declared |
| where the live ones sit | wherever the allocator decides | contiguous, by construction |

**The list is LIFO, and that is not a detail:** the slot just returned is the
next to come out, and it was just touched — it is warm. A FIFO list would return
the coldest slot, and the difference would show at p50.

## How it works

This topic's [`spec.hpp`](spec.hpp) has **five invariants**, and the first
changed shape from 01.01's. There, *"every `acquire` returns a distinct
pointer"* held forever, because the batch frees nothing until the end. Here a
freed slot **must** come back — that is invariant 4 — so two equal pointers at
different times are correct, and two equal pointers at the same time are a
defect.

The invariant became *"distinct **while live**"*, and the test tracks the live
set instead of accumulating everything. A copy of 01.01's test would pass here
without verifying anything that matters.

And individual freeing, a **conditional** invariant there, is in the `concept`
here: an arm that does not offer it **does not compile**. The divergence between
the two topics became a build error rather than a footnote.

## Implementation

The list index is **32 bits**, not a pointer. The pool has a declared ceiling, so
the index suffices — and in a 4096-slot pool the whole list takes 16 KB and stays
in L1, against 32 KB in pointers.

The `std` arm is the **same** `std::allocator` as 01.01's, with the interface
this spec requires and nothing else. Rewriting it would be a chance to write it
differently, and two different baselines in one module would make the two topics
incomparable.

## Experiment

A campaign of **5** runs, in `release`, **10000** samples per point, **64** churn
steps per sample and **64** warm-up rounds, the same in both arms.

**The freeing order is shuffled by a step coprime with the live set.** Always
freeing the last one would turn the pattern into a stack, which is both arms'
best case — the allocator would hand back the block just freed, and the pool the
slot just pushed. The table would publish the best case under the name of churn.

The sweep is over the **live-set size**: 64 nodes are 4 KB and fit in L1; 4096
are 256 KB and exceed this machine's L2.

```bash
./scripts/build-all.sh release
./scripts/arquivar-medicao.sh docs/01-memoria/02-pool-freelist bench-pool pool-contra-heap 5
```

## Execution environment

Generated, not described:
[`ambiente.md`](bench/medicoes/historico/2026-09-22-pool-contra-heap/ambiente.md).

## Results

| Metric | Median across runs | Spread across runs | Unit |
|---|---:|---:|---|
| std::allocator, 64 live — p50 | 481.00 | 20.8% | ns |
| pool, 64 live — p50 | 161.00 | 19.3% | ns |
| std::allocator, 64 live — p99 | 501.00 | 18.0% | ns |
| pool, 64 live — p99 | 171.00 | 128.7% | ns |
| std::allocator, 512 live — p50 | 421.00 | 21.4% | ns |
| pool, 512 live — p50 | 150.00 | 20.0% | ns |
| std::allocator, 512 live — p99 | 451.00 | 20.0% | ns |
| pool, 512 live — p99 | 151.00 | 26.5% | ns |
| std::allocator, 4096 live — p50 | 411.00 | 26.8% | ns |
| pool, 4096 live — p50 | 150.00 | 26.7% | ns |
| std::allocator, 4096 live — p99 | 431.00 | 23.2% | ns |
| pool, 4096 live — p99 | 151.00 | 27.2% | ns |

## What this measurement does not show

- **what the benchmark removed:** a single stream, a fixed-size live set, a
  single object size. The global allocator has a per-thread cache, and it is
  under contention that it degrades — here it is never contended;
- **which arm that favours:** the `std` one, again. This is its best case;
- **what conclusion a reader would draw stopping at the table:** that the pool
  wins ~3×. That is true, and it is **a tenth** of what the arena won in 01.01 —
  and that comparison is the subject;
- **what would be needed** for the broader conclusion: a live set that grows and
  shrinks, objects of different sizes, and more than one thread.

## When it goes wrong

**The free list empties.** Invariant 5 says `acquire()` returns `nullptr`, and
keeps returning it — and, after the live ones are returned, the pool serves the
whole capacity again. The test requires all three.

The capacity in the benchmark is **exactly** the live set, with no slack: the
step frees before acquiring, so there are never more than N live. Slack would
hide the regime where the list runs empty, which is precisely this topic's.

## Analysis

**The pool wins 2.7× to 3.0×, and that is not the comparison that matters.**

The [arena](../01-alocador-arena/README.en.md), at batch 64, won **27.9×**. The
pool, under churn, wins **3.0×** — and the two structures do almost the same
thing inside: one adds to an index, the other reads an index from a list.

The difference between 27.9× and 3.0× **is the price of individual freeing**.
Not the price of implementing it — the price of the baseline having it too. When
the pattern allows throwing everything away at once, `std::allocator` has to
return block by block and the arena has to return nothing; when the pattern
requires returning one at a time, both return one at a time, and what is left for
the pool is the difference between reading an index and walking the allocator's
structures.

**The per-step cost barely moves with the live set**: 7.52 ns in `std` with 64
live, 6.42 with 4096; in the pool, 2.52 and 2.34. One would expect 4096 nodes —
256 KB, above L2 — to hurt both, and they do not. Churn recycles the same slots,
and the **effective** working set is far smaller than the live set. It remains an
open question whether the same holds with random rather than cyclic freeing.

## Decision rule

> **Under churn, the pool is worth ~3× over the global allocator, at any live-set
> size — and it is worth the same at p50 and at p99.**

| Field | Value |
|---|---|
| starting structure → replacement | `std::allocator<node>` → a pool with a free list |
| decision variable | live-set size |
| turning point | **none** — the gain is ~3× from 64 to 4096, and there is no size at which `std` catches up |
| gain | **3.0×** at p50 and **2.9×** at p99 with 64 live; **2.7×** and **2.9×** with 4096 |
| percentile where it shows | both, in the same proportion |
| machine, configuration, commit | see the [`metadata.json`](bench/medicoes/historico/2026-09-22-pool-contra-heap/metadata.json) |
| **what the rule does not cover** | a growing live set, objects of different sizes, random freeing, more than one thread |

**The rule has a corollary worth more than the rule itself:** if your pattern
allows the arena, use the arena — the pool's 3× is a tenth of its 27.9×. The pool
is what you use when the arena is **not** possible.

## Literature comparison

The finding that per-step cost barely varies with the live set is consistent with
glibc's per-thread cache: under cyclic churn, recently freed blocks come straight
back and never leave the allocator's fast structure. The pool-allocator
literature usually reports gains of an order of magnitude; this topic measures
**3×**, and the difference is that most of that literature compares against
allocation **without** the recycling pattern, which is the modern allocator's best
case.

## Trade-offs

| Choice | Gain | Price |
|---|---|---|
| pool | ~3× under churn, contiguous live objects, a declared ceiling | 4 bytes of index per slot; a capacity to size |
| `std::allocator` | general purpose, no ceiling | ~3× more expensive in this pattern |

## When to use

When the live set is bounded and known, and objects come and go individually.

## When not to use

When the batch dies whole — there the [arena](../01-alocador-arena/README.en.md)
is an order of magnitude better. And when objects have different sizes: one pool
per size becomes managing several pools, which is where
[`std::pmr`](../README.en.md) comes in.

## Limitations

One machine, one compiler, one stream, a fixed-size object, a constant live set,
cyclic rather than random freeing.

## Exercises

1. Switch the list from LIFO to FIFO and redo it. How much of the gain was the
   warm slot?
2. Free in random order instead of cyclic. Does the effective working set grow,
   and does the gain fall?
3. Measure with two live sets in two threads. Which arm feels it first?

## References

- [01.01 — the arena allocator](../01-alocador-arena/README.en.md)
- [08.01 — the harness measuring itself](../../08-medicao/01-harness/README.en.md)
- [Project standard](../../padrao-do-projeto.en.md), sections 11, 34 and 35

## Navigation

- [Module 01](../README.en.md)
- [Study plan](../../plano-estudo.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
