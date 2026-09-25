# Expose the existing AST class lookup to inlining

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer requires this completed plan and
report in the slice commit; the parent owns independent acceptance and commits. Worker does not commit.

## Purpose and Observable Result

Test one hypothesis from research251: an inline `SyntaxClassBase(ASTNodeType)` constructor reduces
intact tiled-brass material compile time while retaining every AST metadata and cast contract.
Promotion requires the exact gates below. A failed experiment leaves production source restored and
records negative evidence; it does not authorize a second optimization.

## Progress

- [x] 2026-09-25: Read AGENTS, plan standard, workflow/status, research251, implementation246,
      current250 validation, design and local native slang-build skill. Confirm clean accepted251 base
      `0c460441c0d539ad2f09f3139ce620a8e6a12dd3`, native Linux branch nvvm-backend.
- [x] 2026-09-25: Declare this plan before prototype or production edits.
- [x] 2026-09-25: Capture exact 250 identity and complete isolated layout; baseline proofs pass
      in matching Debug/optimized configurations. Both invalid Debug tags assert as intended. Full
      units 1047 pass/13 ignore (1060 IDs), semantic 1052 pass/77 ignore (1129 IDs); all 514 relevant250
      unit outcomes match. Exact 702 metadata records across baseline configurations.
- [x] 2026-09-25: Implement and format the minimal two-file constructor prototype.
- [x] 2026-09-25: Both matching candidate builds passed.
- [x] 2026-09-25: Small runtime 4 and both exhaustive candidate proofs passed, including
      specific Debug -1/702 assertions. All 702 metadata records exact across all four proofs.
      Hidden/local table symbol 5616 bytes, no export delta, unchanged NatVis.
- [x] 2026-09-25: Both isolated loaders verified their own libraries/modules/providers.
      All 264 compiles/24 assemblies exact 251; all gates pass: semantic 16.86–18.89% lower,
      summed wall7.35527% lower, all 12 per-round wall changes improve. Full checkpoint started.
- [x] 2026-09-25: Performance passed; full checkpoint preserves all 1701 cells/1662 correct/39
      unresolved and 18 resolved histories. Exact 1060 unit/1129 semantic outcomes, toolkit 18,
      contracts 6 and six final exact material compiles/assemblies pass. Retain minimal prototype.
- [x] 2026-09-25: Final identities, five-part report, compact timing/runtime evidence,
      design and STATUS draft completed; worker releases checkout for independent parent acceptance.

- [x] 2026-09-25: Parent independently accepted all performance/correctness evidence,120 snapshots
      and4224 indexed artifacts; full checkpoint252 and cadence0 are authoritative.

## Surprises and Discoveries

- The proof example docstring is corrected after timing to name the final header assertion source;
  its exact one-string replacement and hashes are retained. Compiler sources/binaries/inputs stay
  measured identity. Before/candidate generated proof source hashes differ only by include ordering
  and comment wrapping, retained as a diff; metadata records and executable checks are unchanged.
- Post-timing loader-oracle audit initially omitted `_buffer` from the identity suffix and raised
  StopIteration. Retained the failure and corrected to the exact 251 identity; no benchmark rerun.

- Initial pre-timing audit rejected a generated-file byte difference. It is exactly the +7 shift
  in FIDDLE source-line macro names after the header insertion; v2 proves that exact transform.
  Generated metadata and enum bytes remain identical. Initial audit/script/log retained; no
  production adjustment or timing retry.

- Before identity is exact 250:119 source,12 artifacts,563 inputs. Complete isolated layout and
  its `generators/layout/bin/slang-capability-generator` copied before tests. Debug requires a
  full 462-step rebuild; no mixed-layout shortcut is used.
- Slang assertions throw InternalError in this build. The proof prints its actual Message and
  rethrows so the child terminates normally with SIGABRT; acceptance also checks the bounds text
  and explicitly configured expected source, not merely an unrelated abort.

The sandbox fails before process launch (`bwrap` loopback setup); use approved native execution.
The table name is referenced by six NatVis conditions. Preserve namespace/name and those consumers.

