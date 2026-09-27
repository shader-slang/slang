# Characterize local FP8/BF16 record-array boundaries

This bounded ExecPlan follows `.agent/PLANS.md` and the committed NVVM-plan exception. The user
resumed the development loop, then explicitly requested stopping after the current slice296 on
2026-09-27. Complete this slice only; skip Slack, no push/system changes. Root owns scope/execution/acceptance
and commits. Fresh record_arrays296 prepares raw fixtures/scripts; a separate reviewer must approve
sources, independent oracles, exact obligations and interpretation. No compiler widening in this slice.

## Purpose and Observable Result

Determine which canonical local record-array shapes reach the current NVVM admission boundaries,
and demonstrate independent bit-transport results wherever the accepted compiler can execute them.
Separate source arrays optimized away from retained helper/storage arrays. Preserve correct unsupported
diagnostics; a diagnostic moving to another unsupported operation is not a compiler fix.

## Progress

- [x] 2026-09-27:295 accepted/committed292a2534c; full293/compiled285 remains installed unchanged.
- [x] Read WORKFLOW/STATUS and contract; fresh read-only proposal traces admission/layout/address roles.
- [x] Root before-identity check passed100layout/37runtime/11source/2config/576inputs/22pins
      at292a2534c. Fresh independent reviewer assigned; no workload execution.
- [x] Prepared four families/control and18obligations; independent review approved v2 sources,
      integer oracle, exact native IDs and strict runner.
- [x] Root before/after identity gates preserve100layout/37runtime/11source/2config/576input/22pins.
      All18obligations completed across original13 plus reviewed unrun5; independent results/IR audit passed.
- [x] Compact report/18-row record/plan/navigation formatted; final independent closeout review
      accepts the slice. This local commit closes296; the loop is stopped. No subsequent slice authorized.

## Context and Current Pipeline

Flat/nested FP8/BF16 local record values and selected mutable references are qualified by270/279/287.
Source record arrays remain explicitly excluded. `_isNVVMSupportedSubstandardRecordFieldType` in
slang-emit-nvvm-type-lowering.cpp walks struct fields but not arrays; its public record proof requires
an IRStructType root. General copyable/helper array proofs recurse arrays but reject substandard leaves
and authorize broader memory domains. Do not add FP8 leaves to those general predicates.

Additional owners include `_lowerArrayType` and type-use `supports`, local aggregate CUDA layout,
`_getNVVMSequentialElementPointer`, `_getNVVMStructFieldAddress`, and helper parameter/result gates.
Physical i8/i16 lowering is representation, not admission authority. Canonical source arrays may be
valid inputs intentionally rejected by these gates. If a physical `_Array_*` wrapper appears, trace
its producer in slang-ir-lower-buffer-element-type.cpp before interpreting it as a local source array.

Provider285's integer nested-store correction remains unchanged. It reduces one known physical-store
risk but establishes no FP8 array admission. BF3/BF4 physical component arrays and BF16 array layout
queries likewise do not prove retained runtime record-array support. Do not reopen267's material gap,
known NVRTC nested-copy vendor research or N65536 compile-timeout experiments.

## Scope and Proposed Fixtures

Prepare four fixed families under ignored build/nvvm-record-arrays296, not the main corpus:

1. Local-only mixed Cell[2], dynamic index0/1, every field initialized and checked.
2. Root Pair through explicit noinline initialization and whole out-copy helper boundaries.
3. Guarded Wrapper with the same array through retained initialization/copy boundaries.
4. Integer-only guarded-wrapper control with identical field widths, inputs and expected bits.

Use the smallest mixed record and independently calculated CUDA layouts:

```slang
struct Cell
{
    uint16_t before;
    FloatE4M3 a;
    FloatE5M2 b;
    BFloat16 scalar;
    uint16_t after;
};
typedef Cell Pair[2];
struct Wrapper { uint head; Cell values[2]; uint tail; };
```

