# NVVM backend status

The general development loop is **stopped**. The authorized finite maintenance sequence has completed
harness consolidation261 and master integration/acceptance262. Only results package263 remains;
then stop. No push. Read [WORKFLOW](WORKFLOW.md), [RESULTS](RESULTS.md) and
[HANDOFF](HANDOFF.md) for a fresh session.

## Accepted compiler baseline

[Validation262](runtime-validation.slice-262.json) is authoritative; [report262](report.slice-262-master-integration.md)
explains the merge, exact deltas and validation. Master `6eb89786ca882d71049c8568638e247f60864b6f`
is merged at `5294b5ae6`. Compiler source is `49593da724e172838bd65b9eb48b2a3f11334522`,
version `2026.18.3-275-g49593da72`; runner source is `c3455e606`. All22 dependency pins are exact.
Provider ABI42 and qualified support through scalar FP8 widening260 are retained.

| Evidence                                       | Accepted262 result                                          |
| ---------------------------------------------- | ----------------------------------------------------------- |
| Frozen / discovery                             | 1356 / 357 cells                                            |
| Combined                                       | 1713 total;1674 correct;39 unresolved;18 resolved histories |
| Units / semantics                              | 1086 pass13 skip /1170 pass78 skip                          |
| Focused / runtime / toolkit / material support | 29 /4 /18 /6 pass                                           |
| AST proof                                      | 705 tags;497025 pairs;zero failures (optimized)             |
| Last full / targeted / implementation cadence  | 262 /233 /0                                                 |

Acceptance is **full checkpoint plus explicit serial infrastructure closure**. The raw full run had
1673 correct and one extra NVRTC failure deleting `default_program.pch`. Three declared serial rounds
passed all three modes (9/9); only that NVRTC outcome is substituted, with original failure retained.
Concurrent automatic-PCH reliability remains unresolved. Six intentional upstream texture rejections
change existing unresolved diagnoses; none is new support or a semantic resolution. Known semantic
mismatches remain the three column-major modes. The other36 gaps are infrastructure/preflight.
Material runtime is unassessed: binding, texture/LUT, input and output contracts are unavailable.

Compiler SHA256: `4a207ec0d3f390152dd593a21af81f4b1fe30d9e1c5edd065343bd3590f8bbbc`.
Provider SHA256: `fbef1a9e22f3ac0cd42d3ffbade22470f7143930608924fc39b5bc57e40eb913`.

## Current finite action

Execute [results plan263](plan.slice-263-results-package.md) with unchanged accepted binaries.
Raw measurements go under `build/nvvm-results/2026-09-26-integration`; the reviewed package goes
under `results/2026-09-26`. No compiler optimization or development-loop restart is authorized.

Environment: native Ubuntu24.04, L4 SM89, driver580.126.09, SM80 target, CUDA12.9.2,
NVRTC12.9.86, LLVM14, RelWithDebInfo. Four CPU workers maximum, two unit servers, sequential GPU
suites and isolated measurements. Retain the parallel PCH incident when planning future validation.
Raw maintenance evidence is under `build/nvvm-maintenance`; prior260 evidence remains unchanged.
[HISTORY](HISTORY.md) navigates earlier architecture, inventories and findings.
