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
  `Status` enum, and the `Flags` bit set.
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

1. `TryCheckOverloadCandidateClassNewMatchUp` — match-up of the
   candidate's class-new pattern (or whatever the actual purpose is at
   `source_commit`; verify before writing).
2. `TryCheckOverloadCandidateArity` — arity check; how default
   parameters interact.
3. `TryCheckOverloadCandidateFixity` — operator-fixity check.
4. `TryCheckGenericOverloadCandidateTypes` — generic-parameter
   inference; only relevant when `OverloadCandidate::Flavor` is
   `Generic` or `UnspecializedGeneric`.
5. Per-argument type / direction check (find the actual function
   name; it may be `TryCheckOverloadCandidateTypes` or similar).
6. `TryCheckOverloadCandidateVisibility` — visibility filter; cite the
   forward link to [visibility.md](visibility.md).
7. Conversion-cost computation — cite the helper that sums the cost
   contributions per argument.

For each step, name the diagnostic family the step can produce (or
note "no diagnostic; silent rejection").

Include a mermaid flowchart showing the candidate filter pipeline.

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

- The `ResolvedOperatorOverload` cache (cite its declaration).
- How operator lookup specializes from the general algorithm.
- The implicit `this` argument for binary / unary operator overloads.

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
