# Support FP64 masked-wave arithmetic

This ExecPlan follows `.agent/PLANS.md`. The NVVM loop explicitly requires this completed plan
and report to be committed with the slice. Parent owns acceptance and commit; worker owns edits.

## Purpose and Observable Result

Make valid double scalar/vector partitioned sum/product reductions and inclusive/exclusive
prefixes execute correctly through direct NVVM O0/O3, with independently expected GPU results.
Use precision beyond float32 and full, sparse and partial partitions with every lane checked.

## Progress

- [x] 2026-09-24: Read workflow/build skill and matched accepted 207 binary hashes at base
      `cdb5a654732183df67b5c6db834eee652723e690`; native Ubuntu, unchanged optimized environment.
- [x] 2026-09-24: Select bounded recipe/aggregate wave domain before implementation.
- [x] 2026-09-24: Final three fixtures pass NVRTC and reject both direct modes on reverted old emitter.
- [x] 2026-09-24: Implemented, formatted and rebuilt final FP64 admission/identity changes.
- [x] 2026-09-24: All final focused/runtime/unit/toolkit/selected-corpus/complex gates completed.
- [x] 2026-09-24: Exact preservation comparison, self-review and durable handoff completed.
- [x] 2026-09-24: Parent independently reviewed and accepted the targeted slice for local commit.

## Surprises and Discoveries

CUDA `WaveOpMin/Max::doOp` uses compare-select; existing NVVM recipes use numeric min/max.
NaN and signed-zero distinctions require a separate semantic decision, so arithmetic support
must not casually admit FP64 min/max. Sum/product semantics and identities are unambiguous.

## Decision Log

2026-09-24, worker: rank FP64 masked arithmetic first (two frozen sources, existing canonical
recipes/provider operations, no ABI change); reconvergence second (one identity, separate
control-flow semantics), helper KernelContext pointers third (two identities, separate storage
contract), FP8/BF16/prelude next (different numerical/library contracts). All complex cells already
compile/assemble; runtime bindings/oracles remain unavailable. Rolling 206/207/208 retains one
complex-driven slice. FP64 min/max is deferred if reconciling edge semantics cannot remain bounded.

## Outcomes and Retrospective

Accepted by the parent after independent implementation and evidence review. Four old runtime
cells are fixed and nine new cells pass, with no unexpected deltas. All 12 artifact hashes and
7 tested-source hashes match. Targeted acceptance advances cadence to 1; last full checkpoint 207.

## Context and Current Pipeline

`WaveMultiSum(double(lane) + 16777216.25, uint4(mask,0,0,0))` specializes in
`hlsl.meta.slang` to the valid canonical `GenericAsm` `_waveSum($1.x, $0)` with
`double(double,uint4)` signature. Vector overloads produce corresponding `Multiple` helpers.
`_resolveNVVMMaskedWaveScalarOperation` and `_resolveNVVMAggregateWaveOperation` validate exact
spellings/signatures, then `_initializeNVVMMaskedWaveScalarOperation` builds typed recipes.
Identity and aggregate-leaf admission currently reject double. Existing scalar arithmetic and
slice-207 indexed shuffle already admit double. `_emitNVVMMaskedWaveScalarValue` and aggregate
recursion can reuse those typed operations and bit-width-aware floating constants unchanged.

## Scope and Non-Goals

Only masked-wave identity/admission and homogeneous wave aggregate admission in
`source/slang/slang-emit-nvvm.cpp`, focused tests and evidence. No provider, operation catalog,
ABI, general type lowering, standard library, source parser, or semantic representation changes.
The shared aggregate wave recognizer also admits FP64 matrix reductions and explicit-mask
indexed matrix shuffles; focused coverage includes every component and partial participation.
New FP64 implicit matrix shuffles remain rejected pending correct active-mask semantics.
No pointer/reconvergence/FP8/BF16 feature. Retain lower-target slice-207 branches unchanged.

## Architecture and Invariants

