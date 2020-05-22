// slang-ast-modifier.h

#pragma once

#include "slang-ast-base.h"

namespace Slang { 

// Syntax class definitions for modifiers.

// Simple modifiers have no state beyond their identity

class InModifier : public Modifier { SLANG_CLASS(InModifier)};
class OutModifier : public Modifier { SLANG_CLASS(OutModifier)};
class ConstModifier : public Modifier { SLANG_CLASS(ConstModifier)};
class InstanceModifier : public Modifier { SLANG_CLASS(InstanceModifier)};
class BuiltinModifier : public Modifier { SLANG_CLASS(BuiltinModifier)};
class InlineModifier : public Modifier { SLANG_CLASS(InlineModifier)};
class PublicModifier : public Modifier { SLANG_CLASS(PublicModifier)};
class RequireModifier : public Modifier { SLANG_CLASS(RequireModifier)};
class ParamModifier : public Modifier { SLANG_CLASS(ParamModifier)};
class ExternModifier : public Modifier { SLANG_CLASS(ExternModifier)};
class InputModifier : public Modifier { SLANG_CLASS(InputModifier)};
class TransparentModifier : public Modifier { SLANG_CLASS(TransparentModifier)};
class FromStdLibModifier : public Modifier { SLANG_CLASS(FromStdLibModifier)};
class PrefixModifier : public Modifier { SLANG_CLASS(PrefixModifier)};
class PostfixModifier : public Modifier { SLANG_CLASS(PostfixModifier)};
class ExportedModifier : public Modifier { SLANG_CLASS(ExportedModifier)};
class ConstExprModifier : public Modifier { SLANG_CLASS(ConstExprModifier)};
class GloballyCoherentModifier : public Modifier { SLANG_CLASS(GloballyCoherentModifier)};

// A modifier that marks something as an operation that
// has a one-to-one translation to the IR, and thus
// has no direct definition in the high-level language.
//
class IntrinsicOpModifier : public Modifier 
{
    SLANG_CLASS(IntrinsicOpModifier)
 
    // Token that names the intrinsic op.
    Token opToken;

    // The IR opcode for the intrinsic operation.
    //
    uint32_t op = 0;
};

// A modifier that marks something as an intrinsic function,
// for some subset of targets.
class TargetIntrinsicModifier : public Modifier 
{
    SLANG_CLASS(TargetIntrinsicModifier)
 
    // Token that names the target that the operation
    // is an intrisic for.
    Token targetToken;

    // A custom definition for the operation
    Token definitionToken;
};

// A modifier that marks a declaration as representing a
// specialization that should be preferred on a particular
// target.
class SpecializedForTargetModifier : public Modifier 
{
    SLANG_CLASS(SpecializedForTargetModifier)
 
    // Token that names the target that the operation
    // has been specialized for.
    Token targetToken;
};

// A modifier to tag something as an intrinsic that requires
// a certain GLSL extension to be enabled when used
class RequiredGLSLExtensionModifier : public Modifier 
{
    SLANG_CLASS(RequiredGLSLExtensionModifier)
 
    Token extensionNameToken;
};

// A modifier to tag something as an intrinsic that requires
// a certain GLSL version to be enabled when used
class RequiredGLSLVersionModifier : public Modifier 
{
    SLANG_CLASS(RequiredGLSLVersionModifier)
 
    Token versionNumberToken;
};


// A modifier to tag something as an intrinsic that requires
// a certain SPIRV version to be enabled when used. Specified as "major.minor"
class RequiredSPIRVVersionModifier : public Modifier 
{
    SLANG_CLASS(RequiredSPIRVVersionModifier)
 
    SemanticVersion version;
};

// A modifier to tag something as an intrinsic that requires
// a certain CUDA SM version to be enabled when used. Specified as "major.minor"
class RequiredCUDASMVersionModifier : public Modifier 
{
    SLANG_CLASS(RequiredCUDASMVersionModifier)
 
