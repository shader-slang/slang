# Qualify canonical AST subtype-check optimization

This ExecPlan follows `.agent/PLANS.md`. NVVM workflow requires the completed plan/report committed
by the parent; raw measurements and prototype sources remain under ignored `build/`.

## Purpose and Observable Result

Reduce intact tiled-brass material compile time without changing any shader output or AST casting
contract. Compare minimal predicate inlining with direct canonical NodeBase tags. Promote only the
smallest implementation meeting predeclared semantic and timing gates; otherwise restore production
source and retain a research-only result. No material runtime claim is possible without its bindings,
texture/LUT inputs and expected-output contract.

## Progress

- [x] 2026-09-25: Read repository/workflow, accepted244/245, build skill and AST producer/metadata.
- [x] 2026-09-25: Baseline43/12/561 matches244; complete compiler layout and loader trace saved.
      Semantic before run passes1052 executed tests with77 ignored; complete per-test log retained.
- [x] 2026-09-25: Formatted inline-only build succeeds; exact body/source review passes.
      Final proof covers702 tags/492804 pairs/636 concrete/66 abstract with zero failures.
      Full paired timing passes:264 exact PTX outputs/24 exact cubins, semantic18.43–25.56% lower,
      aggregate wall9.43% lower; sample/NVVM O3 pooled wall0.066% slower retained as a limitation.
      Direct-tag B deferred per parent decision because minimal A passes.
- [x] Parent reviews exact relocation and final proof source; all final gates pass on one stable
      117-source/12-artifact identity. Frozen/discovery preserve all 1695 old five-field outcomes.
- [x] Complete compact evidence, five-part report, durable design note and draft STATUS.
      Raw evidence and final source snapshots are indexed; checkout released and independently accepted by parent.

## Surprises and Discoveries

Final durable proof initially assumed `as<DeclRefBase>(DeclRefBase*)` was allowed. Its compile-time
assertion failed because existing `Slang::IsBaseOf` excludes the identical type. Both relevant headers
are unchanged. The corrected proof preserves that existing deleted-overload restriction; the failed
compile and initial wrapper remain visible under after/proof-final. No compiler fix or baseline reset.

Existing casts do an out-of-line tag-to-metadata lookup followed by an out-of-line constant-time
unsigned interval comparison. This is already a generated contiguous hierarchy, not a traversal.
`ASTBuilder::_initAndAdd` calls `node->init(T::kType, this)` before testing/using the node. Default
NodeBase tag -1 is invalid; the table constructor asserts both bounds in Debug. Default class
metadata is null and its subtype test returns false. Fiddle includes abstract classes in its tags.

## Decision Log

- 2026-09-25, worker: Compare A (move unchanged predicate inline) and B (shared existing range test
  using canonical NodeBase tag directly). Do not add hierarchy/cache/semantic equivalence or patch
  AST producers. Retain original SyntaxClass default/null behavior and DeclRefBase restrictions.
- 2026-09-25, worker: Save complete matching bin/lib layout for timing comparisons, with loader
  path checks; copying slangc alone would load the subsequently rebuilt compiler shared library.
- 2026-09-25, parent: Screen A first. If exact relocation passes the full semantic and paired
  timing gates, select it as the minimal implementation and defer B. Only build the more invasive
  direct-tag path if A fails timing; an additional rebuild solely for extra speed is unnecessary.
- 2026-09-25, parent: Exact production relocation and final durable proof source approved.
  The successful formatted inline build is the final candidate; no redundant compiler rebuild.
- 2026-09-25, worker: No full corpus before prototype passes. Parent reviews source before final
  selected build. One worker mutates; parent accepts/commits. No push or system changes.

## Outcomes and Retrospective

Minimal exact predicate relocation passes the predeclared performance gate and exhaustive semantics.
Full compiler units1045/13 skips, semantic regressions1052/77 skips, toolkit18 and contracts6 pass;
frozen/discovery retain 1695 cells, 1654 correct, 41 unresolved and all 16 resolved histories;
all six material compile/assembly cells pass with exact historical PTX/cubin bytes. A copied-baseline generator-path skip is
independently explained and passes a focused old-compiler replay; no optimization-caused capability
claim. Full246 is independently accepted; targeted233 remains latest targeted acceptance and cadence is zero.

## Context and Current Pipeline

Material `make_surface_interaction` normalizes `wi_ws` and calls `dot`; surrounding overload/generic
calls flow through `checkAllTranslationUnits`, `ResolveInvoke`, overload candidate constraints,
`GenericArgumentSolver::solve`, `TryJoinTypes`, `as<DeclRefType>` and `dynamicCast`.
ASTBuilder initializes canonical `astNodeType`; `getClass()` maps it through `kAllSyntaxClasses`;
`isSubClassOf` reads its firstTag back and compares unsigned distance to target tagCount.
Fiddle's generated class metadata owns this interval. Research245 reports semantic checking medians
651–655ms and qualitative debugger leaves at both operations; no speedup is assumed.

## Scope and Non-Goals

Only this shared AST cast boundary, necessary focused tests and evidence. No generic-solving,
shader/material edits, backend ABI/admission, new hierarchy or out-of-contract naked downcasts.
Preserve all245 evidence (528 indexed raw artifacts,15 snapshots,4 parent audit artifacts).

