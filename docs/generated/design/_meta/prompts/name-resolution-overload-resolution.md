# Prompt: name-resolution/overload-resolution.md

See [_common.md](_common.md) for the universal rules and the
**Name-resolution family contract** that applies to this page.

## Target

Produce `docs/generated/design/name-resolution/overload-resolution.md` —
the page that documents how a `LookupResult` containing multiple
candidates is narrowed to a single best candidate (or an unambiguous
error), including the per-step filter pipeline and the conversion-cost
ranking.

Audience: a developer modifying overload-resolution logic, adding a
new candidate flavor or a new filter step, or chasing an ambiguous-
overload diagnostic.

## Family-specific guidance

This page uses the `## Algorithm` heading.

The `## Concepts` section must cover, at minimum:

- `OverloadCandidate` — declared in
  [slang-check-impl.h](../../../../source/slang/slang-check-impl.h). Cite
  the `Flavor` enum values (`Func`, `Generic`, `UnspecializedGeneric`,
  `Expr`, plus whatever else is present at `source_commit`), the
  `Status` enum, and the `Flags` bit set. Document `retainedExpr` as the
  optional checked expression path from which candidate construction
  derived a candidate. Distinguish how completion uses that field for
  `Func`, fully or partially applied `Generic`, and `Expr` candidates.
- `OverloadResolveContext` — also in `slang-check-impl.h`. Cite its
  `mode` field and the `JustTrying` vs `ForReal` distinction; explain
  that `JustTrying` performs the same checks without emitting
  diagnostics, so the checker can probe several call shapes.
- `ConversionCost` — declared / used in
  [slang-check-conversion.cpp](../../../../source/slang/slang-check-conversion.cpp).
  Cite the enum and the line range.
- `CoercionSite` — also in `slang-check-conversion.cpp`. Cite it as
  the source-level context the cost is computed against.
- Anything else materially relevant in the watched headers
  (`InvokeExpr`, `OverloadedExpr`, `PartiallyAppliedGenericExpr`,
  `ResolvedOperatorOverload`).

The `## Algorithm` section must walk through the candidate filter
pipeline in order. Each step is a sub-bullet with the function name in
[slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp)
and a 1-3 sentence explanation:

1. `TryCheckOverloadCandidateArity` — arity check; how default
   parameters interact.
2. `TryCheckOverloadCandidateFixity` — operator-fixity check.
3. `TryCheckGenericOverloadCandidateTypes` — generic-parameter
   inference for a `Generic` candidate. Explain how failed inference records
   a separate `UnspecializedGeneric` candidate for later diagnostics.
4. Per-argument type and passing-mode check (find the actual function
   name; it may be `TryCheckOverloadCandidateTypes` or similar).
   Ordinary parameter modes such as `inout` belong to this candidate
   type-checking step; do not invent a separate direction phase.
   Name `TryCheckOverloadCandidateTypes` as the routine that obtains
   parameter information and calls `matchOverloadArgument` for each
   ordinary or pack argument. Attribute brace-initializer matching to
   `matchOverloadArgument`, not to data or to the candidate itself.
   Explain that trial matching records costs and mismatches without
   producing converted expressions. During the `ForReal` re-check,
   `matchOverloadArgument` calls `commitOverloadArgumentConversion`
   before callable-expression construction.
   That operation validates storage immediately for an ordinary call,
   but records storage uses for the later consumer of a type cast or
   compiler-synthesized initializer invocation.
5. `TryCheckOverloadCandidateConstraints` — validate generic
   constraints and resolve defaults.
6. `TryCheckOverloadCandidateVisibility` — visibility filter; cite the
   forward link to [visibility.md](visibility.md).
7. `TryCheckOverloadCandidateClassNewMatchUp` — match-up of the
   candidate's class-new pattern. State that this runs only during the
   `ForReal` finalize re-check, before the ordinary phase ladder; it is
   not a probe-phase filter.
8. Conversion-cost computation — cite the helper that sums the cost
   contributions per argument.

For each step, name the diagnostic family the step can produce (or
note "no diagnostic; silent rejection").

Include a mermaid flowchart showing the candidate filter pipeline.

