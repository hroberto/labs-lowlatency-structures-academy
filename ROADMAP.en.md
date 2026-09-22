> [🇧🇷 Português](ROADMAP.md) | 🇺🇸 English

# Project roadmap

**This document is the BUILD order of the material. It is not a reading order.**

The canonical study order will be the track's, in `docs/plano-estudo.md`, **not
yet written**. The distinction exists because in the upstream project two
competing numberings shared one README and "level 5" came to mean two different
things depending on the document.

> **Skeleton.** This document is a skeleton and says so. The stages below
> stage 2 come from
> [`docs/origem/setup-lowlatency-structures-academy.md`](docs/origem/setup-lowlatency-structures-academy.md)
> and have not yet been reconciled with the absorbed standard.

## Stage 1 — Absorbing the foundation · **partial**

`labs_cpp_performance_tuning_academy` — C++23, same machine, same method, track
not started, **zero commits and no remote** — was absorbed by this repository on
2026-09-21. What was already executable came over:

- [x] `git init`, MIT licence, `.gitignore`, `.gitattributes`, `.clang-format`,
      `.clang-tidy`
- [x] **both** origin documents preserved intact under
      [`docs/origem/`](docs/origem/)
- [x] [standard](docs/padrao-do-projeto.en.md): 33 sections, 6 parts, with the 14
      divergences from its own origin recorded auditably
- [x] [reference catalogue](docs/referencias.en.md) with stable identifiers
- [x] `lib/measurement/` — the C++23 measurement ruler, header-only on purpose:
      a call across a translation boundary would measure the call together with
      the operation
- [x] `lib/contract/` — the three gates T/R/E, and `verificar-suposicao.py`,
      which checks in the assembly that the gate-R precondition reaches the
      optimizer
- [x] 5 checkers, each with **its own self-test** registered beside it
- [x] `meson.build` with the three-configuration matrix and the conditional
      `-Werror=assume`
- [x] suite green in all three configurations on **this** machine (GCC 15.2.0):
      `debug` and `release-checked` at 11 OK + 1 expected failure, `release` at
      11 OK + 1 skipped
- [x] **absorption delta in the standard** — 33 sections and 6 parts became 36
      and 7: section 30 reconciled, section 11 promoted to the pair's publication
      gate, section 16 without arm64 and with two sanitizer profiles, sections 28
      and 29 with CCD, histogram precision, between-run variation and the named
      machine, and the new **Part VII** — the `std` and `custom` pair (34), the
      decision rule (35), the generated-code axis (36)
- [x] divergence table against this track's setup document: 12 items, each
      verified on this machine before entering the standard
- [x] `contract.hpp` name collision: a topic's law is `spec.hpp` (section 34)
- [x] path collision, and **the rule behind it inverted**: the name is
      `bench/medicoes/`, and the campaign history is now **versioned**. The
      inherited `.gitignore` excluded raw output; in a project whose asset is the
      provenance of the number, that removes the evidence. The DPDK Academy's
      practice holds: `medicoes/historico/<date>-<campaign>/`, environment
      beside it, one output per repetition
- [x] the `language ↔ compiler ↔ generated code` axis became section 36, and the
      cross-cutting module `10-codigo-gerado` is planned in the track
- [ ] the `CPP_ACADEMY_*` prefix on the measurement ruler's environment
      variables: kept as-is during absorption, to be decided against the new identity
- [ ] `pre-commit.sh` and CI running the three-configuration matrix
- [ ] a **language-pair** checker: the identical-stem rule (standard, section 2)
      exists to be verifiable, and today it is not

## Stage 2 — Environment · **partial**

- [x] `scripts/ambiente.sh`, ported **without rewriting the parsing**: the
      upstream version uses `lscpu -p=` and sysfs because parsing `lscpu` text
      silently yields empty fields on a machine with a translated locale — and
      this machine has one. Three modes: text, `--markdown`, `--json`
- [x] recording PMU availability (`perf_event_paranoid`)
- [x] recording which build configurations exist in the tree
- [x] the CCD and SMT sibling of every logical CPU, one line each
- [x] `scripts/check-env.sh` — a diagnosis with a real C++23 probe (it compiles
      `<expected>`, `<print>`, `<span>` and
      `hardware_destructive_interference_size` instead of reading the compiler
      version), and every warning saying what stops being possible
- [x] `scripts/build-all.sh`, `scripts/test-all.sh` and
      `scripts/lib-configuracoes.sh`, with the configuration matrix in one place
- [x] self-tests in the `l1+scripts` suite: a synthetic two-L3-domain arm for
      the grouping, and a control arm that hides Meson from `PATH` and requires
      `check-env.sh` to report it
- [ ] **`perf_event_paranoid=4` on this machine blocks hardware counters.** The
      `perf` and cache-miss module cannot measure without `sysctl
      kernel.perf_event_paranoid=0`, which is a system change and stays a
      declared decision, not a silent prerequisite
- [ ] `governor=powersave` on the reference machine: the campaign must declare
      whether it pins `performance` before measuring, or publishes the variation
      the governor introduces

## Stage 3 — Harness and the full cycle

- [ ] `trilha/08-medicao/01-harness/`: a minimal topic that builds, runs and
      archives a trivial measurement with `metadata.json` — to prove the whole
      cycle **before** any content is written
- [ ] HdrHistogram versus raw samples in a preallocated buffer
- [ ] `docs/regras-de-decisao.md` with a data schema and a checker that verifies
      each row against an existing `metadata.json`

## Stage 4 onwards — the track

The nine modules, the order-book scenario and the capstone are described in
[`docs/origem/setup-lowlatency-structures-academy.md`](docs/origem/setup-lowlatency-structures-academy.md),
sections 4 to 6, and enter here once reconciled with the standard.

## Navigation

- [Repository README](README.en.md)
- [Documentation and standard](docs/README.en.md)

> [🇧🇷 Português](ROADMAP.md) | 🇺🇸 English