## Architecture and Invariants

All registered tag × target pairs, including abstract targets, must agree with both independently
constructed C++ inheritance truth and the original range predicate. Real ASTBuilder-created objects
exercise casts: no fake NodeBase with a forged derived tag/downcast. Preserve null and const
casts, default class metadata, invalid tag assertions (-1 and CountOf), DeclRefBase restrictions,
and serialization. For A, source audit preserves the unchanged constructor assertion contract for
-1 and CountOf; never execute invalid tags in optimized assertion-disabled code or mix Debug AST
headers with Release object layouts. A direct-tag variant would require a consistent Debug check.
Range metadata remains sole production hierarchy truth. Document every new helper
and audit its valid producer-owned input before retaining it.

## Interfaces and Dependencies

Native Ubuntu, nvvm-backend base c29b9b7158ab069141476761f5585c26d3cf7460, optimized native
releaseWithDebugInfo preset. Accepted source244 at8d53504112617efda0e3446f7b3117bcc2f77fd2,
245 docs-only. CUDA12.9.2, providerABI40, L4SM89 targetSM80. Inspect/source
`build/nvvm-loop/slice-203-env.sh`; explicit RelWithDebInfo tools, maximum4 CPUs total, unit servers2,
sequential GPU suites and30-minute bounds. Build:
`CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
Use local slang-build skill, no intermediate full-build polling. Formatter uses setup tools PATH and
explicit changed files; no shader fixture/history formatting.

## Milestones

1. Create separate slice-246-before, slice-246-prototype-inline, slice-246-prototype-direct and
   slice-246-after roots. Verify43 source/12 artifact/561 input hashes against244. Extend source
   identity with AST producer, generated metadata/Fiddle and tests. Save baseline layout and loader
   evidence. Establish non-NVVM regression before results using old binaries.
2. Implement/test A; build B only if A fails the gate, saving exact identities independently. Initial
   screening uses all6 identities; full paired experiment uses2 opposite-order rounds,2warmups+
   9samples each cell/build/round, piped Popen.communicate timed to completion, serial with no competing
   build/benchmark. Notify parent before/after timing. Preserve every output and attempt.
3. Parent reviews proposed selected diff and early evidence before expensive final build/gates.
4. Final accepted candidate requires gates below after formatting; rejected candidate restores source
   and matching baseline binaries, retaining honest rejection evidence instead.

## Validation and Acceptance

Performance: each identity >=5% lower SemanticChecking median; sum of6 wall medians >=2% lower
(18 measured samples each); no identity wall regression >2% in either order round. Exact PTX and
cubin equality versus244/245 before accepting timings. Fixed245 compile commands/options/input,
optimized matched builds and precise communicate timer; retain all logs and outputs.

Before promotion: all-tag/all-target independent hierarchy plus old-predicate proof, null/default/
const/invalid/DeclRef restrictions/serialized AST coverage; relevant overload/generic/constraint/
diagnostic/serialization regression and compiler units (known infrastructure recorded exactly).
After final selected source: small GPU runtime4 first; focused unit/equivalence; NVVM/routing/reporter
484 plus math27 =511 passes/1 existing Windows skip; fuller compiler units/non-NVVM regressions;
toolkit18; runner contracts6; frozen452/1356 with explicit
`--workload-ids-from issue-nvvm-backend/census.slice-195.tsv`; discovery113/339;6 material assemblies;
final paired timing on exact stable source. Capture source/12artifact identity before every gate and
after. Exact1695 old5fields,1654 correct/41 unresolved/16 resolved histories,561 old input hashes must
survive. No additions expected. Historical source snapshots remain immutable even if live headers
change; do not require live headers to match historical snapshots.

## Failure and Recovery

Reject if either semantics or robust performance fails. Preserve source patches, binary identities,
all failed/timed-out observations and raw outputs; restore owned production source from saved baseline
(no broad git reset), restore/rebuild baseline matching artifacts, and report research-only results.
Do not change baseline or remove failures. Stop device loss immediately with no GPU retries. Stop
before any unrelated independent feature. Repeated timing is only justified by source/protocol changes,
not selecting favorable observations.

## Artifacts and Hand-Off

Plan, five-part `report.slice-246-ast-subtype.md`, compact runtime-validation (if promoted) or research
evidence (if rejected), timing comparison, durable material design note and draft STATUS. Raw scripts,
source variants, binary layouts, all logs/outputs and indexed evidence in unique ignored246roots.
Report exact fresh versus inherited controls. Parent receives <=500word handoff and explicit checkout
release. Independent parent acceptance establishes full checkpoint246/cadence0. No worker commit.

Parent acceptance independently verifies exact production-body relocation, all264 timing compiles and
24 assemblies, all distributions and gates,1695 exact corpus outcomes and complete failure histories,
1058 unit and1129 semantic identities,117/12/561 live hashes,117 snapshots and2830 indexed raw files.
It verifies918 references including historical245 before its six own audit references. The accepted
paired timing limitation remains explicit. Six parent audit files are linked by validation246.
