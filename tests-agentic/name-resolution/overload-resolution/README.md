---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T16:48:41Z
source_commit: bbd84dc65e58598bfa71fafe72764b4076b0869b
watched_paths_digest: d7fae8a1806639a15313fe59c01c12b8f4ae0fe974bd4ed186d70b04f6e3bcd5
source_doc: docs/llm-generated/name-resolution/overload-resolution.md
source_doc_digest: 95e33141dfae79469a1ce5e5765e1021a5332906f7b6d8c7f1b50495fc3bd3b1
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for name-resolution/overload-resolution

## Intent

Tests verify the overload-resolution behaviors documented in
[`docs/llm-generated/name-resolution/overload-resolution.md`](../../../docs/llm-generated/name-resolution/overload-resolution.md):
the `TryCheckOverloadCandidate` probe-phase filter pipeline (arity,
fixity, types/coercion, directions, constraints, visibility), the
`ConversionCost` ranking, the `CompareOverloadCandidates`
tie-breaker (status, cost, lookup-result, implicit-conversion
preference, specificity, scope-distance), partial generic
application, operator overloading dispatch (including witness-table
dispatch through interface-bounded generics), and the documented
failure modes (ambiguous-overload, no-applicable, arity, direction,
visibility, generic-argument-inference-failed).

The bundle pairs algorithm-shape claims with positive
`//TEST:INTERPRET` tests (a `printf` echo of an integer tag
identifies which overload was picked) and failure-mode claims with
`//DIAGNOSTIC_TEST:SIMPLE` tests anchored at the call site.

