> [🇧🇷 Português](referencias.md) | 🇺🇸 English

# Reference catalogue

The central bibliography of the **Low-Latency Structures Academy**. The identifiers in
brackets are **stable**: they do not change if the documents are reorganized,
and they are what the modules cite.

How to cite, from inside a module:

```markdown
[INTEL-ORM](../../docs/referencias.en.md#intel-orm)
```

The anchor comes from the **entry title**, not from an `<a id>` tag. GitHub
generates a section's identifier with github-slugger, so `### INTEL-ORM` yields
`#intel-orm` on its own — and that is what
[`verificar-links.py`](../ferramental/qualidade/) checks. The initial version of
this project's standard also required an explicit `<a id>` tag; it was removed
because it was **unverified decoration**: nothing checked that the tag and the
title agreed, and two identifiers for the same section diverge silently.

## State of link verification

> **No address in this catalogue has been opened and checked yet.** The entries
> carry author, title, venue and year, which are the metadata a citation needs;
> the addresses were written from editorial memory and are **pending
> verification**. Where the canonical identifier is a DOI, the field reads
> `DOI: pending` — this project does not invent DOIs, because a wrong DOI is
> worse than a missing one: it looks verifiable.
>
> Checking the addresses and filling in the DOIs is an open task, recorded in
> the [ROADMAP.en.md](../ROADMAP.en.md).

## Source levels

| Level | What it is | Role |
|---|---|---|
| **A** | normative or primary | highest precedence when it directly defines the behaviour under study |
| **B** | peer-reviewed academic literature | mechanism and method |
| **C** | recognized technical literature | explanation and interpretation |
| **D** | supplementary material | support; never replaces A when A exists |

---

# CPU architecture

### INTEL-SDM

**Intel 64 and IA-32 Architectures Software Developer's Manual** — Level A

x86 architecture, instruction set, memory architecture, performance counters,
system programming.

- Version consulted: **to be declared by the first module citing it** (see versioned references)
- Address: `https://www.intel.com/sdm` — pending verification

### INTEL-ORM

**Intel 64 and IA-32 Architectures Optimization Reference Manual** — Level A

Microarchitecture, pipeline, cache, prefetch, SIMD, branch prediction.

- Version consulted: to be declared
- Address: pending verification

### AMD-OPT

**AMD Software Optimization Guides** — Level A

Zen microarchitecture, cache, execution, prefetch, SIMD, performance
monitoring.

**Always** select the document matching the generation actually used in the
experiment. "AMD documentation" without a generation does not identify an
authority.

- Version consulted: to be declared per generation
- Address: pending verification

### ARM-ARM

**Arm Architecture Reference Manual** — Level A

AArch64 architecture, memory model, ordering, barriers.

It is in the catalogue because **arm64 is in this project's declared scope**. The
first version of the standard listed only Intel and AMD while the sibling
project declared arm64 support — a divergence between promise and bibliography.

- Address: pending verification

### ARM-NEOVERSE

**Arm Neoverse Software Optimization Guides** — Level A

Per-core Neoverse optimization; the equivalent of INTEL-ORM and AMD-OPT for
arm64 server targets.

- Address: pending verification

---

# C++: standard and evolution

### CPP-STD

**ISO/IEC 14882 — Programming Languages — C++** — Level A

The normative source for object lifetime, memory model, atomics, sequencing,
undefined behaviour, language semantics.

This project declares **C++23** as its target. See the toolchain section of the
[project standard](padrao-do-projeto.en.md).

- Address: pending verification

### WG21

**ISO C++ WG21 Papers** — Level A (with a caveat)

Origin of features, design rationale, changes to the memory model, proposals for
C++23 and C++26.

> **Mandatory caveat:** a WG21 paper must **not** be automatically interpreted
> as a feature incorporated into the standard. Its status must be checked — and,
> in this project, toolchain support must be checked too, because "adopted into
> the standard" and "available in the compiler" are different things.

- Address: `https://wg21.link/` — pending verification

### P2900

**Contracts for C++** — Level A (proposal)

`pre`, `post` and `contract_assert`. Adopted for **C++26**; **does not exist in
C++23**.

