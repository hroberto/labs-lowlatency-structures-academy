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
- [x] `SECURITY.md` in both languages, `CITATION.cff` at 0.01.00 citing the
      absorbed project as an unpublished reference, and `CLAUDE.md` — the latter
      exempt from the pair rule by a decision declared in the checker: it is tool
      configuration, and two copies of instructions diverging silently are a
      worse defect than a missing pair
- [ ] the `CPP_ACADEMY_*` prefix on the measurement ruler's environment
      variables: kept as-is during absorption, to be decided against the new identity
- [x] `pre-commit.sh` — syntax, the six checkers plus their six self-tests, a
      secret scan with a declared scope, and a check that the pinned CI action
      SHA is still the top of the major declared in the comment. The `--rapido`
      mode is the CI's; the local hook also runs the suite
- [x] CI in two jobs: consistency before build, and build with a compiler matrix
      (GCC 14 and Clang 18 from the `ubuntu-24.04` image) × the three
      configurations, which come from `lib-configuracoes.sh` and are not
      duplicated in the workflow. Action pinned by SHA,
      `permissions: contents: read`, triggered only on `main` and `pull_request`
- [x] `dependabot.yml` for `github-actions` only, with the reason for having no
      `ignore` rule recorded in the file itself
- [ ] **the CI ↔ reference-machine contract, declared and unresolved:** CI runs
      GCC 14 and Clang 18; the machine measures on GCC 15.2.0 and Clang 21.1.8.
      Portability belongs to CI, measurement to the named machine, and no
      published number comes out of CI. If one day a number has to come from
      there, this becomes a decision to revisit
- [x] a **language-pair** checker: `verificar-paridade.py`, with four rules —
      pair, navigation, structure and numbers —, 14 self-test cases and two baits
      born from its own false accusations (the per-language decimal separator,
      and the example navigation inside a code block)

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
- [x] **`perf_event_paranoid` settled at 2, not 0.** The machine shipped at
      **4** — an Ubuntu extension that blocks `perf_event_open` entirely; the
      upstream kernel stops at 2. The adjustment applied was the **smallest that
      serves**: 2 allows own-process counters in user space, which is what the
      track measures. CPU-wide scope and kernel profiling stay closed, and under
      section 30 of the standard they belong to the DPDK Academy and EX442.
      Verified: `perf stat` returns `cycles:u`, `cache-misses:u` and
      `branch-misses:u` for the benchmark itself
- [ ] the adjustment is **not persistent** and reverts to 4 on the next boot.
      Pinning it in `/etc/sysctl.d/` is a system change, and stays a declared
      decision — the alternative is reapplying it before each campaign, which
      `check-env.sh` diagnoses
- [ ] `governor=powersave` on the reference machine: the campaign must declare
      whether it pins `performance` before measuring, or publishes the variation
      the governor introduces

## Stage 3 — Harness and the full cycle · **partial**

- [x] **decided: raw samples in a preallocated buffer, not HdrHistogram.**
      `lib/measurement/tail.hpp`, exact percentile, with no precision to declare
      because there is no loss to declare. Two verified reasons: HdrHistogram_c
      is not in WrapDB and its log writer pulls in zlib, against section 16; and
      the problem it solves — an unbounded stream in constant space — is not the
      problem of a laboratory that chooses how many operations to measure. What
      is lost is written in the file's header
- [x] the convention for how many samples a percentile requires:
      `min_samples_for(p)`, with 10 observations beyond the percentile — 1000
      samples for p99, 10,000 for p99.9 — and `tail_statistics` carrying the
      highest supported percentile so that publishing above it is a visible error
- [x] `trilha/08-medicao/01-harness/`: it builds, runs, and archives a campaign
      of 5 runs with `metadata.json`, `ambiente.json`, a publishable table in
      both languages and the raw output of every repetition
- [x] `scripts/arquivar-medicao.sh` and `scripts/compor-medicao.py`: they require
      `release` checked against Meson's real configuration (not the directory
      name), refuse a single-run campaign, and publish the **spread across runs**
      of every metric
- [x] `trilha/README.en.md` as an index that does not announce topics that do
      not exist
- [ ] `docs/regras-de-decisao.md` with a data schema and a checker that verifies
      each row against an existing `metadata.json` — **it only makes sense from
      the first topic that compares**, and 08.01 does not compare
- [ ] pending decision: **GoogleTest**. The track's origin document planned GTest
      1.17.0 pinned by wrap; the absorbed foundation tests with sanity programs
      and `static_assert`, with no dependency. The L1 tests parameterized by
      `spec.hpp` (section 34) are the first case where GTest would pay its own
      way

## Stage 4 onwards — the track

The nine modules, the order-book scenario and the capstone are described in
[`docs/origem/setup-lowlatency-structures-academy.md`](docs/origem/setup-lowlatency-structures-academy.md),
sections 4 to 6, and enter here once reconciled with the standard.

## Navigation

- [Repository README](README.en.md)
- [Documentation and standard](docs/README.en.md)

> [🇧🇷 Português](ROADMAP.md) | 🇺🇸 English
