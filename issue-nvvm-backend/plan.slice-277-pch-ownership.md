# Give each NVRTC compiler a private automatic-PCH directory

This bounded ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception. Authorized
loop active; no Slack/push/install/host changes. Root owns acceptance, build/gates, durable records
and local commit. Fresh277 worker unavailable at thread limit; existing worker may own production
and test edits, root performs separate local audits without claiming independent-agent review.

## Purpose and Observable Result

Independent Slang processes should retain automatic-PCH reuse without replacing another process's
cache file. The fixed275 shared-cwd comparison currently has8 reuse failures+2crashes/16 shared
processes. After this fix all40 declared serial/shared/private processes must pass with positive PCH
creation and within-process reuse. Preserve source names, explicit user cache-directory options and
full accepted correctness. Slice276 establishes safe directory retirement with NVRTC held open on
installed12.9.86; actual Slang compiler-owner lifetime tests still required.

## Progress

- [x] 2026-09-26: Research275/276 committed5afd52ef0/9210ef5a1. Accepted270 compiler unchanged.
- [x] Fresh277 delegation unavailable; record fallback and establish one source writer.
- [x] Five-file candidate implements exclusive helper/default ownership plus3 new unit IDs.
      Root and author separate source audits pass; Windows/acquisition-failure branches source-only.
- [x] Formatting/source review complete. Configure refresh identifies9210ef5a1;100-entry snapshot saved.
- [x] Build final candidate30e61108 patch; compiler aa1fe42e/version9210ef5a1 and provider unchanged.
      Test-only syntax expectation corrected after direct NVRTC probe; all attempts retained.
- [x] Focused5/5pass0skip. Fixed40processes allpass;120PCHcreations,40distinct retired directories,
      all20overlap checks and320rawhashes independently checked by root.
- [x] Full checkpoint preserves all1740 outcomes/576 hashes and37 unresolved/20 resolved histories.
      Units1090pass/13skip preserve1100 old IDs plus3 new passes; semantics1170/78 preserve1248 IDs.
      Runtime4/toolkit18/material6 and four runner suites pass. Material PTX/cubin/resources exact.
- [x] Review exact final bytes, compact five-part report/validation record and navigation.
- [x] Local acceptance prepared: final formatting/diff check and post-gate identity audit pass;
      this completed plan is committed with the accepted implementation and validation record.

## Context and Root Cause

`emitEntryPointsSourceFromIR` produces a valid unnamed CUDA source artifact; `ArtifactUtil::findPath`
returns empty and `NVRTCDownstreamCompiler::compile` passes that to `nvrtcCreateProgram`. For a leading
include, the adapter adds automatic `-pch`, so independent library instances use `default_program.pch`
in their shared working directory. Research275 traces cross-owner replacement/truncation and failures.
Do not change source identity, semantic lowering or caller cwd. The downstream adapter owns default
cache namespace and must preserve explicitly caller-selected namespaces.

## Architecture, Scope and Invariants

One lazily acquired, stable, exclusive temporary directory belongs to one NVRTC compiler object.
Use it for Slang-requested automatic PCH only when no explicit caller PCH-directory option exists.
Recognize documented long/short aliases and separate/equal-value forms, preserving original arguments
and NVRTC diagnostics for malformed options. Leave explicit caller directories caller-owned; never
clean them up. Keep program/source names unchanged. Reuse is guaranteed across calls to the same compiler object, not across independently created
compiler objects. Those intentionally get independent caches; explicit caller directories retain
caller-managed sharing. Directory survives all this owner's compiles,
then cleanup occurs after its program handles are destroyed and its own library reference is released,
even if another NVRTC reference remains. This permits normal NVRTC last-reference cleanup first.
If acquisition fails, emit a useful compile failure rather than silently disable PCH or retry compilation.
No global process cwd changes, synthetic program names, PID-only namespace, compile serialization or
per-compile directory. Synchronize lazy directory initialization if necessary for concurrent calls;
do not imply new general thread-safety guarantees for preexisting mutable adapter state.

Reuse Path/File helpers where sound. File::generateTemporary reserves a file, not a directory; don't
remove it then mkdir the same path with a race. Prefer a documented core temporary-directory helper:
Unix mkdtemp with private permissions; Windows exclusive directory acquisition using a reserved unique
candidate and no preexisting-directory acceptance. New helper must have a meaningful lifecycle test.
Destructor cleanup should only touch the exact successfully acquired private directory. Audit every
new helper/fallback/special case and demonstrate its test/ownership layer. Avoid public API additions.

## Milestones and Execution

1. Source writer inspects core IO and native NVRTC PCH tests; inventories helpers and explicit-option
   behavior. Implement smallest ownership change and tests. Actual-adapter coverage includes existing
   unnamed creation/reuse/invalidation and no-include gate; named source and owner lifetimes with an
   external library reference; second owner survives first destruction; explicit --pch-dir=/-pch-dir= forms retain caller ownership, while malformed standalone
   long/short options retain NVRTC errors. A direct SDK probe rejects separate option/value arguments. Include malformed or unavailable caller path
   failure, unchanged source-name diagnostics and private-directory acquisition/lifetime control where
   practical. No broad unrelated test rewrites. Preserve existing unit identities/status gates.
