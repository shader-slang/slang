# Measure inline visibility of the current AST builder getter

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM plans/reports
in local commits. The development loop is authorized. Delegation remains unavailable after the
agent-thread limit; one local writer uses separate statistical, source and identity audits.

## Purpose and Observable Result

Determine whether exposing the existing getCurrentASTBuilder body reduces intact tiled-brass
compilation while preserving its single per-thread pointer. Promote only if the predeclared gates
pass. Otherwise restore the exact accepted source and retain the negative experiment as research.
No material kernel runtime claim is possible without its missing application contracts.

## Progress

- [x] 2026-09-25: Read WORKFLOW/STATUS, research257, build skill and prior paired252 driver.
- [x] Declare this plan before compiler edits. Base c1955155467403566171e4655cec3692b62ecbf6;
      accepted compiler/provider/runtime identities remain256.
- [x] Capture baseline133 source/12 artifact/565 input identities and immutable optimized layout.
      All match256; copied layout has80 files with independent contents and preserved symlinks.
- [x] Expose only getter and explicit constant initialization; matching optimized build passed.
- [x] One local TLS definition, unchanged public exports, no initialization guard and direct TLS
      access in Val::resolve verified. Isolated loader checks precede paired timing.
- [x] Paired264 compiles/24 assemblies preserve exact outputs. Independent audit passes.
      Semantic gains1.6512–2.4123%, summed wall0.66549%: promotion thresholds fail.
- [x] Conditional promotion checks are not run: performance failed. No candidate correctness
      or runtime-support claim; restore accepted sources/artifacts instead.
- [x] Discard and restore source/artifacts; independent acceptance passes. Evidence closure and
      five-part report/design/STATUS are complete for the local research commit.

## Surprises and Discoveries

Default sandbox network setup fails; native commands use approved escalation. No system setting
changes are authorized. The first identity audit needed an absolute-toolkit-path correction; its
rollback guard prevented source changes, but the non-fail-fast shell ran a retained no-op build.
Corrected audit passes and actual rollback is fail-fast. No measured attempt was repeated.
Research257 has ten getter-path snapshots across three profiles, which justifies an experiment
but gives no reliable estimate of attainable speedup.

## Decision Log

- 2026-09-25, parent: Select the sole research257 getter-visibility hypothesis. Keep setter, TLS
  model and resolution logic unchanged. A C++20 extern thread_local constinit declaration makes
  existing constant null initialization explicit across translation units without a new TLS guard.
- Qualify optimized performance before expensive full correctness suites. Failed performance
  causes exact rollback, making candidate semantic promotion proofs unnecessary; source/binary
  restoration and existing accepted256 identities must still be verified.

## Outcomes and Retrospective

The prototype is discarded after missing both improvement thresholds. All264 compiles and24
assemblies preserve outputs, but no production optimization is retained. Exact source restoration
and artifact restoration are complete. Optimized rebuild reproduces compiler/test libraries;
the regenerated builtin cache differs (timestamp plus8 payload bytes), so restore the saved cache
and original identical compiler file timestamp together. Fresh loader control reads it unchanged
and preserves PTX. Final133 source/12 artifact/565 input hashes and18 submodule pins match256.

Latest implementation/full checkpoint256, targeted233, cadence0 and rolling
252/254/256 remain authoritative until a candidate passes every required gate.

## Context and Current Pipeline

Tiled-brass eval_buffer/sample_buffer use generic normalize/dot and many overloaded calls.
checkTranslationUnit installs its linkage ASTBuilder with SetASTBuilderContextRAII. Canonical
Type::getCanonicalType and Val::equals reach Val::resolve, which gets the active builder before
checking its cached epoch. The getter currently calls the platform TLS accessor and returns the
single gCurrentASTBuilder pointer. It is valid canonical context, not a representation defect.
Reflection allocation builders cannot substitute for this checking context. The candidate only
makes the existing getter body visible to its callers.

## Scope and Non-Goals

Only source/slang/slang-ast-builder.h and .cpp may change in the prototype. Keep one shared TLS
storage definition, inline existing getter, and explicitly constant-initialize it. No new helper,
cache, fallback, resolver/substitution/class hierarchy change, public export, compiler flag or TLS
model. No per-TU static storage or assumption of early shared-library loading. No fixture/oracle,
provider ABI41, corpus manifest or shader option changes. No driver, permission, reboot or push.

## Architecture and Invariants

