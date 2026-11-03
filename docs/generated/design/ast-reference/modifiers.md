---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-03T07:02:26Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: b01b225b8bde27c3d88361d42256af2a347366a98b51c393066f46d210f81d54
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Modifiers and Attributes Reference

The reference for every concrete `Modifier` subclass in the Slang
AST, written for a contributor working in the parser, checker, or
backend emit who needs to know which class a particular `[attr]` or
keyword becomes, and where its semantic checking lives. `Modifier`
itself is documented in [base.md](base.md#modifier-syntaxnode).

Slang draws a deliberate distinction between two related families of
syntax:

- **Modifiers** are keyword-like markers without an argument list:
  `in`, `out`, `inout`, `const`, `static`, `uniform`,
  `globallycoherent`, `noperspective`, ... Each one is its own
  `Modifier` subclass.
- **Attributes** use the `[name(args)]` syntax and derive from
  `AttributeBase` (which itself derives from `Modifier`). Concrete
  attribute classes (`UnrollAttribute`, `NumThreadsAttribute`, ...)
  hold parsed arguments and any post-checking metadata.

Both are linked off `ModifiableSyntaxNode::modifiers` and walked with
`findModifier<T>()` / `hasModifier<T>()`.

## Source

Modifier classes are declared in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h),
on top of the `Modifier` / `ModifiableSyntaxNode` pair in
[slang-ast-base.h](../../../../source/slang/slang-ast-base.h).
Parsing happens in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp):
`parseAttributeName` and `ParseSquareBracketAttributes` handle the
`[name(args)]` form, and `parseUncheckedGLSLLayoutAttribute` handles
the valued entries of a GLSL `layout(...)` group. Attributes are
dispatched through the `AttributeDecl` table documented in
[../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md).

The *surface spelling* of an attribute is not declared in the C++
modifier header at all. It comes from `attribute_syntax` declarations in
four sources — three core-module `.meta.slang` files and one
experimental standard module: the bulk in
[core.meta.slang](../../../../source/slang/core.meta.slang), the
differentiability attributes (the markers in `core.meta.slang`, the
derivative bindings in
[diff.meta.slang](../../../../source/slang/diff.meta.slang)), the
Vulkan pointer attributes in
[hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang), and the
work-graph attributes in the experimental standard module
[workgraph.slang](../../../../source/standard-modules/experimental/workgraph.slang).
The spellings quoted below were read from those four files, all of which
this page watches, so renaming a spelling marks the page stale.

## Family hierarchy

```mermaid
flowchart TD
  Modifier --> VisibilityModifier
  Modifier --> InModifier
  Modifier --> OutModifier
  Modifier --> ConstModifier
  Modifier --> InlineModifier
  Modifier --> BuiltinModifier
  Modifier --> AttributeBase
  Modifier --> HLSLSemantic
  Modifier --> InterpolationModeModifier
  Modifier --> SimpleModifier
  Modifier --> TypeModifier
  Modifier --> GLSLLayoutModifierGroupMarker
  Modifier --> GLSLPreprocessorDirective
  Modifier --> HLSLGeometryShaderInputPrimitiveTypeModifier
  Modifier --> HLSLMeshShaderOutputModifier
  Modifier --> OtherModifier["IntrinsicOpModifier, TargetIntrinsicModifier, BuiltinTypeModifier, ..."]
  OutModifier --> InOutModifier
  VisibilityModifier --> PublicModifier
  VisibilityModifier --> PrivateModifier
  VisibilityModifier --> InternalModifier
  AttributeBase --> Attribute
  AttributeBase --> UncheckedAttribute
  AttributeBase --> UncheckedGLSLLayoutAttribute
  AttributeBase --> GLSLImplicitOffsetLayoutAttribute
  Attribute --> DifferentiableAttribute
  Attribute --> InheritanceControlAttribute
  Attribute --> ReceiverParamModeSpecifierAttribute
  ReceiverParamModeSpecifierAttribute --> MutatingAttribute
  ReceiverParamModeSpecifierAttribute --> NonmutatingAttribute
  ReceiverParamModeSpecifierAttribute --> ConstRefAttribute
  ReceiverParamModeSpecifierAttribute --> RefAttribute
  Attribute --> GLSLSimpleIntegerLayoutAttribute
  Attribute --> OtherAttr["UnrollAttribute, NumThreadsAttribute, EntryPointAttribute, ..."]
  DifferentiableAttribute --> UserDefinedDerivativeAttribute
  DifferentiableAttribute --> DerivativeOfAttribute
  DifferentiableAttribute --> OtherDiff["TreatAsDifferentiableAttribute, ForwardDifferentiableAttribute, BackwardDifferentiableAttribute, ..."]
  HLSLSemantic --> HLSLLayoutSemantic
  HLSLSemantic --> HLSLSimpleSemantic
  HLSLSemantic --> RayPayloadAccessSemantic
  HLSLLayoutSemantic --> HLSLRegisterSemantic
  HLSLLayoutSemantic --> HLSLPackOffsetSemantic
  RayPayloadAccessSemantic --> RayPayloadReadSemantic
  RayPayloadAccessSemantic --> RayPayloadWriteSemantic
  TypeModifier --> WrappingTypeModifier
  TypeModifier --> ResourceElementFormatModifier
  TypeModifier --> NoDiffModifier
  TypeModifier --> MatrixLayoutModifier
  ResourceElementFormatModifier --> UNormModifier
  ResourceElementFormatModifier --> SNormModifier
  MatrixLayoutModifier --> RowMajorLayoutModifier
  MatrixLayoutModifier --> ColumnMajorLayoutModifier
  HLSLGeometryShaderInputPrimitiveTypeModifier --> HLSLPointModifier
  HLSLGeometryShaderInputPrimitiveTypeModifier --> HLSLLineModifier
  HLSLGeometryShaderInputPrimitiveTypeModifier --> HLSLTriangleModifier
  HLSLGeometryShaderInputPrimitiveTypeModifier --> HLSLLineAdjModifier
  HLSLGeometryShaderInputPrimitiveTypeModifier --> HLSLTriangleAdjModifier
  HLSLMeshShaderOutputModifier --> HLSLVerticesModifier
  HLSLMeshShaderOutputModifier --> HLSLIndicesModifier
  HLSLMeshShaderOutputModifier --> HLSLPrimitivesModifier
```

Abstract intermediates: `VisibilityModifier`,
`GLSLLayoutModifierGroupMarker`, `HLSLSemantic`,
`MatrixLayoutModifier`, `RowMajorLayoutModifier`,
`ColumnMajorLayoutModifier`, `InterpolationModeModifier`,
`AttributeBase`, `InheritanceControlAttribute`,
`ReceiverParamModeSpecifierAttribute`.

## Nodes

Every attribute inherits `attributeDecl: AttributeDecl*`,
`originalIdentifierToken: Token`, and `args: List<Expr*>` from
`AttributeBase`, and every checked `Attribute` adds
`intArgVals: List<Val*>`. Many attribute classes declare no members of
their own and read their arguments straight out of those inherited
lists; those rows name the inherited `args: List<Expr*>` in the
**Key fields** column and leave the meaning of each argument to the
Summary column, while `(no additional state)` marks a class that carries
no data at all.

