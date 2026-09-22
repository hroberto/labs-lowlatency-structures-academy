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
- [x] [standard](docs/padrao-do-projeto.en.md) — it arrived with 33 sections, 6
      parts and today has 36 and 7, with the 14 divergences from its own origin
      recorded auditably
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
- [x] the environment-variable prefix settled: **`HARNESS_SAMPLES`,
      `HARNESS_ROUNDS`, `HARNESS_TAIL_SAMPLES`**. The prefix names the
      **instrument**, not the academy, because `lib/` is a candidate for its own
      repository consumed by the three projects in the family. And the suffix
      left Portuguese: an environment variable is a program interface, not prose
- [x] the language boundary written into the standard (section 3): identifiers,
      program output, flags, environment variables and emitted data schemas in
      English; comments, scripts and script-emitted records in Portuguese. The
      absence of that boundary produced measured drift — 35 Portuguese messages,
      `custo_do_relogio()` and `--braco-de-controle` in the repository's first
      program, all corrected
- [x] the "commit messages in English" clause, inherited from the origin
      document and contradicted by the entire history of both projects, now says
      **Portuguese**, with the reason recorded: the commit subject is the
      finding, and a finding is prose
- [x] `pre-commit.sh` — syntax, the seven checkers plus their seven self-tests, a
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
- [x] **the CI ↔ reference-machine contract, settled by measurement.** CI ran
      `ubuntu-24.04` with GCC 14 and Clang 18 and failed on its first run:
      **Clang 18.1.3 does not compile** `<expected>` nor
      `hardware_destructive_interference_size`, while the standard claimed
      "Clang 18+". What caught it was `check-env.sh`, because it probes features
      instead of trusting versions. The image is now `ubuntu-26.04`, which ships
      **the same GCC 15.2.0 and Clang 21.1.8 as the reference machine**, and the
      matrix became two jobs: `maquina` and `piso` (GCC 14 + Clang 20). A floor
      that compiles nothing is a promise, not a floor. Portability still belongs
      to CI and measurement to the machine — no published number comes from
      there
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

- [x] `governor=powersave`: **it stays as it is, and the decision is by
      measurement.** The first campaign suggested a warm-up (r0 and r1 ~26% more
      expensive); the second refuted it — the outlier moved to r3 and the
      percentiles came out identical across all five runs. What varies is the
      **maximum**, depending on whether that run caught an interrupt. Pinning
      `performance` would require root and break the promise that nothing
      didactic needs privilege — and whoever reproduces will also be on
      `powersave`. The spread across runs stays published, and it measures the
      state of the machine during the campaign
- [x] `perf_event_paranoid` made **persistent**:
      `/etc/sysctl.d/80-perf-event-paranoid.conf` pins the value 2, with the
      reason for not being 0 written in the file itself

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
- [x] `docs/08-medicao/01-harness/`: it builds, runs, and archives a campaign
      of 5 runs, versioning `metadata.json` — with the full per-run series —,
      the publishable table in both languages and `ambiente.md`. The raw
      per-repetition output and `ambiente.json` stay local: keeping the same data
      twice does not increase provenance, it increases the chance they diverge
- [x] `scripts/arquivar-medicao.sh` and `scripts/compor-medicao.py`: they require
      `release` checked against Meson's real configuration (not the directory
      name), refuse a single-run campaign, and publish the **spread across runs**
      of every metric
- [x] `docs/README.en.md` as an index that does not announce topics that do
      not exist
- [ ] `docs/regras-de-decisao.md` with a data schema and a checker that verifies
      each row against an existing `metadata.json` — **it only makes sense from
      the first topic that compares**, and 08.01 does not compare
- [x] **decided: GoogleTest wherever it is needed, and TAP where it does not
      reach.** The suite has 22 tests hiding more than a hundred assertions —
      `tail-sanity` is one test with eleven cases, and a regression in one of
      them reports "1 of 22". The deficit did not come from the absence of GTest:
      it came from `protocol: 'exitcode'`, Meson's default, which also accepts
      `tap` and `gtest`. So: **GoogleTest** (`protocol: 'gtest'`) for C++ tests
      with several cases, with Portuguese case names because they are prose;
      **TAP** (`protocol: 'tap'`) for the shell and Python tests, which GTest
      would never cover — `l2_run.sh`, the script self-tests and the checkers'
      own self-tests
- [ ] carry the decision out: `subprojects/gtest.wrap` pinned by hash (it is in
      WrapDB), converting `tail_sanity` and `contract_sanity`, and TAP output in
      the shell and Python tests. A framework with no user is a dependency with
      no payer, so the wrap lands in the same commit as the first test that uses it
- [ ] port `verificar-medicao.py` from the DPDK Academy — the **13th checker**,
      and today's only coupling with no gate: a number published in a README
      against the campaign it cites. Re-running a campaign changes the numbers
      and nothing goes red
