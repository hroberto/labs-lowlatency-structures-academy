> [🇧🇷 Português](README.md) | 🇺🇸 English

# 01.03 — `std::pmr` as the middle ground

**State: measured.** There is a published number, with a generated environment
and a program that produces it, archived under
[`bench/medicoes/`](bench/medicoes/historico/2026-09-22-pmr-contra-arena/).

## Objectives

Answer whether **the standard library already offers the mechanism** the two
previous topics wrote by hand — and, if it does, how much of the gain it
delivers.

## Problem

Topics [01.01](../01-alocador-arena/README.en.md) and
[01.02](../02-pool-freelist/README.en.md) compared `std::allocator` against
hand-written code, and the hand-written side won 27.9× and 3.0×. But
`std::allocator` is **not the only thing the language offers**: since C++17
there is `std::pmr::monotonic_buffer_resource`, which is an arena.

If it delivers the same, the two previous topics stay correct and gain an
expensive caveat: they measured the cost of `std::allocator`, not the cost of
what the language offers.

**The order is deliberate.** The middle ground can only be judged once both
extremes are known — otherwise "pmr is good" has nothing to be good against.

## Mental model

`monotonic_buffer_resource` allocates by bumping a pointer inside a buffer you
hand it; `release()` returns everything at once. It is 01.01's design, written by
the committee.

The internal difference is that it is **polymorphic**: every `allocate` goes
through virtual dispatch and a generic size-and-alignment path, because it serves
any type. 01.01's arena serves one type and knows that at compile time.

## How it works

The spec is **the same as 01.01's** — [`spec.hpp`](spec.hpp) only says so out
loud. The L1 runs the seven invariants over the **three** arms, and that is where
the claim becomes verification: if `pmr_arm` fulfilled a slightly different law,
the file would not compile.

And the topic's result shows up in the test output before it shows up in the
table: `pmr_arm` skips **exactly the same case** as the hand-written arena —
invariant 7, individual freeing. The standard library offers the same mechanism,
with the same renunciations.

## Implementation

**The upstream resource is `null_memory_resource`, and that is invariant 6.** By
default, when the buffer runs out, `monotonic_buffer_resource` asks the upstream
resource for more — and that is the global allocator. This is convenient and it
destroys the ceiling: mid-batch, with no warning, allocation costs what it used
to cost again.

With `null_memory_resource`, exhaustion becomes `std::bad_alloc`, which this arm
converts into `nullptr` to fulfil the spec. It is the difference between *an
arena with a ceiling* and *an arena that sometimes becomes the heap*, and it is a
configuration decision that barely appears in `pmr` examples.

### A defect invariant 6 caught

The first version reserved `capacity * sizeof(node) + alignof(node)` bytes, "for
alignment". But `alignof(node)` is 64 and `sizeof(node)` is 64: the slack was
**exactly one node**, and the arm served `capacity + 1`.

The test reported it — *"served beyond the declared capacity"* — and that is
literally what it exists for. A ceiling that is not the declared ceiling is worse
than no ceiling: the batch gets sized against a number the code does not respect.

## Experiment

A campaign of **5** runs, in `release`, **10000** samples per point, **64**
warm-up batches identical across the three arms, and **the same load as 01.01** —
same batch sizes, same object, same sample shape. Without that, this topic's
question would have nothing to be answered against.

The `std::allocator` arm appears again, and not for symmetry: it is the **anchor**
that says whether these numbers and 01.01's came from the same machine in the
same state.

```bash
./scripts/build-all.sh release
./scripts/arquivar-medicao.sh docs/01-memoria/03-pmr-o-meio-termo bench-pmr pmr-contra-arena 5
```

## Execution environment

Generated, not described:
[`ambiente.md`](bench/medicoes/historico/2026-09-22-pmr-contra-arena/ambiente.md).

## Results

| Metric | Median across runs | Spread across runs | Unit |
|---|---:|---:|---|
| std::allocator, batch 64 — p50 | 1674.00 | 26.9% | ns |
| hand-written arena, batch 64 — p50 | 60.00 | 16.7% | ns |
| std::pmr, batch 64 — p50 | 90.00 | 33.3% | ns |
| std::allocator, batch 512 — p50 | 14758.00 | 1.0% | ns |
| hand-written arena, batch 512 — p50 | 371.00 | 2.4% | ns |
| std::pmr, batch 512 — p50 | 591.00 | 1.7% | ns |
| std::allocator, batch 512 — p99 | 20710.10 | 56.5% | ns |
| hand-written arena, batch 512 — p99 | 381.00 | 2.6% | ns |
| std::pmr, batch 512 — p99 | 601.00 | 0.2% | ns |
| std::allocator, batch 4096 — p50 | 88829.00 | 1.6% | ns |
| hand-written arena, batch 4096 — p50 | 3526.00 | 70.4% | ns |
| std::pmr, batch 4096 — p50 | 5020.00 | 71.6% | ns |

Batch 8 stayed **out of the table**: there all three arms measure 2.50 ns per
node, which is the instrument floor measured in
[08.01](../../08-medicao/01-harness/README.en.md). Publishing the row would
invite comparing numbers that describe the clock.

## What this measurement does not show

