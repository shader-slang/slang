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

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| An exact-type match (cost 0) beats a scalar-to-vector promotion (kConversionCost_ScalarToVector additive); the int3 overload is not chosen when the call passes an int. | functional | [#conversion-costs](../../../docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs) | [`scalar-to-vector-promotion-cost-loses.slang`](scalar-to-vector-promotion-cost-loses.slang) |
| Conversion-cost table places kConversionCost_BoolToInt (120) deliberately cheaper than kConversionCost_IntegerToFloatConversion (400); when both overloads are callable with a bool argument, the int overload wins. | functional | [#conversion-costs](../../../docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs) | [`overload-prefers-bool-to-int-over-int-to-float.slang`](overload-prefers-bool-to-int-over-int-to-float.slang) |
| When two overloads are callable with an int argument, the int->int identity (cost 0) wins over int->float (kConversionCost_IntegerToFloatConversion=400) per the conversion-cost ranking. | functional | [#conversion-costs](../../../docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs) | [`overload-prefers-exact-match-over-int-to-float.slang`](overload-prefers-exact-match-over-int-to-float.slang) |
| Two Applicable candidates with identical cost / specificity / scope produce ambiguous-overload-for-name-with-args (39999); the resolver emits the diagnostic with the name and a candidate note per tied entry. | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes) | [`ambiguous-call-equal-cost-diagnostic.slang`](ambiguous-call-equal-cost-diagnostic.slang) |
| Two overloads that both accept a single int argument (one directly, one via a defaulted trailing parameter) tie under the comparator and produce ambiguous-call (the doc's compareOverloadCandidateSpecificity step about default params does not break this tie in practice). | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes) | [`ambiguous-call-with-default-param-overload.slang`](ambiguous-call-with-default-param-overload.slang) |
| When TryCheckGenericOverloadCandidateTypes cannot infer T from the argument shape, the candidate ends at Status::GenericArgumentInferenceFailed and CompleteOverloadCandidate emits generic-argument-inference-failed (could not specialize generic for arguments of type). | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes) | [`generic-argument-inference-failed-diagnostic.slang`](generic-argument-inference-failed-diagnostic.slang) |
| When no candidate in the overload set is Applicable for the given argument types, the resolver emits no-overload-applicable-with-args / ambiguous-call-with-args (code 39999) with a candidate note per rejected overload. | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes) | [`no-applicable-overload-diagnostic.slang`](no-applicable-overload-diagnostic.slang) |
| Operator candidate sets are keyed by operand basic-type pair; two distinct user-defined operator+ overloads for different struct types each fire on their respective operand types without ambiguity. | functional | [#operator-overloading](../../../docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading) | [`operator-overload-on-different-types.slang`](operator-overload-on-different-types.slang) |
| Operator expressions reach overload resolution through the same OverloadResolveContext machinery as named calls; a user-defined operator+ for a struct fires when both operands are that struct type. | functional | [#operator-overloading](../../../docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading) | [`operator-overload-on-struct.slang`](operator-overload-on-struct.slang) |
| Witness-table dispatch through an interface-bounded generic picks the conforming type's implementation of the interface method, observable as the conforming-type-specific return value. | functional | [#operator-overloading](../../../docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading) | [`witness-table-dispatch-through-generic.slang`](witness-table-dispatch-through-generic.slang) |
| A generic candidate's TryCheckGenericOverloadCandidateTypes infers the type parameter from the argument's type when the call does not supply explicit generic arguments; the resolved call is callable and returns the typed result. | functional | [#partial-generic-application](../../../docs/llm-generated/name-resolution/overload-resolution.md#partial-generic-application) | [`partial-generic-application-resolves-via-arg.slang`](partial-generic-application-resolves-via-arg.slang) |
| A candidate that survives all probe-phase steps (arity, fixity, types, directions, constraints, visibility) is tagged Status::Applicable; with one candidate set the resolver picks it directly. | functional | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | [`overload-survives-arity-fixity-types-directions.slang`](overload-survives-arity-fixity-types-directions.slang) |
| An explicit generic application G<A> binds the generic's type parameter directly, satisfying TryCheckOverloadCandidateConstraints; the call resolves and returns the expected value. | functional | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | [`explicit-generic-app-resolves.slang`](explicit-generic-app-resolves.slang) |
| TryCheckOverloadCandidateArity rejects a call with fewer arguments than required; in ForReal the resolver emits not-enough-arguments-to-call (code 39999). | negative | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | [`arity-too-few-arguments-diagnostic.slang`](arity-too-few-arguments-diagnostic.slang) |
| TryCheckOverloadCandidateArity rejects a call with more arguments than the allowed max; in ForReal the resolver emits too-many-arguments-to-call (code 39999). | negative | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | [`arity-too-many-arguments-diagnostic.slang`](arity-too-many-arguments-diagnostic.slang) |
| TryCheckOverloadCandidateDirections enforces parameter directions; passing a literal r-value to an `out` parameter fails the direction step (argument-must-be-l-value). | negative | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | [`direction-out-rejects-rvalue-diagnostic.slang`](direction-out-rejects-rvalue-diagnostic.slang) |
| TryCheckOverloadCandidateVisibility rejects a candidate whose declaration is not visible from the source scope; a private member accessed outside its host emits decl-is-not-visible (30600). | negative | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | [`visibility-private-hidden-diagnostic.slang`](visibility-private-hidden-diagnostic.slang) |
| When TryCheckOverloadCandidateTypes cannot find any implicit conversion for an argument (canConvertImplicitly rejects), the only candidate is dropped and the user sees a no-applicable-overload diagnostic at the call site. | negative | [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | [`type-mismatch-no-conversion-diagnostic.slang`](type-mismatch-no-conversion-diagnostic.slang) |
| AddCtorOverloadCandidate produces candidates from a type's ConstructorDecls; the overload resolver picks among constructors based on argument types just like free functions. | functional | [#probe-phase-where-candidates-come-from](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from) | [`constructor-candidate-fires.slang`](constructor-candidate-fires.slang) |
| Member-access overload set assembled by AddDeclRefOverloadCandidates resolves to the receiver-typed method whose parameter list matches the call. | functional | [#probe-phase-where-candidates-come-from](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from) | [`member-overload-fires-on-receiver-method.slang`](member-overload-fires-on-receiver-method.slang) |
| CompareLookupResultItems (comparator step 3) prefers a closer-scope same-name candidate over an outer one when both are Applicable; a struct-local method is preferred over a free function of the same name accessed unqualified on a struct receiver. | functional | [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator) | [`overload-prefers-no-default-over-default-param.slang`](overload-prefers-no-default-over-default-param.slang) |
| CompareLookupResultItems (comparator step 3) prefers an override of an inherited member over the inherited member itself; a conforming-type override of an interface default fires through a generic dispatch. | functional | [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator) | [`override-of-interface-default-wins.slang`](override-of-interface-default-wins.slang) |
| Scope-distance step of the comparator (step 7) picks the closer-scope candidate when other ranking signals tie; a same-named function in the enclosing namespace at the use site beats a sibling-namespace one of equal cost. | functional | [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator) | [`overload-prefers-closer-scope.slang`](overload-prefers-closer-scope.slang) |
| The comparator's compareOverloadCandidateSpecificity step prefers a non-generic candidate over a generic one when both are Applicable with the same conversion cost. | functional | [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator) | [`overload-prefers-non-generic-over-generic.slang`](overload-prefers-non-generic-over-generic.slang) |

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

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator) | ambiguous-claim | The doc's `## Tie-breaking comparator` section claims step 5 `compareOverloadCandidateSpecificity` prefers "fewer default parameters > more". The actual resolver does not break ties on this dimension for the simple case of `pick(int)` vs `pick(int, int = 99)` called with one int: both candidates are considered equally good and the resolver emits ambiguous-call. | Either the comparator does not run this preference for default parameters in `Func` flavor, or the documented claim is too broad. `ambiguous-call-with-default-param-overload.slang` records the observed behavior; a precise claim would name the conditions under which the default-count preference actually fires (e.g. only for variadic vs non-variadic, or only for explicit generic applications). |
| [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator) | undocumented-behavior | The doc's `## Tie-breaking comparator` lists step 4 "Implicit conversion preference" — "If exactly one candidate is marked `ImplicitConversionModifier`, that one wins." The `__implicit_conversion` attribute is a core-module-only modifier; user code cannot decorate an overload with it from a `.slang` source. No user-observable test anchors to this claim, so it is recorded here as a doc gap (a claim that maps to a user-writable surface would unblock a test). |  |
| [#tie-breaking-comparator](../../../docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator) | undocumented-behavior | The doc's `## Tie-breaking comparator` step 6 names `getExportRank` and says `export` decls are preferred over `extern` decls. `export` and `extern` are module-system primitives that require multi-file compilation; `slang-test` runs each `.slang` file standalone (no `-r` resolution), so this preference is not observable from a single test file. A claim that names a single-file surface where `getExportRank` is observable would unblock a test. |  |
| [#operator-overloading](../../../docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading) | undocumented-behavior | The doc's `## Operator overloading` section names `ResolvedOperatorOverload` / `TypeCheckingCache::resolvedOperatorOverloadCache` and the invalidation by `cacheVersion`. The cache is a performance optimization "correctness does not depend on it", so there is no user-observable consequence: the same expression resolves to the same overload whether the cache hits or misses. No test anchors to the cache itself. |  |
| [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes) | undocumented-behavior | The doc's `## Edge cases and failure modes` section says an unresolved `PartiallyAppliedGenericExpr` reaching the lowerer is an "internal compiler error" (a correctness invariant on the resolver). The user-visible diagnostic is generic-argument- inference-failed at the resolver, which we cover; the lowerer's internal-error surface is not anchored. |  |
| [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | undocumented-behavior | The doc's `## Probe phase: TryCheckOverloadCandidate` step 7 (`TryCheckOverloadCandidateClassNewMatchUp`) is documented as finalize-only; its user-visible surface is the choice between `Class()` and `Class.new()` AST forms. There is no documented diagnostic that fires when this step rejects, and the surface is identical at the call site, so no test anchors here directly. |  |
| [#probe-phase-trycheckoverloadcandidate](../../../docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate) | undocumented-behavior | The doc's `## Probe phase: TryCheckOverloadCandidate` step 2 (`TryCheckOverloadCandidateFixity`) names catching "calling an infix operator in prefix position". Slang's surface does not let user code write a free `prefix` / `postfix` / `infix` marker on a function, so the fixity-failure path is not reachable from a `.slang` source. A claim that names a user-writable surface for fixity (or a confirmation that fixity is implicit) would unblock a test. |  |

## Out of scope (no-GPU runner)

None of the overload-resolution claims in this bundle require a GPU.
All tests use `//TEST:INTERPRET(filecheck=CHECK):` or
`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`, both of which run on the
no-GPU runner.
