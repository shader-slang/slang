---
generated: true
model: claude-opus-4-8
generated_at: 2026-07-02T00:00:00+00:00
source_commit: 8e2a63cbf22c85332796702fd02a875466486a23
watched_paths_digest: ad6660d3f4a1bc7aff674633e08a1a323b5dcee5172803c940c5f796f8cde066
source_doc: docs/language-reference/generics.md
source_doc_digest: b0c5e1d5ca87b9ca5bb42dc8ae9e6903730d17bbdfe4e2d476ec8c4508dd3533
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/generics

## Intent

Tests verify generics claims in the **language reference** at
[`docs/language-reference/generics.md`](../../../../language-reference/generics.md).
Coverage strategy: enumerate every normative claim across the Syntax, Description, Type
Checking, Parameter Binding, Examples, Type-Parameter-Packs, and Pack-Queries sections, then
write one test per claim along the dimensions the doc commits to (basic, boundary, negative,
types, back-ends). Functional value observations use `COMPARE_COMPUTE -cpu`; target-dependent
emission is fanned out across `hlsl / spirv-asm / glsl / metal / wgsl / cpp` `SIMPLE`
directives; negative claims use `DIAGNOSTIC_TEST` pinned to `E39999`. Pack features are
exercised through the same `-cpu` value path (which specializes and monomorphizes them). Two
doc surfaces are unimplemented by the compiler and are recorded under Doc gaps rather than
shipped as failing tests: the `where countof(P) == expr` pack-count grammar (rejected at parse
with `E20001`) and the `__first`/`__last` partial pack queries (SIGSEGV).

## Claims

### C-Syntax: grammar productions

- **C1**: A generic parameter list `<...>` may be attached to a struct, interface, typealias, function, constructor, or subscript operator.
- **C2**: A generic value parameter can be declared with `let N : type` syntax.
- **C3**: A generic value parameter can be declared with the traditional `type N` syntax.
- **C4**: A generic value parameter pack can be declared with `let each N [: type]` syntax.
- **C5**: A generic type parameter can be declared with `[typename] T [: Constraint] [= Default]` syntax.
- **C6**: A generic type parameter pack can be declared with `each T [: Constraint]` syntax.
- **C7**: A `where` clause can add a conformance constraint (`where T : U`) or an equality constraint (`where T == U`).
- **C7b**: A `where countof(P) == IntExpr` pack-count constraint requires the named pack to have exactly that compile-time count.

### C-Desc: parameter kinds and constraints

- **C8**: A generic value parameter's type may be Boolean, integer, or enumeration.
- **C9**: A generic type parameter pack is a variable-length list of types, bindable to different lengths.
- **C10**: A constraint on a type parameter pack applies to every type in the pack.
- **C11**: Value parameters that are not packs cannot be constrained; only their declared type applies.
- **C12**: A conformance/equality constraint may be `optional`; then `T is U` is true when T conforms/equals U, and inside `if (T is U)` a value of T may be used as U.
- **C13**: A generic value parameter may have a default value via `= init-expr`.
- **C14**: A generic type parameter may have a default type via `= simple-type-spec`.
- **C15**: Type/value parameter packs may be constrained with `where nonempty(P)`, requiring the pack non-empty at specialization time.

### C-TypeCheck: checking before specialization

- **C16**: Type checking of a generic body happens before specialization: an operation is legal only if legal for all conforming concrete types.
- **C17**: A type equality constraint `T == U` causes T to be treated as U for all purposes.
- **C18**: A type conformance constraint `T : U` causes T to be considered to implement all requirements of U.
- **C19**: No assumptions are made about a generic value parameter beyond its declared type.
- **C20**: An associated-type equality constraint `where T.AssocType == int` requires T's associated type to be exactly int.

### C-Bind: parameter binding

- **C21**: Arguments can be bound explicitly with angle-bracket syntax after the identifier.
- **C22**: Arguments can be inferred from call-site argument expressions (implicit binding).
- **C23**: Mixed binding is allowed: leftmost parameters explicit, the rest inferred.
- **C24**: Ambiguous inference over fundamental scalar/same-length-vector types resolves to the highest-promotion-rank element type (int8_t … float … double).
- **C25**: When inference yields multiple options for a generic value argument it is an error.
- **C26**: When inference is ambiguous for a non-fundamental type it is an error; explicit binding resolves it.