> **Measured on this machine on 2026-09-16:** GCC 15.2.0 rejects `int f(int x)
> pre(x>0)` even with `-std=c++2c -fcontracts`, with the error `expected
> initializer before 'pre'`. That is why this project's contract mechanism uses
> C++23's `[[assume]]` behind a macro rather than Contracts: it is not a style
> preference, it is what the toolchain offers. See the three-contract-gates
> section of the [project standard](padrao-do-projeto.en.md).

- Address: `https://wg21.link/p2900` — pending verification

### P1774

**Portable assumptions** — Level A

The origin of `[[assume]]`, incorporated in **C++23**.

> **Measured on this machine on 2026-09-16:** `[[assume(x > 0)]]` compiles on
> GCC 15.2.0 and on Clang 21.1.8 with `-std=c++23 -Wall -Wextra`.

- Address: `https://wg21.link/p1774` — pending verification

### P1938

**`if consteval`** — Level A

Incorporated in C++23. Lets a contract predicate behave differently under
constant evaluation and at runtime.

> **Measured on this machine on 2026-09-16:** compiles on GCC 15.2.0 with
> `-std=c++23`.

- Address: `https://wg21.link/p1938` — pending verification

### CPPREFERENCE

**cppreference** — Level C

An excellent operational reference and index for language and library features.

> `cppreference != ISO C++ Standard`. In a normative discussion, CPP-STD or WG21
> take precedence.

- Address: `https://en.cppreference.com/` — pending verification

### CPP-ASSERT

**`assert` — `<cassert>` / `<assert.h>`** — Level A (standard) and C (operational reference)

The assertion macro, switched off by defining `NDEBUG`. **It is the origin of
this project's Gate R design**: `PERF_EXPECTS` is `assert` plus `[[assume]]`,
inheriting from it the `NDEBUG` trigger and the behaviour under constant
evaluation.

> **Measured on this machine on 2026-09-16, GCC 15.2.0, `-O2`:**
>
> | configuration | `assert` alone | `assert` + `[[assume]]` |
> |---|---|---|
> | without `NDEBUG` | checked; dead path eliminated | same |
> | with `NDEBUG` | **not** checked; dead path **present** | not checked; eliminated |
>
> With `NDEBUG`, `assert` expands to `((void) 0)` and neither the check nor the
> information survives — the redundant conditional returns to the release
> binary, which is exactly what the project's second directive aims to
> eliminate. Hence the two halves of the mechanism.
>
> Also verified: `assert` is a one-argument macro and rejects a comma inside
> `<>` (`macro 'assert' passed 2 arguments, but takes just 1`), which is why
> `PERF_EXPECTS` is variadic; and a false `assert` in a `constexpr` function
> already breaks the build, by reaching `__assert_fail`, which is not
> `constexpr`.

