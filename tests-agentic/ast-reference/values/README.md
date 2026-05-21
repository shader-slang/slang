---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:03:13Z
source_commit: ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
watched_paths_digest: 4f74d91e4cf48490043c25f1aa4fe35ca34e369bae1bba7089a2d3a8a0006cd1
source_doc: docs/llm-generated/ast-reference/values.md
source_doc_digest: 3c729c296770be4c202ddf019ae318401d2d1620ed7c48b1c4aaaf50f0f0762a
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ast-reference/values

## Intent

Tests verify the **user-observable** consequences of the non-Type
`Val` subclasses enumerated in
[`docs/llm-generated/ast-reference/values.md`](../../../docs/llm-generated/ast-reference/values.md):
the `IntVal` family (compile-time integers — `ConstantIntVal`,
`DeclRefIntVal`, `PolynomialIntVal`, `CountOfIntVal`, `SizeOfIntVal`,
`ErrorIntVal`), the `DeclRefBase` family (`GenericAppDeclRef`), the
`Witness` family (`DeclaredSubtypeWitness`, `TransitiveSubtypeWitness`),
and the `ModifierVal` family (`UNormModifierVal`).

The values reference is fundamentally about internal AST shape:
hash-cons identity, operand layouts, abstract intermediates, and the
classes used by the autodiff / pack subsystems. Those claims are
unobservable through `slang-test`. The surface-visible consequences
that this bundle exercises are:

- a compile-time integer flows into an array-bound and the array
  carries that many elements;
- a generic value parameter is unsubstituted until instantiation, at
  which point it collapses to its concrete integer;
- polynomial canonicalization makes textually-different but
  algebraically-equal index expressions name the same type;
- `countof` of a concrete pack folds to a ConstantIntVal at check
  time;