### C-Ex: worked examples (normative illustrations)

- **C27**: A generic struct with type and value parameters can be specialized and its array field accessed per element.
- **C28**: A generic type alias can partially bind a generic type, fixing one parameter and leaving the rest open.
- **C29**: A generic function taking an `IFunc<T,...>` parameter can be called with a concrete struct implementing IFunc.
- **C30**: A generic constructor `__init<T : IInteger>` can initialize a struct from values of type T.
- **C31**: A generic subscript operator `__subscript<T>(T i) where T : IInteger` accepts any IInteger index for get and set.
- **C32**: A conformance constraint (`where T : IArithmetic`) enables arithmetic operations on T inside the body.
- **C33**: An optional conformance constraint with `if (T is IInteger)` selects behavior at specialization time.
- **C34**: A `where T.PropertyType == int` equality constraint lets the body use the associated type as int; a conformer whose PropertyType is float is rejected.
- **C35**: Two generic extensions with different type constraints (IFloat vs IInteger) each contribute their method only for the matching specialization.
- **C36**: A type parameter pack with `expand each` expands into a per-element call sequence.
- **C37**: Multiple function parameter packs of equal length expand pairwise (each-expressions iterate in lockstep).
- **C38**: Explicit binding of the un-inferable return-type parameter (`diffSingle<int32_t>(a, b)`) pins it while the rest are inferred.
- **C39**: Both a type parameter and a value parameter can be inferred together from a fixed-size array argument.
- **C40**: Explicit binding `testFunc<IBase>(a, b)` resolves an otherwise-ambiguous non-fundamental type inference.

### C-Pack: pack queries