Expected Cell offsets0/2/3/4/6, size8/alignment2; Pair size16/alignment2;
Wrapper offsets0/4/20, size24/alignment4. These are proposed canonical calculations, not observed
emitted layouts. Confirm actual queries/IR where available; do not use a failed compile as layout proof.
Keep flat Cell fields to avoid deliberately exercising the known nested-record vendor-copy defect.
No BF2/BF3/BF4 extension, multidimensional/large arrays, resource/shared/readonly/exported roles,
pointer-result admission, aggregate ABI promise or floating arithmetic/conversion scope.

The raw oracle uses host seed and all65,536 low16 patterns over two elements, not a65,536-element array.
For element k, let t=(i^seed^(k*0x5a5a))&65535; expected fields are before=t^0x1357,
E4M3bits=t&255, E5M2bits=(t>>8)^0xa5, BF16bits=t^0x6c39, after=t^0xe17b. Compare only integer bits
recovered through canonical bit casts. Every BF16 encoding and both FP8 bytes occur; combinations are
correlated, not Cartesian. No numeric conversion is an oracle.

For copy families, initialize the destination differently; check destination and unchanged source
against independent expressions immediately after the whole copy. Wrapper guards use distinct
input-dependent values. Output mismatch masks must be zero and completion counts must equal65,536;
all output slots start at nonzero sentinels that differ from expected values. Author must freeze the
exact complete output vector and bit assignments before review/execution. Local-only outcomes must
identify any scalarization; source spelling alone never proves backend array storage.

Prepare one deliberately corrupted integer-wrapper oracle control at NVRTC O3, with one guard-bit
flip and an independently predicted nonzero mask. Freeze corruption source and expected rejection;
retain it separately from passing fixtures. No learned NVRTC output is used as the reference.

## Architecture, Invariants and Dependencies

No tracked compiler/provider/test source changes. Preserve original valid shapes, compare roles
without pointer casts, physical payload substitutions, manually unrolled copies or forced admission.
Use existing noinline/copy fixture conventions and maintained native CUDA directive adaptation.
A syntax/front-end failure is an invalid probe, not an expected backend unsupported outcome.

Run four families at NVRTC O3/NVVM O0/O3:12 mode obligations. Mixed retained arrays may legitimately
fail E52017; the first precise refusal is unknown until capture and must be recorded without retargeting
the probe. An unexpected success requires exact GPU output and final-IR evidence before any support
statement. Integer control requires real GPU success in all modes; a wrong NVRTC control remains wrong.
Existing negative units prove exact root FP8 array helper-parameter and wrapper-result exclusions:
select nvvmSlangFloat8UnsupportedRolesStopBeforeEmission and
nvvmSlangNestedSubstandardRecordsRejectOtherRoles; record their full native IDs before freeze,
and preserve their no-builder/provider-mutation checks. Do not author replacement unit expectations.

Capture canonical IR for the three mixed source roles at NVVM O0 with identical source and explicit
entry/target/capability; three additional compile obligations, retaining failures and their diagnostic
stage. Failed NVVM compiles provide no GPU evidence. Total bounded inventory:12 mode cells,
1 expected-corruption control,2 existing native unit selections,3 IR captures. No extra shape matrix.
If an exact retained shape cannot be reached with valid source, report that limit rather than add a
compiler prototype or silently alter the intended role.

Installed identity is accepted285/full293: compiler62469125, providerABI42 af1661de, version301-g8fbf0f84e,
source8fbf0f84e+patch12f503e9. Verify100layout/37runtime/11source/2config/576inputs/22pins before/after.
Native L4SM89/driver580.126.09, CUDA12.9.2/NVRTC12.9.86, targetSM80. No build required; future builds
must refresh version metadata and rebuild restored294 sources. Reuse unchanged owned_process.py for
cleanup. One workload at a time; shader/IR cells180s, outer gate1800s, max4CPU, no retries.

## Milestones and Acceptance

