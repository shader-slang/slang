---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T00:00:00+00:00
source_commit: 690f6a3084801386b77186394e0f6e8c120824a4
watched_paths_digest: 05b1016228f8c0bbf2fd6e6ea6d165c4d173a37c116aea0c0eccdb47c10abf97
source_doc: docs/llm-generated/ast-reference/types.md
source_doc_digest: 7007cb8ec84defc795a9f755431fcd600ec3df44f2f392d04896c57ec9ce17b9
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ast-reference/types

## Intent

Tests verify the documented user-observable roles of the concrete
`Type` subclasses enumerated in
[`docs/llm-generated/ast-reference/types.md`](../../../docs/llm-generated/ast-reference/types.md):
the general-purpose pseudotypes (`ErrorType`, `OverloadGroupType`,
`InitializerListType`), the `DeclRefType` family and its arithmetic
descendants (`BasicExpressionType`, `VectorExpressionType`,
`MatrixExpressionType`), the aggregate / collection family
(`ArrayExpressionType`, `TupleType`, `OptionalType`), the
pointer / parameter-passing types (`PtrType`, `BorrowInParamType`,
`OutParamType`, `BorrowInOutParamType`), the resource / texture /
sampler family (`TextureType`, `SamplerStateType`), the buffer family
(`HLSLStructuredBufferType`, `HLSLRWStructuredBufferType`,
`HLSLByteAddressBufferType`, `HLSLRWByteAddressBufferType`), the
parameter-group / constant-buffer family (`ConstantBufferType`,
`ParameterBlockType`), the `FuncType` leaf, the `AndType` conjunction,
the `ThisType` of an interface, the `ModifiedType` (e.g. `const T`),
the `NullPtrType` / `NoneType` singleton literals, and the
`StringType`.

The doc is fundamentally about the **internal AST shape** of these
type classes; the user-observable surface is what type-resolution
(positive) or type-mismatch diagnostics (negative) make visible. Each
test picks one type class, writes the smallest Slang program that
exercises its documented role, and verifies the documented behavior
through an interpreter print or a diagnostic match. INTERPRET is the
primary directive; one multi-target SIMPLE test
(`rwstructuredbuffertype-multi-target.slang`) is included as the
canonical per-target lowering observation for the buffer family.