- [ ] `docs/plano-estudo.md` — **the study order**, which is a different document
      from this file's build order. It is cited in six files and does not exist;
      it can be written now, because the module indexes exist

## Stage 3.5 — Tooling debt, declared

These four were **promised by the material and tracked nowhere** — the standard
and the READMEs cited them as known work, and they were items of no stage. A debt
that exists only in prose is not a debt: it is forgetting with a date on it.

- [x] `verificar-autodescricao.py` — **rewritten, not ported**. The original's
      838 lines carry 34 coupling points with the origin project's structure;
      what gets ported is the idea, not the file. It checks five censuses against
      the disk — the standard, the track, the checkers, the suite and the state
      label of section 5 — and it understands spelled-out numerals, because the
      standard writes "the seven have a self-test", not "the 7".
      **On its first run it reported eleven problems; nine were real**: the census
      said 31 planned topics where the disk has 23, six checkers with self-tests
      where there are seven, and a suite of 20 tests where Meson registers 22.
      Two were its own, and became the *change-record* rule
- [ ] `verificar-promessa.py` (18 coupling points) — every cited program exists
      in the tree and enters the build
- [ ] the **chart generator**, ported from `ferramental/graficos/` of the DPDK
      Academy. Section 31 of the standard cites it as missing; no item tracked it.
      High-percentile dispersion is read in a chart, and PT/EN parity applies to
      images — the upstream project keeps `.svg` and `.en.svg` in pairs
- [ ] decide whether a document **for the reader** on how to read the numbers
      exists, distinct from the standard, which is for the author. The
      `docs/00-visao-geral/` of the origin document left with the merge of the two
      trees and left no declared replacement

## Stages 4 onwards — the track

The ten modules have been reconciled with the standard and have an index under
[`docs/`](docs/README.en.md), with a question, a failure question and the `std` →
`custom` pair. **23 planned topics, 1 written.**

The done criterion is the same in all of them: every topic with `std/`,
`custom/`, `spec.hpp`, a parameterized L1, an archived campaign and a **decision
rule**.

### Stage 4 — Memory and layout · modules [01](docs/01-memoria/README.en.md) and [02](docs/02-layout/README.en.md)

- [ ] 6 topics. This is the stage where the `std` → `custom` pair really debuts,
      and with it `spec.hpp`, the invariant `traits` and the parameterized L1 —
      all three are design decisions that only settle with code in front of them
- [ ] it is also where `docs/regras-de-decisao.md` is born, with the first real
      rule

### Stage 5 — Containers and order book · modules [03](docs/03-conteineres/README.en.md) and [04](docs/04-livro-de-ofertas/README.en.md)

- [ ] 6 topics. Module 04 is where `spec.hpp` is put to the test: `std::map` and
      a flat array by tick do **not** fulfil the same law, and the divergence is
      the topic's subject
- [ ] pending an editorial decision: is the order book a scenario or the
      protagonist? §1.2 of the origin document says scenario, and the track gives
      it a whole module plus the capstone (divergence 12 in the standard)

### Stage 6 — Queues and time · modules [05](docs/05-filas-e-concorrencia/README.en.md) and [06](docs/06-tempo-e-erros/README.en.md)

- [ ] 6 topics. A clean TSan is a criterion, and the `sanitize-thread` profile
      exists for that — but what TSan does **not** report is no proof of
      correctness, and the argument comes from the standard cited by clause
- [ ] a negative test on every queue invariant

### Stage 7 — Parsing and measurement · modules [07](docs/07-parsing/README.en.md) and [08.02](docs/08-medicao/README.en.md)

- [ ] 3 topics. Includes the uncomfortable result that gives the rest its credit:
      the cases where `string_view` with `from_chars` is already the answer
- [ ] 08.02 depends on the PMU, which the reference machine already has at
      `perf_event_paranoid=2`

### Stage 8 — Capstone · module [09](docs/09-capstone/README.en.md)

- [ ] the order-book engine in two versions, with a reproducible synthetic feed
- [ ] the table that **attributes** the end-to-end difference to each topic —
      without it the capstone is a demo, not a closing

### Stage 9 — Generated code · module [10](docs/10-codigo-gerado/README.en.md)

- [ ] cross-cutting, and fed by the others: wherever `custom/` wins by code
      generation rather than by layout, the finding becomes a topic here

### Stage 10 — Extracting `lib/`

- [ ] `lib/measurement` and `lib/contract` become their own repository, consumed
      by this project, by the DPDK Academy and by the Messaging Academy
- [ ] the boundary has to be a **C ABI**, and that is a constraint from now: the
      DPDK Academy is written in C, and a harness built on `<expected>` and
      templates is not consumable from there (standard, section 33)

## Navigation

- [Repository README](README.en.md)
- [Documentation and standard](docs/README.en.md)

> [🇧🇷 Português](ROADMAP.md) | 🇺🇸 English
