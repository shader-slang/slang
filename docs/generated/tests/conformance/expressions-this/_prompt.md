# Prompt: docs/generated/tests/conformance/expressions-this/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/expressions-this/`,
anchored to
[`docs/language-reference/expressions-this.md`](../../../../language-reference/expressions-this.md).

## Triage result

The doc is very short (~16 lines, one normative section "Mutability") and carries a `> TODO`
banner. Triage confirmed 5 claims total, 3 of which are testable in this bundle. The threshold
is met (>= 3 testable normative claims); bundle proceeds.

## Sub-areas and claim extraction strategy

### Introduction / identity (`#this-expression`)

Two implicit claims from the opening paragraph:

- `this` refers to the implicit instance of the enclosing type (identity claim).
- The type of `this` is `This` (type claim).

Both are already exercised by `conformance/types-struct` (C26 / C44). They are recorded in
`## Untested claims` as `out-of-bundle` with links to the covering tests. No new tests are
written for them here — doing so would duplicate rather than add coverage.

### Mutability (`#mutability`)

Three normative sentences:

1. "If a `[mutating]` instance is being called, the argument for the implicit `this` parameter
   must be an l-value." — Testable both positively (l-value receiver → mutation observable)
   and negatively (r-value receiver → E30050).

2. "The argument expressions corresponding to any `out` or `in out` parameters of the callee
   must be l-values." — Testable positively (l-value variable → callee write observable) and
   negatively for both `out` and `inout` (r-value → E30047).

3. "A call expression is never an l-value." — Testable negatively (assign to call result → E30011).

## Claim-to-test mapping

Each sentence in the Mutability section yields positive + negative tests (or negative only when
there is no distinct observable difference from other positive tests). Test count is 6 files.

## What counts as a normative claim here

Any "must" or "is never" sentence in the Mutability section. The opening paragraph is
definitional/identity — it does not add testable constraints beyond what types-struct already
covers. The `> TODO` banner is non-normative meta-commentary about doc completeness.

## Anchors used

All tests anchor to `docs/language-reference/expressions-this.md#mutability` because the doc
has only one explicitly named section ("Mutability" normalized to `#mutability`). The opening
paragraph has no explicit heading anchor; the closest parent is the document root, which is
treated as `#this-expression` for cross-reference purposes.
