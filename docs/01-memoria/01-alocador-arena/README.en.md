> [🇧🇷 Português](README.md) | 🇺🇸 English

# 01.01 — Arena allocator versus the global allocator

**State: measured.** There is a published number, with a generated environment
and a program that produces it, archived under
[`bench/medicoes/`](bench/medicoes/historico/2026-09-22-arena-contra-heap/).

## Objectives

Measure the allocation rate at which the global allocator starts dominating the
cost of a batch, and the point at which swapping it for an arena pays off — with
the number that sustains the rule.

## Problem

The hot-path pattern in this project is **a batch is born, used, and dies
whole**: messages arrive, become objects, get processed, disappear. In that
pattern, `new`/`delete` pays for two things the batch does not need — finding a
free block for each object, and returning each one separately.

The question is not whether it can be done better. It is **from where on** it
makes a difference.

## Mental model

| | `std::allocator` | arena |
|---|---|---|
| allocate | finds a free block, updates the allocator's structures | adds one to an index |
| free | object by object | zeroes the index |
| ceiling | none; throws when memory runs out | declared, and returns `nullptr` |
| free one alone | **yes** | **no** |

The last two rows are the whole trade. Everything the arena gains comes from
having given up individual freeing.

## How it works

[`spec.hpp`](spec.hpp) declares **seven invariants**. Five hold for both arms;
two hold only for whoever honours them, and each arm says in `traits` which are
its own. The L1 test is parameterized by the spec and consults the `traits`: an
invariant an arm does not honour becomes a case **skipped with a reason**, never
a missing case.

That is not laxity in the spec — it is the topic's subject. Requiring individual
freeing from both would produce a spec only `std::allocator` fulfils, and the
comparison would become *"the arena is a worse `std::allocator`"*.

## Implementation

The [`std/`](std/heap_arm.hpp) arm uses `std::allocator<node>` rather than raw
`new`: it is what the standard library offers whoever may want to swap the
allocator later. The [`custom/`](custom/arena_arm.hpp) arm reserves its buffer
**once**, in the constructor, outside any measurement — an arena growing on
demand would call the global allocator mid-batch, and would start measuring, now
and then, the very thing it exists to avoid.

**Neither arm keeps the list of pointers.** The caller keeps it, the same for
both. If each arm kept its own, the `std` one would also pay the bookkeeping,
and the table would publish as "cost of allocating" what is the cost of
bookkeeping (standard, section 11).

The object is **64 bytes** with `alignas(64)`: exactly one cache line. Smaller
than that, two neighbouring objects would share a line and the topic would start
measuring false sharing too, which is module 02's subject.

## Experiment

A campaign of **5** runs, in `release`, with **10000** samples per point and
**64** warm-up batches — the same in both arms, because without them the first
allocation of each pays the first page fault of its buffer and the table would
publish the warm-up difference under the structure's name.

Each sample is **one whole batch**: acquire L nodes, write to each, release them
all. The clock is read twice per batch, not per object — this machine's
per-operation floor is **20** ns ([08.01](../../08-medicao/01-harness/README.en.md))
and an allocation costs on the order of 25 ns; measuring object by object would
publish a table where half the number is the clock.

**A sweep, not a single point**: the batch size is the variable.

```bash
./scripts/build-all.sh release
./scripts/arquivar-medicao.sh docs/01-memoria/01-alocador-arena bench-arena arena-contra-heap 5
```

## Execution environment

Generated, not described:
[`ambiente.md`](bench/medicoes/historico/2026-09-22-arena-contra-heap/ambiente.md)
and the same content inside
[`metadata.json`](bench/medicoes/historico/2026-09-22-arena-contra-heap/metadata.json).

## Results

| Metric | Median across runs | Spread across runs | Unit |
|---|---:|---:|---|
| std::allocator, batch 8 — p50 | 60.00 | 33.3% | ns |
| arena, batch 8 — p50 | 20.00 | 50.0% | ns |
| std::allocator, batch 64 — p50 | 1673.00 | 26.4% | ns |
| arena, batch 64 — p50 | 60.00 | 50.0% | ns |
| std::allocator, batch 64 — p99 | 1703.00 | 145.3% | ns |
| arena, batch 64 — p99 | 71.00 | 28.2% | ns |
| std::allocator, batch 512 — p50 | 14828.00 | 3.9% | ns |
| arena, batch 512 — p50 | 371.00 | 5.1% | ns |
| std::allocator, batch 512 — p99 | 18966.00 | 35.6% | ns |
| arena, batch 512 — p99 | 481.00 | 22.9% | ns |
| std::allocator, batch 4096 — p50 | 88388.00 | 1.2% | ns |
| arena, batch 4096 — p50 | 3576.00 | 82.1% | ns |
| std::allocator, batch 4096 — p99 | 91704.30 | 12.0% | ns |
| arena, batch 4096 — p99 | 4458.00 | 62.5% | ns |

## What this measurement does not show

