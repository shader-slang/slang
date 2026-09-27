# Qualify inheritance, initialization and scoped constants on CUDA

This bounded ExecPlan follows `.agent/PLANS.md` and the maintainer's committed-plan exception for
NVVM slices. The user explicitly resumed the development loop; skip Slack, no push/system changes.
Root owns scope, execution, acceptance and commits. Fresh language_surface295 owns raw preparation;
separate review checks inputs/oracles/runner before execution and final evidence before acceptance.

## Purpose and Observable Result

Qualify four unchanged authored compute contracts outside the main corpus at NVRTC O3 and NVVM
O0/O3. Demonstrate exact complete GPU outputs for inherited members/base conversion, inherited
aggregate initialization, mixed-width default initialization and scoped constants used in array
indexing. Keep outcomes bounded to these inputs; do not infer a language-feature coverage percentage.

## Progress

- [x] 2026-09-27: Read WORKFLOW/STATUS;294 accepted/committed c0ddac70b and exact accepted installation
      restored. Fresh worker's raw proposal verifies four candidates outside main576/prior focused sets.
- [x] Root selected the four unchanged sources and independent oracles below before preparation.
- [x] Before-identity check passed100layout/37runtime/11source/2config/576inputs/22pins at c0ddac70b.
- [x] Prepared-v2 freezes12mode mirrors/sidecars,58refs, exactnativeIDs and independent48words.
      Root and independent pre-execution review passed. Unexecuted v1 retained.
- [x] Root ran12serialized cells; all passed48words in19.71s, no skips/failures/unrun/retries.
      Independent outcome review passed; before/after identity maps remain exact.
- [x] Independent final review accepted research/identity with no findings; final formatting
      and research-only local commit close this slice.

## Context and Current Pipeline

Historical inventory269 selected inheritance0/4, initializer-lists0/3 and constants2/18 compute files.
These are source-selection counts, not passing semantic coverage. Prior271/278 focused tests and
287–292 dynamic/error work do not include these four sources. Material CPU study294 measured a
modest instrumented OR bucket with substantial whole-observer overhead; no safe pruning follows.
Return to an independently verifiable language slice without speculative compiler changes.

The native compute harness compiles source entry points, binds authored inputs, launches CUDA and
compares output. Maintained run-compute-discovery.py selects the native directive and adapts only
API/mode flags; shared census helpers mirror bodies/sidecars and classify exact native execution.
No AST/IR producer or consumer is changed; these existing language shapes are intentionally valid.

## Scope, Oracles and Non-Goals

All paths below are under tests/language-feature; each selects native ordinal0 and preserves its
original `.slang.expected.txt` sidecar. Output is four hexadecimal uint words in every mode.

| Source                                           | Expected hexadecimal words | Source-derived proof                                                        |
| ------------------------------------------------ | -------------------------- | --------------------------------------------------------------------------- |
| inheritance/struct-inheritance.slang             | 1113,1002,1335,1224        | cbuffer x=1,y=2 and lane v yield4096+273*(v^1)+2                            |
| inheritance/derived-struct-init-list.slang       | 11201,11212,11223,11234    | defaults produce0x112; explicit y={v,v+1} yields0x11200+17v+1               |
| initializer-lists/default-init-16bit-types.slang | 0,1111,2222,3333           | zero-init int/int16/half/int fields then weighted lane additions yield4369v |
| constants/static-const-in-struct.slang           | 0,1111,2222,3333           | method/global array indexing each yields17v; combine256*(17v)+17v           |

Only the first reads host constant-buffer values; all four read dispatch IDs0–3. Default construction,
array fills and calls may fold. No retained-instruction, aggregate layout/ABI, pointer aliasing,
existential dispatch, overflow, nonintegral-half or arbitrary inheritance-depth claim. No compiler,
provider, runner, authored shader, oracle, manifest or input change. No material/performance study.
The mixed-width/constants lane0 output starts at its expected0; a buffer match alone does not
prove that lane wrote. Other three lanes plus actual native execution remain required.
These12 focused cells remain outside main580cases/576sources/1740cells and discovery capacity128.