| Class | Parent | Key fields | Grammar | Summary |
| --- | --- | --- | --- | --- |
| `InModifier` | `Modifier` | (no additional state) | [parameter modifier](../syntax-reference/grammar.md#modifiers) | `in` parameter direction. |
| `OutModifier` | `Modifier` | (no additional state) | [parameter modifier](../syntax-reference/grammar.md#modifiers) | `out` parameter direction. |
| `InOutModifier` | `OutModifier` | (no additional state) | [parameter modifier](../syntax-reference/grammar.md#modifiers) | `inout` parameter direction (a refinement of `out`). |
| `RefModifier` | `Modifier` | (no additional state) | [parameter modifier](../syntax-reference/grammar.md#modifiers) | `__ref` parameter passing mode; the un-prefixed `ref` is not a parameter modifier (it spells a `ref` property accessor instead). |
| `BorrowModifier` | `Modifier` | (no additional state) | [parameter modifier](../syntax-reference/grammar.md#modifiers) | Borrow-by-reference (read-only) parameter passing mode; the parser registers it under both `__borrow` and `__constref`, and there is no un-prefixed `borrow` spelling. |
| `RefReadOnlyModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `__ref_readonly` parameter passing mode: by reference, read-only in the callee. |
| `RefWriteOnlyModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `__ref_writeonly` parameter passing mode: by reference, write-only in the callee. |
| `ConstModifier` | `Modifier` | (no additional state) | [storage class](../syntax-reference/grammar.md#modifiers) | Prefix `const`; on a `__ref` parameter it is the legacy alias of `__ref_readonly`. |
| `CppStyleTrailingReceiverParamConstModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | A `const` written after a function's parameter list (`int read() const`); attached to the `FuncDecl`. |
| `InlineModifier` | `Modifier` | (no additional state) | [storage class](../syntax-reference/grammar.md#modifiers) | `inline`. |
| `ParamModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | Internal marker on synthesized parameters. |
| `ConstExprModifier` | `Modifier` | (no additional state) | [storage class](../syntax-reference/grammar.md#modifiers) | `constexpr`. |
| `ExternModifier` | `Modifier` | (no additional state) | [storage class](../syntax-reference/grammar.md#modifiers) | `extern` (link-time declaration). |
| `DynModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | Marks a dynamic-dispatch context. |
| `ExternCppModifier` | `Modifier` | (no additional state) | (none) | Marks `extern "C++"` mapping for record/replay. |
| `PublicModifier` | `VisibilityModifier` | (no additional state) | [visibility](../syntax-reference/grammar.md#modifiers) | `public`. |
| `PrivateModifier` | `VisibilityModifier` | (no additional state) | [visibility](../syntax-reference/grammar.md#modifiers) | `private`. |
| `InternalModifier` | `VisibilityModifier` | (no additional state) | [visibility](../syntax-reference/grammar.md#modifiers) | `internal` visibility keyword. |
| `OverrideModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `override` for interface-default overrides. |
| `IsOverridingModifier` | `Modifier` | `overridedDecl: Decl*` | (none) | Internal marker set by checking once an override is bound to the decl it overrides. |
| `RequireModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `require` (interface requirement marker). |
| `BuiltinModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | Marks a declaration as part of the core module. |
| `HLSLExportModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | HLSL-style `export` keyword. |
| `ExportedModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | Marks an exported declaration. |
| `TransparentModifier` | `Modifier` | (no additional state) | (none) | Marks a member as transparent for name lookup. |
| `FromCoreModuleModifier` | `Modifier` | (no additional state) | (none) | Marks decls imported from the core module. |
| `PrefixModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | Marks an operator as prefix-arity. |
| `PostfixModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | Marks an operator as postfix-arity. |
| `HLSLEffectSharedModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | HLSL `shared` (effect-shared variable). |
| `HLSLGroupSharedModifier` | `Modifier` | (no additional state) | [HLSL storage](../syntax-reference/grammar.md#modifiers) | `groupshared`. |
| `HLSLStaticModifier` | `Modifier` | (no additional state) | [HLSL storage](../syntax-reference/grammar.md#modifiers) | `static`. |
| `HLSLUniformModifier` | `Modifier` | (no additional state) | [HLSL storage](../syntax-reference/grammar.md#modifiers) | `uniform`. |
| `HLSLVolatileModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `volatile`. |
| `PreciseModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `precise`. |
| `HLSLNoInterpolationModifier` | `InterpolationModeModifier` | (no additional state) | [interpolation](../syntax-reference/grammar.md#modifiers) | `nointerpolation`. |
| `HLSLNoPerspectiveModifier` | `InterpolationModeModifier` | (no additional state) | [interpolation](../syntax-reference/grammar.md#modifiers) | `noperspective`. |
| `HLSLLinearModifier` | `InterpolationModeModifier` | (no additional state) | [interpolation](../syntax-reference/grammar.md#modifiers) | `linear`. |
| `HLSLSampleModifier` | `InterpolationModeModifier` | (no additional state) | [interpolation](../syntax-reference/grammar.md#modifiers) | `sample`. |
| `HLSLCentroidModifier` | `InterpolationModeModifier` | (no additional state) | [interpolation](../syntax-reference/grammar.md#modifiers) | `centroid`. |
| `PerVertexModifier` | `InterpolationModeModifier` | (no additional state) | [interpolation](../syntax-reference/grammar.md#modifiers) | `pervertex`. |
| `HLSLRowMajorLayoutModifier` | `RowMajorLayoutModifier` | (no additional state) | [matrix layout](../syntax-reference/grammar.md#modifiers) | HLSL `row_major`. |
| `HLSLColumnMajorLayoutModifier` | `ColumnMajorLayoutModifier` | (no additional state) | [matrix layout](../syntax-reference/grammar.md#modifiers) | HLSL `column_major`. |
| `GLSLRowMajorLayoutModifier` | `ColumnMajorLayoutModifier` | (no additional state) | (none) | GLSL `row_major`; no parse path constructs it. |
| `GLSLColumnMajorLayoutModifier` | `RowMajorLayoutModifier` | (no additional state) | (none) | GLSL `column_major`; no parse path constructs it. |
| `HLSLGeometryShaderInputPrimitiveTypeModifier` | `Modifier` | (no additional state) | (none) | Common base for the geometry-shader input-primitive markers below. |
| `HLSLPointModifier` | `HLSLGeometryShaderInputPrimitiveTypeModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `point` (GS input). |
| `HLSLLineModifier` | `HLSLGeometryShaderInputPrimitiveTypeModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `line`. |
| `HLSLTriangleModifier` | `HLSLGeometryShaderInputPrimitiveTypeModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `triangle`. |
| `HLSLLineAdjModifier` | `HLSLGeometryShaderInputPrimitiveTypeModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `lineadj`. |
| `HLSLTriangleAdjModifier` | `HLSLGeometryShaderInputPrimitiveTypeModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `triangleadj`. |
| `HLSLMeshShaderOutputModifier` | `Modifier` | (no additional state) | (none) | Common base for the mesh-shader output-array markers below. |
| `HLSLVerticesModifier` | `HLSLMeshShaderOutputModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `vertices` (mesh shader). |
| `HLSLIndicesModifier` | `HLSLMeshShaderOutputModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `indices`. |
| `HLSLPrimitivesModifier` | `HLSLMeshShaderOutputModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `primitives`. |
| `HLSLPayloadModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `payload` (amplification-shader payload). |
| `HLSLLayoutSemantic` | `HLSLSemantic` | `registerName: Token`, `componentMask: Token` | (none) | Base class for HLSL semantics that affect layout (register / packoffset). |
| `RayPayloadAccessSemantic` | `HLSLSemantic` | `stageNameTokens: List<Token>` | (none) | Base class for ray-payload read/write access semantics. |
| `HLSLSimpleSemantic` | `HLSLSemantic` | `name: Token` (on `HLSLSemantic`) | [semantic](../syntax-reference/grammar.md#declarations) | `: NAME` (no parenthesized arguments). |
| `HLSLRegisterSemantic` | `HLSLLayoutSemantic` | `spaceName: Token`, plus inherited `registerName` | [register binding](../syntax-reference/grammar.md#modifiers) | `: register(...)`. |
| `HLSLPackOffsetSemantic` | `HLSLLayoutSemantic` | `uniformOffset: int` | [pack offset](../syntax-reference/grammar.md#modifiers) | `: packoffset(...)`. |
| `RayPayloadReadSemantic` | `RayPayloadAccessSemantic` | (no additional state) | [semantic](../syntax-reference/grammar.md#declarations) | `: read(...)` ray-payload semantic. |
| `RayPayloadWriteSemantic` | `RayPayloadAccessSemantic` | (no additional state) | [semantic](../syntax-reference/grammar.md#declarations) | `: write(...)` ray-payload semantic. |
| `GLSLPrecisionModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | GLSL precision qualifier. |
| `GLSLModuleModifier` | `Modifier` | (no additional state) | (none) | Legacy marker kept registered for deserializing older module ASTs; the front-end neither creates nor interprets it. |
| `GLSLPreprocessorDirective` | `Modifier` | (no additional state) | (none) | Base class for GLSL preprocessor directives preserved in the AST. |
| `GLSLVersionDirective` | `GLSLPreprocessorDirective` | `versionNumberToken: Token`, `glslProfileToken: Token` | (none) | `#version` preprocessor directive carried as AST. |
| `GLSLExtensionDirective` | `GLSLPreprocessorDirective` | `extensionNameToken: Token`, `dispositionToken: Token` | (none) | `#extension` directive. |
| `GLSLLayoutModifierGroupBegin` | `GLSLLayoutModifierGroupMarker` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | Start marker of a `layout(...)` group. |
| `GLSLLayoutModifierGroupEnd` | `GLSLLayoutModifierGroupMarker` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | End marker of a `layout(...)` group. |
| `GLSLUnparsedLayoutModifier` | `Modifier` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | Marker for an unrecognized or assignment-less GLSL layout qualifier; the parser diagnoses it immediately. |
| `GLSLBufferDataLayoutModifier` | `Modifier` | (no additional state) | (none) | Base for buffer-layout modifiers. |
| `GLSLStd140Modifier` | `GLSLBufferDataLayoutModifier` | (no additional state) | [layout(std140)](../syntax-reference/grammar.md#modifiers) | `std140`. |
| `GLSLStd430Modifier` | `GLSLBufferDataLayoutModifier` | (no additional state) | [layout(std430)](../syntax-reference/grammar.md#modifiers) | `std430`. |
| `GLSLScalarModifier` | `GLSLBufferDataLayoutModifier` | (no additional state) | (none) | `scalar` layout. |
| `GLSLBufferModifier` | `WrappingTypeModifier` | (no additional state) | (none) | `buffer` modifier on a type. |
| `GLSLWriteOnlyModifier` | `SimpleModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `writeonly`. |
| `GLSLReadOnlyModifier` | `SimpleModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `readonly`. |
| `GLSLVolatileModifier` | `SimpleModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | GLSL `volatile`. |
| `GLSLRestrictModifier` | `SimpleModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `restrict`. |
| `GLSLPatchModifier` | `SimpleModifier` | (no additional state) | (none) | `patch` (tess input). |
| `GloballyCoherentModifier` | `SimpleModifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | `globallycoherent`. |
| `SimpleModifier` | `Modifier` | (no additional state) | (none) | Base for keyword-only modifiers parsed via the generic SimpleModifier path. |
| `MemoryQualifierSetModifier` | `Modifier` | `memoryQualifiers: uint32_t`, `memoryModifiers: List<Modifier*>` | (none) | Aggregated GLSL memory qualifiers (`coherent`, `volatile`, etc.) on a single decl. |
| `TypeModifier` | `Modifier` | (no additional state) | (none) | Base for modifiers that semantically attach to a type. |
| `WrappingTypeModifier` | `TypeModifier` | (no additional state) | (none) | Type modifier that wraps a child type. |
| `ResourceElementFormatModifier` | `TypeModifier` | (no additional state) | (none) | Base for resource-element format modifiers. |
| `UNormModifier` | `ResourceElementFormatModifier` | (no additional state) | [unorm](../syntax-reference/grammar.md#modifiers) | `unorm`. |
| `SNormModifier` | `ResourceElementFormatModifier` | (no additional state) | [snorm](../syntax-reference/grammar.md#modifiers) | `snorm`. |
| `NoDiffModifier` | `TypeModifier` | (no additional state) | [no_diff](../syntax-reference/grammar.md#modifiers) | `no_diff` type modifier. |
| `BitFieldModifier` | `Modifier` | `width: IntegerLiteralValue`, `offset: IntegerLiteralValue`, `backingDeclRef: DeclRef<VarDecl>` | (none) | C-style bitfield specification on a member variable; `offset` and `backingDeclRef` are filled in during checking. |
| `DynamicUniformModifier` | `Modifier` | (no additional state) | [modifier](../syntax-reference/grammar.md#modifiers) | Marks a parameter as dynamic-uniform. |
| `ToBeSynthesizedModifier` | `Modifier` | (no additional state) | (none) | Placeholder marking a decl that the checker should synthesize. |
| `SynthesizedModifier` | `Modifier` | `op: uint32_t`, `operands: List<Val*>` | (none) | Marks decls produced by checker synthesis. |
| `SynthesizedStaticLambdaFuncModifier` | `Modifier` | (no additional state) | (none) | Marks the static-lambda function synthesized for `LambdaDecl`. |
| `SynthesizedParamPassingModeModifier` | `Modifier` | `mode: ParamPassingMode` | (none) | Records the passing mode a synthesized declaration must preserve. No source spelling. |
| `ExplicitlyDeclaredCapabilityModifier` | `Modifier` | `declaredCapabilityRequirements: CapabilitySetVal*` | (none) | Marks capability sets that were written by the user. |
| `LocalTempVarModifier` | `Modifier` | (no additional state) | (none) | Marks compiler-introduced local temporaries. |
| `ExistentialOpenedOnVarModifier` | `Modifier` | (no additional state) | (none) | Marks a variable as the result of opening an existential. |
| `VarReassignedModifier` | `Modifier` | (no additional state) | (none) | Marks a variable that has been re-assigned (data-flow info). |
| `ExtensionExternVarModifier` | `Modifier` | `originalDecl: DeclRef<Decl>` | (none) | Marks variables surfaced from an extension via `extern`. |
| `ActualGlobalModifier` | `Modifier` | (no additional state) | (none) | Marks the real backing decl behind a global generic. |
| `IgnoreForLookupModifier` | `Modifier` | (no additional state) | (none) | Hides a decl from ordinary name lookup. |
| `OptionalConstraintModifier` | `Modifier` | (no additional state) | (none) | Marks a constraint as optional during inference. |
| `IntrinsicOpModifier` | `Modifier` | `opToken: Token`, `op: uint32_t` | (none) | Binds a decl to a Slang IR opcode (core-module intrinsics). |
| `TargetIntrinsicModifier` | `Modifier` | `targetToken: Token`, `definitionString: String`, `predicateToken: Token`, `scrutineeDeclRef: DeclRef<Decl>` | (none) | Binds a decl to a target-backend intrinsic. |
| `SpecializedForTargetModifier` | `Modifier` | `targetToken: Token` | (none) | Marks a decl as specialized for a target. |
| `RequiredGLSLExtensionModifier` | `Modifier` | `extensionNameToken: Token` | (none) | Marks a required GLSL extension. |
| `RequiredGLSLVersionModifier` | `Modifier` | `versionNumberToken: Token` | (none) | Marks the minimum GLSL version a decl requires. |
| `RequiredSPIRVVersionModifier` | `Modifier` | `version: SemanticVersion` | (none) | Marks the minimum SPIRV version. |
| `RequiredWGSLExtensionModifier` | `Modifier` | `extensionNameToken: Token` | (none) | Required WGSL extension. |
| `RequiredCUDASMVersionModifier` | `Modifier` | `version: SemanticVersion` | (none) | Required CUDA SM version. |
| `NVAPIMagicModifier` | `Modifier` | (no additional state) | (none) | NVAPI-magic binding flag. |
| `NVAPISlotModifier` | `Modifier` | `registerName: String`, `spaceName: String` | (none) | NVAPI slot binding, sourced from the `NV_SHADER_EXTN_SLOT` / `NV_SHADER_EXTN_REGISTER_SPACE` macros. |
| `BuiltinTypeModifier` | `Modifier` | `tag: BaseType` | (none) | Tags a decl as the canonical declaration of a built-in type. |
| `MagicTypeModifier` | `Modifier` | `magicName: String`, `tag: uint32_t`, `magicNodeType: SyntaxClass<NodeBase>` | (none) | Tags a decl as a magic type known to checker/IR-lowering. |
| `BuiltinRequirementModifier` | `Modifier` | `kind: BuiltinRequirementKind` | (none) | Tags interface requirements known to the compiler. |
| `IntrinsicTypeModifier` | `Modifier` | `irOp: uint32_t`, `irOperands: List<uint32_t>` | (none) | Tags a decl as an intrinsic type and names the IR opcode it lowers to. |
| `ImplicitConversionModifier` | `Modifier` | `cost: ConversionCost`, `builtinConversionKind: BuiltinConversionKind` | (none) | Marks an implicit conversion constructor and gives its ranking cost. |
| `AttributeTargetModifier` | `Modifier` | `syntaxClass: SyntaxClass<NodeBase>` | (none) | Internal modifier produced by `[__AttributeUsage(...)]`. |
| `ImplicitParameterGroupVariableModifier` | `Modifier` | (no additional state) | (none) | Internal marker on auto-introduced parameter-group variables. |
| `ImplicitParameterGroupElementTypeModifier` | `Modifier` | (no additional state) | (none) | Internal marker on auto-introduced element types. |
| `ParameterGroupReflectionName` | `Modifier` | `nameAndLoc: NameLoc` | (none) | Carries the reflection name for an implicit parameter group. |
| `SharedModifiers` | `Modifier` | (no additional state) | (none) | Aggregates modifiers shared between several decls (e.g. multiple declarators in one declaration). |
| `UnscopedEnumCaseAliasModifier` | `Modifier` | `enumCaseDecl: EnumCaseDecl*` | (none) | Marks the `static const` alias the parser adds to the enclosing container for one case of an unscoped enum: for `[UnscopedEnum] enum E { A }` the alias `A` refers to `E.A`. |
| `HasInterfaceDefaultImplModifier` | `Modifier` | `defaultImplDecl: Decl*` | (none) | Marks an interface as having default-implementation requirements. |
| `UncheckedAttribute` | `AttributeBase` | `scope: Scope*`, plus inherited `args` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | Attribute as parsed before checking has resolved it to a concrete subclass; `scope` records where to look the name up. |
| `Attribute` | `AttributeBase` | `intArgVals: List<Val*>`, plus inherited `args` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | Base for all checker-resolved attribute classes. |
| `UserDefinedAttribute` | `Attribute` | `attributeDecl: AttributeDecl*` (on `AttributeBase`) | [user attribute](../syntax-reference/grammar.md#attributes-and-decorations) | A user-declared attribute (introduced via `attribute_syntax`). |
| `AttributeUsageAttribute` | `Attribute` | `targetSyntaxClass: SyntaxClass<NodeBase>` | [AttributeUsage](../syntax-reference/grammar.md#attributes-and-decorations) | `[__AttributeUsage(target)]` declaring where an attribute may be applied. |
| `UnrollAttribute` | `Attribute` | `args: List<Expr*>` (inherited) | [unroll](../syntax-reference/grammar.md#attributes-and-decorations) | `[unroll(N)]`; a hint forwarded to the downstream compiler that does not change Slang's own behaviour, so the loop survives into the emitted code carrying the hint. |
| `ForceUnrollAttribute` | `Attribute` | `maxIterations: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[ForceUnroll(N)]`; Slang performs the unrolling itself before emitting target code, so no loop remains in the output. |
| `MaxItersAttribute` | `Attribute` | `value: IntVal*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[MaxIters(N)]`. |
| `InferredMaxItersAttribute` | `Attribute` | `inductionVar: DeclRef<Decl>`, `value: int32_t` | (none) | Iteration bound inferred by checking rather than written by the user. |
| `LoopAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[loop]`. |
| `FastOptAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[fastopt]`. |
| `AllowUAVConditionAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[allow_uav_condition]`. |
| `BranchAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[branch]`. |
| `FlattenAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[flatten]`. |
| `ForceCaseAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[forcecase]`. |
| `CallAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[call]`. |
| `UnscopedEnumAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[UnscopedEnum]`; enum cases resolve unqualified through `static const` aliases the parser adds to the enclosing container. |
| `EnumClassModifier` | `Modifier` | (no additional state) | (none) | Marker the parser adds for the `enum class` spelling. |
| `FlagsAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[Flags]`; cases without an explicit initialiser are numbered with successive powers of two (1, 2, 4, ...) rather than consecutive integers. |
| `NonDynamicUniformAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NonUniformReturn]`. |
| `UnsafeForceInlineEarlyAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__unsafeForceInlineEarly]`. |
| `ForceInlineAttribute` | `Attribute` | (no additional state) | [ForceInline](../syntax-reference/grammar.md#attributes-and-decorations) | `[ForceInline]`. |
| `NoInlineAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[noinline]`. |
| `NoRefInlineAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[noRefInline]`. |
| `PreferRecomputeAttribute` | `Attribute` | `sideEffectBehavior: SideEffectBehavior` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[PreferRecompute]`; the nested enum selects whether side effects warn or are allowed. |
| `PreferCheckpointAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[PreferCheckpoint]`. |
| `AlwaysFoldIntoUseSiteAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__AlwaysFoldIntoUseSiteAttribute]`. |
| `OverloadRankAttribute` | `Attribute` | `rank: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[OverloadRank(N)]`. |
| `SpecializeAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[Specialize]`. |
| `KnownBuiltinAttribute` | `Attribute` | `name: IntVal*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[KnownBuiltin(name)]`; marks a decl as a known builtin. |
| `ReadNoneAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__readNone]`. |
| `MaximallyReconvergesAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[MaximallyReconverges]`. |
| `QuadDerivativesAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[QuadDerivatives]`. |
| `RequireFullQuadsAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[RequireFullQuads]`. |
| `DeprecatedAttribute` | `Attribute` | `message: String` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[deprecated(message)]`. |
| `RemovedSinceAttribute` | `Attribute` | `sinceVersion: int32_t`, `message: String` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[RemovedSince(...)]`. |
| `NonCopyableTypeAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__NonCopyableType]`. |
| `NoSideEffectAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__NoSideEffect]`. |
| `BuiltinAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[builtin]`. |
| `AutoDiffBuiltinAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__AutoDiffBuiltin]`; marks an autodiff builtin. |
| `RequireCapabilityAttribute` | `Attribute` | `capabilitySet: CapabilitySetVal*` | [require](../syntax-reference/grammar.md#attributes-and-decorations) | `[require(capability)]`; ties to the capability system. |
| `RequiresNVAPIAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__requiresNVAPI]`. |
| `RequirePreludeAttribute` | `Attribute` | `capabilitySet: CapabilitySetVal*`, `prelude: String` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[RequirePrelude(...)]`. |
| `AllowAttribute` | `Attribute` | `diagnostic: DiagnosticInfo const*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[allow("diagnostic-name")]`; suppresses one diagnostic. |
| `FormatAttribute` | `Attribute` | `format: ImageFormat` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[format("rgba32f")]`, also spelled `[vk::image_format("rgba16f")]`. |
| `ExternAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__extern]`. |
| `ComInterfaceAttribute` | `Attribute` | `guid: String` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[COM(guid)]`. |
| `PushConstantAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[push_constant]`. |
| `SpecializationConstantAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[SpecializationConstant]`. |
| `VkConstantIdAttribute` | `Attribute` | `location: int` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk::constant_id(...)]`, and the checked form of `layout(constant_id=N)`. |
| `ShaderRecordAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[shader_record]`. |
| `GLSLBindingAttribute` | `Attribute` | `binding: int32_t`, `set: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk::binding(...)]` / `[gl::binding(...)]`; also the merged checked form of `layout(binding=..., set=...)`. |
| `VkAliasedPointerAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk::aliased_pointer]`. |
| `VkRestrictPointerAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk::restrict_pointer]`. |
| `GLSLOffsetLayoutAttribute` | `Attribute` | `offset: int64_t` | (none) | Checked form of `layout(offset=N)`. |
| `GLSLImplicitOffsetLayoutAttribute` | `AttributeBase` | (no additional state) | (none) | Placeholder the parser adds when a `layout(...)` group has no explicit `offset`; checking turns it into a `GLSLOffsetLayoutAttribute`. |
| `GLSLSimpleIntegerLayoutAttribute` | `Attribute` | `value: int32_t` | (none) | Base for integer-valued GLSL layout attributes. |
| `GLSLInputAttachmentIndexLayoutAttribute` | `Attribute` | `location: IntegerLiteralValue` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk::input_attachment_index(...)]`. |
| `GLSLLocationAttribute` | `GLSLSimpleIntegerLayoutAttribute` | inherited `value: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk::location(...)]`. |
| `GLSLIndexAttribute` | `GLSLSimpleIntegerLayoutAttribute` | inherited `value: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk::index(...)]`. |
| `VkStructOffsetAttribute` | `GLSLSimpleIntegerLayoutAttribute` | inherited `value: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk_offset(...)]`. |
| `SPIRVInstructionOpAttribute` | `Attribute` | `args: List<Expr*>` (inherited) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[vk::spirv_instruction(...)]`. |
| `SPIRVTargetEnv13Attribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[spv_target_env_1_3]`. |
| `DisableArrayFlatteningAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[disable_array_flattening]`. |
| `GLSLLayoutLocalSizeAttribute` | `Attribute` | `extents[3]: IntVal*`, `axisIsSpecConstId[3]: bool`, `specConstExtents[3]: DeclRef<VarDeclBase>` | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(local_size_*)` workgroup size. |
| `GLSLLayoutDerivativeGroupQuadAttribute` | `Attribute` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `derivative_group_quadsNV` layout. |
| `GLSLLayoutDerivativeGroupLinearAttribute` | `Attribute` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `derivative_group_linearNV` layout. |
| `GLSLLayoutEarlyFragmentTestsAttribute` | `Attribute` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `early_fragment_tests` layout entry, from `parseLayoutModifier`; the checker lifts it to an `EarlyDepthStencilAttribute` on the fragment entry point. |
| `GLSLRequireShaderInputParameterAttribute` | `Attribute` | `parameterNumber: uint32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__GLSLRequireShaderInputParameter(N)]`; marks a required shader input. |
| `UncheckedGLSLLayoutAttribute` | `AttributeBase` | `args: List<Expr*>` (inherited) | (none) | Base for unchecked GLSL layout(...) entries. |
| `UncheckedGLSLBindingLayoutAttribute` | `UncheckedGLSLLayoutAttribute` | `args: List<Expr*>` (inherited) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(binding=N)`. |
| `UncheckedGLSLSetLayoutAttribute` | `UncheckedGLSLLayoutAttribute` | `args: List<Expr*>` (inherited) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(set=N)`. |
| `UncheckedGLSLOffsetLayoutAttribute` | `UncheckedGLSLLayoutAttribute` | `args: List<Expr*>` (inherited) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(offset=N)`. |
| `UncheckedGLSLInputAttachmentIndexLayoutAttribute` | `UncheckedGLSLLayoutAttribute` | `args: List<Expr*>` (inherited) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(input_attachment_index=N)`. |
| `UncheckedGLSLLocationLayoutAttribute` | `UncheckedGLSLLayoutAttribute` | `args: List<Expr*>` (inherited) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(location=N)`. |
| `UncheckedGLSLIndexLayoutAttribute` | `UncheckedGLSLLayoutAttribute` | `args: List<Expr*>` (inherited) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(index=N)`. |
| `UncheckedGLSLConstantIdAttribute` | `UncheckedGLSLLayoutAttribute` | `args: List<Expr*>` (inherited) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(constant_id=N)`. |
| `UncheckedGLSLRayPayloadAttribute` | `UncheckedGLSLLayoutAttribute` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(ray_payload)`. |
| `UncheckedGLSLRayPayloadInAttribute` | `UncheckedGLSLLayoutAttribute` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(ray_payload_in)`. |
| `UncheckedGLSLHitObjectAttributesAttribute` | `UncheckedGLSLLayoutAttribute` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(hit_object_attributes)`. |
| `UncheckedGLSLCallablePayloadAttribute` | `UncheckedGLSLLayoutAttribute` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(callable_payload)`. |
| `UncheckedGLSLCallablePayloadInAttribute` | `UncheckedGLSLLayoutAttribute` | (no additional state) | [layout qualifier](../syntax-reference/grammar.md#modifiers) | `layout(callable_payload_in)`. |
| `MaxTessFactorAttribute` | `Attribute` | `args: List<Expr*>` (inherited) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[maxtessfactor(...)]`. |
| `OutputControlPointsAttribute` | `Attribute` | `args: List<Expr*>` (inherited) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[outputcontrolpoints(...)]`. |
| `OutputTopologyAttribute` | `Attribute` | `args: List<Expr*>` (inherited) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[outputtopology(...)]` (also spelled `[OutputTopology(...)]`). |
| `PartitioningAttribute` | `Attribute` | `args: List<Expr*>` (inherited) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[partitioning(...)]`. |
| `PatchConstantFuncAttribute` | `Attribute` | `patchConstantFuncDecl: FuncDecl*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[patchconstantfunc(...)]`. |
| `DomainAttribute` | `Attribute` | `args: List<Expr*>` (inherited) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[domain(...)]`. |
| `EarlyDepthStencilAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[earlydepthstencil]`; the checker also adds it for `layout(early_fragment_tests) in;`. |
| `PostDepthCoverageAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[postdepthcoverage]`; a fragment-entry-point hint that `SV_Coverage` reports only samples that survived the early depth/stencil test. |
| `Shader64BitIndexingAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[Shader64BitIndexing]`. |
| `NumThreadsAttribute` | `Attribute` | `extents[3]: IntVal*`, `specConstExtents[3]: DeclRef<VarDeclBase>` | [numthreads](../syntax-reference/grammar.md#attributes-and-decorations) | `[numthreads(x,y,z)]` (also spelled `[NumThreads(...)]`); an axis is either a constant or a specialization constant. |
| `WaveSizeAttribute` | `Attribute` | `numLanes: IntVal*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[WaveSize(...)]`. |
| `MaxVertexCountAttribute` | `Attribute` | `value: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[maxvertexcount(...)]`. |
| `InstanceAttribute` | `Attribute` | `value: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[instance(count)]`. |
| `EntryPointAttribute` | `Attribute` | `capabilitySet: CapabilitySetVal*` | [shader](../syntax-reference/grammar.md#attributes-and-decorations) | `[shader("stage")]`. |
| `ExperimentalModuleAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[ExperimentalModule]`. |
| `FunctionInterfaceAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__FunctionInterface]`. |
| `NodeLaunchAttribute` | `Attribute` | `mode: String` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NodeLaunch("broadcasting" \| "thread" \| "coalescing")]`. |
| `NodeMaxDispatchGridAttribute` | `Attribute` | `x: IntVal*`, `y: IntVal*`, `z: IntVal*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NodeMaxDispatchGrid(x,y,z)]`; upper bound for a dynamic grid. |
| `NodeDispatchGridAttribute` | `Attribute` | `x: IntVal*`, `y: IntVal*`, `z: IntVal*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NodeDispatchGrid(x,y,z)]`; fixed grid size. |
| `MaxRecordsAttribute` | `Attribute` | `value: IntVal*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[MaxRecords(count)]`. |
| `NodeIDAttribute` | `Attribute` | `name: String`, `arrayIndex: IntVal*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NodeID(name, arrayIndex = 0)]`. |
| `NodeIsProgramEntryAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NodeIsProgramEntry]`. |
| `AllowSparseNodesAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[AllowSparseNodes]` on an output-node array parameter. |
| `NodeArraySizeAttribute` | `Attribute` | `count: IntVal*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NodeArraySize(count)]`. |
| `VulkanRayPayloadAttribute` | `Attribute` | `location: int` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__vulkanRayPayload(location)]`; also the checked form of `layout(ray_payload)`. |
| `VulkanRayPayloadInAttribute` | `Attribute` | `location: int` | (none) | Checked form of `layout(ray_payload_in)`; no `attribute_syntax` spelling. |
| `VulkanCallablePayloadAttribute` | `Attribute` | `location: int` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__vulkanCallablePayload(location)]`; also the checked form of `layout(callable_payload)`. |
| `VulkanCallablePayloadInAttribute` | `Attribute` | `location: int` | (none) | Checked form of `layout(callable_payload_in)`; no `attribute_syntax` spelling. |
| `VulkanHitAttributesAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__vulkanHitAttributes]`. |
| `VulkanHitObjectAttributesAttribute` | `Attribute` | `location: int` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__vulkanHitObjectAttributes(location)]`; also the checked form of `layout(hit_object_attributes)`. |
| `RayPayloadAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | Older HLSL `[raypayload]`. |
| `ThisParamInfoAttribute` | `Attribute` | `info: ParamInfo` | (none) | Checked type and passing mode of a declaration's effective `this` parameter. No source spelling. |
| `MutatingAttribute` | `ReceiverParamModeSpecifierAttribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[mutating]`. |
| `NonmutatingAttribute` | `ReceiverParamModeSpecifierAttribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[nonmutating]`. |
| `NoDiscardAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NoDiscard]`; marks a function whose result should not be discarded. |
| `ConstRefAttribute` | `ReceiverParamModeSpecifierAttribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[constref]`. |
| `RefAttribute` | `ReceiverParamModeSpecifierAttribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[__ref]`. |
| `AnyValueSizeAttribute` | `Attribute` | `size: int32_t` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[anyValueSize(size)]`. |
| `DifferentiableAttribute` | `Attribute` | `m_associatedValMapping: OrderedDictionary<Val*, OrderedDictionary<SlangInt, Val*>>` | (none) | Base of the family; has no `attribute_syntax` spelling of its own and caches each type's `IDifferentiable` witness. |
| `TreatAsDifferentiableAttribute` | `DifferentiableAttribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[TreatAsDifferentiable]`. |
| `HasTrivialForwardDerivativeAttribute` | `DifferentiableAttribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[HasTrivialForwardDerivative]`. |
| `ForwardDifferentiableAttribute` | `DifferentiableAttribute` | (no additional state) | [ForwardDifferentiable](../syntax-reference/grammar.md#attributes-and-decorations) | `[ForwardDifferentiable]`. |
| `UserDefinedDerivativeAttribute` | `DifferentiableAttribute` | `funcExpr: Expr*` | (none) | Base for explicit-derivative attributes. |
| `ForwardDerivativeAttribute` | `UserDefinedDerivativeAttribute` | inherited `funcExpr: Expr*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[ForwardDerivative(fn)]`. |
| `DerivativeOfAttribute` | `DifferentiableAttribute` | `funcExpr: Expr*`, `backDeclRef: Expr*` | (none) | Base for "X is the derivative of Y" attributes. |
| `ForwardDerivativeOfAttribute` | `DerivativeOfAttribute` | inherited `funcExpr: Expr*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[ForwardDerivativeOf(fn)]`. |
| `BackwardDifferentiableAttribute` | `DifferentiableAttribute` | `maxOrder: int` | [Differentiable](../syntax-reference/grammar.md#attributes-and-decorations) | Produced by both `[BackwardDifferentiable(order = 0)]` and the plain `[Differentiable(order = 0)]`. |
| `BackwardDerivativeAttribute` | `UserDefinedDerivativeAttribute` | inherited `funcExpr: Expr*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[BackwardDerivative(fn)]`. |
| `BackwardDerivativeOfAttribute` | `DerivativeOfAttribute` | inherited `funcExpr: Expr*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[BackwardDerivativeOf(fn)]`. |
| `PrimalSubstituteAttribute` | `Attribute` | `funcExpr: Expr*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[PrimalSubstitute(fn)]`. |
| `PrimalSubstituteOfAttribute` | `Attribute` | `funcExpr: Expr*`, `backDeclRef: Expr*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[PrimalSubstituteOf(fn)]`. |
| `NoDiffThisAttribute` | `Attribute` | `isSynthesized: bool` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[NoDiffThis]`; `isSynthesized` distinguishes a checker-added instance from a user-written one. |
| `DerivativeMemberAttribute` | `Attribute` | `memberDeclRef: DeclRefExpr*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[DerivativeMember(...)]`. |
| `MaybeDifferentiableAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[MaybeDifferentiable]` on an interface requirement: an optional conformance to `IForwardDifferentiable<Self>` / `IBackwardDifferentiable<Self>`. |
| `OpenAttribute` | `InheritanceControlAttribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[open]` on an aggregate type: inheritable from other modules. |
| `SealedAttribute` | `InheritanceControlAttribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[sealed]` on an aggregate type: not inheritable or implementable from other modules. |
| `DllImportAttribute` | `Attribute` | `modulePath: String, functionName: String` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[DllImport(...)]`. |
| `DllExportAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[DllExport]`. |
| `TorchEntryPointAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[TorchEntryPoint]`. |
| `CudaDeviceExportAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[CudaDeviceExport]` (also `[CUDADeviceExport]`). |
| `CudaKernelAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[CudaKernel]` (also `[CUDAKernel]`). |
| `CudaHostAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[CudaHost]` (also `[CUDAHost]`). |
| `AutoPyBindCudaAttribute` | `Attribute` | `fwdDiffFuncDeclRef: DeclRefExpr*`, `bwdDiffFuncDeclRef: DeclRefExpr*` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[AutoPyBindCUDA]`. |
| `PyExportAttribute` | `Attribute` | `name: String` | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[PyExport(name)]`. |
| `DerivativeGroupQuadAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[DerivativeGroupQuad]`. |
| `DerivativeGroupLinearAttribute` | `Attribute` | (no additional state) | [attribute](../syntax-reference/grammar.md#attributes-and-decorations) | `[DerivativeGroupLinear]`. |

## Notable nodes

### Modifiers vs Attributes

The split is by syntax: a *modifier* is a bare keyword that the
parser knows by name and immediately constructs the appropriate
`Modifier` subclass for; an *attribute* is an `[ident(args)]`
construct that the parser builds as an `UncheckedAttribute` and that
the checker then resolves to a concrete `Attribute` subclass by
looking up an `AttributeDecl` (see
[declarations.md](declarations.md)). Both ultimately end up linked
off `ModifiableSyntaxNode::modifiers`, so most consumer code does
not need to distinguish.

`ParseSquareBracketAttributes` accepts a few surface spellings that are
easy to miss. A group may be written either `[a]` or `[[a]]`, several
attributes may share one group, and the comma between them is optional,
so `[a, b]` and `[a b]` parse the same way. `parseAttributeName`
flattens a `::`-qualified name into a single identifier by replacing
each `::` with `_`, which is why the user-facing `[vk::binding(0)]`
resolves against an `AttributeDecl` registered under the name
`vk_binding`. A leading `::` becomes a leading `_`.

### IntrinsicOpModifier and the core module binding

`IntrinsicOpModifier` binds a core-module function declaration to the
Slang IR opcode it lowers to. For example,
`__intrinsic_op(select) T select(bool condition, T ifTrue, T ifFalse)` in
[core.meta.slang](../../../../source/slang/core.meta.slang) (line 1117)
carries an `IntrinsicOpModifier` whose `op` is the `select` opcode, so
lowering emits that opcode without recognizing the function by name. By
contrast, `sin` in `hlsl.meta.slang` has no opcode binding; it uses
per-target bodies (`__target_switch`, `__intrinsic_asm`). See
[../cross-cutting/core-module.md](../cross-cutting/core-module.md).

### TargetIntrinsicModifier and SpecializedForTargetModifier

`TargetIntrinsicModifier` binds a function declaration to a textual
intrinsic on a specific backend (e.g. `"fma"` on HLSL,
`"OpExtInst ..."` on SPIRV); its `targetToken` names the target whose
capabilities select the binding. Separately, it can carry an optional
predicate, which guards the intrinsic using the declaration its
`scrutineeDeclRef` resolves to.
`SpecializedForTargetModifier` is a per-target marker placed on
function declarations that are specifically intended for a target
backend; the checker prefers them when emitting.

### GLSLLayout*Modifier family

GLSL's `layout(...)` qualifier compiles to a chain of layout
modifiers: a `GLSLLayoutModifierGroupBegin`, one entry per
qualifier (each a concrete subclass like
`UncheckedGLSLBindingLayoutAttribute`,
`UncheckedGLSLLocationLayoutAttribute`, etc.), and a
`GLSLLayoutModifierGroupEnd`. The "Unchecked" prefix denotes the
parser-time representation; the checker resolves each entry to an
`Attribute`-rooted equivalent (e.g.
`GLSLBindingAttribute`, `GLSLLocationAttribute`).

These modifiers are how a parameter's binding is *represented* in the
AST. A checked `GLSLBindingAttribute` holds the resolved
`binding: int32_t` and `set: int32_t` pair, and the HLSL side stores the
same information as tokens: `HLSLLayoutSemantic` holds
`registerName: Token` and `componentMask: Token`, and
`HLSLRegisterSemantic` adds `spaceName: Token` for the register space.
A modifier records only what the declaration said; the rules that turn
those values into an actual binding belong to checking and layout, see
[../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md).

### Matrix layout modifiers

`row_major` and `column_major` always parse into the HLSL classes; the
`GLSL*` matrix-layout classes are never constructed by the parser. These
are `TypeModifier`s, so the parser moves them from a declarator onto the
type specifier.

### Visibility modifiers and language version

Whether a decl is `public`, `internal`, or `private` is encoded as a
`VisibilityModifier` subclass attached to the declaration. The
default depends on the module's `ModuleDecl::languageVersion` and
`defaultVisibility`: legacy Slang treats everything as `public`, the
modern language defaults to `internal`. See [declarations.md](declarations.md)
for `ModuleDecl` and
[../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)
for visibility resolution.

### RequireCapabilityAttribute

`[require(...)]` ties a declaration to one or more
capability atoms; the checker uses these to verify that calls into
this declaration are well-formed under the surrounding capability
set. See [../cross-cutting/targets.md](../cross-cutting/targets.md)
for the capability system.

### Differentiable attribute family

The differentiability attributes form a small hierarchy under
`DifferentiableAttribute` (declared at line 1754 of the header): marker
attributes (`[ForwardDifferentiable]`, `[BackwardDifferentiable]`)
describe *that* a function is differentiable,
`UserDefinedDerivativeAttribute` subclasses
(`[ForwardDerivative(fn)]`, `[BackwardDerivative(fn)]`) bind a
derivative function explicitly, and the `DerivativeOf` subclasses
(`[ForwardDerivativeOf(fn)]`, etc.) declare a function as the
derivative of another.

The class-to-spelling mapping is not one-to-one, which is easy to get
wrong when reading only the header. `DifferentiableAttribute` is an
abstract-in-practice base with no `attribute_syntax` of its own; the
plain `[Differentiable(order = 0)]` that most user code writes maps to
`BackwardDifferentiableAttribute`, exactly like the explicit
`[BackwardDifferentiable(order = 0)]`. Only `[ForwardDifferentiable]`
maps to `ForwardDifferentiableAttribute`.

The attribute is a front-end marker, not a fact that later stages
re-read. When checking accepts a `[Differentiable]` function `f`, it
synthesizes a conformance of the form
`extension __func_as_type(f) : IForwardDifferentiable<__func_as_type(f)>`,
and the checker's differentiability queries
(`SemanticsVisitor::isFuncForwardDifferentiable` and its backward
counterpart) hand back the resulting `SubtypeWitness*` rather than a
boolean read off the modifier. Downstream autodiff work keys off that
witness; see
[../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)
and [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).

### Work-graph node attribute spellings

The eight `Node*` / `MaxRecords` / `AllowSparseNodes` attribute classes
have their spellings declared outside `core.meta.slang`, in the
experimental
[workgraph.slang](../../../../source/standard-modules/experimental/workgraph.slang)
standard module; the derivative-binding attributes
(`diff.meta.slang`) and the Vulkan pointer attributes
(`hlsl.meta.slang`) are other groups outside it. Reading only
`slang-ast-modifier.h` therefore gives no hint of how they are spelled. Two of them are near-twins that are easy
to confuse: `NodeDispatchGridAttribute` records a grid size fixed in
the source, while `NodeMaxDispatchGridAttribute` records only an upper
bound for a grid supplied at launch. Both carry the same `x`/`y`/`z`
`IntVal*` triple, so the class, not the field shape, is what tells the
two apart.

### Receiver parameter modes

A declaration's receiver (`this`) mode is spelled by a
`ReceiverParamModeSpecifierAttribute` subclass (`[mutating]`,
`[nonmutating]`, `[constref]`, `[__ref]`) or by trailing `const`. Checking
attaches a single `ThisParamInfoAttribute` recording the result; a
synthesized declaration carries `SynthesizedParamPassingModeModifier`
instead. The mapping rules live in
[../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md).

### Reference-parameter keywords

`__ref_readonly` and `__ref_writeonly` are parameter passing modes in the
same family as `out`, `inout`, `__ref` and `__borrow`; `const` on a `__ref`
parameter is the legacy spelling of `__ref_readonly`. Their checking rules
are covered in
[../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md).

### HLSL semantics

HLSL semantics (`: SV_Target`, `: register(t0)`, `: packoffset(c0)`)
are modeled as a small hierarchy under `HLSLSemantic`. The simple
form (`HLSLSimpleSemantic`) just stores a name; the layout-bearing
forms (`HLSLRegisterSemantic`, `HLSLPackOffsetSemantic`) carry a
register class, index, or offset. Ray-payload accessor semantics
(`RayPayloadReadSemantic`, `RayPayloadWriteSemantic`) sit alongside
the standard variants under `RayPayloadAccessSemantic`.

### Synthesized modifiers and checker state

Several modifiers carry no user-visible state and exist purely to
let the checker tag declarations with derived properties:
`ToBeSynthesizedModifier`, `SynthesizedModifier`,
`IgnoreForLookupModifier`, `VarReassignedModifier`, and
`ExistentialOpenedOnVarModifier` are examples. These never come from
user syntax; they are added by the checker and inspected later.

### MemoryQualifierSetModifier and GLSL memory qualifiers

GLSL allows several memory qualifiers (`coherent`, `volatile`,
`readonly`, `writeonly`, `restrict`) to be applied to the same
declaration. Rather than carrying each as a separate modifier, the
checker may aggregate them into a single `MemoryQualifierSetModifier`
with a bitmask of flags. The individual GLSL-prefixed modifiers
(`GLSLReadOnlyModifier`, `GLSLWriteOnlyModifier`, etc.) still exist
and may appear at parse time; checking merges them into the
aggregated form when convenient.

### Absent groupings: no HLSLAttribute or LayoutModifier base

There is no `HLSLAttribute`, `LayoutModifier`, or `HLSLLayoutModifier`
class; none of the three names appears anywhere under `source/`. HLSL
shader-stage attributes such as `NumThreadsAttribute` and
`EntryPointAttribute` derive directly from `Attribute` with no
HLSL-specific intermediate, and the layout-related roles are split
between `HLSLLayoutSemantic` (semantics that affect layout),
`MatrixLayoutModifier` (matrix storage layout), and the
`GLSLLayout*Modifier` group described above.

## See also

- [base.md](base.md) — `Modifier` base class.
- [declarations.md](declarations.md) — declarations that carry
  modifiers; `AttributeDecl` for attribute declarations.
- [expressions.md](expressions.md) — `ModifiedTypeExpr` carries an
  inline `Modifiers` list.
- [types.md](types.md) — `ModifiedType` Val that wraps a type with
  modifiers.
- [values.md](values.md) — `ModifierVal` family used inside
  `ModifiedType`.
- [../syntax-reference/grammar.md#modifiers](../syntax-reference/grammar.md#modifiers)
  — surface syntax for modifiers and attributes.
- [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md) — where
  modifiers and attributes are parsed.
- [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md) —
  modifier checking.
- [../cross-cutting/targets.md](../cross-cutting/targets.md) —
  capability system that interprets `RequireCapabilityAttribute`,
  `TargetIntrinsicModifier`, etc.
- [../cross-cutting/core-module.md](../cross-cutting/core-module.md)
  — how `IntrinsicOpModifier`, `BuiltinTypeModifier`,
  `MagicTypeModifier` bind core-module declarations.
