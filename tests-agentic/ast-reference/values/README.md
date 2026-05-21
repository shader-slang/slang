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

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Two GenericAppDeclRefs of the same decl with different generic arguments are distinct types; assigning one to the other is rejected by the type checker. | negative | [#declref-family](../../../docs/llm-generated/ast-reference/values.md#declref-family) | [`genericappdeclref-different-args-distinct.slang`](genericappdeclref-different-args-distinct.slang) |
| Two GenericAppDeclRefs of the same generic decl with identical argument lists produce type-compatible references; a value of one is assignable to the other. | functional | [#declref-family](../../../docs/llm-generated/ast-reference/values.md#declref-family) | [`genericappdeclref-same-args-compatible.slang`](genericappdeclref-same-args-compatible.slang) |
| A ConstantIntVal is the leaf IntVal used for a known compile-time integer; it stands in for the literal array size at type-check time and the array carries that many elements. | functional | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family) | [`constantintval-array-size.slang`](constantintval-array-size.slang) |
| A CountOfIntVal is a compile-time IntVal that produces the element count of a concrete type pack; the integer is usable wherever a compile-time integer is accepted. | functional | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family) | [`countof-intval-pack-length.slang`](countof-intval-pack-length.slang) |
| A DeclRefIntVal is an unsubstituted generic value parameter; after the generic is instantiated, the IntVal collapses to its concrete value and the array carries the substituted size. | functional | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family) | [`declrefintval-generic-value-param.slang`](declrefintval-generic-value-param.slang) |
| A SizeOfIntVal is a compile-time IntVal produced by sizeof; the resulting integer is usable at compile time and prints as a positive integer. | functional | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family) | [`sizeof-intval-compile-time.slang`](sizeof-intval-compile-time.slang) |
| An ErrorIntVal is the integer-value placeholder used when a compile-time integer cannot be computed; checking continues without cascading further integer-value diagnostics. | negative | [#intval-family](../../../docs/llm-generated/ast-reference/values.md#intval-family) | [`errorintval-suppresses-cascade.slang`](errorintval-suppresses-cascade.slang) |
| UNormModifierVal is the type-level ModifierVal for an unorm resource-format modifier; a `Texture2D<unorm float4>` declaration preserves the unorm modifier through to the emitted HLSL surface. | functional | [#modifier-values](../../../docs/llm-generated/ast-reference/values.md#modifier-values) | [`unormmodifierval-resource-format.slang`](unormmodifierval-resource-format.slang) |
| A PolynomialIntVal stores a polynomial in unsubstituted parameters in a canonical form; type-equality for dependent array types treats commuted or re-associated expressions of the same polynomial as the same type. | functional | [#polynomialintval-and-polynomial-canonicalization](../../../docs/llm-generated/ast-reference/values.md#polynomialintval-and-polynomial-canonicalization) | [`polynomialintval-equivalent-expressions.slang`](polynomialintval-equivalent-expressions.slang) |
| A DeclaredSubtypeWitness represents the proof carried by an InheritanceDecl that T conforms to interface I; the witness is what permits dispatching I's methods on a T value through a generic constraint. | functional | [#witness-and-witness-table-evidence](../../../docs/llm-generated/ast-reference/values.md#witness-and-witness-table-evidence) | [`declaredsubtypewitness-interface-dispatch.slang`](declaredsubtypewitness-interface-dispatch.slang) |
| A TransitiveSubtypeWitness composes two existing witnesses A:B and B:C to obtain A:C; a concrete type that implements a derived interface satisfies a generic constraint on the base interface. | functional | [#witness-and-witness-table-evidence](../../../docs/llm-generated/ast-reference/values.md#witness-and-witness-table-evidence) | [`transitivesubtypewitness-chain.slang`](transitivesubtypewitness-chain.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#an-integer-value-resolved-through-a-witness-table-lookup](../../../docs/llm-generated/ast-reference/values.md#an-integer-value-resolved-through-a-witness-table-lookup) | undocumented-behavior | The doc names `WitnessLookupIntVal` as "an integer value resolved through a witness-table lookup", but the surface spelling that produces this Val is not given. | Without a surface form (e.g. an interface with a `static const int` requirement that another generic reads), it is not anchorable. |
| [#a-compile-time-call-to-an-integer-returning-function](../../../docs/llm-generated/ast-reference/values.md#a-compile-time-call-to-an-integer-returning-function) | undocumented-behavior | The doc names `FuncCallIntVal` as "a compile-time call to an integer-returning function" but does not name the spelling. Slang has `__intrinsic` / `[__BuiltinFunc]` helpers, but no portable user-spellable form is documented. A doc-level pointer to a minimal example (e.g. a `constexpr`-like function call in array bounds) would let an agent anchor a test. |  |
| [#an-integer-cast-to-a-different-integer-type](../../../docs/llm-generated/ast-reference/values.md#an-integer-cast-to-a-different-integer-type) | undocumented-behavior | `TypeCastIntVal` is "an integer cast to a different integer type" but the surface that produces this Val (vs. just an inline cast in IR) is not named. A spelling pointer would help. |  |
| [#out-of-scope](../../../docs/llm-generated/ast-reference/values.md#out-of-scope) | undocumented-behavior | The `Polynomial helpers` section (`PolynomialIntValFactor`, `PolynomialIntValTerm`) is purely internal: they "are not `IntVal`s themselves: they appear as operands of a `PolynomialIntVal`". Their observable surface is identical to `PolynomialIntVal`'s; no user-spellable distinction exists. The doc could state this explicitly under `## Out of scope`. |  |
| [#how-the-declaration-was-reached](../../../docs/llm-generated/ast-reference/values.md#how-the-declaration-was-reached) | undocumented-behavior | The doc lists `MemberDeclRef`, `LookupDeclRef`, and `DirectDeclRef` in the `DeclRefBase` family but their user-observable surface is the same as a plain identifier lookup. No claim distinguishes them at the surface level (the distinction is internal: "how the declaration was reached"). A doc-level note saying "these three variants are observationally indistinguishable through `slangc`; only `GenericAppDeclRef` produces a distinct surface (generic args)" would clarify boundary ownership. |  |
| [#intval](../../../docs/llm-generated/ast-reference/values.md#intval) | undocumented-behavior | The doc lists all the pack-related `IntVal` and `SubtypeWitness` variants (`FirstIntVal`, `LastIntVal`, `EachIntVal`, `EachSubtypeWitness`, etc.). The user-observable surface for these belongs to the variadic-generic / pack feature bundle. The doc could note that explicitly. |  |
| [#differentiateval](../../../docs/llm-generated/ast-reference/values.md#differentiateval) | undocumented-behavior | The doc lists the `DifferentiateVal` family (`ForwardDifferentiateVal`, `BackwardDifferentiateVal`, ...) alongside autodiff machinery. Their observable surface belongs to the autodiff feature bundle. A doc pointer to that bundle would clarify ownership. |  |

## Out of scope

| Anchor | Reason | Claim | Why it's terminal |
| --- | --- | --- | --- |
| (unspecified) | (unclassified) | **The `## Family hierarchy` mermaid diagram itself.** | Not reachable via any allowed test directive. |
| [#directdeclref](../../../docs/llm-generated/ast-reference/values.md#directdeclref) | (unclassified) | **`DirectDeclRef` / `MemberDeclRef` / `LookupDeclRef`** as distinct surface observations. The doc's distinction is internal ("how the declaration was reached"); a plain identifier or a qualified member reference looks the same at the type-checker surface. | Only `GenericAppDeclRef`'s presence of generic arguments is user-distinguishable. |
| [#dynamicsubtypewitness](../../../docs/llm-generated/ast-reference/values.md#dynamicsubtypewitness) | (unclassified) | **`DynamicSubtypeWitness`** — "evidence used for `DynamicType` dispatch". `DynamicType` is not user-spellable (it is produced by existential elimination); recorded under types.md's out-of-scope list. The witness has no distinct user-observable surface. | Not reachable via any allowed test directive. |
| [#extractexistentialsubtypewitness](../../../docs/llm-generated/ast-reference/values.md#extractexistentialsubtypewitness) | (unclassified) | **`ExtractExistentialSubtypeWitness`** — "evidence carried by an opened existential value". Observable through the existential / `some IFoo` surface, which belongs to the existential-feature bundle. | Not reachable via any allowed test directive. |
| [#firstintval](../../../docs/llm-generated/ast-reference/values.md#firstintval) | (unclassified) | **Pack / variadic IntVals and SubtypeWitnesses**: `FirstIntVal`, `LastIntVal`, `ConcreteIntValPack`, `TrimFirstIntValPack`, `TrimLastIntValPack`, `ShapeConcatIntValPack`, `ShapePermuteIntValPack`, `ShapeSwapIntValPack`, `ShapeReduceIntValPack`, `ExpandIntValPack`, `EachIntVal`, `TypePackSubtypeWitness`, `EachSubtypeWitness`, `FirstSubtypeWitness`, `LastSubtypeWitness`, `TrimFirstSubtypeWitness`, `TrimLastSubtypeWitness`, `PackBranchSubtypeWitness`, `ExpandSubtypeWitness`, `NonEmptyPackWitness`. These are observable through the pack-expression / variadic-generic feature; that surface belongs to the `language-feature/generics-and-packs` bundle (or the expressions bundle's pack tests). | Not reachable via any allowed test directive. |
| [#funccallintval](../../../docs/llm-generated/ast-reference/values.md#funccallintval) | (unclassified) | **`FuncCallIntVal`** — the doc names it but does not give a surface spelling. Recorded as a doc gap above. | Not reachable via any allowed test directive. |
| [#hasdifftypeinfowitness](../../../docs/llm-generated/ast-reference/values.md#hasdifftypeinfowitness) | (unclassified) | **`HasDiffTypeInfoWitness`**, **`DiffTypeInfoWitness`**, **`HigherOrderDiffTypeTranslationWitness`** — autodiff-specific witnesses; their surface belongs to the autodiff bundle. | Not reachable via any allowed test directive. |
| [#hash-consing-and-the-astbuilder](../../../docs/llm-generated/ast-reference/values.md#hash-consing-and-the-astbuilder) | (unclassified) | **Hash-cons identity / `Val*` pointer equality** described under [`### Hash-consing and the ASTBuilder`](../../../docs/llm-generated/ast-reference/values.md#hash-consing-and-the-astbuilder). Surface tests can only observe behavioral equivalence (e.g. that two specializations of a generic with the same argument are interchangeable); pointer equality of two `Val*` is not visible. This bundle's `genericappdeclref-same-args-compatible.slang` exercises the **observable** form of this invariant. | Not reachable via any allowed test directive. |
| [#intval](../../../docs/llm-generated/ast-reference/values.md#intval) | (unclassified) | **Abstract intermediates** named in the doc: `IntVal`, `SizeOfLikeIntVal`, `ShapeTransformIntValPack`, `Witness`, `SubtypeWitness`, `TypeCoercionWitness`. These produce no user spelling of their own; only their concrete descendants are user-anchorable. | Not reachable via any allowed test directive. |
| [#nonewitness](../../../docs/llm-generated/ast-reference/values.md#nonewitness) | (unclassified) | **Singleton-ness of `NoneWitness`** (and any other zero-operand Val): hash-cons makes it one-per-`ASTBuilder`, but the surface cannot distinguish two `Val*` from one. | Not reachable via any allowed test directive. |
| [#operand-semantics](../../../docs/llm-generated/ast-reference/values.md#operand-semantics) | (unclassified) | **Operand-list layout** described under `## Nodes` ("operand semantics" column). The `m_operands: List<ValNodeOperand>` storage and the indices each concrete class uses are not visible at the surface. | Not reachable via any allowed test directive. |
| [#polynomialintvalfactor](../../../docs/llm-generated/ast-reference/values.md#polynomialintvalfactor) | (unclassified) | **Polynomial helpers** (`PolynomialIntValFactor`, `PolynomialIntValTerm`) — the operands of `PolynomialIntVal`; no user-observable surface distinct from `PolynomialIntVal` itself. | Not reachable via any allowed test directive. |
| [#snormmodifierval](../../../docs/llm-generated/ast-reference/values.md#snormmodifierval) | (unclassified) | **`SNormModifierVal`**, **`NoDiffModifierVal`** — additional type-level modifier Vals. `unorm` is exercised (`unormmodifierval-resource-format.slang`); `snorm` / `no_diff` share the same modifier-survival observation and are not duplicated here. `no_diff` in particular belongs to the autodiff bundle. | Not reachable via any allowed test directive. |
| [#typecastintval](../../../docs/llm-generated/ast-reference/values.md#typecastintval) | (unclassified) | **`TypeCastIntVal`** — no surface-distinct spelling from a plain cast in an array bound. Recorded as a doc gap. | Not reachable via any allowed test directive. |
| [#typeequalitywitness](../../../docs/llm-generated/ast-reference/values.md#typeequalitywitness) | (unclassified) | **`TypeEqualityWitness`** — surface-observable as "two type aliases yield interchangeable values". That observation is owned by `ast-reference/types`'s `NamedExpressionType` / typedef tests (or by the `pipeline/03-semantic-check` bundle's equality rules). Not duplicated here. | Not reachable via any allowed test directive. |
| [#uintsetval](../../../docs/llm-generated/ast-reference/values.md#uintsetval) | (unclassified) | **`UIntSetVal`** — "a hash-consed bitset used by the capability system". The capability system has its own bundle; the bitset identity has no direct user-spelling. | Not reachable via any allowed test directive. |
| [#val](../../../docs/llm-generated/ast-reference/values.md#val) | (unclassified) | **Differentiation `Val`s**: `DifferentiateVal`, `ForwardDifferentiateVal`, `BackwardDifferentiateVal`, `BackwardDifferentiateIntermediateTypeVal`, `BackwardDifferentiatePrimalVal`, `BackwardDifferentiatePropagateVal`. Observable through the autodiff feature; that surface belongs to the autodiff bundle. | Not reachable via any allowed test directive. |
| [#witnesslookupintval](../../../docs/llm-generated/ast-reference/values.md#witnesslookupintval) | (unclassified) | **`WitnessLookupIntVal`** — same as `FuncCallIntVal`; no surface spelling documented. Recorded as a doc gap. | Not reachable via any allowed test directive. |
| [#val](../../../docs/llm-generated/ast-reference/values.md#val) | internal-source-fact | **The C++ parent class** of any concrete `Val` (e.g. that `SizeOfIntVal` extends `SizeOfLikeIntVal`, that `UNormModifierVal` extends `ResourceFormatModifierVal`). | Only the user-observable role of each leaf is testable. |
| [#typecoercionwitness](../../../docs/llm-generated/ast-reference/values.md#typecoercionwitness) | out-of-bundle | **`TypeCoercionWitness`** family (`BuiltinTypeCoercionWitness`, `DeclRefTypeCoercionWitness`) — these underlie implicit-cast insertion. The mechanics of implicit casting are owned by the `pipeline/03-semantic-check` bundle; the witness identities have no distinct surface observation. | Not reachable via any allowed test directive. |