    SemanticVersion version;
};

class InOutModifier : public OutModifier 
{
    SLANG_CLASS(InOutModifier)
};


// `__ref` modifier for by-reference parameter passing
class RefModifier : public Modifier 
{
    SLANG_CLASS(RefModifier)
};


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
class SharedModifiers : public Modifier 
{
    SLANG_CLASS(SharedModifiers)
};


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
class GLSLLayoutModifier : public Modifier 
{
    SLANG_ABSTRACT_CLASS(GLSLLayoutModifier)
 

    // The token used to introduce the modifier is stored
    // as the `nameToken` field.

    // TODO: may want to accept a full expression here
    Token valToken;
};

// AST nodes to represent the begin/end of a `layout` modifier group
class GLSLLayoutModifierGroupMarker : public Modifier 
{
    SLANG_ABSTRACT_CLASS(GLSLLayoutModifierGroupMarker)
};

class GLSLLayoutModifierGroupBegin : public GLSLLayoutModifierGroupMarker 
{
    SLANG_CLASS(GLSLLayoutModifierGroupBegin)
};

class GLSLLayoutModifierGroupEnd : public GLSLLayoutModifierGroupMarker 
{
    SLANG_CLASS(GLSLLayoutModifierGroupEnd)
};


// We divide GLSL `layout` modifiers into those we have parsed
// (in the sense of having some notion of their semantics), and
// those we have not.
class GLSLParsedLayoutModifier : public GLSLLayoutModifier 
{
    SLANG_ABSTRACT_CLASS(GLSLParsedLayoutModifier)
};

class GLSLUnparsedLayoutModifier : public GLSLLayoutModifier 
{
    SLANG_CLASS(GLSLUnparsedLayoutModifier)
};


// Specific cases for known GLSL `layout` modifiers that we need to work with
class GLSLConstantIDLayoutModifier : public GLSLParsedLayoutModifier 
{
    SLANG_CLASS(GLSLConstantIDLayoutModifier)
};

class GLSLLocationLayoutModifier : public GLSLParsedLayoutModifier 
{
    SLANG_CLASS(GLSLLocationLayoutModifier)
};


// A catch-all for single-keyword modifiers
class SimpleModifier : public Modifier 
{
    SLANG_CLASS(SimpleModifier)
};


// Some GLSL-specific modifiers
class GLSLBufferModifier : public SimpleModifier 
{
    SLANG_CLASS(GLSLBufferModifier)
};

class GLSLWriteOnlyModifier : public SimpleModifier 
{
    SLANG_CLASS(GLSLWriteOnlyModifier)
};

class GLSLReadOnlyModifier : public SimpleModifier 
{
    SLANG_CLASS(GLSLReadOnlyModifier)
};

class GLSLPatchModifier : public SimpleModifier 
{
    SLANG_CLASS(GLSLPatchModifier)
};


// Indicates that this is a variable declaration that corresponds to
// a parameter block declaration in the source program.
class ImplicitParameterGroupVariableModifier : public Modifier 
{
    SLANG_CLASS(ImplicitParameterGroupVariableModifier)
};


// Indicates that this is a type that corresponds to the element
// type of a parameter block declaration in the source program.
class ImplicitParameterGroupElementTypeModifier : public Modifier 
{
    SLANG_CLASS(ImplicitParameterGroupElementTypeModifier)
};


// An HLSL semantic
class HLSLSemantic : public Modifier 
{
    SLANG_ABSTRACT_CLASS(HLSLSemantic)
 
    Token name;
};

// An HLSL semantic that affects layout
class HLSLLayoutSemantic : public HLSLSemantic 
{
    SLANG_CLASS(HLSLLayoutSemantic)
 
    Token registerName;
    Token componentMask;
};

// An HLSL `register` semantic
class HLSLRegisterSemantic : public HLSLLayoutSemantic 
{
    SLANG_CLASS(HLSLRegisterSemantic)
 
