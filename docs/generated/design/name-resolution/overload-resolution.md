---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-05T10:25:25+00:00
source_commit: 52339028a2aa703271533454c6b9528a534bac31
watched_paths_digest: 4b84c7f99f5cc6586a2852af0165acc8c35f69765bd0593a6595e5852f1750ec
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Overload Resolution

This document specifies how Slang narrows a `LookupResult` containing
multiple candidates to a single best candidate (or to a structured
ambiguity error). It covers the candidate filter pipeline, the
conversion-cost ranking, partial generic application, operator
overloading, and how failures are reported.

The intended reader is a developer modifying overload-resolution
logic, adding a new candidate flavor or a new filter step, or chasing
an ambiguous-overload diagnostic.

Where overload candidates come from is described in
[lookup.md](lookup.md). The visibility filter that interleaves with
this pipeline is described in [visibility.md](visibility.md).

## Source

The candidate type, status, flags, and resolve-context are declared
in [slang-check-impl.h](../../../../source/slang/slang-check-impl.h)
(`OverloadCandidate` line 272, `OverloadResolveContext` line 2988,
`ResolvedOperatorOverload` line 331, `TypeCheckingCache` line 346).
The filter pipeline, candidate construction, and comparator are in
[slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp).
The `ConversionCost` enum and conversion-cost utilities live in
[slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
(lines 89-170+) and
[slang-check-conversion.cpp](../../../../source/slang/slang-check-conversion.cpp).
`PartiallyAppliedGenericExpr` is in
[slang-ast-expr.h](../../../../source/slang/slang-ast-expr.h) line 951.

## Concepts

- `OverloadCandidate`
  ([slang-check-impl.h line
  272](../../../../source/slang/slang-check-impl.h)) — one candidate the
  resolver is evaluating. Key fields:
  - `Flavor flavor` — one of `Func`, `Generic`,
    `UnspecializedGeneric`, `Expr` (lines 274-280).
  - `Status status` — pipeline progress; one of
    `GenericArgumentInferenceFailed`, `Unchecked`, `ArityChecked`,
    `FixityChecked`, `TypeChecked`, `DirectionChecked`,
    `VisibilityChecked`, `Applicable` (lines 283-294).
  - `Flags flags` — bitset; today only `IsPartiallyAppliedGeneric =
    1 << 0` (lines 296-301).
  - `LookupResultItem item` — the underlying `DeclRef` + breadcrumb
    chain returned from lookup (line 304).
  - `Expr* exprVal` — for `Flavor::Expr` candidates (e.g. a function
    value passed as an argument).
  - `FuncType* funcType` — function type when the candidate is a
    function value rather than a declared callable.
  - `Type* resultType` — the result type of the call if this
    candidate is chosen.
  - `ConversionCost conversionCostSum` — the per-argument
    implicit-conversion costs accumulated by
    `TryCheckOverloadCandidateTypes`.
  - `SubstitutionSet subst` — the inferred substitution; used by
    generic candidates to avoid re-running inference in
    `CompleteOverloadCandidate`.
- `OverloadResolveContext`
  ([slang-check-impl.h line
  2988](../../../../source/slang/slang-check-impl.h)) — the bundle of
  call-site state passed to every filter step. Key fields:
  - `Mode mode` — `JustTrying` or `ForReal` (lines 2990-2997).
    `JustTrying` silently rejects bad candidates; `ForReal` emits
    diagnostics for each rejection. Most of pipeline runs in
    `JustTrying` to score candidates; `CompleteOverloadCandidate`
    switches to `ForReal` once the best candidate is chosen.
  - `Scope* sourceScope` — the requesting scope, used by the
    visibility step (line 3009).
  - `Index argCount`, `List<Expr*>* args`, `Type** argTypes` — call-
    site arguments (lines 3012-3014).
  - `OverloadCandidate* bestCandidate`,
    `List<OverloadCandidate> bestCandidates` — the running winner
    plus the equally-best siblings if there is an ambiguity (lines
    3055-3059).
- `ConversionCost`
  ([slang-ast-support-types.h line
  89](../../../../source/slang/slang-ast-support-types.h)) — `unsigned
  int`. Specific levels are defined as `kConversionCost_*`
  enumerators (lines 90-170+), summing across arguments. Threshold
  `kConversionCost_GeneralConversion` (900) is the implicit-
  conversion ceiling: anything at or above is rejected for implicit
  use by `canConvertImplicitly`
  ([slang-check-conversion.cpp line
  3135](../../../../source/slang/slang-check-conversion.cpp)).
  `kConversionCost_Explicit` (90000) is the "explicit cast only"
  marker; `kConversionCost_Impossible` represents "no conversion
  exists".
- `CoercionSite`
  ([slang-check-impl.h line
  355](../../../../source/slang/slang-check-impl.h)) — `General`,
  `Assignment`, `Argument`, `Return`, `Initializer`,
  `ExplicitCoercion`. Cost computation can vary per site (for
  example, an explicit cast at `ExplicitCoercion` permits the
  expensive `kConversionCost_Explicit` conversions).
- `ResolvedOperatorOverload` /
  `TypeCheckingCache::resolvedOperatorOverloadCache`
  ([slang-check-impl.h lines
  331-353](../../../../source/slang/slang-check-impl.h)) — per-`Linkage`
  cache that memoizes operator overload resolution by operand types,
  so a hot expression like `a + b` does not re-run the filter
  pipeline on every encounter.
- `PartiallyAppliedGenericExpr`
  ([slang-ast-expr.h line
  951](../../../../source/slang/slang-ast-expr.h)) — the AST node
  representing a generic that has been bound to some but not all of
  its parameters; the resolver produces one when a candidate is
  matched but not enough type information is available to
  monomorphize it.

## Algorithm

The resolver runs in two phases: a *probe* phase that scores every
candidate in `JustTrying` mode, and a *finalize* phase that re-runs
the pipeline on the winning candidate in `ForReal` mode and produces
the AST node for the call.

### Probe phase: `TryCheckOverloadCandidate`

`SemanticsVisitor::TryCheckOverloadCandidate`
([slang-check-overload.cpp lines
1240-1268](../../../../source/slang/slang-check-overload.cpp)) is the
inner driver. It advances `candidate.status` step-by-step, returning
as soon as any step fails:

```mermaid
flowchart TB
  arity["TryCheckOverloadCandidateArity"]
  fix["TryCheckOverloadCandidateFixity"]
  types["TryCheckOverloadCandidateTypes (incl. generic inference)"]
  dirs["TryCheckOverloadCandidateDirections"]
  cstr["TryCheckOverloadCandidateConstraints"]
  vis["TryCheckOverloadCandidateVisibility"]
  applicable["Status = Applicable"]
  arity --> fix --> types --> dirs --> cstr --> vis --> applicable
  arity -. fail .-> reject1["candidate rejected at Status = Unchecked"]
  fix -. fail .-> reject2["candidate rejected at Status = ArityChecked"]
  types -. fail .-> reject3["candidate rejected at Status = FixityChecked"]
  dirs -. fail .-> reject4["candidate rejected at Status = TypeChecked"]
  cstr -. fail .-> reject5["candidate rejected at Status = DirectionChecked"]
  vis -. fail .-> reject6["candidate rejected at Status = VisibilityChecked"]
```

The individual steps:

1. **`TryCheckOverloadCandidateArity`**
   ([slang-check-overload.cpp line
   145](../../../../source/slang/slang-check-overload.cpp)) — verifies
   the call has between `required` and `allowed` arguments. Trailing
   defaults can pad up to `allowed`; variadic parameters allow any
   count >= `required`. Failures in `ForReal` mode emit
   `not-enough-arguments-for-call` /
   `too-many-arguments-for-call`;
   in `JustTrying` mode the candidate is dropped silently.
2. **`TryCheckOverloadCandidateFixity`**
   ([slang-check-overload.cpp line
   225](../../../../source/slang/slang-check-overload.cpp)) — for
   operator calls only, checks that prefix / postfix / infix
   modifiers on the candidate match the call form. Catches things
   like calling an `infix` operator in prefix position.
3. **`TryCheckOverloadCandidateTypes`**
   ([slang-check-overload.cpp line
   752](../../../../source/slang/slang-check-overload.cpp)) — the
   per-argument type/coercion check.
   - For `Flavor::Generic` and `Flavor::UnspecializedGeneric`, the
     step delegates to `TryCheckGenericOverloadCandidateTypes`
     ([slang-check-overload.cpp line
     299](../../../../source/slang/slang-check-overload.cpp)) to
     infer the missing generic arguments. Inference failure sets
     `Status::GenericArgumentInferenceFailed` and returns false.
   - For every argument, the step computes
     `getImplicitConversionCostWithKnownArg`
     ([slang-check-conversion.cpp line
     1614](../../../../source/slang/slang-check-conversion.cpp)) and
     accumulates the cost into `candidate.conversionCostSum`.
   - When `canConvertImplicitly` rejects an argument's cost, the
     candidate is dropped (silently in `JustTrying`, with a
     conversion-error diagnostic in `ForReal`).
4. **`TryCheckOverloadCandidateDirections`**
   ([slang-check-overload.cpp line
   997](../../../../source/slang/slang-check-overload.cpp)) — enforces
   parameter directions (`in`, `out`, `inout`, `ref`). An `out`
   parameter must receive an l-value; passing a literal to
   `inout` fails here.
5. **`TryCheckOverloadCandidateConstraints`**
   ([slang-check-overload.cpp line
   1071](../../../../source/slang/slang-check-overload.cpp)) — for an
   explicit generic application `G<A, B>`, this step is where the
   `where`-clauses on `G`'s parameters are validated against the
   inferred substitutions.
6. **`TryCheckOverloadCandidateVisibility`**
   ([slang-check-overload.cpp lines
   265-287](../../../../source/slang/slang-check-overload.cpp)) —
   delegates to
   `isDeclVisibleFromScope(candidate.item.declRef, context.sourceScope)`.
   In `ForReal` mode emits diagnostic `decl-is-not-visible`
   (`slang-diagnostics.lua` 30600); in `JustTrying` mode just
   returns false. See [visibility.md](visibility.md) for the rule.
7. **`TryCheckOverloadCandidateClassNewMatchUp`**
   ([slang-check-overload.cpp lines
   107-143](../../../../source/slang/slang-check-overload.cpp)) —
   runs only in the finalize / `ForReal` re-check inside
   `CompleteOverloadCandidate`, not in the probe phase. It
   distinguishes `Class()` / `Class.new()` syntax from a plain
   constructor call so the right AST form is emitted. See
   "Finalize phase" below for the full sequencing.

A candidate that survives every probe-phase step is tagged
`Status::Applicable` with its total `conversionCostSum` populated,
ready for ranking. `TryCheckOverloadCandidateClassNewMatchUp` is
*not* part of probe-phase filtering; it gates only the
finalize-phase AST construction.

`AddOverloadCandidateInner`
([slang-check-impl.h line
3162](../../../../source/slang/slang-check-impl.h)) is the call site
that runs `TryCheckOverloadCandidate` and either:

- discards the candidate if it failed before any candidate has yet
  reached `Applicable`;
- adopts it as `context.bestCandidate` if its `status` /
  `conversionCostSum` strictly beat the current best (via
  `CompareOverloadCandidates`);
- appends it to `context.bestCandidates` if it is tied with the
  current best;
- discards it if it is strictly worse.

### Probe phase: where candidates come from

The resolver populates candidate sets via family-specific helpers
in [slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp):

- `AddDeclRefOverloadCandidates` — iterates a `LookupResult` of
  function decls and runs each through the pipeline.
- `AddFuncOverloadCandidate(LookupResultItem, DeclRef<CallableDecl>, ..., baseCost)`
  (line 2241) — single-callable variant.
- `AddCtorOverloadCandidate` (line 2343) — calls that go through a
  `ConstructorDecl`. The owning `Type` is captured in the candidate
  so the resolver can produce a constructor-invocation expression.
- `AddFuncOverloadCandidate(FuncType*, ..., baseCost)` — first-class
  function values; the candidate's `funcType` is set and `flavor`
  is `Expr`.
- `AddHigherOrderOverloadCandidates` — `__fwd_diff`,
  `__bwd_diff`-style operators that wrap a callee.

Each helper accepts a `baseCost` (`ConversionCost`) that is added to
the candidate's accumulated cost. Member-access lookups bias against
deep base-class hits and pointer auto-derefs by adding small
`kConversionCost_*` increments here.

### Finalize phase: `CompleteOverloadCandidate`

Once the probe phase finishes with exactly one
`context.bestCandidate`, `CompleteOverloadCandidate`
([slang-check-overload.cpp lines
1310-1520+](../../../../source/slang/slang-check-overload.cpp)) flips
`context.mode = ForReal` and re-runs the full pipeline starting from
`TryCheckOverloadCandidateClassNewMatchUp`
([slang-check-overload.cpp lines
107-143](../../../../source/slang/slang-check-overload.cpp)). The
`ClassNewMatchUp` step is only required in `ForReal` mode: it
distinguishes `Class()` / `Class.new()` syntax from a plain
constructor call so the right AST node is emitted.

The `ForReal` re-check is what produces user-visible diagnostics for
the chosen candidate. If even the best candidate fails (e.g. its
`Status::GenericArgumentInferenceFailed` was the *least bad* score),
the path emits `generic-argument-inference-failed` (`slang-
diagnostics.lua` 39999) together with a
`generic-signature-tried` note.

After `ForReal` succeeds, `CompleteOverloadCandidate` constructs the
final AST node:

- `Flavor::Func` or `Flavor::Generic` ->
  `ConstructLookupResultExpr` + the call form
  (`InvokeExpr` / `MemberExpr.invoke`).
- `Flavor::UnspecializedGeneric` -> wraps the result in a
  `PartiallyAppliedGenericExpr` when the
  `IsPartiallyAppliedGeneric` flag is set.
- `Flavor::Expr` -> uses `candidate.exprVal` directly as the callee.

## Conversion costs

The `ConversionCost` enum
([slang-ast-support-types.h line
89](../../../../source/slang/slang-ast-support-types.h)) is `unsigned
int`. Each per-argument conversion is first vetted by
`canConvertImplicitly`; only conversions it accepts contribute to
the candidate's ranking sum. The full enum, in source-declaration
order:

| Constant | Numeric | Meaning |
| --- | --- | --- |
| `kConversionCost_None` | 0 | identity |
| `kConversionCost_GenericParamUpcast` | 1 | up-cast through a generic parameter |
| `kConversionCost_LambdaToFunc` | 1 | lambda used where a `Func` value is expected |
| `kConversionCost_UnconstraintGenericParam` | 20 | binding to an unconstrained generic parameter |
| `kConversionCost_SizedArrayToUnsizedArray` | 30 | sized -> unsized array |
| `kConversionCost_MatrixLayout` | 5 | matrix layout adapter |
| `kConversionCost_GetRef` | 5 | extracting a reference from a buffer-like type |
| `kConversionCost_ImplicitDereference` | 10 | dereferencing a pointer-like value |
| `kConversionCost_InRangeIntLitConversion` | 23 | int literal fits in target integer type |
| `kConversionCost_InRangeIntLitSignedToUnsignedConversion` | 32 | signed lit -> unsigned target |
| `kConversionCost_InRangeIntLitUnsignedToSignedConversion` | 81 | unsigned lit -> signed target |
| `kConversionCost_MutablePtrToConstPtr` | 20 | mutable ptr -> const ptr |
| `kConversionCost_CastToInterface` | 50 | concrete type -> conforming interface |
| `kConversionCost_BoolToInt` | 120 | `bool` -> int (deliberately cheaper to break ties) |
| `kConversionCost_RankPromotion` | 150 | rank-preserving numeric promotion |
| `kConversionCost_NoneToOptional` | 150 | none -> Optional |
| `kConversionCost_ValToOptional` | 150 | T -> Optional |
| `kConversionCost_NullPtrToPtr` | 150 | nullptr -> ptr |
| `kConversionCost_PtrToVoidPtr` | 150 | T* -> void* |
| `kConversionCost_FailedOptionalConstraint` | 150 | optional constraint did not match |
| `kConversionCost_UnsignedToSignedPromotion` | 200 | promoting unsigned to wider signed |
| `kConversionCost_SameSizeUnsignedToSignedConversion` | 300 | same-size unsigned -> signed |
| `kConversionCost_SignedToUnsignedConversion` | 250 | signed -> unsigned of same/greater size |
| `kConversionCost_IntegerToFloatConversion` | 400 | int -> float |
| `kConversionCost_PtrToBool` | 400 | pointer -> bool |
| `kConversionCost_IntegerTruncate` | 450 | int -> narrower int |
| `kConversionCost_IntegerToHalfConversion` | 500 | int -> half |
| `kConversionCost_ParameterPack` | 500 | binding to a parameter pack |
| `kConversionCost_Default` | 500 | user-defined conversion default |
| `kConversionCost_GeneralConversion` | 900 | implicit ceiling (anything `>=` rejected by `canConvertImplicitly`) |
| `kConversionCost_Explicit` | 90000 | explicit cast only; never accepted implicitly |
| `kConversionCost_OneVectorToScalar` | 1 | additive when downcasting a 1-vector to a scalar |
| `kConversionCost_ScalarToVector` | 2 | additive when promoting a scalar to a vector |
| `kConversionCost_ScalarToMatrix` | 10 | additive when promoting a scalar to a matrix |
| `kConversionCost_ScalarIntegerToFloatMatrix` | 410 | `kConversionCost_IntegerToFloatConversion + kConversionCost_ScalarToMatrix`. |
| `kConversionCost_ScalarToCoopVector` | 1 | additive when promoting a scalar to a cooperative vector |
| `kConversionCost_LValueCast` | 800 | additive when casting an l-value |
| `kConversionCost_TypeCoercionConstraint` | 1000 | cost contributed by a type-coercion constraint |
| `kConversionCost_TypeCoercionConstraintPlusScalarToVector` | 1002 | `kConversionCost_TypeCoercionConstraint + kConversionCost_ScalarToVector`. |
| `kConversionCost_Impossible` | `0xFFFFFFFF` | "no conversion exists"; never summed because the candidate is rejected before reaching ranking |

`canConvertImplicitly`
([slang-check-conversion.cpp line
3135](../../../../source/slang/slang-check-conversion.cpp)) is the
binary "is this allowed implicitly" predicate: anything cheaper than
`kConversionCost_GeneralConversion` is allowed implicitly, anything
at or above is not. Cost levels beyond `Explicit` exist for
discouraged conversions that must remain reachable (some test
infrastructure compares against `kConversionCost_Impossible`).

### Tie-breaking comparator

`CompareOverloadCandidates`
([slang-check-overload.cpp lines
2009-2173+](../../../../source/slang/slang-check-overload.cpp)) is a
total-ordering comparator on two candidates of the same `Status`:

1. **Status difference.** A candidate with a higher `Status` wins
   (so an `Applicable` candidate is always preferred to an
   `ArityChecked` one).
2. **Conversion-cost sum.** Lower `conversionCostSum` wins.
3. **`CompareLookupResultItems`**
   ([slang-check-overload.cpp line
   1617](../../../../source/slang/slang-check-overload.cpp)) — lexical
   "how was the candidate found" comparison. Prefers shadowing
   members of a closer scope over an outer one; prefers an override
   of an inherited member.
4. **Implicit conversion preference.** If exactly one candidate is
   marked `ImplicitConversionModifier`, that one wins. This is what
   lets a user-supplied `__implicit_conversion` overload be selected
   in preference to a builtin one with the same cost.
5. **`compareOverloadCandidateSpecificity`**
   ([slang-check-overload.cpp line
   1843](../../../../source/slang/slang-check-overload.cpp)) — structural
   preference: non-generic > generic, non-variadic > variadic, fewer
   default parameters > more.
6. **`getExportRank`** — `export` decls are preferred to `extern`
   decls of the same name.
7. **Scope distance.** For non-generic flavors, the comparator
   computes the lexical    distance from the call site to each
   declaration and prefers the closer one. The comment at
   [slang-check-overload.cpp lines
   2106-2128](../../../../source/slang/slang-check-overload.cpp)
   explains why this step is skipped for generic candidates: the
   first pass of generic-candidate filtering is keyed on parameter
   list shape rather than actual applicability, so applying scope
   distance there would prefer the wrong candidate.

If every step returns zero, the candidates are considered equally
good; the caller will eventually emit an ambiguous-overload
diagnostic.

## Partial generic application

When the call expression supplies arguments that pin down some but
not all of a generic's parameters,
`TryCheckGenericOverloadCandidateTypes`
([slang-check-overload.cpp lines
299+](../../../../source/slang/slang-check-overload.cpp)) may produce a
candidate that is *partially specialized*. In that case the helpers
set `candidate.flags |= OverloadCandidate::Flag::IsPartiallyAppliedGeneric`
at four sites in [slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp)
(lines 406, 483, 571, 658), and `CompleteOverloadCandidate`
wraps the result in `PartiallyAppliedGenericExpr`
([slang-check-overload.cpp lines
1466-1470](../../../../source/slang/slang-check-overload.cpp)).

A `PartiallyAppliedGenericExpr` carries the bound substitution and
the unresolved generic parameters; the surrounding context (for
example, an explicit type ascription, a later
`GenericAppExpr<...>`, or another argument that pins the type) is
expected to close the remaining holes. Overload resolution treats a
fully resolved generic application as an invariant once checking
completes; how an unresolved residual is handled by later phases is
documented in
[../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md).

## Operator overloading

Operator expressions
(`+`, `-`, `*`, `[]`, `==`, etc.) reach overload resolution through
the same `OverloadResolveContext` machinery as named calls, but with
two extras:

- The candidate set is collected via member lookup on the operands'
  types using the operator's name (the parser maps `a + b` to a call
  expression with the appropriate operator-keyed `Name`).