- **what the benchmark removed:** a single object type, fixed size, one stream.
  `pmr` exists for the case where types are many and the allocator is chosen at
  run time — and that case is **not** measured here;
- **which arm that favours:** the **hand-written arena**. It is specialized on
  one type, and this benchmark has one type. Its advantage would shrink in a
  program with ten different types, where the arena would become ten arenas;
- **what conclusion a reader would draw stopping at the table:** that the
  hand-written arena is worth it. It is worth 1.5×, and the table does not say
  what maintaining it costs;
- **what would be needed** for the broader conclusion: several object types, and
  `pmr` used as it was designed — with containers receiving the resource, rather
  than direct calls to `allocate`.

## When it goes wrong

**The buffer runs out mid-batch.** With `null_memory_resource` upstream,
`allocate` throws `std::bad_alloc` and the arm returns `nullptr` — a declared
ceiling that stays declared. **Without** that configuration, the same case
silently allocates from the heap and the measurement would publish a number
mixing both paths.

The exception path sits in the hot loop and costs nothing while it does not fire:
GCC's and Clang's model on x86-64 is zero-cost. When it fires, the batch is over.

## Analysis

**The standard library already offers the mechanism, and it delivers most of the
gain.** At batch 512, `std::allocator` costs 28.82 ns per node, `pmr` costs 1.15
and the hand-written arena costs 0.72 — **25.0×** and **39.8×** over the
baseline.

Put another way: **the hand-written arena is 1.59× faster than `pmr`**, not 40×
as a hasty reading of 01.01 might suggest. The 27.9× that 01.01 published is not
the gain of "writing an arena" — it is the gain of "not using `std::allocator`
for this pattern", and `pmr` captures most of it without a line of hand-written
code.

The 1.59× difference has a known cause: `monotonic_buffer_resource` is
polymorphic — virtual dispatch per call, and a generic size-and-alignment path.
01.01's arena serves one type and knows that at compile time.

At batch 8 all three coincide at 2.50 ns per node, because all three sit at the
instrument floor. **The 1.00× ratio there is not a tie: it is the absence of a
measurement.**

## Decision rule

> **Use `std::pmr::monotonic_buffer_resource` with `null_memory_resource`
> upstream. The hand-written arena is only justified if 1.6× is the difference
> between fitting and not fitting the budget — and that has to be measured, not
> assumed.**

| Field | Value |
|---|---|
| starting structure → replacement | `std::allocator<node>` → `std::pmr::monotonic_buffer_resource` |
| decision variable | allocations per batch |
| turning point | **64**, the same as 01.01's — below that the measurement cannot tell any of the three from the instrument |
| gain over the baseline | **25.0×** at the p50 of batch 512 |
| what the hand-written arena adds | **1.59×** over `pmr`, at the same point |
| percentile where it shows | both; at batch 512 `pmr`'s p99 is 601 ns against `std::allocator`'s 20710 |
| machine, configuration, commit | see the [`metadata.json`](bench/medicoes/historico/2026-09-22-pmr-contra-arena/metadata.json) |
| **what the rule does not cover** | several object types, `pmr` used with containers, run-time resource choice, more than one thread |

**This is a result where the standard library suffices**, and it is worth as much
as the two before it: publishing only the cases where the hand-written side wins
would produce material that confirms what the author already believed.

## Literature comparison

Material on `pmr` usually presents it as a convenience tool — swapping the
allocator without swapping the type — and rarely publishes the cost of polymorphic
dispatch against a specialized arena. This topic measures **1.59×** on this
machine, for a single type.

It remains open how much of that factor is virtual dispatch and how much is the
generic alignment path. Telling the two apart requires reading generated code,
which is module [10](../../10-codigo-gerado/README.en.md).

## Trade-offs

| Choice | Gain | Price |
|---|---|---|
| `std::pmr` | 25× over the baseline, with no hand-written code; serves any type | 1.59× slower than the specialized arena; the upstream resource **must** be configured |
| hand-written arena | another 1.59× | one type per arena; code to maintain |
| `std::allocator` | general purpose | 25× more expensive in this pattern |

## When to use

Whenever the pattern is a batch that dies whole — which is when 01.01 applies.
`pmr` is the starting point; the hand-written arena is an optimization that needs
a number to justify it.

## When not to use

When objects come and go individually: there neither `pmr` nor the arena serves,
and the way is the [pool from 01.02](../02-pool-freelist/README.en.md).

## Limitations

One machine, one compiler, one object type, one stream. `pmr` was used through
direct calls to `allocate`, not through a container — which is the use it was
designed for.

## Exercises

1. Remove `null_memory_resource` and leave the default upstream. Which percentile
   reports the batch that overran the buffer?
2. Measure with four object types of different sizes. Does the arena's 1.59×
   advantage survive?
3. Use `pmr` through a `std::pmr::vector`. Does the per-node cost change?

## References

- [01.01 — the arena allocator](../01-alocador-arena/README.en.md)
- [01.02 — the pool with a free list](../02-pool-freelist/README.en.md)
- [Project standard](../../padrao-do-projeto.en.md), sections 11, 34 and 35

## Navigation

- [Module 01](../README.en.md)
- [Study plan](../../plano-estudo.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
