> [🇧🇷 Português](README.md) | 🇺🇸 English

# 08 — Measurement

> **State: one measured topic.** [08.01](01-harness/README.en.md) has a
> published number with an archived campaign; 08.02 has not been started.

## The module's question

> how does the harness avoid measuring itself?

It does not. It measures its own cost, publishes that number, and only then can
attribute the rest to the operation under test. What is avoided is measuring
itself **without knowing**.

## The failure question

> when does the warm-up hide the tail?

## Why this module comes first in the build order

It is not first in the study order — it is first in the build order, and the
distinction lives in the [ROADMAP](../../ROADMAP.en.md). A structure topic
written before it would publish per-operation percentiles without the
instrument's floor being measured, and the difference between *the structure
costs 20 ns* and *the instrument costs 20 ns* would show up in no test at all.

This module is **cross-cutting**: there is no `std` → `custom` pair here,
because it does not compare two structures — it measures the instrument the
others will be compared with. For the same reason it has no *Decision rule*:
section 35 of the standard requires one from whoever compares.

## Topics

| Topic | Question | State |
|---|---|---|
| [`01-harness`](01-harness/README.en.md) — the harness measuring itself | below what latency does per-operation measurement stop describing the operation? | **measured** |
| `02-perf-e-cache-misses` — hardware counters | what does the PMU show that the clock does not? | **not started** |

08.02 depends on an accessible PMU. On the reference machine that required
lowering `kernel.perf_event_paranoid` from 4 to 2 — the smallest value that
serves, because a performance counter is a side channel. See
[`scripts/check-env.sh`](../../scripts/check-env.sh), which diagnoses it.

## What this module delivers to the others

- `lib/measurement/tail.hpp` — exact percentiles over raw samples, with the
  convention for how many samples each percentile requires;
- `scripts/arquivar-medicao.sh` — the campaign, with repetitions and the
  **spread across runs**, which is what separates the structure's tail from the
  machine's;
- this machine's **per-operation floor**, published, which every topic measuring
  per operation must subtract or avoid with a criterion.

## Navigation

- [Documentation and standard](../README.en.md)
- [Repository README](../../README.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
