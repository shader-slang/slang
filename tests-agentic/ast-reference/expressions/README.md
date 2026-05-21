---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T16:30:00+00:00
source_commit: 3250005059a2746ebc504a9d3f71ed112f1f2b94
watched_paths_digest: 095ac036e831eb7631a84f942f39c0461e59910aefb1c4c6522c17f442f3bf95
source_doc: docs/llm-generated/ast-reference/expressions.md
source_doc_digest: b98fffd8c6cc8711efedeacdd86661dc131dc617e39a464934db0b59040faf44
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ast-reference/expressions

## Intent

Tests verify the concrete `Expr` subclasses enumerated in
[`docs/llm-generated/ast-reference/expressions.md`](../../../docs/llm-generated/ast-reference/expressions.md):
`VarExpr`, the `LiteralExpr` family, `InvokeExpr` and its
operator/cast specializations, `SelectExpr`,
`LogicOperatorShortCircuitExpr`, `MemberExpr`, `StaticMemberExpr`,
`IndexExpr`, `AssignExpr`, `ParenExpr`, `SwizzleExpr`, the
`SizeOfExpr` / `AlignOfExpr` / `CountOfExpr` family, the
`IsTypeExpr` / `AsTypeExpr` / `CastToSuperTypeExpr` trio,
`GenericAppExpr` / `PartiallyAppliedGenericExpr`, `LambdaExpr`,
the differentiate-family expressions, and the type-expression family
(`AndTypeExpr`, `TupleTypeExpr`, `ThisTypeExpr`, `ModifiedTypeExpr`).

The strategy is **per-expression-kind observation**: for each
user-spellable expression node in the doc's `## Nodes` table, write
the smallest source program whose observable value or diagnostic
proves the node behaves as documented. Synthesized / non-user-spellable
nodes (`MakeArrayFromElementExpr`, `BuiltinCastExpr`,
`ImplicitCastExpr` and the `LValueImplicitCastExpr` chain,
`MakeRefExpr`, `OpenRefExpr`, `ExtractExistentialValueExpr`,
`PackExpr`, `SharedTypeExpr`, `ModifierCastExpr`, `ReturnValExpr`,
`FloatBitCastExpr`, `FuncAsTypeExpr`, `FuncTypeOfExpr`,
`IncompleteExpr`, `AggTypeCtorExpr`, `OverloadedExpr` / `OverloadedExpr2`
as intermediates) appear in `## Out of scope` because they are
checker-internal and have no user-written spelling.

Most claims are target-independent: a literal, an operator, an
overload, or a member-access produces the same AST node regardless
of backend. INTERPRET is the primary directive. `SwizzleExpr` is the
canonical target-divergent case (HLSL/GLSL emit `.zyx` text;
SPIR-V emits `OpVectorShuffle`); it carries a multi-target SIMPLE
test alongside an INTERPRET value test.

Negative tests cover the natural rejection cases: assignment to a
non-l-value, undeclared name reference, member-access of a missing
field, an unconvertible cast, and an InvokeExpr whose argument types
don't match the only candidate.