Multi-backend rule: overload resolution runs at semantic-check
before any backend lowering. Per `_common.md`, INTERPRET is the
primary single-directive form. No multi-backend coverage is needed.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                                          | Claim (one line)                                                                                                                                                              | Tests                                                            |
| -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------- |
| O-01     | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate)                                              | A candidate that survives arity / fixity / types / directions / constraints / visibility is tagged Status::Applicable; the resolver picks it directly when it is unique.       | [`overload-survives-arity-fixity-types-directions.slang`](overload-survives-arity-fixity-types-directions.slang)          |
| O-02     | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate)                                              | TryCheckOverloadCandidateConstraints validates explicit generic application `G<A>`; with no constraints the call resolves and returns the typed result.                       | [`explicit-generic-app-resolves.slang`](explicit-generic-app-resolves.slang)                            |
| O-03     | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate)                                              | TryCheckOverloadCandidateArity emits `not enough arguments to call` (39999) when the call has fewer arguments than the candidate's required parameter count.                  | [`arity-too-few-arguments-diagnostic.slang`](arity-too-few-arguments-diagnostic.slang)                       |
| O-04     | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate)                                              | TryCheckOverloadCandidateArity emits `too many arguments to call` (39999) when the call has more arguments than the candidate's allowed max.                                  | [`arity-too-many-arguments-diagnostic.slang`](arity-too-many-arguments-diagnostic.slang)                      |
| O-05     | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate)                                              | TryCheckOverloadCandidateDirections rejects a literal r-value passed to an `out` parameter; the resolver emits `argument must be l-value`.                                    | [`direction-out-rejects-rvalue-diagnostic.slang`](direction-out-rejects-rvalue-diagnostic.slang)                  |
| O-06     | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate)                                              | TryCheckOverloadCandidateVisibility emits `decl-is-not-visible` (30600) "declaration not accessible" when the chosen candidate is hidden by `private` from the source scope.   | [`visibility-private-hidden-diagnostic.slang`](visibility-private-hidden-diagnostic.slang)                     |
| O-07     | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate)                                              | When `canConvertImplicitly` rejects an argument, the only candidate is dropped and ForReal mode surfaces a type-mismatch diagnostic at the argument site.                     | [`type-mismatch-no-conversion-diagnostic.slang`](type-mismatch-no-conversion-diagnostic.slang)                   |
| O-08     | [#probe-phase-where-candidates-come-from](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from)                                            | AddCtorOverloadCandidate produces candidates from a type's ConstructorDecls; passing two ints picks the two-arg constructor.                                                  | [`constructor-candidate-fires.slang`](constructor-candidate-fires.slang)                              |
| O-09     | [#probe-phase-where-candidates-come-from](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from)                                            | AddDeclRefOverloadCandidates collects same-name struct methods and the resolver picks the matching arity.                                                                     | [`member-overload-fires-on-receiver-method.slang`](member-overload-fires-on-receiver-method.slang)                 |
| O-10     | [#conversion-costs](../../../docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs)                                                                                        | An exact-type match (cost 0) beats an int->float conversion (kConversionCost_IntegerToFloatConversion=400) in the ranking sum.                                                | [`overload-prefers-exact-match-over-int-to-float.slang`](overload-prefers-exact-match-over-int-to-float.slang)           |
| O-11     | [#conversion-costs](../../../docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs)                                                                                        | An exact-type match (cost 0) beats a scalar-to-vector promotion (kConversionCost_ScalarToVector additive).                                                                    | [`scalar-to-vector-promotion-cost-loses.slang`](scalar-to-vector-promotion-cost-loses.slang)                    |
| O-12     | [#conversion-costs](../../../docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs)                                                                                        | kConversionCost_BoolToInt (120) is deliberately cheaper than kConversionCost_IntegerToFloatConversion (400); passing `true`, the int overload wins.                            | [`overload-prefers-bool-to-int-over-int-to-float.slang`](overload-prefers-bool-to-int-over-int-to-float.slang)           |
| O-13     | [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator)                                                                          | compareOverloadCandidateSpecificity prefers non-generic over generic when both are Applicable at the same cost.                                                               | [`overload-prefers-non-generic-over-generic.slang`](overload-prefers-non-generic-over-generic.slang)                |
| O-14     | [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator)                                                                          | Scope-distance step prefers a same-name candidate in a closer namespace over a same-name candidate in an outer scope.                                                         | [`overload-prefers-closer-scope.slang`](overload-prefers-closer-scope.slang)                            |
| O-15     | [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator)                                                                          | CompareLookupResultItems prefers a struct-member candidate over a same-name free function when called through the struct receiver.                                            | [`overload-prefers-no-default-over-default-param.slang`](overload-prefers-no-default-over-default-param.slang)           |
| O-16     | [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator)                                                                          | CompareLookupResultItems prefers an override of an inherited interface default over the default; the override fires through a generic dispatch.                              | [`override-of-interface-default-wins.slang`](override-of-interface-default-wins.slang)                       |
| O-17     | [#partial-generic-application](../../../docs/llm-generated/name-resolution/overload-resolution.md#partial-generic-application)                                                                  | TryCheckGenericOverloadCandidateTypes infers T from the argument's type when no explicit generic arguments are supplied; the call resolves and returns the typed result.      | [`partial-generic-application-resolves-via-arg.slang`](partial-generic-application-resolves-via-arg.slang)             |
| O-18     | [#operator-overloading](../../../docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading)                                                                                | Operator expressions reach overload resolution through the OverloadResolveContext machinery; a user-defined `operator+` for a struct fires on its operand types.              | [`operator-overload-on-struct.slang`](operator-overload-on-struct.slang)                              |
| O-19     | [#operator-overloading](../../../docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading)                                                                                | Operator candidate sets are keyed by operand basic-type pair; two distinct struct operator+ overloads each fire on their respective operand types.                            | [`operator-overload-on-different-types.slang`](operator-overload-on-different-types.slang)                     |
| O-20     | [#operator-overloading](../../../docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading)                                                                                | Witness-table dispatch picks the conforming type's implementation of an interface method called through an interface-bounded generic.                                         | [`witness-table-dispatch-through-generic.slang`](witness-table-dispatch-through-generic.slang)                   |
| O-21     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes)                                                                | Two Applicable candidates with identical cost / specificity / scope produce `ambiguous call to '~name'` (39999); candidate notes follow the primary error.                    | [`ambiguous-call-equal-cost-diagnostic.slang`](ambiguous-call-equal-cost-diagnostic.slang)                     |
| O-22     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes)                                                                | No applicable candidate: the resolver emits `no overload applicable to arguments of type ~args` when no overload in the set accepts the call.                                 | [`no-applicable-overload-diagnostic.slang`](no-applicable-overload-diagnostic.slang)                        |
| O-23     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes)                                                                | Generic-argument inference failure: the candidate ends at Status::GenericArgumentInferenceFailed and CompleteOverloadCandidate emits `could not specialize generic`.          | [`generic-argument-inference-failed-diagnostic.slang`](generic-argument-inference-failed-diagnostic.slang)             |
| O-24     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes)                                                                | Two overloads where one accepts the call directly and the other via a defaulted param produce ambiguous-call rather than the "fewer defaults wins" preference the doc claims. | [`ambiguous-call-with-default-param-overload.slang`](ambiguous-call-with-default-param-overload.slang)               |

## Tests in this bundle

| File                                                              | Intent     | Doc anchor                                              |
| ----------------------------------------------------------------- | ---------- | ------------------------------------------------------- |
| [`ambiguous-call-equal-cost-diagnostic.slang`](ambiguous-call-equal-cost-diagnostic.slang)                      | negative   | `#edge-cases-and-failure-modes`                         |
| [`ambiguous-call-with-default-param-overload.slang`](ambiguous-call-with-default-param-overload.slang)                | negative   | `#edge-cases-and-failure-modes`                         |
| [`arity-too-few-arguments-diagnostic.slang`](arity-too-few-arguments-diagnostic.slang)                        | negative   | `#probe-phase-trycheckoverloadcandidate`                |
| [`arity-too-many-arguments-diagnostic.slang`](arity-too-many-arguments-diagnostic.slang)                       | negative   | `#probe-phase-trycheckoverloadcandidate`                |
| [`constructor-candidate-fires.slang`](constructor-candidate-fires.slang)                               | functional | `#probe-phase-where-candidates-come-from`               |
| [`direction-out-rejects-rvalue-diagnostic.slang`](direction-out-rejects-rvalue-diagnostic.slang)                   | negative   | `#probe-phase-trycheckoverloadcandidate`                |
| [`explicit-generic-app-resolves.slang`](explicit-generic-app-resolves.slang)                             | functional | `#probe-phase-trycheckoverloadcandidate`                |
| [`generic-argument-inference-failed-diagnostic.slang`](generic-argument-inference-failed-diagnostic.slang)              | negative   | `#edge-cases-and-failure-modes`                         |
| [`member-overload-fires-on-receiver-method.slang`](member-overload-fires-on-receiver-method.slang)                  | functional | `#probe-phase-where-candidates-come-from`               |
| [`no-applicable-overload-diagnostic.slang`](no-applicable-overload-diagnostic.slang)                         | negative   | `#edge-cases-and-failure-modes`                         |
| [`operator-overload-on-different-types.slang`](operator-overload-on-different-types.slang)                      | functional | `#operator-overloading`                                 |
| [`operator-overload-on-struct.slang`](operator-overload-on-struct.slang)                               | functional | `#operator-overloading`                                 |
| [`overload-prefers-bool-to-int-over-int-to-float.slang`](overload-prefers-bool-to-int-over-int-to-float.slang)            | functional | `#conversion-costs`                                     |
| [`overload-prefers-closer-scope.slang`](overload-prefers-closer-scope.slang)                             | functional | `#tie-breaking-comparator`                              |
| [`overload-prefers-exact-match-over-int-to-float.slang`](overload-prefers-exact-match-over-int-to-float.slang)            | functional | `#conversion-costs`                                     |
| [`overload-prefers-no-default-over-default-param.slang`](overload-prefers-no-default-over-default-param.slang)            | functional | `#tie-breaking-comparator`                              |
| [`overload-prefers-non-generic-over-generic.slang`](overload-prefers-non-generic-over-generic.slang)                 | functional | `#tie-breaking-comparator`                              |
| [`overload-survives-arity-fixity-types-directions.slang`](overload-survives-arity-fixity-types-directions.slang)           | functional | `#probe-phase-trycheckoverloadcandidate`                |
| [`override-of-interface-default-wins.slang`](override-of-interface-default-wins.slang)                        | functional | `#tie-breaking-comparator`                              |
| [`partial-generic-application-resolves-via-arg.slang`](partial-generic-application-resolves-via-arg.slang)              | functional | `#partial-generic-application`                          |
| [`scalar-to-vector-promotion-cost-loses.slang`](scalar-to-vector-promotion-cost-loses.slang)                     | functional | `#conversion-costs`                                     |
| [`type-mismatch-no-conversion-diagnostic.slang`](type-mismatch-no-conversion-diagnostic.slang)                    | negative   | `#probe-phase-trycheckoverloadcandidate`                |
| [`visibility-private-hidden-diagnostic.slang`](visibility-private-hidden-diagnostic.slang)                      | negative   | `#probe-phase-trycheckoverloadcandidate`                |
| [`witness-table-dispatch-through-generic.slang`](witness-table-dispatch-through-generic.slang)                    | functional | `#operator-overloading`                                 |

## Sibling-bundle overlap

`tests-agentic/name-resolution/lookup/` covers how the **candidate set**
is built (lookup walks, container accumulation, inheritance, transparent
members). This bundle covers **how the resolver narrows that set to a
winner or to an ambiguity error**. Claims intentionally not duplicated
here:

- container-level overload accumulation (lookup owns the
  accumulation; this bundle picks the winner from the set).
- inheritance-walk member lookup (lookup owns producing the
  candidate; this bundle picks among the inherited members).
- transparent-member resolution (lookup owns the breadcrumb
  construction).

## Doc gaps observed

- The doc's `## Tie-breaking comparator` section claims step 5
  `compareOverloadCandidateSpecificity` prefers "fewer default
  parameters > more". The actual resolver does not break ties on
  this dimension for the simple case of `pick(int)` vs
  `pick(int, int = 99)` called with one int: both candidates are
  considered equally good and the resolver emits ambiguous-call.
  Either the comparator does not run this preference for default
  parameters in `Func` flavor, or the documented claim is too broad.
  `ambiguous-call-with-default-param-overload.slang` records the
  observed behavior; a precise claim would name the conditions
  under which the default-count preference actually fires (e.g.
  only for variadic vs non-variadic, or only for explicit generic
  applications).
- The doc's `## Tie-breaking comparator` lists step 4
  "Implicit conversion preference" — "If exactly one candidate is
  marked `ImplicitConversionModifier`, that one wins." The
  `__implicit_conversion` attribute is a core-module-only modifier;
  user code cannot decorate an overload with it from a `.slang`
  source. No user-observable test anchors to this claim, so it is
  recorded here as a doc gap (a claim that maps to a user-writable
  surface would unblock a test).
- The doc's `## Tie-breaking comparator` step 6 names
  `getExportRank` and says `export` decls are preferred over
  `extern` decls. `export` and `extern` are module-system primitives
  that require multi-file compilation; `slang-test` runs each
  `.slang` file standalone (no `-r` resolution), so this preference
  is not observable from a single test file. A claim that names a
  single-file surface where `getExportRank` is observable would
  unblock a test.
- The doc's `## Operator overloading` section names
  `ResolvedOperatorOverload` /
  `TypeCheckingCache::resolvedOperatorOverloadCache` and the
  invalidation by `cacheVersion`. The cache is a performance
  optimization "correctness does not depend on it", so there is no
  user-observable consequence: the same expression resolves to the
  same overload whether the cache hits or misses. No test anchors
  to the cache itself.
- The doc's `## Edge cases and failure modes` section says an
  unresolved `PartiallyAppliedGenericExpr` reaching the lowerer is
  an "internal compiler error" (a correctness invariant on the
  resolver). The user-visible diagnostic is generic-argument-
  inference-failed at the resolver, which we cover; the lowerer's
  internal-error surface is not anchored.
- The doc's `## Probe phase: TryCheckOverloadCandidate` step 7
  (`TryCheckOverloadCandidateClassNewMatchUp`) is documented as
  finalize-only; its user-visible surface is the choice between
  `Class()` and `Class.new()` AST forms. There is no documented
  diagnostic that fires when this step rejects, and the surface
  is identical at the call site, so no test anchors here directly.
- The doc's `## Probe phase: TryCheckOverloadCandidate` step 2
  (`TryCheckOverloadCandidateFixity`) names catching "calling an
  infix operator in prefix position". Slang's surface does not
  let user code write a free `prefix` / `postfix` / `infix`
  marker on a function, so the fixity-failure path is not reachable
  from a `.slang` source. A claim that names a user-writable
  surface for fixity (or a confirmation that fixity is implicit)
  would unblock a test.

## Out of scope (no-GPU runner)

None of the overload-resolution claims in this bundle require a GPU.
All tests use `//TEST:INTERPRET(filecheck=CHECK):` or
`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`, both of which run on the
no-GPU runner.
