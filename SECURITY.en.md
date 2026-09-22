> [🇧🇷 Português](SECURITY.md) | 🇺🇸 English

# Security policy

## What this project is, and why that defines the scope

**This is study material.** There is no running service, no user data, no
credentials, and nothing here is deployed to production by this repository. The
code exists to be read, compiled and measured by whoever is studying.

That changes what "vulnerability" means here. The asset to protect is the
**integrity and provenance of the content** — that nobody publishes as if they
were the maintainer, and that published numbers correspond to the programs that
produce them. It is not confidentiality: there is no secret in this repository.

## What is in scope

Report as a security problem:

- **a published number with no program that produces it, or with broken
  provenance.** This is the project's asset. A table whose campaign is not in
  the tree, or a `metadata.json` that does not match the commit it declares, is
  the gravest possible defect here;
- **example code that leads to unsafe practice without warning.** The material
  studies custom allocators, lock-free structures and hardware counter reads. A
  snippet that leads the reader to corrupt memory or to trust an invariant the
  code does not guarantee, *without declaring the risk*, makes the damage happen
  on the studying reader's machine;
- **a script that changes the host in an undeclared way.** No script in this
  repository requires privilege for the track, and the only one asking for
  `sudo` does so to read firmware (`ambiente.sh --cachear-memoria`). A script
  that starts changing system state without declaring it is a security problem,
  not a style one;
- **build chain compromise:** a dependency, CI action or artefact that executes
  unintended code. The CI action is pinned by SHA and `pre-commit.sh` checks
  that the SHA is still the top of the declared major;
- **any content that allows publishing in the maintainer's name.**

## What is out of scope

- a **deliberately** defective program, when the suite requires it to fail. The
  control arm of `bench-harness` (`--braco-de-controle`) simulates an invalid
  collection on purpose, and the L2 test requires the program to reject it;
- declared limitations: measurements from one machine only, modules marked as
  skeletons, the twelve checkers not yet ported and listed in the ROADMAP;
- automated scanner reports with no exploitation analysis in this project's
  context.

## How to report

**Open a public issue** at
`https://github.com/hroberto/labs-lowlatency-structures-academy/issues`.

Choosing a public issue over a private channel is consistent with the scope: if
the asset is the provenance of the content, the correction is public too — a
wrong table is fixed by publishing the fix, not in silence. There is no user
data to protect while a fix is prepared.

## Provenance, and what sustains it

| Mechanism | What it guarantees |
|---|---|
| signed commits, and `main` requiring a verified signature | whoever wrote it is who they say they are |
| a versioned measurement campaign, with `metadata.json` | the published number points at the program, machine and commit that produced it |
| `pre-commit.sh` and CI running the same checkers | consistency does not depend on anyone remembering |
| the CI action pinned by SHA, `permissions: contents: read` | the build chain does not execute what was not reviewed |

## Navigation

- [Repository README](README.en.md)
- [Project standard](docs/padrao-do-projeto.en.md)

> [🇧🇷 Português](SECURITY.md) | 🇺🇸 English