A second pass extends the bundle with boundary, stress, and
additional negative tests instantiated against the mandatory axes in
`_meta/prompts/_common.md`. For literals: int/uint MIN/MAX edges and
unsigned-overflow wrap at both `uint` and `uint8` widths; `+0.0`
vs `-0.0` bit-pattern distinction observed via `1.0/zero` -> ±inf;
arithmetic overflow to `+inf`/`-inf`; `NaN != NaN` from `0/0`. For
`InfixExpr`: precedence boundary, deeply nested paren-arithmetic,
and `% 1` lower edge. For short-circuit logic operators: chains of
three or more side-effecting RHS calls that must not fire. For
`MemberExpr`: a five-deep `a.b.c.d.e.v` access, plus a negative
test on a non-aggregate base. For `InvokeExpr`: 0-arg, 8-arg, and
arity-mismatch negatives. For `IndexExpr`: index 0, N-1, and a
three-deep `arr[i][j][k]` chain. For `TypeCastExpr`: uint32→uint8
narrowing, float→int truncation in both signs, signed→unsigned wrap,
and an additional struct→bool rejection. For `AssignExpr`: implicit
narrowing conversion, and assigning to a call result (non-l-value).
For `SelectExpr`: nested ternary chain and mixed-arm common-type
emergence. For `InitializerListExpr`: empty `{}`, single-element,
and an over-count negative. For `LambdaExpr`: zero parameters,
single capture, and a curried lambda-inside-lambda.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                                                                              | Claim (one line)                                                                                                                       | Tests                                                                                          |
| -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| C-01     | [#varexpr-and-declrefexpr](../../../docs/llm-generated/ast-reference/expressions.md#varexpr-and-declrefexpr)                                                                                                                        | A bare identifier in expression context parses as a VarExpr that the checker resolves to its declaration.                              | [`var-expr-resolves-name.slang`](var-expr-resolves-name.slang)                                                                 |
| C-02     | [#varexpr-and-declrefexpr](../../../docs/llm-generated/ast-reference/expressions.md#varexpr-and-declrefexpr)                                                                                                                        | A VarExpr whose name resolves to nothing is rejected with an "undefined identifier" diagnostic.                                        | [`undeclared-name-rejected.slang`](undeclared-name-rejected.slang)                                                               |
| C-03     | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family)                                                                                                                                  | An IntegerLiteralExpr carries an integer value that survives evaluation.                                                               | [`literal-int-typed.slang`](literal-int-typed.slang)                                                                      |
| C-04     | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family)                                                                                                                                  | An IntegerLiteralExpr's suffixType drives overload selection (`1u` selects `uint`, `1` selects `int`).                                 | [`literal-int-suffix-selects-type.slang`](literal-int-suffix-selects-type.slang)                                                        |
| C-05     | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family)                                                                                                                                  | A FloatingPointLiteralExpr's suffix steers overload resolution: `1.5f` is float, `1.5lf` is double.                                    | [`literal-float-typed.slang`](literal-float-typed.slang)                                                                    |
| C-06     | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family)                                                                                                                                  | A BoolLiteralExpr `true`/`false` evaluates to the corresponding boolean value.                                                         | [`literal-bool-true-false.slang`](literal-bool-true-false.slang)                                                                |
| C-07     | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family)                                                                                                                                  | A NullPtrLiteralExpr `nullptr` denotes the null pointer.                                                                               | [`literal-nullptr-typed.slang`](literal-nullptr-typed.slang)                                                                  |
| C-08     | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family)                                                                                                                                  | A NoneLiteralExpr `none` denotes the empty Optional; an Optional-typed initializer accepts both `none` and a wrapped value.            | [`make-optional-some-and-none.slang`](make-optional-some-and-none.slang)                                                            |
| C-09     | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family)                                                                                                                                  | A StringLiteralExpr's parsed text appears in emitted target code.                                                                      | [`literal-string-cpp-emit.slang`](literal-string-cpp-emit.slang)                                                                |
| C-10     | [#invokeexpr-and-the-call-operator-cast-unification](../../../docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification)                                                                    | An InvokeExpr `f(x)` calls the function with the given arguments and returns its result.                                               | [`invoke-call-function.slang`](invoke-call-function.slang)                                                                   |
| C-11     | [#invokeexpr-and-the-call-operator-cast-unification](../../../docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification)                                                                    | Operator application is unified with calls: a user-defined `operator+` is invoked by writing `a + b`.                                  | [`invoke-operator-and-call-unified.slang`](invoke-operator-and-call-unified.slang)                                                       |
| C-12     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An InfixExpr evaluates a binary arithmetic operator (+, -, \*, /) to the expected numeric result.                                      | [`infix-arithmetic-operators.slang`](infix-arithmetic-operators.slang)                                                             |
| C-13     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An InfixExpr comparison (<, ==, !=) yields a boolean reflecting the comparison.                                                        | [`infix-comparison-operators.slang`](infix-comparison-operators.slang)                                                             |
| C-14     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An InfixExpr compound-assignment `+=` updates the l-value in place.                                                                    | [`compound-assign-plus.slang`](compound-assign-plus.slang)                                                                   |
| C-15     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A PrefixExpr `-x` is unary negation.                                                                                                   | [`prefix-unary-minus.slang`](prefix-unary-minus.slang)                                                                     |
| C-16     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A PrefixExpr `!x` is logical-not.                                                                                                      | [`prefix-logical-not.slang`](prefix-logical-not.slang)                                                                     |
| C-17     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A PrefixExpr `--x` is prefix decrement.                                                                                                | [`prefix-decrement.slang`](prefix-decrement.slang)                                                                       |
| C-18     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A PostfixExpr `a++` yields the pre-increment value and bumps the operand.                                                              | [`postfix-increment.slang`](postfix-increment.slang)                                                                      |
| C-19     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A SelectExpr `c ? a : b` yields a when c is true and b when c is false.                                                                | [`select-expr-ternary.slang`](select-expr-ternary.slang)                                                                    |
| C-20     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A SelectExpr only evaluates the chosen arm; the unchosen arm's side effects do not happen.                                             | [`select-evaluates-only-chosen-arm.slang`](select-evaluates-only-chosen-arm.slang)                                                       |
| C-21     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A LogicOperatorShortCircuitExpr `&&` does not evaluate the right operand when the left is false.                                       | [`logic-and-short-circuit.slang`](logic-and-short-circuit.slang)                                                                |
| C-22     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A LogicOperatorShortCircuitExpr `\|\|` does not evaluate the right operand when the left is true.                                      | [`logic-or-short-circuit.slang`](logic-or-short-circuit.slang)                                                                 |
| C-23     | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr)                                                                                | A MemberExpr `a.b` reads a field on a value-typed base.                                                                                | [`member-expr-field-access.slang`](member-expr-field-access.slang)                                                               |
| C-24     | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr)                                                                                | A MemberExpr `a.b` with no matching member is rejected with a "member not found" diagnostic.                                           | [`member-missing-field-rejected.slang`](member-missing-field-rejected.slang)                                                          |
| C-25     | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr)                                                                                | A StaticMemberExpr `T.m` reads a static member of a type.                                                                              | [`static-member-on-type.slang`](static-member-on-type.slang)                                                                  |
| C-26     | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr)                                                                                | A StaticMemberExpr can name a static method; `T.f()` invokes it.                                                                       | [`static-member-call-method.slang`](static-member-call-method.slang)                                                              |
| C-27     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An IndexExpr `a[i]` reads the i-th element of an indexed value.                                                                        | [`index-expr-subscript.slang`](index-expr-subscript.slang)                                                                   |
| C-28     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | IndexExpr chains: `a[i][j]` reads the (i,j)-th element of a multi-dim array.                                                           | [`index-expr-multi-dim-array.slang`](index-expr-multi-dim-array.slang)                                                             |
| C-29     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An AssignExpr `a = b` updates the l-value `a`.                                                                                         | [`assign-expr-value-update.slang`](assign-expr-value-update.slang)                                                               |
| C-30     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An AssignExpr with a non-l-value on the left is rejected with "left of '=' is not an l-value".                                         | [`assign-to-non-lvalue-rejected.slang`](assign-to-non-lvalue-rejected.slang)                                                          |
| C-31     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A ParenExpr `(e)` groups a sub-expression and overrides operator precedence.                                                           | [`paren-expr-grouping.slang`](paren-expr-grouping.slang)                                                                    |
| C-32     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A SwizzleExpr `v.zyx` reorders vector components by index; the resulting components match the swizzle.                                 | [`swizzle-expr-interpret-value.slang`](swizzle-expr-interpret-value.slang), [`swizzle-expr-vector-multi-target.slang`](swizzle-expr-vector-multi-target.slang)                 |
| C-33     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An InitializerListExpr `{a, b, c}` builds an aggregate whose fields are populated positionally.                                        | [`initializer-list-aggregate.slang`](initializer-list-aggregate.slang), [`initializer-list-array-elements.slang`](initializer-list-array-elements.slang)                    |
| C-34     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An ExplicitCastExpr `(T) e` invokes a conversion; int-to-float yields the numeric value as a float.                                    | [`explicit-cast-int-to-float.slang`](explicit-cast-int-to-float.slang)                                                             |
| C-35     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An ExplicitCastExpr with no available conversion is rejected with a type-mismatch diagnostic.                                          | [`cast-incompatible-rejected.slang`](cast-incompatible-rejected.slang)                                                             |
| C-36     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A SizeOfExpr `sizeof(T)` yields the compile-time-known size of T.                                                                      | [`sizeof-expr-type.slang`](sizeof-expr-type.slang)                                                                       |
| C-37     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An AlignOfExpr `alignof(T)` yields the compile-time-known alignment of T.                                                              | [`alignof-expr-type.slang`](alignof-expr-type.slang)                                                                      |
| C-38     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A CountOfExpr `countof(T)` yields the element count of a static type pack.                                                             | [`countof-expr-type-pack.slang`](countof-expr-type-pack.slang)                                                                 |
| C-39     | [#astypeexpr-istypeexpr-casttosupertypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr)                                                                                    | An IsTypeExpr `value is Type` yields true when the dynamic type matches.                                                               | [`istype-expr-runtime.slang`](istype-expr-runtime.slang)                                                                    |
| C-40     | [#astypeexpr-istypeexpr-casttosupertypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr)                                                                                    | An AsTypeExpr `value as Type` yields an Optional<Type> populated when the dynamic type matches.                                        | [`astype-expr-subtype-cast.slang`](astype-expr-subtype-cast.slang)                                                               |
| C-41     | [#astypeexpr-istypeexpr-casttosupertypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr)                                                                                    | A CastToSuperTypeExpr is inserted at an interface up-cast; the interface method dispatches to the concrete type.                       | [`cast-to-supertype-implicit.slang`](cast-to-supertype-implicit.slang)                                                             |
| C-42     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A GenericAppExpr `g<T>` applies explicit generic arguments; the specialization is invokable.                                           | [`generic-app-explicit-args.slang`](generic-app-explicit-args.slang)                                                              |
| C-43     | [#partiallyappliedgenericexpr](../../../docs/llm-generated/ast-reference/expressions.md#partiallyappliedgenericexpr)                                                                                                                | A PartiallyAppliedGenericExpr provides some generic arguments and lets overload resolution infer the rest from call-site types.        | [`partially-applied-generic.slang`](partially-applied-generic.slang)                                                              |
| C-44     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | An ExplicitCtorInvokeExpr `T(...)` invokes a constructor and returns a value of T.                                                     | [`explicit-ctor-invoke.slang`](explicit-ctor-invoke.slang)                                                                   |
| C-45     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A DefaultConstructExpr `T()` yields a default-constructed value with zero-initialized fields.                                          | [`default-construct-value.slang`](default-construct-value.slang)                                                                |
| C-46     | [#lambdaexpr-and-lambdadecl](../../../docs/llm-generated/ast-reference/expressions.md#lambdaexpr-and-lambdadecl)                                                                                                                    | A LambdaExpr `(params) => body` forms a callable; calling it returns the body's value.                                                 | [`lambda-expr-call.slang`](lambda-expr-call.slang)                                                                       |
| C-47     | [#differentiate-family-expressions](../../../docs/llm-generated/ast-reference/expressions.md#differentiate-family-expressions)                                                                                                      | A ForwardDifferentiateExpr `__fwd_diff(f)(diffPair)` evaluates the forward-mode derivative.                                            | [`fwd-differentiate-expr.slang`](fwd-differentiate-expr.slang)                                                                 |
| C-48     | [#differentiate-family-expressions](../../../docs/llm-generated/ast-reference/expressions.md#differentiate-family-expressions)                                                                                                      | A BackwardDifferentiateExpr `__bwd_diff(f)(diffPair, dL)` propagates a gradient back into the input differential.                      | [`bwd-differentiate-expr.slang`](bwd-differentiate-expr.slang)                                                                 |
| C-49     | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes)                                                                                                                                                            | A ThisExpr inside a member function refers to the receiver instance.                                                                   | [`this-expr-in-method.slang`](this-expr-in-method.slang)                                                                    |
| C-50     | [#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr) | A ThisTypeExpr `This` refers to the surrounding type; a method returning `This` resolves to each conformer's own type.                  | [`this-type-expr-in-interface.slang`](this-type-expr-in-interface.slang)                                                            |
| C-51     | [#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr) | An AndTypeExpr `IA & IB` denotes a conjunction-of-conformances type; both interfaces' methods are callable on a conforming value.       | [`and-type-expr-conjunction.slang`](and-type-expr-conjunction.slang)                                                              |
| C-52     | [#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr) | A TupleTypeExpr `(T1, T2)` declares a tuple type; the elements are accessed via `._0` / `._1`.                                          | [`tuple-type-expr-as-return.slang`](tuple-type-expr-as-return.slang)                                                              |
| C-53     | [#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr) | A ModifiedTypeExpr `const T` attaches modifier prefixes to a type expression.                                                           | [`modified-type-expr-const.slang`](modified-type-expr-const.slang)                                                               |
| C-54     | [#overloadedexpr-and-overloadedexpr2](../../../docs/llm-generated/ast-reference/expressions.md#overloadedexpr-and-overloadedexpr2)                                                                                                  | When OverloadedExpr resolution finds no applicable candidate, the InvokeExpr is rejected with a diagnostic.                            | [`no-applicable-overload-rejected.slang`](no-applicable-overload-rejected.slang)                                                        |

## Tests in this bundle

| File | Intent | Doc anchor |
| --- | --- | --- |
| [`alignof-expr-type.slang`](alignof-expr-type.slang) | functional | `#nodes` |
| [`and-type-expr-conjunction.slang`](and-type-expr-conjunction.slang) | functional | `#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr` |
| [`assign-expr-value-update.slang`](assign-expr-value-update.slang) | functional | `#nodes` |
| [`assign-implicit-narrowing-conversion.slang`](assign-implicit-narrowing-conversion.slang) | boundary | `#nodes` |
| [`assign-to-call-result-rejected.slang`](assign-to-call-result-rejected.slang) | negative | `#nodes` |
| [`assign-to-non-lvalue-rejected.slang`](assign-to-non-lvalue-rejected.slang) | negative | `#nodes` |
| [`astype-expr-subtype-cast.slang`](astype-expr-subtype-cast.slang) | functional | `#astypeexpr-istypeexpr-casttosupertypeexpr` |
| [`bwd-differentiate-expr.slang`](bwd-differentiate-expr.slang) | functional | `#differentiate-family-expressions` |
| [`cast-float-to-int-truncates.slang`](cast-float-to-int-truncates.slang) | boundary | `#nodes` |
| [`cast-incompatible-rejected.slang`](cast-incompatible-rejected.slang) | negative | `#nodes` |
| [`cast-incompatible-struct-to-bool-rejected.slang`](cast-incompatible-struct-to-bool-rejected.slang) | negative | `#nodes` |
| [`cast-negative-int-to-uint-wraps.slang`](cast-negative-int-to-uint-wraps.slang) | boundary | `#nodes` |
| [`cast-to-supertype-implicit.slang`](cast-to-supertype-implicit.slang) | functional | `#astypeexpr-istypeexpr-casttosupertypeexpr` |
| [`cast-uint32-to-uint8-narrows.slang`](cast-uint32-to-uint8-narrows.slang) | boundary | `#nodes` |
| [`compound-assign-multiply.slang`](compound-assign-multiply.slang) | boundary | `#nodes` |
| [`compound-assign-plus.slang`](compound-assign-plus.slang) | functional | `#nodes` |
| [`countof-expr-type-pack.slang`](countof-expr-type-pack.slang) | functional | `#nodes` |
| [`default-construct-value.slang`](default-construct-value.slang) | functional | `#nodes` |
| [`explicit-cast-int-to-float.slang`](explicit-cast-int-to-float.slang) | functional | `#nodes` |
| [`explicit-ctor-invoke.slang`](explicit-ctor-invoke.slang) | functional | `#nodes` |
| [`fwd-differentiate-expr.slang`](fwd-differentiate-expr.slang) | functional | `#differentiate-family-expressions` |
| [`generic-app-explicit-args.slang`](generic-app-explicit-args.slang) | functional | `#nodes` |
| [`index-expr-index-zero.slang`](index-expr-index-zero.slang) | boundary | `#nodes` |
| [`index-expr-last-element.slang`](index-expr-last-element.slang) | boundary | `#nodes` |
| [`index-expr-multi-dim-array.slang`](index-expr-multi-dim-array.slang) | functional | `#nodes` |
| [`index-expr-subscript.slang`](index-expr-subscript.slang) | functional | `#nodes` |
| [`index-expr-three-dim-chain.slang`](index-expr-three-dim-chain.slang) | stress | `#nodes` |
| [`infix-arithmetic-operators.slang`](infix-arithmetic-operators.slang) | functional | `#nodes` |
| [`infix-comparison-operators.slang`](infix-comparison-operators.slang) | functional | `#nodes` |
| [`infix-deeply-nested-arithmetic.slang`](infix-deeply-nested-arithmetic.slang) | stress | `#nodes` |
| [`infix-int-modulo-by-one.slang`](infix-int-modulo-by-one.slang) | boundary | `#nodes` |
| [`infix-precedence-mul-before-add.slang`](infix-precedence-mul-before-add.slang) | boundary | `#nodes` |
| [`initializer-list-aggregate.slang`](initializer-list-aggregate.slang) | functional | `#nodes` |
| [`initializer-list-array-elements.slang`](initializer-list-array-elements.slang) | functional | `#nodes` |
| [`initializer-list-empty-default-zeros.slang`](initializer-list-empty-default-zeros.slang) | boundary | `#nodes` |
| [`initializer-list-single-element.slang`](initializer-list-single-element.slang) | boundary | `#nodes` |
| [`initializer-list-too-many-elements-rejected.slang`](initializer-list-too-many-elements-rejected.slang) | negative | `#nodes` |
| [`invoke-call-function.slang`](invoke-call-function.slang) | functional | `#invokeexpr-and-the-call-operator-cast-unification` |
| [`invoke-many-args.slang`](invoke-many-args.slang) | stress | `#invokeexpr-and-the-call-operator-cast-unification` |
| [`invoke-operator-and-call-unified.slang`](invoke-operator-and-call-unified.slang) | functional | `#invokeexpr-and-the-call-operator-cast-unification` |
| [`invoke-wrong-arity-rejected.slang`](invoke-wrong-arity-rejected.slang) | negative | `#invokeexpr-and-the-call-operator-cast-unification` |
| [`invoke-zero-args.slang`](invoke-zero-args.slang) | boundary | `#invokeexpr-and-the-call-operator-cast-unification` |
| [`istype-expr-runtime.slang`](istype-expr-runtime.slang) | functional | `#astypeexpr-istypeexpr-casttosupertypeexpr` |
| [`istype-no-match-false.slang`](istype-no-match-false.slang) | boundary | `#astypeexpr-istypeexpr-casttosupertypeexpr` |
| [`lambda-captures-single-local.slang`](lambda-captures-single-local.slang) | boundary | `#lambdaexpr-and-lambdadecl` |
| [`lambda-expr-call.slang`](lambda-expr-call.slang) | functional | `#lambdaexpr-and-lambdadecl` |
| [`lambda-nested-inside-lambda.slang`](lambda-nested-inside-lambda.slang) | stress | `#lambdaexpr-and-lambdadecl` |
| [`lambda-zero-args.slang`](lambda-zero-args.slang) | boundary | `#lambdaexpr-and-lambdadecl` |
| [`literal-bool-overload-selection.slang`](literal-bool-overload-selection.slang) | boundary | `#literalexpr-family` |
| [`literal-bool-true-false.slang`](literal-bool-true-false.slang) | functional | `#literalexpr-family` |
| [`literal-float-nan-self-compare.slang`](literal-float-nan-self-compare.slang) | boundary | `#literalexpr-family` |
| [`literal-float-negative-infinity.slang`](literal-float-negative-infinity.slang) | boundary | `#literalexpr-family` |
| [`literal-float-positive-infinity.slang`](literal-float-positive-infinity.slang) | boundary | `#literalexpr-family` |
| [`literal-float-positive-vs-negative-zero.slang`](literal-float-positive-vs-negative-zero.slang) | boundary | `#literalexpr-family` |
| [`literal-float-typed.slang`](literal-float-typed.slang) | functional | `#literalexpr-family` |
| [`literal-int-max-value.slang`](literal-int-max-value.slang) | boundary | `#literalexpr-family` |
| [`literal-int-min-value.slang`](literal-int-min-value.slang) | boundary | `#literalexpr-family` |
| [`literal-int-suffix-selects-type.slang`](literal-int-suffix-selects-type.slang) | functional | `#literalexpr-family` |
| [`literal-int-typed.slang`](literal-int-typed.slang) | functional | `#literalexpr-family` |
| [`literal-int8-min-max.slang`](literal-int8-min-max.slang) | boundary | `#literalexpr-family` |
| [`literal-nullptr-typed.slang`](literal-nullptr-typed.slang) | functional | `#literalexpr-family` |
| [`literal-string-cpp-emit.slang`](literal-string-cpp-emit.slang) | functional | `#literalexpr-family` |
| [`literal-string-empty-cpp-emit.slang`](literal-string-empty-cpp-emit.slang) | boundary | `#literalexpr-family` |
| [`literal-uint-max-plus-one-wraps.slang`](literal-uint-max-plus-one-wraps.slang) | stress | `#literalexpr-family` |
| [`literal-uint-max-value.slang`](literal-uint-max-value.slang) | boundary | `#literalexpr-family` |
| [`literal-uint8-max-plus-one-wraps.slang`](literal-uint8-max-plus-one-wraps.slang) | stress | `#literalexpr-family` |
| [`logic-and-rhs-side-effect-skipped-deep.slang`](logic-and-rhs-side-effect-skipped-deep.slang) | stress | `#nodes` |
| [`logic-and-short-circuit.slang`](logic-and-short-circuit.slang) | functional | `#nodes` |
| [`logic-or-rhs-side-effect-skipped-deep.slang`](logic-or-rhs-side-effect-skipped-deep.slang) | stress | `#nodes` |
| [`logic-or-short-circuit.slang`](logic-or-short-circuit.slang) | functional | `#nodes` |
| [`make-optional-some-and-none.slang`](make-optional-some-and-none.slang) | functional | `#literalexpr-family` |
| [`member-access-on-non-aggregate-rejected.slang`](member-access-on-non-aggregate-rejected.slang) | negative | `#memberexpr-staticmemberexpr-derefmemberexpr` |
| [`member-expr-deeply-chained.slang`](member-expr-deeply-chained.slang) | stress | `#memberexpr-staticmemberexpr-derefmemberexpr` |
| [`member-expr-field-access.slang`](member-expr-field-access.slang) | functional | `#memberexpr-staticmemberexpr-derefmemberexpr` |
| [`member-missing-field-rejected.slang`](member-missing-field-rejected.slang) | negative | `#memberexpr-staticmemberexpr-derefmemberexpr` |
| [`modified-type-expr-const.slang`](modified-type-expr-const.slang) | functional | `#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr` |
| [`no-applicable-overload-rejected.slang`](no-applicable-overload-rejected.slang) | negative | `#overloadedexpr-and-overloadedexpr2` |
| [`paren-expr-grouping.slang`](paren-expr-grouping.slang) | functional | `#nodes` |
| [`partially-applied-generic.slang`](partially-applied-generic.slang) | functional | `#partiallyappliedgenericexpr` |
| [`postfix-increment-precedence-vs-deref.slang`](postfix-increment-precedence-vs-deref.slang) | boundary | `#nodes` |
| [`postfix-increment.slang`](postfix-increment.slang) | functional | `#nodes` |
| [`prefix-decrement-zero-becomes-negative.slang`](prefix-decrement-zero-becomes-negative.slang) | boundary | `#nodes` |
| [`prefix-decrement.slang`](prefix-decrement.slang) | functional | `#nodes` |
| [`prefix-double-logical-not.slang`](prefix-double-logical-not.slang) | stress | `#nodes` |
| [`prefix-logical-not.slang`](prefix-logical-not.slang) | functional | `#nodes` |
| [`prefix-unary-minus.slang`](prefix-unary-minus.slang) | functional | `#nodes` |
| [`select-arms-common-type-int-and-float.slang`](select-arms-common-type-int-and-float.slang) | boundary | `#nodes` |
| [`select-evaluates-only-chosen-arm.slang`](select-evaluates-only-chosen-arm.slang) | functional | `#nodes` |
| [`select-expr-ternary.slang`](select-expr-ternary.slang) | functional | `#nodes` |
| [`select-nested-ternary.slang`](select-nested-ternary.slang) | stress | `#nodes` |
| [`sizeof-expr-type.slang`](sizeof-expr-type.slang) | functional | `#nodes` |
| [`sizeof-int8-equals-one.slang`](sizeof-int8-equals-one.slang) | boundary | `#nodes` |
| [`static-member-call-method.slang`](static-member-call-method.slang) | functional | `#memberexpr-staticmemberexpr-derefmemberexpr` |
| [`static-member-on-type.slang`](static-member-on-type.slang) | functional | `#memberexpr-staticmemberexpr-derefmemberexpr` |
| [`swizzle-expr-interpret-value.slang`](swizzle-expr-interpret-value.slang) | functional | `#nodes` |
| [`swizzle-expr-vector-multi-target.slang`](swizzle-expr-vector-multi-target.slang) | functional | `#nodes` |
| [`swizzle-repeated-component.slang`](swizzle-repeated-component.slang) | stress | `#nodes` |
| [`swizzle-single-component.slang`](swizzle-single-component.slang) | boundary | `#nodes` |
| [`this-expr-in-method.slang`](this-expr-in-method.slang) | functional | `#nodes` |
| [`this-type-expr-in-interface.slang`](this-type-expr-in-interface.slang) | functional | `#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr` |
| [`tuple-type-expr-as-return.slang`](tuple-type-expr-as-return.slang) | functional | `#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr` |
| [`undeclared-name-rejected.slang`](undeclared-name-rejected.slang) | negative | `#varexpr-and-declrefexpr` |
| [`var-expr-resolves-name.slang`](var-expr-resolves-name.slang) | functional | `#varexpr-and-declrefexpr` |

## Doc gaps observed

- The `## Nodes` table lists ~60 concrete `Expr` subclasses but does
  not partition them into "user-spellable" vs "synthesized only".
  This bundle infers the partition by surveying the parser entry
  points; a one-line column added to the table would let an agent
  draw the line without consulting `slang-parser.cpp`.
- `IndexExpr` is documented with `indexExprs: List<Expr*>` and the
  note "(one or more indices)" — but Slang's surface for the
  multi-index form (`a[i, j]` vs `a[i][j]`) is left implicit. The
  AST stores a list; the surface uses chained `[]` for plain arrays
  and varies for buffer / matrix types. A worked example or a
  cross-link to `grammar.md` listing the supported surface forms
  would let us anchor a tighter test.
- The Differentiate-family Notable Nodes section hands off to
  `pipeline/05-ir-passes.md` for the autodiff arithmetic. The doc
  for this bundle could add a one-line claim about the
  expression-level acceptance contract (i.e. "`__fwd_diff(f)` is
  accepted when `f` is `[Differentiable]`; otherwise diagnosed");
  the current claim is silent on the negative case and we did not
  write a negative test for it.
- `TryExpr` has user-spellable surface (`try expr` with
  Standard / Optional / Assert clauseType) but the doc neither
  spells out a minimal usage example nor lists the diagnostic
  emitted when the surrounding function is not `throws`. We
  attempted a test and could not anchor it cleanly to the doc; no
  test for TryExpr ships in this bundle.
- `CountOfExpr` is documented as "element count of a static array",
  but the actual checker requires a type pack or `Tuple<...>` —
  passing a plain `int[N]` is rejected with "argument to countof
  can only be a type pack or tuple". The doc's description is
  misleading; either the implementation or the doc should change,
  and our test follows the implementation by using a variadic
  generic.
- `DispatchKernelExpr` is listed in the Nodes table with a
  user-spelled form `__dispatch_kernel` but no example of the
  surface arguments (`threadGroupSize` / `dispatchSize`) or the
  contexts where it is accepted. We did not write a test for it.
- The `IndexExpr` row does not commit to a static-bounds behavior
  for `arr[N]` where `N >= length(arr)`. The implementation emits
  `E30029 "array index out of bounds"` at a late IR stage that the
  `DIAGNOSTIC_TEST` runner does not capture by position; we could
  not anchor a one-past-end boundary test as a portable negative.
  A short claim in the IndexExpr section naming the rejection
  contract and the stage at which it fires would let an agent
  attach a test for the index-`N` / index-`N+1` upper boundary.
- The `FloatingPointLiteralExpr` row mentions a "parsed value" but
  does not specify the in-language surface for IEEE-special values
  (`+inf`, `-inf`, `NaN`). The boundary tests in this bundle observe
  the values indirectly (overflow, `0/0`, `1/±0`) because no direct
  literal spelling is documented. A note pointing at the canonical
  way to spell those values (or confirming there is no direct
  spelling) would let us write tighter literal-level boundary
  tests.

## Out of scope

(In this bundle, `## Out of scope` covers AST nodes that are
checker-synthesized or parser-internal and therefore have no
user-written spelling we can target through `slangc`. It is not a
GPU-runner exclusion.)

### Synthesized / non-user-spellable Expr nodes

- `IncompleteExpr` — parser placeholder created after a syntax
  error; the doc says it "could not be filled".
- `MakeArrayFromElementExpr` — checker-synthesized; no user
  spelling.
- `GetArrayLengthExpr` — checker-synthesized for array-length
  queries; the surface (`array.getLength()` on dynamic arrays /
  buffers) does not work for static arrays in the interpreter, so
  we did not anchor a portable test.
- `AggTypeCtorExpr` — used internally during checking; no user
  spelling.
- `OverloadedExpr` / `OverloadedExpr2` — intermediate nodes that
  the doc explicitly says "never survives into the IR". Their
  effects are observable only via the diagnostic when overload
  resolution fails (covered by `no-applicable-overload-rejected.slang`).
- `ImplicitCastExpr`, `LValueImplicitCastExpr`, `OutImplicitCastExpr`,
  `InOutImplicitCastExpr`, `BuiltinCastExpr` — checker-inserted
  casts; their existence is observable only as "the conversion
  happened", which is what `cast-to-supertype-implicit.slang` and
  `explicit-cast-int-to-float.slang` verify implicitly.
- `MakeRefExpr`, `OpenRefExpr` — checker-synthesized
  l-value-to-reference conversions; no user spelling.
- `ExtractExistentialValueExpr` — checker-synthesized; no user
  spelling.
- `MakeOptionalExpr` — checker-synthesized; the user surface is
  the `Optional<T> = ...` initializer, covered by
  `make-optional-some-and-none.slang`.
- `ModifierCastExpr` — same-type cast with different modifiers;
  no user spelling.
- `ReturnValExpr` — implicit `__return_val` reference for
  non-copyable returns; no user spelling.
- `FloatBitCastExpr` — surface is `__floatAsInt` / similar builtins;
  the doc points at the intrinsic name, but the AST node is
  populated by the checker, not parsed directly.
- `FuncAsTypeExpr` / `FuncTypeOfExpr` — checker-internal
  function-type machinery; no surface.
- `SharedTypeExpr` — re-used type-expression node; no surface.
- `PackExpr` — internal bundle of pack arguments built during
  overload resolution; no surface.
- `ExpandExpr` / `EachExpr` — pack-expansion surface (`expand E`
  / `each E`) belongs to the variadic-generics feature whose
  observable claims are owned by a dedicated bundle
  (`language-feature/generics-and-packs`, when written), not by
  this AST-node bundle.
- `FirstExpr` / `LastExpr` / `TrimFirstExpr` / `TrimLastExpr` —
  pack-query expressions; same as above.
- `ShapeConcatExpr` / `ShapePermuteExpr` / `ShapeSwapExpr` /
  `ShapeReduceExpr` — shape-pack transformations belonging to the
  shape-pack feature; same reasoning.
- `SPIRVAsmExpr` — inline-SPIRV-assembly sub-language; out of
  scope for this AST-node bundle.
- `PrimalSubstituteExpr` — autodiff selector akin to
  `__fwd_diff` / `__bwd_diff`; the doc hands off to
  `pipeline/05-ir-passes.md` for the autodiff machinery, and the
  user surface is not minimally documented here.
- `DispatchKernelExpr` — host-side dispatch primitive; doc surface
  is too thin (see "Doc gaps").
- `TreatAsDifferentiableExpr` / `DetachExpr` — autodiff annotation
  surfaces (`no_diff`, `__detach`-style); belong to an autodiff
  feature bundle.
- `LetExpr` — `let x = ...; body` expression form; the doc lists
  it but the surface usage in Slang programs is rare and not
  exercised by existing tests; we deferred to keep this bundle
  focused on the more common nodes.
- `NewExpr` — `new T(...)` allocation; not portable across
  targets / CPU interpreter without a known allocator surface; we
  did not include a test.
- `AddressOfExpr` — `&e` where supported; depends on a pointer
  feature gate; not portably exercisable through `slangi`.
- `MatrixSwizzleExpr` — `m._m00_m11` matrix swizzle surface; not
  exercised in this bundle (the vector swizzle case carries the
  same "swizzle observable" claim).
- `DerefMemberExpr` — `a->b` pointer member access; depends on the
  pointer feature, not portably available through `slangi`.
- `FuncTypeExpr` (`(T1, T2) -> R`) and `PointerTypeExpr` (`T*`)
  type-expression surfaces — accepted by the parser but the
  observable usage requires the function-type / pointer feature
  gate; not exercised here.

### Internal AST machinery

- The `originalFunctionExpr` field on `AppExprBase`.
- That `IncompleteExpr` is what a parse error leaves in place.
- The `argumentDelimeterLocs` field on `InvokeExpr`.
- Helper data types declared in `slang-ast-expr.h`:
  `SPIRVAsmOperand`, `SPIRVAsmInst`, `MatrixCoord`.
- The FIDDLE-generated `ASTNodeType` tag for each expression class.
