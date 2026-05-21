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

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A CastToSuperTypeExpr is inserted when a concrete struct is passed where an interface is expected; the interface method dispatches to the concrete implementation. | functional | [#astypeexpr-istypeexpr-casttosupertypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr) | [`cast-to-supertype-implicit.slang`](cast-to-supertype-implicit.slang) |
| An AsTypeExpr `value as Type` yields an Optional<Type> populated when the dynamic type matches; the value is recoverable. | functional | [#astypeexpr-istypeexpr-casttosupertypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr) | [`astype-expr-subtype-cast.slang`](astype-expr-subtype-cast.slang) |
| An IsTypeExpr `value is Type` is a runtime type test: it yields true when the dynamic type matches. | functional | [#astypeexpr-istypeexpr-casttosupertypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr) | [`istype-expr-runtime.slang`](istype-expr-runtime.slang) |
| Verifies the false branch of IsTypeExpr: `value is OtherImpl` yields false when the dynamic type does not match. | boundary | [#astypeexpr-istypeexpr-casttosupertypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr) | [`istype-no-match-false.slang`](istype-no-match-false.slang) |
| A BackwardDifferentiateExpr `__bwd_diff(f)` selects the backward-mode derivative; calling it with dL/dy propagates a gradient back into the input differential. | functional | [#differentiate-family-expressions](../../../docs/llm-generated/ast-reference/expressions.md#differentiate-family-expressions) | [`bwd-differentiate-expr.slang`](bwd-differentiate-expr.slang) |
| A ForwardDifferentiateExpr `__fwd_diff(f)` selects the forward-mode derivative of a [Differentiable] function. | functional | [#differentiate-family-expressions](../../../docs/llm-generated/ast-reference/expressions.md#differentiate-family-expressions) | [`fwd-differentiate-expr.slang`](fwd-differentiate-expr.slang) |
| A direct call `f(x)` parses as an InvokeExpr whose functionExpr is the callee and whose arguments are the argument list; the call returns the callee's result. | functional | [#invokeexpr-and-the-call-operator-cast-unification](../../../docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification) | [`invoke-call-function.slang`](invoke-call-function.slang) |
| An InvokeExpr whose argument-list length does not match any candidate is rejected with a "not enough arguments to call" diagnostic. | negative | [#invokeexpr-and-the-call-operator-cast-unification](../../../docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification) | [`invoke-wrong-arity-rejected.slang`](invoke-wrong-arity-rejected.slang) |
| Operator application is unified with function call: an overloaded `operator+` on a struct is invoked by writing `a + b`, the same way as a function call. | functional | [#invokeexpr-and-the-call-operator-cast-unification](../../../docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification) | [`invoke-operator-and-call-unified.slang`](invoke-operator-and-call-unified.slang) |
| Stresses InvokeExpr.arguments with eight positional arguments — the call must dispatch and the eight values must arrive in order. | stress | [#invokeexpr-and-the-call-operator-cast-unification](../../../docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification) | [`invoke-many-args.slang`](invoke-many-args.slang) |
| Verifies an InvokeExpr with zero arguments — `f()` — calls the function and returns its result. | boundary | [#invokeexpr-and-the-call-operator-cast-unification](../../../docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification) | [`invoke-zero-args.slang`](invoke-zero-args.slang) |
| A LambdaExpr `(params) => { body }` forms a callable that returns the body's value when invoked. | functional | [#lambdaexpr-and-lambdadecl](../../../docs/llm-generated/ast-reference/expressions.md#lambdaexpr-and-lambdadecl) | [`lambda-expr-call.slang`](lambda-expr-call.slang) |
| Stresses LambdaExpr nesting: a curried `(int x) => (int y) => x + y` parses, captures `x` into the inner lambda, and yields the documented sum. | stress | [#lambdaexpr-and-lambdadecl](../../../docs/llm-generated/ast-reference/expressions.md#lambdaexpr-and-lambdadecl) | [`lambda-nested-inside-lambda.slang`](lambda-nested-inside-lambda.slang) |
| Verifies that a LambdaExpr capturing a single enclosing local variable reads its value through the synthesized LambdaDecl environment struct. | boundary | [#lambdaexpr-and-lambdadecl](../../../docs/llm-generated/ast-reference/expressions.md#lambdaexpr-and-lambdadecl) | [`lambda-captures-single-local.slang`](lambda-captures-single-local.slang) |
| Verifies that a LambdaExpr with zero parameters — `() => 42` — forms a callable that returns the body's constant. | boundary | [#lambdaexpr-and-lambdadecl](../../../docs/llm-generated/ast-reference/expressions.md#lambdaexpr-and-lambdadecl) | [`lambda-zero-args.slang`](lambda-zero-args.slang) |
| A BoolLiteralExpr carries `true` or `false`; both spellings evaluate to their respective boolean values. | functional | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-bool-true-false.slang`](literal-bool-true-false.slang) |
| A FloatingPointLiteralExpr carries a float-typed value; the suffix `lf` picks the double overload over float. | functional | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-float-typed.slang`](literal-float-typed.slang) |
| A NoneLiteralExpr `none` yields the empty optional; an Optional<T> initialized from a T value yields the populated form via MakeOptionalExpr. | functional | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`make-optional-some-and-none.slang`](make-optional-some-and-none.slang) |
| A NullPtrLiteralExpr `nullptr` denotes the null pointer; assigning nullptr to a void* and comparing equal yields true. | functional | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-nullptr-typed.slang`](literal-nullptr-typed.slang) |
| A StringLiteralExpr carries the parsed string value; the same string text appears in emitted C++ output. | functional | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-string-cpp-emit.slang`](literal-string-cpp-emit.slang) |
| An IntegerLiteralExpr carries a parsed integer value that survives to runtime as that integer. | functional | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-int-typed.slang`](literal-int-typed.slang) |
| An IntegerLiteralExpr's suffixType drives overload selection: 1u selects the uint overload, 1 selects the int overload. | functional | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-int-suffix-selects-type.slang`](literal-int-suffix-selects-type.slang) |
| Verifies that `uint(0xFFFFFFFFu) + 1u` wraps to 0 — the documented unsigned overflow semantics for the uint base type. | stress | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-uint-max-plus-one-wraps.slang`](literal-uint-max-plus-one-wraps.slang) |
| Verifies that `uint8_t(255) + uint8_t(1)` wraps to 0 — narrow unsigned overflow for the uint8 base type. | stress | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-uint8-max-plus-one-wraps.slang`](literal-uint8-max-plus-one-wraps.slang) |
| Verifies that a BoolLiteralExpr `true`/`false` is typed as `bool` and steers overload resolution to the bool candidate (not to int). | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-bool-overload-selection.slang`](literal-bool-overload-selection.slang) |
| Verifies that a FloatingPointLiteralExpr arithmetic chain that overflows to +inf compares greater than the largest finite literal. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-float-positive-infinity.slang`](literal-float-positive-infinity.slang) |
| Verifies that a NaN value (produced by `0.0/0.0`) does not equal itself, matching IEEE-754 semantics for the FloatingPointLiteralExpr family. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-float-nan-self-compare.slang`](literal-float-nan-self-compare.slang) |
| Verifies that an IntegerLiteralExpr at the documented int MAX (2147483647) parses and round-trips through evaluation as that exact value. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-int-max-value.slang`](literal-int-max-value.slang) |
| Verifies that an IntegerLiteralExpr at the documented int MIN (-2147483648) parses and round-trips through evaluation as that exact value. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-int-min-value.slang`](literal-int-min-value.slang) |
| Verifies that an IntegerLiteralExpr with the `u` suffix at uint MAX (0xFFFFFFFFu) carries the unsigned MAX value through evaluation. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-uint-max-value.slang`](literal-uint-max-value.slang) |
| Verifies that an arithmetic underflow to -inf compares less than the smallest finite (negated MAX) float literal. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-float-negative-infinity.slang`](literal-float-negative-infinity.slang) |
| Verifies that an empty StringLiteralExpr `""` lowers and appears as an empty string literal in emitted C++ output. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-string-empty-cpp-emit.slang`](literal-string-empty-cpp-emit.slang) |
| Verifies that the FloatingPointLiteralExpr distinguishes +0.0f and -0.0f at the bit level — `1.0/+0` yields +inf while `1.0/-0` yields -inf. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-float-positive-vs-negative-zero.slang`](literal-float-positive-vs-negative-zero.slang) |
| Verifies that the int8 base type round-trips the documented MIN (-128) and MAX (127) literal values through evaluation. | boundary | [#literalexpr-family](../../../docs/llm-generated/ast-reference/expressions.md#literalexpr-family) | [`literal-int8-min-max.slang`](literal-int8-min-max.slang) |
| A MemberExpr `a.b` accesses a field on a value-typed base; the read returns the field's value. | functional | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr) | [`member-expr-field-access.slang`](member-expr-field-access.slang) |
| A MemberExpr `a.b` where the base is a scalar (`int`) with no field `foo` is rejected with the same "member not found" diagnostic as a missing struct field. | negative | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr) | [`member-access-on-non-aggregate-rejected.slang`](member-access-on-non-aggregate-rejected.slang) |
| A MemberExpr `a.b` where the base type does not have a member `b` is rejected with a "member not found" diagnostic. | negative | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr) | [`member-missing-field-rejected.slang`](member-missing-field-rejected.slang) |
| A StaticMemberExpr `T.m` (or `T::m`) accesses a static member of a type; the value of the static field is returned. | functional | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr) | [`static-member-on-type.slang`](static-member-on-type.slang) |
| A StaticMemberExpr can also name a static method on a type; calling `T.f()` invokes the method without an instance. | functional | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr) | [`static-member-call-method.slang`](static-member-call-method.slang) |
| Stresses MemberExpr chaining: a five-deep `a.b.c.d.e.v` field access must resolve and read the written value. | stress | [#memberexpr-staticmemberexpr-derefmemberexpr](../../../docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr) | [`member-expr-deeply-chained.slang`](member-expr-deeply-chained.slang) |
| A CountOfExpr `countof(T)` yields the number of elements in a static type pack. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`countof-expr-type-pack.slang`](countof-expr-type-pack.slang) |
| A GenericAppExpr `g<T>` applies explicit generic arguments to a generic decl; the resulting specialization is invokable. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`generic-app-explicit-args.slang`](generic-app-explicit-args.slang) |
| A LogicOperatorShortCircuitExpr `&&` does not evaluate the right operand when the left is false. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`logic-and-short-circuit.slang`](logic-and-short-circuit.slang) |
| A LogicOperatorShortCircuitExpr `\|\|` does not evaluate the right operand when the left is true. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`logic-or-short-circuit.slang`](logic-or-short-circuit.slang) |
| A ParenExpr `(e)` groups a sub-expression and overrides the default operator precedence at evaluation. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`paren-expr-grouping.slang`](paren-expr-grouping.slang) |
| A PostfixExpr `a++` is the unary postfix increment operator; its value is the pre-increment value of the operand. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`postfix-increment.slang`](postfix-increment.slang) |
| A PrefixExpr `!x` is the logical-not unary prefix operator; it flips a bool value. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`prefix-logical-not.slang`](prefix-logical-not.slang) |
| A PrefixExpr `--x` is the prefix decrement operator; its value is the post-decrement value of the operand. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`prefix-decrement.slang`](prefix-decrement.slang) |
| A PrefixExpr `-x` is a unary prefix operator that negates an integer value. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`prefix-unary-minus.slang`](prefix-unary-minus.slang) |
| A SelectExpr `c ? a : b` yields a when c is true and b when c is false. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`select-expr-ternary.slang`](select-expr-ternary.slang) |
| A SelectExpr only evaluates the chosen arm; the unchosen arm's side effects do not happen. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`select-evaluates-only-chosen-arm.slang`](select-evaluates-only-chosen-arm.slang) |
| A SizeOfExpr `sizeof(T)` yields a compile-time-known size in bytes; for `int` the layout is 4. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`sizeof-expr-type.slang`](sizeof-expr-type.slang) |
| A SwizzleExpr `v.zyx` reorders components by the listed indices; the resulting vector's components match the swizzle. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`swizzle-expr-interpret-value.slang`](swizzle-expr-interpret-value.slang) |
| A SwizzleExpr `v.zyx` reorders vector components; every text-emit target preserves the swizzle in a target-appropriate form. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`swizzle-expr-vector-multi-target.slang`](swizzle-expr-vector-multi-target.slang) |
| A ThisExpr inside a member function refers to the enclosing aggregate's instance; reads through `this` see the receiver's state. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`this-expr-in-method.slang`](this-expr-in-method.slang) |
| A `T()` zero-argument invocation default-constructs a value of T, observable as zero-initialized fields of a plain struct. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`default-construct-value.slang`](default-construct-value.slang) |
| A compound-assignment InfixExpr `a += b` updates `a` to `a + b`; the resulting value is the sum. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`compound-assign-plus.slang`](compound-assign-plus.slang) |
| An AlignOfExpr `alignof(T)` yields a compile-time-known alignment in bytes; for `int` the value is 4. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`alignof-expr-type.slang`](alignof-expr-type.slang) |
| An AssignExpr `a = b` updates the l-value `a` to the value of `b`; reading `a` afterward yields the new value. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`assign-expr-value-update.slang`](assign-expr-value-update.slang) |
| An AssignExpr whose left side is not an l-value is rejected with a diagnostic about "left of '=' is not an l-value". | negative | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`assign-to-non-lvalue-rejected.slang`](assign-to-non-lvalue-rejected.slang) |
| An AssignExpr with the return value of a non-ref-returning function call on the LHS is rejected with "left of '=' is not an l-value". | negative | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`assign-to-call-result-rejected.slang`](assign-to-call-result-rejected.slang) |
| An ExplicitCastExpr `(T) e` invokes a user-spelled cast; converting int to float yields the numeric value as a float. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`explicit-cast-int-to-float.slang`](explicit-cast-int-to-float.slang) |
| An ExplicitCastExpr `(T) e` with no available conversion from typeof(e) to T is rejected with a type-mismatch diagnostic. | negative | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`cast-incompatible-rejected.slang`](cast-incompatible-rejected.slang) |
| An ExplicitCastExpr from a struct to bool is rejected because no conversion exists — a distinct destination type from the existing struct-to-int negative case. | negative | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`cast-incompatible-struct-to-bool-rejected.slang`](cast-incompatible-struct-to-bool-rejected.slang) |
| An ExplicitCtorInvokeExpr `T(...)` is a constructor invocation; it returns a value of type T built from the arguments. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`explicit-ctor-invoke.slang`](explicit-ctor-invoke.slang) |
| An IndexExpr `a[i]` reads the i-th element of an indexed value; with a baseExpression and one index expression. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`index-expr-subscript.slang`](index-expr-subscript.slang) |
| An IndexExpr can chain `a[i][j]` for multi-dimensional indexing; the result is the (i,j)-th element. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`index-expr-multi-dim-array.slang`](index-expr-multi-dim-array.slang) |
| An InfixExpr can be a comparison operator; the result is a bool that reflects the comparison. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`infix-comparison-operators.slang`](infix-comparison-operators.slang) |
| An InfixExpr evaluates a binary arithmetic operator; each operator yields the expected numeric result. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`infix-arithmetic-operators.slang`](infix-arithmetic-operators.slang) |
| An InitializerListExpr `{a, b, c}` builds an aggregate value whose elements correspond positionally to the listed expressions. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`initializer-list-aggregate.slang`](initializer-list-aggregate.slang) |
| An InitializerListExpr in array context populates the array elements positionally. | functional | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`initializer-list-array-elements.slang`](initializer-list-array-elements.slang) |
| An InitializerListExpr with more elements than the aggregate has fields is rejected with a "too many initializers" diagnostic. | negative | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`initializer-list-too-many-elements-rejected.slang`](initializer-list-too-many-elements-rejected.slang) |
| Stresses InfixExpr parsing with a five-deep paren-nested arithmetic chain: ((((1+2)*3)-4)/5) must evaluate to 1. | stress | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`infix-deeply-nested-arithmetic.slang`](infix-deeply-nested-arithmetic.slang) |
| Stresses LogicOperatorShortCircuitExpr `&&`: a leading `false` must skip three chained side-effecting right operands. | stress | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`logic-and-rhs-side-effect-skipped-deep.slang`](logic-and-rhs-side-effect-skipped-deep.slang) |
| Stresses LogicOperatorShortCircuitExpr `\|\|`: a leading `true` must skip three chained side-effecting right operands. | stress | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`logic-or-rhs-side-effect-skipped-deep.slang`](logic-or-rhs-side-effect-skipped-deep.slang) |
| Stresses SelectExpr: a three-deep nested `c ? a : (c' ? a' : (c'' ? a'' : b''))` ternary chain must select the matching arm. | stress | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`select-nested-ternary.slang`](select-nested-ternary.slang) |
| Stresses SwizzleExpr by repeating the same source component across the result: `v.xxxx` on (5,_,_,_) builds a vector of four 5s. | stress | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`swizzle-repeated-component.slang`](swizzle-repeated-component.slang) |
| Stresses chained IndexExpr nodes: `arr[i][j][k]` on a 3-D fixed-size array reads back the assigned element. | stress | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`index-expr-three-dim-chain.slang`](index-expr-three-dim-chain.slang) |
| Verifies an IndexExpr at the lower-edge index 0 reads the first stored element. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`index-expr-index-zero.slang`](index-expr-index-zero.slang) |
| Verifies an IndexExpr at the upper-edge index N-1 (here, 3 for a 4-element array) reads the last stored element. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`index-expr-last-element.slang`](index-expr-last-element.slang) |
| Verifies that PrefixExpr `--x` at the lower-edge value 0 produces -1 on a signed type. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`prefix-decrement-zero-becomes-negative.slang`](prefix-decrement-zero-becomes-negative.slang) |
| Verifies that `x % 1` evaluates to 0 for any integer x — the lower-edge boundary of the InfixExpr modulo operator. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`infix-int-modulo-by-one.slang`](infix-int-modulo-by-one.slang) |
| Verifies that a PostfixExpr `a[i]++` increments the indexed array element (not the index): the IndexExpr is the operand of the PostfixExpr. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`postfix-increment-precedence-vs-deref.slang`](postfix-increment-precedence-vs-deref.slang) |
| Verifies that a SelectExpr with arms of different but convertible types (`int` and `float`) is accepted and yields the chosen arm in the common type (float). | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`select-arms-common-type-int-and-float.slang`](select-arms-common-type-int-and-float.slang) |
| Verifies that a single-element InitializerListExpr `{99}` populates the sole field of a one-field aggregate. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`initializer-list-single-element.slang`](initializer-list-single-element.slang) |
| Verifies that a single-element SwizzleExpr (`v.y`) carries a one-entry elementIndices ShortList and extracts the corresponding scalar component. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`swizzle-single-component.slang`](swizzle-single-component.slang) |
| Verifies that an AssignExpr accepts a right-hand expression of a different (convertible) type — an implicit float-to-int conversion is inserted and the value is truncated to 3. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`assign-implicit-narrowing-conversion.slang`](assign-implicit-narrowing-conversion.slang) |
| Verifies that an ExplicitCastExpr narrowing uint32 to uint8 keeps only the low 8 bits — the high bits set in 0xFF03 are dropped. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`cast-uint32-to-uint8-narrows.slang`](cast-uint32-to-uint8-narrows.slang) |
| Verifies that an empty InitializerListExpr `{}` zero-initializes the aggregate — every field reads back as 0. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`initializer-list-empty-default-zeros.slang`](initializer-list-empty-default-zeros.slang) |
| Verifies that the InfixExpr compound assignment `*=` updates the l-value in place, parallel to the documented `+=` case. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`compound-assign-multiply.slang`](compound-assign-multiply.slang) |
| Verifies that the InfixExpr parser binds `*` tighter than `+`: `2 + 3 * 4` evaluates to 14, not 20. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`infix-precedence-mul-before-add.slang`](infix-precedence-mul-before-add.slang) |
| Verifies that the TypeCastExpr `uint(-1)` wraps to 0xFFFFFFFF — the documented two's-complement reinterpretation across the sign boundary. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`cast-negative-int-to-uint-wraps.slang`](cast-negative-int-to-uint-wraps.slang) |
| Verifies that the TypeCastExpr from float to int truncates toward zero for positive and negative finite values. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`cast-float-to-int-truncates.slang`](cast-float-to-int-truncates.slang) |
| Verifies that two stacked PrefixExpr `!` operators are an identity on a boolean: `!!true == true` and `!!false == false`. | stress | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`prefix-double-logical-not.slang`](prefix-double-logical-not.slang) |
| Verifies the lower-edge boundary of SizeOfExpr: `sizeof(int8_t)` is the documented value 1. | boundary | [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | [`sizeof-int8-equals-one.slang`](sizeof-int8-equals-one.slang) |
| When the call's argument type does not match any candidate parameter type, the InvokeExpr is rejected (the overload set collapse fails to find a match). | negative | [#overloadedexpr-and-overloadedexpr2](../../../docs/llm-generated/ast-reference/expressions.md#overloadedexpr-and-overloadedexpr2) | [`no-applicable-overload-rejected.slang`](no-applicable-overload-rejected.slang) |
| A PartiallyAppliedGenericExpr provides some generic arguments and lets overload resolution infer the rest from call-site argument types. | functional | [#partiallyappliedgenericexpr](../../../docs/llm-generated/ast-reference/expressions.md#partiallyappliedgenericexpr) | [`partially-applied-generic.slang`](partially-applied-generic.slang) |
| A ModifiedTypeExpr `const T` carries a modifier prefix on the type; reads of the const-qualified value return the initialized value. | functional | [#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr) | [`modified-type-expr-const.slang`](modified-type-expr-const.slang) |
| A ThisTypeExpr `This` refers to the surrounding type; in an interface, a method returning `This` resolves to each conformer's own type. | functional | [#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr) | [`this-type-expr-in-interface.slang`](this-type-expr-in-interface.slang) |
| A TupleTypeExpr `(T1, T2)` is a tuple-type spelling; declaring a tuple-typed variable and accessing the i-th element via _i works. | functional | [#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr) | [`tuple-type-expr-as-return.slang`](tuple-type-expr-as-return.slang) |
| An AndTypeExpr `IA & IB` denotes a conjunction-of-conformances type; a value conforming to both can be passed where the conjunction is required. | functional | [#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr](../../../docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr) | [`and-type-expr-conjunction.slang`](and-type-expr-conjunction.slang) |
| A VarExpr whose name lookup finds nothing is rejected with an "undefined identifier" diagnostic. | negative | [#varexpr-and-declrefexpr](../../../docs/llm-generated/ast-reference/expressions.md#varexpr-and-declrefexpr) | [`undeclared-name-rejected.slang`](undeclared-name-rejected.slang) |
| A bare identifier in expression context parses as a VarExpr; after lookup it resolves to the declared variable and reads its value. | functional | [#varexpr-and-declrefexpr](../../../docs/llm-generated/ast-reference/expressions.md#varexpr-and-declrefexpr) | [`var-expr-resolves-name.slang`](var-expr-resolves-name.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#nodes](../../../docs/llm-generated/ast-reference/expressions.md#nodes) | undocumented-behavior | The `## Nodes` table lists ~60 concrete `Expr` subclasses but does not partition them into "user-spellable" vs "synthesized only". This bundle infers the partition by surveying the parser entry points; a one-line column added to the table would let an agent draw the line without consulting `slang-parser.cpp`. |  |
| [#one-or-more-indices](../../../docs/llm-generated/ast-reference/expressions.md#one-or-more-indices) | ambiguous-claim | `IndexExpr` is documented with `indexExprs: List<Expr*>` and the note "(one or more indices)" — but Slang's surface for the multi-index form (`a[i, j]` vs `a[i][j]`) is left implicit. The AST stores a list; the surface uses chained `[]` for plain arrays and varies for buffer / matrix types. | A worked example or a cross-link to `grammar.md` listing the supported surface forms would let us anchor a tighter test. |
| [#fwddifff-is-accepted-when-f-is-differentiable-otherwise-diagnosed](../../../docs/llm-generated/ast-reference/expressions.md#fwddifff-is-accepted-when-f-is-differentiable-otherwise-diagnosed) | undocumented-behavior | The Differentiate-family Notable Nodes section hands off to `pipeline/05-ir-passes.md` for the autodiff arithmetic. The doc for this bundle could add a one-line claim about the expression-level acceptance contract (i.e. "`__fwd_diff(f)` is accepted when `f` is `[Differentiable]`; otherwise diagnosed"); the current claim is silent on the negative case and we did not write a negative test for it. |  |
| [#tryexpr](../../../docs/llm-generated/ast-reference/expressions.md#tryexpr) | missing-example | `TryExpr` has user-spellable surface (`try expr` with Standard / Optional / Assert clauseType) but the doc neither spells out a minimal usage example nor lists the diagnostic emitted when the surrounding function is not `throws`. We attempted a test and could not anchor it cleanly to the doc; no test for TryExpr ships in this bundle. |  |
| [#element-count-of-a-static-array](../../../docs/llm-generated/ast-reference/expressions.md#element-count-of-a-static-array) | drift-from-source | `CountOfExpr` is documented as "element count of a static array", but the actual checker requires a type pack or `Tuple<...>` — passing a plain `int[N]` is rejected with "argument to countof can only be a type pack or tuple". The doc's description is misleading; either the implementation or the doc should change, and our test follows the implementation by using a variadic generic. |  |
| [#dispatchkernelexpr](../../../docs/llm-generated/ast-reference/expressions.md#dispatchkernelexpr) | missing-example | `DispatchKernelExpr` is listed in the Nodes table with a user-spelled form `__dispatch_kernel` but no example of the surface arguments (`threadGroupSize` / `dispatchSize`) or the contexts where it is accepted. We did not write a test for it. |  |
| [#array-index-out-of-bounds](../../../docs/llm-generated/ast-reference/expressions.md#array-index-out-of-bounds) | undocumented-behavior | The `IndexExpr` row does not commit to a static-bounds behavior for `arr[N]` where `N >= length(arr)`. The implementation emits `E30029 "array index out of bounds"` at a late IR stage that the `DIAGNOSTIC_TEST` runner does not capture by position; we could not anchor a one-past-end boundary test as a portable negative. | A short claim in the IndexExpr section naming the rejection contract and the stage at which it fires would let an agent attach a test for the index-`N` / index-`N+1` upper boundary. |
| [#parsed-value](../../../docs/llm-generated/ast-reference/expressions.md#parsed-value) | undocumented-behavior | The `FloatingPointLiteralExpr` row mentions a "parsed value" but does not specify the in-language surface for IEEE-special values (`+inf`, `-inf`, `NaN`). The boundary tests in this bundle observe the values indirectly (overflow, `0/0`, `1/±0`) because no direct literal spelling is documented. | A note pointing at the canonical way to spell those values (or confirming there is no direct spelling) would let us write tighter literal-level boundary tests. |

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