    Token spaceName;
};

// TODO(tfoley): `packoffset`
class HLSLPackOffsetSemantic : public HLSLLayoutSemantic 
{
    SLANG_CLASS(HLSLPackOffsetSemantic)
};


// An HLSL semantic that just associated a declaration with a semantic name
class HLSLSimpleSemantic : public HLSLSemantic 
{
    SLANG_CLASS(HLSLSimpleSemantic)
};


// GLSL

// Directives that came in via the preprocessor, but
// that we need to keep around for later steps
class GLSLPreprocessorDirective : public Modifier 
{
    SLANG_CLASS(GLSLPreprocessorDirective)
};


// A GLSL `#version` directive
class GLSLVersionDirective : public GLSLPreprocessorDirective 
{
    SLANG_CLASS(GLSLVersionDirective)
 

    // Token giving the version number to use
    Token versionNumberToken;

    // Optional token giving the sub-profile to be used
    Token glslProfileToken;
};

// A GLSL `#extension` directive
class GLSLExtensionDirective : public GLSLPreprocessorDirective 
{
    SLANG_CLASS(GLSLExtensionDirective)
 

    // Token giving the version number to use
    Token extensionNameToken;

    // Optional token giving the sub-profile to be used
    Token dispositionToken;
};

class ParameterGroupReflectionName : public Modifier 
{
    SLANG_CLASS(ParameterGroupReflectionName)
 
    NameLoc nameAndLoc;
};

// A modifier that indicates a built-in base type (e.g., `float`)
class BuiltinTypeModifier : public Modifier 
{
    SLANG_CLASS(BuiltinTypeModifier)
 
    BaseType tag;
};

// A modifier that indicates a built-in type that isn't a base type (e.g., `vector`)
//
// TODO(tfoley): This deserves a better name than "magic"
class MagicTypeModifier : public Modifier 
{
    SLANG_CLASS(MagicTypeModifier)
 
    String name;
    uint32_t tag;
};

// A modifier applied to declarations of builtin types to indicate how they
// should be lowered to the IR.
//
// TODO: This should really subsume `BuiltinTypeModifier` and
// `MagicTypeModifier` so that we don't have to apply all of them.
class IntrinsicTypeModifier : public Modifier 
{
    SLANG_CLASS(IntrinsicTypeModifier)
 
    // The IR opcode to use when constructing a type
    uint32_t irOp;