- `sizeof` of a scalar is a positive integer at compile time;
- distinct generic instantiations are distinct types
  (`GenericAppDeclRef`'s identity flows through);
- a witness from `InheritanceDecl` enables interface-method dispatch
  through a constraint;
- a transitive witness enables base-interface dispatch on a derived
  conformer;
- a type-level `unorm` modifier survives lowering to HLSL emit;
- `ErrorIntVal` keeps checking running when an array bound cannot be
  computed.

`INTERPRET` is the primary directive. One `-target hlsl` test
(`unormmodifierval-resource-format.slang`) is used because the
`unorm` modifier's observable surface is in the emitted text.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                | Claim (one line)                                                                                                                       | Tests                                                |
| -------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------- |
| C-01     | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family)                                                                                   | `ConstantIntVal` is the leaf IntVal for a known compile-time integer; it is the IntVal that flows into a literal-sized array bound.    | [`constantintval-array-size.slang`](constantintval-array-size.slang)                    |
| C-02     | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family)                                                                                   | `DeclRefIntVal` names an unsubstituted generic value parameter and collapses to a constant after substitution.                         | [`declrefintval-generic-value-param.slang`](declrefintval-generic-value-param.slang)            |
| C-03     | [#polynomialintval-and-polynomial-canonicalization](../../../docs/llm-generated/ast-reference/values.md#polynomialintval-and-polynomial-canonicalization)             | `PolynomialIntVal` canonicalizes polynomial expressions; `2*N + 3` and `3 + 2*N` are the same type, so array types using them match.   | [`polynomialintval-equivalent-expressions.slang`](polynomialintval-equivalent-expressions.slang)      |
| C-04     | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family)                                                                                   | `CountOfIntVal` folds to a `ConstantIntVal` for a concrete type pack with a known element count.                                       | [`countof-intval-pack-length.slang`](countof-intval-pack-length.slang)                   |
| C-05     | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family)                                                                                   | `SizeOfIntVal` is a compile-time integer; for a non-empty scalar type it is a positive integer.                                        | [`sizeof-intval-compile-time.slang`](sizeof-intval-compile-time.slang)                   |
| C-06     | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family)                                                                                   | `ErrorIntVal` is the placeholder used when an integer value cannot be computed; checking continues with the root-cause diagnostic.     | [`errorintval-suppresses-cascade.slang`](errorintval-suppresses-cascade.slang)               |
| C-07     | [#declref-family](../../../docs/llm-generated/ast-reference/values.md#declref-family)                                                                                 | `GenericAppDeclRef`s of the same decl with identical argument lists yield type-compatible references; cross-assignment is accepted.    | [`genericappdeclref-same-args-compatible.slang`](genericappdeclref-same-args-compatible.slang)       |
| C-08     | [#declref-family](../../../docs/llm-generated/ast-reference/values.md#declref-family)                                                                                 | `GenericAppDeclRef`s of the same decl with different generic arguments are distinct types; cross-assignment is rejected.               | [`genericappdeclref-different-args-distinct.slang`](genericappdeclref-different-args-distinct.slang)    |
| C-09     | [#witness-and-witness-table-evidence](../../../docs/llm-generated/ast-reference/values.md#witness-and-witness-table-evidence)                                         | `DeclaredSubtypeWitness` carries the proof from `InheritanceDecl`; it enables interface-method dispatch through a generic constraint.  | [`declaredsubtypewitness-interface-dispatch.slang`](declaredsubtypewitness-interface-dispatch.slang)    |
| C-10     | [#witness-and-witness-table-evidence](../../../docs/llm-generated/ast-reference/values.md#witness-and-witness-table-evidence)                                         | `TransitiveSubtypeWitness` composes two witnesses along an inheritance chain; a derived conformer satisfies a base-interface bound.    | [`transitivesubtypewitness-chain.slang`](transitivesubtypewitness-chain.slang)               |
| C-11     | [#modifier-values](../../../docs/llm-generated/ast-reference/values.md#modifier-values)                                                                               | `UNormModifierVal` is the type-level Val for `unorm`; the modifier survives lowering and appears in HLSL emit.                         | [`unormmodifierval-resource-format.slang`](unormmodifierval-resource-format.slang)             |

## Tests in this bundle

| File                                              | Intent     | Doc anchor                                                          |
| ------------------------------------------------- | ---------- | ------------------------------------------------------------------- |
| [`constantintval-array-size.slang`](constantintval-array-size.slang)                 | functional | `#intval-family`                                                    |
| [`declrefintval-generic-value-param.slang`](declrefintval-generic-value-param.slang)         | functional | `#intval-family`                                                    |
| [`polynomialintval-equivalent-expressions.slang`](polynomialintval-equivalent-expressions.slang)   | functional | `#polynomialintval-and-polynomial-canonicalization`                 |
| [`countof-intval-pack-length.slang`](countof-intval-pack-length.slang)                | functional | `#intval-family`                                                    |
| [`sizeof-intval-compile-time.slang`](sizeof-intval-compile-time.slang)                | functional | `#intval-family`                                                    |
| [`errorintval-suppresses-cascade.slang`](errorintval-suppresses-cascade.slang)            | negative   | `#intval-family`                                                    |
| [`genericappdeclref-same-args-compatible.slang`](genericappdeclref-same-args-compatible.slang)    | functional | `#declref-family`                                                   |
| [`genericappdeclref-different-args-distinct.slang`](genericappdeclref-different-args-distinct.slang) | negative   | `#declref-family`                                                   |
| [`declaredsubtypewitness-interface-dispatch.slang`](declaredsubtypewitness-interface-dispatch.slang) | functional | `#witness-and-witness-table-evidence`                               |
| [`transitivesubtypewitness-chain.slang`](transitivesubtypewitness-chain.slang)            | functional | `#witness-and-witness-table-evidence`                               |
| [`unormmodifierval-resource-format.slang`](unormmodifierval-resource-format.slang)          | functional | `#modifier-values`                                                  |

## Doc gaps observed

- The doc names `WitnessLookupIntVal` as "an integer value resolved
  through a witness-table lookup", but the surface spelling that
  produces this Val is not given. Without a surface form (e.g. an
  interface with a `static const int` requirement that another
  generic reads), it is not anchorable.
- The doc names `FuncCallIntVal` as "a compile-time call to an
  integer-returning function" but does not name the spelling. Slang
  has `__intrinsic` / `[__BuiltinFunc]` helpers, but no portable
  user-spellable form is documented. A doc-level pointer to a
  minimal example (e.g. a `constexpr`-like function call in array
  bounds) would let an agent anchor a test.
- `TypeCastIntVal` is "an integer cast to a different integer type"
  but the surface that produces this Val (vs. just an inline cast in
  IR) is not named. A spelling pointer would help.
- The `Polynomial helpers` section (`PolynomialIntValFactor`,
  `PolynomialIntValTerm`) is purely internal: they "are not `IntVal`s
  themselves: they appear as operands of a `PolynomialIntVal`".
  Their observable surface is identical to `PolynomialIntVal`'s; no
  user-spellable distinction exists. The doc could state this
  explicitly under `## Out of scope`.
- The doc lists `MemberDeclRef`, `LookupDeclRef`, and `DirectDeclRef`
  in the `DeclRefBase` family but their user-observable surface is
  the same as a plain identifier lookup. No claim distinguishes them
  at the surface level (the distinction is internal: "how the
  declaration was reached"). A doc-level note saying "these three
  variants are observationally indistinguishable through `slangc`;
  only `GenericAppDeclRef` produces a distinct surface (generic
  args)" would clarify boundary ownership.
- The doc lists all the pack-related `IntVal` and `SubtypeWitness`
  variants (`FirstIntVal`, `LastIntVal`, `EachIntVal`,
  `EachSubtypeWitness`, etc.). The user-observable surface for these
  belongs to the variadic-generic / pack feature bundle. The doc
  could note that explicitly.
- The doc lists the `DifferentiateVal` family
  (`ForwardDifferentiateVal`, `BackwardDifferentiateVal`, ...)
  alongside autodiff machinery. Their observable surface belongs to
  the autodiff feature bundle. A doc pointer to that bundle would
  clarify ownership.

## Out of scope

The doc is overwhelmingly about internal AST shape. The following
claim families are not observable through `slangc` / `slang-test`
directives that this bundle can run; they are recorded here rather
than tested.

- **Hash-cons identity / `Val*` pointer equality** described under
  [`### Hash-consing and the ASTBuilder`](../../../docs/llm-generated/ast-reference/values.md#hash-consing-and-the-astbuilder).
  Surface tests can only observe behavioral equivalence (e.g. that
  two specializations of a generic with the same argument are
  interchangeable); pointer equality of two `Val*` is not visible.
  This bundle's `genericappdeclref-same-args-compatible.slang`
  exercises the **observable** form of this invariant.
- **Operand-list layout** described under `## Nodes` ("operand
  semantics" column). The `m_operands: List<ValNodeOperand>` storage
  and the indices each concrete class uses are not visible at the
  surface.
- **Abstract intermediates** named in the doc:
  `IntVal`, `SizeOfLikeIntVal`, `ShapeTransformIntValPack`, `Witness`,
  `SubtypeWitness`, `TypeCoercionWitness`. These produce no user
  spelling of their own; only their concrete descendants are
  user-anchorable.
- **The C++ parent class** of any concrete `Val` (e.g. that
  `SizeOfIntVal` extends `SizeOfLikeIntVal`, that `UNormModifierVal`
  extends `ResourceFormatModifierVal`). Only the user-observable
  role of each leaf is testable.
- **Singleton-ness of `NoneWitness`** (and any other zero-operand
  Val): hash-cons makes it one-per-`ASTBuilder`, but the surface
  cannot distinguish two `Val*` from one.
- **`DirectDeclRef` / `MemberDeclRef` / `LookupDeclRef`** as
  distinct surface observations. The doc's distinction is internal
  ("how the declaration was reached"); a plain identifier or a
  qualified member reference looks the same at the type-checker
  surface. Only `GenericAppDeclRef`'s presence of generic arguments
  is user-distinguishable.
- **`FuncCallIntVal`** — the doc names it but does not give a
  surface spelling. Recorded as a doc gap above.
- **`WitnessLookupIntVal`** — same as `FuncCallIntVal`; no surface
  spelling documented. Recorded as a doc gap.
- **`TypeCastIntVal`** — no surface-distinct spelling from a plain
  cast in an array bound. Recorded as a doc gap.
- **Polynomial helpers** (`PolynomialIntValFactor`,
  `PolynomialIntValTerm`) — the operands of `PolynomialIntVal`; no
  user-observable surface distinct from `PolynomialIntVal` itself.
- **Pack / variadic IntVals and SubtypeWitnesses**:
  `FirstIntVal`, `LastIntVal`, `ConcreteIntValPack`,
  `TrimFirstIntValPack`, `TrimLastIntValPack`,
  `ShapeConcatIntValPack`, `ShapePermuteIntValPack`,
  `ShapeSwapIntValPack`, `ShapeReduceIntValPack`,
  `ExpandIntValPack`, `EachIntVal`, `TypePackSubtypeWitness`,
  `EachSubtypeWitness`, `FirstSubtypeWitness`, `LastSubtypeWitness`,
  `TrimFirstSubtypeWitness`, `TrimLastSubtypeWitness`,
  `PackBranchSubtypeWitness`, `ExpandSubtypeWitness`,
  `NonEmptyPackWitness`. These are observable through the
  pack-expression / variadic-generic feature; that surface belongs
  to the `language-feature/generics-and-packs` bundle (or the
  expressions bundle's pack tests).
- **Differentiation `Val`s**: `DifferentiateVal`,
  `ForwardDifferentiateVal`, `BackwardDifferentiateVal`,
  `BackwardDifferentiateIntermediateTypeVal`,
  `BackwardDifferentiatePrimalVal`,
  `BackwardDifferentiatePropagateVal`. Observable through the
  autodiff feature; that surface belongs to the autodiff bundle.
- **`HasDiffTypeInfoWitness`**, **`DiffTypeInfoWitness`**,
  **`HigherOrderDiffTypeTranslationWitness`** — autodiff-specific
  witnesses; their surface belongs to the autodiff bundle.
- **`ExtractExistentialSubtypeWitness`** — "evidence carried by an
  opened existential value". Observable through the existential /
  `some IFoo` surface, which belongs to the existential-feature
  bundle.
- **`DynamicSubtypeWitness`** — "evidence used for `DynamicType`
  dispatch". `DynamicType` is not user-spellable (it is produced by
  existential elimination); recorded under types.md's out-of-scope
  list. The witness has no distinct user-observable surface.
- **`TypeCoercionWitness`** family (`BuiltinTypeCoercionWitness`,
  `DeclRefTypeCoercionWitness`) — these underlie implicit-cast
  insertion. The mechanics of implicit casting are owned by the
  `pipeline/03-semantic-check` bundle; the witness identities have
  no distinct surface observation.
- **`SNormModifierVal`**, **`NoDiffModifierVal`** — additional
  type-level modifier Vals. `unorm` is exercised
  (`unormmodifierval-resource-format.slang`); `snorm` / `no_diff`
  share the same modifier-survival observation and are not
  duplicated here. `no_diff` in particular belongs to the autodiff
  bundle.
- **`UIntSetVal`** — "a hash-consed bitset used by the capability
  system". The capability system has its own bundle; the bitset
  identity has no direct user-spelling.
- **`TypeEqualityWitness`** — surface-observable as "two type
  aliases yield interchangeable values". That observation is owned
  by `ast-reference/types`'s `NamedExpressionType` / typedef tests
  (or by the `pipeline/03-semantic-check` bundle's equality rules).
  Not duplicated here.
- **The `## Family hierarchy` mermaid diagram itself.**
