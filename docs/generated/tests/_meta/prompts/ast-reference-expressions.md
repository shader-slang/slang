# Prompt: docs/generated/tests/ast-reference/expressions/

See [`_common.md`](_common.md) for universal rules. See
[`ast-reference-base.md`](ast-reference-base.md) for the sibling AST
bundle that establishes the AST-level "translation rule" (claims to
observations) — that rule applies here unchanged.

## Target

Produce the test bundle at `docs/generated/tests/ast-reference/expressions/`,
anchored to
[`docs/generated/design/ast-reference/expressions.md`](../../../docs/generated/design/ast-reference/expressions.md).

Audience: nightly CI. The bundle exercises the concrete `Expr`
subclasses — `VarExpr`, `MemberExpr`, `IndexExpr`, `InvokeExpr` and its
operator/cast specializations, the `LiteralExpr` family, `SwizzleExpr`,
`SelectExpr`, `LogicOperatorShortCircuitExpr`, `AssignExpr`,
`SizeOfExpr` / `AlignOfExpr` / `CountOfExpr`, the `Is/As/CastToSuperTypeExpr`
trio, the differentiate-family expressions, `LambdaExpr`, and the
type-expression family (`PointerTypeExpr`, `FuncTypeExpr`,
`TupleTypeExpr`, `ThisTypeExpr`, etc.) — through their **observable
consequences** in source-level behavior.

## The translation rule (carried from `ast-reference-base.md`)

`expressions.md` enumerates one concrete `Expr` subclass per syntactic
expression kind. Slang as a compiler does **not** expose its AST to the
user. So a claim such as "`InvokeExpr` has a `functionExpr` field and
a list of `arguments`" is testable only via its observable surface: a
function call returns the right value, has the right overload picked
on the basis of argument types, or diagnoses when no overload applies.

- **Testable** ⇔ "if the doc's claim about this expression kind were
  false, the program-text behavior we wrote would change in a way
  `slangc` reports."
- **Not testable through slangc** ⇔ "the claim is about which C++
  class the parser allocated for the node, which field on that class
  holds the operands, or which intermediate AST class is later
  rewritten by the checker."

### Observable claims (write tests for these)

The doc's `## Nodes` table lists ~60 concrete expression kinds. Most
claims are user-observable as the value, type, or diagnostic produced
by the expression. Group them by family for coverage:

- **VarExpr / DeclRefExpr** (`#varexpr-and-declrefexpr`,
  `#nodes`) — a name reference resolves to a declaration; an
  undeclared name diagnoses.
- **LiteralExpr family** (`#literalexpr-family`, `#nodes`) — integer,
  float, bool, string, `nullptr`, `none` literals each carry the
  expected typed value. Suffixed integer literal (`1u`, `1.0f`)
  picks the suffixed base type for overload resolution.
- **InvokeExpr and the call/operator/cast unification**
  (`#invokeexpr-and-the-call-operator-cast-unification`) — a function
  call, an operator application, and an explicit cast all participate
  in overload resolution uniformly. Observable axes: `f(x)` calls
  `f`; `a + b` calls the `+` operator overload; `(T)x` invokes the
  appropriate conversion.
- **InfixExpr / PrefixExpr / PostfixExpr / SelectExpr**
  (`#nodes`) — these inherit observable arithmetic/comparison
  semantics; coverage is per-operator-kind and includes precedence
  edge cases (which `pipeline/02-parse-ast/` covers as parse-stage;
  here we cover **value** correctness per node kind).
- **LogicOperatorShortCircuitExpr** (`#nodes`) — `&&` / `||`
  short-circuit; the right operand is not evaluated when the left
  decides the result.
- **MemberExpr / StaticMemberExpr / DerefMemberExpr**
  (`#memberexpr-staticmemberexpr-derefmemberexpr`, `#nodes`) — member
  access on a value (`a.b`), on a type (`T::m` / `T.m`), and on a
  pointer-like value (`a->b`). The checker picks the right one based
  on the base's type.
- **SwizzleExpr / MatrixSwizzleExpr** (`#nodes`) — vector swizzle
  `v.xyz` and matrix swizzle `m._m00_m11`. Vector swizzle is one of
  the rare expression-level claims whose **emit** differs per target;
  include a multi-target `SIMPLE` directive.
- **IndexExpr** (`#nodes`) — `a[i]` subscript; chains as
  `a[i][j]` or `a[i, j]` depending on type.
- **AssignExpr** (`#nodes`) — `a = b` is an expression and its value
  is the assigned value; assigning to a non-l-value diagnoses.
- **ParenExpr / TupleExpr / InitializerListExpr** (`#nodes`) —
  parenthesized, tuple (`(a, b)`), and brace-initializer (`{a, b}`)
  forms.
- **TypeCastExpr / ExplicitCastExpr** (`#nodes`) — `(T)x` converts
  between numeric types; an unconvertible cast diagnoses.
