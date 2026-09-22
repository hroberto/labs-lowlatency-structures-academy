> [🇧🇷 Português](README.md) | 🇺🇸 English

# 10 — Generated code

> **State: not started**, with one piece already in use. See
> [state labelling](../padrao-do-projeto.en.md#5-state-labelling).

## The module's question

> what does the compiler do with `custom/` that it does not do with `std/`?

It is the question the structure modules do not ask. This axis was the absorbed
project's own object (see the [ROADMAP](../../ROADMAP.en.md), stage 1) and
survived as a **cross-cutting module** instead of being diluted into the others —
section 36 of the standard defines it.

## What this module brings

- `constexpr`, templates, `[[assume]]` and the shift of work to compile time as a
  **measured hypothesis**, never as a style rule;
- **a sweep, not a single point**, because the variable under test interacts with
  the memory hierarchy;
- the **cost of compiling** among the metrics.

## What this module is not

It is not a comparison between compilers, nor a catalogue of flags. The pair is
still `std` and `custom`, and the compiler is the **instrument**, not the object.

## The piece that already exists

[`verificar-suposicao.py`](../../ferramental/qualidade/verificar-suposicao.py)
compiles a probe and checks **in the assembly**, with a control arm, that the
gate R precondition reaches the optimizer. It was born from a measured defect —
Clang discarding the assumption silently — and it is the proof that on this axis
a claim about generated code is verified by reading generated code.

It is the only one of the six checkers that needs a compiler, which is why it
does not run in the fast mode of the
[gate](../../ferramental/qualidade/pre-commit.sh): it runs in the suite, where
the compilers exist.

## Planned topics

None yet. The first candidates come out of what the structure modules produce:
wherever `custom/` wins by code generation rather than by layout, the finding
becomes a topic here.

## Navigation

- [Documentation and standard](../README.en.md)
- [Repository README](../../README.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
