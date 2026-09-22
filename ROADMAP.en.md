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
- [ ] **absorption delta in the standard** — section 30 (two upstream projects,
      not three), section 11 promoted to the definition of the `std` → `custom`
      pair, a new section for the decision rule, section 29 with the two-CCD
      machine
- [ ] divergence table against this track's setup document: non-existent
      `.wraplock`, HdrHistogram absent from WrapDB, two sanitizer profiles,
      `docs/0N-*/medicoes/`, the harness C ABI, CCD and SMT in the metadata,
      arm64 without a machine, non-invariant TSC not reproducible here
- [ ] `contract.hpp` name collision: a topic's law becomes `spec.hpp`
- [ ] path collision: the inherited `.gitignore` ignores `resultados/**`, while
      this track's setup archives under `bench/medicoes/` — one of the two names goes
- [ ] the `CPP_ACADEMY_*` prefix on the measurement ruler's environment
      variables: kept as-is during absorption, to be decided against the new identity
- [ ] the `language ↔ compiler ↔ generated code` axis, which was the absorbed
      project's own object, gets a cross-cutting module `10-codigo-gerado`
- [ ] `pre-commit.sh` and CI running the three-configuration matrix
- [ ] a **language-pair** checker: the identical-stem rule (standard, section 2)
      exists to be verifiable, and today it is not

## Stage 2 — Environment

- [ ] `scripts/ambiente.sh`, ported **without rewriting the parsing**: the
      upstream version uses `lscpu -p=` and sysfs because parsing `lscpu` text
      silently yields empty fields on a machine with a translated locale — and
      this machine has one
- [ ] recording PMU availability (`perf_event_paranoid`)
- [ ] recording which of the three configurations produced the number
- [ ] the CCD and SMT sibling of the core used, not just its id

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