- **C41**: `__first(P)` and `__last(P)` are partial operations requiring P known non-empty (via `where nonempty(P)`).
- **C42**: `__trimFirst(P)` and `__trimLast(P)` are total operations, yielding an empty pack on an empty pack.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| ----- | ------ | ------ | ----- |
| C1: A generic interface declared with `<...>` parameters can be conformed to at a concrete argument and its requirement called. | functional | [#syntax](../../../../language-reference/generics.md#syntax) | [`generic-interface-functional.slang`](generic-interface-functional.slang) |
| C3: A generic value parameter can be declared with the traditional `type name` syntax (generic-value-param-trad-decl) instead of `let name : type`. | functional | [#syntax](../../../../language-reference/generics.md#syntax) | [`value-param-trad-syntax-functional.slang`](value-param-trad-syntax-functional.slang) |
| C8: A generic value parameter may be of Boolean type and drive compile-time branch selection. | functional | [#description](../../../../language-reference/generics.md#description) | [`value-param-bool-functional.slang`](value-param-bool-functional.slang) |
| C13: A generic value parameter with a default value (`let N : int = 3`) can be specialized without an explicit value argument. | functional | [#description](../../../../language-reference/generics.md#description) | [`default-value-arg-functional.slang`](default-value-arg-functional.slang) |
| C14: A generic type parameter with a default type (`T = int`) can be specialized without an explicit type argument. | functional | [#description](../../../../language-reference/generics.md#description) | [`default-type-arg-functional.slang`](default-type-arg-functional.slang) |
| C16: A generic body is type-checked before specialization using only the declared constraints, so an operation valid for all conforming types compiles once and works for every specialization. | functional | [#type-checking](../../../../language-reference/generics.md#type-checking) | [`type-checking-before-specialization-functional.slang`](type-checking-before-specialization-functional.slang) |
| C17: A type equality constraint `where T == int` causes T to be treated as int for all purposes inside the generic body. | functional | [#type-checking](../../../../language-reference/generics.md#type-checking) | [`type-equality-alias-functional.slang`](type-equality-alias-functional.slang) |
| C18: A `where T : IArithmetic` conformance constraint lets the generic body call the interface operations (here `+`) on values of type T. | functional | [#type-checking](../../../../language-reference/generics.md#type-checking) | [`type-conformance-constraint-functional.slang`](type-conformance-constraint-functional.slang) |
| C19: A generic value parameter is usable only as its declared compile-time constant (e.g. an array bound / loop bound); no other assumption is made about it. | functional | [#type-checking](../../../../language-reference/generics.md#type-checking) | [`value-param-as-array-size-functional.slang`](value-param-as-array-size-functional.slang) |
| C20/C34: A `where T.PropertyType == int` equality constraint lets the body treat the associated type as int and add the property values. | functional | [#type-checking](../../../../language-reference/generics.md#type-checking) | [`type-equality-assoc-constraint-functional.slang`](type-equality-assoc-constraint-functional.slang) |
| C20/C34: A call violating an associated-type equality constraint (`where T.PropertyType == int` applied to a conformer whose PropertyType is float) is rejected. | negative | [#examples](../../../../language-reference/generics.md#examples) | [`type-equality-constraint-negative.slang`](type-equality-constraint-negative.slang) |
| C22: A generic function's type parameter is inferred implicitly from the call-site argument expression type. | functional | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | [`generic-function-basic-functional.slang`](generic-function-basic-functional.slang) |
| C22: A generic function with two independent type parameters infers each one separately from its own argument. | functional | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | [`multiple-type-params-functional.slang`](multiple-type-params-functional.slang) |
| C23/C38: Mixed binding: the leftmost generic parameter is bound explicitly and the remaining parameters are inferred from arguments. | functional | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | [`explicit-and-implicit-binding-functional.slang`](explicit-and-implicit-binding-functional.slang) |
| C24: When inference is ambiguous across fundamental scalar types, the element type with the highest promotion rank (int32_t over int8_t) is chosen. | functional | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | [`inference-promotion-rank-functional.slang`](inference-promotion-rank-functional.slang) |
| C25: When inference yields conflicting values for one generic value parameter (two arrays of different lengths bound to the same `let N`), specialization fails with an error. | negative | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | [`ambiguous-value-arg-negative.slang`](ambiguous-value-arg-negative.slang) |
| C26: When inference for a generic type parameter is ambiguous and the candidates are non-fundamental types, specialization fails with an error. | negative | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | [`ambiguous-type-arg-negative.slang`](ambiguous-type-arg-negative.slang) |
| C27: A generic struct with a type parameter and a value parameter can be specialized and its array field accessed per element. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`generic-struct-type-value-param-functional.slang`](generic-struct-type-value-param-functional.slang) |
| C27: A specialized generic struct emits as a concrete monomorphized entry point on every text-emit target. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`generic-struct-emission.slang`](generic-struct-emission.slang) |
| C28: A generic type alias partially binds a generic type, fixing one parameter and leaving the rest open. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`generic-typealias-partial-functional.slang`](generic-typealias-partial-functional.slang) |
| C29: A generic function taking an `IFunc<T,T,T>` parameter can be called with a concrete struct implementing IFunc. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`generic-func-as-param-functional.slang`](generic-func-as-param-functional.slang) |
| C30: A generic constructor `__init<T : IInteger>` initializes a struct from two values of type T. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`generic-constructor-functional.slang`](generic-constructor-functional.slang) |
| C31: A generic subscript operator `__subscript<T>(T i) where T : IInteger` accepts any IInteger index type for get and set. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`generic-subscript-functional.slang`](generic-subscript-functional.slang) |
| C32: A constrained generic function specialized to a concrete type emits monomorphized target code on every text-emit back-end. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`generic-function-emission.slang`](generic-function-emission.slang) |
| C33/C12: An `optional` conformance constraint enables `if (T is IInteger)` to select behavior at specialization time, using T as IInteger in the then-branch. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`optional-constraint-functional.slang`](optional-constraint-functional.slang) |
| C35: Two generic extensions on the same struct with different type constraints each contribute their method only for the matching specialization. | functional | [#examples](../../../../language-reference/generics.md#examples) | [`generic-extension-functional.slang`](generic-extension-functional.slang) |
| C36: A type parameter pack expanded with `expand ... each ...` produces one call per pack element, in order. | functional, boundary, stress | [#type-param-packs](../../../../language-reference/generics.md#type-param-packs) | [`type-param-pack-expand-functional.slang`](type-param-pack-expand-functional.slang), [`pack-single-element-boundary.slang`](pack-single-element-boundary.slang), [`pack-empty-expand-boundary.slang`](pack-empty-expand-boundary.slang), [`pack-large-expand-stress.slang`](pack-large-expand-stress.slang), [`pack-expand-emission.slang`](pack-expand-emission.slang) |
| C37: Two function parameter packs of equal length expand pairwise, iterating each-expressions in lockstep. | functional | [#type-param-packs](../../../../language-reference/generics.md#type-param-packs) | [`multi-pack-expand-functional.slang`](multi-pack-expand-functional.slang) |
| C39: Both a type parameter and a value parameter are inferred implicitly from a fixed-size array argument. | functional | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | [`implicit-type-and-value-binding-functional.slang`](implicit-type-and-value-binding-functional.slang) |
| C40: Explicit binding of an otherwise-ambiguous non-fundamental type parameter (`testFunc<IBase>(a, b)`) resolves the ambiguity and compiles. | functional | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | [`explicit-binding-resolves-ambiguous-functional.slang`](explicit-binding-resolves-ambiguous-functional.slang) |
| C15: A `where nonempty(T)` constraint is accepted and a function using it compiles and runs when called with a non-empty pack. | functional | [#type-param-packs](../../../../language-reference/generics.md#type-param-packs) | [`pack-nonempty-constraint-functional.slang`](pack-nonempty-constraint-functional.slang) |
| C15: A `where nonempty(T)` constraint rejects specialization when the pack is bound to zero arguments. | negative | [#type-param-packs](../../../../language-reference/generics.md#type-param-packs) | [`pack-nonempty-empty-negative.slang`](pack-nonempty-empty-negative.slang) |
| C42: `__trimFirst(P)` is a total pack-query operation that drops the first element and yields the remaining pack. | functional | [#pack-queries](../../../../language-reference/generics.md#pack-queries) | [`pack-trim-first-total-functional.slang`](pack-trim-first-total-functional.slang) |
| C42: `__trimLast(P)` is a total pack-query operation that drops the last element and yields the remaining pack. | functional | [#pack-queries](../../../../language-reference/generics.md#pack-queries) | [`pack-trim-last-total-functional.slang`](pack-trim-last-total-functional.slang) |
| C32: A call whose argument type does not satisfy the generic's conformance constraint (`where T : IArithmetic`) is rejected at the call site. | negative | [#type-checking](../../../../language-reference/generics.md#type-checking) | [`constraint-not-satisfied-negative.slang`](constraint-not-satisfied-negative.slang) |

## Untested claims

| Claim | Reason | Anchor | Why untested |
| ----- | ------ | ------ | ------------ |
| C2: A generic value parameter can be declared with `let N : type` syntax. | out-of-bundle | [#syntax](../../../../language-reference/generics.md#syntax) | Exercised implicitly by every value-parameter test that uses `let N : uint` (e.g. `generic-struct-type-value-param-functional.slang`, `value-param-as-array-size-functional.slang`); no separate probe distinguishes it from those. |
| C4: A generic value parameter pack can be declared with `let each N` syntax. | out-of-bundle | [#syntax](../../../../language-reference/generics.md#syntax) | Value-pack semantics coincide with type-pack expansion, which is covered by the `expand each` tests (C36/C37). The doc gives no value-pack-specific observable distinct from those. |
| C5: A generic type parameter can be declared with `[typename] T [: Constraint] [= Default]` syntax. | out-of-bundle | [#syntax](../../../../language-reference/generics.md#syntax) | The constraint and default sub-forms are covered by C18/C32 (constraint) and C14 (`default-type-arg-functional.slang`); the bare `T` form is covered by every generic-function/struct test. |
| C6: A generic type parameter pack can be declared with `each T [: Constraint]` syntax. | out-of-bundle | [#type-param-packs](../../../../language-reference/generics.md#type-param-packs) | The `each T` declaration form is exercised by all C36/C37 pack tests; the pack-constraint sub-form (`each T : Constraint`) has no doc-named observable distinct from `where T == int`. |
| C7b: A `where countof(P) == IntExpr` pack-count constraint requires the named pack to have exactly that compile-time count. | compile-time-toggle | [#pack-count-constraints](../../../../language-reference/generics.md#pack-count-constraints) | The documented `countof(...) ==` grammar is rejected by the parser (`E20001: unexpected '=='`), so no valid shader can express it. Recorded under Doc gaps as a `drift-from-source`; a test can be written once the parser accepts the production. |
| C9: A generic type parameter pack is a variable-length list of types. | out-of-bundle | [#description](../../../../language-reference/generics.md#description) | The variable-length property is exercised by the pack length axis (`pack-empty-expand-boundary`, `pack-single-element-boundary`, `pack-large-expand-stress`) under C36. |
| C10: A constraint on a type parameter pack applies to every type in the pack. | out-of-bundle | [#description](../../../../language-reference/generics.md#description) | The `where T == int` constraint on `each T` (applied to every element) is what makes every pack test's `expand each T` legal; no standalone observable separates it from those tests. |
| C11: Value parameters that are not packs cannot be constrained. | (unclassified) | [#description](../../../../language-reference/generics.md#description) | This is a stated restriction on declaration syntax; the doc names no diagnostic and no rejected surface, so a test would have to guess the error shape. Recorded under Doc gaps (ambiguous-claim). |
| C21: Arguments can be bound explicitly with angle-bracket syntax. | out-of-bundle | [#parameter-binding](../../../../language-reference/generics.md#parameter-binding) | Explicit binding is directly exercised by C38 (`explicit-and-implicit-binding-functional.slang`, `diffSingle<int32_t>`) and C40 (`testFunc<IBase>`); no additional standalone probe adds coverage. |
| C41: `__first(P)` and `__last(P)` are partial operations requiring P known non-empty. | (unclassified) | [#pack-queries](../../../../language-reference/generics.md#pack-queries) | Calling `__first(args)` / `__last(args)` on a pack parameter crashes slangc with SIGSEGV (exit 139) even with `where nonempty(T)` declared, on every back-end and via slangc codegen. Finding: `docs/generated/tests/_meta/findings/pack-query-first-last-compiler-crash.yaml` (a fix would move this claim to Functional coverage). Once fixed, add an INTERPRET/`-cpu` test asserting `__first(42, 7) == 42`. |
| Type coercion constraint `where U(T)` in generic extensions. | out-of-bundle | [#examples](../../../../language-reference/generics.md#examples) | The doc restricts the coercion constraint to generic extensions; covered by `conformance/types-extension`. |
| Generic enumeration declarations. | deprecated | [#syntax](../../../../language-reference/generics.md#syntax) | The doc's parser remark states generic enumerations "are not currently useful" (issue #10078); no observable behavior to test. |
| Remark: the `__generic` modifier as an alternative to `generic-params-decl`. | implementation-detail | [#description](../../../../language-reference/generics.md#description) | The doc recommends `generic-params-decl`; `__generic` is an alternative spelling with no user-observable behavioral difference. |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| ------ | ---- | --- | ------------------ |
| [#pack-count-constraints](../../../../language-reference/generics.md#pack-count-constraints) | drift-from-source | The doc devotes a full "Pack count constraints" section with normative examples such as `void foo<let N : int, each T>() where countof(T) == N {}`, but the compiler rejects the `countof(...) ==` production at parse time with `error[E20001]: unexpected '=='`. Every example in the section (including the nested `where countof(TIndex) == countof(D)` form) fails to parse, so the feature is not usable as documented. | State that pack-count (`where countof(P) == IntExpr`) constraints are not yet accepted by the parser and mark the section as a forward-looking / proposed feature, or gate the examples behind a note until the grammar is implemented. |
| [#pack-queries](../../../../language-reference/generics.md#pack-queries) | undocumented-behavior | The doc presents `__first(P)` / `__last(P)` as ordinary partial operations that become "well-formed" under `where nonempty(P)`, with no caveat. In practice, using either in a function with `where nonempty(T)` declared crashes the compiler with a SIGSEGV (exit 139) at codegen. The `__trimFirst` / `__trimLast` total operations do work. | Add a note that `__first` / `__last` are currently experimental / unimplemented for generic pack parameters and may crash the compiler, or remove them from the reference until the codegen path is stable. |
| [#description](../../../../language-reference/generics.md#description) | ambiguous-claim | The doc states "Value parameters that are not packs cannot be constrained" but names no diagnostic code and no example of the rejected form, so a conformance test cannot anchor to a specific error. It is unclear whether the constraint is rejected at parse, at check, or silently ignored. | Add a one-line example of the rejected syntax (e.g. `func f<let N : int where N : ISomething>()`) together with the exact diagnostic code the compiler emits, so the restriction is testable. |