- A `ResolvedOperatorOverload`
  ([slang-check-impl.h line
  331](../../../../source/slang/slang-check-impl.h)) cache memoizes the
  result, keyed by `OperatorOverloadCacheKey` (which captures the
  operator name and the operand `BasicType` pair). The cache lives
  in `TypeCheckingCache::resolvedOperatorOverloadCache`
  ([slang-check-impl.h line
  348](../../../../source/slang/slang-check-impl.h)) and is populated /
  consulted in
  [slang-check-overload.cpp lines
  3093 and 3355](../../../../source/slang/slang-check-overload.cpp).
  Each cached entry carries a `cacheVersion` because
  `OverloadCandidate` values are not portable across linkages; a
  newer linkage invalidates the entry by bumping
  `TypeCheckingCache::version`.

Implicit `this` is supplied automatically for non-static member
operator overloads: the lookup that produced the candidate left a
`Breadcrumb::Kind::This` step on the item, and
`CompleteOverloadCandidate` consumes it when constructing the
`InvokeExpr`. The `ThisParameterMode` recorded on the breadcrumb
(see [lookup.md](lookup.md)) is what makes a `[mutating]` operator
overload reject a `const` left operand.

## Edge cases and failure modes

- **Two `Applicable` candidates with identical cost / specificity /
  scope.** `CompareOverloadCandidates` returns 0; the candidate is
  appended to `context.bestCandidates`. After the probe phase the
  caller emits `ambiguous-overload-for-name-with-args`
  (`slang-diagnostics.lua` 39999) or
  `ambiguous-overload-with-args` (39999), depending on whether the
  callee name is known. Each tied candidate is shown as a separate
  note.