## Decision Log

- 2026-09-25, worker: Use incomplete `extern SyntaxClassInfo const* kAllSyntaxClasses[]` in the
  internal header, a deduced generated definition and independent `static_assert` on its count.
  This avoids a fixed-size declaration silently zero-filling too few initializers. Preserve name,
  namespace, pointer mutability and sole generated initializer. Use CountOf for the equivalent
  inline bound. Review actual ELF visibility/exports before promotion.
- 2026-09-25, worker: Prove performance before the expensive full GPU corpus; a rejected prototype
  cannot be accepted as an implementation merely because it passes correctness.

## Outcomes and Retrospective

The minimal constructor-visibility change passes every predeclared gate. Semantic medians improve
16.86–18.89%, summed wall medians7.35527%, with all 12 per-round cell wall comparisons improving.
All 264 compile outputs/24 support cubins and final 6 material support cells are exact 250/251. Full
runtime1701 cells preserves1662 correct/39 unresolved and 18 resolved histories; all shared-AST unit,
semantic, toolkit and runner gates pass. The one generated-table representation remains canonical.

The failed initial generated-byte audit taught that source-line macros legitimately shift+7; its
exact correction changed no production code. All samples remain; observed phase variability is
unattributed. A proof-driver help-example correction after timing is doc-only and separately hashed.
All compiler/generated sources, artifacts and 563 runtime inputs remain the measured candidate.
120 final snapshots include the119 tracked source/test/generated paths plus unchanged NatVis.

Independent parent acceptance is complete. Full checkpoint252, latest targeted233 and cadence0
are authoritative. The parent independently recomputed all timing statistics and verified exact
corpus/unit/semantic outcomes, failure histories, four proofs, identities,120 snapshots and4224
indexed artifacts. Local commit includes the completed plan and report. No worker commit/push or
material runtime claim; the next bounded research is selected in STATUS.

## Context and Current Pipeline

The intact material helper initializes SurfaceInteraction and calls `normalize(wi_ws)`,
`dot(si.wi_ws, si.normal_ws)` and `Frame::identity()`. Generic/overload checking reaches
`inferGenericArguments -> as<CallableDecl>(genericDecl.inner) -> NodeBase::getClass ->
SyntaxClassBase(ASTNodeType)`. `ASTBuilder::_initAndAdd -> NodeBase::init` already stores generated
`T::kType`. The constructor bounds-checks and maps that canonical tag through the one Fiddle-generated
`kAllSyntaxClasses` table. The existing inlined predicate consumes class intervals. No malformed AST,
producer repair or source-syntax reconstruction is indicated.251 has qualitative evidence, not speedup.

## Scope and Non-Goals

Only constructor visibility/internal table declaration/count proof. Preserve predicate, all casts,
metadata pointers/order/factories/destructors, null/default and invalid policies, serialized AST and
NatVis. No direct-tag cast/dispatcher changes, alternate hierarchy, allocator/cache, generic solver,
provider/API/ABI, fixture/oracle, material or runtime semantics change. Do not bundle another
optimization if this hypothesis fails. No material runtime/kernel-speed claim without its contract.

## Architecture and Invariants

The generated table remains the only tag-to-metadata mapping. Its deduced size equals CountOf at
compile time. Every valid tag returns exactly `&T::kSyntaxClassInfo`; node tags remain canonical.
Factories/destructors retain their existing function pointers and abstract null policy. No new public
export or public header. Invalid tags -1 and CountOf must assert with matching Debug objects; never
execute them in optimized builds or mix Debug/Release AST layouts.

## Interfaces and Dependencies

Internal source/slang/slang-ast-support-types.h and slang-ast-boilerplate.cpp; one shared internal
array declaration and unchanged generated definition. Native Ubuntu24.04, L4SM89 driver580.126.09,
CUDA12.9.2/NVRTC12.9.86, isolated LLVM14.0.6, targetSM80/providerABI41. Read/source 203 environment
which overrides202 Debug paths. Max4 active CPUs, build parallel4, unit servers2, corpus workers4,
sequential GPU suites,180-second timed child/30-minute suite bounds. Complete isolated bin/lib
layouts include sibling CapabilityGenerator so unit selection is unchanged.