Describe completion in execution order. `CompleteOverloadCandidate`
first runs the `ForReal` check ladder, including committed argument
conversions, and only then constructs the selected callable expression.
When a selected `Func` candidate has no `retainedExpr`, it calls
`ConstructLookupResultExpr` to replay the selected lookup item's
breadcrumbs.
Use an overloaded, interface-bounded member call to make that path
concrete: `_lookupMembersInSuperTypeFacets` records a `SuperType`
breadcrumb and subtype witness on each member's lookup item,
`createLookupResultExpr` produces a raw `OverloadedExpr`, and
`AddOverloadCandidates` therefore creates each candidate without a
`retainedExpr`.
`CompleteOverloadCandidate` is the consumer that replays the selected
item through `ConstructLookupResultExpr`.

After the probe and finalize-phase candidate checks, add a distinct
subsection for selected receiver validation. Start from a concrete call
to a mutating method through an immutable receiver. Explain that
`_lookUpInScopes` records only a structural `ThisValue` or `ThisType`
breadcrumb.
When a single lookup result is materialized before selection,
`ConstructLookupResultExpr` replays that path and `checkThisExpr`
determines the reconstructed receiver's type and value category;
for an instance callable, `checkThisExpr` reads the receiver information
that `checkAndAttachEffectiveThisParamInfo` published during signature
checking.
`CompleteOverloadCandidate` later preserves that retained expression
while rebuilding its direct callable leaf.
A candidate that has no `retainedExpr` is reconstructed from its lookup
item during completion instead.
Neither construction path validates the receiver against a callable.
Overload resolution
selects the callable without using receiver mutability, and final
selected-expression checking then calls `validateCallableReceiver`.
Receiver validation may reject the selected expression, but it does not
remove or rank candidates and does not trigger fallback to another
overload.

The page must also contain a level-2 section `## Conversion costs`
covering:

- The `ConversionCost` enum levels in order (cheapest to most
  expensive).
- How costs are summed across arguments.
- How ties are broken: argument-level priority, partial-ordering of
  candidates, etc. Cite the comparator in `slang-check-overload.cpp`
  or `slang-check-conversion.cpp`.

A second level-2 section `## Partial generic application` covering:

- `IsPartiallyAppliedGeneric` flag.
- `PartiallyAppliedGenericExpr` (cite in
  [slang-ast-expr.h](../../../../source/slang/slang-ast-expr.h)).
- When the resolver returns this instead of fully resolving.

A third level-2 section `## Operator overloading` covering:

- The builtin-operator fast path. Note that the `ResolvedOperatorOverload`
  cache this prompt used to ask for **no longer exists** — it was removed
  and replaced by `convertToBuiltinArithmeticOp` and its helpers, declared
  in [slang-check-impl.h](../../../../source/slang/slang-check-impl.h)
  (lines 3927-3982) and defined in
  [slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp).
  The fast path lets builtin scalar/vector/matrix operators bypass general
  overload resolution, producing a `BuiltinOperatorExpr` that carries a
  `BuiltinOperationKind`. Do not describe a memoization cache.
- How source infix, prefix, and postfix operators are found as free functions
  in lexical scope.
- How member-resolved operators such as `operator()` keep the callee
  value as an explicit base expression. Any lookup breadcrumbs record
  only additional structural navigation; receiver mutability is
  validated only after overload selection.

The `## Edge cases and failure modes` section must cover:

- Two candidates with identical conversion cost — what
  ambiguous-overload diagnostic is produced; cite the diagnostic
  identifier from
  [slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h).
- No candidate matches — `noOverloadFound`-style diagnostic; what
  near-match information is reported.
- A candidate that matches except for visibility — whether it is
  reported as "filtered" with a hint or silently dropped.
- Implicit-conversion explosion: a candidate whose arguments each
  require a chain of conversions; how the cost cap (if any) handles
  this.
- Generic-argument inference failure — what `Status` value the
  candidate ends up with and how that surfaces.

## Forbidden content (in addition to the universal forbidden list and the family contract)

- The lookup algorithm itself — link [lookup.md](lookup.md).
- Visibility rules in detail — link [visibility.md](visibility.md);
  this page references only the `TryCheckOverloadCandidateVisibility`
  step.
- IR-level lowering of resolved overloads — that belongs in
  [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every `TryCheckOverloadCandidate*` function name is verified
      against
      [slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp)
      at `source_commit`; do not invent a step.
- [ ] Every `OverloadCandidate::Flavor` value cited exists in
      [slang-check-impl.h](../../../../source/slang/slang-check-impl.h).
- [ ] The conversion-cost enum levels are listed in the same order as
      declared in `slang-check-conversion.cpp`.
- [ ] No claim about partial-ordering or tie-breaking that is not
      grounded in a cited comparator.