- **what the benchmark removed:** a single stream, with no other thread
  contending for the allocator. That is the `std::allocator`'s best case, and it
  favours **it**: the global allocator has a per-thread cache, and it is under
  contention that it degrades;
- **which arm that favours:** the `std` one, on both axes — no contention and a
  batch always of the same size, the pattern that helps the free list most;
- **what conclusion a reader would draw stopping at the table:** that the arena
  is ~25 to 40 times faster. That is true **in this usage pattern** and says
  little outside it;
- **what would be needed** for the broader conclusion: batches of varying size,
  objects of different sizes, and more than one thread.

## When it goes wrong

**The arena runs out mid-batch.** That is the module's failure question, and the
answer is invariant 6: `acquire()` returns `nullptr`, and keeps returning it —
exhaustion is not a transient state. The `std` arm **does not honour that
invariant**, and its case is skipped with that reason written down: when memory
runs out, it throws.

In the benchmark, a `nullptr` mid-batch is a **measurement error**, not a path
to measure: the program fails rather than publishing an incomplete batch as if
it were a batch.

## Analysis

**The `std::allocator`'s per-node cost is not constant, and that is what answers
the module's question.** It goes from **7.50** ns at batch 8 to **26.14** ns at
batch 64 and **28.96** ns at batch 512. A small batch recycles the same few
blocks, which stay warm in the allocator's cache; from a few dozen on, every
allocation touches a new block.

The arena moves the other way: **2.50** ns per node at batch 8, **0.72** at
batch 512. It amortizes — what it does per object is add one to an index, and
the rest of the batch's cost is writing to the objects, which both arms pay
equally.

**At batch 8, the arena sits at the instrument's floor.** The 20.00 ns measured
are exactly the per-operation floor published in
[08.01](../../08-medicao/01-harness/README.en.md). The number does not describe
the arena: it describes the clock. The table row stays, with this caveat beside
it — and it is why the decision rule does not start at 8.

The ratio between the arms goes from **27.9×** at the p50 of batch 64 to
**40.0×** at batch 512, and falls back to **24.7×** at batch 4096, when the
working set stops fitting in cache and both arms start waiting on memory.

## Decision rule

> **Above ~64 allocations per batch, swapping the global allocator for the arena
> is worth an order of magnitude — and the gain is larger at p99 than at p50.**

| Field | Value |
|---|---|
| starting structure → replacement | `std::allocator<node>` → an arena with a declared ceiling |
| decision variable | allocations per batch |
| turning point | **64** — below that the measurement cannot tell the arena from the instrument |
| gain at the turning point | **27.9×** at p50, **24.0×** at p99 |
| percentile where it shows | both; at batch 512 the `std` p99 is 39.4× the arena's |
| machine, configuration, commit | see the [`metadata.json`](bench/medicoes/historico/2026-09-22-arena-contra-heap/metadata.json) |
| **what the rule does not cover** | batches of varying size, objects of different sizes, more than one thread, and any use that needs to free an object individually |

The last row matters most: **the rule only holds where the batch dies whole**.
Outside that, invariant 7 — which the arena does not honour — stops being a
detail and becomes an impediment.

## Literature comparison

The curve's shape matches what is documented about glibc's allocator: a
per-thread cache serves small, recently freed blocks without a lock, which is
why batch 8 is cheaper **per node** than batch 64. What this topic adds is not
the mechanism, it is this machine's number and the **point** at which it stops
being enough.

It remains open whether the knee between 8 and 64 is the limit of the per-thread
cache or of the data cache itself. Telling those apart needs a hardware counter,
which is module [08.02](../../08-medicao/README.en.md).

## Trade-offs

| Choice | Gain | Price |
|---|---|---|
| arena | constant-time allocation and O(1) release | no individual freeing; a ceiling to size |
| `std::allocator` | general purpose, no ceiling, frees individually | a per-object cost that grows with the batch |

## When to use

When the batch is born and dies whole, and the maximum size is known — which is
the case of the message decoder and the in-memory index it feeds.

## When not to use

When an object has to outlive its siblings. The arena does not serve there, and
the way out is not "an arena with individual freeing" — that is a pool with a
free list, which is [topic 01.02](../README.en.md).

## Limitations

One machine, one compiler, one stream, a fixed-size object. `governor` on
`powersave`. The turning point of 64 belongs to **this** machine: on another,
with another system allocator, it moves — and the program that finds it is here.

## Exercises

1. Run the sweep with batches 16 and 32 and locate the knee more precisely. Is
   it closer to 8 or to 64?
2. Change `node` from 64 to 32 bytes and redo it. Does the turning point move?
3. Make the arena grow on demand instead of reserving in the constructor. Which
   percentile feels it first?

## References

- [Project standard](../../padrao-do-projeto.en.md), sections 11, 34 and 35
- [08.01 — the harness measuring itself](../../08-medicao/01-harness/README.en.md)
- [Module 01 — memory](../README.en.md)

## Navigation

- [Module 01](../README.en.md)
- [Study plan](../../plano-estudo.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
