# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 209 is accepted as the latest full checkpoint.** Read the
[completed worker plan](plan.slice-209-active-mask.md),
[five-part report](report.slice-209-active-mask.md), and
[result manifest](runtime-validation.slice-209.json). Parent completed independent acceptance
review and owns the local commit. No push is authorized.

The existing hardware-mask mapping now uses a sideeffect/convergent PTX active-mask read through
typed provider ABI 36. Raw scalar/uint4 CUDA intrinsics no longer emit illegal full-mask ballots.
Existing implicit aggregate shuffle matches the CUDA prelude's raw-read then ballot(mask,true)
composition. Logical `WaveGetActiveMask` synthesis is unchanged. Hardware snapshots do not promise
source-level logical reconvergence; the prelude's tracking TODO remains.

Latest accepted implementation and full checkpoint: 209. Implementation slices since it: 0.
Next: investigate and correct the demonstrated tiny-double source-literal roundtrip defect in a
bounded slice. Correctness takes priority; all six material cells compile, but application runtime
contracts remain absent. Reconsider complex motivation at each selection.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                      |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| Slice 209 accepted full     | [Manifest](runtime-validation.slice-209.json), [frozen](census.slice-209.tsv), [discovery](discovery-census.slice-209.tsv) | 1644 fresh cells: 1591 correct, 53 retained failures. Six additions, no old deltas. |
| Slice 208 accepted targeted | [Manifest](runtime-validation.slice-208.json)                                                                              | Four old fixes and nine additions, preserved freshly by 209.                        |
| Slice 207 accepted full     | [Manifest](runtime-validation.slice-207.json)                                                                              | Previous accepted full checkpoint; 209 refreshes all preservation obligations.      |

Full frozen 452 × 3 has 1333 correct (449/442/442); discovery 96 × 3 has 258 correct (86/86/86).
All 1585 accepted 208 correct cells are freshly preserved, with no inherited runtime cells. Exact
identity/mode, classification, return code, full execution counts, diagnostics and canonical shapes
match cumulative 207+208 for every old cell. No missing/duplicate cells or lost prior passes.
Historical healthy denominators remain 427 frozen and 72 discovery. No old source/oracle changed.

## Unresolved failures and limitations

The slice 209 manifest preserves all 53 open failures with history/evidence and all four resolved
records from slice 208. Frozen retains 18 preflight and 5 infrastructure cells; discovery retains 4 preflight,
22 infrastructure and 4 runtime mismatches. Both full runners return the expected diagnostic exit 2.

- Quad reconvergence, FP64 min/max and prefix-min/max KernelContext pointer shapes remain separate.
- FP64 vector-by-value and implicit matrix shuffle admission remain deferred; this slice only
  corrects existing hardware-mask semantics. Slice 208 explicit matrix and FP64 arithmetic stay valid.
- Hardware mask subsets are scheduling-dependent. New oracles check caller/exclusion/singleton
  membership and own-lane matrix transport; exact PTX and operand provenance prove the correction.
- FP8/BF16/prelude and discovery infrastructure/output gaps remain visible.
- A demonstrated next correctness candidate is tiny-double CUDA spelling: `SourceWriter::emit(double)`
  in `source/slang/slang-emit-source-writer.cpp:227` uses fixed fractional precision for small
  exponents, losing significant digits below one. Slice 208's retained
  `build/nvvm-loop/slice-208-before/aggregate.cu:326` contains `0.00001525878907671`.
  Parent may select a bounded round-trip precision slice; no implementation was started here.
- Six complex cells freshly compile/assemble. Bindings, textures/LUT/input and output contract are
  absent; no material runtime, speed or correctness claim.

Rolling 207 rotation, 208 FP64 wave arithmetic, 209 active-mask correctness is wave-heavy. The existing
correctness defect explicitly overrides complex cadence; do not infer material semantics to meet it.

## Current working environment and final gates

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09; target SM80. CUDA 12.9.2/NVRTC 86, LLVM 14, provider ABI 36. Matching
optimized tools/libraries are in `build/RelWithDebInfo/{bin,lib}`. Source
`build/nvvm-loop/slice-203-env.sh`; the bare setup environment selects Debug. Use the local
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Four total CPU workers, two unit servers,
sequential suites and `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Final gates: focused 9, runtime 4, units 475 with one existing Windows-only skip, toolkit 18, full
corpora and complex 6 pass their defined criteria. Explicit SM50/60 raw-mask probes compile/assemble
in both NVRTC and NVVM (four cells). Lower-target probes make no runtime claim.

Compiler SHA256: `483db7465914c1626c8fd427f425eebbcd04e8f996a26ba031dec65c0893a231`.
Provider SHA256: `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Tested source base: `d54288b4d9483bd3d6a3036453fa7079321fd163` plus manifest source hashes.
Raw artifacts: `build/nvvm-loop/slice-209-before` and `slice-209-after`. Exact final-fixture
emitter-only revert proves the old full-mask ballot defect without dispatching undefined kernels.
After unit-whitespace cleanup, units were rebuilt/rerun; compiler/provider/runtime hashes are
unchanged. All final hashes match. No GPU loss, driver change or reboot occurred; old A6000 history
remains in slice 201 without an established cause.