- **SizeOfExpr / AlignOfExpr / CountOfExpr** (`#nodes`) — produce
  compile-time-known sizes of types or static arrays.
- **IsTypeExpr / AsTypeExpr / CastToSuperTypeExpr**
  (`#astypeexpr-istypeexpr-casttosupertypeexpr`) — runtime/compile-time
  type tests and subtype casts; require a conformance witness.
- **GenericAppExpr / PartiallyAppliedGenericExpr**
  (`#partiallyappliedgenericexpr`) — `g<T>` applies a generic, and a
  partially-applied generic completes by overload resolution at the
  call site.
- **NewExpr / ExplicitCtorInvokeExpr** (`#nodes`) — explicit
  constructor invocation; `new T(...)` where supported.
- **LambdaExpr** (`#lambdaexpr-and-lambdadecl`) — `(params) => body`
  forms a callable; calling it produces the expected value.
- **DifferentiateExpr family** (`#differentiate-family-expressions`)
  — `__fwd_diff(f)(x)` and `__bwd_diff(f)(x, dx)` selectors over a
  `[Differentiable]` function. Only the structural acceptance is
  observable here; the actual derivative arithmetic is the autodiff
  pipeline's territory.
- **Type-expression family**
  (`#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr`)
  — `T*`, `(T1) -> R`, `(T1, T2)` tuple, `T & U` conjunction,
  `This` keyword, etc. The observable form is "the spelling is
  accepted in the right syntactic position".
- **`true`/`false` short-circuit interaction with SelectExpr** — the
  ternary `c ? a : b` evaluates only the chosen arm.
- **Failure modes** — assigning to a non-l-value, an undeclared name
  reference, an unconvertible explicit cast, an unresolved member
  reference: each of these is a negative test for one expression
  kind.

### Not testable through slangc (do NOT write tests for these)

- That `InvokeExpr` derives from `AppExprBase` (internal class
  identity).
- That `OverloadedExpr` is materialized between name lookup and
  overload resolution (an intermediate node the user never sees;
  what's observable is the final diagnostic when resolution fails).
- The `originalFunctionExpr` field on `AppExprBase`.
- That `LiteralExpr` stores the originating `Token` (internal).
- The internal layout of `IndexExpr::indexExprs` as `List<Expr*>`.
- That synthesized nodes (`MakeArrayFromElementExpr`,
  `DefaultConstructExpr`, `GetArrayLengthExpr`, `BuiltinCastExpr`,
  `ImplicitCastExpr`, the `LValueImplicitCastExpr` chain,
  `MakeRefExpr`, `OpenRefExpr`, `ExtractExistentialValueExpr`,
  `PackExpr`, `SharedTypeExpr`, `MakeOptionalExpr`, `ModifierCastExpr`,
  `ReturnValExpr`, `FloatBitCastExpr`, `FuncAsTypeExpr`,
  `FuncTypeOfExpr`, `IncompleteExpr`, `AggTypeCtorExpr`) exist as
  distinct AST nodes — these are checker-synthesized or
  parser-placeholder kinds with no user-written spelling. Their
  effects bleed through into other observables (an implicit cast is
  observable as the conversion happening, not as a node identity).
- That `EachExpr` / `ExpandExpr` / pack-query / shape-pack expressions
  exist as distinct nodes — these are observable through the pack /
  variadic-generic feature, which is covered by a separate bundle
  (`language-feature/generics-and-packs`, when written). Anchor any
  pack-shape claims to the surface grammar, not to the AST node
  catalog here.
- That `SPIRVAsmExpr` carries inline SPIR-V assembly — the inline-asm
  feature is a separate sub-language; do not test it here.

If you find yourself thinking "this would verify that the AST node
allocated is class X", stop — that is a source-targeting probe in
disguise, the same anti-pattern called out in
`ast-reference-base.md`.

## Avoid duplication with sibling bundles

This bundle is **AST-node-kind centric**. Adjacent bundles cover
adjacent surfaces:

- `docs/generated/tests/pipeline/02-parse-ast/` covers **parse-stage**
  observable claims (precedence, associativity, the `<`-disambiguation
  heuristic, expr/type unification at parse time). Do not duplicate
  precedence/associativity tests here; cite that bundle's anchors if a
  parser-stage observation is required as a precondition.
- `docs/generated/tests/pipeline/03-semantic-check/` covers **checker-stage**
  observable claims (overload resolution mechanics, implicit-cast
  insertion, type-mismatch diagnostics). Do not duplicate
  overload-resolution rules here; the angle here is "this **kind of
  expression** participates in overload resolution".
- `docs/generated/tests/ast-reference/base/` covers the abstract-base-class
  story (`Expr` carries a `QualType`, etc.). Do not re-test that here;
  cite it if needed.

The angle for this bundle's tests is: **per concrete expression kind
in the `## Nodes` table, one observable consequence of that kind**.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` rather than `## Untested claims`:
   the "out of scope" reasons here are mostly "internal AST node
   identity" and "synthesized-not-user-spellable", which are not
   GPU-related. List the synthesized nodes from `## Nodes` that are
   not user-spellable in this section, citing the AST-node row in the
   doc.
2. 25 to 50 `.slang` test files. The doc enumerates ~60 nodes; aim
   for one observable test per user-spellable kind, plus a handful of
   negative tests for the most natural rejection cases. The cap is
   80, but quality matters more than quantity — drop a test rather
   than ship a flaky one. Bundle limit: 80.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/ast-reference/expressions.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off or the primary anchor is too coarse):