Preserve initial nullptr per thread, independent thread state, cross-translation-unit identity,
setter behavior, nested and explicit-null RAII restoration, Session initialization/destruction,
Val cache hit/miss/no-context/epoch and recursive resolution behavior, and core-module assertions.
TLS must remain compatible with late library loading and existing/new threads. Inline declaration
must not add a dynamic initialization guard. Hidden/internal linkage and public exports stay exact.

## Interfaces and Dependencies

Native Ubuntu24.04, CUDA12.9.2/NVRTC12.9.86, LLVM14/provider41, L4SM89 driver580.126.09, SM80.
Use the203 environment override and local slang-build skill. Max4 total CPU workers; no owned
competing work during measurement. Compiler hash1fc2311e9e0c332f2bff54d65dedb1a225218f740042a2c23c5210e76245c85f;
provider5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab. Preserve133 source,
12 artifact,565 input identities except the explicitly recorded candidate files/artifacts.

## Milestones

1. Capture256 identity/environment into build/nvvm-loop/slice-258-before and copy the optimized
   bin/lib layout with symlinks preserved and independent file contents. Retain source snapshots.
2. Edit only the two declared files. Build with CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset
   releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server, bounded30m.
   Retain full logs/exit. Copy candidate layout into slice-258-prototype; inspect TLS/caller assembly.
3. Reuse252 paired driver with257 accepted PTX/cubin reference. For each of six cells, run baseline
   then candidate in round0 and candidate then baseline in round1. Reverse cell order in round1.
   Each cell/build/round has2 warmups plus9 measured fresh processes:264 compiles total,216 measured
   and48 warmups. Use piped communicate through process exit and write logs outside the timer;
   each child bounded180s, overall30m. No retries or sample/outlier exclusion based on results.
   Assemble one identical output per cell/build/round:24 assemblies, support checks only.
4. Independently recompute medians/inclusive quartiles/extrema and every output identity. Require
   > =5% lower pooled semantic median in all6 cells, >=2% lower sum of six pooled wall medians,
   > and no cell/round wall regression >2%. Preserve exact accepted PTX/cubins in every attempt.
5. If thresholds pass, add standalone proof using matching configured Debug and optimized compiler
   objects: cross-TU TLS identity, initial null, threads, nested/null scopes, Session lifecycle,
   real Val cache/epoch/no-context/recursive behavior and existing/new threads after late dlopen.
   Inspect supported dynamic TLS access and unchanged exports. Never mix AST layouts/configs.
6. For promotion, run focused NVVM/routing tests, full units (including parallelGenericEntryPointCompile),
   semantic generic/overload/conformance/serialization/reflection neighbors, runtime4 first before
   expensive GPU gates, toolkit18/contracts6, full frozen1356/discovery351/material6. Compare every
   prior unit/semantic identity and all1707 runtime outcomes/failure histories against256.
7. If a threshold fails, retain candidate evidence/patch, restore the two source files exactly,
   rebuild matching optimized tools and verify all256 artifact/input/source hashes. Record no
   implementation or runtime improvement. Close research evidence and select next bounded action.

## Validation and Acceptance

Performance promotion requires all three thresholds, exact outputs, no missing/duplicate attempts,
unchanged options/inputs/toolchain and loader proof for both isolated layouts. A success exit alone
is insufficient. If retained, full correctness acceptance must preserve1668 correct/39 unresolved,
18 resolved histories,1049 passing/13 skipped unit identities and1052 passing/77 skipped semantic
identities. New tests count separately. Source/binary snapshots and closed raw references establish
exact tested states. If discarded, full256 evidence remains inherited, not rerun or relabeled fresh.

## Failure and Recovery

Record failed commands/attempts. Diagnose hangs/output differences before further timing. Do not
rescue a failed gate with unrelated changes, more favorable retries, relaxed thresholds or a new
TLS model. Exact rollback restores baseline; restore generated artifacts by native build and hash
audit. If the regenerated timestamp-bound cache differs, retain that attempt, verify compiler bytes
against the saved layout, restore the accepted cache and original compiler file timestamp together,
and require an unchanged-cache loader control plus all final hashes. Stop only for WORKFLOW conditions such as irrecoverable resource loss or unresolved regression.

## Artifacts and Hand-Off

Raw roots slice-258-before and slice-258-prototype retain identities/layouts/commands/logs/patch,
paired timing/assembly, audits and closure. Commit completed plan/report/compact timing evidence,
material compile-time design and STATUS; production files only if fully promoted. No push.
