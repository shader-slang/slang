# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 208 is accepted with targeted validation.** The last full checkpoint remains 207. Read the
[completed worker plan](plan.slice-208-fp64-masked-wave.md),
[five-part report](report.slice-208-fp64-masked-wave.md), and
[result manifest](runtime-validation.slice-208.json).

FP64 masked sum/product reductions and inclusive/exclusive prefixes now execute through existing
typed recipes for scalars/vectors, with matrix reductions and explicit-mask matrix transport.
The two unchanged frozen sum/product workloads pass NVVM O0/O3. Source-faithful negative/positive
zero identities and singleton passthrough preserve raw signaling-NaN, quiet-NaN, infinity and
negative-zero bits. FP64 min/max, vector-return shuffles and implicit matrix shuffles remain
explicitly unsupported; provider/library/catalog/ABI 35/general type handling are unchanged.

Next: investigate and correct the audited active-mask semantics gap in a bounded slice. All six material cells compile/assemble; application runtime bindings,
texture/LUT/input and expected-output contracts remain absent. No material speed/correctness claim.

## Checkpoints and evidence

Latest accepted implementation and targeted acceptance: 208. Last full checkpoint: 207.
Implementation slices since the full checkpoint: 1. Slice 208 is not a full checkpoint.

| Area                               | Authoritative record                                                                                                                                                                      | Interpretation                                                                                                                 |
| ---------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| Slice 208 accepted targeted        | [Manifest](runtime-validation.slice-208.json), [frozen outcomes](census.slice-208.tsv), [discovery outcomes](discovery-census.slice-208.tsv), [selection](selection.slice-208-frozen.tsv) | 603 fresh cells: 565 correct, 38 retained failures. Four old fixes and nine additions. 1035 frozen cells explicitly inherited. |
| Slice 207 accepted full checkpoint | [Manifest](runtime-validation.slice-207.json), [report](report.slice-207-wave-rotation.md)                                                                                                | 1629 fresh cells: 1572 correct, 57 retained failures.                                                                          |
| Complex support                    | `complex_corpus` in [slice 208 manifest](runtime-validation.slice-208.json)                                                                                                               | All six cells freshly compile/assemble; no runtime contract.                                                                   |

Fresh selection covers 107 frozen identities (all wave/quad, double/helper neighbors) and the complete
94-identity discovery manifest, all three modes. Exact keys, classifications, return codes, full
execution counts, diagnostics and canonical shapes match 207 except four expected sum/product fixes.
No missing/duplicate cells or lost prior passes. Cumulative ledger: 1638 cells, 1585 correct, 53 known
failures; 552 previous passes refreshed and 1020 inherited. Cumulative frozen correct counts are
449/442/442 over 452 identities; fresh discovery 84/84/84 over 94 identities. Historical healthy
benchmarks remain 427 frozen and 72 discovery. No frozen source or old discovery oracle changed.

## Unresolved failures and next candidates

The slice 208 manifest preserves all 57 prior failure histories: 53 remain unresolved and 4 are resolved
with exact transitions. It identifies each fresh/inherited failure and links prior evidence.

- Four wave/quad frozen identities remain: quad-control reconvergence, scalar/vector FP64 min/max,
  and the two prefix-min/max KernelContext pointer shapes. They remain separate features.
- A newly audited existing correctness gap: `_emitNVVMActiveMaskValue` uses a full-mask ballot,
  not a divergent active-mask read. Actual PTX and the authoritative participation contract are
  linked in the manifest. A provisional implicit low16 pass is excluded from correctness evidence;
  new FP64 implicit matrix support is rejected pending the correct semantics. Existing 32-bit behavior is unchanged.
- FP64 vector-by-value shuffle still reaches a separate 32-bit compound resolver. The admitted
  explicit matrix OutParam path is covered independently.
- FP8/BF16/prelude and discovery infrastructure/output gaps remain visible. A development probe
  also exposed pre-existing tiny-double CUDA constant spelling precision; raw evidence is retained,
  no source-emitter change or new corpus claim made.
- Slice 207 lower-target rotation probes remain inherited, including its explicit SM50/SM60 options
  and exploratory half-prelude limitation. Their library/standard-module implementation is unchanged.

Rolling accepted history: 206 Boolean lanes (complex-driven), 207 rotation (wave), 208 FP64 wave
arithmetic (wave). Reconsider complex cadence at next selection; correctness work can override it
only with an explicit reason. Material runtime/performance still needs application semantics.

## Current working environment

- Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
- L4 SM89, driver 580.126.09; target SM80. CUDA 12.9.2, NVRTC/NVCC 12.9.86, LLVM 14, provider ABI 35.
- Matching optimized tools/libraries: `build/RelWithDebInfo/{bin,lib}`. Source optimized environment
  `build/nvvm-loop/slice-203-env.sh`; bare setup `env.sh` selects Debug.
- Four build/corpus workers, two unit servers, sequential suites, `CMAKE_BUILD_PARALLEL_LEVEL=1`.
- Build skill: `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`.
- Parent accepted the slice after independent review and owns local commits; no push is authorized.

Final 208 gates pass: focused 10, runtime 4, units 474 with one Windows-only skip, toolkit 18, selected
corpora and complex 6. Compiler SHA256:
`242c23acfafa2e08d5ae9d23fdaa2457656550a7ba7a0a687d978d18f05319c8`.
Provider SHA256: `6da3abff11e6b67a1dcd5e3b12f341bc7cbfa356fd29c9d7b5dcad8e20efd676`.
Tested source base `cdb5a654732183df67b5c6db834eee652723e690` plus exact manifest source hashes.
Raw artifacts: `build/nvvm-loop/slice-208-before` and `slice-208-after`. Exact final fixtures reject
on the reverted accepted compiler (same 207 library hash), then pass after restoration. All tested
source/artifact hashes match after gates. No GPU loss, driver change or reboot occurred. The old
A6000 incident remains historical in slice 201 with no established cause.

## Updating this handoff

Keep slices bounded and all historical evidence distinctions intact. Parent owns acceptance/local
commits; one fresh worker owns each implementation slice.
