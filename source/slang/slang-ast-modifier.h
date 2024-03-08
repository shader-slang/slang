// slang-ast-modifier.h

#pragma once

#include "slang-ast-base.h"

namespace Slang { 

// Syntax class definitions for modifiers.

// Simple modifiers have no state beyond their identity

class InModifier : public Modifier { SLANG_AST_CLASS(InModifier)};
class OutModifier : public Modifier { SLANG_AST_CLASS(OutModifier)};
class ConstModifier : public Modifier { SLANG_AST_CLASS(ConstModifier)};
class BuiltinModifier : public Modifier { SLANG_AST_CLASS(BuiltinModifier)};
class InlineModifier : public Modifier { SLANG_AST_CLASS(InlineModifier)};
class VisibilityModifier : public Modifier {SLANG_AST_CLASS(VisibilityModifier)};
class PublicModifier : public VisibilityModifier { SLANG_AST_CLASS(PublicModifier)};
class PrivateModifier : public VisibilityModifier { SLANG_AST_CLASS(PrivateModifier) };
class InternalModifier : public VisibilityModifier { SLANG_AST_CLASS(InternalModifier) };
class RequireModifier : public Modifier { SLANG_AST_CLASS(RequireModifier)};
class ParamModifier : public Modifier { SLANG_AST_CLASS(ParamModifier)};
class ExternModifier : public Modifier { SLANG_AST_CLASS(ExternModifier)};
class HLSLExportModifier : public Modifier { SLANG_AST_CLASS(HLSLExportModifier) };
class TransparentModifier : public Modifier { SLANG_AST_CLASS(TransparentModifier)};
class FromStdLibModifier : public Modifier { SLANG_AST_CLASS(FromStdLibModifier)};
class PrefixModifier : public Modifier { SLANG_AST_CLASS(PrefixModifier)};
class PostfixModifier : public Modifier { SLANG_AST_CLASS(PostfixModifier)};
class ExportedModifier : public Modifier { SLANG_AST_CLASS(ExportedModifier)};
class ConstExprModifier : public Modifier { SLANG_AST_CLASS(ConstExprModifier)};
class GloballyCoherentModifier : public Modifier { SLANG_AST_CLASS(GloballyCoherentModifier)};
class ExternCppModifier : public Modifier { SLANG_AST_CLASS(ExternCppModifier)};
class GLSLPrecisionModifier : public Modifier { SLANG_AST_CLASS(GLSLPrecisionModifier)};
class GLSLModuleModifier : public Modifier {SLANG_AST_CLASS(GLSLModuleModifier)};
// Marks that the definition of a decl is not yet synthesized.
class ToBeSynthesizedModifier : public Modifier {SLANG_AST_CLASS(ToBeSynthesizedModifier)};

// Marks that the definition of a decl is synthesized.
class SynthesizedModifier : public Modifier { SLANG_AST_CLASS(SynthesizedModifier) };

// An `extern` variable in an extension is used to introduce additional attributes on an existing
// field.
class ExtensionExternVarModifier : public Modifier
{
    SLANG_AST_CLASS(ExtensionExternVarModifier)
    DeclRef<Decl> originalDecl;
};

// An 'ActualGlobal' is a global that is output as a normal global in CPU code. 
// Globals in HLSL/Slang are constant state passed into kernel execution 
class ActualGlobalModifier : public Modifier { SLANG_AST_CLASS(ActualGlobalModifier)};

    /// A modifier that indicates an `InheritanceDecl` should be ignored during name lookup (and related checks).
class IgnoreForLookupModifier : public Modifier { SLANG_AST_CLASS(IgnoreForLookupModifier) };

// A modifier that marks something as an operation that
// has a one-to-one translation to the IR, and thus
// has no direct definition in the high-level language.
//
class IntrinsicOpModifier : public Modifier 
{
    SLANG_AST_CLASS(IntrinsicOpModifier)
 
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
    SLANG_AST_CLASS(TargetIntrinsicModifier)
 
    // Token that names the target that the operation
    // is an intrisic for.
    Token targetToken;

    // A custom definition for the operation, one of either an ident or a
    // string (the concatenation of several string literals)
    Token definitionIdent;
    String definitionString;
    bool isString;

    // A predicate to be used on an identifier to guard this intrinsic
    Token predicateToken;
    NameLoc scrutinee;
    DeclRef<Decl> scrutineeDeclRef;
};

// A modifier that marks a declaration as representing a
// specialization that should be preferred on a particular
// target.
class SpecializedForTargetModifier : public Modifier 
{
    SLANG_AST_CLASS(SpecializedForTargetModifier)
 
    // Token that names the target that the operation
    // has been specialized for.
    Token targetToken;
};

// A modifier to tag something as an intrinsic that requires
// a certain GLSL extension to be enabled when used
class RequiredGLSLExtensionModifier : public Modifier 
{
    SLANG_AST_CLASS(RequiredGLSLExtensionModifier)
 
    Token extensionNameToken;
};

// A modifier to tag something as an intrinsic that requires
// a certain GLSL version to be enabled when used
class RequiredGLSLVersionModifier : public Modifier 
{
    SLANG_AST_CLASS(RequiredGLSLVersionModifier)
 
    Token versionNumberToken;
};


// A modifier to tag something as an intrinsic that requires
// a certain SPIRV version to be enabled when used. Specified as "major.minor"
class RequiredSPIRVVersionModifier : public Modifier 
{
    SLANG_AST_CLASS(RequiredSPIRVVersionModifier)
 