## Architecture, Invariants and Dependencies

Use the maintained directive enumerator and source_without_test_directives, preserving original
command/categories, shader-object flags and the mixed-width test's `-render-feature int16` gate.
Only target/mode directives change. Bodies, authored TEST_INPUT declarations and all four sidecars
must remain byte-identical after normalization of test directives. No imports/includes are present.

Fresh raw root: build/nvvm-language-surface295. Build is unnecessary: installed accepted compiler
62469125/version301-g8fbf0f84e, provider ABI42/af1661de, source8fbf0f84e+patch12f503e9, full293 validation.
Verify100layout/37runtime/11qualifiedsource/2config/576inputs/22pins before and after. CUDA12.9.2,
NVRTC12.9.86, native L4SM89/driver580.126.09, targetSM80. A future build must refresh version metadata
and rebuild restored294 sources; this slice does not build. Reuse existing owned-process cleanup.

Each shader process has180s limit; outer gate1800s. One worker, serial modes, no retries or competing
GPU/build/suite work. Use repository-relative selectors consistently with native NoRoot normalization.
Treat feature gating/skips, zero execution, missing outputs, crashes, timeouts and mismatches as failures.

## Milestones and Acceptance

1. Author prepares only raw scripts/mirrors and freezes exact12(id,mode) rows, all original/adapted
   directives, body/source/sidecar hashes, independent expected words, runner/helper/provenance hashes.
   Reuse maintained adaptation and compare all source bodies/sidecars before execution. Root verifies
   baseline identities; reviewer independently derives four oracles and approves exact preparation.
2. Root runs the single bounded frozen gate, using maintained census execution or equivalently the
   already-qualified owned runner with native commands. Require one result per requested cell,
   return0, passed/executed1/1, ignored0, empty diagnostic/shape and exact full four-word output.
   Preserve original native logs and output artifacts. No old `.actual` reuse.
3. Verify unchanged identities and12-cell/48-word inventory; independent final review distinguishes
   fresh focused evidence from inherited full293. No broad rerun is needed with no source/toolchain/
   configuration/corpus-runner changes. A real regression blocks further feature work and triggers
   a separate bounded investigation; do not silently repair/widen this qualification slice.
4. Commit compact report/record/completed plan/navigation only. Full293/targeted233/cadence0 and
   1703correct/37unresolved/20resolved histories stay inherited. Select next slice from evidence.

## Surprises and Discoveries

Proposal rejected dedicated field-initializer and examined type tests as weaker literal-only
runtime probes. The mixed-width gate is retained; CUDA RHI advertises Int16 on this device by source
inspection, but only actual executed cells establish qualification. Selection retained one harmless
read-only filename typo; no workload has run. Root review rejected draft custom process cleanup
that could miss detached children after leader exit; v2 reuses proven owned_process.py, checks exact
native identities, and drops inherited observer/preload variables. V1 is frozen and unexecuted.

## Decision Log

- 2026-09-27, root: Choose four existing runtime-observable contracts and their original sidecars,
  rather than inventing new shader semantics. Preserve compiler and corpus; qualify precise outputs.
- 2026-09-27, root: No IR survival claim. Host-input inheritance is strongest; lane-dependent cases
  still test semantics across four values while permitting legal optimization of defaults/constants.

## Outcomes and Retrospective

All12cells/48words pass with exact native identity/counts and unchanged source bodies/sidecars.
Independent review confirms raw outputs and58frozenrefs. No compiler defect emerges. Before/after
100layout/37runtime/11source/2config/576inputs/22pins match; full293/targeted233/cadence0 remains
inherited. Final documentation/ledger review accepted with no findings; commit-ready.

## Failure, Recovery and Hand-Off

Preserve every attempted and unrun obligation and version any runner/preparation correction; never
turn a zero-execution or gated test into a pass. Stop dependent work on identity drift or incorrect
output. No restoration should be needed because no installed/source bytes change. Raw mirrors,
logs and repeated details stay under ignored build; checked-in research retains outcomes/provenance
and failure histories. Next command follows reviewed raw preparation; root alone executes workloads.
