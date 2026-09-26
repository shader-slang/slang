# Widen canonical FP8 scalars to Float32

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM plans/reports in
local commits. The maintainer now requests finishing only this slice and then stopping the development loop. Fresh delegation remains unavailable at the
agent-thread limit; one local writer uses separate source, oracle and mechanical acceptance audits.

## Purpose and Observable Result

Execute `float(bit_cast<FloatE4M3>(byte))` and the corresponding FloatE5M2 expression dynamically at
NVVM O0/O3. Every finite byte must widen to exact Float32 bits, including both signed zeros and all
subnormals. E5M2 infinities stay signed infinity; FP8 NaNs produce Float32 NaNs without promising their
payload or sign. NVRTC O3 and an independent rational oracle must agree on this stated contract.
Do not admit reverse narrowing or decide the separate shared-overflow versus CUDA SATFINITE policy.

## Progress

- [x] 2026-09-25: Accepted259 committed as dbb9bfb9015618ebf730fe54fde5b9233658e475; clean tree and
      all 136 source/12 artifact/566 input hashes verified after commit.
- [x] Read current STATUS/WORKFLOW, research243, implementation249 and FP8 design/provider contracts.
- [x] Select and bound scalar widening before implementation; material evidence257/258 reconsidered.
- [x] Baseline136/12/566 matches259; runtime4 passes. The frozen512-word fixture passes NVRTC
      exactly and both direct modes reject floatCast. Separate E5M2 O0/O3 probes reject the same
      canonical conversion. Before source/oracle/output and traces retained.
- [x] ABI42 catalog/provider widening implemented. First512-word GPU fixture passes all 3 modes;
      both provider units pass. A new FP8-to-UInt negative was frontend-ambiguous; remove it while
      migrating only the obsolete widening rejection to positive coverage. Production unchanged.
- [x] Corrected runtime4/focused6 and 6 exhaustive GPU controls pass. Independent oracle checks
      1536 fixture words and 24768 supplemental words, including 48 raw NaN classifications. PTX
      retains dynamic helpers/conversion at SM80. Final prototype2 identity137/12/567 captured.
- [x] 2026-09-26: Full checkpoint and separate mechanical acceptance pass:1713 cells/1674 correct,
      all 1710 old outcomes exact,39 unresolved/18 resolved histories retained; all prior unit and
      semantic IDs preserved, material6 PTX/cubins exact259.
- [x] Complete helper/input-shape review and report/design/STATUS; record the maintainer stop.
- [x] Evidence closure passes:4,854 indexed artifacts,137 final source snapshots and351 compact
      references. This completed plan accompanies the accepted local slice commit; loop stopped.

## Surprises and Discoveries

Research243 already qualifies all 256 FP8 encodings with independent integer/rational expectations
and raw widening. Producer244 repairs finite/subnormal folding. Transport249 supplies distinct
canonical FP8 descriptors with physical i8. Runtime FloatCast remains separate; the original dynamic
object workload still first rejects helper result A and needs record/any-value contracts beyond this
slice. No claim of resolving that workload follows from scalar widening.

## Decision Log

- 2026-09-26, maintainer: Finish the interrupted slice260, commit it if accepted, then stop.
  Do not select another slice; further development requires explicit resume.
- 2026-09-26, parent: The interrupted full driver has no surviving process or final exit. Units,
  semantic regressions, toolkit and runner contracts have recorded successful exits and are retained.
  The frozen attempt wrote results but lost its wrapper exit; archive the whole attempt and repeat
  that uncertified gate, then run the remaining discovery/material gates on the same source.

- 2026-09-25, parent: Select scalar FP8 widening over whole-record BF16/FP8 transport. Exact widening
  has a complete finite input domain and prior qualified recipe; record values require additional
  representation and marshalling proof. The narrowing/overflow policy is unnecessary here.
- Current material profiles257 and failed/discarded getter258 provide the cadence review. Do not
  repeat the failed optimization or invent a new one from no evidence. Material runtime inputs and
  oracle remain unavailable. This capability exception is explicit; reconsider material work next.
- Keep semantic format distinct from physical i8. Add only scalar FP8→Float32 to the existing
  FloatCast operation resolver and a named provider recipe; no generic IEEE float or integer-cast
  widening. Advance provider ABI41→42 to negotiate the new supported semantic contract.

## Outcomes and Retrospective

Scalar FP8 widening is accepted with exact finite/signed-zero/infinity bits and NaN classification.
Every byte under both branch flags passes at NVRTC O3 and NVVM O0/O3. The full checkpoint has 1713
cells/1674 correct/39 unresolved and preserves18 resolved histories. All 1710 old five-field outcomes
are exact; the new fixture adds3 correct cells. Units1051 pass/13 skip and semantics1052 pass/77 skip
preserve every prior identity. Material6 PTX/cubins are exact259, without a runtime/performance claim.
Full260/targeted233/cadence0 is current. Rolling implementations256,259,260 are capability slices;
research258's discarded material experiment records the cadence exception. No next slice is selected.
Evidence closure passes. This completed plan is committed with the accepted slice. The development
loop is stopped as explicitly requested by the maintainer; no further slice is authorized.

## Context and Current Pipeline

Consider `[noinline] float expand(FloatE4M3 x) { return float(x); }`, called with a byte loaded at
runtime and bit_cast to the canonical format. Core declarations and lowering produce FloatE4M3Type
and FloatCast to Float. `_getNVVMSemanticType` already records the exact FP8 descriptor, while the
shared semantic catalog rejects this pair. Provider `emitValueOperation` dispatches the catalog's
family and has existing BF16 conversion helpers to illustrate the ownership boundary. The checked
input is valid; no producer repair or reconstituted syntax is needed.