    SemanticVersion version;
};

// A modifier to tag something as an intrinsic that requires
// a certain CUDA SM version to be enabled when used. Specified as "major.minor"
class RequiredCUDASMVersionModifier : public Modifier 
{
    SLANG_AST_CLASS(RequiredCUDASMVersionModifier)
 
    SemanticVersion version;
};

class InOutModifier : public OutModifier 
{
    SLANG_AST_CLASS(InOutModifier)
};


// `__ref` modifier for by-reference parameter passing
class RefModifier : public Modifier 
{
    SLANG_AST_CLASS(RefModifier)
};

// `__ref` modifier for by-reference parameter passing
class ConstRefModifier : public Modifier
{
    SLANG_AST_CLASS(ConstRefModifier)
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
    SLANG_AST_CLASS(SharedModifiers)
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
    SLANG_ABSTRACT_AST_CLASS(GLSLLayoutModifier)
 

    // The token used to introduce the modifier is stored
    // as the `nameToken` field.

    // TODO: may want to accept a full expression here
    Token valToken;
};

// AST nodes to represent the begin/end of a `layout` modifier group
class GLSLLayoutModifierGroupMarker : public Modifier 
{
    SLANG_ABSTRACT_AST_CLASS(GLSLLayoutModifierGroupMarker)
};

class GLSLLayoutModifierGroupBegin : public GLSLLayoutModifierGroupMarker 
{
    SLANG_AST_CLASS(GLSLLayoutModifierGroupBegin)
};

class GLSLLayoutModifierGroupEnd : public GLSLLayoutModifierGroupMarker 
{
    SLANG_AST_CLASS(GLSLLayoutModifierGroupEnd)
};


// We divide GLSL `layout` modifiers into those we have parsed
// (in the sense of having some notion of their semantics), and
// those we have not.
class GLSLParsedLayoutModifier : public GLSLLayoutModifier 
{
    SLANG_ABSTRACT_AST_CLASS(GLSLParsedLayoutModifier)
};

class GLSLUnparsedLayoutModifier : public GLSLLayoutModifier 
{
    SLANG_AST_CLASS(GLSLUnparsedLayoutModifier)
};


// Specific cases for known GLSL `layout` modifiers that we need to work with
class GLSLConstantIDLayoutModifier : public GLSLParsedLayoutModifier 
{
    SLANG_AST_CLASS(GLSLConstantIDLayoutModifier)
};

class GLSLLocationLayoutModifier : public GLSLParsedLayoutModifier 
{
    SLANG_AST_CLASS(GLSLLocationLayoutModifier)
};

class GLSLBufferDataLayoutModifier : public GLSLParsedLayoutModifier
{
    SLANG_AST_CLASS(GLSLBufferDataLayoutModifier)
};

class GLSLStd140Modifier : public GLSLBufferDataLayoutModifier
{
    SLANG_AST_CLASS(GLSLStd140Modifier)
};

class GLSLStd430Modifier : public GLSLBufferDataLayoutModifier
{
    SLANG_AST_CLASS(GLSLStd430Modifier)
};

class GLSLScalarModifier : public GLSLBufferDataLayoutModifier
{
    SLANG_AST_CLASS(GLSLScalarModifier)
};


// A catch-all for single-keyword modifiers
class SimpleModifier : public Modifier 
{
    SLANG_AST_CLASS(SimpleModifier)
};


// Indicates that this is a variable declaration that corresponds to
// a parameter block declaration in the source program.
class ImplicitParameterGroupVariableModifier : public Modifier 
{
    SLANG_AST_CLASS(ImplicitParameterGroupVariableModifier)
};


// Indicates that this is a type that corresponds to the element
// type of a parameter block declaration in the source program.
class ImplicitParameterGroupElementTypeModifier : public Modifier 
{
    SLANG_AST_CLASS(ImplicitParameterGroupElementTypeModifier)
};


// An HLSL semantic
class HLSLSemantic : public Modifier 
{
    SLANG_ABSTRACT_AST_CLASS(HLSLSemantic)
 
    Token name;
};

// An HLSL semantic that affects layout
class HLSLLayoutSemantic : public HLSLSemantic 
{
    SLANG_AST_CLASS(HLSLLayoutSemantic)
 
    Token registerName;
    Token componentMask;
};

// An HLSL `register` semantic
class HLSLRegisterSemantic : public HLSLLayoutSemantic 
{
    SLANG_AST_CLASS(HLSLRegisterSemantic)
 
    Token spaceName;
};

// TODO(tfoley): `packoffset`
class HLSLPackOffsetSemantic : public HLSLLayoutSemantic 
{
    SLANG_AST_CLASS(HLSLPackOffsetSemantic)

    Index uniformOffset = 0;
};


// An HLSL semantic that just associated a declaration with a semantic name
class HLSLSimpleSemantic : public HLSLSemantic 
{
    SLANG_AST_CLASS(HLSLSimpleSemantic)
};

// A semantic applied to a field of a ray-payload type, to control access
class RayPayloadAccessSemantic : public HLSLSemantic
{
    SLANG_AST_CLASS(RayPayloadAccessSemantic)

    List<Token> stageNameTokens;
};

class RayPayloadReadSemantic : public RayPayloadAccessSemantic
{
    SLANG_AST_CLASS(RayPayloadReadSemantic)
};

class RayPayloadWriteSemantic : public RayPayloadAccessSemantic
{
    SLANG_AST_CLASS(RayPayloadWriteSemantic)
};


// GLSL

// Directives that came in via the preprocessor, but
// that we need to keep around for later steps
class GLSLPreprocessorDirective : public Modifier 
{
    SLANG_AST_CLASS(GLSLPreprocessorDirective)
};


// A GLSL `#version` directive
class GLSLVersionDirective : public GLSLPreprocessorDirective 
{
    SLANG_AST_CLASS(GLSLVersionDirective)
 

