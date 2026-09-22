> [🇧🇷 Português](padrao-do-projeto.md) | 🇺🇸 English

# Project standard — Low-Latency Structures Academy

This document defines the mandatory rules for **method, measurement, contract,
architecture, documentation and references** of the Low-Latency Structures
Academy.

> **This standard was absorbed, and the absorption is on the record.** It was
> born in `labs_cpp_performance_tuning_academy` — standard written, foundation
> executable, track not started, zero commits — and now governs this repository,
> which had the track and no foundation. What the absorption added is in
> sections [34](#34-the-std-and-custom-pair-as-the-unit-of-work),
> [35](#35-decision-rule) and [36](#36-the-generated-code-axis); what it
> corrected is in the second divergence table.

## What this document replaces

It is the normalized version of [`origem/setup-cpp-performance-academy.md`](origem/setup-cpp-performance-academy.md),
**preserved intact** so that the divergences between the original intent and the
standard in force remain auditable. The corrections applied are listed in the
last part, under [Divergences from the origin document](#divergences-from-the-origin-document).

> The origin document exists in PT-BR only. That is deliberate: it is a
> preserved historical artefact, not living documentation, and translating it
> would make it something other than what was originally written.

## The upstream projects

| Project | What this project takes from it |
|---|---|
| [DPDK-ACADEMY](referencias.en.md#dpdk-academy) | high-throughput architecture, already-measured mechanism costs, the measurement ruler, the quality tooling |
| [EX442-LABS](referencias.en.md#ex442-labs) | **only in the final stage**: the operating-system tuning surface, in the integration plan with low-level optimization (section 30) |
| C++ Performance Academy | **absorbed**: this standard, `lib/measurement/`, `lib/contract/`, the checkers — and the `language ↔ compiler ↔ generated code` axis, which became section 36 |
| this project | **where the standard library stops**: the `std` and `custom` pair measured under one contract, and the decision rule that comes out of it |

The third is no longer upstream in the sense the other two are: it does not
exist apart from this repository. It is listed because the standard you are
reading is its own, and because section 30 legislated over this repository
before this repository had a foundation.

---

# Part I — Language and form

## 1. Official languages

All academic documentation exists in **Brazilian Portuguese (PT-BR)** and in
**English (EN)**. Portuguese is the primary language in which the material is
developed.

The English version preserves technical depth, examples, tables, references,
conclusions, limitations and experimental results. **It is not an abridged
version** — it represents the same technical content.

## 2. File convention

A language pair with **the same stem**:

```text
README.md        Portuguese — Brazil
README.en.md     English
```

The identical-stem rule is not aesthetic: it is what makes it possible to verify
automatically that every document has a pair. A pair `referencias.md` /
`references.en.md` — as the origin version specified — breaks any pairing
verifier, because the two files share no prefix. The catalogue is therefore
[`referencias.md`](referencias.md) / [`referencias.en.md`](referencias.en.md).

In each topic:

```text
docs/
└── 04-livro-de-ofertas/
    └── 02-array-plano-com-bitmap/
        ├── README.md
        ├── README.en.md
        ├── spec.hpp            the topic's law: interface and traits of both arms
        ├── std/                the baseline the language ships
        ├── custom/             the complement, under the same contract
        ├── bench/
        │   └── medicoes/       archived output: metadata, CSV, table
        ├── tests/
        └── meson.build
```

This standard's origin document gave the example as `src/` and `resultados/`,
which presuppose a topic with **one** implementation. Here a topic has **two**,
and the structure says so: `std/` and `custom/` side by side, not the
alternative in an appendix. Why the name is `spec.hpp` and not `contract.hpp` is
in section 34.

## 3. Language of identifiers

**Documentation in Portuguese; code in English.** The same rule as the sibling
project.

```cpp
class MemoryPool;
class EventBuffer;

void process_event();
void measure_latency();
```

To avoid:

```cpp
class PoolMemoria;

void processar_evento();
```

This applies to classes, functions, variables, namespaces, tests, benchmark
names, log messages and CI identifiers. The goal is to keep the code compatible
with the vocabulary of the literature, the tooling and the international C++
community.

### Where the boundary runs, and why it had to be stated

The rule above did not say where **English** stops, and the omission produced
measured drift: this repository's first measurement program was born with
`custo_do_relogio()`, 35 messages in Portuguese and the flag
`--braco-de-controle`. None of that contradicts the letter of the rule; it
contradicts what the rule meant.

| Layer | Language | Examples |
|---|---|---|
| C++ identifiers | English | `clock_read_cost`, `tail_collector`, `sample_count` |
| **program output** | English | `"reading the clock"`, `"CONTRACT VIOLATED (precondition)"`, table headers |
| **command-line flags** | English | `--csv`, `--violate`, `--control-arm` |
| **environment variables** | English, prefixed by the INSTRUMENT | `HARNESS_SAMPLES`, `HARNESS_ROUNDS`, `HARNESS_TAIL_SAMPLES` |
| **data schema emitted by a program** | English | the CSV `arm,metric,value,unit` |
| code comments | Portuguese | they are prose, and prose is Portuguese |
| scripts under `scripts/` and `ferramental/` | Portuguese | they are automation for whoever studies and whoever publishes, and the file names are already Portuguese |
| records emitted by scripts | Portuguese | `ambiente.json`, `metadata.json` — a Portuguese structure, with English metric identifiers inside |
| commit messages | **Portuguese** | see below |

**The environment-variable prefix names the instrument, not the academy.**
`HARNESS_*` and not `CPP_ACADEMY_*`: the measurement ruler is a candidate for
its own repository, consumed by this project, by the DPDK Academy and by the
Messaging Academy (section 33). A shared ruler that reads `CPP_ACADEMY_*` in one
repository and `DPDK_ACADEMY_*` in another is not shared — it is copied.

**Commit messages are the exception, and it is deliberate.** This standard's
origin document listed "commit messages" among the English items, and the
history of this repository and of the DPDK Academy is entirely in Portuguese.
The rule follows the practice, not the other way round: the commit subject is
the **finding** — what broke, what it produced, and which check would have
caught it — and a finding is prose. Prose is Portuguese.

### Directories: the per-layer rule

The origin document contradicted itself here — the structure example used
`trilha/03-cache-memory/`, in English, while the identifier rule mandates
English for technical directories and the sibling project uses Portuguese
throughout the track.

The standard in force distinguishes a **curriculum directory** from a **code
directory**:

| Layer | Language | Examples |
|---|---|---|
| curriculum and documentation | Portuguese | `docs/01-memoria/`, `docs/`, `medicoes/` |
| code and build | English | `std/`, `custom/`, `bench/`, `tests/`, `lib/measurement/`, `lib/contract/` |

The reason is that the name of a curriculum directory is **text of the
material** — it appears in navigation, in the track and in document links —
whereas the name of a code directory is an **identifier**, subject to rule 3.

### The measurement archive is versioned, and has one name

The archived-output directory is **`bench/medicoes/`**, once. The inherited
`.gitignore` ignored `resultados/**` while the track's setup archived under
`bench/medicoes/`: two names for one thing, and the second one entered git
history because the first was the ignored one.

The name collision was the lesser defect. The greater one was the **rule**: the
inherited `.gitignore` declared that raw measurement output is not versioned,
because it "differs on every host and would inflate the history". That
contradicts section 8 of this standard and the DPDK Academy's practice, which
versions every campaign under `medicoes/historico/<YYYY-MM-DD>-<campaign>/`,
with the environment beside it and **one output per repetition** (`r0`, `r1`,
`r2`, `r3`).

The rule in force is the DPDK Academy's, and the reason is what this project
publishes:

> A p99.9 whose campaign is not in the tree is not auditable afterwards. The
> project's asset is the **provenance** of the number, and provenance that lives
> outside the repository is not provenance.

Therefore: the campaign history **is versioned**. What goes in is the auditable
record, not everything the campaign produces:

| File | Versioned | Why |
|---|---|---|
| `metadata.json` | **yes** | parameters, build, commit, embedded environment, and **the full per-run series** of every metric |
| `tabela.md` / `tabela.en.md` | **yes** | it is what goes into both of the topic's READMEs |
| `ambiente.md` | **yes** | the readable collection, the one checked by eye |
| `r*.csv` | no | an intermediate: measured that **every** metric already carries `por_execucao` in the metadata |
| `ambiente.json` | no | it duplicates the `.md`, and the metadata already embeds the whole environment |

Keeping the same data twice does not increase provenance; it increases the
chance of the two copies diverging. Whoever measured still has the raw output on
their machine; whoever audits has, in `metadata.json`, the same series it
contains.

## 4. Language navigation

Every document opens and closes with the navigation. In the Portuguese document:

```markdown
> 🇧🇷 Português | [🇺🇸 English](README.en.md)
```

In the English one:

```markdown
> [🇧🇷 Português](README.md) | 🇺🇸 English
```

This applies to the main README, to the track documents **and to the documents
in `docs/`** — the origin document only foresaw READMEs, which is why the very
document instituting bilingualism existed in Portuguese only and without
navigation.

## 5. State labelling

Every document and every index declares the state of what it describes, using
these labels and no euphemism:

| Label | Meaning |
|---|---|
| **not started** | no content exists |
| **written** | the document exists; no code |
| **environment measured, no code** | the environment was recorded; the experiment does not exist |
| **code exists, not measured** | it compiles and runs; no number published |
| **measured** | there is a published number with an environment and a program that produces it |

> **Why this is a norm and not good manners.** In the sibling project a
> "content not written" banner survived in a document with 171 measured lines,
> and the fix was to turn the rule into a verifier
> (`verificar-autodescricao.py`). Material that describes itself incorrectly
> lies about the one thing the reader cannot check without reading everything.

---

# Part II — Method

## 6. The methodological path

Each topic follows, whenever applicable:

```text
Problem → Mechanism → Trade-offs → Implementation → Measurement
       → Literature comparison → Analysis → Architecture decision
```

We do not want `use technique X because it is fast`. We want:

```text
problem → hypothesis → implementation → measurement → explanation
       → literature → conclusion
```

## 7. Classes of claim, and the mandatory notation

All technical content allows four categories to be distinguished. The origin
document defined them in prose, without notation — and without a syntactic mark
they are neither locatable nor verifiable. The standard in force requires a
**marker**:

```markdown
> **Measured.** On this machine, the SoA implementation showed 18% fewer cycles
> per element. [benchmark, environment, n runs]

> **Documented fact.** The processor transfers data between certain levels of
> the memory hierarchy in units of a cache line. [INTEL-ORM rev. X]

> **Inference.** The observed reduction in cache misses probably explains part
> of the throughput gain.

> **Open question.** The difference was observed, but it has not yet been
> possible to determine whether it is caused by the hardware prefetcher or by
> the compiler's code-generation strategy.
```

What each class requires:

| Class | Requires |
|---|---|
| **Measured** | benchmark, hardware, compiler, flags, dataset, number of runs, raw result |
| **Documented fact** | a Level A or B source, **with a version** |
| **Inference** | nothing beyond being presented as analysis, never as fact |
| **Open question** | nothing |

The project must be able to say **we do not know yet**. That is preferable to
inventing an explanation.

## 8. Rule for numbers

> **A number produced by the project needs a program that produces the number.**

Do not write `Memory access costs 100 ns`. Prefer:

```text
In the reference environment and in this experiment, the measured access showed
a median of X ns.
```

pointing to the benchmark, the script, the raw result, the hardware and the
methodology.

## 9. Rule for normative claims

Every normative claim identifies the corresponding authority.

Bad: `A cache line is always 64 bytes.`

Better: `On the microarchitecture used by the reference environment, the
vendor's documentation describes 64-byte cache lines. [INTEL-ORM rev. X]`

This avoids turning a characteristic of one piece of hardware into a universal
rule.

## 10. Declared budget

Optimization without a stopping criterion never ends. The sibling project solves
this by opening with the question **"how much time exists per unit of work"**,
with the number derived from a standard — the IEEE 802.3 frame plus the
interframe gap, at the link rate.

Here there is no link rate, so the budget is **declared**, and declared as a
compile-time constant:

```cpp
template <std::uint64_t EventsPerSecond>
struct workload_contract
{
    static_assert(EventsPerSecond > 0, "a null workload defines no budget");
    static constexpr double budget_ns = 1e9 / static_cast<double>(EventsPerSecond);
    static constexpr bool fits(double measured_ns) noexcept { return measured_ns <= budget_ns; }
};

using feed = workload_contract<5'000'000>;   // 200 ns per event
static_assert(feed::budget_ns == 200.0);
static_assert(!feed::fits(240.0));
```

Rule: **every module with an experiment declares the budget before measuring.**
A result is only "good" or "bad" against a budget; without one, it is merely
different.

## 11. Symmetric comparison protocol

This section exists because of a finding from the sibling project, and it is the
most important rule of Part II.

In the comparison between the DPDK version and the C++23 version of the same
problem, the initial table was also measuring **the difference in warm-up
between the two arms**, because only one of them warmed up:

> A comparison that does not control the method measures the method, not the
> object.

And the published result — pure C++23 being 1.6 to 2.2 times faster — comes with
the warning that **the test removed everything DPDK charges for**: with no NIC
there is no DMA, with no cross-core exchange the ring's memory barriers buy
nothing, with a single core the per-lcore cache is pure indirection. *"It is
like weighing a seat belt in a parked car."*

This project's central thesis (section 14) has **exactly the same failure
mode**. A microbenchmark of a `constexpr` table in a loop, with the table hot in
L1 and perfectly predictable access, removes everything the conditional version
would have cost — and the converse holds too.

**This protocol is the publication gate of the `std` and `custom` pair**
(section 34). It is not extra care: it is the difference between measuring the
structure and measuring the harness. The failure mode here is the one from
there — a `custom/` that warms up and a `std/` that does not publish the warm-up
difference under the structure's name.

Therefore every comparison between two implementations satisfies:

1. **symmetric warm-up** in both arms, declared in the document;
2. **sampling floor**: both arms refuse to publish below the declared minimum
   number of operations;
3. **core frequency** printed alongside the result;
4. **a declaration of what the benchmark removed** from the scenario, and
   **which arm that favours**;
5. **an explicit separation** between what the measurement legitimately shows
   and what it appears to show (section 27);
6. **a sweep, not a single point**, when the variable under test interacts with
   the memory hierarchy — see section 14.

## 12. Negative results

The project publishes experiments where the optimization did not help or made
performance worse:

```text
manual prefetch → +3.8% cycles/op → optimization rejected
```

That result has academic value: it shows the technique is not universal, and
depends on the mechanism and the workload.

> **A rejected optimization is knowledge too.** Negative results get their own
> index, so they can be found instead of being buried in the module that
> produced them.

---

# Part III — Contracts and code

## 13. The three contract gates

This is the project's central implementation directive: **each attribute
explicitly declares its acceptance terms**, and redundant validation disappears
from the optimized code.

The initial formulation asked for a per-attribute contract whose acceptance
terms are checked **only in DEBUG mode**. The mechanism for that intent is
[`assert`](referencias.en.md#cpp-assert), switched off by defining `NDEBUG` —
not `static_assert`, which the origin document named and which has different
semantics:

- `static_assert` is evaluated at compile time, **unconditionally** — there is
  no release build in which it is switched off;
- `static_assert` **cannot see a runtime value**: there is no way to use it to
  validate a number coming from an environment variable, a configuration file or
  the command line.

Both are necessary, for different things. And `assert` alone is not enough
either, for a measured reason: with `NDEBUG` it expands to `((void) 0)`, and
neither the check **nor the information** survives — the redundant downstream
conditional returns to the release binary, which is precisely what the second
directive aims to eliminate. The Gate R mechanism is therefore **`assert` plus
`[[assume]]`**: the check in debug, the information in release. The measured
table is in [CPP-ASSERT](referencias.en.md#cpp-assert).

Adding up the three distinct situations, each with its own mechanism:

| Gate | What it validates | Mechanism | Cost in release |
|---|---|---|---|
| **T** — compile time | acceptance term over a constant value, type property, relation between constants, layout, alignment | `static_assert`, concept, `requires` | zero, always active |
| **R** — trusted runtime | internal value whose violation is a **programmer bug** | `assert` in DEBUG; `[[assume]]` in release | zero |
| **E** — external boundary | value coming from an environment variable, a file, the CLI | `std::expected` — **always active** | paid, and it should be |

### Gate E is never compiled out

A malformed number in an environment variable, a configuration file or the
command line is **not a programmer bug**: it is environment data. If validation
of external input disappeared in release, the program would start trusting
content nobody controls — that is a correctness hole, not an optimization.

"Avoid unnecessary validation conditionals" applies to T and R. **For E the
conditional is necessary**, and the norm is: verification of external input
lives in `adapters/` and is never conditioned on `NDEBUG`.

### The acceptance term is written once

What unifies the three gates is a single `constexpr` predicate:

```cpp
template <std::integral T, T Lo, T Hi>
struct closed_range
{
    static_assert(Lo <= Hi, "inverted domain: Lo > Hi");
    static constexpr bool accepts(T v) noexcept { return v >= Lo && v <= Hi; }
};
```

consumed by all three gates: `static_assert(Domain::accepts(V))` when the value
is constant, `PERF_EXPECTS(Domain::accepts(v))` when it is internal,
`std::expected` when it comes from outside.

### The runtime mechanism, and why it is a macro

```cpp
#ifdef NDEBUG
#  define PERF_EXPECTS(expr) [[assume(expr)]]
#else
#  define PERF_EXPECTS(expr) ::perf::contract::check((expr), #expr, \
                                 std::source_location::current())
#endif
```

`[[assume]]` is **C++23** ([P1774](referencias.en.md#p1774)); Contracts (`pre`,
`post`, `contract_assert`) is **C++26** ([P2900](referencias.en.md#p2900)) and
**is not available in the reference toolchain** — measured; see the catalogue
entry. The macro is the migration seam: when Contracts exists, its body changes
and no call site is touched. That is why `.clang-tidy` disables
`cppcoreguidelines-macro-usage`, with the reason written in the file itself.

### The serious warning

**A false predicate in `[[assume]]` is undefined behaviour**, and the optimizer
is licensed to delete entire code paths. A wrong contract does not become an
assert that fires: it becomes a silently miscompiled release.

Normative consequence — the build matrix has **three** configurations, not two.
See section 16.

### The gain that closes the directive

If the domain is `constexpr`, **the test of the domain is a `static_assert`**:
zero runtime cost, and the violation breaks the build instead of breaking the
suite.

```cpp
static_assert(sample_count_domain::accepts(3));
static_assert(!sample_count_domain::accepts(2));
```

Already applied in [`lib/contract/measurement_domains.hpp`](../lib/contract/measurement_domains.hpp)
and in [`lib/measurement/statistics.hpp`](../lib/measurement/statistics.hpp).

### Which domains come first, and why

A deliberate scope decision: **the first concrete domains are those of the
measurement ruler itself** — sample count, rounds per sample, percentile,
alignment. They are conceptual, self-contained, and their acceptance terms
already existed in prose inside `statistics.hpp`, implemented as a bare number
in the middle of a call. Turning them into domains is the most honest example of
the directive: the acceptance term stops being a comment and becomes verified.

Operating-system tuning domains **do not belong to the foundation**. They enter
in the final stage, with the integration plan of section 30.

## 14. Compile time versus runtime: the thesis, not the rule

The project's second directive — raising logic to avoid a conditional or a loop
that could be replaced by a compile-time mechanism — **is not a style rule in
this project. It is the central thesis, and it is measured.**

> Where, and under which conditions, does moving work from runtime to compile
> time actually pay off?

As a style rule it would be dogma, and it would collide head-on with the
editorial policy (section 32): an `if` that the predictor gets right close to
100% of the time costs almost nothing, whereas the `constexpr` table replacing
it may cost a cache miss — trading a predicted branch for an access to `.rodata`
outside L1. Asserting the gain before measuring is precisely the `use technique
X because it is fast` that section 6 forbids.

### What is a structural gain and needs no measurement

- `if constexpr` for policy selection: it removes the branch and the dead code;
- `if consteval` ([P1938](referencias.en.md#p1938)) and
  `std::is_constant_evaluated()` for a dual-mode predicate;
- `<bit>`: `has_single_bit`, `bit_width`, `countl_zero`, `popcount` — alignment,
  page-size and rounding arithmetic with no loop and no conditional, with a
  dedicated instruction on most targets;
- non-type template parameters for **strong units** (`Bytes<N>`,
  `Pages<Size>`), which eliminate the "I passed KiB where pages were expected"
  class of error;
- `std::expected` at the boundary; `std::span` and `std::mdspan` for copy-free
  reading.

### What is a hypothesis and requires an experiment

- a `static constexpr std::array` table built by a `consteval` function, in
  place of an `if` chain;
- a perfect hash frozen at compile time to resolve a parameter name;
- loop unrolling and branch elimination through arithmetic;
- template specialization in place of dynamic dispatch.

> **`std::ranges` in place of a hand-written loop is a gain in readability and
> provenance, not in speed.** It is the same loop. Selling it as an optimization
> violates section 6.

### A sweep, not a single point

The sibling project measured the batching curve: it helps a lot up to 8,
marginally up to 32, and **regresses at 128** — and the shape is the same in
both arms, because it is cache behaviour, not library behaviour.

This predicts the behaviour of `constexpr` tables: **there is an optimum, and
going past it costs.** Therefore every table-versus-conditional experiment is a
**sweep over working-set size**, never a single point.

### The thesis metrics include the cost of compiling

Moving work to compile time **has a cost**, and it is real:

| Metric | Why |
|---|---|
| cycles/op, ns/op | the intended gain |
| cache misses, branch misses | the mechanism of the gain or the loss |
| **compilation time** | template instantiation is not free |
| **binary size** | the table goes into the binary |
| **`.rodata` size** | it is what competes with the data for cache |

A domain that compiles in 40 s and produces 200 KB of tables to save 2 ns on a
cold path is a **rejected optimization** — and the project can only say so if it
measures both ends.

## 15. Layered architecture

```text
lib/
├── measurement/   the measurement ruler: clock, statistics, wait hint
└── contract/      contract, domains, units, budget

<topic>/
├── spec.hpp       the law: shared interface and invariant traits (section 34)
├── std/           the baseline the language ships
├── custom/        the complement, under the same spec
├── bench/         both arms in the same harness, same load
└── tests/         L1 parameterized by the spec, L2 as the reader runs it

<capstone>/
├── domain/        pure computation: units, domains, invariants
│                  100% constexpr, ZERO I/O, tested by static_assert
├── ports/         interfaces: FeedReader, BookSink, ClockSource
├── adapters/      environment reading: files, variables, counters
├── application/   use cases: build the book, measure, report
└── cli/           presentation and evidence report
```

The `domain/ports/adapters/application/cli` layers belong to the **capstone**,
not to every topic. A topic comparing two structures has neither use case nor
adapter: it has two implementations, one spec and one benchmark. Imposing the
five layers on it would produce empty directories, which is false
self-description (section 5).

The separation is not fashion: **the domain is only `constexpr`-testable because
it has no I/O.** The architecture decision and the performance decision are the
same decision here. And the Gate E rule becomes verifiable by directory
inspection — the `std::expected` of environment validation appears in
`adapters/`, not scattered around.

## 16. Toolchain and the three-configuration matrix

| Item | Value |
|---|---|
| Language standard | **C++23** (`cpp_std=c++23`) |
| Reference compilers | GCC 15.2.0 and Clang 21.1.8 |
| Verified floor | GCC 14 and Clang 20 — exercised in CI on every push |
| **Refuted** floor | Clang 18.1.3 — measured, does not compile (see below) |
| Build | Meson ≥ 1.1 + Ninja |
| Architectures in scope | **x86-64 only** |
| System dependencies | **none beyond the compiler**, Meson and Ninja |

The choice of strict C++23 is what makes `[[assume]]` the contract mechanism
rather than Contracts — see section 13.

### The compiler minimum is measured, not announced

The standard said **"GCC 14+ and Clang 18+"**, inherited from the track's origin
document. The first CI run refuted the second half: the **Clang 18.1.3** of the
`ubuntu-24.04` image does not compile what this project uses — `<expected>` and
`hardware_destructive_interference_size` are missing, the latter depending in
libstdc++ on a macro Clang only started defining later.

What caught it was `check-env.sh`, and it caught it because it **probes the
feature instead of trusting the version number**:

```cpp
static_assert(std::hardware_destructive_interference_size >= 32);
std::expected<int, const char*> f(bool b);
```

The probe is the authority, not this table. If a future compiler starts failing,
CI goes red in the diagnosis step, before any compilation — which is where the
information is cheap.

The floor is now **exercised**: the CI matrix runs one job with the reference
machine's compilers and another with the declared minimum. A minimum version
that compiles nothing is a promise, not a floor.

**arm64 left the scope, and that departure corrects a promise with no checker.**
The track's origin document said "arm64 compiles, no numbers published". There
is no arm64 machine in this project, and no suite compiles for arm64 — an
architecture "in scope" that nothing exercises is false self-description
(section 5). The per-architecture branches for `rdtsc` and `_mm_pause` remain in
`lib/measurement/`, because they are portable-code correctness, not a platform
promise. arm64 returns to scope the day a machine or a CI runner compiles it.

### The sanitizers are two profiles, not one

The track's origin document planned "one `sanitize` profile with ASan, UBSan and
TSan". **ASan and TSan do not coexist in one binary** — the compiler itself
refuses. They are two profiles, and the distinction has a methodological
consequence:

| Profile | Tool | Where it is mandatory |
|---|---|---|
| `sanitize-address` | ASan + UBSan | every topic with its own allocator, arena or pool |
| `sanitize-thread` | TSan | every topic in the queues-and-concurrency module |

TSan does not understand the relaxed ordering of a hand-written lock-free
algorithm: it reports what it observed, and what it does **not** report is no
proof of correctness. The suite uses TSan as a filter, and the correctness
argument still comes from the standard cited by clause (section 9), not from the
absence of a warning.

### Why three configurations

Because `[[assume]]` with a false predicate is undefined behaviour, and neither
of the two usual configurations detects it:

| Configuration | Contracts | Optimization | What it proves |
|---|---|---|---|
| `debug` | checked | `-O0`/`-Og` | the contracts hold in unoptimized code |
| `release` | `[[assume]]` | `-O2`/`-O3` | it is what gets published and measured |
| **`release-checked`** | **checked** | `-O2`/`-O3` | **the contracts hold in optimized code** |

Without the third, the project assumes without ever verifying what it assumed.
The suite runs on all three.

---

# Part IV — Sources

## 17. Source hierarchy

| Level | What it is | Examples |
|---|---|---|
| **A** | normative or primary | ISO C++, WG21, Intel SDM/ORM, AMD, Arm ARM, Linux Kernel Documentation, GCC, LLVM, Clang, POSIX, IEEE, RFC |
| **B** | academic literature | ACM, IEEE, USENIX, SOSP, OSDI, ASPLOS, PLDI, PPoPP, ISCA, MICRO, ISMM, ISPASS |
| **C** | recognized technical literature | Agner Fog, Brendan Gregg, Paul McKenney, Ulrich Drepper, Anthony Williams, Herb Sutter, Maurice Herlihy, Scott Meyers, Chandler Carruth, Andrei Alexandrescu |
| **D** | supplementary material | talks, engineering blogs, vendor technical articles, reference implementations |

Level A takes precedence when it directly defines the behaviour under study.
Level D never replaces A when A exists.

Level B papers must carry author, title, venue, year and DOI.

## 18. Sources that do not support important technical claims

Avoid as a primary source: Wikipedia, Stack Overflow, social-media posts, blogs
without identifiable technical authorship, SEO articles, benchmarks without
methodology.

Such material helps to **locate** a subject; it is not a final authority.

### On AI-generated content

The origin document listed "AI-generated content" among the sources to avoid.
The rule as written was ambiguous in material that is produced with AI
assistance — and a norm the project itself violates is not a norm.

The formulation in force says what is actually meant:

> **Every technical claim traces to a Level A–D source or to a reproducible
> experiment in this repository. The authorship of the text is not a source** —
> neither human nor automatic. The output of a language model is not citable as
> an authority; model-assisted text whose claims trace to an identified source
> or to an experiment is acceptable, and is how this material is written.

## 19. Central catalogue

The catalogue is [`referencias.md`](referencias.md) / [`referencias.en.md`](referencias.en.md).

Identifiers are **stable**: they do not change if documents are reorganized.

## 20. Anchors

The anchor of each reference comes from the **entry title**. `### INTEL-ORM`
produces `#intel-orm` via github-slugger, and that is what `verificar-links.py`
checks:

```markdown
[INTEL-ORM](../../docs/referencias.en.md#intel-orm)
```

The origin document also required an explicit `<a id="intel-orm"></a>` tag. It
was **removed from the norm**: nothing verified the agreement between the tag
and the title, and two identifiers for the same section diverge silently. It was
the ported tooling itself that exposed this — the verifier only knows about
title anchors.

## 21. Per-module references

Each module ends with the list of sources that support it:

```markdown
## References

- [INTEL-ORM](../../docs/referencias.en.md#intel-orm)
- [AGNER-CPP](../../docs/referencias.en.md#agner-cpp)
- [LINUX-PERF](../../docs/referencias.en.md#linux-perf)
```

This makes it immediately clear what supports that chapter. The central
catalogue remains the complete source of metadata.

## 22. Versioned references

When a reference depends on a version, the document identifies it: GCC 15,
Clang 21, Linux 6.x, the optimization manual revision, the microarchitecture
generation.

Avoid `according to Intel's documentation`. Prefer `according to Intel 64 and
IA-32 Architectures Optimization Reference Manual, revision X`.

## 23. Stable links

Prefer official documentation, DOI, a permanent vendor page, a WG21 paper
number, an RFC, a standard identifier. Avoid search links, temporary URLs,
unknown mirrors, blogs that copy documentation.

## 24. Reference ↔ module mapping

| Module | Main references |
|---|---|
| Benchmarking and method | GOOGLE-BENCHMARK, KALIBERA-JONES-2013, LINUX-PERF, BRENDAN-GREGG-SP |
| Contracts and domains | CPP-STD, P1774, P2900, P1938, LINUX-DOC, CGROUP-V2 |
| Compile time versus runtime | CPP-STD, GCC-DOC, CLANG-DOC, LLVM-MCA, UICA, AGNER-INST |
| Cache and memory | INTEL-ORM, AMD-OPT, ARM-NEOVERSE, DREPPER-MEM, AGNER-CPP, DPDK-ACADEMY |
| Data layout | INTEL-ORM, AMD-OPT, AGNER-CPP |
| Allocation | CPP-STD, AGNER-CPP, JEMALLOC, TCMALLOC, MIMALLOC |
| Zero-copy | CPP-STD, POSIX, LINUX-DOC |
| Branch prediction and prefetch | INTEL-ORM, AMD-OPT, AGNER-UARCH |
| Compiler optimization | GCC-DOC, LLVM-DOC, CLANG-DOC, LLVM-MCA, AGNER-CPP |
| SIMD | INTEL-SDM, INTEL-ORM, AMD-OPT, ARM-ARM |
| Atomics and memory ordering | CPP-STD, WG21, BOEHM-ADVE-2008, MCKENNEY-PERFBOOK, WILLIAMS-CCIA |
| Lock-free | CPP-STD, HERLIHY-WING-1990, MICHAEL-SCOTT-1996, MCKENNEY-PERFBOOK |
| NUMA | LINUX-DOC, INTEL-ORM, AMD-OPT, DREPPER-MEM, DPDK-ACADEMY |
| Profiling and counters | LINUX-PERF, PERF-EVENT-OPEN, YASIN-2014, PMU-TOOLS, BRENDAN-GREGG-SP |
| Microarchitecture | INTEL-ORM, AMD-OPT, AGNER-UARCH, ARM-NEOVERSE |
| System tuning — **final stage** | LINUX-DOC, CGROUP-V2, EX442-LABS, BRENDAN-GREGG-SP |

---

# Part V — Document structure

## 25. Standard chapter structure

The origin document listed 18 sections as "preferred", which left it ambiguous
what was required — and mandated "Hardware counters" even in a conceptual
chapter, where it is noise. The standard in force separates them:

| Section | State |
|---|---|
| Title, Objectives, Problem | **mandatory** |
| Mental model, Fundamentals, How it works | **mandatory** |
| Baseline implementation | mandatory **if there is code** |
| Experiment, Execution environment, Results | mandatory **if there is measurement** |
| Hardware counters | mandatory **if there is microarchitecture measurement** and the PMU is available |
| **What this measurement does not show** | mandatory **if there is measurement** (section 27) |
| **Decision rule** | mandatory **if the topic compares** (section 35) |
| **When it goes wrong** | mandatory **if there is an invariant**, and answered with an experiment |
| Analysis | mandatory **if there is measurement** |
| Literature comparison | mandatory **if there is a relevant experiment** (section 26) |
| Trade-offs, When to use, When not to use | **mandatory** |
| Limitations | **mandatory** |
| Exercises | **mandatory** |
| References, Navigation | **mandatory** |

*When it goes wrong* is not the same section as *Limitations*: a limitation is
what the numbers do not license concluding; *when it goes wrong* is what the
system does off the happy path — the arena exhausted mid-batch, the queue full,
the price outside the array's range, the truncated message. The rule inherited
from the DPDK Academy is that it is answered with a program, never in prose, and
that **the invariant has a negative test**: the version that violates it exists
and the suite requires it to fail. A check that has never failed is
indistinguishable from a check that never fires.

The Portuguese version has exactly the same conceptual structure: Objetivos,
Problema, Modelo mental, Fundamentos, Como funciona, Implementação baseline,
Experimento, Ambiente de execução, Resultados, Hardware counters, **O que esta
medição não mostra**, Análise, Confronto com a literatura, Trade-offs, Quando
utilizar, Quando não utilizar, Limitações, Exercícios, Referências, Navegação.

## 26. Literature comparison

The question is: **is the measured behaviour compatible with what the technical
sources describe?**

Three possible outcomes:

```text
the measurement confirms the literature
the measurement diverges from it       → investigate the context
the literature does not predict it     → record as an experimental observation
```

Divergence is not hidden. It may be the most interesting part of the study.

And divergence is not assumed to mean the literature is wrong. It may come from
hardware, microarchitecture, compiler, flags, OS, kernel, SMT, turbo, governor,
NUMA, dataset, an incorrect benchmark or an uncontrolled effect. The process is:

```text
Literature → Hypothesis → Experiment → Result → Critical analysis
```

## 27. What this measurement does not show

A mandatory section in every document with a measurement. It answers:

1. **what the benchmark removed** from the real scenario;
2. **which arm** of the comparison that removal favours;
3. **which conclusion the reader would draw** if they stopped at the table;
4. **what would be needed** to support the broader conclusion.

The model is the sibling project's alternative document, which publishes a table
favourable to pure C++23 and then writes *"if you stop reading here, you will
draw the wrong conclusion"*, explaining that the test removed DMA, cross-core
exchange and memory pressure — everything the measured abstraction charges for.

> A table without this section is a number without a scope. The project treats
> that as a document defect, not as a matter of style.

## 28. Reproducibility

Every published result makes it possible to answer: which CPU, which kernel,
which compiler, which version, which flags, how many runs, which affinity, which
NUMA node, was turbo on, was SMT on, which governor, which dataset size, which
code produced the number.

Added for this project: **which of the three build configurations** of section
16, and **was the PMU available?**

And what the reference machine forces us to add:

| Record | Why this, and not only what is usually recorded |
|---|---|
| **the core's CCD, and its SMT sibling** | the machine has two L3 domains; a pair of cores on the same CCD and a pair across CCDs give different results for the **same** structure. "Core 3" does not identify the experiment; "core 3, CCD 0, SMT sibling 15" does |
| **histogram precision and range** | a published p99.9 without the instrument's precision is a number with no error bar |
| **between-run variation** | the campaign runs the whole experiment more than once and publishes the variation. A percentile table from a single run does not separate the structure's tail from the machine's |

The last one is inherited directly from a DPDK Academy defect: the validity
criterion was published and its calibration was nowhere. Here the calibration is
a campaign deliverable, not a footnote.

An unknown variable is documented as unknown.

## 29. Reference environment

The record is **generated** by `scripts/ambiente.sh` instead of described in
prose by the author. State of that script in this repository: **code exists, not
measured** — it collects in three modes (`texto`, `--markdown`, `--json`), and
`--json` is what feeds each measurement's `metadata.json`. The reason is in the origin script: a prose
description diverges between files without anyone noticing, ages silently when
the kernel or the compiler changes, and does not tell whoever reproduces it what
to compare.

It records at least: CPU model, microarchitecture when identifiable, cores,
threads, SMT, NUMA topology, cache topology **with the L3 domains and each
core's CCD**, RAM, kernel, governor, turbo, compiler and version, Meson, Ninja,
perf, `perf_event_paranoid`, and whether the TSC is invariant (`constant_tsc`,
`nonstop_tsc`). When relevant: mitigations, hugepages, CPU isolation, frequency,
thermal state.

**This project's reference machine** is an AMD Ryzen 9 9900X — 12 cores, 24
threads, SMT on, **two L3 domains**, one NUMA node, invariant TSC. The two
consequences that run through the track: the chosen core pair is a variable of
the experiment, not an execution detail (section 28); and the failure question
"what happens when the TSC is not invariant" is **not reproducible here** — it
is recorded as an open question (section 7), not as a promised experiment.

> **Inherited lesson, to be preserved in the port:** the origin script does
> **not** parse `lscpu` text, because its output is localized — the first
> version silently returned empty fields on a Portuguese-language machine, which
> is ironic in a script whose reason to exist is reproducibility. The forms used
> are `lscpu -p=` and sysfs, which are language-independent. **Do not rewrite
> that part.**

---

# Part VI — Governance

## 30. Division of labour with the sibling projects

The DPDK Academy already covers, **with measurement**, the mechanism
fundamentals: budget per unit of work, the user/kernel boundary, address
translation and the page walk, the TLB, hugepages, cache and locality, false
sharing, NUMA and first touch, SMT, polling versus sleeping with a comparison of
primitives, DMA and descriptors, IOMMU and IOTLB, the bus budget, and the
metrics vocabulary anchored in standards.

**Norm: this project cites and extends; it does not re-teach.**

| Subject | Where it lives | What this project adds |
|---|---|---|
| cost of a cache miss, TLB, NUMA, syscall | DPDK-ACADEMY | how the C++ object model and layout cause or avoid it |
| cost of synchronization | DPDK-ACADEMY | `std::atomic`, memory ordering, linearizable correctness |
| OS tuning surface | EX442-LABS | **nothing, until the final stage** — see below |
| code generation, `constexpr`, templates | here, section 36 | came from the absorbed project, and became a cross-cutting module |
| **where the standard library's structures stop** | here | **it is this project's own object** — sections 34 and 35 |

**This section legislated over this repository before it existed.** It was
written in the absorbed project, and it assigns cache, TLB, NUMA and false
sharing to the DPDK Academy — which are the subject of the memory and layout
modules here. The rule that settles it has not changed, and it is the one that
opens the section: **cites and extends; does not re-teach.** In practice, a
module here does not re-present the cost of a cache miss: it cites the number
already measured in the DPDK Academy and measures what the C++ structure does
with that cost. When the cited number does not serve — another machine, another
compiler, another load — the module re-measures and says why it did not.

### When EX442 enters, and why not earlier

The operating-system tuning axis — `sysctl`, `sysfs`, cgroups, hugepages,
scheduler, writeback — **does not appear in the foundation or in the conceptual
modules**. It enters in a stage of its own, at the end, as an **integration
plan** between what this project teaches and the low-level optimization that
EX442 exercises.

The reason is pedagogical, not one of scope: both axes are large, and mixing
them makes the reader study kernel tuning when they should be studying the
object model, layout and code generation. A `cpu.weight` domain in a module
about contracts teaches cgroups as a stowaway, and the stowaway costs the
attention the concept needed.

Practical consequence, already applied: the contract library models the domains
of **this project's measurement ruler**, not those of the operating system. See
section 13.

### "DPDK versus C++" is a false opposition

Recorded here because it is the project's identity, and the sibling document
already established it: DPDK programs **are** written in C and C++; the DPDK API
is C, callable from C++23 with no intermediary. What is being compared are two
architectures of memory and flow management, and the real difference is **who
guarantees** that there is no allocation on the hot path: in DPDK, the
programmer; in C++23, the type system and RAII.

This project is **not** the "anti-DPDK". It is the same discipline with the
abstraction boundary moved.

## 31. Automated verification

Policy without a gate is worthless. The verifiers in
[`ferramental/qualidade/`](../ferramental/qualidade/) run in the suite and in CI:

| Verifier | Rule |
|---|---|
| `verificar-links.py` | a relative link points to an existing file and anchor |
| `verificar-ancoras.py` | a line anchor points to the right piece of code |
| `verificar-retratacoes.py` | a value declared retracted does not survive outside the block retracting it |
| `verificar-aritmetica.py` | a percentage the text makes checkable adds up |
| `verificar-suposicao.py` | the Gate R precondition reaches the optimizer, checked in the assembly, with a control arm |
| `verificar-paridade.py` | the language pair exists, the navigation points at it, the structure matches, and no published number is missing from either language |

The first four come from the DPDK Academy, where **each was born from a measured
defect**; the fifth was born in the absorbed project, from a defect of its own —
Clang was discarding the assumption silently. The sixth was born here, and it was
born from this very section: it recorded that the identical-stem rule existed to
be verifiable and was not verified. All have self-tests. The reason for porting instead of writing is
in the header of `verificar-retratacoes.py`: in the defect that originated it,
whoever wrote the retraction knew the rule, had just stated it, and still left
the retracted value circulating on other pages. **Manual correction does not
scale.**

**Twelve are missing, and the count is meant to be read.** The DPDK Academy has
16 checkers in `ferramental/qualidade/`, inside 23 quality programs and 6,827
lines. Four of them were ported: twelve are missing. The two that matter most,
with their coupling already measured, are `verificar-promessa.py` (every cited
program exists and enters the build, 18 coupling points) and
`verificar-autodescricao.py` (what the material claims about itself matches the
disk, 838 lines and 34 points) — the latter is what sustains the state labelling
of section 5, and until it exists section 5 is an author's promise, not a gate.
Also missing is the **chart generator**, because high-percentile dispersion is
read in a chart and the PT/EN parity applies to images. State: **not ported** —
recorded in the [ROADMAP.en.md](../ROADMAP.en.md).

The porting order follows real need, not bulk: porting 6,800 lines before there
is content to check is a promise with no code, which is what this standard
forbids.

## 32. Editorial policy

> **Mechanism before optimization.**
>
> **Measurement before conclusion.**
>
> **Correctness before performance.**
>
> **A number without an experiment is not a result.**
>
> **A normative claim without an identified authority is not a reference.**
>
> **A benchmark without a documented environment is not reproducible.**
>
> **A comparison without a symmetric method measures the method.**
>
> **A measurement without a declared scope is a number without meaning.**
>
> **An observed difference with no known mechanism remains explicitly a
> hypothesis.**
>
> **A rejected optimization is knowledge too.**
>
> **An assumed and unverified contract is undefined behaviour, not
> optimization.**

## 33. Academic identity

The documentation is deep enough to connect:

```text
C++ source → language semantics → compiler → assembly
          → microarchitecture → cache/memory → operating system → measurement
```

and this project's chain adds one link, which is its own:

```text
what the standard library ships → the measured point where it stops
                                → the complement, under the same contract
                                → the decision rule
```

The goal is not to teach **how to write fast C++**. It is to teach:

> **how to investigate scientifically why a given piece of C++ software exhibits
> a given performance behaviour** — and, with that, **when** to replace the
> mechanism the language gives you, not only how.

This philosophy stays consistent across the DPDK Academy, this project and the
Messaging Academy, forming a coherent family of studies on **High Performance
Systems Engineering**. Comparability across the three depends on a shared
measurement ruler, which is why `lib/` is treated as a candidate for its own
repository from the start: the boundary has to be a **C ABI**, or the extraction
never happens — the DPDK Academy is written in C, and a harness built on
`<expected>` and templates is not consumable from there.

---

# Part VII — The std and custom pair

This part did not exist in the absorbed standard. It is what this repository has
and the origin project did not: the track, and its unit of work.

## 34. The std and custom pair as the unit of work

> A topic is not a structure. It is **a pair of structures under one law**,
> measured in the same harness, on the same machine, under the same contract.

The order is fixed and it is not a preference: **the language's offering first.**
No `custom/` is written before the corresponding `std/` has been measured and
published. A complement proposed against an unmeasured baseline is an opinion
with code attached.

Three outcomes are possible per topic, and **all three get published**:

| Outcome | What the reader learns |
|---|---|
| the standard library suffices | which facility to use, and why the hand-rolled alternative does not pay off |
| it suffices up to a volume or percentile | the decision rule: the point at which to switch (section 35) |
| it does not serve on the hot path | the mechanism that replaces it, and what that costs in exchange |

The second and third are the expected ones. **The first is what gives the other
two credit**, and so it has its own index alongside the negative results of
section 12: in several cases `string_view` with `from_chars` is already the
answer, and the hand-rolled decoder does not pay off.

### `spec.hpp` is the law, and `contract.hpp` is something else

Two words collided. In this standard, **contract** is the mechanism of the three
gates of section 13 — `PERF_EXPECTS`, `[[assume]]`, `lib/contract/`. In the
track's origin document, "contract" was the shared interface that `std/` and
`custom/` must fulfil. Two concepts, one name, in one repository.

The name of a topic's interface is **`spec.hpp`**. The one in `lib/contract/`
stays, because it is what `-Werror=assume` and `verificar-suposicao.py` already
name.

### A contract divergence is a finding, and it is declared in code

`std::map<Price, Level>` and a flat array indexed by tick do **not** fulfil the
same law: they differ in iteration order, reference stability and price domain.
That is not a defect of the topic, it is the topic's subject.

Rule: `spec.hpp` declares the invariants, and **each arm declares in `traits`
which of them it honours**. The L1 tests are parameterized by the spec and
consult the traits; an invariant an arm does not honour is a test **skipped with
a reason**, not a missing test. The divergence stays in code, verifiable,
instead of in prose, where it ages without anyone noticing.

> A spec both arms fulfil without reservation is usually a sign that the spec was
> written weak enough to fit both.

## 35. Decision rule

A mandatory section in every topic that compares, and **it is the project's
internal-reference artefact** — what one consults when designing a real system,
not when studying.

The rule answers one question: **beyond what point does the switch pay off?**
The point is a number — volume, cardinality, arrival rate, percentile — not an
adjective.

Every published rule declares at least:

| Field | Example |
|---|---|
| the starting structure and the replacement | `std::map<Price, Level>` → flat array by tick with a bitmap |
| the decision variable and the turning point | cardinality of active levels, above N |
| the percentile where the turn shows up | shows in p99.9 before it shows in p50 |
| the machine, the build configuration and the commit | environment generated, not described (section 29) |
| what the rule does **not** cover | the load, access pattern and range outside which it does not hold |

The consolidated table lives in `docs/regras-de-decisao.md`, and **each of its
rows points at an existing `metadata.json`**. The table is machine-verified,
like every other numeric claim under section 8: a decision rule without an
archived measurement is an opinion wearing the clothes of a reference, and it is
the most expensive defect this project could publish, because it is the one
someone would use to decide.

> A decision rule that does not say where it stops holding is not a rule: it is a
> slogan.

## 36. The generated-code axis

The absorbed project's own object, preserved here as a **cross-cutting module**
instead of diluted into the others. Its question is the one the structure
modules do not ask:

> **what does the compiler do with `custom/` that it does not do with `std/`?**

What this axis brings, and what it is not:

- it brings `constexpr`, templates, `[[assume]]` and the shift of work to
  compile time as a **measured hypothesis**, never as a style rule (section 14);
- it brings the sweep instead of the single point, because the variable under
  test interacts with the memory hierarchy;
- it brings the cost of compiling into the metrics;
- it is **not** a comparison between compilers, nor a catalogue of flags: the
  pair is still `std` and `custom`, and the compiler is the instrument, not the
  object.

The checker that sustains this axis already exists: `verificar-suposicao.py`
compiles a probe and checks in the assembly, with a control arm, that the
precondition reaches the optimizer. It was born from a measured defect — Clang
discarding the assumption silently — and it is the proof that on this axis a
claim about generated code is verified by reading generated code.

---

## Divergences from the origin document

An auditable record of the corrections applied on top of
[`origem/setup-cpp-performance-academy.md`](origem/setup-cpp-performance-academy.md), which remains intact.

| # | Origin | Correction |
|---|---|---|
| 1 | §11 asked for `referencias.md` / `references.en.md` | identical stem: `referencias.md` / `referencias.en.md` (section 2) |
| 2 | §2 exemplified `trilha/03-cache-memory/` in English; §3 mandated English for technical directories | per-layer rule: curriculum in Portuguese, code in English (section 3) |
| 3 | the heading hierarchy mixed `##` and `#` for the same level | six parts as `#`, sections as `##` |
| 4 | §1 and §4 instituted bilingualism; the document existed in PT-BR only and without navigation | navigation also mandatory in `docs/`; this document's EN pair |
| 5 | §20 cited GCC, LLVM, POSIX, Linux, Intel/AMD as authorities with no identifier; CPPREFERENCE was defined and absent from the list | identifiers created in the catalogue; the table in section 24 cites only existing ids |
| 6 | §21 said "preferably" for 18 sections, and required Hardware counters in a conceptual chapter | mandatory versus conditional made explicit (section 25) |
| 7 | §6 defined the four classes of claim without notation | mandatory marker (section 7) |
| 8 | §10 banned "AI-generated content" without saying what that implies in AI-assisted material | traceability rule (section 18) |
| 9 | §12 required an `<a id>` tag in addition to the title | removed: it was unverified decoration; the verifier only knows title anchors (section 20) |
| 10 | the contract directive named `static_assert` as "active only in DEBUG" | the mechanism for that intent is `assert` (the `NDEBUG` trigger); `static_assert` is unconditional and cannot see runtime values. **Both are used, for different things**, and `assert` alone loses the information in release — three gates T/R/E (section 13) |
| 11 | the directive to avoid conditionals/loops would have been a style rule | promoted to a measured thesis (section 14) |
| 12 | nothing required a comparison protocol, a measurement scope, a budget, or a build matrix | sections 10, 11, 16 and 27 |
| 13 | nothing defined the division of labour with the sibling projects | section 30 |
| 14 | nothing required state labelling | section 5 |

---

## Divergences from the track's origin document

A second table, and a second origin document:
[`origem/setup-lowlatency-structures-academy.md`](origem/setup-lowlatency-structures-academy.md),
also preserved intact. The corrections below were verified on this machine
before entering the standard.

| # | Origin | Correction |
|---|---|---|
| 1 | §2.2 said test dependencies come "pinned by hash in `subprojects/*.wrap`, with `.wraplock`" | `.wraplock` **does not exist** in the DPDK Academy, and in Meson `.wraplock` is the directory lock used during download, not a dependency lockfile to commit. The hash pin is already in the `.wrap`; the mention goes |
| 2 | §7 planned "HdrHistogram_c, wrap pinned by hash" | it is **not in WrapDB** — `releases.json` queried on 2026-09-21: 367 projects, none of them a histogram. The wrap has to be hand-written with its own `meson.build`, because the project is CMake; and its log writer pulls in zlib, which collides with "no dependency beyond the compiler" (section 16). Decision recorded in the ROADMAP, harness stage |
| 3 | §7 planned "one `sanitize` profile with ASan, UBSan and TSan" | ASan and TSan do not coexist: two profiles (section 16) |
| 4 | §2.2 described the quality tooling as "checkers for anchors, self-description and parity" | it is 23 programs and 6,827 lines in the DPDK Academy; porting it is the project's largest task, not a checklist item (section 31) |
| 5 | §4 planned **two trees for one subject**: `docs/0N-*` with the theory and `trilha/0N-*` with the practice, linked by a table | **a single tree, under `docs/`**. Three reasons, the first internal: section 30 assigns TLB, cache and false sharing to the DPDK Academy — exactly the content of `docs/01-memoria` and `docs/02-layout` —, so the structure mandated re-teaching what the standard mandates citing. Second: section 25 already makes *Mental model, Fundamentals, How it works* mandatory **inside** every topic, so the theory already has a home. Third is measured: in the DPDK Academy the code migrated into `docs/` (31 files, against 4 in the track), and the track became an index of topics with no code — two trees predict which half stays empty. The word *trilha* survives where it means something: the **order**, in `docs/plano-estudo.md` |
| 6 | §7 put arm64 in scope, "compiles, no numbers published" | there is no machine and no runner; arm64 leaves the scope until there is one (section 16) |
| 7 | §6 promised the experiment "what happens when the TSC is not invariant" | the reference machine has `constant_tsc` and `nonstop_tsc`: not reproducible here, and it stays an open question (sections 7 and 29) |
| 8 | §8 listed what each measurement records, with no CCD, no histogram precision and no between-run variation | all three go in, and the third is inherited from a DPDK Academy defect: a validity criterion published with its calibration nowhere (section 28) |
| 9 | §10 planned extracting `ferramental/bench/` as a final stage, consumed by the DPDK Academy too | the DPDK Academy is C; the boundary has to be a C ABI **from the start**, or the extraction does not happen (section 33) |
| 10 | §5 called the shared interface of `std/` and `custom/` `contract.hpp` | collision with the three-gate mechanism; a topic's law is `spec.hpp` (section 34) |
| 11 | §4 planned `ferramental/bench/` for the harness | the absorbed harness is `lib/measurement/`, and `lib/` is the extraction boundary. `ferramental/` is left to quality and charts (section 15) |
| 12 | §1.2 said the order book is a "recurring scenario, not an object of study", while the track gives it a whole module plus the capstone | pending an editorial decision, recorded in the ROADMAP: either the README owns the book as the protagonist, or module 04 shrinks |

## Navigation

- [Reference catalogue](referencias.en.md)
- [Documentation index](README.en.md)
- [Repository README](../README.en.md)
- [This standard's origin document, intact](origem/setup-cpp-performance-academy.md)
- [The track's origin document, intact](origem/setup-lowlatency-structures-academy.md)

> [🇧🇷 Português](padrao-do-projeto.md) | 🇺🇸 English
