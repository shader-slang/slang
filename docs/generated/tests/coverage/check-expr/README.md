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

The 2026-08-05 deepening pass added twelve tests covering 138 further
lines of `slang-check-expr.cpp` that the profiled baseline reported as
never executed. It targeted areas the hand-written suite leaves alone
rather than more swizzle variants: the `spirv_asm` operand/id resolution
ladder, error-handling (`try`) shape rules, `__dispatch_kernel` argument
validation, `__return_val` outside a non-copyable return, type-modifier
rejection (`const`, `row_major` on a scalar), the subscript arity guard,
and the tuple-swizzle bounds check.

A caveat for whoever re-profiles this bundle: the coverage profile the
pass started from under-reports what `tests/` already covers (constructs
demonstrably exercised by `tests/`, such as the shape-pack transforms and
the `float-as-int` bit-cast mismatch, appear as zero-count in it). Every
target below was therefore additionally checked by grepping `tests/` for
the diagnostic text or the syntax before the test was written, and each
test's coverage delta was re-measured individually against the baseline
profile. Prefer that two-step check over trusting the profile alone.

## Functional coverage

| Test                                                                                             | What it pins (current behaviour)                                                                                                       | covers=                           |
| ------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------- |
| [`ambiguous-call-diag.slang`](ambiguous-call-diag.slang)                                         | Two overloads that each win on one argument but not the other give an ambiguous-call error (E39999) plus per-candidate notes (E40011). | source/slang/slang-check-expr.cpp |
| [`call-operator-not-found-diag.slang`](call-operator-not-found-diag.slang)                       | Invoking a struct value with no call operator (`s(3)`) reports "no call operation found for type 'S'" (E30016).                        | source/slang/slang-check-expr.cpp |
| [`comma-operator-warning-diag.slang`](comma-operator-warning-diag.slang)                         | A comma sequence inside an expression emits the may-be-unintended warning (E41024) and still compiles.                                 | source/slang/slang-check-expr.cpp |
| [`const-type-modifier-diag.slang`](const-type-modifier-diag.slang)                               | `typedef const int` reports E20018 and the modifier is dropped, so the alias still checks as plain `int` with no follow-on error.      | source/slang/slang-check-expr.cpp |
| [`dispatch-kernel-size-type-mismatch-diag.slang`](dispatch-kernel-size-type-mismatch-diag.slang) | `__dispatch_kernel` checks both size arguments independently and reports each against `vector<uint,3>` (E30019).                       | source/slang/slang-check-expr.cpp |
| [`matrix-layout-modifier-on-scalar-diag.slang`](matrix-layout-modifier-on-scalar-diag.slang)     | `typedef row_major int` reports E39026 and drops the layout modifier, leaving the alias usable as `int`.                               | source/slang/slang-check-expr.cpp |
| [`matrix-swizzle-vector-result.slang`](matrix-swizzle-vector-result.slang)                       | A multi-component matrix swizzle `m._m00_m11` lowers (HLSL) to a `float2(...)` built from per-element `[row][col]` matrix subscripts.  | source/slang/slang-check-expr.cpp |
| [`return-val-not-available-diag.slang`](return-val-not-available-diag.slang)                     | `__return_val` in a function returning a copyable type reports E38104; only error-typed and non-copyable returns define it.            | source/slang/slang-check-expr.cpp |
| [`spirv-asm-id-redefinition-diag.slang`](spirv-asm-id-redefinition-diag.slang)                   | Defining the same `spirv_asm` result id twice reports E29113 on the second definition, not the first.                                  | source/slang/slang-check-expr.cpp |
| [`spirv-asm-not-enough-operands-diag.slang`](spirv-asm-not-enough-operands-diag.slang)           | A `spirv_asm` instruction with fewer operands than its result-id index needs reports E29112.                                           | source/slang/slang-check-expr.cpp |
| [`spirv-asm-unknown-builtin-name-diag.slang`](spirv-asm-unknown-builtin-name-diag.slang)         | An unknown name in a `spirv_asm` `builtin(...)` operand reports E29107 from the dedicated `BuiltIn` lookup.                            | source/slang/slang-check-expr.cpp |
| [`spirv-asm-unknown-enum-name-diag.slang`](spirv-asm-unknown-enum-name-diag.slang)               | A `spirv_asm` named-value operand that matches no enumerator, type-prefixed enumerator or opcode reports E29107.                       | source/slang/slang-check-expr.cpp |
| [`subscript-non-array-diag.slang`](subscript-non-array-diag.slang)                               | Subscripting a scalar (`x[0]` where x is float) reports "no subscript declarations found for type 'float'" (E30013).                   | source/slang/slang-check-expr.cpp |
| [`subscript-too-many-indices-diag.slang`](subscript-too-many-indices-diag.slang)                 | `a[1, 2]` is rejected by the subscript arity guard before overload resolution, as "too many arguments to call (got 2, expected 1)".    | source/slang/slang-check-expr.cpp |
| [`swizzle-duplicate-not-lvalue-diag.slang`](swizzle-duplicate-not-lvalue-diag.slang)             | A swizzle with a repeated component (`v.xx`) is not an l-value, so assigning to it is rejected with E30011.                            | source/slang/slang-check-expr.cpp |
| [`swizzle-out-of-range-diag.slang`](swizzle-out-of-range-diag.slang)                             | A component beyond the vector width (`v.z` on float2) is rejected as a missing member (E30027), not a swizzle-specific error.          | source/slang/slang-check-expr.cpp |
| [`swizzle-reorder-and-dup-rvalue.slang`](swizzle-reorder-and-dup-rvalue.slang)                   | `v.wzyx` reverses the components and `v.xxy` duplicates the first component when used as an r-value.                                   | source/slang/slang-check-expr.cpp |
| [`swizzle-rgba-alias.slang`](swizzle-rgba-alias.slang)                                           | The r/g/b/a swizzle letters select the same elements as x/y/z/w.                                                                       | source/slang/slang-check-expr.cpp |
| [`swizzle-too-long-diag.slang`](swizzle-too-long-diag.slang)                                     | A swizzle longer than four valid components (`v.xxxxx`) is rejected as a missing member (E30027).                                      | source/slang/slang-check-expr.cpp |
| [`try-clause-shape-diag.slang`](try-clause-shape-diag.slang)                                     | The three `try` shape rules get distinct diagnostics: non-call operand (E30090), non-throwing callee (E30091), missing `try` (E30094). | source/slang/slang-check-expr.cpp |
| [`try-error-type-mismatch-diag.slang`](try-error-type-mismatch-diag.slang)                       | A `try` call whose callee and caller declare different error types reports E30095, distinct from the uncaught-`try` error.             | source/slang/slang-check-expr.cpp |
| [`tuple-swizzle-out-of-range-diag.slang`](tuple-swizzle-out-of-range-diag.slang)                 | `t._5` on a two-element tuple is an invalid swizzle (E30052) rather than a missing member.                                             | source/slang/slang-check-expr.cpp |

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
- **Dead code — `visitThisTypeExpr`** (`slang-check-expr.cpp:9135`, 23
  lines, 0% covered). Nothing in the repository ever constructs a
  `ThisTypeExpr`: the only mentions outside this visitor are the class
  declaration in `slang-ast-expr.h`, the visitor declaration in
  `slang-check-impl.h`, and the no-op handlers in `slang-ast-print.cpp`,
  `slang-check-decl.cpp`, `slang-lower-to-ir.cpp`,
  `slang-ast-iterator.h` and the language server. The parser spells `This`
  as an ordinary name bound to a `ThisTypeDecl` (`slang-parser.cpp:6122`),
  not as this node. Not testable; a removal candidate, not a bug.
