> [🇧🇷 Português](README.md) | 🇺🇸 English

# Low-Latency Structures Academy

*Beyond the Standard Library.* Where the C++ standard library stops and the hot
path begins. Every published number has a program that produces it, measured on
a named machine.

## Project state

> **Skeleton.** This document is a skeleton and says so. The project's public
> identity — thesis, readers, what the project is not — lives in
> [`docs/origem/setup-lowlatency-structures-academy.md`](docs/origem/setup-lowlatency-structures-academy.md)
> and has **not** yet been rewritten for the reader. **No performance number has
> been published.**

| Component | State |
|---|---|
| [Project standard](docs/padrao-do-projeto.en.md) | **written** — 36 sections, 7 parts; absorption delta applied |
| [Reference catalogue](docs/referencias.en.md) | **written** — addresses pending verification |
| [Quality tooling](ferramental/qualidade/) | **code exists, not measured** — 5 checkers, self-tests passing |
| [Measurement ruler](lib/measurement/) | **code exists, not measured** — builds in all three configurations |
| [Contract library](lib/contract/) | **code exists, not measured** — three gates, suite in all three configurations |
| [`scripts/`](scripts/) | **code exists, not measured** — `check-env.sh`, `ambiente.sh` (text, markdown, JSON), `build-all.sh`, `test-all.sh`, `arquivar-medicao.sh`, with self-tests in the suite |
| [CI](.github/workflows/ci.yml) and [pre-commit](ferramental/qualidade/pre-commit.sh) | **code exists, not measured** — consistency before build; GCC 14 × Clang 18 × three configurations matrix |
| [Module track](trilha/README.en.md) | **one measured topic** — [08.01, the harness measuring itself](trilha/08-medicao/01-harness/README.en.md); the other eight modules not started |
| Capstone (order book engine) | **not started** |

State labels are normative — see
[project standard, section 5](docs/padrao-do-projeto.en.md#5-state-labelling).

## The thesis

Every topic takes the mechanism the language ships, measures the exact point
where it stops serving, and builds the complement — in the same harness, under
the same contract, on the same machine. The `std` → `custom` pair is the unit of
work, and the **decision rule** — the volume, cardinality or percentile beyond
which the swap pays off — is the deliverable.

## Origin

This repository absorbed the foundation of `labs_cpp_performance_tuning_academy`
(standard, measurement ruler, contract library, checkers), which was never
published. Both setup documents are preserved intact under
[`docs/origem/`](docs/origem/).

## Navigation

- [Documentation and standard](docs/README.en.md)
- [Build order](ROADMAP.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
