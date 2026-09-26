# Explain material inheritance-cache work

This bounded research ExecPlan follows `.agent/PLANS.md` and the committed NVVM plan exception.
The user-authorized development loop remains active. Root owns scope/acceptance/records/commits;
a fresh worker owns temporary instrumentation and execution. Reviewer spawn and completed-agent
reactivation both hit the agent-thread limit; WORKFLOW's fallback applies: root performs separate
local method/source/evidence audits, without claiming an independent reviewer.
Skip Slack, pushes, installs and host changes. One writer, serial builds/workloads, max4 build jobs.

## Purpose and Observable Result

Research273 observes `_calcInheritanceInfo` in338/982 material semantic stacks (312/912 sensitivity),
but cannot distinguish distinct specialized work from repeated computation. Explain the existing
inheritance cache's query outcomes and canonical identities. Open an optimization gate only if a
specific avoidable repeated case and representative reduction are established. This is the one bounded
follow-up to273; if the evidence shows required/distinct work or remains inconclusive, close the lead
and select another development target rather than extending profiling indefinitely.

## Progress

- [x] 2026-09-26: Research273 committed f9a941368; accepted270 compiler/layout unchanged.
- [x] 2026-09-26: Verify37runtime/576inputs/22pins/100layout/3config/material and preserve full
      accepted layout/source/config under raw274.
- [x] 2026-09-26: Local method audit approves raw canonical access, two cache owners, existing epoch
      comparison, generation and incomplete/explicit invalidation events. Final observer patch808694bb
      adds exceptional-scope cleanup and safe unavailable source text; build/qualification approved.
      All42 controls enable observation; server unit/semantic gates disable it to preserve unique-file
      scope constraints. Disabled/enabled material pilot and main8 exercise the observed path.
- [x] 2026-09-26: Build passes (temporary compiler1121cf2e, patch808694bb). All42 enabled
      PTX/cubin/resource controls exact; units1100 and semantics1248 identity/status maps match270.
      Separate lead log parsing and byte hashing pass. Pilot disabled/enabled PTX exact,160791events,
      zerooverflow,6230keys each cold/complete once. Main8 approved after gate review.
- [x] 2026-09-26: Main8 all pass with identical exact-output/counter results;6230keys each
      computed once,160791events/zerooverflow. Separate raw-event audit verifies query-parent lifetimes
      and every counter. Zero reductions; close this repeated-work lead.
- [x] 2026-09-26: Remove4source edits and restore100layout/37runtime/576inputs/22pins/3config;
      root live hash/inventory/source audit passes. No residual workload. Sources newer than objects.
- [x] 2026-09-26: Final local audit verifies37artifact refs, exact8CLI/output pairs and all compact
      obligations. Reviewed candidate95334158 retained; format/diff checks pass. Local commit closes274.

## Context and Current Pipeline

Actual sensitivity-qualified material stack: generic overload inference -> structural type unification
-> `SharedSemanticsContext::_getInheritanceInfo` -> `_calcInheritanceInfo` -> `considerExtension`
-> extension application -> canonical substitution. Both overload and inheritance paths recur across
all16 profiles; their relative rank is not stable. Source trace is in report273/raw273.

`source/slang/slang-check-inheritance.cpp` has two cache owners: Type keys for non-DeclRef types and
canonical DeclRef keys otherwise. Existing cache entries track `isComputing`, generation and dependency
extension epochs. Valid entries are reused; computing entries break cycles. Missing or stale entries
recompute. Results with unresolved skipped ancestors are intentionally not retained. Never change
these decisions, key equality, canonical construction, dependency collection or semantic diagnostics.

## Scope, Architecture and Invariants

Temporary observation only; no production optimization, semantic alternate representation, new cache,
provider ABI, corpus/runner or source-input change. Restrict first implementation to existing cache
query/decision boundaries and necessary observation scope plumbing. Do not instrument all generic
inference/extension operations by default. A minimum observer records:

- Count every query/outcome at both cache owners: cold, valid, computing-cycle, stale, complete store,
  incomplete nonretention. Separate query entry decisions from computation completion outcomes.
- Existing canonical object identity plus SharedSemanticsContext/request identity, cache kind, query
  and parent query IDs. Canonical key equality is exactly existing pointer/DeclRef identity.
- Stored/current generation and invalidated dependency identity/old/new epoch where already observed;
  incomplete ancestor identities where the existing code produces them. Do not perform extra semantic
  lookups, resolution, substitution or conformance checks for logging.
- Defining declaration/source identity only when safely available from existing producers or direct
  fields. Document unavailable call-site/argument context rather than inventing or forcing it.
- Aggregate counters remain complete. Bound detailed retention explicitly with overflow counts and
  per-request identity scope. No claim about unrecorded equal-key repeats when detail overflows.

Choose low-overhead primitive event capture; format names/output after the observed scope where
possible. Avoid recursively invoking semantic work or allocating new semantic nodes. Temporary
observer allocations are separate from inferred compiler allocation costs. Counts are not timings,
CPU shares or optimization wins. Review helper inventory/input shapes before any build: every new
helper must serve observation only and be removed at closeout.

## Milestones and Exact Acceptance