- Standard: [CPP-STD](#cpp-std), the `<cassert>` clause
- Operational reference: `https://en.cppreference.com/w/cpp/error/assert` — pending verification

---

# Compilers

### GCC-DOC

**GCC documentation** — Level A

Optimization options, attributes, builtins, implementation-defined behaviour.

The first version of the standard cited "GCC" as an authority in the
per-module reference table without giving it an identifier. This entry fixes
that.

- Version: **GCC 15.2.0** on the reference machine
- Address: `https://gcc.gnu.org/onlinedocs/` — pending verification

### LLVM-DOC

**LLVM documentation** — Level A

Intermediate representation, optimization passes, cost model.

- Address: pending verification

### CLANG-DOC

**Clang documentation** — Level A

Options, attributes, diagnostics, extensions.

- Version: **Clang 21.1.8** on the reference machine
- Address: pending verification

### LLVM-MCA

**llvm-mca — Machine Code Analyzer** — Level A

Static analysis of throughput and execution ports over already-generated code.
An instrument for the compile-time thesis: it allows comparing two code
generations without running either.

- Address: pending verification

### UICA

Abel, A.; Reineke, J. — **uiCA: Accurate Throughput Prediction of Basic Blocks
on Recent Intel Microarchitectures** — Level B

Basic-block throughput prediction. Complements and frequently corrects llvm-mca
on recent microarchitectures.

- DOI: pending

---

# Operating system and Linux

### LINUX-DOC

**Linux Kernel Documentation** — Level A

The authority for the domains of tunable parameters: `vm.*`, `net.*`, the
scheduler, writeback, hugepages, NUMA.

> **Mandatory authority for any domain describing a kernel tunable**, and
> **with the kernel version**: the limits have changed between versions, and
> `vm.swappiness` is the known case. Domains of that kind belong to the final
> stage (see [EX442-LABS](#ex442-labs)); the foundation's contract library
> models the domains of the measurement ruler itself.

- Address: `https://www.kernel.org/doc/html/latest/` — pending verification

### CGROUP-V2

**Control Group v2 — Linux Kernel Documentation** — Level A

The unified interface, `cpu.weight`, `cpu.max`, `memory.max`, `io.weight`,
`cpuset`.

The direct authority for the cgroup domains that EX442 exercises.

- Address: pending verification

### LINUX-PERF

**Linux perf / perf_events documentation** — Level A

Hardware and software counters, sampling, PMU, `perf stat`, `perf record`,
`perf report`.

- Address: pending verification

### PERF-EVENT-OPEN

**`perf_event_open(2)`** — Level A

`perf` is not merely an external tool: there is a kernel subsystem exposing
performance events, and the chain is

```text
CPU PMU → Linux perf_event → perf(1) → measurement
```

It matters here because the capstone reads counters **through the call**,
without depending on `perf(1)` in the path.

> **An operational warning the first version of the standard did not carry:**
> PMU access depends on `perf_event_paranoid` and frequently **does not exist**
> inside a virtual machine or a container. A document publishing a hardware
> counter must state that the PMU was available.

- Address: `https://man7.org/linux/man-pages/man2/perf_event_open.2.html` — pending verification

### PMU-TOOLS

Kleen, A. — **pmu-tools / toplev** — Level C

An implementation of top-down analysis over Linux counters. The instrument of
YASIN-2014.

- Address: pending verification

### POSIX

**IEEE Std 1003.1 — POSIX** — Level A

System interfaces used outside the C++ standard: shared memory, mapping,
affinity, signals.

Also without an identifier in the first version of the standard, despite being
cited in the per-module table.

- Address: pending verification

---

# Performance engineering

### AGNER-CPP

Fog, A. — **Optimizing Software in C++** — Level C

C++ optimization, compiler behaviour, CPU, SIMD, branching, data structures,
code generation.

- Address: `https://www.agner.org/optimize/` — pending verification

### AGNER-INST

Fog, A. — **Instruction Tables** — Level C

Latency, throughput and execution ports per instruction and per
microarchitecture.

A **distinct** entry from AGNER-CPP: they are different manuals, and the first
version of the standard treated the author as a single reference.

- Address: pending verification

### AGNER-UARCH

Fog, A. — **The Microarchitecture of Intel, AMD and VIA CPUs** — Level C

Pipeline, out-of-order execution, branch prediction per microarchitecture.

- Address: pending verification

### BRENDAN-GREGG-SP

Gregg, B. — **Systems Performance: Enterprise and the Cloud** — Level C

Performance methodology, profiling, observability, CPU analysis, latency, the
USE method, Linux performance.

- Address: pending verification

### DREPPER-MEM

Drepper, U. — **What Every Programmer Should Know About Memory** — Level C

Memory hierarchy, cache, TLB, prefetch, NUMA, access patterns.

- Address: pending verification

### MCKENNEY-PERFBOOK

McKenney, P. E. — **Is Parallel Programming Hard, And, If So, What Can You Do
About It?** — Level C

Cost of synchronization, memory ordering, RCU, scalability. Its table of
operation costs is a comparison point already used by the sibling project.

- Address: `https://arxiv.org/abs/1701.00854` — pending verification

---

# Measurement method and benchmarking

### GOOGLE-BENCHMARK

**Google Benchmark — User Guide** — Level D

Microbenchmark methodology, fixtures, timing, iterations, counters,
configuration.

> Google Benchmark **measures code**. It does not replace scientific
> methodology.

- Address: pending verification

### KALIBERA-JONES-2013

Kalibera, T.; Jones, R. — **Rigorous Benchmarking in Reasonable Time** —
ISMM 2013 — Level B

Rigorous treatment of repetition, warm-up, autocorrelation and confidence
intervals in software benchmarking.

> **It is the authority behind this project's measurement ruler.**
> `lib/measurement/statistics.hpp` explicitly declares that it does **not** do
> formal inference — there is no confidence interval and no hypothesis test,
> because microbenchmark samples are neither independent nor normally
> distributed. This reference is what the reader should seek when they need the
> treatment that file declines to perform.

- DOI: pending

### YASIN-2014

Yasin, A. — **A Top-Down Method for Performance Analysis and Counters
Architecture** — ISPASS 2014 — Level B

The basis of Top-down Microarchitecture Analysis (TMA): attributing issue slots
to useful retirement, bad speculation, front-end bound and back-end bound.

> **It is the method missing for the "Hardware counters" section to make
> sense.** Without it, a hardware counter is a list of numbers with no
> attribution model.

- DOI: pending

---

# Concurrency and correctness

### HERLIHY-WING-1990

Herlihy, M.; Wing, J. — **Linearizability: A Correctness Condition for
Concurrent Objects** — ACM TOPLAS, 1990 — Level B

The foundational reference for concurrent structures, lock-free programming,
queues, correctness, linearizability.

> Performance never replaces correctness. An extremely fast and incorrect queue
> is not a valid optimization.

- DOI: pending

### MICHAEL-SCOTT-1996

Michael, M. M.; Scott, M. L. — **Simple, Fast, and Practical Non-Blocking and
Blocking Concurrent Queue Algorithms** — PODC 1996 — Level B

The canonical concurrent queue algorithm; a mandatory companion to
HERLIHY-WING-1990 in the lock-free module.

- DOI: pending

### BOEHM-ADVE-2008

Boehm, H.-J.; Adve, S. V. — **Foundations of the C++ Concurrency Memory Model** —
PLDI 2008 — Level B

The rationale of the C++ memory model. The atomics and memory-ordering modules
pointed only at CPP-STD and WG21, which say **what** the model is and not **why**
it is that way.

- DOI: pending

### WILLIAMS-CCIA

Williams, A. — **C++ Concurrency in Action** — Level C

Practical use of atomics, ordering, primitives and concurrent patterns in modern
C++. Cited as a name in the first version of the standard, without an
identifier.

- Address: pending verification

---

# Memory allocation

### JEMALLOC

**jemalloc** — Level D

Arenas, size classes, per-thread cache, fragmentation.

### TCMALLOC

**TCMalloc** — Level D

Per-thread cache, batch transfer, pages.

### MIMALLOC

**mimalloc** — Level D

Sharded free lists, locality, behaviour under concurrent load.

> These three are here because the allocation module pointed only at CPP-STD and
> AGNER-CPP — and the **mechanism** of a modern allocator is in neither. They
> are Level D: they describe implementations, not standards.

---

# Sibling projects

### DPDK-ACADEMY

**labs-dpdk-academy** — Roberto, H. M.

`https://github.com/hroberto/labs-dpdk-academy`

A study of high-throughput, low-latency architecture with DPDK. **It is this
project's authority for already-measured hardware and operating-system mechanism
costs**: budget per unit of work, the user/kernel boundary, address translation
and the TLB, hugepages, cache and locality, false sharing, NUMA and the
first-touch policy, SMT, polling versus sleeping, DMA and descriptors, IOMMU,
the bus budget.

See the division of labour declared in the
[project standard](padrao-do-projeto.en.md). This project **cites and extends**;
it does not re-teach.

### EX442-LABS

**labs-EX442** — Roberto, H. M.

A performance-tuning lab on RHEL 10: `sysctl`, `sysfs`, TuneD, cgroups, process
priority and scheduling policy, hugepages, overcommit, swappiness, NUMA, SYSV
shm, I/O scheduler, writeback, network buffers from BDP, PCP, eBPF.

> **This reference is only citable in the final stage.** The
> operating-system tuning axis does not enter the foundation or the conceptual
> modules: it appears in a stage of its own, at the end, as an integration plan
> between this project and low-level optimization. The reason is pedagogical —
> mixing the two axes means studying kernel tuning instead of the object model,
> layout and code generation. See section 30 of the
> [project standard](padrao-do-projeto.en.md).

The experiment cycle adopted here — baseline, hypothesis, one change, new
measurement, verification — comes from it, and that one applies from the
start.

---

## Navigation

- [Project standard](padrao-do-projeto.en.md)
- [Documentation index](README.en.md)
- [Repository README](../README.en.md)

> [🇧🇷 Português](referencias.md) | 🇺🇸 English
