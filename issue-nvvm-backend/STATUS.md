# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 210 is accepted as the latest full checkpoint.** Read the
[completed worker plan](plan.slice-210-double-roundtrip.md),
[five-part report](report.slice-210-double-roundtrip.md), and
[result manifest](runtime-validation.slice-210.json). Parent completed independent acceptance
review and owns the local commit. No push is authorized.

Shared `SourceWriter::emit(double)` now preserves small finite double constants with scientific
notation below the binary bound where fixed precision supplies enough significant digits. The
motivating `(1 + 2^-30) / 65536` roundtrips in CUDA, HLSL, GLSL and C++. Classic locale, signed zero
and existing trimming remain. No NVVM semantic, provider or ABI change occurred.

Latest accepted implementation and full checkpoint: 210. Implementation slices since it: 0.
Next: establish repeated compile-time measurements for the two unchanged material entries and
identify a measured optimization opportunity before implementation. Six material cells compile,
but application runtime contracts remain absent; do not infer runtime semantics.

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
