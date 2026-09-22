<!-- cita-defeito -->
> [🇧🇷 Português](README.md) | 🇺🇸 English

# Quality — the checkers

<!-- This document REPRODUCES the wrong arithmetic that the arithmetic checker
     catches, in order to explain what it does. The marker above exempts it from
     that gate and shows up in its final count: the exemption is declared, not
     silent. -->

Ported from `ferramental/qualidade/` of
[labs-dpdk-academy](https://github.com/hroberto/labs-dpdk-academy). **Each one
was born from a measured defect, not from theory** — and the reason to port
instead of rewriting is in the header of `verificar-retratacoes.py`: in the
defect that originated it, whoever wrote the retraction knew the rule, had just
stated it, and still left the retracted value circulating on other pages.
**Manual correction does not scale.**

## The four ported from the DPDK Academy

| Checker | Rule | The defect that originated it |
|---|---|---|
| `verificar-links.py` | a relative link points to a file and anchor that exist | a dead anchor in a heading with an em dash |
| `verificar-ancoras.py` | a line anchor points to the right piece of code | a line number ages silently when the code changes |
| `verificar-retratacoes.py` | a value declared retracted does not survive outside the block retracting it | the retraction was written, and the retracted number stayed published on another page |
| `verificar-aritmetica.py` | a percentage the text makes checkable adds up | "100 of the 123 ms, 83%" — and 100/123 is 81,3% |

## What was born here

| Checker | Rule | The defect that originated it |
|---|---|---|
| `verificar-suposicao.py` | the Gate R precondition reaches the optimizer in release | Clang was discarding the assumption silently, because the predicate is a function call |
| `verificar-paridade.py` | the language pair: existence, navigation, structure and numbers | the identical-stem rule existed to be verifiable and was not verified |

The first is the only one of the six that looks at **generated code**. It exists
because `PERF_EXPECTS` promises two things — costing nothing in release AND
handing the precondition to the optimizer — and the second can stop holding
without anything breaking: the code stays correct, it compiles, and the suite
stays green.

It has a **control arm**: the probe is compiled with the assumption and with it
neutralized, and the dead path is required to disappear in the first and to
**survive** in the second. That is not zeal — the control arm caught two defects
in the probe itself:

1. the first version had `main` calling `probe(argc)`, and GCC removed the dead
   path without the assumption, by propagating the range of `argc`;
2. the second used a sentinel constant, and GCC emitted it as
   `andl $0x5EEDBEEE` + `addl $1` — the constant **minus one** — keeping the
   branch and escaping the detector.

Both times it said "I am not measuring what I claim to measure" instead of
reporting success. The final correction looks for an **external symbol**, which
does not reduce to arithmetic and shows up in the assembly of any architecture.

## Self-test

**All six have a self-test**, and it checks the checker itself against
constructed cases, including false-positive baits:

```bash
for v in links ancoras aritmetica retratacoes suposicao paridade; do
    python3 ferramental/qualidade/verificar-$v.py --autoteste
done
```

This matters because a checker that starts accepting everything keeps printing
`N links verificados, 0 quebrados` — green indistinguishable from the legitimate
kind.

And all of them **skip with code 77** when `python3` is missing, instead of
disappearing from the suite: `pular-sem-python3.sh` exists because the absence
of that handling made the suite shrink and report green.

## The two from the DPDK Academy not yet ported

| Checker | Rule | Why not yet |
|---|---|---|
| `verificar-promessa.py` | every cited program exists in the tree and enters the build | 18 coupling points with the origin project's structure |
| `verificar-autodescricao.py` | what the material claims about itself matches the disk | 838 lines, 34 coupling points |

The second is what sustains the standard's state labelling
([section 5](../../docs/padrao-do-projeto.en.md#5-state-labelling)), and its
absence is declared debt in the [ROADMAP](../../ROADMAP.en.md).

## What the parity checker caught on its first run

The rule of the **identical-stem language pair**
([section 2](../../docs/padrao-do-projeto.en.md#2-file-convention)) exists
precisely to be machine-verifiable, and now it is. On its first run against the
tree it reported ten problems, and **eight were its own** — six about numbers
and two about navigation:

- it read `.` as a thousands separator in Portuguese, which turned `15.2.0` into
  `1520`, `1003.1` into `10031` and `802.3` into `8023` — a compiler version and
  a standard's identifier do not change separator when the language changes. The
  canonical form now **removes** the separator instead of interpreting it,
  accepting a rare false negative (`12,3` and `123` collide) in exchange for
  zero false accusations;
- it read the **example** navigation of [section
  4](../../docs/padrao-do-projeto.en.md#4-language-navigation), which lives in a
  code block because the standard teaches the rule by showing it, as if it were
  the document's own navigation — and demanded that the standard point at
  `README.en.md`.

Both defects became false-positive baits in the self-test. The **two**
legitimate findings were the `ferramental/` READMEs with no English pair, now
written.

The proportion is the argument for self-tests with baits: four out of every five
accusations on the first run came from the checker, and all of them looked like
findings.

## The gate: `pre-commit.sh`

It runs in both places, and the reason is that each covers a gap the other does
not: a local hook does not run on a third party's pull request nor on
Dependabot's, by construction; a CI step does not prevent the bad commit from
existing, it only fails it afterwards.

| Step | What it checks |
|---|---|
| syntax | `bash -n` on every script and `compileall` on every `.py` |
| documentation | the six checkers **and their six self-tests** |
| secrets | private keys and recognisable token patterns in tracked files |
| CI pin | the action's pinned SHA is still the top of the major declared in the comment |
| suite | full mode only, which is the local hook's |

The `--rapido` mode exists because without it CI would run the suite twice and a
failure would have no address: the red would show up before `meson setup`,
without saying which configuration broke.

The pin check has already paid for itself: it caught that the SHA written in the
workflow's first version was not the top of `v5`.

To install the hook:

```bash
ln -sf ../../ferramental/qualidade/pre-commit.sh .git/hooks/pre-commit
```

## Navigation

- [Tooling](../README.en.md)
- [Documentation and standard](../../docs/README.en.md)

> [🇧🇷 Português](README.md) | 🇺🇸 English
