---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T13:22:38+00:00
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 50f3927d8d5182d8b0ea76a1385cd5f0b05daac09fba7d68297998ea30f2809b
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/check-expr

## Intent

White-box characterization tests for `source/slang/slang-check-expr.cpp`
(expression checking; ~56% covered). These pin the **current observed
behaviour** of CLI-reachable expression-checking branches, not a spec.
Strategy: focus on the under-tested swizzle-checking paths
(`CheckSwizzleExpr` / `CheckMatrixSwizzleExpr`) and the error paths that
expression checking funnels distinct mistakes into (member-not-found,
non-l-value assignment, no-call-operator, non-array subscript, ambiguous
call, comma-operator warning). Error paths are pinned with
`DIAGNOSTIC_TEST` (which validate locally without FileCheck); positive
swizzle value/permutation behaviour is pinned with `INTERPRET`; the
target-dependent matrix-swizzle lowering shape is pinned with a `SIMPLE`
HLSL emit (FileCheck — ignored locally, validated in CI).

All emitted diagnostics, codes, and values were copied verbatim from
running the local `slangc` / `slang-test`; none are unverified, so no test
carries `characterization-unverified=true`.

## Functional coverage

| Test                                                                                 | What it pins (current behaviour)                                                                                                       | covers=                           |
| ------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------- |
| [`ambiguous-call-diag.slang`](ambiguous-call-diag.slang)                             | Two overloads that each win on one argument but not the other give an ambiguous-call error (E39999) plus per-candidate notes (E40011). | source/slang/slang-check-expr.cpp |
| [`call-operator-not-found-diag.slang`](call-operator-not-found-diag.slang)           | Invoking a struct value with no call operator (`s(3)`) reports "no call operation found for type 'S'" (E30016).                        | source/slang/slang-check-expr.cpp |
| [`comma-operator-warning-diag.slang`](comma-operator-warning-diag.slang)             | A comma sequence inside an expression emits the may-be-unintended warning (E41024) and still compiles.                                 | source/slang/slang-check-expr.cpp |
| [`matrix-swizzle-vector-result.slang`](matrix-swizzle-vector-result.slang)           | A multi-component matrix swizzle `m._m00_m11` lowers (HLSL) to a `float2(...)` built from per-element `[row][col]` matrix subscripts.  | source/slang/slang-check-expr.cpp |
| [`subscript-non-array-diag.slang`](subscript-non-array-diag.slang)                   | Subscripting a scalar (`x[0]` where x is float) reports "no subscript declarations found for type 'float'" (E30013).                   | source/slang/slang-check-expr.cpp |
| [`swizzle-duplicate-not-lvalue-diag.slang`](swizzle-duplicate-not-lvalue-diag.slang) | A swizzle with a repeated component (`v.xx`) is not an l-value, so assigning to it is rejected with E30011.                            | source/slang/slang-check-expr.cpp |
| [`swizzle-out-of-range-diag.slang`](swizzle-out-of-range-diag.slang)                 | A component beyond the vector width (`v.z` on float2) is rejected as a missing member (E30027), not a swizzle-specific error.          | source/slang/slang-check-expr.cpp |
| [`swizzle-reorder-and-dup-rvalue.slang`](swizzle-reorder-and-dup-rvalue.slang)       | `v.wzyx` reverses the components and `v.xxy` duplicates the first component when used as an r-value.                                   | source/slang/slang-check-expr.cpp |
| [`swizzle-rgba-alias.slang`](swizzle-rgba-alias.slang)                               | The r/g/b/a swizzle letters select the same elements as x/y/z/w.                                                                       | source/slang/slang-check-expr.cpp |
| [`swizzle-too-long-diag.slang`](swizzle-too-long-diag.slang)                         | A swizzle longer than four valid components (`v.xxxxx`) is rejected as a missing member (E30027).                                      | source/slang/slang-check-expr.cpp |

## Unreachable gaps

- `CheckMatrixSwizzleExpr`'s mixed zero/one indexing rejection
  (`zeroIndexOffset == 0` / `== 1` guards) and out-of-range matrix
  components both return `nullptr`, which the caller turns into the same
  E30027 "member not found" as the vector-swizzle and bad-character cases.
  Probed at the CLI (`m._m00_11`, `m._m22` on a 2x2) and confirmed
  observationally identical to `swizzle-out-of-range-diag` /
  `swizzle-too-long-diag`; not given separate tests because the only
  distinguishing token (`_m00_11` / `_m22` vs `xxxxx`) is the input text,
  not an output difference, so a separate test would re-pin the same
  behaviour with no extra signal.
- `Diagnostics::InternalCompilerError` sites in the member-access lowering
  (`slang-check-expr.cpp:3848`, `:3929`) assert on impossible
  post-resolution states (a resolved member ref whose decl is neither a
  value nor a callable). No CLI input drives them without first failing an
  earlier, user-facing diagnostic. Defensive — not targeted.
- The `MaximumTypeNestingLevelExceeded` and
  `GenericEvaluationRecursionLimitExceeded` limiter branches are reachable
  only with pathological deeply-nested types / recursive generic
  evaluation; they are stress-limit guards rather than ordinary
  expression-checking behaviour and are out of scope for this bundle.

## Doc gaps observed

NA