- **No candidate is `Applicable`.** When a single candidate scores
  best, its `Status` says which step failed first and the resolver
  re-runs the pipeline in `ForReal` mode on that candidate (via
  `CompleteOverloadCandidate`) so the user sees the most-specific
  diagnostic (a per-argument coercion failure rather than a generic
  "no overload" message). When several non-applicable candidates tie
  for best, the resolver instead emits
  `NoApplicableOverloadForNameWithArgs` (or `NoApplicableWithArgs`
  when the callee name is unknown) directly, without calling
  `CompleteOverloadCandidate`
  ([slang-check-overload.cpp lines
  3216-3232](../../../../source/slang/slang-check-overload.cpp)).
- **Generic-argument inference failure.** The candidate ends at
  `Status::GenericArgumentInferenceFailed`.
  `CompleteOverloadCandidate` emits
  `generic-argument-inference-failed` together with a
  `generic-signature-tried` note that prints the generic signature
  via `ASTPrinter::getDeclSignatureString`
  ([slang-check-overload.cpp lines
  1314-1326](../../../../source/slang/slang-check-overload.cpp)).
- **A candidate matches but is hidden by visibility.** In
  `JustTrying` it is silently dropped; in `ForReal` it emits
  `decl-is-not-visible` (30600). Other candidates of lower
  applicability may still win, so the user sometimes sees the
  invisible-decl diagnostic followed by acceptance of a different
  candidate.