    // Token giving the version number to use
    Token versionNumberToken;

    // Optional token giving the sub-profile to be used
    Token glslProfileToken;
};

// A GLSL `#extension` directive
class GLSLExtensionDirective : public GLSLPreprocessorDirective 
{
    SLANG_AST_CLASS(GLSLExtensionDirective)
 

    // Token giving the version number to use
    Token extensionNameToken;

    // Optional token giving the sub-profile to be used
    Token dispositionToken;
};

class ParameterGroupReflectionName : public Modifier 
{
    SLANG_AST_CLASS(ParameterGroupReflectionName)
 
    NameLoc nameAndLoc;
};

// A modifier that indicates a built-in base type (e.g., `float`)
class BuiltinTypeModifier : public Modifier 
{
    SLANG_AST_CLASS(BuiltinTypeModifier)
 
    BaseType tag;
};

// A modifier that indicates a built-in type that isn't a base type (e.g., `vector`)
//
// TODO(tfoley): This deserves a better name than "magic"
class MagicTypeModifier : public Modifier 
{
    SLANG_AST_CLASS(MagicTypeModifier)

    ASTNodeType magicNodeType = ASTNodeType(-1);

        /// Modifier has a name so call this magicModifier to disambiguate
    String magicName;
    uint32_t tag = uint32_t(0);
};

// A modifier that indicates a built-in associated type requirement (e.g., `Differential`)
class BuiltinRequirementModifier : public Modifier
{
    SLANG_AST_CLASS(BuiltinRequirementModifier);

    BuiltinRequirementKind kind;
};


// A modifier applied to declarations of builtin types to indicate how they
// should be lowered to the IR.
//
// TODO: This should really subsume `BuiltinTypeModifier` and
// `MagicTypeModifier` so that we don't have to apply all of them.
class IntrinsicTypeModifier : public Modifier 
{
    SLANG_AST_CLASS(IntrinsicTypeModifier)
 
    // The IR opcode to use when constructing a type
    uint32_t irOp;

    Token opToken;

    // Additional literal opreands to provide when creating instances.
    // (e.g., for a texture type this passes in shape/mutability info)
    List<uint32_t> irOperands;
};

// Modifiers that affect the storage layout for matrices
class MatrixLayoutModifier : public Modifier 
{
    SLANG_AST_CLASS(MatrixLayoutModifier)
};


// Modifiers that specify row- and column-major layout, respectively
class RowMajorLayoutModifier : public MatrixLayoutModifier 
{
    SLANG_AST_CLASS(RowMajorLayoutModifier)
};

class ColumnMajorLayoutModifier : public MatrixLayoutModifier 
{
    SLANG_AST_CLASS(ColumnMajorLayoutModifier)
};


// The HLSL flavor of those modifiers
class HLSLRowMajorLayoutModifier : public RowMajorLayoutModifier 
{
    SLANG_AST_CLASS(HLSLRowMajorLayoutModifier)
};

class HLSLColumnMajorLayoutModifier : public ColumnMajorLayoutModifier 
{
    SLANG_AST_CLASS(HLSLColumnMajorLayoutModifier)
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
    SLANG_AST_CLASS(GLSLRowMajorLayoutModifier)
};

class GLSLColumnMajorLayoutModifier : public RowMajorLayoutModifier 
{
    SLANG_AST_CLASS(GLSLColumnMajorLayoutModifier)
};


// More HLSL Keyword

class InterpolationModeModifier : public Modifier 
{
    SLANG_ABSTRACT_AST_CLASS(InterpolationModeModifier)
 
};

// HLSL `nointerpolation` modifier
class HLSLNoInterpolationModifier : public InterpolationModeModifier 
{
    SLANG_AST_CLASS(HLSLNoInterpolationModifier)
};


// HLSL `noperspective` modifier
class HLSLNoPerspectiveModifier : public InterpolationModeModifier 
{
    SLANG_AST_CLASS(HLSLNoPerspectiveModifier)
};


// HLSL `linear` modifier
class HLSLLinearModifier : public InterpolationModeModifier 
{
    SLANG_AST_CLASS(HLSLLinearModifier)
};


// HLSL `sample` modifier
class HLSLSampleModifier : public InterpolationModeModifier 
{
    SLANG_AST_CLASS(HLSLSampleModifier)
};


// HLSL `centroid` modifier
class HLSLCentroidModifier : public InterpolationModeModifier 
{
    SLANG_AST_CLASS(HLSLCentroidModifier)
};

    /// Slang-defined `pervertex` modifier
class PerVertexModifier : public InterpolationModeModifier
{
    SLANG_AST_CLASS(PerVertexModifier)
};


// HLSL `precise` modifier
class PreciseModifier : public Modifier 
{
    SLANG_AST_CLASS(PreciseModifier)
};


// HLSL `shared` modifier (which is used by the effect system,
// and shouldn't be confused with `groupshared`)
class HLSLEffectSharedModifier : public Modifier 
{
    SLANG_AST_CLASS(HLSLEffectSharedModifier)
};


// HLSL `groupshared` modifier
class HLSLGroupSharedModifier : public Modifier 
{
    SLANG_AST_CLASS(HLSLGroupSharedModifier)
};


// HLSL `static` modifier (probably doesn't need to be
// treated as HLSL-specific)
class HLSLStaticModifier : public Modifier 
{
    SLANG_AST_CLASS(HLSLStaticModifier)
};


// HLSL `uniform` modifier (distinct meaning from GLSL
// use of the keyword)
class HLSLUniformModifier : public Modifier 
{
    SLANG_AST_CLASS(HLSLUniformModifier)
};


// HLSL `volatile` modifier (ignored)
class HLSLVolatileModifier : public Modifier 
{
    SLANG_AST_CLASS(HLSLVolatileModifier)
};


class AttributeTargetModifier : public Modifier 
{
    SLANG_AST_CLASS(AttributeTargetModifier)
 