Source helper signature is canonical and intentionally valid, not a producer accident.
Keep the exact finite spelling table and full type validation; reuse scalar recipe closure and
existing provider operation support checks. FP64 sum identity is positive zero; product identity
is exact IEEE-754 double one. Integer widths remain unchanged. Unsupported operators and malformed
masks/signatures must still reject before provider discovery. Aggregate shape remains homogeneous
vectors/fixed arrays, now allowing double leaves whose selected operation must independently pass
recipe/catalog support. No fallback is added. FP64 reductions additionally preserve source seed and singleton passthrough
semantics with existing typed operations; general 32-bit recipes remain unchanged.

## Interfaces and Dependencies

Provider ABI 35 and CUDA 12.9.2/NVRTC12.9.86/LLVM 14 unchanged. L4 SM89 driver 580.126.09,
runtime target SM80. Inspect/source `build/nvvm-loop/slice-203-env.sh` for optimized paths.
Use native tools, `CMAKE_BUILD_PARALLEL_LEVEL=1`, four build/corpus workers, two unit servers,
suites sequentially; bound every suite. Sandbox loopback failure requires authorized escalation.

## Milestones

1. Add final scalar/vector GPU fixture plus malformed FP64 mask/bitwise negative coverage; run
   before production edit and retain formatted source hashes.
2. Implement the recipe changes, format explicit files, build optimized compiler/test tools.
3. Run focused tests and runtime smoke before broad gates; inspect every executed/passed count.
4. Compare requested cell inventory and exact fields to accepted 207; retain historical failures.

## Validation and Acceptance

Targeted acceptance: frozen identity selection is `selection.slice-208-frozen.tsv` (107
identities, all wave/quad plus float64/double/helper/value/vector/matrix neighboring paths), all
three modes. Execute the complete discovery manifest (91 existing identities plus three additions) to
cover helper/control-flow/type neighbors without relying on tag-only name inference. Record all
fresh and inherited keys; other frozen cells inherit accepted full slice 207, never claim fresh.
Run all six registered complex cells, four runtime fixtures, NVVM/routing/reporter unit suite,
18 toolkit cells, focused new fixtures at NVRTC O3/NVVM O0/O3. Compare classification, return
code, full execution_counts, diagnostic and canonical_shape for every fresh old key. Expected
fixes: `wave-multi-sum-product` and `wave-multi-prefix-sum-product` at NVVM O0/O3 if no separate
blocker. Retain all 57 prior failure histories, recording exact fix transitions separately.
Full checkpoint is required if provider/library/catalog/ABI/general lowering changes occur,
impact becomes uncertain, or selected coverage reveals unexplained regression. Current full
cadence 0; accepted targeted 208 advances to 1; full208 resets0. Parent reviews domain decision.

Commands: use `cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test
render-test test-server`; `slang-test -use-test-server -server-count 2 -disable-retries` for focused
and units; workflow runtime/toolkit/complex commands with `slice-208-after` outputs; census uses
`--workload-ids-from issue-nvvm-backend/selection.slice-208-frozen.tsv --jobs 4`; discovery uses
full authoritative manifest with `--jobs 4`. Each suite is bounded by `timeout --kill-after=30s`.

## Failure and Recovery

Keep before/final evidence in separate directories. A new unsupported diagnostic alone is no
acceptance: final focused GPU oracles must execute. Stop GPU dispatch on device loss. Investigate
only enough to identify an independent next blocker. Revert/revise within scope for regressions;
never weaken or remove an oracle. Production changes after final gates require affected reruns.

## Artifacts and Hand-Off

Raw logs/scripts/IR/binaries live in ignored `build/nvvm-loop/slice-208-{before,after}`. Durable
plan, five-part report, manifest, selected/full inherited outcome TSVs, design note and STATUS
record exact source/binary identity, fresh/inherited counts, unresolved ledger and self-review.

## 2026-09-24 Focused Audit Update

