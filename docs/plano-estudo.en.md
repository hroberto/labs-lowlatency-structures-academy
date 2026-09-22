> [🇧🇷 Português](plano-estudo.md) | 🇺🇸 English

# Study plan — the reading order

> **State: written.** The order exists; **1 of the 23 topics** exists. Each
> module declares its own state in its index, and this page does not announce
> them as if they were ready.

This document is the **reading order**. It is not the build order, which is the
[ROADMAP](../ROADMAP.en.md) — and the distinction exists because in the upstream
project two competing numberings shared one README and "level 5" came to mean two
different things depending on the document.

The two orders **diverge on purpose**: module 08 is the first to be built,
because it measures the instrument all the others use, and the second to last to
be read, because it only matters to someone who already wants to measure
something.

## What is assumed, and what is not

**Assumed:** intermediate C++ — templates, RAII, everyday `std::` —, Linux from
the command line, and a notion of threads.

**Not assumed, and taught by the material:** cache hierarchy, TLB, the C++ memory
model, atomics and ordering, allocators, memory layout, percentiles and how to
measure them.

Asymptotic complexity is a **premise, not a subject**. This material does not
teach algorithms: it measures what the constant hidden behind the big-O costs on
this machine.

## Before any module

| Step | Why |
|---|---|
| [Repository README](../README.en.md) | what the project is, and what it is not |
| [`./scripts/check-env.sh`](../scripts/check-env.sh) | tells you whether your machine compiles what the track uses, by probing features instead of trusting versions |
| [Standard, sections 6 to 9](padrao-do-projeto.en.md#6-the-methodological-path) | the path, the classes of claim, and the rule that a number needs a program |
| [Standard, section 28](padrao-do-projeto.en.md#28-reproducibility) | how to read the numbers: percentile, spread across runs, and what the CCD has to do with it |

Whoever is only going to **consult** — not study — has a shortcut: the table in
`regras-de-decisao.md`, not yet written, is the reference artefact. It says
*where to switch*, without the path that led there.

## The order

### First, the instrument — [08 measurement](08-medicao/README.en.md)

Out of order, and deliberately: reading [08.01](08-medicao/01-harness/README.en.md)
first costs fifteen minutes and prevents the wrong reading of **every** number
that follows. It answers why a p50 of 20 ns may be the clock rather than the
structure.

Anyone who would rather skip the detour can come back to it when the first
number bothers them. It will.

### 1. [Memory](01-memoria/README.en.md) → 2. [Layout](02-layout/README.en.md)

Where the data **lives** before how it is **organized**. Allocation comes first
because the cost of layout only shows up once allocation stops dominating —
measuring SoA against AoS under allocator pressure measures the allocator.

### 3. [Containers](03-conteineres/README.en.md) → 4. [Order book](04-livro-de-ofertas/README.en.md)

Module 03 gives the generic structures; 04 is the same problem with **a domain
that constrains**, and the constraint is what allows beating the generic
structure. Reading 04 before 03 makes the flat array look like magic instead of
what it is: a trade that only fits because the price has a range and a tick.

### 5. [Queues and concurrency](05-filas-e-concorrencia/README.en.md) → 6. [Time and errors](06-tempo-e-erros/README.en.md)

05 is the first module where **two cores** appear, and therefore the first that
needs the C++ memory model cited by clause. 06 comes after because `rdtsc` only
matters to someone who has measured enough for the cost of `steady_clock` to
bother them.

### 7. [Parsing](07-parsing/README.en.md)

The module of the **uncomfortable result**: in several cases `string_view` with
`from_chars` is already the answer, and the hand-rolled decoder does not pay off.
Deliberately near the end — whoever got here has seen six modules where the
alternative did pay off, and that is what gives weight to the case where it does
not.

### 8. [Measurement, part 2](08-medicao/README.en.md) and 9. [Generated code](10-codigo-gerado/README.en.md)

The two cross-cutting ones, and both better **after** there is something to
measure. 08.02 opens the PMU; 10 asks what the compiler did with `custom/` that
it did not do with `std/`.

### 10. [Capstone](09-capstone/README.en.md)

The full engine, in two versions, with the table that **attributes** the
end-to-end difference to each earlier module. It is not reading: it is the check
that the previous nine add up.

## Three ways to use this material

| Reader | Path |
|---|---|
| **studying** | the order above, in full, with 08.01 in front |
| **consulting while designing** | `regras-de-decisao.md` and, from the module at hand, only *Decision rule* and *Limitations* |
| **reproducing** | `check-env.sh`, then the topic's `bench/medicoes/` and the `metadata.json` beside it — that machine is described there, not in prose |

## Navigation

- [Documentation and standard](README.en.md)
- [Build order](../ROADMAP.en.md)

> [🇧🇷 Português](plano-estudo.md) | 🇺🇸 English