    // Additional literal opreands to provide when creating instances.
    // (e.g., for a texture type this passes in shape/mutability info)
    List<uint32_t> irOperands;
};

// Modifiers that affect the storage layout for matrices
class MatrixLayoutModifier : public Modifier 
{
    SLANG_CLASS(MatrixLayoutModifier)
};


// Modifiers that specify row- and column-major layout, respectively
class RowMajorLayoutModifier : public MatrixLayoutModifier 
{
    SLANG_CLASS(RowMajorLayoutModifier)
};

class ColumnMajorLayoutModifier : public MatrixLayoutModifier 
{
    SLANG_CLASS(ColumnMajorLayoutModifier)
};


// The HLSL flavor of those modifiers
class HLSLRowMajorLayoutModifier : public RowMajorLayoutModifier 
{
    SLANG_CLASS(HLSLRowMajorLayoutModifier)
};

class HLSLColumnMajorLayoutModifier : public ColumnMajorLayoutModifier 
{
    SLANG_CLASS(HLSLColumnMajorLayoutModifier)
};


// The GLSL flavor of those modifiers
//
// Note(tfoley): The GLSL versions of these modifiers are "backwards"
// in the sense that when a GLSL programmer requests row-major layout,
// we actually interpret that as requesting column-major. This makes
// sense because we interpret matrix conventions backwards from how
// GLSL specifies them.
class GLSLRowMajorLayoutModifier : public ColumnMajorLayoutModifier 
{
    SLANG_CLASS(GLSLRowMajorLayoutModifier)
};

class GLSLColumnMajorLayoutModifier : public RowMajorLayoutModifier 
{
    SLANG_CLASS(GLSLColumnMajorLayoutModifier)
};


// More HLSL Keyword

class InterpolationModeModifier : public Modifier 
{
    SLANG_ABSTRACT_CLASS(InterpolationModeModifier)
 
};

// HLSL `nointerpolation` modifier
class HLSLNoInterpolationModifier : public InterpolationModeModifier 
{
    SLANG_CLASS(HLSLNoInterpolationModifier)
};


// HLSL `noperspective` modifier
class HLSLNoPerspectiveModifier : public InterpolationModeModifier 
{
    SLANG_CLASS(HLSLNoPerspectiveModifier)
};


// HLSL `linear` modifier
class HLSLLinearModifier : public InterpolationModeModifier 
{
    SLANG_CLASS(HLSLLinearModifier)
};


// HLSL `sample` modifier
class HLSLSampleModifier : public InterpolationModeModifier 
{
    SLANG_CLASS(HLSLSampleModifier)
};


// HLSL `centroid` modifier
class HLSLCentroidModifier : public InterpolationModeModifier 
{
    SLANG_CLASS(HLSLCentroidModifier)
};


// HLSL `precise` modifier
class PreciseModifier : public Modifier 
{
    SLANG_CLASS(PreciseModifier)
};


// HLSL `shared` modifier (which is used by the effect system,
// and shouldn't be confused with `groupshared`)
class HLSLEffectSharedModifier : public Modifier 
{
    SLANG_CLASS(HLSLEffectSharedModifier)
};


// HLSL `groupshared` modifier
class HLSLGroupSharedModifier : public Modifier 
{
    SLANG_CLASS(HLSLGroupSharedModifier)
};


// HLSL `static` modifier (probably doesn't need to be
// treated as HLSL-specific)
class HLSLStaticModifier : public Modifier 
{
    SLANG_CLASS(HLSLStaticModifier)
};


// HLSL `uniform` modifier (distinct meaning from GLSL
// use of the keyword)
class HLSLUniformModifier : public Modifier 
{
    SLANG_CLASS(HLSLUniformModifier)
};


// HLSL `volatile` modifier (ignored)
class HLSLVolatileModifier : public Modifier 
{
    SLANG_CLASS(HLSLVolatileModifier)
};


class AttributeTargetModifier : public Modifier 
{
    SLANG_CLASS(AttributeTargetModifier)
 
    // A class to which the declared attribute type is applicable
    SyntaxClass<RefObject> syntaxClass;
};

// Base class for checked and unchecked `[name(arg0, ...)]` style attribute.
class AttributeBase : public Modifier 
{
    SLANG_CLASS(AttributeBase)
 
    List<RefPtr<Expr>> args;
};

// A `[name(...)]` attribute that hasn't undergone any semantic analysis.
// After analysis, this will be transformed into a more specific case.
class UncheckedAttribute : public AttributeBase 
{
    SLANG_CLASS(UncheckedAttribute)
 
    RefPtr<Scope> scope;
};

// A `[name(arg0, ...)]` style attribute that has been validated.
class Attribute : public AttributeBase 
{
    SLANG_CLASS(Attribute)
 
    AttributeArgumentValueDict intArgVals;
};

class UserDefinedAttribute : public Attribute 
{
    SLANG_CLASS(UserDefinedAttribute)
 
};

class AttributeUsageAttribute : public Attribute 
{
    SLANG_CLASS(AttributeUsageAttribute)
 
    SyntaxClass<RefObject> targetSyntaxClass;
};

// An `[unroll]` or `[unroll(count)]` attribute
class UnrollAttribute : public Attribute 
{
    SLANG_CLASS(UnrollAttribute)
 