Internal AST shape claims (which C++ class a checker allocated for a
type, which operand index holds the element type, whether two
identical types share the same `Val*`, the abstract intermediates in
the family hierarchy, synthesized-only types, GPU-only / autodiff-only
type families, pack / variadic types, data-layout marker types) are
recorded under `## Out of scope` because they are unobservable through
any allowed `slang-test` directive.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                              | Claim (one line)                                                                                                                  | Tests                                                                                  |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| C-01     | [#declreftype](../../../docs/llm-generated/ast-reference/types.md#declreftype)                                                                                      | A struct name is a `DeclRefType` referencing the StructDecl; field access flows through the decl-ref.                             | `declreftype-struct-name.slang`                                                        |
| C-02     | [#declreftype](../../../docs/llm-generated/ast-reference/types.md#declreftype)                                                                                      | A `DeclRefType` carries generic substitutions; two specializations of the same generic decl carry independent storage.            | `declreftype-generic-substitution.slang`                                               |
| C-03     | [#declreftype](../../../docs/llm-generated/ast-reference/types.md#declreftype)                                                                                      | Two `DeclRefType`s with different substitutions of the same decl are distinct types; one cannot be assigned to the other.         | `declreftype-substitution-distinguishes.slang`                                         |
| C-04     | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | Each `BasicExpressionType` (int / uint / float / bool) carries the value of its scalar built-in.                                  | `basictype-scalars.slang`                                                              |
| C-05     | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | A `VectorExpressionType` is parameterised by element-type and count; each component is independently readable.                    | `vectortype-components.slang`                                                          |
| C-06     | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | A `MatrixExpressionType` is parameterised by element-type, row count, and column count; element access via `[r][c]` works.        | `matrixtype-row-col.slang`                                                             |
| C-07     | [#aggregate-collection-types](../../../docs/llm-generated/ast-reference/types.md#aggregate-collection-types)                                                        | An `ArrayExpressionType` carries an element-type and an element-count; each element is independently addressable.                 | `arraytype-sized-elements.slang`                                                       |
| C-08     | [#aggregate-collection-types](../../../docs/llm-generated/ast-reference/types.md#aggregate-collection-types)                                                        | A `TupleType` holds a heterogeneous member-type list; the i-th element is reachable as `._i` with the i-th declared type.         | `tupletype-named-fields.slang`                                                         |
| C-09     | [#aggregate-collection-types](../../../docs/llm-generated/ast-reference/types.md#aggregate-collection-types)                                                        | An `OptionalType<T>` distinguishes a wrapped value from no-value; `.hasValue` and `.value` expose the state.                      | `optionaltype-value-hasvalue.slang`                                                    |
| C-10     | [#pointer-reference-parameter-passing-types](../../../docs/llm-generated/ast-reference/types.md#pointer-reference-parameter-passing-types)                          | `NullPtrType` and `NoneType` are the types of `nullptr` and `none`; the literals match their typed targets at comparison.         | `nonetype-and-nullptrtype.slang`                                                       |
| C-11     | [#pointer-reference-parameter-passing-types](../../../docs/llm-generated/ast-reference/types.md#pointer-reference-parameter-passing-types)                          | A `PtrType<T>` (spelled `T*`) is a raw pointer; dereferencing writes pointed-to storage.                                          | `ptrtype-dereference.slang`                                                            |
| C-12     | [#pointer-reference-parameter-passing-types](../../../docs/llm-generated/ast-reference/types.md#pointer-reference-parameter-passing-types)                          | `BorrowInParamType` / `OutParamType` / `BorrowInOutParamType` distinguish parameter-passing modes; only out / inout propagate.    | `paramtype-out-inout-in.slang`                                                         |
| C-13     | [#functype](../../../docs/llm-generated/ast-reference/types.md#functype)                                                                                            | A `FuncType` holds parameter types and a result type; a function-typed value is invokable.                                        | `functype-callable.slang`                                                              |
| C-14     | [#andtype](../../../docs/llm-generated/ast-reference/types.md#andtype)                                                                                              | An `AndType` `IA & IB` requires conformance to both interfaces; a type that conforms to both has both methods dispatched.         | `andtype-conjunction-dispatch.slang`                                                   |
| C-15     | [#andtype](../../../docs/llm-generated/ast-reference/types.md#andtype)                                                                                              | An `AndType` `IA & IB` rejects a type that does not conform to all of its interface conjuncts.                                    | `andtype-missing-conformance-rejected.slang`                                           |
| C-16     | [#thistype-and-assoctypedecl](../../../docs/llm-generated/ast-reference/types.md#thistype-and-assoctypedecl)                                                        | A `ThisType` inside an interface stands for the conforming type; a `This`-returning method returns the concrete type.             | `thistype-interface-method.slang`                                                      |
| C-17     | [#existentials-and-this-types](../../../docs/llm-generated/ast-reference/types.md#existentials-and-this-types)                                                      | A `ModifiedType` `const T` removes l-value-ness; assigning to a const-modified local is rejected.                                 | `modifiedtype-const-rejects-assign.slang`                                              |
| C-18     | [#errortype-and-bottomtype](../../../docs/llm-generated/ast-reference/types.md#errortype-and-bottomtype)                                                            | `ErrorType` is the type of expressions that failed checking; downstream uses do not produce cascading type-mismatch diagnostics.  | `errortype-cascading-suppression.slang`                                                |
| C-19     | [#general-purpose-types](../../../docs/llm-generated/ast-reference/types.md#general-purpose-types)                                                                  | `InitializerListType` is the pseudo-type of a brace-initializer before matching; the same `{a, b}` collapses into the target.     | `initializerlisttype-collapse-to-target.slang`                                         |
| C-20     | [#general-purpose-types](../../../docs/llm-generated/ast-reference/types.md#general-purpose-types)                                                                  | `OverloadGroupType` is the pseudo-type of an unresolved overload set; ambiguous references in non-callable context are rejected.  | `overloadgrouptype-ambiguous-rejected.slang`                                           |
| C-21     | [#string-dynamic-misc](../../../docs/llm-generated/ast-reference/types.md#string-dynamic-misc)                                                                      | `StringType` is the Slang `String` type; a string-typed literal preserves its text in emitted target code.                        | `stringtype-literal-emitted.slang`                                                     |
| C-22     | [#parameter-group-constant-buffer-types](../../../docs/llm-generated/ast-reference/types.md#parameter-group-constant-buffer-types)                                  | A `ConstantBufferType<T>` uniform lowers to HLSL `cbuffer` at the emit stage.                                                     | `constantbuffertype-hlsl-emit.slang`                                                   |
| C-23     | [#parameter-group-constant-buffer-types](../../../docs/llm-generated/ast-reference/types.md#parameter-group-constant-buffer-types)                                  | A `ParameterBlockType<T>` uniform lowers through parameter-group machinery; HLSL emits T's fields as a cbuffer block.             | `parameterblocktype-hlsl-emit.slang`                                                   |
| C-24     | [#buffer-types](../../../docs/llm-generated/ast-reference/types.md#buffer-types)                                                                                    | An `HLSLRWStructuredBufferType` uniform lowers to every text-emit backend; per-target spellings differ (RWStructuredBuffer / buffer / global). | `rwstructuredbuffertype-multi-target.slang`                                            |
| C-25     | [#buffer-types](../../../docs/llm-generated/ast-reference/types.md#buffer-types)                                                                                    | `HLSLStructuredBufferType` (read-only) and `HLSLRWStructuredBufferType` (RW) are distinct buffer-family classes.                  | `structuredbuffer-readonly-hlsl-emit.slang`                                            |
| C-26     | [#buffer-types](../../../docs/llm-generated/ast-reference/types.md#buffer-types)                                                                                    | `HLSLByteAddressBufferType` / `HLSLRWByteAddressBufferType` lower to the matching HLSL surface form.                              | `byteaddressbuffertype-hlsl-emit.slang`                                                |
| C-27     | [#resource-and-texture-type-families](../../../docs/llm-generated/ast-reference/types.md#resource-and-texture-type-families)                                        | `TextureType` and `SamplerStateType` are first-class types; HLSL emits them as `Texture2D<T>` and `SamplerState`.                 | `texturetype-samplerstate-hlsl-emit.slang`                                             |

## Tests in this bundle

| File                                              | Intent     | Doc anchor                                                                |
| ------------------------------------------------- | ---------- | ------------------------------------------------------------------------- |
| `declreftype-struct-name.slang`                   | functional | `#declreftype`                                                            |
| `declreftype-generic-substitution.slang`          | functional | `#declreftype`                                                            |
| `declreftype-substitution-distinguishes.slang`    | negative   | `#declreftype`                                                            |
| `basictype-scalars.slang`                         | functional | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vectortype-components.slang`                     | functional | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `matrixtype-row-col.slang`                        | functional | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `arraytype-sized-elements.slang`                  | functional | `#aggregate-collection-types`                                             |
| `tupletype-named-fields.slang`                    | functional | `#aggregate-collection-types`                                             |
| `optionaltype-value-hasvalue.slang`               | functional | `#aggregate-collection-types`                                             |
| `nonetype-and-nullptrtype.slang`                  | functional | `#pointer-reference-parameter-passing-types`                              |
| `ptrtype-dereference.slang`                       | functional | `#pointer-reference-parameter-passing-types`                              |
| `paramtype-out-inout-in.slang`                    | functional | `#pointer-reference-parameter-passing-types`                              |
| `functype-callable.slang`                         | functional | `#functype`                                                               |
| `andtype-conjunction-dispatch.slang`              | functional | `#andtype`                                                                |
| `andtype-missing-conformance-rejected.slang`      | negative   | `#andtype`                                                                |
| `thistype-interface-method.slang`                 | functional | `#thistype-and-assoctypedecl`                                             |
| `modifiedtype-const-rejects-assign.slang`         | negative   | `#existentials-and-this-types`                                            |
| `errortype-cascading-suppression.slang`           | negative   | `#errortype-and-bottomtype`                                               |
| `initializerlisttype-collapse-to-target.slang`    | functional | `#general-purpose-types`                                                  |
| `overloadgrouptype-ambiguous-rejected.slang`      | negative   | `#general-purpose-types`                                                  |
| `stringtype-literal-emitted.slang`                | functional | `#string-dynamic-misc`                                                    |
| `constantbuffertype-hlsl-emit.slang`              | functional | `#parameter-group-constant-buffer-types`                                  |
| `parameterblocktype-hlsl-emit.slang`              | functional | `#parameter-group-constant-buffer-types`                                  |
| `rwstructuredbuffertype-multi-target.slang`       | functional | `#buffer-types`                                                           |
| `structuredbuffer-readonly-hlsl-emit.slang`       | functional | `#buffer-types`                                                           |
| `byteaddressbuffertype-hlsl-emit.slang`           | functional | `#buffer-types`                                                           |
| `texturetype-samplerstate-hlsl-emit.slang`        | functional | `#resource-and-texture-type-families`                                     |
| `basictype-int8-min-max.slang`                    | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `basictype-uint8-zero-and-max.slang`              | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `basictype-uint-wrap-overflow.slang`              | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `basictype-int-min-max.slang`                     | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `basictype-float-extremes.slang`                  | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `basictype-half-arithmetic.slang`                 | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `basictype-literal-too-large-for-uint64.slang`    | negative   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vectortype-vec2-smallest.slang`                  | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vectortype-mixed-arithmetic.slang`               | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vectortype-vec4-full-swizzle.slang`              | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `matrixtype-2x2-square.slang`                     | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `matrixtype-4x4-square-stress.slang`              | stress     | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `matrixtype-3x2-rectangular.slang`                | boundary   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `arraytype-single-element.slang`                  | boundary   | `#aggregate-collection-types`                                             |
| `arraytype-multi-dim-2x3.slang`                   | boundary   | `#aggregate-collection-types`                                             |
| `arraytype-stress-16-elements.slang`              | stress     | `#aggregate-collection-types`                                             |
| `ptrtype-alias-roundtrip.slang`                   | boundary   | `#pointer-reference-parameter-passing-types`                              |
| `nullptrtype-equality.slang`                      | boundary   | `#pointer-reference-parameter-passing-types`                              |
| `optionaltype-optional-of-float.slang`            | boundary   | `#aggregate-collection-types`                                             |
| `optionaltype-optional-of-bool.slang`             | boundary   | `#aggregate-collection-types`                                             |
| `optionaltype-nested-stress.slang`                | stress     | `#aggregate-collection-types`                                             |
| `functype-zero-param.slang`                       | boundary   | `#functype`                                                               |
| `functype-eight-param-stress.slang`               | stress     | `#functype`                                                               |
| `andtype-three-way-conjunction.slang`             | stress     | `#andtype`                                                                |
| `constantbuffertype-single-scalar.slang`          | boundary   | `#parameter-group-constant-buffer-types`                                  |
| `parameterblocktype-struct-of-arrays.slang`       | boundary   | `#parameter-group-constant-buffer-types`                                  |
| `stringtype-empty-literal.slang`                  | boundary   | `#string-dynamic-misc`                                                    |
| `stringtype-long-literal-stress.slang`            | stress     | `#string-dynamic-misc`                                                    |
| `vec2-half-arithmetic.slang`                      | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-half-arithmetic.slang`                      | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec4-half-construction.slang`                    | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec2-double-arithmetic.slang`                    | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-double-arithmetic.slang`                    | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec4-double-construction.slang`                  | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec2-int8-construction.slang`                    | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-uint8-construction.slang`                   | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec4-uint8-construction.slang`                   | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-int16-arithmetic.slang`                     | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec4-uint16-construction.slang`                  | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec2-int64-arithmetic.slang`                     | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-uint64-arithmetic.slang`                    | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec2-bool-logical.slang`                         | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-bool-construction.slang`                    | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-half-zero-and-negzero.slang`                | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-float-inf-nan.slang`                        | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec4-double-extremes.slang`                      | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-to-vec2-truncation.slang`                   | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec2-to-vec3-extension.slang`                    | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec2-vec3-implicit-rejected.slang`               | negative   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-int-to-float-conversion.slang`              | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-uint8-wrap-overflow.slang`                  | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec4-int8-min.slang`                             | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec2-double-zero-and-negzero.slang`              | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec4-uint-zero-and-max.slang`                    | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `vec3-int-min-max.slang`                          | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat2x3-float-construction.slang`                 | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat2x4-float-construction.slang`                 | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat4x2-float-construction.slang`                 | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat3x4-float-arithmetic.slang`                   | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat4x3-float-construction.slang`                 | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat2x4-int-construction.slang`                   | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat2x3-double-construction.slang`                | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat3x3-double-square.slang`                      | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat4x4-int-square.slang`                         | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat3x3-half-hlsl-emit.slang`                     | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat2x4-add-arithmetic.slang`                     | expansion  | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `mat-rectangular-shape-mismatch-rejected.slang`   | negative   | `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`          |
| `tuple-mixed-precision.slang`                     | expansion  | `#aggregate-collection-types`                                             |
| `optional-double2.slang`                          | expansion  | `#aggregate-collection-types`                                             |
| `optional-half3.slang`                            | expansion  | `#aggregate-collection-types`                                             |
| `array-of-half3.slang`                            | expansion  | `#aggregate-collection-types`                                             |
| `array-of-double2.slang`                          | expansion  | `#aggregate-collection-types`                                             |
| `struct-mixed-precision-hlsl-emit.slang`          | expansion  | `#declreftype`                                                            |

## Bootstrap test issues observed

(none)

## Compiler-bug observations

- Under `slangi` (INTERPRET), elementwise add of two
  `vector<half, N>` values aborts with
  `Cannot find execution handler for instruction add.f16v2`
  (and presumably `add.f16v3` / `add.f16v4`). Scalar half add via
  per-lane reads works fine. The expansion tests
  `vec2-half-arithmetic.slang` and `vec3-half-arithmetic.slang`
  therefore add per-lane, not at the vector level.
- Under `slangi`, casting a `half` lane read from a
  `matrix<half, R, C>` to `int` or `float` aborts with
  `Cannot find execution handler for instruction cast.i32.f16` /
  `cast.f32.f16` even though the same cast works for a scalar
  `half` local. The half-matrix corner is therefore observed
  through `-target hlsl` text emission
  (`mat3x3-half-hlsl-emit.slang`) instead of INTERPRET.
- Under `slangi`, `isnan(...)` and `isinf(...)` resolve to
  intrinsics whose `__target_switch` has no interpreter-
  compatible branch (`E41011`). The float-inf/nan vector
  boundary test (`vec3-float-inf-nan.slang`) uses the
  documented `x != x` (NaN) and `x > 1e38f` (inf) per-lane
  predicates instead.
- Under `slangi`, default-initializing a struct that contains a
  `half` field and then writing each field by assignment aborts
  with SIGSEGV (138/139). Using an aggregate initializer
  (`Mixed m = { half(1.5), ..., 4 };`) crashes the interpreter
  too. The struct-mixed-precision observation
  (`struct-mixed-precision-hlsl-emit.slang`) therefore uses
  `-target hlsl` text emission instead of INTERPRET.

## Doc gaps observed

- The doc names `NamedExpressionType` as "a typedef'd / typealias'd
  type that preserves the original name for diagnostics". In
  practice, both `typedef T Y;` and `typealias Y = T;` produce a
  type-mismatch diagnostic that renders the **underlying** type's
  name, not the alias name. Either the doc's claim about
  diagnostic-preservation should be removed, or the diagnostic
  renderer needs to look through `NamedExpressionType` differently.
  No test in this bundle exercises that claim; recording as a gap.
- The doc says `ConditionalType` is "compile-time conditional type"
  but does not name a user-spelling production. The grammar pointer
  is empty `(none)`; without a surface form this is not testable
  through `slangc`. A doc-level pointer to the conditional-type
  spelling (or a note saying it is checker-synthesized) would
  resolve this.
- `AtomicType` is "atomic wrapper type" in the doc with grammar
  `(none)`. `Atomic<T>` is a surface spelling but its observable
  behavior depends on the target (groupshared / structured-buffer)
  and is not portable on the no-GPU runner. A doc-level pointer to
  the minimal portable observation would let an agent anchor a
  test.
- The doc lists `EnumTypeType` as "the type of an `enum` type
  itself (its kind)". The user-facing claim is unclear: an enum
  used as a type is observable (covered by
  `ast-reference/declarations/enumdecl-tagged-union.slang`), but
  the "type of the enum type" itself is a kind-level claim that
  has no obvious surface. Recording as a gap.
- The doc lists `BottomType` as "bottom type representing 'no
  value'; used in the type lattice". The note says it is "used as
  the result type of expressions that do not return (e.g. throw)".
  `throw` is unobservable under INTERPRET (per `_common.md`
  lessons); a portable-on-no-GPU surface would be needed to test
  this.
- The doc lists 9 operands for `TextureType` but does not enumerate
  them. The observable surface (one test that observes the
  presence of `Texture2D<T>` on the HLSL backend) covers only the
  first operand. A doc-level table of the 9 operands and which
  surface spellings select each combination would let agents
  anchor finer-grained tests.
- The doc lists a "Fp8" family (`FloatE4M3Type`, `FloatE5M2Type`)
  and a `BFloat16Type` but no portable surface for these is
  documented. The user-spelling appears target-gated; a doc-level
  pointer to the minimal targetable spelling would let an agent
  anchor a test.
- `DifferentialPairType` and `DifferentialPtrPairType` are
  "differential pair (primal, differential) used by autodiff".
  Observable only through `[Differentiable]` machinery; the
  autodiff feature lives in a separate bundle. A doc pointer to
  that bundle would clarify boundary ownership.

## Out of scope

The doc is overwhelmingly about internal AST shape. The following
claim families are not observable through `slangc` / `slang-test`
directives that this bundle can run; they are recorded here rather
than tested.

- **Internal C++ parent class** of any concrete `Type` (e.g. that
  `VectorExpressionType` extends `ArithmeticExpressionType`, that
  `Fp8Type` extends `DeclRefType`, that `BuiltinGenericType`
  extends `BuiltinType`). Only the user-observable role of each
  leaf is testable.
- **Operand-list layout** described under "Key fields" in `## Nodes`
  ("operand-encoded", "(element-type, count operand-encoded)",
  ...). The `m_operands: List<ValNodeOperand>` storage and the
  indices each concrete class uses are not visible at the surface.
- **Singleton-ness / hash-cons identity** of pseudo-types
  (`ErrorType`, `BottomType`, `NullPtrType`, `NoneType`,
  `OverloadGroupType`, `InitializerListType` are each one-per-
  `ASTBuilder`). Two textually identical Slang types share the
  same `Val*` after hash-cons, but the surface only sees the
  post-hoist behavior, not the identity.
- **Abstract intermediates** from the family hierarchy
  (`ArithmeticExpressionType`, `Fp8Type`, `BuiltinType`,
  `DataLayoutType`, `PointerLikeType`, `BuiltinGenericType`,
  `ResourceType`, `TextureTypeBase`, `TextureShapeType`,
  `HLSLStructuredBufferTypeBase`, `ParameterGroupType`,
  `OutParamTypeBase`, `PtrTypeBase`, `ParamPassingModeType`,
  `UniformParameterGroupType`, `VaryingParameterGroupType`,
  `UntypedBufferResourceType`, `StringTypeBase`). Each carries no
  FIDDLE concrete tag and produces no user spelling of its own.
- **Synthesized / lowering-only types** with no user-written
  spelling: `ExtractExistentialType`, `ExistentialSpecializedType`,
  `GenericDeclRefType`, `NamespaceType`, `NativeRefType`,
  `ExplicitRefType`, `RefParamType` (the `ref T` spelling exists
  but its observable surface is the same as `inout T` for the
  no-GPU runner cases we can write; covered indirectly by
  `paramtype-out-inout-in.slang`).
- **Fp8 family** (`FloatE4M3Type`, `FloatE5M2Type`, `BFloat16Type`)
  — storage-only floats; surface support varies by target.
- **`CoopVectorExpressionType`**, **`DifferentialPairType`**,
  **`DifferentialPtrPairType`** — subsystem-specific (cooperative
  math, autodiff); observable surface belongs to those feature
  bundles.
- **GLSL-only types** (`GLSLImageType`, `GLSLShaderStorageBufferType`,
  `GLSLInputParameterGroupType`, `GLSLOutputParameterGroupType`,
  `GLSLAtomicUintType`, `GLSLInputAttachmentType`) — observable
  only via GLSL-source ingest; not exercised here.
- **Data-layout marker types** (`DefaultDataLayoutType`,
  `Std430DataLayoutType`, `Std140DataLayoutType`,
  `ScalarDataLayoutType`, `CDataLayoutType`,
  `DefaultPushConstantDataLayoutType`, `IBufferDataLayoutType`) —
  visible only inside the buffer-type encoding; not user-spellable
  in surface Slang.
- **Geometry / tessellation / mesh IO types** (`HLSLPatchType`,
  `HLSLInputPatchType`, `HLSLOutputPatchType`, the stream-output
  family, the mesh-output family) — require a real GPU pipeline
  stage and are not exercisable on the no-GPU runner.
- **Sampler-feedback** (`FeedbackType`), **subpass-input**
  (`SubpassInputType`), **raytracing-acceleration-structure**
  (`RaytracingAccelerationStructureType`), **dynamic-resource**
  (`DynamicResourceType`), **descriptor-handle**
  (`DescriptorHandleType`), **tensor-view** (`TensorViewType`) —
  GPU-only / target-only surfaces; existence is documented but
  not exercisable from a portable .slang on the no-GPU runner.
- **All pack / variadic types** (`EachType`, `ExpandType`,
  `PackBranchType`, `FirstPackElementType`, `LastPackElementType`,
  `TrimFirstTypePack`, `TrimLastTypePack`, `ConcreteTypePack`,
  `ValuePackType`) — observable through the pack-expression family
  (covered by `ast-reference/expressions`'s pack-related tests and
  the future `language-feature/generics-and-packs` bundle).
- **Differentiable-function marker types**
  (`DifferentiableType`, `DifferentiablePtrType`,
  `DefaultInitializableType`, `FunctionBaseType`,
  `DifferentiableFuncBaseType`, `ForwardDiffFuncInterfaceType`,
  `BwdCallableBaseType`, `BwdDiffFuncInterfaceType`,
  `LegacyBwdDiffFuncInterfaceType`, `FwdDiffFuncType`,
  `BwdDiffFuncType`, `BwdCallableFuncType`, `ApplyForBwdFuncType`,
  `RematFuncType`) — produced by autodiff machinery; observable
  through the differentiate-expression family
  (`ast-reference/expressions`).
- **`DynamicType`** — the dynamic-dispatch erased type, produced
  by existential elimination. Not user-spellable.
- **`NativeStringType`** — internal native `const char*`-style
  string used in low-level lowering; not user-spellable.
- **`AtomicType`** — surface spelling exists (`Atomic<T>`) but
  observable behavior depends on the target; recorded as a doc
  gap above.
- **`ConditionalType`** — no surface spelling documented; doc gap
  above.
- **`EnumTypeType`** — the "kind" of an enum type; no surface
  observation distinct from `EnumDecl` (covered in
  `ast-reference/declarations`).
- **`NamedExpressionType`** — alias-name diagnostic preservation
  does not hold on the current implementation; doc gap above. The
  semantic interchangeability of an alias with the original type
  is covered by `ast-reference/declarations/typedefdecl.slang` and
  `ast-reference/declarations/typealiasdecl.slang`.
- **`BottomType`** — bottom of the type lattice, used by `throw`;
  `throw` is not portably observable on the no-GPU runner under
  INTERPRET (per `_common.md` lessons).
- The **`## Family hierarchy`** mermaid diagram itself.