    // A class to which the declared attribute type is applicable
    SyntaxClass<NodeBase> syntaxClass;
};


// Base class for checked and unchecked `[name(arg0, ...)]` style attribute.
class AttributeBase : public Modifier 
{
    SLANG_AST_CLASS(AttributeBase)
    
    AttributeDecl* attributeDecl = nullptr;

    // The original identifier token representing the last part of the qualified name.
    Token originalIdentifierToken;

    List<Expr*> args;
};

// A `[name(...)]` attribute that hasn't undergone any semantic analysis.
// After analysis, this will be transformed into a more specific case.
class UncheckedAttribute : public AttributeBase 
{
    SLANG_AST_CLASS(UncheckedAttribute)

    SLANG_UNREFLECTED
    Scope* scope = nullptr;
};

// A `[name(arg0, ...)]` style attribute that has been validated.
class Attribute : public AttributeBase 
{
    SLANG_AST_CLASS(Attribute)
 
    List<Val*> intArgVals;
};

class UserDefinedAttribute : public Attribute 
{
    SLANG_AST_CLASS(UserDefinedAttribute)
};

class AttributeUsageAttribute : public Attribute 
{
    SLANG_AST_CLASS(AttributeUsageAttribute)
 
    SyntaxClass<NodeBase> targetSyntaxClass;
};

class NonDynamicUniformAttribute : public Attribute
{
    SLANG_AST_CLASS(NonDynamicUniformAttribute)
};

class RequireCapabilityAttribute : public Attribute
{
    SLANG_AST_CLASS(RequireCapabilityAttribute)
    CapabilitySet capabilitySet;
};


// An `[unroll]` or `[unroll(count)]` attribute
class UnrollAttribute : public Attribute 
{
    SLANG_AST_CLASS(UnrollAttribute)

};

// An `[unroll]` or `[unroll(count)]` attribute
class ForceUnrollAttribute : public Attribute
{
    SLANG_AST_CLASS(ForceUnrollAttribute)

    int32_t maxIterations = 0;
};

// An `[maxiters(count)]`
class MaxItersAttribute : public Attribute 
{
    SLANG_AST_CLASS(MaxItersAttribute)
 
    int32_t value = 0;
};

// An inferred max iteration count on a loop.
class InferredMaxItersAttribute : public Attribute
{
    SLANG_AST_CLASS(InferredMaxItersAttribute)
    DeclRef<Decl> inductionVar;
    int32_t value = 0;
};

class LoopAttribute : public Attribute 
{
    SLANG_AST_CLASS(LoopAttribute)
};
               // `[loop]`
class FastOptAttribute : public Attribute 
{
    SLANG_AST_CLASS(FastOptAttribute)
};
            // `[fastopt]`
class AllowUAVConditionAttribute : public Attribute 
{
    SLANG_AST_CLASS(AllowUAVConditionAttribute)
};
  // `[allow_uav_condition]`
class BranchAttribute : public Attribute 
{
    SLANG_AST_CLASS(BranchAttribute)
};
             // `[branch]`
class FlattenAttribute : public Attribute 
{
    SLANG_AST_CLASS(FlattenAttribute)
};
            // `[flatten]`
class ForceCaseAttribute : public Attribute 
{
    SLANG_AST_CLASS(ForceCaseAttribute)
};
          // `[forcecase]`
class CallAttribute : public Attribute 
{
    SLANG_AST_CLASS(CallAttribute)
};
               // `[call]`


// [[vk_push_constant]] [[push_constant]]
class PushConstantAttribute : public Attribute 
{
    SLANG_AST_CLASS(PushConstantAttribute)
};


// [[vk_shader_record]] [[shader_record]]
class ShaderRecordAttribute : public Attribute 
{
    SLANG_AST_CLASS(ShaderRecordAttribute)
};


// [[vk_binding]]
class GLSLBindingAttribute : public Attribute 
{
    SLANG_AST_CLASS(GLSLBindingAttribute)
 
    int32_t binding = 0;
    int32_t set = 0;
};

class GLSLSimpleIntegerLayoutAttribute : public Attribute 
{
    SLANG_AST_CLASS(GLSLSimpleIntegerLayoutAttribute)
 
    int32_t value = 0;
};

// [[vk_location]]
class GLSLLocationAttribute : public GLSLSimpleIntegerLayoutAttribute 
{
    SLANG_AST_CLASS(GLSLLocationAttribute)
};


// [[vk_index]]
class GLSLIndexAttribute : public GLSLSimpleIntegerLayoutAttribute 
{
    SLANG_AST_CLASS(GLSLIndexAttribute)
};

// [[vk_spirv_instruction]]
class SPIRVInstructionOpAttribute : public Attribute
{
    SLANG_AST_CLASS(SPIRVInstructionOpAttribute)
};

// [[spv_target_env_1_3]]
class SPIRVTargetEnv13Attribute : public Attribute
{
    SLANG_AST_CLASS(SPIRVTargetEnv13Attribute);
};

// [[disable_array_flattening]]
class DisableArrayFlatteningAttribute : public Attribute
{
    SLANG_AST_CLASS(DisableArrayFlatteningAttribute);
};

// A GLSL layout(local_size_x = 64, ... attribute)
class GLSLLayoutLocalSizeAttribute : public Attribute
{
    SLANG_AST_CLASS(GLSLLayoutLocalSizeAttribute)