The parent confirmed targeted domain and narrower arithmetic scope. Final initial arithmetic and
aggregate fixtures passed NVRTC and rejected canonical FP64 helper shapes before implementation.
The scalar/vector arithmetic fixture passed both direct modes after basic admission. Matrix
shuffle admission only covers Void/OutParam helpers; vector return-by-value shuffle uses the
separate 32-bit compound resolver and remains unsupported. The final matrix fixture now targets
only that admitted path, including implicit active masks and low16 partial masks.

The parent requested a signed-zero audit. Raw probe `slice-208-after/zero-probe.slang` demonstrated
NVRTC bitset 51 versus NVVM 16: full-mask scalar/vector and singleton sums lost negative zero;
product signs and prefix positive-zero identities agreed. This is a demonstrated new FP64
mismatch, unlike the deferred static min/max audit. Source `_waveReduceScalar/Multiple` uses a
caller-seeded butterfly for `_waveCalcPow2Offset(mask)>0`, no arithmetic for singleton masks,
and a positive-zero identity for other sums. The bounded correction reproduces the mask predicate
using typed uint addition/countbits/equality and Boolean conjunction, reusing existing negation,
bitwise AND, select and generic recipe emission. Negative zero is neutral for ordinary sums while
preserving all-negative-zero butterfly results. Singleton sum/product selects the original value,
including signaling-NaN payloads. No provider/library/catalog/general lowering change occurs.
Third independent edge fixture tests raw singleton bits, full/sparse/partial sum signs, product
and prefix signs, and NaN/infinity classification. Exact final fixtures require an old-emitter
revert run after this refinement, then restoration/build and all final gates.

Fixture development also found a pre-existing NVRTC source-output limitation: folding
`(1 + 2^-30) / 65536` emitted `0.00001525878907671`, losing significant digits. Raw pre-change
component probes isolate this. Final independently authored aggregate input instead uses
positive power-of-two scaling through `2^32`, retaining precision pressure without introducing
a different CUDA numeric-emission feature. Frozen sources/oracles remain untouched.

## Final Scope and Revert Milestone

The implicit-mask audit changed the provisional adjacent scope. Actual O0 PTX proves the existing
implicit path calls full-mask vote.sync.ballot; PTX 8.8 requires every non-exited named lane to
participate. A passing low16 branch therefore cannot validate the intended active-mask semantics.
New FP64 implicit matrix shuffle is now explicitly rejected at the wave resolver and covered by
a sixth negative case. Final positive matrix coverage uses explicit masks; ordinary vector-return
shuffle remains restricted separately. Parent approved this bounded constraint and final emitter.
The provisional fixture/PTX stay under build as excluded investigation evidence.

Final formatted fixtures were run with the production emitter reverse-patched to HEAD: 3/9 NVRTC
passes and 6/9 expected NVVM rejection. The exact emitter patch was restored and rebuilt. Every
final gate uses recorded source/binary hashes. Provider/catalog/ABI/library/general lowering stay
unchanged, so targeted 107 frozen/full 94 discovery remains the agreed acceptance domain.

## Final Validation Outcome

Final gates: focused 10/10 (nine GPU cells plus strict preflight unit), runtime 4/4, units 474/474
with one Windows-only skip, toolkit 18/18, and complex 6/6 compile/assembly. Frozen107 identities
produce321 fresh cells: 313 correct and 8 retainedpreflight. Full discovery 94 identities produce282 fresh
cells: 252 correct and 30 retainedknownfailures. Exact requested keys have no duplicates/omissions.
All five comparison fields match 207 except four expected old sum/product/prefix fixes. Nine
additions pass separately. The other1035 frozen cells explicitly inherit accepted 207. Cumulative
preservation ledger: 1638 cells, 1585 correct, 53 unresolved; all 57 old failure histories survive as53 open
and 4 resolved transitions. This is targeted acceptance, not a fresh full checkpoint.

Final source/artifact hashes match after all gates. Revert build reproduces accepted 207 compiler
hash exactly; every final fixture is byte-identical to the old-emitter run. Parent approved domain
and reviewed final identity graph. All raw artifacts remain in build; durable manifest/outcome TSVs
carry exact provenance and failure history. No commit or push performed by worker.