1. Rawroot `build/nvvm-inheritance-observation274`. Read AGENTS/WORKFLOW/STATUS/RESULTS and installed
   `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Verify37runtime artifacts,576inputs,
   22pins,100layout entries,3config hashes and material source against273/270. Preserve complete
   accepted layout with modes/symlinks, source snapshots and config. Actual compiler SHA74bb18f3;
   provider ABI42 SHAfbef1a9e remain baseline identity.
2. Prepare the smallest discarded observer patch and event schema under ignored rawroot; preserve
   exact applied diff/hash. Send method/branch accounting/cleanup to root for separate local review;
   WAIT for method approval before build/execution. No target function evaluation or host changes.
   Discard/repair observer on semantic mutation, ambiguous identities or unreliable accounting.
3. Build via existing native releaseWithDebugInfo preset, max4 workers; retain all build attempts.
   This discarded instrumentation may preserve accepted cached version metadata as272 did; report
   exact source revision+patch and loaded bytes separately. Next real production build must refresh
   version metadata. Include slangc/slang-test/numerics modules and use matching temporary layout.
4. Qualify observer output with the existing42 PTX/assembly controls (quality36 plus material6),
   exact accepted272 bytes/resources, and exact accepted270 unit/semantic test identities. Run gates
   serially with2unit servers, no retries, each bounded1800s. Observer-disabled and enabled fixed
   material pilot output must match accepted PTX; counters must reconcile and detailed overflow must
   be explicit. No performance interpretation until this review gate; no full GPU checkpoint required
   for discarded observer provided accepted artifacts are restored exactly and no production change.
5. Fixed main inventory: two material entries x NVRTC O3/NVVM O3 x two fresh-process repetitions =8
   observed compiles, second repetition reverse order, jobs1, each180s/batch1800s. Copy exact272 CLI
   arguments except output path and explicit observer output control. Retain all outcomes/events and
   compare PTX. No benchmark claim; log wall/observer cost only as method overhead. No retry/removal.
   Count same canonical key within a context separately from different specialized keys; distinguish
   stale-epoch and incomplete-result repetitions. Complete counters should agree across repetitions
   unless a documented source of nondeterminism explains differences.
6. Separate local analysis identifies whether repeated work is required and source context is concrete.
   At most4 reduction variants for one demonstrated case, only after root review of evidence.
   Preserve variants/errors and independent semantics; reduced compilation alone is not proof.
   If canonical context or repeated avoidable work is not established, zero variants is acceptable.
   No optimizer implementation in274; no further instrumentation expansion to chase another path.
7. Remove every temporary source edit; full accepted layout replacement, no overlay. Verify all100
   files/symlinks/modes/no extras,37runtime/576inputs/22pins/3config, empty source diff. Ensure future
   source mtimes cause rebuilding discarded observer objects; document version refresh prerequisite.
   Preserve experimental binaries separately and confirm no residual workload processes.
8. Compact five-part report, completed plan and one structured evidence record with8 main obligations,
   all42controls, exact unit/semantic identity comparison and inherited270 baseline, failure histories,
   observation/reduction limits and restoration proof. Raw events/logs/snapshots stay ignored. Format,
   separate local final review and local commit; update STATUS/HISTORY/HANDOFF, skip Slack, continue loop.

## Decision Log

- 2026-09-26, lead: Choose one cache-outcome follow-up rather than infer a new cache from stack presence.
  Limit the observer to existing inheritance owners; deeper generic/extension context is deferred unless
  naturally available. Required distinct work or inconclusive evidence closes this material lead.
- 2026-09-26, lead: Add explicit equality-constraint invalidation events at the existing removal
  owner so later absent-key queries are not mistaken for unexplained misses. Use process-unique
  context lifetime IDs rather than pointer-only identity; detail capacity524288 with complete counters.
  Require exception cleanup and safe unavailable source text before executing observer.
- Fresh compiler changes are not authorized by this plan; any subsequent optimization requires a new
  bounded slice, failing/reducing tests, input-shape audit and paired performance/output qualification.

## Surprises and Discoveries

Reviewer delegation is unavailable: both a fresh spawn and reactivating completed semantic_review273
fail with agent-thread limit. The fresh writer started successfully; root audits separately per WORKFLOW.
Debugger273 establishes no concrete material declaration/type pair. Its sampling durations
are not baseline timings. At slice start, installed layout was270 while build objects included discarded272 observer objects;
the successful build refreshes changed sources. Temporary observer emits a GCC dangling-pointer
warning: local query RAII stores its address while active and restores its parent on every return/
unwind before lifetime ends. Local source audit proves this scoped use; no suppression added.
Pilot has154561queries and6230canonical keys; zero stale/incomplete/explicit-invalidation events,
so those observer branches are source-audited only. A lead gate-name schema assumption was corrected
without rerunning tests. These observations remain provisional until all8 main obligations pass.

## Validation, Recovery and Stopping Conditions

A temporary observer must preserve compiler control flow/results and exact output. Preserve failed
attempts; repair or discard without relabeling failures. Essential unavailable resources or a semantic
policy decision can require human input; routine instrumentation/research remains authorized. Before
any stop restore the accepted source/layout if possible. A research-only closeout does not reset full
checkpoint cadence or erase unresolved/resolved histories. No material GPU runtime claim.

## Outcomes and Retrospective

All8 profiles compute each existing canonical key exactly once;148325 valid queries reuse results
and6 use computing-cycle entries. No missed reuse or optimization/reducer established. Close this
material lead without changing production. All42 control pairs/403resource rows and exact1100unit/
1248semantic maps preserve acceptance. Full accepted source/layout restored; local audits completed
under reviewer-thread-limit fallback. Baseline remains270/full270/targeted233/cadence0. Next bounded
slice investigates the previously recorded concurrent NVRTC automatic-PCH reliability failure.
Final compact audit and formatting pass. This accepted research is ready for its closing local commit;
no remaining validation obligation. Loop continues and Slack stays skipped.
