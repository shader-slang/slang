# Legalize common wave-mask and selected-vector shuffle helpers

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct backend will lower the largest coherent subset of the leading
wave/reconvergence cluster through reusable typed operations. The measured subset contains 18 of
the 31 healthy-MVP first blockers: five scalar ballot helpers, ten selected Float32x2 shuffle
helpers, two selected Int32x2 all-equal helpers, and one ballot-popcount helper.

The compiler will classify the canonical CUDA-prelude helper and legalize compound vector or
ballot semantics into the provider's existing generic scalar operations plus structural vector
construction/extraction. No provider callback or ABI revision is planned: scalar ballot,
read-lane-at, all-equal, Boolean conjunction, integer population count, and vector construction are
already expressible by revision 27. Every workload that becomes correct at both O0 and O3 will
receive explicit direct regression lanes; later blockers remain measured failures.

## Progress

- [x] (2026-08-30) Committed Slice 135 as `edd1dfd76`; the fixed census records 31 healthy-MVP
  wave/reconvergence first blockers and no old-correct regression.
- [x] (2026-08-30) Grouped the cluster by exact final assembly and specialized signature. The
  ballot/shuffle/all-equal/popcount subset accounts for 18 first blockers and reuses established
  scalar provider semantics.
- [x] (2026-08-30) Added exact typed compound-helper classification and preflight requirements without fixture or
  source-name dispatch.
- [x] (2026-08-30) Emitted compound helpers by composing existing scalar operations and structural vector
  operations; add deterministic adjacent-negative and provider-observation coverage.
- [x] (2026-08-30) Reran the focused family and fixed 452-row census at O0/O3, promoted all newly correct
  workloads, refresh coverage/Pareto evidence and representative metrics, validate, self-review,
  and commit Slice 136.

## Surprises and Discoveries

- The leading cluster is not one operation. Ten rows share `_waveShuffleMultiple` over the same
  canonical Float32x2 signature, while ballot, reductions, active/converged masks, and
  reconvergence IR operations have distinct representations.
- Revision 27 already contains every scalar operation needed by the selected subset. Widening the
  provider's wave descriptors to vectors would duplicate legalization that the compiler can state
  using existing extraction, scalar operation, and construction callbacks.
- `__activemask()` is a distinct converged-mask semantic and LLVM 14 exposes no matching NVVM
  intrinsic in the isolated provider headers. It is deliberately outside this slice rather than
  approximated with a ballot or guessed symbol.
- Twelve of the 18 selected workloads become correct. Four advance to aggregate Void/out-parameter
  shuffle helpers and two advance to scalar masked reductions, confirming those are separate
  representation boundaries rather than missing overloads of the selected value recipe.

## Decision Log

- Decision: select the 18-row ballot, selected-vector shuffle/all-equal, and ballot-popcount family
  as Slice 136.
  Rationale: it covers most of the leading cluster with one reusable legalization boundary, while
  reductions, active masks, and reconvergence operations require different invariants.
  Date/author: 2026-08-30, Codex.
- Decision: decompose compound helpers in compiler lowering and query every resulting scalar
  descriptor before provider mutation.
  Rationale: the revision-27 interface already expresses the exact semantics. Compiler-owned
  composition keeps the isolated LLVM provider economical and makes capability discovery match
  emission.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The compiler now represents exact scalar ballot plus three compound helper recipes. One ordered
step table per compound operation supplies both preflight descriptors and emitted operations, so
capability discovery cannot drift from construction. Selected-vector shuffle and all-equal use
generic extraction/construction around the established scalar wave descriptors; ballot-popcount
composes established ballot and UInt32 population count. Builder ABI and the LLVM provider remain
unchanged at revision 27.

The focused 18-workload family reports 12 correct and six preflight failures in both direct modes.
The 12 correct workloads receive 24 direct lanes, and all 36 native/direct CUDA lanes pass. Four
remaining workloads expose aggregate out-parameter shuffle ABI and two expose wave reductions.
The selected regression prefix passes 405/405.

The fixed 452-row census reaches 276 correct at O0 and 272 at O3, exact +12/+12 gains with no
old-correct regression. Among 427 healthy MVP rows, O0/O3/both correctness is 275/270/267, or
64.4%/63.2%/62.5%. The wave/reconvergence cluster falls from 31 to 19; helper ABI (28) and
aggregate/pointer/layout transport (23) are now the leading measured clusters.

The three representative gates remain correct. Direct O3 PTX remains accepted by CUDA 12.9 for
SM70, SM80, and SM90; runtime remains on SM120. CUDA 13 and physical SM70/SM80/SM90 workers remain
infrastructure gaps.