    IntegerLiteralValue getCount();
};

class LoopAttribute : public Attribute 
{
    SLANG_CLASS(LoopAttribute)
};
               // `[loop]`
class FastOptAttribute : public Attribute 
{
    SLANG_CLASS(FastOptAttribute)
};
            // `[fastopt]`
class AllowUAVConditionAttribute : public Attribute 
{
    SLANG_CLASS(AllowUAVConditionAttribute)
};
  // `[allow_uav_condition]`
class BranchAttribute : public Attribute 
{
    SLANG_CLASS(BranchAttribute)
};
             // `[branch]`
class FlattenAttribute : public Attribute 
{
    SLANG_CLASS(FlattenAttribute)
};
            // `[flatten]`
class ForceCaseAttribute : public Attribute 
{
    SLANG_CLASS(ForceCaseAttribute)
};
          // `[forcecase]`
class CallAttribute : public Attribute 
{
    SLANG_CLASS(CallAttribute)
};
               // `[call]`


// [[vk_push_constant]] [[push_constant]]
class PushConstantAttribute : public Attribute 
{
    SLANG_CLASS(PushConstantAttribute)
};


// [[vk_shader_record]] [[shader_record]]
class ShaderRecordAttribute : public Attribute 
{
    SLANG_CLASS(ShaderRecordAttribute)
};


// [[vk_binding]]
class GLSLBindingAttribute : public Attribute 
{
    SLANG_CLASS(GLSLBindingAttribute)
 
    int32_t binding = 0;
    int32_t set = 0;
};

class GLSLSimpleIntegerLayoutAttribute : public Attribute 
{
    SLANG_CLASS(GLSLSimpleIntegerLayoutAttribute)
 
    int32_t value = 0;
};

// [[vk_location]]
class GLSLLocationAttribute : public GLSLSimpleIntegerLayoutAttribute 
{
    SLANG_CLASS(GLSLLocationAttribute)
};


// [[vk_index]]
class GLSLIndexAttribute : public GLSLSimpleIntegerLayoutAttribute 
{
    SLANG_CLASS(GLSLIndexAttribute)
};


// TODO: for attributes that take arguments, the syntax node
// classes should provide accessors for the values of those arguments.

class MaxTessFactorAttribute : public Attribute 
{
    SLANG_CLASS(MaxTessFactorAttribute)
};

class OutputControlPointsAttribute : public Attribute 
{
    SLANG_CLASS(OutputControlPointsAttribute)
};

class OutputTopologyAttribute : public Attribute 
{
    SLANG_CLASS(OutputTopologyAttribute)
};

class PartitioningAttribute : public Attribute 
{
    SLANG_CLASS(PartitioningAttribute)
};

class PatchConstantFuncAttribute : public Attribute 
{
    SLANG_CLASS(PatchConstantFuncAttribute)
 
    RefPtr<FuncDecl> patchConstantFuncDecl;
};
class DomainAttribute : public Attribute 
{
    SLANG_CLASS(DomainAttribute)
};


class EarlyDepthStencilAttribute : public Attribute 
{
    SLANG_CLASS(EarlyDepthStencilAttribute)
};
  // `[earlydepthstencil]`

// An HLSL `[numthreads(x,y,z)]` attribute
class NumThreadsAttribute : public Attribute 
{
    SLANG_CLASS(NumThreadsAttribute)
 
    // The number of threads to use along each axis
    //
    // TODO: These should be accessors that use the
    // ordinary `args` list, rather than side data.
    int32_t x;
    int32_t y;
    int32_t z;
};

class MaxVertexCountAttribute : public Attribute 
{
    SLANG_CLASS(MaxVertexCountAttribute)
 
    // The number of max vertex count for geometry shader
    //
    // TODO: This should be an accessor that uses the
    // ordinary `args` list, rather than side data.
    int32_t value;
};

class InstanceAttribute : public Attribute 
{
    SLANG_CLASS(InstanceAttribute)
 
