// slang-modifier-defs.h

// Syntax class definitions for modifiers.

// Simple modifiers have no state beyond their identity
#define SIMPLE_MODIFIER(NAME) \
    SIMPLE_SYNTAX_CLASS(NAME##Modifier, Modifier)

SIMPLE_MODIFIER(In);
SIMPLE_MODIFIER(Out);
SIMPLE_MODIFIER(Const);
SIMPLE_MODIFIER(Instance);
SIMPLE_MODIFIER(Builtin);
SIMPLE_MODIFIER(Inline);
SIMPLE_MODIFIER(Public);
SIMPLE_MODIFIER(Require);
SIMPLE_MODIFIER(Param);
SIMPLE_MODIFIER(Extern);
SIMPLE_MODIFIER(Input);
SIMPLE_MODIFIER(Transparent);
SIMPLE_MODIFIER(FromStdLib);
SIMPLE_MODIFIER(Prefix);
SIMPLE_MODIFIER(Postfix);
SIMPLE_MODIFIER(Exported);
SIMPLE_MODIFIER(ConstExpr);
SIMPLE_MODIFIER(GloballyCoherent)

#undef SIMPLE_MODIFIER

// A modifier that marks something as an operation that
// has a one-to-one translation to the IR, and thus
// has no direct definition in the high-level language.
//
SYNTAX_CLASS(IntrinsicOpModifier, Modifier)

    // token that names the intrinsic op
    FIELD(Token, opToken)

    // The opcode for the intrinsic operation
    FIELD_INIT(IROp, op, kIROp_Nop)
END_SYNTAX_CLASS()

// A modifier that marks something as an intrinsic function,
// for some subset of targets.
SYNTAX_CLASS(TargetIntrinsicModifier, Modifier)
    // Token that names the target that the operation
    // is an intrisic for.
    FIELD(Token, targetToken)

    // A custom definition for the operation
    FIELD(Token, definitionToken)
END_SYNTAX_CLASS()

// A modifier that marks a declaration as representing a
// specialization that should be preferred on a particular
// target.
SYNTAX_CLASS(SpecializedForTargetModifier, Modifier)
    // Token that names the target that the operation
    // has been specialized for.
    FIELD(Token, targetToken)
END_SYNTAX_CLASS()

// A modifier to tag something as an intrinsic that requires
// a certain GLSL extension to be enabled when used
SYNTAX_CLASS(RequiredGLSLExtensionModifier, Modifier)
FIELD(Token, extensionNameToken)
END_SYNTAX_CLASS()

// A modifier to tag something as an intrinsic that requires
// a certain GLSL version to be enabled when used
SYNTAX_CLASS(RequiredGLSLVersionModifier, Modifier)
FIELD(Token, versionNumberToken)
END_SYNTAX_CLASS()


SIMPLE_SYNTAX_CLASS(InOutModifier, OutModifier)

// `__ref` modifier for by-reference parameter passing
SIMPLE_SYNTAX_CLASS(RefModifier, Modifier)

// This is a special sentinel modifier that gets added
// to the list when we have multiple variable declarations
// all sharing the same modifiers:
//
//     static uniform int a : FOO, *b : register(x0);
//
// In this case both `a` and `b` share the syntax
// for part of their modifier list, but then have
// their own modifiers as well:
//
//     a: SemanticModifier("FOO") --> SharedModifiers --> StaticModifier --> UniformModifier
//                                 /
//     b: RegisterModifier("x0")  /
//
SIMPLE_SYNTAX_CLASS(SharedModifiers, Modifier)

// A GLSL `layout` modifier
//
// We use a distinct modifier for each key that
// appears within the `layout(...)` construct,
// and each key might have an optional value token.
//
// TODO: We probably want a notion of  "modifier groups"
// so that we can recover good source location info
// for modifiers that were part of the same vs.
// different constructs.
ABSTRACT_SYNTAX_CLASS(GLSLLayoutModifier, Modifier)

// The token used to introduce the modifier is stored
// as the `nameToken` field.

// TODO: may want to accept a full expression here
FIELD(Token, valToken)
END_SYNTAX_CLASS()

// AST nodes to represent the begin/end of a `layout` modifier group
ABSTRACT_SYNTAX_CLASS(GLSLLayoutModifierGroupMarker, Modifier)
END_SYNTAX_CLASS()
SIMPLE_SYNTAX_CLASS(GLSLLayoutModifierGroupBegin, GLSLLayoutModifierGroupMarker)
SIMPLE_SYNTAX_CLASS(GLSLLayoutModifierGroupEnd, GLSLLayoutModifierGroupMarker)

// We divide GLSL `layout` modifiers into those we have parsed
// (in the sense of having some notion of their semantics), and
// those we have not.
ABSTRACT_SYNTAX_CLASS(GLSLParsedLayoutModifier     , GLSLLayoutModifier)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(GLSLUnparsedLayoutModifier   , GLSLLayoutModifier)

// Specific cases for known GLSL `layout` modifiers that we need to work with
SIMPLE_SYNTAX_CLASS(GLSLConstantIDLayoutModifier , GLSLParsedLayoutModifier)
SIMPLE_SYNTAX_CLASS(GLSLLocationLayoutModifier   , GLSLParsedLayoutModifier)

SIMPLE_SYNTAX_CLASS(GLSLLocalSizeLayoutModifier,    GLSLUnparsedLayoutModifier)
SIMPLE_SYNTAX_CLASS(GLSLLocalSizeXLayoutModifier,    GLSLLocalSizeLayoutModifier)
SIMPLE_SYNTAX_CLASS(GLSLLocalSizeYLayoutModifier,    GLSLLocalSizeLayoutModifier)
SIMPLE_SYNTAX_CLASS(GLSLLocalSizeZLayoutModifier,    GLSLLocalSizeLayoutModifier)

// A catch-all for single-keyword modifiers
SIMPLE_SYNTAX_CLASS(SimpleModifier, Modifier)

// Some GLSL-specific modifiers
SIMPLE_SYNTAX_CLASS(GLSLBufferModifier   , SimpleModifier)
SIMPLE_SYNTAX_CLASS(GLSLWriteOnlyModifier, SimpleModifier)
SIMPLE_SYNTAX_CLASS(GLSLReadOnlyModifier , SimpleModifier)
SIMPLE_SYNTAX_CLASS(GLSLPatchModifier    , SimpleModifier)

// Indicates that this is a variable declaration that corresponds to
// a parameter block declaration in the source program.
SIMPLE_SYNTAX_CLASS(ImplicitParameterGroupVariableModifier   , Modifier)

// Indicates that this is a type that corresponds to the element
// type of a parameter block declaration in the source program.
SIMPLE_SYNTAX_CLASS(ImplicitParameterGroupElementTypeModifier, Modifier)

// An HLSL semantic
ABSTRACT_SYNTAX_CLASS(HLSLSemantic, Modifier)
    FIELD(Token, name)
END_SYNTAX_CLASS()

// An HLSL semantic that affects layout
SYNTAX_CLASS(HLSLLayoutSemantic, HLSLSemantic)

    FIELD(Token, registerName)
    FIELD(Token, componentMask)
END_SYNTAX_CLASS()

// An HLSL `register` semantic
SYNTAX_CLASS(HLSLRegisterSemantic, HLSLLayoutSemantic)
    FIELD(Token, spaceName)
END_SYNTAX_CLASS()

// TODO(tfoley): `packoffset`
SIMPLE_SYNTAX_CLASS(HLSLPackOffsetSemantic, HLSLLayoutSemantic)

// An HLSL semantic that just associated a declaration with a semantic name
SIMPLE_SYNTAX_CLASS(HLSLSimpleSemantic, HLSLSemantic)

// GLSL

// Directives that came in via the preprocessor, but
// that we need to keep around for later steps
SIMPLE_SYNTAX_CLASS(GLSLPreprocessorDirective, Modifier)

// A GLSL `#version` directive
SYNTAX_CLASS(GLSLVersionDirective, GLSLPreprocessorDirective)

    // Token giving the version number to use
    FIELD(Token, versionNumberToken)

    // Optional token giving the sub-profile to be used
    FIELD(Token, glslProfileToken)
END_SYNTAX_CLASS()

// A GLSL `#extension` directive
SYNTAX_CLASS(GLSLExtensionDirective, GLSLPreprocessorDirective)

    // Token giving the version number to use
    FIELD(Token, extensionNameToken)

    // Optional token giving the sub-profile to be used
    FIELD(Token, dispositionToken)
END_SYNTAX_CLASS()

SYNTAX_CLASS(ParameterGroupReflectionName, Modifier)
    FIELD(NameLoc, nameAndLoc)
END_SYNTAX_CLASS()

// A modifier that indicates a built-in base type (e.g., `float`)
SYNTAX_CLASS(BuiltinTypeModifier, Modifier)
    FIELD(BaseType, tag)
END_SYNTAX_CLASS()

// A modifier that indicates a built-in type that isn't a base type (e.g., `vector`)
//
// TODO(tfoley): This deserves a better name than "magic"
SYNTAX_CLASS(MagicTypeModifier, Modifier)
    FIELD(String, name)
    FIELD(uint32_t, tag)
END_SYNTAX_CLASS()

// A modifier applied to declarations of builtin types to indicate how they
// should be lowered to the IR.
//
// TODO: This should really subsume `BuiltinTypeModifier` and
// `MagicTypeModifier` so that we don't have to apply all of them.
SYNTAX_CLASS(IntrinsicTypeModifier, Modifier)
    // The IR opcode to use when constructing a type
    FIELD(uint32_t, irOp)

    // Additional literal opreands to provide when creating instances.
    // (e.g., for a texture type this passes in shape/mutability info)
    FIELD(List<uint32_t>, irOperands)
END_SYNTAX_CLASS()

// Modifiers that affect the storage layout for matrices
SIMPLE_SYNTAX_CLASS(MatrixLayoutModifier, Modifier)

// Modifiers that specify row- and column-major layout, respectively
SIMPLE_SYNTAX_CLASS(RowMajorLayoutModifier, MatrixLayoutModifier)
SIMPLE_SYNTAX_CLASS(ColumnMajorLayoutModifier, MatrixLayoutModifier)

// The HLSL flavor of those modifiers
SIMPLE_SYNTAX_CLASS(HLSLRowMajorLayoutModifier, RowMajorLayoutModifier)
SIMPLE_SYNTAX_CLASS(HLSLColumnMajorLayoutModifier, ColumnMajorLayoutModifier)

// The GLSL flavor of those modifiers
//
// Note(tfoley): The GLSL versions of these modifiers are "backwards"
// in the sense that when a GLSL programmer requests row-major layout,
// we actually interpret that as requesting column-major. This makes
// sense because we interpret matrix conventions backwards from how
// GLSL specifies them.
SIMPLE_SYNTAX_CLASS(GLSLRowMajorLayoutModifier, ColumnMajorLayoutModifier)
SIMPLE_SYNTAX_CLASS(GLSLColumnMajorLayoutModifier, RowMajorLayoutModifier)

// More HLSL Keyword

ABSTRACT_SYNTAX_CLASS(InterpolationModeModifier, Modifier)
END_SYNTAX_CLASS()

// HLSL `nointerpolation` modifier
SIMPLE_SYNTAX_CLASS(HLSLNoInterpolationModifier, InterpolationModeModifier)

// HLSL `noperspective` modifier
SIMPLE_SYNTAX_CLASS(HLSLNoPerspectiveModifier, InterpolationModeModifier)

// HLSL `linear` modifier
SIMPLE_SYNTAX_CLASS(HLSLLinearModifier, InterpolationModeModifier)

// HLSL `sample` modifier
SIMPLE_SYNTAX_CLASS(HLSLSampleModifier, InterpolationModeModifier)

// HLSL `centroid` modifier
SIMPLE_SYNTAX_CLASS(HLSLCentroidModifier, InterpolationModeModifier)

// HLSL `precise` modifier
SIMPLE_SYNTAX_CLASS(PreciseModifier, Modifier)

// HLSL `shared` modifier (which is used by the effect system,
// and shouldn't be confused with `groupshared`)
SIMPLE_SYNTAX_CLASS(HLSLEffectSharedModifier, Modifier)

// HLSL `groupshared` modifier
SIMPLE_SYNTAX_CLASS(HLSLGroupSharedModifier, Modifier)

// HLSL `static` modifier (probably doesn't need to be
// treated as HLSL-specific)
SIMPLE_SYNTAX_CLASS(HLSLStaticModifier, Modifier)

// HLSL `uniform` modifier (distinct meaning from GLSL
// use of the keyword)
SIMPLE_SYNTAX_CLASS(HLSLUniformModifier, Modifier)

// HLSL `volatile` modifier (ignored)
SIMPLE_SYNTAX_CLASS(HLSLVolatileModifier, Modifier)

SYNTAX_CLASS(AttributeTargetModifier, Modifier)
    // A class to which the declared attribute type is applicable
    FIELD(SyntaxClass<RefObject>, syntaxClass)
END_SYNTAX_CLASS()

// Base class for checked and unchecked `[name(arg0, ...)]` style attribute.
SYNTAX_CLASS(AttributeBase, Modifier)
    SYNTAX_FIELD(List<RefPtr<Expr>>, args)
END_SYNTAX_CLASS()

// A `[name(...)]` attribute that hasn't undergone any semantic analysis.
// After analysis, this will be transformed into a more specific case.
SYNTAX_CLASS(UncheckedAttribute, AttributeBase)
    FIELD(RefPtr<Scope>, scope)
END_SYNTAX_CLASS()

// A `[name(arg0, ...)]` style attribute that has been validated.
SYNTAX_CLASS(Attribute, AttributeBase)
    FIELD(AttributeArgumentValueDict, intArgVals)
END_SYNTAX_CLASS()

SYNTAX_CLASS(UserDefinedAttribute, Attribute)
END_SYNTAX_CLASS()

SYNTAX_CLASS(AttributeUsageAttribute, Attribute)
    FIELD(SyntaxClass<RefObject>, targetSyntaxClass)
END_SYNTAX_CLASS()

// An `[unroll]` or `[unroll(count)]` attribute
SYNTAX_CLASS(UnrollAttribute, Attribute)
    RAW(IntegerLiteralValue getCount();)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(LoopAttribute, Attribute)               // `[loop]`
SIMPLE_SYNTAX_CLASS(FastOptAttribute, Attribute)            // `[fastopt]`
SIMPLE_SYNTAX_CLASS(AllowUAVConditionAttribute, Attribute)  // `[allow_uav_condition]`
SIMPLE_SYNTAX_CLASS(BranchAttribute, Attribute)             // `[branch]`
SIMPLE_SYNTAX_CLASS(FlattenAttribute, Attribute)            // `[flatten]`
SIMPLE_SYNTAX_CLASS(ForceCaseAttribute, Attribute)          // `[forcecase]`
SIMPLE_SYNTAX_CLASS(CallAttribute, Attribute)               // `[call]`


// [[vk_push_constant]] [[push_constant]]
SIMPLE_SYNTAX_CLASS(PushConstantAttribute, Attribute)

// [[vk_shader_record]] [[shader_record]]
SIMPLE_SYNTAX_CLASS(ShaderRecordAttribute, Attribute)

// [[vk_binding]]
SYNTAX_CLASS(GLSLBindingAttribute, Attribute)
    FIELD(int32_t, binding = 0)
    FIELD(int32_t, set = 0)
END_SYNTAX_CLASS()

// TODO: for attributes that take arguments, the syntax node
// classes should provide accessors for the values of those arguments.

SIMPLE_SYNTAX_CLASS(MaxTessFactorAttribute, Attribute)
SIMPLE_SYNTAX_CLASS(OutputControlPointsAttribute, Attribute)
SIMPLE_SYNTAX_CLASS(OutputTopologyAttribute, Attribute)
SIMPLE_SYNTAX_CLASS(PartitioningAttribute, Attribute)
SYNTAX_CLASS(PatchConstantFuncAttribute, Attribute)
    FIELD(RefPtr<FuncDecl>, patchConstantFuncDecl)
END_SYNTAX_CLASS()
SIMPLE_SYNTAX_CLASS(DomainAttribute, Attribute)

SIMPLE_SYNTAX_CLASS(EarlyDepthStencilAttribute, Attribute)  // `[earlydepthstencil]`

// An HLSL `[numthreads(x,y,z)]` attribute
SYNTAX_CLASS(NumThreadsAttribute, Attribute)
    // The number of threads to use along each axis
    //
    // TODO: These should be accessors that use the
    // ordinary `args` list, rather than side data.
    FIELD(int32_t, x)
    FIELD(int32_t, y)
    FIELD(int32_t, z)
END_SYNTAX_CLASS()

SYNTAX_CLASS(MaxVertexCountAttribute, Attribute)
    // The number of max vertex count for geometry shader
    //
    // TODO: This should be an accessor that uses the
    // ordinary `args` list, rather than side data.
    FIELD(int32_t, value)
END_SYNTAX_CLASS()

SYNTAX_CLASS(InstanceAttribute, Attribute)
    // The number of instances to run for geometry shader
    //
    // TODO: This should be an accessor that uses the
    // ordinary `args` list, rather than side data.
    FIELD(int32_t, value)
END_SYNTAX_CLASS()

// A `[shader("stageName")]` attribute, which marks an entry point
// to be compiled, and specifies the stage for that entry point
SYNTAX_CLASS(EntryPointAttribute, Attribute)
    // The resolved stage that the entry point is targetting.
    //
    // TODO: This should be an accessor that uses the
    // ordinary `args` list, rather than side data.
    FIELD(Stage, stage);
END_SYNTAX_CLASS()

// A `[__vulkanRayPayload]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the input/output interface for a Vulkan
// ray tracing shader to pass per-ray payload information.
SIMPLE_SYNTAX_CLASS(VulkanRayPayloadAttribute, Attribute)

// A `[__vulkanCallablePayload]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the input/output interface for a Vulkan
// ray tracing shader to pass payload information to/from a callee.
SIMPLE_SYNTAX_CLASS(VulkanCallablePayloadAttribute, Attribute)

// A `[__vulkanHitAttributes]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the output interface for a Vulkan
// intersection shader to pass hit attribute information.
SIMPLE_SYNTAX_CLASS(VulkanHitAttributesAttribute, Attribute)

// A `[mutating]` attribute, which indicates that a member
// function is allowed to modify things through its `this`
// argument.
//
SIMPLE_SYNTAX_CLASS(MutatingAttribute, Attribute)

// A `[__readNone]` attribute, which indicates that a function
// computes its results strictly based on argument values, without
// reading or writing through any pointer arguments, or any other
// state that could be observed by a caller.
//
SIMPLE_SYNTAX_CLASS(ReadNoneAttribute, Attribute)


// HLSL modifiers for geometry shader input topology
SIMPLE_SYNTAX_CLASS(HLSLGeometryShaderInputPrimitiveTypeModifier, Modifier)
SIMPLE_SYNTAX_CLASS(HLSLPointModifier        , HLSLGeometryShaderInputPrimitiveTypeModifier)
SIMPLE_SYNTAX_CLASS(HLSLLineModifier         , HLSLGeometryShaderInputPrimitiveTypeModifier)
SIMPLE_SYNTAX_CLASS(HLSLTriangleModifier     , HLSLGeometryShaderInputPrimitiveTypeModifier)
SIMPLE_SYNTAX_CLASS(HLSLLineAdjModifier      , HLSLGeometryShaderInputPrimitiveTypeModifier)
SIMPLE_SYNTAX_CLASS(HLSLTriangleAdjModifier  , HLSLGeometryShaderInputPrimitiveTypeModifier)

// A modifier to be attached to syntax after we've computed layout
SYNTAX_CLASS(ComputedLayoutModifier, Modifier)
    FIELD(RefPtr<Layout>, layout)
END_SYNTAX_CLASS()


SYNTAX_CLASS(TupleVarModifier, Modifier)
//    FIELD_INIT(TupleFieldModifier*, tupleField, nullptr)
END_SYNTAX_CLASS()

// A modifier to indicate that a constructor/initializer can be used
// to perform implicit type conversion, and to specify the cost of
// the conversion, if applied.
SYNTAX_CLASS(ImplicitConversionModifier, Modifier)
    // The conversion cost, used to rank conversions
    FIELD(ConversionCost, cost)
END_SYNTAX_CLASS()

SYNTAX_CLASS(FormatAttribute, Attribute)
    FIELD(ImageFormat, format)
END_SYNTAX_CLASS()

SYNTAX_CLASS(AllowAttribute, Attribute)
    FIELD_INIT(DiagnosticInfo const*, diagnostic, nullptr)
END_SYNTAX_CLASS()
