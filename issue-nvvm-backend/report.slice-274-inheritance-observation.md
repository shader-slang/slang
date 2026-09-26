# Explain material inheritance-cache work

## Motivation

Research273 repeatedly samples inheritance calculation during material semantic checking. That does
not show whether the compiler repeats work for identical keys. Distinguish useful cache hits, first
computations and legitimate invalidation before proposing a new optimization.

## Proposed solution

Temporarily observe existing cache decisions using their canonical keys and context lifetime IDs.
All eight material runs compute each of6,230 keys exactly once. Close this repeated-inheritance-work
lead; the evidence does not justify another cache, source reduction or compiler change.

## Change summary

[Evidence274](research-evidence.slice-274.json) retains eight main obligations,42 output controls,
exact unit/semantic comparisons, counters, temporary identity and restoration. This report, plan and
navigation are retained. The four-file observer patch, events, scripts, binaries and exhaustive
inventories stay under `build/nvvm-inheritance-observation274`. No production patch survives.

## Concepts and vocabulary

A cache key is the existing Type pointer or canonical DeclRef identity within one semantic context.
A cold query finds no entry. A valid hit reuses a complete current entry. A computing hit returns an
in-progress entry to break a cycle. Extension epochs invalidate stale dependencies; unresolved ancestor
facets make a contextual result incomplete and intentionally unsuitable for caching.

## Process report

The observed path remains generic overload inference -> structural unification -> inheritance ->
extension matching -> canonical substitution. For example, matching an extension of `vector<float,N>`
against `vector<int,2>` requires distinguishing specialized checked types. For an equality cycle
`T.A == T.B`, a partial result must not erase the missing ancestor. These are valid inputs owned by
the existing semantic layer; the observer changes none of those decisions or representations.

The temporary helper inventory is scope ownership/cleanup, query-event capture, explicit invalidation
capture and a monotonic context lifetime ID. The ID prevents recycled context addresses from merging
lifetimes. Direct declaration access reads existing operand fields; logging performs no additional
resolution, substitution, conformance or cache lookup. The existing epoch comparison exposes its
first mismatched dependency without a second query. Explicit invalidation at equality-constraint
readiness is recorded separately, so subsequent cold queries are not misclassified as unexplained.

Primitive events are buffered inside `checkAllTranslationUnits`; names/source locations are formatted
afterward. Complete counters continue beyond the524,288-event detail limit, with explicit overflow.
The scope guard frees/reset state on exceptional exit without claiming complete evidence. Query RAII
restores its active parent before each local lifetime ends; this explains the retained GCC dangling-
pointer warning without suppressing it. Every helper is removed at closeout.

A fresh reviewer agent and reactivation both hit the thread limit. WORKFLOW's fallback applies: root
performs separate local source and evidence audits against the writer's results. This is not claimed
as independent-agent review. Local audits rehash outputs, reparse native test logs, reconcile raw events
and query-parent lifetimes, and verify live restoration.

The temporary build is f9a941368 plus patch808694bb, compiler1121cf2e; provider ABI42 is unchanged.
Cached version metadata stays at the accepted270 value solely for this discarded observer build.
All42 enabled controls preserve PTX/cubin bytes and403 named resource records. Units preserve all1100
identities (1087pass/13skip); semantics all1248 (1170pass/78skip). Server gates disable observation to
avoid multiple scopes sharing one exclusive output path. Disabled/enabled material pilots preserve
PTX. The eight enabled main runs cover two entries, two O3 backends and two fresh repetitions, with
reverse order in the second repetition. Every compile succeeds with exact accepted272 PTX.

Every main run has one context,160,791 retained events and zero overflow. Counts are identical:

| Cache owner | Queries | Cold / complete | Valid hits | Computing hits |
| ----------- | ------- | --------------- | ---------- | -------------- |
| Type        | 370     | 13 /13          | 357        | 0              |
| DeclRef     | 154191  | 6217 /6217      | 147968     | 6              |

All6,230 canonical keys have one cold query and one complete store. There are zero recomputed keys,
stale dependencies, incomplete results or explicit invalidations. Thus148,325 valid queries reuse
cached results; six queries use the established cycle path. The observer's stale/incomplete/
invalidation branches are source-audited only: these workloads do not exercise positive events there.
Different specialized keys can share a declaration name, so textual names are never merged as keys.

This rejects the suspected repeated computation at these existing cache owners; it does not prove
that all first-time semantic work is optimal or establish CPU cost. No call-site/argument tracing or
structural equivalence is added, and zero reduction variants are attempted. Observer wall times are
operational overhead, not compile-performance improvements. Material GPU runtime remains unqualified.

Full layout replacement restores all100 files/symlinks/modes with no extras,37 runtime hashes,
576 inputs,22 pins and3 configurations. All four sources equal HEAD; their mtimes force rebuilding
experimental objects on the next build, which must refresh version metadata. No workload processes
remain. Accepted270 correctness/failure histories and full270/targeted233/cadence0 remain inherited.
The loop continues with the recorded concurrent NVRTC automatic-PCH reliability incident; Slack stays
skipped. No build, test or main-observation failures/retries occurred; exploration/schema errors and
the temporary warning are preserved separately.