    // The number of threads to use along each axis
    //
    // TODO: These should be accessors that use the
    // ordinary `args` list, rather than side data.
    IntVal* x;
    IntVal* y;
    IntVal* z;
};

// TODO: for attributes that take arguments, the syntax node
// classes should provide accessors for the values of those arguments.

class MaxTessFactorAttribute : public Attribute 
{
    SLANG_AST_CLASS(MaxTessFactorAttribute)
};

class OutputControlPointsAttribute : public Attribute 
{
    SLANG_AST_CLASS(OutputControlPointsAttribute)
};

class OutputTopologyAttribute : public Attribute 
{
    SLANG_AST_CLASS(OutputTopologyAttribute)
};

class PartitioningAttribute : public Attribute 
{
    SLANG_AST_CLASS(PartitioningAttribute)
};

class PatchConstantFuncAttribute : public Attribute 
{
    SLANG_AST_CLASS(PatchConstantFuncAttribute)
 
    FuncDecl* patchConstantFuncDecl = nullptr;
};
class DomainAttribute : public Attribute 
{
    SLANG_AST_CLASS(DomainAttribute)
};


class EarlyDepthStencilAttribute : public Attribute 
{
    SLANG_AST_CLASS(EarlyDepthStencilAttribute)
};
  // `[earlydepthstencil]`

// An HLSL `[numthreads(x,y,z)]` attribute
class NumThreadsAttribute : public Attribute 
{
    SLANG_AST_CLASS(NumThreadsAttribute)
 
    // The number of threads to use along each axis
    //
    // TODO: These should be accessors that use the
    // ordinary `args` list, rather than side data.
    IntVal* x;
    IntVal* y;
    IntVal* z;
};

class MaxVertexCountAttribute : public Attribute 
{
    SLANG_AST_CLASS(MaxVertexCountAttribute)
 
    // The number of max vertex count for geometry shader
    //
    // TODO: This should be an accessor that uses the
    // ordinary `args` list, rather than side data.
    int32_t value;
};

class InstanceAttribute : public Attribute 
{
    SLANG_AST_CLASS(InstanceAttribute)
 
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
    SLANG_AST_CLASS(EntryPointAttribute)
 
    // The resolved stage that the entry point is targetting.
    //
    // TODO: This should be an accessor that uses the
    // ordinary `args` list, rather than side data.
    Stage stage;
};

// A `[__vulkanRayPayload(location)]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the input/output interface for a Vulkan
// ray tracing shader to pass per-ray payload information.
class VulkanRayPayloadAttribute : public Attribute 
{
    SLANG_AST_CLASS(VulkanRayPayloadAttribute)

    int location;
};


// A `[__vulkanCallablePayload(location)]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the input/output interface for a Vulkan
// ray tracing shader to pass payload information to/from a callee.
class VulkanCallablePayloadAttribute : public Attribute 
{
    SLANG_AST_CLASS(VulkanCallablePayloadAttribute)

    int location;
};


// A `[__vulkanHitAttributes]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the output interface for a Vulkan
// intersection shader to pass hit attribute information.
class VulkanHitAttributesAttribute : public Attribute 
{
    SLANG_AST_CLASS(VulkanHitAttributesAttribute)
};

// A `[__vulkanHitObjectAttributes(location)]` attribute, which is used in the
// standard library implementation to indicate that a variable
// actually represents the attributes on a HitObject as part of
// Shader ExecutionReordering
class VulkanHitObjectAttributesAttribute : public Attribute
{
    SLANG_AST_CLASS(VulkanHitObjectAttributesAttribute)

    int location;
};

// A `[mutating]` attribute, which indicates that a member
// function is allowed to modify things through its `this`
// argument.
//
class MutatingAttribute : public Attribute 
{
    SLANG_AST_CLASS(MutatingAttribute)
};

// A `[nonmutating]` attribute, which indicates that a
// `set` accessor does not need to modify anything through
// its `this` parameter.
//
class NonmutatingAttribute : public Attribute
{
    SLANG_AST_CLASS(NonmutatingAttribute)
};

// A `[constref]` attribute, which indicates that the `this` parameter of
// a member function should be passed by reference.
//
class ConstRefAttribute : public Attribute
{
    SLANG_AST_CLASS(ConstRefAttribute)
};


// A `[__readNone]` attribute, which indicates that a function
// computes its results strictly based on argument values, without
// reading or writing through any pointer arguments, or any other
// state that could be observed by a caller.
//
class ReadNoneAttribute : public Attribute 
{
    SLANG_AST_CLASS(ReadNoneAttribute)
};



// HLSL modifiers for geometry shader input topology
class HLSLGeometryShaderInputPrimitiveTypeModifier : public Modifier 
{
    SLANG_AST_CLASS(HLSLGeometryShaderInputPrimitiveTypeModifier)
};

class HLSLPointModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_AST_CLASS(HLSLPointModifier)
};

class HLSLLineModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_AST_CLASS(HLSLLineModifier)
};

class HLSLTriangleModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_AST_CLASS(HLSLTriangleModifier)
};

class HLSLLineAdjModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_AST_CLASS(HLSLLineAdjModifier)
};

class HLSLTriangleAdjModifier : public HLSLGeometryShaderInputPrimitiveTypeModifier 
{
    SLANG_AST_CLASS(HLSLTriangleAdjModifier)
};

// Mesh shader paramters

