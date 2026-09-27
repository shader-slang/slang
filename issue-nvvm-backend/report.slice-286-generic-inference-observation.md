# Explain generic inference in the material workload

## Motivation

Sampling273 repeatedly found generic overload inference during material semantic checking. Existing
inheritance caches already compute each canonical key once (274), so another cache needs evidence.
A concrete material call is:

```slang
public bool is_set<T : __EnumType>(T value, T flag)
{
    return value & flag;
}
public void flip_bit<T : __EnumType>(inout T value, T flag)
{
    value = is_set(value, flag) ? (value & ~flag) : (value | flag);
}
```

The observer sees the same `is_set` candidate inferred twice at that call site. Establish how common
such repetition is and distinguish it from separate valid overload candidates before changing lookup.

## Proposed solution

Temporarily record existing inference inputs, produced types and outcomes at their original semantic
boundaries. All eight main runs produce 7,995 inferences, with only21 repeated strict-context pairs.
Broader candidate screening is a larger count-based lead, but no safe optimization or CPU saving is
established. Remove the observer and restore accepted285 exactly.

## Change summary

[Evidence286](research-evidence.slice-286.json) retains eight main obligations, two pilots, six material
controls, exact native identity comparisons, incidents and restoration. This completed plan/report and
navigation are retained. No compiler, workload, corpus or configuration change survives. Three-file
observer patches, events, source-context tables and experimental binaries remain under
`build/nvvm-generic-inference286`. Fresh delegation hit the thread limit; reused author and separate
reused reviewer are disclosed.

## Concepts and vocabulary

A canonical generic reference includes outer substitutions; its declaration name alone does not
identify a specialization. An observed-input projection records only selected fields, not a complete
semantic cache key. Inference success produces a specialized candidate before applicability checks
and ranking; it does not mean that overload is selected.

## Process report

`ResolveInvoke` and coercion lookup enumerate candidates through `AddOverloadCandidates` and
`addOverloadCandidatesForCallToGeneric`. `inferGenericArguments` preserves the canonical outer
reference, obtains parameter types, matches arguments, unifies types and calls `trySolveGenericArguments`.
These are valid semantic inputs at the correct owning layer; there is no malformed representation
for the observer to repair.

Temporary helpers own copy-safe lifetime IDs, bounded primitive capture, scope/return cleanup and
post-scope source formatting. They read raw fields or already-produced matching/unification results;
calling `getArgTypeForInference` again would perform semantic work and is explicitly avoided. Nested
scopes restore their parent state; sequential roots append to one exclusive sink under a global
524,288-row limit. Every helper is removed at closeout. Only ordinary calls in one root occur in the
actual material; higher-order, nested/sequential-root, overflow and exceptional paths remain source
reviewed rather than positively exercised.

Two entries, two O3 backends and two fresh repetitions (second in reverse order) give eight main
observations. Each has95,295 rows, zero overflow, one shared context and1,208 overload-context lifetimes.
Independent raw-event review verifies every scope, parent, stage and indexed list. Outcomes are:

| Inference outcome   | Calls per run |
| ------------------- | ------------: |
| Arity-stage failure |           810 |
| Solver success      |         1,965 |
| Solver null         |         5,220 |

All15 enabled observations (pilot1, controls6, main8) agree on counts and descriptive call metadata;
raw pointers are compared only within their recorded lifetimes. Strict matching finds21 extra events
among7,995 calls (0.263% by count): next_float11, is_set5, gamma3, lgamma1 and sqr1. Every pair succeeds
with the same result identity. Lookup origins/breadcrumbs and mutable readiness are unobserved, so
this does not justify dropping candidates. A looser grouping has one differing result identity:
both int2-to-uint2 conversion inferences succeed at cost0 in separate coercion contexts. The cause
of the reference difference and semantic interchangeability remain unproven.

By name family, bitwise OR considers75 distinct declarations at each of18 callsites:73 fail and two
succeed at each site. Initializers account for741 arity failures and550 solver-null returns. Names
are navigation aids, not equivalence classes. These are candidate-screening hypotheses, not proof
of removable work or time savings; no cache, pruning rule or reduction is retained.

The temporary build uses source35ada673a plus patch033664d5, loaded compilercc6b5823 and unchanged
providerABI42 af1661de; cached version text remains301-g8fbf0f84e and is not its identity. Pilots and
main PTX equal285. All six material PTX/cubins/resources equal285. Native tests preserve all1110unit
and1248semantic identities (1097pass/13skip and1170pass/78skip). No GPU material execution is claimed.

Runner review corrected detached-child cancellation and retained two failed cleanup-probe checks.
A capture script initially used a relative provider key against an absolute-key map. Root mistakenly
launched the pilot after that metadata failure; corrected capture matches the pilot's independently
frozen precompile37runtime/3source identities exactly. Old scripts, ordering incident and amendment
are retained. No build or compiler workload failed or was retried.

Full restoration verifies100 installed file/link/mode entries,37runtime artifacts,11 qualified source
files,2configurations,576 main inputs and22pins. Three observer sources equal HEAD and have mtimes
newer than experimental objects. A later production build must refresh version metadata and rebuild
them. Accepted285 corpus outcomes,37unresolved/20resolved histories and full285/targeted233/cadence0
remain inherited. The loop continues; next priority is a bounded nested FP8/BF16 dynamic-dispatch
qualification, connecting the flat270 and nested279 domains. Slack remains skipped.