    // The number of instances to run for geometry shader
    //
    // TODO: This should be an accessor that uses the
    // ordinary `args` list, rather than side data.
    int32_t value;
};

// A `[shader("stageName")]` attribute, which marks an entry point
// to be compiled, and specifies the stage for that entry point
class EntryPointAttribute : public Attribute 
{
    SLANG_CLASS(EntryPointAttribute)
 
    // The resolved stage that the entry point is targetting.
    //
    // TODO: This should be an accessor that uses the
    // ordinary `args` list, rather than side data.
    Stage stage;
};

// A `[__vulkanRayPayload]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the input/output interface for a Vulkan
// ray tracing shader to pass per-ray payload information.
class VulkanRayPayloadAttribute : public Attribute 
{
    SLANG_CLASS(VulkanRayPayloadAttribute)
};


// A `[__vulkanCallablePayload]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the input/output interface for a Vulkan
// ray tracing shader to pass payload information to/from a callee.
class VulkanCallablePayloadAttribute : public Attribute 
{
    SLANG_CLASS(VulkanCallablePayloadAttribute)
};


// A `[__vulkanHitAttributes]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the output interface for a Vulkan
// intersection shader to pass hit attribute information.
class VulkanHitAttributesAttribute : public Attribute 
{
    SLANG_CLASS(VulkanHitAttributesAttribute)
};


// A `[mutating]` attribute, which indicates that a member
// function is allowed to modify things through its `this`
// argument.
//
class MutatingAttribute : public Attribute 
{
    SLANG_CLASS(MutatingAttribute)
};


// A `[__readNone]` attribute, which indicates that a function
// computes its results strictly based on argument values, without
// reading or writing through any pointer arguments, or any other
// state that could be observed by a caller.
//
class ReadNoneAttribute : public Attribute 
{
    SLANG_CLASS(ReadNoneAttribute)
};



// HLSL modifiers for geometry shader input topology
class HLSLGeometryShaderInputPrimitiveTypeModifier : public Modifier 
{
    SLANG_CLASS(HLSLGeometryShaderInputPrimitiveTypeModifier)
};

class HLSLPointModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_CLASS(HLSLPointModifier)
};

class HLSLLineModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_CLASS(HLSLLineModifier)
};

class HLSLTriangleModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_CLASS(HLSLTriangleModifier)
};

class HLSLLineAdjModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_CLASS(HLSLLineAdjModifier)
};

class HLSLTriangleAdjModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_CLASS(HLSLTriangleAdjModifier)
};


// A modifier to be attached to syntax after we've computed layout
class ComputedLayoutModifier : public Modifier 
{
    SLANG_CLASS(ComputedLayoutModifier)
 
    RefPtr<Layout> layout;
};


class TupleVarModifier : public Modifier 
{
    SLANG_CLASS(TupleVarModifier)
 
//  TupleFieldModifier* tupleField = nullptr;
};

// A modifier to indicate that a constructor/initializer can be used
// to perform implicit type conversion, and to specify the cost of
// the conversion, if applied.
class ImplicitConversionModifier : public Modifier 
{
    SLANG_CLASS(ImplicitConversionModifier)
 
    // The conversion cost, used to rank conversions
    ConversionCost cost;
};

class FormatAttribute : public Attribute 
{
    SLANG_CLASS(FormatAttribute)
 
    ImageFormat format;
};

class AllowAttribute : public Attribute 
{
    SLANG_CLASS(AllowAttribute)
 
    DiagnosticInfo const* diagnostic = nullptr;
};


// A `[__extern]` attribute, which indicates that a function/type is defined externally
//
class ExternAttribute : public Attribute 
{
    SLANG_CLASS(ExternAttribute)
};


// An `[__unsafeForceInlineExternal]` attribute indicates that the callee should be inlined
// into call sites after initial IR generation (that is, as early as possible).
//
class UnsafeForceInlineEarlyAttribute : public Attribute 
{
    SLANG_CLASS(UnsafeForceInlineEarlyAttribute)
};

} // namespace Slang
