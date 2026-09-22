> [🇧🇷 Português](regras-de-decisao.md) | 🇺🇸 English

# Decision rules

> **State: 3 rules.** One per topic that compares. This page is the project's
> **internal-reference** artefact — what you consult when designing a system,
> not when studying.

Every row here comes from an archived campaign, and every campaign has a
program, a generated environment and a commit. No rule is an opinion wearing the
clothes of a reference: the **campaign** column is where you check.

## How to read this table

The **turning point** is a number — volume, cardinality, arrival rate or
percentile — never an adjective. Where it is empty, the swap holds across the
whole measured range, and the range is stated.

The column that decides usage is the **last one**. A rule without its limit is
not a rule: it is a slogan. Before applying any row, read what it does not
cover.

**All numbers are from this machine** — an AMD Ryzen 9 9900X, `governor` on
`powersave`, one stream, GCC 15.2 in `release`. What transfers across machines is
the **shape** of the curve and the mechanism; the turning point, not necessarily.

## The rules

| # | Start → replacement | Variable | Turning point | Gain | Campaign |
|---|---|---|---|---|---|
| [01.01](01-memoria/01-alocador-arena/README.en.md) | `std::allocator` → arena | allocations per batch | **64** | 27.9× at p50, 24.0× at p99 | [2026-09-22](01-memoria/01-alocador-arena/bench/medicoes/historico/2026-09-22-arena-contra-heap/) |
| [01.02](01-memoria/02-pool-freelist/README.en.md) | `std::allocator` → pool with a free list | live-set size | — (holds from 64 to 4096) | 3.0× at p50 and 2.9× at p99 | [2026-09-22](01-memoria/02-pool-freelist/bench/medicoes/historico/2026-09-22-pool-contra-heap/) |
| [01.03](01-memoria/03-pmr-o-meio-termo/README.en.md) | `std::allocator` → `std::pmr::monotonic_buffer_resource` | allocations per batch | **64** | 25.0× at p50; the hand-written arena adds 1.59× | [2026-09-22](01-memoria/03-pmr-o-meio-termo/bench/medicoes/historico/2026-09-22-pmr-contra-arena/) |

## What each rule does **not** cover

| # | Out of reach |
|---|---|
| 01.01 | batches of varying size, objects of different sizes, more than one thread, and any use that needs to free an object individually |
| 01.02 | a growing live set, objects of different sizes, random rather than cyclic freeing, more than one thread |
| 01.03 | several object types, `pmr` used with containers rather than direct `allocate`, run-time resource choice, more than one thread |

Three out of three say **more than one thread**. That is no coincidence: the
global allocator has a per-thread cache, and it is under contention that it
degrades. All three rules were measured in the **baseline's best case**, and are
therefore conservative — under contention the swap tends to be worth more, not
less.

## What the three say together

Read in sequence, they tell something none of them says alone:

**The arena's gain is not the gain of writing an arena.** 01.01 measured 27.9×
over `std::allocator`, and 01.03 showed that `std::pmr` — which already ships
with the language — delivers 25.0× of those same 27.9×. What hand-written code
adds is **1.59×**, not an order of magnitude.

**Most of the gain comes from giving up individual freeing, not from writing
code.** 01.02 measures the pool, which does almost the same thing inside as the
arena — reading an index instead of adding to one — and wins **3.0×** instead of
27.9×. The difference between those two numbers is the price of the *baseline*
having to free one at a time too.

Hence the practical order: **check whether your pattern allows throwing
everything away at once.** If it does, `std::pmr` solves most of it. If it does
not, the pool gives 3× and there is no escaping that without changing the
pattern.

## Navigation

- [Documentation and standard](README.en.md)
- [Study plan](plano-estudo.en.md)

> [🇧🇷 Português](regras-de-decisao.md) | 🇺🇸 English