- **Dead code — `visitFuncTypeOfExpr`** (`slang-check-expr.cpp:6168`,
  11 lines, 0% covered). Same situation: no `FuncTypeOfExpr` is ever
  created. The syntax that looks like it (`__func_as_type(fn)`) parses to
  `FuncAsTypeExpr` instead (`slang-parser.cpp:3191`), and
  `slang-lower-to-ir.cpp:6520` already asserts the node "should not be
  present in checked AST". Removal candidate.
- The `StaticRefToNonStaticMember` site inside `ConstructDeclRefExpr`
  (`slang-check-expr.cpp:520`) is shadowed: `_lookupStaticMember` filters
  non-static items out and diagnoses first (`:8588`). Probed with `S::f`,
  `S.f`, `IFoo::m()` and a call from inside a generic — all three reach
  the `:8588` site, none the `:520` one. Defensive duplicate, not
  targeted; the `:8588` behaviour already has a hand-written test at
  `tests/diagnostics/static-ref-to-nonstatic-member-2.slang`.
- The `SpirvInstructionWithTooManyOperands` branch with `maxOperands == 0`
  (`slang-check-expr.cpp:9522`) **is** CLI-reachable
  (`spirv_asm { OpNop %x; };`) but was not turned into a test: after
  emitting the warning the compile aborts with
  `error[E99997] ... InternalError ... unexpected: ErrorType`. Filed as
  `_meta/findings/spirv-asm-extra-operand-on-zero-operand-op-internal-error.yaml`.
- The unary-`+` arm of symbolic constant folding
  (`slang-check-expr.cpp:2521`) could not be reached: `int arr[+N]` for a
  generic `let N : int` is rejected as "not a compile-time constant" even
  though `-N + 8` and `N * 2 + 1` in the same position fold. Recorded as
  `_meta/findings/unary-plus-on-generic-value-param-not-constant-folded.yaml`
  rather than pinned as behaviour.
- The completion-suggestion blocks in member-expression checking
  (`slang-check-expr.cpp:8852`+) only run when the linkage has content
  assist enabled, which is a language-server mode with no slangc/slangi
  equivalent. Not reachable from this suite.
- The following areas showed as uncovered in the supplied profile but are
  in fact exercised by the hand-written suite, and were deliberately left
  alone to avoid duplicating it: the shape-pack transforms
  (`visitShapePackTransformExpr`, `tests/language-feature/generics/mock-tile-shape-pack-ops.slang`),
  `__floatAsInt` bit-cast mismatch (`tests/diagnostics/float-as-int-type-mismatch.slang`),
  ambiguous-reference-as-member-base (`tests/diagnostics/ambiguous-member-base/`),
  implicit-cast-as-l-value (`tests/diagnostics/implicit-cast-lvalue.slang`),
  non-copyable capture in a lambda (`tests/language-feature/lambda/lambda-diagnostics.slang`),
  `volatile` as a type modifier (`tests/diagnostics/volatile-typedef.slang`),
  `each` outside a pack (`tests/language-feature/generics/diagnose-each-without-type-pack.slang`),
  the non-empty pack-query constraint
  (`tests/language-feature/generics/variadic-pack-query-nonempty-constraint.slang`),
  `is`/`as` with an interface on the right-hand side
  (`tests/language-feature/interface-as-rhs-error.slang`),
  and the non-short-circuiting `?:` diagnostics
  (`tests/diagnostics/autodiff-non-short-circuit-operator.slang`).

## Doc gaps observed

NA