1. Author prepares/fixes raw source syntax by static review only, deriving every expected output and
   corruption mask. Freeze all18 obligations, originals/directives/options, exact existing unit IDs,
   body/oracle/script hashes, baseline identity and source-role hypotheses. Reviewer independently
   checks the oracle and valid noinline shapes; root verifies identity before any execution.
2. Run frozen characterization serially. Require exact native IDs/counts/full outputs on executable
   cells; retain unsupported diagnostics, zero execution and unrun dependents distinctly. Expected
   rejection is a negative contract, not GPU correctness. Stop on syntax/identity/oracle/control failure
   for a reviewed versioned repair or bounded inconclusive closeout; no production changes.
3. Trace producer IR -> admission -> representation/layout/address consumer for each refusal or
   unexpected success. Explain whether source shape is canonical, optimized away or physical wrapper.
   A root signature refusal does not prove downstream layout/address correctness. Record whole-copy
   survival where supported; do not infer it from fields merely appearing in source.
4. Independently review evidence and unchanged identity. Commit compact research/report/plan/navigation
   only. Baseline293, main580cases/576inputs/1740cells,1703correct/37unresolved/20histories and
   lastfull293/targeted233/cadence0 stay inherited. A later implementation needs its own bounded plan,
   complete local-domain proof and full shared-type checkpoint; it is not authorized by this slice.

## Surprises and Discoveries

Read-only selection found several independent gates, not one predicate to broaden. Existing BF16
array metadata and physical BF3/BF4 component arrays must remain distinct from runtime record arrays.
One read-only inventory script encountered a directory ending in .slang; skipping non-files corrected
that inspection without running a workload.

## Decision Log

- 2026-09-27, root: Characterize the excluded boundary before proposing admission. Select a flat
  mixed Cell plus integer control to avoid combining a new role with known nested-copy uncertainty.
- 2026-09-27, user: Stop the development loop after the current slice296. Root owns final acceptance/
  local commit; queued parser repair requires explicit resume.
- Root: Keep only18 explicit obligations; no optional BF2 or resource/address-space cross-product.
  Preserve unsupported cases as evidence, and distinguish each failed compilation from GPU execution.

## Outcomes and Retrospective

Six GPU cells pass22complete output words. Mixed NVVM O0/O3 reject local var, root
OutParam<Array<Cell,2>> and OutParam<Wrapper>; three O0 IR captures repeat those refusals (rc255),
retaining canonical arrays, dynamic indexing, noinline helpers and whole loads/stores in pre-emission
Slang IR. No LLVM/PTX emitted. Layout decorations use Natural rule0, not observed CUDA/provider layout.
Both existing native units pass. Corruption rejects[32,0,65536,9321] as predicted; shader remains failed.
All18obligations are accounted for and no completed cell was rerun. Baseline293/compiled285 is unchanged.

V1 runner was replaced before execution to distinguish compiler-only results and additional compiler
errors. V2 stopped after cell13 because an earlier NVRTC heuristic matched nvrtc-o3 in the source path
and preempted the valid FileCheck matcher. Original infrastructure/failed result and five unrun entries
remain. Reviewed raw adjudication preserves that history; v4 executes only the unrun suffix. Unexecuted
v3's incorrect FileCheck-wording explanation is separately retained. No production parser change.

Final independent acceptance is complete; this local commit closes296. The loop is stopped under
the latest explicit user instruction.
Queue a separate path-sensitive classifier repair and required full checkpoint, but do not start it.
No array admission or broader storage-role qualification follows from these results.

## Failure, Recovery and Hand-Off

Never retry over existing output/logs. Preserve preparation versions, attempted/unrun obligations,
source/diagnostic differences and any counterexample. Stop dependent work on invalid-source, wrong
control output or identity drift. No production restoration should be needed. Raw source/IR/logs stay
under ignored build; durable record retains exact outcomes/limitations. Root completed final acceptance and formatting. This local commit closes the slice; the loop is
stopped under the explicit user request. No next-slice execution is authorized.