## Milestones

1. Save before source/artifact/input hashes and snapshots, complete bin/lib layout, commands,
   ELF sections/exports and environment. Reuse246 test inventories and compare to fresh before where
   needed. Build matching baseline Debug compiler using native `cmake --build --preset debug`.
2. Extend `check-ast-subtype.py`/`ast-subtype-proof.cpp.in` meaningfully for exact typed metadata pointer,
   create/destruct pointer and count identity. Run on matching baseline/candidate objects; retain
   source audit, metadata comparisons and hidden symbol review. Invalid cases use matching Debug.
3. Move constructor body inline, remove old definition, declare shared table and assert generated
   count. Format explicit paths before final build/tests/timing. Optimized build command:
   `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
4. Run small runtime and proofs. Copy complete candidate layout and verify actual loaded compiler,
   builtin module and provider. Run paired measurement, then apply predeclared gates without retries.
5. If passed, run full units, non-NVVM regressions, relevant NVVM/routing/reporter/math, toolkit 18,
   contracts 6, full frozen census195 and discovery 115 in all 3 modes and all 6 complex support cells.
6. Otherwise restore exact production sources, rebuild optimized artifacts, prove restored identity,
   preserve negative research and hand off without full candidate acceptance or cadence advancement.

## Validation and Acceptance

Exhaustive702 tags/492804 pairs/66 abstract/636typedobjects; all four cast overloads, null/const/default,
strict DeclRefBase restrictions. Extend pointer/count/factory/destructor proof; preserve source
assertions and verify -1/CountOf abort in matching Debug baseline/candidate. Full compiler unit
identities (246 had1058; preserve all new units) and all 1129 semantic identities for generics, overloads,
operator overloads, diagnostics and serialization. Current relevant units 513 pass/1 skip including
`doubleSourceLiteralsRoundTrip`. Full corpus exact requested inventory1701 cells/1662correct/39unresolved,
18resolved histories; frozen 1356/1347 and discovery 345/315 (115 sources). Compare classification,
return code, execution counts, diagnostic and canonical shape, with no missing/duplicates. Current250
is authoritative. All 6 intact material PTX/cubins must remain byte-identical.

Paired optimized before/candidate builds: two rounds reverse identity AND build order, each with
2 warmups+9 samples per6 cells/build/round =264 compiles/216measured/48 warmups. Piped
Popen.communicate through actual exit; logs outside timer. Retain every sample, no timing retries or
outlier exclusion, no competing build/benchmark/GPU suite. All 24 support assemblies exact; no assembly
performance claim. Require every pooled SemanticChecking median >=5% lower, SUM six pooled wall
medians >=2% lower, and no cell wall regression >2% in EITHER round. Inspect phase dispersion and
actual ELF `.text` size. Final tested source must equal measured candidate for promotion.

## Failure and Recovery

Retain failed attempts, exact commands and sources in fresh raw roots. Fix harness mistakes with a
new evidence directory; never erase prior results. A failed performance/semantic gate rejects the
minimal prototype: restore only worker production edits, rebuild baseline, preserve all negative
findings. Do not run full corpus on a performance-rejected candidate. A GPU/device loss stops GPU
runs; no reboot/driver/system changes/push. Missing material semantics remain a limitation, not an
invitation to invent an oracle. Any unresolved regression blocks acceptance and is handed off.

## Artifacts and Hand-Off

Fresh roots `build/nvvm-loop/slice-252-before`, `slice-252-prototype`, `slice-252-after`. Preserve
scripts, logs, source/binary/input snapshots and complete raw hash index. Durable completed plan,
five-part report, timing/runtime compact evidence with fresh/inherited distinction, design update
and accepted STATUS are complete. Record every new helper/fallback/special case and
input-shape audit. End handoff with RELEASE CHECKOUT; parent independently reviews and commits.