- **Argument that needs a chain of conversions.** Each argument's
  conversion must first be accepted by `canConvertImplicitly`
  ([slang-check-conversion.cpp lines
  3135-3140](../../../../source/slang/slang-check-conversion.cpp)),
  which rejects anything at or above
  `kConversionCost_GeneralConversion` (900). Only the per-argument
  costs that pass this check are then summed into the candidate's
  `conversionCostSum` for ranking ([slang-check-overload.cpp lines
  806-827](../../../../source/slang/slang-check-overload.cpp)) — that
  sum has no separate ceiling.
- **First-class function value vs declared callable of the same
  signature.** Both are `Applicable` with identical
  `conversionCostSum`; `CompareLookupResultItems` and
  `compareOverloadCandidateSpecificity` are the deciding factors —
  the function value typically loses to the declared overload
  because the declared overload is closer in scope.
- **Generic candidate plus non-generic candidate.** Scope-distance
  tie-breaking is *skipped* when at least one candidate is
  `Generic` or `UnspecializedGeneric`
  ([slang-check-overload.cpp lines
  2122-2127](../../../../source/slang/slang-check-overload.cpp)). Other
  steps (cost, specificity) still apply; if they tie, the candidates
  are reported ambiguous.
- **Operator overload cache stale.** A newer
  `TypeCheckingCache::version` invalidates older entries; the
  resolver recreates the candidate from the cached `decl`. The
  cache is a performance optimization only — correctness does not
  depend on it.
- **`PartiallyAppliedGenericExpr` left unresolved.** Overload
  resolution relies on the surrounding context closing the remaining
  generic holes; leaving a residual unresolved is a correctness
  invariant violation, not a feature. Downstream handling is
  described in
  [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md).

## See also

- [lookup.md](lookup.md) — the lookup that produces the candidate
  set the resolver narrows.
- [visibility.md](visibility.md) — the visibility filter integrated
  with `TryCheckOverloadCandidateVisibility`.
- [scopes.md](scopes.md) — the scope chain that determines the
  lexical distance used in tie-breaking.
- [../ast-reference/expressions.md](../ast-reference/expressions.md)
  — per-class reference for `InvokeExpr`,
  `PartiallyAppliedGenericExpr`, `OverloadedExpr`,
  `GenericAppExpr`.
- [../ast-reference/values.md](../ast-reference/values.md) — the
  `Val` family that backs `SubstitutionSet` and witness arguments
  used during generic inference.
- [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)
  — pipeline-level overview of where overload resolution sits.
- [../glossary.md](../glossary.md) — entries for
  `overload resolution`, `conversion cost`,
  `partial generic application`, `decl-ref`, `lookup result`.
