# Prompt: docs/generated/tests/regression/ast-reference/values/

See [`_common.md`](_common.md) for universal rules. See
[`ast-reference-types.md`](ast-reference-types.md) for the sibling
structural template; the same translation rule (claims to
observations) applies.

## Target

Produce the test bundle at `docs/generated/tests/regression/ast-reference/values/`,
anchored to
[`docs/generated/design/ast-reference/values.md`](../../../design/ast-reference/values.md).

Audience: nightly CI. The bundle exercises the **non-Type** `Val`
subclasses in the Slang AST — the `IntVal` family (`ConstantIntVal`,
`DeclRefIntVal`, `PolynomialIntVal`, `CountOfIntVal`, `SizeOfIntVal`,
`AlignOfIntVal`, `ErrorIntVal`), the `DeclRefBase` family
(`DirectDeclRef`, `MemberDeclRef`, `LookupDeclRef`,
`GenericAppDeclRef`), the `Witness` family (`DeclaredSubtypeWitness`,
`TransitiveSubtypeWitness`, `TypeEqualityWitness`,
`TypeCoercionWitness`, ...), and the `ModifierVal` family
(`UNormModifierVal`, `SNormModifierVal`, `NoDiffModifierVal`) —
through their **user-observable consequences** at type-resolution
and type-mismatch-diagnostic time.

## The translation rule (carried from `ast-reference-types.md`)

`values.md` is a per-class catalog of non-Type `Val`s. Slang does
not expose its AST. So a claim such as "two textually identical
`ConstantIntVal`s share the same `Val*`" is testable only via the
observable consequence: two specializations of the same generic
with the same argument are interchangeable; two array types whose
canonicalised index polynomials agree are the same type; an
interface-method call dispatched through a generic constraint is
permitted because a `DeclaredSubtypeWitness` exists.

- **Testable** ⇔ "if the doc's claim about this `Val` class were
  false, the program-text behavior we wrote would change in a way
  `slangc` reports."
- **Not testable through slangc** ⇔ "the claim is about hash-cons
  pointer identity, the index of an operand slot, the abstract
  intermediate's existence in the C++ hierarchy, or the fact that a
  zero-operand `Val` is one-per-`ASTBuilder`."

### Observable claims (write tests for these)

Pick one representative claim per concrete user-spellable Val:

- **`ConstantIntVal`** — a literal integer flows into an array
  bound; the array carries that many elements.
- **`DeclRefIntVal`** — a generic value parameter `let N : int` is
  unsubstituted until generic application; after substitution the
  bound is the concrete integer.
- **`PolynomialIntVal`** — `T[2*N+3]` and `T[3+2*N]` are the same
  type (canonicalization makes them interchangeable in a generic
  body).
- **`CountOfIntVal`** — `countof(T)` of a concrete type pack /
  tuple folds to a constant integer.
- **`SizeOfIntVal`** — `sizeof(int)` is a positive compile-time
  integer (the exact value is target-dependent; assert only
  positivity).
- **`AlignOfIntVal`** — analogous to `SizeOfIntVal`; assert
  positivity.
- **`ErrorIntVal`** — when an array bound contains an undeclared
  identifier, the checker emits the root-cause diagnostic and does
  not flood the user with cascading integer-value errors.
- **`GenericAppDeclRef`** (positive) — same generic arguments yield
  interchangeable types (the surface form of hash-cons identity).
- **`GenericAppDeclRef`** (negative) — different generic arguments
  yield distinct types; cross-assignment is rejected.
- **`DeclaredSubtypeWitness`** — a generic constrained by `T : I`
  can dispatch `I`'s methods on a `T` value because of the witness
  produced by the `InheritanceDecl`.
- **`TransitiveSubtypeWitness`** — a derived conformer satisfies a
  base-interface bound via the composed witness.
- **`UNormModifierVal`** — `unorm` on a `Texture2D<...>` element
  type survives lowering and appears in HLSL emit.

### Internal-only claims (record under `## Untested claims`)

The doc carries many claims about the **internal shape** of these
`Val`s. These are unobservable:

- **Hash-cons pointer identity** (the
  [`### Hash-consing and the ASTBuilder`](../../../design/ast-reference/values.md#hash-consing-and-the-astbuilder)
  invariant). Surface tests see behavioral equivalence, not `Val*`
  equality.
- **Operand-list layout** described under "Operand semantics" in
  `## Nodes`. The `m_operands` storage is not visible.
- **Abstract intermediates**: `IntVal`, `SizeOfLikeIntVal`,
  `ShapeTransformIntValPack`, `Witness`, `SubtypeWitness`,
  `TypeCoercionWitness`. No user spelling of their own.
- **C++ parent classes** of any concrete `Val`.
- **Singleton-ness** of `NoneWitness` / zero-operand `Val`s.
- **`DirectDeclRef` / `MemberDeclRef` / `LookupDeclRef`** —
  distinct internally, but at the type-checker surface a plain
  identifier and a qualified member ref look the same. Only
  `GenericAppDeclRef`'s presence of generic arguments distinguishes
  it.
- **`FuncCallIntVal`** — no surface spelling documented; record as
  a doc gap.
- **`WitnessLookupIntVal`** — no surface spelling documented;
  record as a doc gap.
- **`TypeCastIntVal`** — no surface-distinct spelling.
- **Polynomial helpers** (`PolynomialIntValFactor`,
  `PolynomialIntValTerm`) — internal operands of
  `PolynomialIntVal`; no distinct surface.
- **All pack / variadic `IntVal`s and `SubtypeWitness`es**:
  `FirstIntVal`, `LastIntVal`, `ConcreteIntValPack`,
  `TrimFirstIntValPack`, `TrimLastIntValPack`,
  `ShapeConcatIntValPack`, `ShapePermuteIntValPack`,
  `ShapeSwapIntValPack`, `ShapeReduceIntValPack`,
  `ExpandIntValPack`, `EachIntVal`, `TypePackSubtypeWitness`,
  `EachSubtypeWitness`, `FirstSubtypeWitness`, `LastSubtypeWitness`,
  `TrimFirstSubtypeWitness`, `TrimLastSubtypeWitness`,
  `PackBranchSubtypeWitness`, `ExpandSubtypeWitness`,
  `NonEmptyPackWitness`. Observable through pack-expression
  surface; owned by the variadic-generics / packs bundle.
- **Differentiation `Val`s** (`DifferentiateVal`,
  `ForwardDifferentiateVal`, `BackwardDifferentiateVal`,
  `BackwardDifferentiateIntermediateTypeVal`,
  `BackwardDifferentiatePrimalVal`,
  `BackwardDifferentiatePropagateVal`,
  `HasDiffTypeInfoWitness`, `DiffTypeInfoWitness`,
  `HigherOrderDiffTypeTranslationWitness`, `NoDiffModifierVal`) —
  observable through autodiff; owned by the autodiff bundle.
- **`ExtractExistentialSubtypeWitness`** — observable through the
  existential / `some IFoo` surface; owned by the existential
  feature bundle.
- **`DynamicSubtypeWitness`** — `DynamicType` is not user-spellable.
- **`TypeCoercionWitness`** family — underlies implicit-cast
  insertion; mechanics are owned by `pipeline/03-semantic-check`.
- **`TypeEqualityWitness`** — surface observation belongs to
  typedef / alias tests under `ast-reference/types` /
  `ast-reference/declarations`.
- **`SNormModifierVal`** — analogous to `UNormModifierVal`; one
  test (`unormmodifierval-resource-format.slang`) covers the
  modifier-survival observation.
- **`UIntSetVal`** — capability-system bitset; owned by the
  capability bundle.
- The **`## Family hierarchy`** mermaid diagram itself.

If you find yourself thinking "this would verify that two `Val*`
are pointer-equal" or "this would assert that operand i is the
function decl" — stop, that is a source-targeting probe.

## Avoid duplication with sibling bundles

This bundle is **non-Type-`Val`-centric**. Adjacent bundles cover
adjacent surfaces:

- `docs/generated/tests/regression/ast-reference/types/` — covers the **`Type`**
  side. `Type` _is_ a `Val`, but its concrete subclasses are
  exercised there. Do not retest type-class observations here.
- `docs/generated/tests/regression/ast-reference/declarations/` — covers the **`Decl`**
  side that `DeclRefBase` references. Do not retest decl mechanics
  here.
- `docs/generated/tests/regression/ast-reference/expressions/` — covers expressions
  that produce these `Val`s (e.g. `SizeOfExpr`, `CountOfExpr`,
  `IsTypeExpr`). Anchor here at the **`Val`** side of the
  observation, not at the expression that produced it. If two
  bundles seem to want the same test, the expression bundle wins
  if the surface is the expression; this bundle wins if the
  observation is "what the Val carries".
- `docs/generated/tests/regression/ast-reference/modifiers.md` — covers AST modifiers.
  The `ModifierVal` subhierarchy here is the deduplicated, `Val`
  representation of those modifiers. The `unorm` test here is the
  canonical "modifier survives to emit" observation; do not
  duplicate.
- `docs/generated/tests/regression/ir-reference/types/` — covers the IR-level
  observations. Do not re-test IR spellings here; this bundle
  stays at the AST / type-checker surface (interpreter values and
  diagnostics).
- `docs/generated/tests/regression/pipeline/03-semantic-check/` — covers
  type-coercion mechanics. The `TypeCoercionWitness` family's
  surface lives there.
- `docs/generated/tests/language-feature/generics-and-packs/` (future) —
  pack / variadic claims.
- `docs/generated/tests/language-feature/autodiff/` (future) — autodiff
  claims (Differentiate*, *Diff\*Witness, NoDiff).

## Allowed secondary doc citations

- `docs/generated/design/ast-reference/base.md`
- `docs/generated/design/ast-reference/types.md`
- `docs/generated/design/ast-reference/expressions.md`
- `docs/generated/design/ast-reference/declarations.md`
- `docs/generated/design/ast-reference/modifiers.md`
- `docs/generated/design/ir-reference/types.md` (cross-link only)
- `docs/generated/design/pipeline/03-semantic-check.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for verification only

- `source/slang/slang-ast-val.h`
- `source/slang/slang-ast-val.cpp`
- `source/slang/slang-ast-base.h`
- `source/slang/slang-ast-support-types.h`

You may look at these files to confirm the surface form that
produces a given `Val` class (e.g. that `let N : int` produces a
`DeclRefIntVal`, that `countof(T)` on a concrete pack folds to a
`ConstantIntVal`). You may **not** mine them for behavioral claims
that the doc does not make.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` for internal-shape claims and the
   pack-only / autodiff-only / capability-only `Val` classes.
2. **8–15** `.slang` test files (the doc is overwhelmingly internal;
   user-anchorable surface is smaller than for types / expressions /
   declarations). Drop after **3 attempts** per test and record the
   blocking issue under `## Doc gaps observed` or `## Untested claims`.

## Test directives

Most Val claims are target-independent (the `Val`'s representation
and its checker-attached behavior do not differ across backends):

- `//TEST:INTERPRET(filecheck=CHECK):` — **primary** directive.
  Use `printf` to observe the run-time value of an array slot, a
  generic-bound count, or an interface dispatch result.
- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — for negative claims:
  cross-instantiation type-mismatch, undeclared-name in array bound
  (ErrorIntVal cascade-suppression).
- Multi-target `//TEST:SIMPLE` only when the observable surface is
  the emitted text — e.g. `UNormModifierVal` whose preserved
  `unorm` keyword appears in HLSL output. One such test per bundle
  is enough.

## Cast and observation reminders (carried from `_common.md`)

- Use **constructor-style casts** (`int('A')`), not C-style
  (`(int)'A'`).
- `slangi` `printf` does **not** support `%s`. For string-typed
  observation, use `//TEST:SIMPLE(filecheck=CHECK):-target cpp`.
- `slangi` **cannot host `cbuffer` or non-const `static` module
  globals**. For `ModifierVal` claims that need a resource, switch
  to `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`.
- `slangi` **cannot `countof(local-array)`** — the bytecode
  generator currently errors with `unimplemented: VM bytecode gen
for inst.` for that path. Use `countof(T)` of a tuple/pack
  inside a generic, or use `-target hlsl` for the
  observation.
- The runner's "Suggested annotations" output is the source of
  truth for diagnostic caret positions.
- For `DIAGNOSTIC_TEST`, the `non-exhaustive` flag goes **inside**
  the parens: `(diag=CHECK,non-exhaustive)`. Omit it when all
  diagnostics are matched.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ast-reference/values.md` (or a listed secondary doc).
- [ ] The bundle covers the user-spellable user-observable `Val`
      families: `ConstantIntVal`, `DeclRefIntVal`,
      `PolynomialIntVal`, `CountOfIntVal`, `SizeOfIntVal`,
      `ErrorIntVal`, `GenericAppDeclRef` (positive + negative),
      `DeclaredSubtypeWitness`, `TransitiveSubtypeWitness`,
      `UNormModifierVal`.
- [ ] At least one negative / diagnostic test
      (`genericappdeclref-different-args-distinct`,
      `errorintval-suppresses-cascade`).
- [ ] No test asserts hash-cons pointer identity, an operand-slot
      index, an abstract intermediate's existence, or the C++
      parent class of any `Val`.
- [ ] No test depends on a GPU. INTERPRET, diagnostic-only, and at
      most one HLSL-emit `SIMPLE` carry the bundle.
- [ ] No test was written by inspecting an uncovered source line.
      If you find yourself thinking "this would cover the branch at
      `slang-ast-val.h:NNNN`", stop and re-read the doc.
- [ ] `README.md` `## Untested claims` lists the internal-shape and
      pack / autodiff / capability / existential `Val` classes that
      this bundle does not exercise, with one-line reasons.
- [ ] `README.md` `## Doc gaps observed` is honest. If a doc claim
      lacks a surface spelling, record it as a gap rather than
      writing a source-targeting probe.
