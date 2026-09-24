# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 212 research is accepted.** Read its
[plan](plan.slice-212-batch-lifetime.md), [five-part report](report.slice-212-batch-lifetime.md),
and [batch lifecycle measurements](batch-lifetime.slice-212.json). Complete six-cell API batches
improve 10.767% (8848.207 to 7895.490 ms), including global creation/destruction. Every paired
improvement exceeds 10%, all six compile-call medians improve, all 168 outputs exactly match
210/211, and six distinct PTX hashes freshly assemble. Post-release RSS grows 9000 KiB equally
at both policy boundaries, then plateaus for the last three pairs; bounded evidence only.
No production compiler/provider/runner/shader changes or material runtime claims.

**Slice 211 remains accepted supporting research.** Its
[measurements](compile-time.slice-211.json) established the initial steady-state candidate;
212 supplies the previously missing finite-batch startup/teardown and order/lifetime evidence.

**Slice 210 remains accepted as the latest full checkpoint.** Read the
[completed worker plan](plan.slice-210-double-roundtrip.md),
[five-part report](report.slice-210-double-roundtrip.md), and
[result manifest](runtime-validation.slice-210.json). Parent completed independent acceptance
review and owns the local commit. No push is authorized.

Shared `SourceWriter::emit(double)` now preserves small finite double constants with scientific
notation below the binary bound where fixed precision supplies enough significant digits. The
motivating `(1 + 2^-30) / 65536` roundtrips in CUDA, HLSL, GLSL and C++. Classic locale, signed zero
and existing trimming remain. No NVVM semantic, provider or ABI change occurred.

Latest accepted implementation and full checkpoint: 210. Implementation slices since it: 0.
Next: select slice 213 to validate and admit FP64 implicit aggregate shuffles now that slice 209
corrected hardware-mask acquisition. Prefer this bounded backend feature over a production batching
protocol for this iteration. Batching remains a measured performance candidate; its opt-in worker
must preserve mandatory fresh-process coverage, exact identities/options/oracles, deadlines and
crash isolation, and pass a full checkpoint before adoption. Its own performance evidence must
include process startup/exit; the API metric is not a production-runner speedup.
Application runtime contracts remain absent; do not infer runtime semantics.

Research 211/212 does not advance implementation cadence or reset checkpoint obligations. All
accepted 210 recorded source/artifact hashes still match. Full 210 results remain inherited,
with zero fresh runtime cells in 212 and every open/resolved failure history retained.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                          |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| Slice 210 accepted full     | [Manifest](runtime-validation.slice-210.json), [frozen](census.slice-210.tsv), [discovery](discovery-census.slice-210.tsv) | 1647 fresh cells: 1594 correct, 53 retained failures. Three additions, zero old deltas. |
| Slice 209 accepted full     | [Manifest](runtime-validation.slice-209.json)                                                                              | Immediate preservation baseline; all 1591 correct cells freshly preserved.              |
| Slice 208 accepted targeted | [Manifest](runtime-validation.slice-208.json)                                                                              | Four fixes and nine additions remain preserved; resolved histories retained.            |

Frozen 452 x 3 has 1333 correct (449/442/442); discovery 97 x 3 has 261 correct (87/87/87).
Every old cell matches exact identity/mode, classification, return code, full execution counts,
diagnostic and canonical shape. Three new cells pass. No missing, duplicate or inherited runtime
cells. Historical healthy denominators remain 427 frozen and 72 discovery. Old source/oracles unchanged.

## Unresolved failures and limitations

The 210 manifest preserves all 53 open failures with history/evidence and four resolved 208 records.
Frozen retains 18 preflight and 5 infrastructure cells; discovery retains 4 preflight, 22 infrastructure
and 4 runtime mismatches. Both full runners return expected diagnostic exit 2.

- Quad reconvergence, FP64 min/max and prefix-min/max KernelContext pointer shapes remain separate.
- FP64 vector-by-value and implicit matrix shuffle admission remain deferred.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and discovery infrastructure/output gaps remain visible.
- Six complex cells compile/assemble; bindings, textures/LUT/input and output oracle are absent.
  No material runtime, speed or correctness claim.

Rolling 208 FP64 wave arithmetic, 209 active masks and 210 source literals prioritize demonstrated
correctness. The existing source-literal bug explicitly overrides complex cadence for 210; no
speculative material change was made.

## Current environment and final gates

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 86, LLVM 14, provider ABI 36. Use matching
optimized tools/libraries in `build/RelWithDebInfo/{bin,lib}`, source `build/nvvm-loop/slice-203-env.sh`
and local `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Four total CPU workers,
two unit servers, sequential suites and `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Final gates: focused 4 (API 2127 patterns x 4 targets and GPU 3), smoke 4, units 476 with one existing
Windows-only skip, shared-target 16 with 3 unavailable platform skips, toolkit 18, full corpora and
complex 6. Structural probes confirm four-target corrected spelling and runtime loads/FP64 compares
in three PTX modes. Before GPU NVRTC failed and direct NVVM passed; old API emission failed 92 bit checks.

Compiler SHA256: `f8dc709857e70fbf7e4c0bbe2d94f9c1528f789ddf609a4c0db6f5bb90b776c7`.
Provider SHA256: `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Tested base: `1f161256b4bc8f57aa96bc0dd250f8e2a537ef1c` plus manifest source hashes. Final hashes
match; raw under `build/nvvm-loop/slice-210-{before,after}`. No GPU loss, driver change or reboot;
old A6000 history remains in 201 without an established cause.