Self-review inventory: the canonical helper-shape predicate survives as the shared producer
invariant; exact compound spellings/signatures survive because they are final linked CUDA IR; the
step table survives as the sole descriptor source for preflight and emission; compiler-side vector
composition survives because revision 27 already expresses every scalar and structural operation;
and the fake Boolean-binary result check survives because it models an already-supported generic
family. No fixture dispatch, syntax reconstruction, compatibility fallback, provider widening, or
downstream malformed-IR repair was added.

## Context and Current Pipeline

CUDA specialization leaves one-block linked `IRFunc` helpers whose sole ordinary instruction is a
final `IRGenericAsm` terminator. Slice 135 records exact diagnostics such as
`__ballot_sync($0, $1)`, `_waveShuffleMultiple($0, $1, $2)`,
`_waveAllEqualMultiple($0, $1)`, and `__popc(__ballot_sync($0, $1))`, together with their complete
specialized signatures.

Scalar UInt32/Int32/Float32 read-lane-at, scalar ballot, and scalar all-equal already travel through
`SlangNVVMBuilderValueOperationsAPI`. The structural interface can extract vector lanes and build a
result vector. The compiler collects exact typed operation requirements before creating a provider
module, then emits each reachable helper in dominance order.

## Scope and Non-Goals

In scope are exact UInt32-mask/Bool ballot; UInt32 ballot population count; selected two- through
four-lane 32-bit integer/Float32 read-lane-at helpers if their canonical signatures occur; and
selected two- through four-lane 32-bit integer/Float32 all-equal helpers returning Bool. Corpus
promotion is limited to full O0/O3 differential successes.

Out of scope are `__activemask`, wave reductions and prefixes, `waveMaskMatch`, synthesized
reconvergence behavior, quad operations with embedded lane expressions, 16-/64-bit shuffle
transport, atomics, and an ABI revision. If focused execution exposes one of those later blockers,
record it rather than broadening this slice.

## Architecture and Invariants

The accepted producer is exactly one non-entry linked helper with one block and one `IRGenericAsm`
ordinary instruction. Exact final assembly selects one compound semantic; the specialized result
and parameter types prove mask, value, lane, result, and arity. Fixture names and source syntax are
not inputs.

Vector shuffle extracts each canonical value lane, invokes the established scalar read-lane-at
descriptor with the unchanged mask and lane, and reconstructs the exact result type. Vector
all-equal invokes the established scalar all-equal descriptor per lane and combines predicates with
the generic Boolean operation. Ballot-popcount invokes exact scalar ballot and then the established
UInt32 population count. Preflight enumerates the same descriptors emission will use.

Unsupported widths, element types, lane counts, arities, results, and near-miss assembly spellings
must remain deterministic E52017 failures before provider mutation.

## Interfaces and Dependencies

Reuse builder ABI revision 27 unchanged. Extend only compiler-side classification/legalization,
the shared exact scalar catalog spelling for ballot where appropriate, fake-provider observation,
and focused real-provider tests. The isolated LLVM 14 provider should require no semantic change;
its existing scalar intrinsic and generic structural operations remain the execution authority.

## Milestones

First, encode the exact compound-helper contracts and enumerate their scalar capability
requirements. Second, implement typed composition using existing builder operations and add
focused positive/negative tests. Third, run the 18-row family in native, direct O0, and direct O3
modes and promote only full differential successes. Finally, regenerate the fixed census and
Pareto outputs, run the selected regression prefix and representative gates/SM assembly, format,
self-review, and commit all durable evidence.

## Validation and Acceptance

Acceptance requires Release host/provider builds, focused fake- and real-provider tests, the
selected NVVM prefix, all promoted lanes, the focused family, and the complete 452-row census.
Compare exact workload sets against Slice 135 and require zero old-correct regression. Record
compilation success, runtime mismatches, preflight/provider failures, healthy-MVP O0/O3/both
correctness, later blockers, and the updated Pareto counts. Representative gates must remain
correct and direct O3 PTX must assemble for SM70, SM80, and SM90.

## Failure and Recovery

All generated IR, logs, PTX, cubins, and raw census runs stay below ignored build directories and
are safe to rerun. If a compound recipe does not match native runtime behavior, remove its exact
admission and retain the evidence. Do not widen to a similar spelling, add fixture dispatch, or
repair malformed upstream IR in emission.

## Self-Review

Inventory every new classifier, recipe, required descriptor, and emission branch. For each, record
the canonical producer, exact contract, owning tests, and revert result. Confirm that capability
discovery enumerates every operation emitted, all types come from the specialized semantic
signature, vector structure uses existing generic callbacks, and no provider or compatibility
surface was added without necessity.

## Artifacts and Hand-Off

Commit the completed plan, implementation, promoted fixtures, post-slice census TSV/Pareto JSON,
and Slice-136 report. Keep raw focused/census output ignored. The report must distinguish first
blockers removed from newly correct workloads and retain the fixed coverage denominator,
representative gates, and infrastructure gaps.