class HLSLMeshShaderOutputModifier : public Modifier
{
    SLANG_AST_CLASS(HLSLMeshShaderOutputModifier)
};

class HLSLVerticesModifier : public HLSLMeshShaderOutputModifier
{
    SLANG_AST_CLASS(HLSLVerticesModifier)
};

class HLSLIndicesModifier : public HLSLMeshShaderOutputModifier
{
    SLANG_AST_CLASS(HLSLIndicesModifier)
};

class HLSLPrimitivesModifier : public HLSLMeshShaderOutputModifier
{
    SLANG_AST_CLASS(HLSLPrimitivesModifier)
};

class HLSLPayloadModifier : public Modifier
{
    SLANG_AST_CLASS(HLSLPayloadModifier)
};

// A modifier to indicate that a constructor/initializer can be used
// to perform implicit type conversion, and to specify the cost of
// the conversion, if applied.
class ImplicitConversionModifier : public Modifier 
{
    SLANG_AST_CLASS(ImplicitConversionModifier)
 
    // The conversion cost, used to rank conversions
    ConversionCost cost;

    // A builtin identifier for identifying conversions that need special treatment.
    BuiltinConversionKind builtinConversionKind;
};

class FormatAttribute : public Attribute 
{
    SLANG_AST_CLASS(FormatAttribute)
 
    ImageFormat format;
};

class AllowAttribute : public Attribute 
{
    SLANG_AST_CLASS(AllowAttribute)
 
    DiagnosticInfo const* diagnostic = nullptr;
};


// A `[__extern]` attribute, which indicates that a function/type is defined externally
//
class ExternAttribute : public Attribute 
{
    SLANG_AST_CLASS(ExternAttribute)
};


// An `[__unsafeForceInlineExternal]` attribute indicates that the callee should be inlined
// into call sites after initial IR generation (that is, as early as possible).
//
class UnsafeForceInlineEarlyAttribute : public Attribute 
{
    SLANG_AST_CLASS(UnsafeForceInlineEarlyAttribute)
};

// A `[ForceInline]` attribute indicates that the callee should be inlined
// by the Slang compiler.
//
class ForceInlineAttribute : public Attribute
{
    SLANG_AST_CLASS(ForceInlineAttribute)
};


    /// An attribute that marks a type declaration as either allowing or
    /// disallowing the type to be inherited from in other modules.
class InheritanceControlAttribute : public Attribute { SLANG_AST_CLASS(InheritanceControlAttribute) };

    /// An attribute that marks a type declaration as allowing the type to be inherited from in other modules.
class OpenAttribute : public InheritanceControlAttribute { SLANG_AST_CLASS(OpenAttribute) };

    /// An attribute that marks a type declaration as disallowing the type to be inherited from in other modules.
class SealedAttribute : public InheritanceControlAttribute { SLANG_AST_CLASS(SealedAttribute) };

    /// An attribute that marks a decl as a compiler built-in object.
class BuiltinAttribute : public Attribute
{
    SLANG_AST_CLASS(BuiltinAttribute)
};

    /// An attribute that defines the size of `AnyValue` type to represent a polymoprhic value that conforms to
    /// the decorated interface type.
class AnyValueSizeAttribute : public Attribute
{
    SLANG_AST_CLASS(AnyValueSizeAttribute)

    int32_t size;
};

    /// This is a stop-gap solution to break overload ambiguity in stdlib.
    /// When there is a function overload ambiguity, the compiler will pick the one with higher rank
    /// specified by this attribute. An overload without this attribute will have a rank of 0.
    /// In the future, we should enhance our type system to take into account the "specialized"-ness
    /// of an overload, such that `T overload1<T:IDerived>()` is more specialized than `T overload2<T:IBase>()`
    /// and preferred during overload resolution.
class OverloadRankAttribute : public Attribute
{
    SLANG_AST_CLASS(OverloadRankAttribute)
    int32_t rank;
};

    /// An attribute that marks an interface for specialization use only. Any operation that triggers dynamic
    /// dispatch through the interface is a compile-time error.
class SpecializeAttribute : public Attribute
{
    SLANG_AST_CLASS(SpecializeAttribute)
};

    /// An attribute that marks a type, function or variable as differentiable.
class DifferentiableAttribute : public Attribute
{
    SLANG_AST_CLASS(DifferentiableAttribute)

    List<KeyValuePair<DeclRefBase*, SubtypeWitness*>> m_typeToIDifferentiableWitnessMappings;

    void addType(DeclRefBase* declRef, SubtypeWitness* witness)
    {
        getMapTypeToIDifferentiableWitness();
        if (m_mapToIDifferentiableWitness.addIfNotExists(declRef, witness))
        {
            m_typeToIDifferentiableWitnessMappings.add(KeyValuePair<DeclRefBase*, SubtypeWitness*>(declRef, witness));
        }
    }

    /// Mapping from types to subtype witnesses for conformance to IDifferentiable.
    const OrderedDictionary<DeclRefBase*, SubtypeWitness*>& getMapTypeToIDifferentiableWitness();

    SLANG_UNREFLECTED ValSet m_typeRegistrationWorkingSet;
private:
    OrderedDictionary<DeclRefBase*, SubtypeWitness*> m_mapToIDifferentiableWitness;
};

class DllImportAttribute : public Attribute
{
    SLANG_AST_CLASS(DllImportAttribute)

    String modulePath;

    String functionName;
};

class DllExportAttribute : public Attribute
{
    SLANG_AST_CLASS(DllExportAttribute)
};

class TorchEntryPointAttribute : public Attribute
{
    SLANG_AST_CLASS(TorchEntryPointAttribute)
};