## Scope and Non-Goals

Expected production files: source/compiler-core/slang-nvvm-semantic-catalog.h, shared builder API
revision/comment and source/slang-llvm-nvvm/slang-llvm-nvvm.cpp. Only change emitter selection if the
canonical trace proves an additional necessary admission gate. Tests: focused provider/semantic
units, adjacent preflight negatives and one new tests/cuda/nvvm-fp8-widening.slang discovery source.
No FP8 storage/pointers/records/resources/vectors/arithmetic, BF16 change, Float32→FP8 narrowing,
integer/Half/double constructor, literal overflow/nonfinite policy, external helper ABI, dynamic
object implementation, driver/system/profiling permission change or push.

## Architecture and Invariants

Admit exactly scalar kFloatE4M3 or kFloatE5M2 input and scalar kFloat32 output for FloatCast. All
other descriptor dimensions and operation kinds stay rejected. Reuse physical i8 registers and
internal helper transport. The provider decodes sign/exponent/fraction from valid bits. Normal
finite values construct exact Float32 bits; subnormals are exact small integers times 2^-9 (E4M3)
or2^-16 (E5M2). E4M3 magnitude127 and E5M2 exponent31 nonzero fractions are NaNs; E5M2 exponent31
fraction0 is infinity. Signed zero is preserved. Constant shifts are bounded and all speculative
select operands must be defined. Avoid copying unreachable generic research-template branches.
No arithmetic rounding or FP32 underflow occurs in this widening contract.

## Interfaces and Dependencies

Provider ABI42 must be built/staged together with the compiler and tests. Root integration is enabled
with LLVM14.0.6 at build/nvvm-provider/llvm-build/lib/cmake/llvm; targeted compiler/test builds stage
the matching provider. Follow the local slang-build skill. Native Ubuntu24.04, RelWithDebInfo,
CUDA12.9.2/NVRTC12.9.86, SM80 on L4SM89/driver580.126.09. Source slice-203-env.sh; maximum4 CPU workers,
2 test servers, sequential GPU suites,30-minute bounds. Record all final binary/toolkit hashes.

## Milestones

1. Capture baseline259 identities/environment into build/nvvm-loop/slice-260-before. Freeze a
   source regression/oracle spanning all 256 encodings per format, dynamic helper calls and both
   signs/zero/subnormal/finite/nonfinite classes. Normalize only NaN observations to an explicit
   classification marker for FileCheck; raw supplemental output must check classification itself.
   Verify before NVRTC output and both direct failures before editing production.
2. Add the exact catalog family/admission and provider widening. Reuse common LLVM builder
   operations, central format descriptor information and existing semantic operation validation.
   Audit every helper/branch and input shape. Test wrong direction, wrong width/lane count/format,
   unsupported casts, storage and exported FP8 helper boundaries.
3. Build: CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4
   --target slangc slang-test render-test test-server. Verify provider ABI42 and matching artifacts.
   Run runtime4 first, focused source/unit negatives, exhaustive raw-byte widening in3 modes and
   actual IR/PTX path checks. Independent rational enumeration must not reuse the provider recipe.
4. Run full units, semantic neighbors, toolkit18/contracts6, frozen452x3 with census.slice-195.tsv,
   discovery118x3 plus3 new cells, and all 6 material compile/assembly cells. Full checkpoint is required
   by the shared provider/API contract change. Retain every old input and failure history.
5. Compare all 1710 old five-field outcomes exactly (unless an investigated intended resolution is
   established), every 1063 unit and 1129 semantic identity/outcome. Separate additions, missing,
   duplicates, ignored/executed counts and fresh/inherited evidence. Close raw artifacts, report,
   compact manifests, design and STATUS before local acceptance/commit.

## Validation and Acceptance

All finite/infinity widening outputs must match exact IEEE Float32 bits; all transported NaN inputs
must yield NaN classification. Do not mistake numeric equality for signed-zero preservation or
normalize finite mismatches. Check original byte/sentinel fields and whole buffers, including
non-output regions. Preserve E4M3 exponent15 finite values256..448 and the E5M2 infinity distinction.
Provider units must reject the unqualified reverse and descriptor neighbors before mutation.
All corpus cells and gates must use final matching source/binaries; compare material support cells
without claiming material runtime or kernel speed. Preserve original39 unresolved and 18 resolved
histories and all accepted259 local-record outcomes. Baseline compiler hash638db4d1cbbfb6c9a3ccca2f6058a89c45071e94ab1ba51a710fee5b8a3bf5f9,
provider hash5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab on ABI41.

## Failure and Recovery

Keep failed attempts and immutable before inputs. If a producer emits the wrong canonical shape,
fix its invariant or explicitly revise this bounded plan; do not patch malformed semantics in the
provider. Revert an unqualified recipe rather than relax output expectations. Stop on GPU loss or
an unresolvable regression and record the smallest required human decision. Never change drivers,
reboot or reset preservation baselines. A new independent diagnostic is a handoff, not scope growth.

## Artifacts and Hand-Off

Raw roots build/nvvm-loop/slice-260-before and slice-260-after. Completed plan/report, provider/compiler
changes, tests/manifest addition, exact compact result maps, design and STATUS belong in the accepted
local commit. Generated buffers/PTX/cubins/logs/snapshots stay ignored. Retain259 closure immutable.