- `docs/generated/design/ast-reference/base.md`
- `docs/generated/design/syntax-reference/grammar.md`
- `docs/generated/design/pipeline/02-parse-ast.md`
- `docs/generated/design/pipeline/03-semantic-check.md`
- `docs/generated/design/name-resolution/overload-resolution.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm that the doc's claim about a
concrete AST node corresponds to the actual header / parser entry
point (e.g. that the `??` operator is parsed via a particular
production). You may **not** mine them for behavioral claims that the
doc does not make, and you may **not** write tests that probe internal
class identity.

- `source/slang/slang-ast-expr.h`
- `source/slang/slang-ast-base.h`
- `source/slang/slang-parser.cpp`

## Test directives

Most expression-level claims are target-independent (the AST node and
its checker-attached type are the same regardless of backend). So:

- `//TEST:INTERPRET(filecheck=CHECK):` — **primary** directive. Use
  `printf` to observe a typed value, the result of an operator, the
  branch a `SelectExpr` took, the value an `IndexExpr` produced, etc.
- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — for negative claims:
  assignment to non-l-value, undeclared decl-ref, unconvertible
  explicit cast, member-access of a non-existent field.
- Multi-target `//TEST:SIMPLE` is appropriate only when **expression
  lowering observably differs per target**. The clearest case is
  `SwizzleExpr` (vector swizzles emit different text in HLSL vs.
  GLSL vs. SPIR-V vs. Metal vs. WGSL vs. CUDA). For these, add one
  `//TEST:SIMPLE(filecheck=CHECK):-target ...` directive per feasible
  text-emit target and FileCheck a robust pattern. Do **not** add
  multi-target tests for, e.g., integer addition — the emit is
  identical in concept and the cost is repetition.

## Cast and observation reminders (carried from `_common.md` / pilot)

- Use **constructor-style casts** (`int('A')`), not C-style
  (`(int)'A'`). The C-style form returns 0 in `slangi` for some
  conversions.
- `slangi` `printf` does **not** support `%s`. For string-typed
  observation (e.g. confirming a `StringLiteralExpr` value), use
  `//TEST:SIMPLE(filecheck=CHECK):-target cpp` and FileCheck the
  emitted C++ source for the literal text.
- `static const int x = N;` at file scope is the cleanest pattern for
  asserting a compile-time-known result of `sizeof` / `alignof` /
  `countof`.
- FileCheck identifier mangling: emitted code may suffix locals
  (`a_0`, `__ldg(&...->a_0)`). Use wildcards (`a_{{[0-9]+}}`) where
  the local name matters.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ast-reference/expressions.md` (or one of the listed secondary
      docs).
- [ ] At least one test per **user-spellable expression family**
      named in `## Nodes`: VarExpr, literal family, InvokeExpr,
      InfixExpr (binary op), PrefixExpr / PostfixExpr, SelectExpr,
      LogicOperatorShortCircuit (`&&` / `||`), MemberExpr,
      StaticMemberExpr, DerefMemberExpr (if pointer feature
      supported), SwizzleExpr, IndexExpr, AssignExpr, ParenExpr,
      TupleExpr, InitializerListExpr, ExplicitCastExpr, SizeOfExpr,
      AlignOfExpr, CountOfExpr, IsTypeExpr, AsTypeExpr,
      GenericAppExpr, ExplicitCtorInvokeExpr, NewExpr (if `new`
      supported), LambdaExpr, DifferentiateExpr family, type-expression
      family.
- [ ] At least one diagnostic test per natural rejection family:
      assignment to non-l-value, undeclared decl-ref,
      no-applicable-overload, unconvertible cast, member-access of
      missing field.
- [ ] No test asserts the C++ identity of an AST node.
- [ ] No test depends on a GPU. Multi-target `SIMPLE` directives are
      text-emit only.
- [ ] At least one multi-target test exists for the SwizzleExpr
      kind, since swizzle emit is the canonical target-divergent
      expression-lowering case.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-parser.cpp:NNNN`", stop and re-read the doc.
- [ ] Synthesized / non-user-spellable nodes from `## Nodes` are
      recorded under `## Untested claims` in `README.md`, not as tests.
- [ ] `README.md` `## Doc gaps observed` is honest. If a claim wanted
      a test but had no good anchor (or the anchor was too coarse),
      record it.