class CudaDeviceExportAttribute : public Attribute
{
    SLANG_AST_CLASS(CudaDeviceExportAttribute)
};

class CudaKernelAttribute : public Attribute
{
    SLANG_AST_CLASS(CudaKernelAttribute)
};

class CudaHostAttribute : public Attribute
{
    SLANG_AST_CLASS(CudaHostAttribute)
};

class AutoPyBindCudaAttribute : public Attribute
{
    SLANG_AST_CLASS(AutoPyBindCudaAttribute)
};

class PyExportAttribute : public Attribute
{
    SLANG_AST_CLASS(PyExportAttribute)

    String name;
};

class PreferRecomputeAttribute : public Attribute
{
    SLANG_AST_CLASS(PreferRecomputeAttribute)
};

class PreferCheckpointAttribute : public Attribute
{
    SLANG_AST_CLASS(PreferCheckpointAttribute)
};

class DerivativeMemberAttribute : public Attribute
{
    SLANG_AST_CLASS(DerivativeMemberAttribute)

    DeclRefExpr* memberDeclRef;
};

    /// An attribute that marks an interface type as a COM interface declaration.
class ComInterfaceAttribute : public Attribute
{
    SLANG_AST_CLASS(ComInterfaceAttribute)

    String guid;
};

    /// A `[__requiresNVAPI]` attribute indicates that the declaration being modifed
    /// requires NVAPI operations for its implementation on D3D.
class RequiresNVAPIAttribute : public Attribute
{
    SLANG_AST_CLASS(RequiresNVAPIAttribute)
};


    /// A `[__AlwaysFoldIntoUseSite]` attribute indicates that the calls into the modified
    /// function should always be folded into use sites during source emit.
class AlwaysFoldIntoUseSiteAttribute :public Attribute
{
    SLANG_AST_CLASS(AlwaysFoldIntoUseSiteAttribute)
};

// A `[TreatAsDifferentiableAttribute]` attribute indicates that a function or an interface
// should be treated as differentiable in IR validation step.
//
class TreatAsDifferentiableAttribute : public DifferentiableAttribute
{
    SLANG_AST_CLASS(TreatAsDifferentiableAttribute)
};

    /// The `[ForwardDifferentiable]` attribute indicates that a function can be forward-differentiated.
class ForwardDifferentiableAttribute : public DifferentiableAttribute
{
    SLANG_AST_CLASS(ForwardDifferentiableAttribute)
};

class UserDefinedDerivativeAttribute : public DifferentiableAttribute
{
    SLANG_AST_CLASS(UserDefinedDerivativeAttribute)

    Expr* funcExpr;
};

    /// The `[ForwardDerivative(function)]` attribute specifies a custom function that should
    /// be used as the derivative for the decorated function.
class ForwardDerivativeAttribute : public UserDefinedDerivativeAttribute
{
    SLANG_AST_CLASS(ForwardDerivativeAttribute)
};

class DerivativeOfAttribute : public DifferentiableAttribute
{
    SLANG_AST_CLASS(DerivativeOfAttribute)

    Expr* funcExpr;

    Expr* backDeclRef; // DeclRef to this derivative function when initiated from primalFunction.
};

    /// The `[ForwardDerivativeOf(primalFunction)]` attribute marks the decorated function as custom
    /// derivative implementation for `primalFunction`.
    /// ForwardDerivativeOfAttribute inherits from DifferentiableAttribute because a derivative
    /// function itself is considered differentiable.
class ForwardDerivativeOfAttribute : public DerivativeOfAttribute
{
    SLANG_AST_CLASS(ForwardDerivativeOfAttribute)
};

    /// The `[BackwardDifferentiable]` attribute indicates that a function can be backward-differentiated.
class BackwardDifferentiableAttribute : public DifferentiableAttribute
{
    SLANG_AST_CLASS(BackwardDifferentiableAttribute)
    int maxOrder = 0;
};

    /// The `[BackwardDerivative(function)]` attribute specifies a custom function that should
    /// be used as the backward-derivative for the decorated function.
class BackwardDerivativeAttribute : public UserDefinedDerivativeAttribute
{
    SLANG_AST_CLASS(BackwardDerivativeAttribute)
};

    /// The `[BackwardDerivativeOf(primalFunction)]` attribute marks the decorated function as custom
    /// backward-derivative implementation for `primalFunction`.
class BackwardDerivativeOfAttribute : public DerivativeOfAttribute
{
    SLANG_AST_CLASS(BackwardDerivativeOfAttribute)
};

    /// The `[PrimalSubstitute(function)]` attribute specifies a custom function that should
    /// be used as the primal function substitute when differentiating code that calls the primal function.
class PrimalSubstituteAttribute : public Attribute
{
    SLANG_AST_CLASS(PrimalSubstituteAttribute)
    Expr* funcExpr;
};

    /// The `[PrimalSubstituteOf(primalFunction)]` attribute marks the decorated function as
    /// the substitute primal function in a forward or backward derivative function.
class PrimalSubstituteOfAttribute : public Attribute
{
    SLANG_AST_CLASS(PrimalSubstituteOfAttribute)

    Expr* funcExpr;
    Expr* backDeclRef; // DeclRef to this derivative function when initiated from primalFunction.
};

    /// The `[NoDiffThis]` attribute is used to specify that the `this` parameter should not be
    /// included for differentiation.
class NoDiffThisAttribute : public Attribute
{
    SLANG_AST_CLASS(NoDiffThisAttribute)
};

    /// Indicates that the modified declaration is one of the "magic" declarations
    /// that NVAPI uses to communicate extended operations. When NVAPI is being included
    /// via the prelude for downstream compilation, declarations with this modifier
    /// will not be emitted, instead allowing the versions from the prelude to be used.
