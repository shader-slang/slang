// modifier-defs.h

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

#undef SIMPLE_MODIFIER

// Base class for modifiers that mark something as "intrinsic"
// and thus lacking a direct implementation in the language.
ABSTRACT_SYNTAX_CLASS(IntrinsicModifierBase, Modifier)
END_SYNTAX_CLASS()

// A modifier that marks something as one of a small set of
// truly intrinsic operations that the compiler knows about
// directly.
SYNTAX_CLASS(IntrinsicOpModifier, IntrinsicModifierBase)

    // token that names the intrinsic op
    FIELD(Token, opToken)

    // The opcode for the intrinsic operation
    FIELD_INIT(IntrinsicOp, op, IntrinsicOp::Unknown)
END_SYNTAX_CLASS()

// A modifier that marks something as an intrinsic function,
// for some subset of targets.
SYNTAX_CLASS(TargetIntrinsicModifier, IntrinsicModifierBase)
    // Token that names the target that the operation
    // is an intrisic for.
    FIELD(Token, targetToken)

    // A custom definition for the operation
    FIELD(Token, definitionToken)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(InOutModifier, OutModifier)

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

// We divide GLSL `layout` modifiers into those we have parsed
// (in the sense of having some notion of their semantics), and
// those we have not.
ABSTRACT_SYNTAX_CLASS(GLSLParsedLayoutModifier     , GLSLLayoutModifier)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(GLSLUnparsedLayoutModifier   , GLSLLayoutModifier)

// Specific cases for known GLSL `layout` modifiers that we need to work with
SIMPLE_SYNTAX_CLASS(GLSLConstantIDLayoutModifier , GLSLParsedLayoutModifier)
SIMPLE_SYNTAX_CLASS(GLSLBindingLayoutModifier    , GLSLParsedLayoutModifier)
SIMPLE_SYNTAX_CLASS(GLSLSetLayoutModifier        , GLSLParsedLayoutModifier)
SIMPLE_SYNTAX_CLASS(GLSLLocationLayoutModifier   , GLSLParsedLayoutModifier)

// A catch-all for single-keyword modifiers
SIMPLE_SYNTAX_CLASS(SimpleModifier, Modifier)

// Some GLSL-specific modifiers
SIMPLE_SYNTAX_CLASS(GLSLBufferModifier   , SimpleModifier)
SIMPLE_SYNTAX_CLASS(GLSLWriteOnlyModifier, SimpleModifier)
SIMPLE_SYNTAX_CLASS(GLSLReadOnlyModifier , SimpleModifier)
SIMPLE_SYNTAX_CLASS(GLSLPatchModifier    , SimpleModifier)

// Indicates that this is a variable declaration that corresponds to
// a parameter block declaration in the source program.
SIMPLE_SYNTAX_CLASS(ImplicitParameterBlockVariableModifier   , Modifier)

// Indicates that this is a type that corresponds to the element
// type of a parameter block declaration in the source program.
SIMPLE_SYNTAX_CLASS(ImplicitParameterBlockElementTypeModifier, Modifier)

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
SIMPLE_SYNTAX_CLASS(HLSLRegisterSemantic, HLSLLayoutSemantic)

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

SYNTAX_CLASS(ParameterBlockReflectionName, Modifier)
    FIELD(Token, nameToken)
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

// HLSL `nointerpolation` modifier
SIMPLE_SYNTAX_CLASS(HLSLNoInterpolationModifier, Modifier)

// HLSL `linear` modifier
SIMPLE_SYNTAX_CLASS(HLSLLinearModifier, Modifier)

// HLSL `sample` modifier
SIMPLE_SYNTAX_CLASS(HLSLSampleModifier, Modifier)

// HLSL `centroid` modifier
SIMPLE_SYNTAX_CLASS(HLSLCentroidModifier, Modifier)

// HLSL `precise` modifier
SIMPLE_SYNTAX_CLASS(HLSLPreciseModifier, Modifier)

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

// An HLSL `[name(arg0, ...)]` style attribute.
SYNTAX_CLASS(HLSLAttribute, Modifier)
    FIELD(Token, nameToken)
    SYNTAX_FIELD(List<RefPtr<ExpressionSyntaxNode>>, args)
END_SYNTAX_CLASS()

// An HLSL `[name(...)]` attribute that hasn't undergone
// any semantic analysis.
// After analysis, this might be transformed into a more specific case.
SIMPLE_SYNTAX_CLASS(HLSLUncheckedAttribute, HLSLAttribute)

// An HLSL `[numthreads(x,y,z)]` attribute
SYNTAX_CLASS(HLSLNumThreadsAttribute, HLSLAttribute)
    // The number of threads to use along each axis
    FIELD(int32_t, x)
    FIELD(int32_t, y)
    FIELD(int32_t, z)
END_SYNTAX_CLASS()

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

// A modifier attached to types during lowering, to indicate that they
// are logically a "tuple" type
SYNTAX_CLASS(TupleTypeModifier, Modifier)
    FIELD_INIT(AggTypeDecl*, decl, nullptr)
    FIELD_INIT(bool, hasAnyNonTupleFields, false)
END_SYNTAX_CLASS()

SYNTAX_CLASS(TupleFieldModifier, Modifier)
    FIELD_INIT(VarDeclBase*, decl, nullptr)
    FIELD_INIT(bool, hasAnyNonTupleFields, false)
    FIELD_INIT(bool, isNestedTuple, false)
END_SYNTAX_CLASS()

SYNTAX_CLASS(TupleVarModifier, Modifier)
    FIELD_INIT(TupleFieldModifier*, tupleField, nullptr)
END_SYNTAX_CLASS()