2. Root audits diff/helper input shapes, then formatter explicit changed files. Build through local
   slang-build skill, native releaseWithDebugInfo preset, max4. Before first rebuild snapshot accepted270
   installed layout to a new ignored root; do not overwrite historical snapshots. Configure same options
   but clear cached SLANG_VERSION_FULL/NUMERIC; rebuild restored observer-source objects and numerics
   modules. Record actual loaded compiler/provider/version/source patch, config and22pins.
3. CPU focused tests must execute, not skip. Preserve existing failing275 as before-evidence. Adapt
   fixed40-process275 harness into new ignored root with the same native unit/options/group inventory;
   explicit default private-PCH paths replace previous cwd paths, but process cwd sharing/control and
   obligations stay exact. Freeze before execution, no retries,180s child/1800s batch, max2. Require
   all40 pass and positive create/reuse evidence. Trace distinct owner paths and cleanup; do not infer
   reuse from status absence. Preserve all failures and resolve rather than relabel them.
4. Because shared downstream-library behavior changes, run full accepted270 comparison, not only a
   targeted slice. Follow RESULTS commands: checkpoint (runtime4, frozen1356/discovery384/material6),
   unit and semantic suites with2servers/retries disabled, toolkit18, four runner contracts. Bound long
   gates1800s, serialize GPU/build/suites, max4jobs. Compare exact1740 outcome identities/five fields,
   all576input hashes and exact native maps against270. Existing37unresolved/20resolved histories remain;
   no regression becomes baseline. Add new unit identities separately. Preserve original review-required
   comparisons if new expected tests or compiled artifacts need explicit acceptance.
5. Qualify explicit neighboring real GPU NVRTC/NVVMO0/O3 output and material compile/assembly through
   full gate. Preserve matching accepted quality-control PTX/cubin/resource data when practical to
   substantiate unchanged output; do not turn compiler filesystem isolation into a GPU-speed claim.
6. Root rechecks final source/artifact identity, performs input-shape/helper review and focused revert
   drill when feasible (275 supplies real before-failure). Write compact five-part report and one
   accepted-full record with actual per-cell outcomes, failure history and focused concurrency/lifetime
   obligations. Update test manifest only if inventory changes. Format/diff check, reviewed local commit,
   update STATUS/HISTORY/HANDOFF; skip Slack and continue authorized loop.

## Decision Log

- 2026-09-26, lead: Final review selects own-library-reference release before private directory
  removal, matching ScopeSharedLibrary lifetime ordering. No programs remain; if this is the last
  reference NVRTC performs normal cleanup first, while surviving external references remain covered
  by276. Apply after build2 completes and rebuild before testing; no unsupported Windows bug claim.
- 2026-09-26, lead: Stable percompiler ownership is supported by20/100 lifecycle observations, including
  directory retirement with library held and surviving-owner reuse. Explicit user location remains a
  caller contract; don't override it to manufacture isolation. Scope guarantees to actual qualification.

## Surprises and Discoveries

First focused run passes4/5; caller-directory positive assumption for separate option/value fails.
Direct typedNVRTC probe proves5/5 invalid-option for standalone long/short plus0/0 success for equals
forms; correct new test expectations without production normalization or version-specific fallback.
First build failed in new test-only API calls (missing cast helper include; ComPtr address-of).
Corrected using existing as(ICastable*) and ISlangBlob**/writeRef conventions; production unchanged.
Failed candidate/build log retained. Research276 final unload tolerates missing retired files; no compile-stage unlink errors occurred.
This is observed12.9.86 behavior, not a cross-version documented lifetime guarantee. Fresh worker
thread limit requires reuse/local audits. Build must refresh old cached version and restored objects.

## Validation, Recovery and Stopping Conditions

All owned processes bounded and failures retained. Stop new feature work on a correctness regression;
resolve within this bounded ownership slice or restore accepted artifacts/source and record blocker.
Do not change drivers/toolkits/host policy or untracked user files. External semantic-policy decisions
may need human input; routine implementation choices and local commits are authorized.

## Outcomes and Retrospective

All required gates pass for the final source patch30e61108 and actual compiler aa1fe42e. The fixed40
comparison changes shared16 from6pass/8nativefail/2SIGSEGV to16pass; all24 controls still pass, with
40 distinct retired namespaces and120 PCH creations. The full corpus and native maps preserve every
old outcome, with three declared new passing units. All37 runtime artifacts,22pins and576inputs verify.
The compact accepted-full validation277 retains failed attempts and original262 history. The exact262
deletion signature was not independently reproduced, so the correction directly qualifies demonstrated
shared-file ownership rather than claiming that exact incident's causal reconstruction. Caller-shared
directories and same-adapter concurrency retain their stated limits. Last full277/targeted233/cadence0.
Continue with a bounded accessor/generic language-surface probe using existing independent oracles.
