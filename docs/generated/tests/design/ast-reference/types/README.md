---
generated: true
model: claude-opus-4-8
generated_at: 2026-07-02T00:00:00+00:00
source_commit: 8e2a63cbf22c85332796702fd02a875466486a23
watched_paths_digest: 69b1b4b4ebfd2e31c72694cb96999536c58a4466e09db936fc79e843d50fc2ad
source_doc: docs/generated/design/ast-reference/types.md
source_doc_digest: 9c52ba15d0eafbd60acaa7a5a0aaabf4123c728ae98bbcd3de937ab4696060f4
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for design/ast-reference/types

## Intent

This bundle exercises the concrete `Type` subclasses catalogued in
[`docs/generated/design/ast-reference/types.md`](../../../design/ast-reference/types.md)
through their **user-observable consequences** at type-resolution and
type-mismatch-diagnostic time. Slang does not expose its AST, so a
claim about a type class is testable only through program-text
behavior: a `float4` value's four components are readable, a
`const int` refuses assignment, an `IA & IB` bound rejects a type that
conforms to only one interface, an undefined identifier produces
exactly one diagnostic (no cascade), and so on.

Coverage strategy: one representative observation per **user-spellable**
type family, with a negative / diagnostic probe for every family that
has a natural negative form (vector / matrix shape mismatch,
const-write, `AndType` missing-conformance, out-parameter used before
assignment, ambiguous overload, cascading-suppression). The primary
directive is `INTERPRET` (slangi) for target-independent value claims,
`DIAGNOSTIC_TEST` for negative claims, and `COMPARE_COMPUTE -cpu` for
the wrap-overflow / pointer / error-type value claims that INTERPRET
cannot model. One multi-target `SIMPLE` test
(`rwstructuredbuffertype-multi-target`) is the canonical per-target
type-lowering observation across hlsl / glsl / spirv-asm / metal /
wgsl / cuda; the remaining resource / buffer / constant-buffer spelling
tests pin the HLSL surface form. The internal-shape claims (C++ parent
classes, operand slots, hash-cons identity, singletons) and the
synthesized-only / GPU-only / autodiff-only / pack-only classes are
recorded under `## Untested claims`.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| ----- | ------ | ------ | ----- |
| An integer literal too large to be represented in any integer BasicExpressionType is diagnosed with an out-of-range error. | negative | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`basictype-literal-too-large-rejected.slang`](basictype-literal-too-large-rejected.slang) |
| A 32-bit int BasicExpressionType holds its documented MIN (-2147483648) and MAX (2147483647) boundary values without truncation. | boundary | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`basictype-int-min-max.slang`](basictype-int-min-max.slang) |
| A MatrixExpressionType is parameterised by element-type, row count and column count; a float2x3 value's six elements are reachable by [row][col] indexing. | functional | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`matrixtype-row-col.slang`](matrixtype-row-col.slang) |
| A uint BasicExpressionType wraps modulo 2^32 on overflow; uint(0xFFFFFFFF) + 1 wraps to 0. | boundary | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`basictype-uint-wrap-overflow.slang`](basictype-uint-wrap-overflow.slang) |
| A VectorExpressionType at the smallest documented arity (count 2) carries both components; an int2 reads back its two elements. | boundary | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`vectortype-vec2-smallest.slang`](vectortype-vec2-smallest.slang) |
| A VectorExpressionType is parameterised by element-type and count; a float4 value's four components are independently readable by swizzle. | functional | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`vectortype-components.slang`](vectortype-components.slang) |
| Each BasicExpressionType (int, uint, float, bool) behaves as the corresponding scalar built-in; its value flows through evaluation as that type. | functional | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`basictype-scalars.slang`](basictype-scalars.slang) |
| Two MatrixExpressionTypes with different row/column counts are distinct; assigning a float2x2 to a float3x3 is rejected. | negative | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`matrixtype-shape-mismatch-rejected.slang`](matrixtype-shape-mismatch-rejected.slang) |
| Two VectorExpressionTypes with different counts are distinct types; assigning a float2 to a float3 without a conversion is rejected. | negative | [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | [`vectortype-size-mismatch-rejected.slang`](vectortype-size-mismatch-rejected.slang) |
| An AndType `IA & IB` requires both conformances; a type conforming to only IA is rejected where `IA & IB` is required. | negative | [#andtype](../../../design/ast-reference/types.md#andtype) | [`andtype-missing-conformance-rejected.slang`](andtype-missing-conformance-rejected.slang) |
| An AndType `IA & IB` requires conformance to both interfaces; a type conforming to both can be passed and both interface methods dispatch. | functional | [#andtype](../../../design/ast-reference/types.md#andtype) | [`andtype-conjunction-dispatch.slang`](andtype-conjunction-dispatch.slang) |
| An ArrayExpressionType carries an element-type and an element-count; each of the N elements of a sized array is independently readable and writable. | functional | [#aggregate-collection-types](../../../design/ast-reference/types.md#aggregate-collection-types) | [`arraytype-sized-elements.slang`](arraytype-sized-elements.slang) |
| An OptionalType<T> holds either none or a wrapped T; .hasValue and .value expose the two states. | functional | [#aggregate-collection-types](../../../design/ast-reference/types.md#aggregate-collection-types) | [`optionaltype-value-hasvalue.slang`](optionaltype-value-hasvalue.slang) |
| A TupleType holds a heterogeneous member-type list; the i-th element is reachable as ._i and has the i-th declared type. | functional | [#aggregate-collection-types](../../../design/ast-reference/types.md#aggregate-collection-types) | [`tupletype-named-fields.slang`](tupletype-named-fields.slang) |
| An enum name is usable as a value type; a variable of the enum type holds one of its enumerators and compares by underlying value. | functional | [#aggregate-collection-types](../../../design/ast-reference/types.md#aggregate-collection-types) | [`enumtypetype-usable-as-type.slang`](enumtypetype-usable-as-type.slang) |
| A DeclRefType carries generic substitutions; two instantiations of the same generic struct with different type arguments behave as distinct types with substituted field types. | functional | [#declreftype](../../../design/ast-reference/types.md#declreftype) | [`declreftype-generic-substitution.slang`](declreftype-generic-substitution.slang) |
| A struct name is a DeclRefType referencing the StructDecl; a variable declared with that type stores the struct's fields and member access reads them. | functional | [#declreftype](../../../design/ast-reference/types.md#declreftype) | [`declreftype-struct-name.slang`](declreftype-struct-name.slang) |
| ErrorType is the type of a failed expression; downstream uses of the failed value do not produce cascading type-mismatch diagnostics, so a single root cause yields a single diagnostic. | negative | [#errortype-and-bottomtype](../../../design/ast-reference/types.md#errortype-and-bottomtype) | [`errortype-cascading-suppression.slang`](errortype-cascading-suppression.slang) |
| A ModifiedType `const T` qualifies the base type with a modifier; writing through a const-modified local is not an l-value and is diagnosed. | negative | [#existentials-and-this-types](../../../design/ast-reference/types.md#existentials-and-this-types) | [`modifiedtype-const-rejects-assign.slang`](modifiedtype-const-rejects-assign.slang) |
| A FuncType carries an optional error type for a throwing callable; the value on the success path flows through when no error is thrown. | functional | [#functype](../../../design/ast-reference/types.md#functype) | [`functype-error-type.slang`](functype-error-type.slang) |
| A FuncType holds parameter types and a result type; a function-typed value is invokable to produce its result. | functional | [#functype](../../../design/ast-reference/types.md#functype) | [`functype-callable.slang`](functype-callable.slang) |
| A ConstantBufferType<T> uniform makes its fields visible and is the AST type for HLSL cbuffer; the HLSL emit renders the cbuffer keyword. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`constantbuffertype-hlsl-emit.slang`](constantbuffertype-hlsl-emit.slang) |
| A PtrType `T*` is a raw pointer whose pointee is reachable by dereference; a load through the pointer reads the pointed-to value and a store writes it back. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`ptrtype-dereference.slang`](ptrtype-dereference.slang) |
| A StringType String-typed literal preserves its text through to emitted target code. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`stringtype-literal-emitted.slang`](stringtype-literal-emitted.slang) |
| A TextureType Texture2D<T> and a SamplerStateType SamplerState are accepted as uniforms and a sample dispatches; the HLSL emit renders both spellings. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`texturetype-samplerstate-hlsl-emit.slang`](texturetype-samplerstate-hlsl-emit.slang) |
| An HLSLByteAddressBufferType / HLSLRWByteAddressBufferType is accepted at a uniform and bytes are addressable by Load/Store; the HLSL emit renders the ByteAddressBuffer spelling. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`byteaddressbuffertype-hlsl-emit.slang`](byteaddressbuffertype-hlsl-emit.slang) |
| An HLSLRWStructuredBufferType uniform reaches every text-emit backend; each backend lowers it to its documented surface form. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`rwstructuredbuffertype-multi-target.slang`](rwstructuredbuffertype-multi-target.slang) |
| An HLSLStructuredBufferType (read-only StructuredBuffer<T>) is accepted at a uniform and its elements are addressable; the HLSL emit renders the StructuredBuffer spelling. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`structuredbuffer-readonly-hlsl-emit.slang`](structuredbuffer-readonly-hlsl-emit.slang) |
| An InitializerListType `{a, b, ...}` collapses into the target type once matched; a brace list fills a struct field-wise and an array element-wise. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`initializerlisttype-collapse-to-target.slang`](initializerlisttype-collapse-to-target.slang) |
| An OutParamType requires the callee to assign before read; reading an out parameter before assigning it is diagnosed as use of an uninitialized value. | negative | [#nodes](../../../design/ast-reference/types.md#nodes) | [`paramtype-out-not-assigned-rejected.slang`](paramtype-out-not-assigned-rejected.slang) |
| BorrowInParamType / OutParamType / BorrowInOutParamType distinguish parameter-passing modes; only inout and out make callee mutation visible at the caller. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`paramtype-out-inout-in.slang`](paramtype-out-inout-in.slang) |
| OverloadGroupType is the pseudo-type of an unresolved overload set; when overload resolution is ambiguous the diagnostic reports the candidate set. | negative | [#nodes](../../../design/ast-reference/types.md#nodes) | [`overloadgrouptype-ambiguous-rejected.slang`](overloadgrouptype-ambiguous-rejected.slang) |
| The NoneType of `none` is accepted at any Optional target without an explicit cast; an Optional initialized to none reports hasValue false. | functional | [#nodes](../../../design/ast-reference/types.md#nodes) | [`nonetype-and-nullptrtype.slang`](nonetype-and-nullptrtype.slang) |
| A ThisType inside an interface stands for the conforming type; a method whose return type is `This` returns the concrete type when dispatched on a concrete value. | functional | [#thistype-and-assoctypedecl](../../../design/ast-reference/types.md#thistype-and-assoctypedecl) | [`thistype-interface-method.slang`](thistype-interface-method.slang) |

## Untested claims

| Claim | Reason | Anchor | Why untested |
| ----- | ------ | ------ | ------------ |
| A NamedExpressionType from a `typealias` / `typedef` preserves the original name for diagnostics. | (unclassified) | [#nodes](../../../design/ast-reference/types.md#nodes) | Observed behavior contradicts the doc: type-mismatch diagnostics render the resolved/canonical type (`float`, `vector<int,3>`), never the alias spelling. Recorded as a `drift-from-source` doc gap instead of a failing test. |
| The C++ parent class of any `Type` (e.g. `VectorExpressionType : ArithmeticExpressionType`, `Fp8Type : DeclRefType`) and the abstract intermediates (`ArithmeticExpressionType`, `BuiltinType`, `ResourceType`, `PointerLikeType`, `ParameterGroupType`, `OutParamTypeBase`, ...). | internal-source-fact | [#family-hierarchy](../../../design/ast-reference/types.md#family-hierarchy) | The class hierarchy is a C++ implementation fact with no user-observable consequence; only each leaf's user role is testable. |
| The `m_operands: List<ValNodeOperand>` storage and the per-class operand indices (e.g. operand 0 of `VectorExpressionType` is the element type). | internal-source-fact | [#nodes](../../../design/ast-reference/types.md#nodes) | Surface tests cannot see operand layout; only the resulting behavior (component count, element type) is observable. |
| Singleton-ness of `ErrorType` / `BottomType` / `NullPtrType` / `NoneType` (one per `ASTBuilder`) and hash-cons pointer-equality of textually identical types. | internal-source-fact | [#errortype-and-bottomtype](../../../design/ast-reference/types.md#errortype-and-bottomtype) | Identity of `Val*` in the `ASTBuilder` is not surfaced to a `.slang` program. |
| `ExtractExistentialType`, `ExistentialSpecializedType`, `GenericDeclRefType`, `NamespaceType`, `NativeRefType`, `DynamicType` — synthesized-only / lowering-only types. | out-of-bundle | [#existentials-and-this-types](../../../design/ast-reference/types.md#existentials-and-this-types) | Not user-spellable; observable only through the existential-feature bundle, not the type-catalog surface. |
| `Fp8Type` family (`FloatE4M3Type`, `FloatE5M2Type`, `BFloat16Type`), `CoopVectorExpressionType`, `DifferentialPairType`, `TensorViewType`. | gpu-other | [#arithmetic-types-scalar-vector-matrix](../../../design/ast-reference/types.md#arithmetic-types-scalar-vector-matrix) | Surface support is target-dependent (cooperative math / autodiff / low-precision) and not portable on the no-GPU runner. |
| GLSL-only types (`GLSLImageType`, `GLSLShaderStorageBufferType`, `GLSLInputParameterGroupType`, `GLSLOutputParameterGroupType`, `GLSLAtomicUintType`, `GLSLInputAttachmentType`). | out-of-bundle | [#resource-texture-sampler-types](../../../design/ast-reference/types.md#resource-texture-sampler-types) | Observable only via GLSL-source ingest, not exercised in this bundle. |
| Data-layout marker types (`DefaultDataLayoutType`, `Std430DataLayoutType`, `Std140DataLayoutType`, `ScalarDataLayoutType`, `CDataLayoutType`, `DefaultPushConstantDataLayoutType`, `IBufferDataLayoutType`). | internal-source-fact | [#data-layout-types](../../../design/ast-reference/types.md#data-layout-types) | Visible only inside the buffer-type encoding; not user-spellable in surface Slang. |
| Geometry / tessellation / mesh IO types (`HLSLInputPatchType`, `HLSLOutputPatchType`, the stream-output family, `VerticesType` / `IndicesType` / `PrimitivesType`). | gpu-non-compute | [#geometry-and-tessellation-shader-io-types](../../../design/ast-reference/types.md#geometry-and-tessellation-shader-io-types) | Require a real GPU pipeline stage (geometry / hull / domain / mesh); not exercisable on the no-GPU runner. |
| `FeedbackType`, `SubpassInputType`, `RaytracingAccelerationStructureType`, `DynamicResourceType`, `DescriptorHandleType`. | gpu-other | [#resource-texture-sampler-types](../../../design/ast-reference/types.md#resource-texture-sampler-types) | GPU-only / target-only surfaces; documented but not exercisable from a portable .slang on the no-GPU runner. |
| Pack / variadic types (`EachType`, `ExpandType`, `PackBranchType`, `FirstPackElementType`, `LastPackElementType`, `TrimFirstTypePack`, `TrimLastTypePack`, `ConcreteTypePack`, `ValuePackType`). | out-of-bundle | [#pack-variadic-types](../../../design/ast-reference/types.md#pack-variadic-types) | Observable through the pack-expression family covered by `ast-reference/expressions`; the type-side claim is the same observation. |
| Differentiable-function marker types (`DifferentiableType`, `DifferentiablePtrType`, `DefaultInitializableType`, `FunctionBaseType`, and the fwd/bwd diff-function type family). | out-of-bundle | [#differentiable-function-types](../../../design/ast-reference/types.md#differentiable-function-types) | Produced by autodiff machinery; observable through the differentiate-expression family in `ast-reference/expressions`. |
| `TextureBufferType` (HLSL `tbuffer`), `HLSLAppendStructuredBufferType`, `HLSLConsumeStructuredBufferType`, `HLSLRasterizerOrdered*` buffer variants. | out-of-bundle | [#buffer-types](../../../design/ast-reference/types.md#buffer-types) | Sibling variants of the covered `StructuredBuffer` / `RWStructuredBuffer` / `ByteAddressBuffer` families; the type-role observation is identical and does not add a distinct AST claim. |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| ------ | ---- | --- | ------------------ |
| [#nodes](../../../design/ast-reference/types.md#nodes) | drift-from-source | The `NamedExpressionType` row states it "preserves the original name for diagnostics", but a type-mismatch on a `typealias Celsius = float` (or `typedef`) renders the resolved type in the message (`got 'float'`), never `Celsius`. The alias name is not surfaced in E30019 / E30081 diagnostics. | Correct the row to state the alias name is preserved (if anywhere) in reflection / declaration contexts, and drop or qualify the "for diagnostics" claim, since observed type-mismatch diagnostics print the canonical resolved type. |
| [#nodes](../../../design/ast-reference/types.md#nodes) | missing-surface | The `OverloadGroupType`, `InitializerListType`, and `ErrorType` rows describe pseudo-types with "(none)" grammar and no example of the source shape that produces them. A reader cannot tell what user code exercises each. | Add a one-line "produced by" example per pseudo-type row: an ambiguous call for `OverloadGroupType`, a brace-list `{a, b}` for `InitializerListType`, a failed expression for `ErrorType`. |
| [#basicexpressiontype-vectorexpressiontype-matrixexpressiontype](../../../design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype) | undocumented-behavior | The doc describes scalar / vector / matrix leaves but says nothing about overflow / wrap semantics (unsigned wraps mod 2^32; a literal too large for any integer type is diagnosed E10012 + E40016). | Add a short "boundary behavior" note to the arithmetic-type section naming unsigned wrap-on-overflow and the too-large-literal diagnostic, so tests can anchor to a documented claim rather than compiler observation. |

## Sibling-bundle overlap

The `Decl` side that `DeclRefType` references (`StructDecl`,
`InterfaceDecl`, `EnumDecl`) is owned by
`design/ast-reference/declarations`; the type-expression **spellings**
(`PointerTypeExpr`, `FuncTypeExpr`, `AndTypeExpr`, `ThisTypeExpr`,
`ModifiedTypeExpr`) are owned by `design/ast-reference/expressions`;
the IR-level type catalog is owned by `design/ir-reference/types`. This
bundle stays at the AST / type-checker surface (interpreter values and
diagnostics) and does not duplicate those claims.