class NVAPIMagicModifier : public Modifier
{
    SLANG_AST_CLASS(NVAPIMagicModifier)
};

    /// A modifier that attaches to a `ModuleDecl` to indicate the register/space binding
    /// that NVAPI wants to use, as indicated by, e.g., the `NV_SHADER_EXTN_SLOT` and
    /// `NV_SHADER_EXTN_REGISTER_SPACE` preprocessor definitions.
class NVAPISlotModifier : public Modifier
{
    SLANG_AST_CLASS(NVAPISlotModifier)

        /// The name of the register that is to be used (e.g., `"u3"`)
        ///
        /// This value will come from the `NV_SHADER_EXTN_SLOT` macro, if set.
        ///
        /// The `registerName` field must always be filled in when adding
        /// an `NVAPISlotModifier` to a module; if no register name is defined,
        /// then the modifier should not be added.
        ///
    String registerName;

        /// The name of the register space to be used (e.g., `space1`)
        ///
        /// This value will come from the `NV_SHADER_EXTN_REGISTER_SPACE` macro,
        /// if set.
        ///
        /// It is valid for a user to specify a register name but not a space name,
        /// and in that case `spaceName` will be set to `"space0"`.
    String spaceName;
};

    /// A `[noinline]` attribute represents a request by the application that,
    /// to the extent possible, a function should not be inlined into call sites.
    ///
    /// Note that due to various limitations of different targets, it is entirely
    /// possible for such functions to be inlined or specialized to call sites.
    ///
class NoInlineAttribute : public Attribute
{
    SLANG_AST_CLASS(NoInlineAttribute)
};

    /// A `[payload]` attribute indicates that a `struct` type will be used as
    /// a ray payload for `TraceRay()` calls, and thus also as input/output
    /// for shaders in the ray tracing pipeline that might be invoked for
    /// such a ray.
    ///
class PayloadAttribute : public Attribute
{
    SLANG_AST_CLASS(PayloadAttribute)
};

    /// A `[deprecated("message")]` attribute indicates the target is
    /// deprecated.
    /// A compiler warning including the message will be raised if the
    /// deprecated value is used.
    ///
class DeprecatedAttribute : public Attribute
{
    SLANG_AST_CLASS(DeprecatedAttribute)

    String message;
};

class NonCopyableTypeAttribute : public Attribute
{
    SLANG_AST_CLASS(NonCopyableTypeAttribute)
};

class NoSideEffectAttribute : public Attribute
{
    SLANG_AST_CLASS(NoSideEffectAttribute)
};

    /// A `[KnownBuiltin("name")]` attribute allows the compiler to
    /// identify this declaration during compilation, despite obfuscation or
    /// linkage removing optimizations
    ///
class KnownBuiltinAttribute : public Attribute
{
    SLANG_AST_CLASS(KnownBuiltinAttribute)

    String name;
};

    /// A modifier that applies to types rather than declarations.
    ///
    /// In most cases, the Slang compiler assumes that a modifier should
    /// inhere to a declaration. Given input like:
    ///
    /// mod1 mod2 int myVar = ...;
    ///
    /// The default assumption is that `mod1` and `mod2` apply to `myVar`
    /// and *not* to the `int` type.
    ///
    /// In order to allow modifiers to inhere to the type instead, we introduce
    /// a base class for modifiers that really don't want to belong to the declaration,
    /// and instead want to belong to the type (or rather the type *specifier*
    /// from a parsing standpoint).
    ///
class TypeModifier : public Modifier
{
    SLANG_AST_CLASS(TypeModifier)
};

    /// A kind of syntax element which appears as a modifier in the syntax, but
    /// we represent as a function over type expressions
class WrappingTypeModifier : public TypeModifier
{
    SLANG_AST_CLASS(WrappingTypeModifier)
};

    /// A modifier that applies to a type and implies information about the
    /// underlying format of a resource that uses that type as its element type.
    ///
class ResourceElementFormatModifier : public TypeModifier
{
    SLANG_AST_CLASS(ResourceElementFormatModifier)
};

    /// HLSL `unorm` modifier
class UNormModifier : public ResourceElementFormatModifier
{
    SLANG_AST_CLASS(UNormModifier)
};

    /// HLSL `snorm` modifier
class SNormModifier : public ResourceElementFormatModifier
{
    SLANG_AST_CLASS(SNormModifier)
};

class NoDiffModifier : public TypeModifier
{
    SLANG_AST_CLASS(NoDiffModifier)
};

// Some GLSL-specific modifiers
class GLSLBufferModifier : public WrappingTypeModifier
{
    SLANG_AST_CLASS(GLSLBufferModifier)
};

class GLSLWriteOnlyModifier : public SimpleModifier
{
    SLANG_AST_CLASS(GLSLWriteOnlyModifier)
};

class GLSLReadOnlyModifier : public SimpleModifier
{
    SLANG_AST_CLASS(GLSLReadOnlyModifier)
};

class GLSLPatchModifier : public SimpleModifier
{
    SLANG_AST_CLASS(GLSLPatchModifier)
};

//
class BitFieldModifier : public Modifier
{
    SLANG_AST_CLASS(BitFieldModifier)

    IntegerLiteralValue width;

    // Fields filled during semantic analysis
    IntegerLiteralValue offset = 0;
    DeclRef<VarDecl> backingDeclRef;
};

class DynamicUniformModifier : public Modifier
{
    SLANG_AST_CLASS(DynamicUniformModifier)
};

} // namespace Slang
