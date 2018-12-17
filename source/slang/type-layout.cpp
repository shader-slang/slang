// TypeLayout.cpp
#include "type-layout.h"

#include "syntax.h"

#include <assert.h>

namespace Slang {

size_t RoundToAlignment(size_t offset, size_t alignment)
{
    size_t remainder = offset % alignment;
    if (remainder == 0)
        return offset;
    else
        return offset + (alignment - remainder);
}

LayoutSize RoundToAlignment(LayoutSize offset, size_t alignment)
{
    // An infinite size is assumed to be maximally aligned.
    if(offset.isInfinite())
        return LayoutSize::infinite();

    return RoundToAlignment(offset.getFiniteValue(), alignment);
}

static size_t RoundUpToPowerOfTwo( size_t value )
{
    // TODO(tfoley): I know this isn't a fast approach
    size_t result = 1;
    while (result < value)
        result *= 2;
    return result;
}

//

struct DefaultLayoutRulesImpl : SimpleLayoutRulesImpl
{
    // Get size and alignment for a single value of base type.
    SimpleLayoutInfo GetScalarLayout(BaseType baseType) override
    {
        switch (baseType)
        {
        case BaseType::Void:    return SimpleLayoutInfo();

        // Note: By convention, a `bool` in a constant buffer is stored as an `int.
        // This default may eventually change, at which point this logic will need
        // to be updated.
        //
        // TODO: We should probably warn in this case, since storing a `bool` in
        // a constant buffer seems like a Bad Idea anyway.
        //
        case BaseType::Bool:    return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4, 4 );


        case BaseType::Int8:    return SimpleLayoutInfo( LayoutResourceKind::Uniform, 1,1);
        case BaseType::Int16:   return SimpleLayoutInfo( LayoutResourceKind::Uniform, 2,2);
        case BaseType::Int:     return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4,4);
        case BaseType::Int64:   return SimpleLayoutInfo( LayoutResourceKind::Uniform, 8,8);

        case BaseType::UInt8:   return SimpleLayoutInfo( LayoutResourceKind::Uniform, 1,1);
        case BaseType::UInt16:  return SimpleLayoutInfo( LayoutResourceKind::Uniform, 2,2);
        case BaseType::UInt:    return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4,4);
        case BaseType::UInt64:  return SimpleLayoutInfo( LayoutResourceKind::Uniform, 8,8);

        case BaseType::Half:    return SimpleLayoutInfo( LayoutResourceKind::Uniform, 2,2);
        case BaseType::Float:   return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4,4);
        case BaseType::Double:  return SimpleLayoutInfo( LayoutResourceKind::Uniform, 8,8);

        default:
            SLANG_UNEXPECTED("uhandled scalar type");
            UNREACHABLE_RETURN(SimpleLayoutInfo( LayoutResourceKind::Uniform, 0, 1 ));
        }
    }

    SimpleArrayLayoutInfo GetArrayLayout( SimpleLayoutInfo elementInfo, LayoutSize elementCount) override
    {
        SLANG_RELEASE_ASSERT(elementInfo.size.isFinite());
        auto stride = elementInfo.size.getFiniteValue();

        SimpleArrayLayoutInfo arrayInfo;
        arrayInfo.kind = elementInfo.kind;
        arrayInfo.size = stride * elementCount;
        arrayInfo.alignment = elementInfo.alignment;
        arrayInfo.elementStride = stride;
        return arrayInfo;
    }

    SimpleLayoutInfo GetVectorLayout(SimpleLayoutInfo elementInfo, size_t elementCount) override
    {
        SimpleLayoutInfo vectorInfo;
        vectorInfo.kind = elementInfo.kind;
        vectorInfo.size = elementInfo.size * elementCount;
        vectorInfo.alignment = elementInfo.alignment;
        return vectorInfo;
    }

    SimpleLayoutInfo GetMatrixLayout(SimpleLayoutInfo elementInfo, size_t rowCount, size_t columnCount) override
    {
        // The default behavior here is to lay out a matrix
        // as an array of column vectors (that is column-major).
        // That is because this is the default convention
        // used by HLSL.
        //
        // In practice, the code that calls `GetMatrixLayout` will
        // potentially transpose the row/column counts in order
        // to get layouts with a different convention.
        //
        return GetArrayLayout(
            GetVectorLayout(elementInfo, columnCount),
            rowCount);
    }

    UniformLayoutInfo BeginStructLayout() override
    {
        UniformLayoutInfo structInfo(0, 1);
        return structInfo;
    }

    LayoutSize AddStructField(UniformLayoutInfo* ioStructInfo, UniformLayoutInfo fieldInfo) override
    {
        // Skip zero-size fields
        if(fieldInfo.size == 0)
            return ioStructInfo->size;

        ioStructInfo->alignment = std::max(ioStructInfo->alignment, fieldInfo.alignment);
        ioStructInfo->size = RoundToAlignment(ioStructInfo->size, fieldInfo.alignment);
        LayoutSize fieldOffset = ioStructInfo->size;
        ioStructInfo->size += fieldInfo.size;
        return fieldOffset;
    }


    void EndStructLayout(UniformLayoutInfo* ioStructInfo) override
    {
        ioStructInfo->size = RoundToAlignment(ioStructInfo->size, ioStructInfo->alignment);
    }
};

// Capture common behavior betwen HLSL and GLSL (`std140`) constnat buffer rules
struct DefaultConstantBufferLayoutRulesImpl : DefaultLayoutRulesImpl
{
    // The `std140` rules require that all array elements
    // be a multiple of 16 bytes.
    //
    // HLSL agrees.
    SimpleArrayLayoutInfo GetArrayLayout(SimpleLayoutInfo elementInfo, LayoutSize elementCount) override
    {
        if(elementInfo.kind == LayoutResourceKind::Uniform)
        {
            if (elementInfo.alignment < 16)
                elementInfo.alignment = 16;
            elementInfo.size = RoundToAlignment(elementInfo.size, elementInfo.alignment);
        }
        return DefaultLayoutRulesImpl::GetArrayLayout(elementInfo, elementCount);
    }

    // The `std140` rules require that a `struct` type be
    // aligned to at least 16.
    //
    // HLSL agrees.
    UniformLayoutInfo BeginStructLayout() override
    {
        return UniformLayoutInfo(0, 16);
    }
};

struct GLSLConstantBufferLayoutRulesImpl : DefaultConstantBufferLayoutRulesImpl
{
};

// The `std140` and `std430` rules require vectors to be aligned to the next power of
// two up from their size (so a `float2` is 8-byte aligned, and a `float3` is
// 16-byte aligned).
//
// Note that in this case we have a type layout where the size is *not* a multiple
// of the alignment, so it should be possible to pack a scalar after a `float3`.
static SimpleLayoutInfo getGLSLVectorLayout(
    SimpleLayoutInfo elementInfo, size_t elementCount)
{
    SLANG_RELEASE_ASSERT(elementInfo.kind == LayoutResourceKind::Uniform);
    SLANG_RELEASE_ASSERT(elementInfo.size.isFinite());

    auto size = elementInfo.size.getFiniteValue() * elementCount;
    SimpleLayoutInfo vectorInfo(
        LayoutResourceKind::Uniform,
        size,
        RoundUpToPowerOfTwo(size));
    return vectorInfo;
}

// The `std140` rules combine the GLSL-specific layout for 3-vectors with the
// alignment padding for structures and arrays that is common to both HLSL
// and GLSL constant buffers.
struct Std140LayoutRulesImpl : GLSLConstantBufferLayoutRulesImpl
{
    SimpleLayoutInfo GetVectorLayout(SimpleLayoutInfo elementInfo, size_t elementCount) override
    {
        return getGLSLVectorLayout(elementInfo, elementCount);
    }
};

struct HLSLConstantBufferLayoutRulesImpl : DefaultConstantBufferLayoutRulesImpl
{
    // Can't let a `struct` field straddle a register (16-byte) boundary
    LayoutSize AddStructField(UniformLayoutInfo* ioStructInfo, UniformLayoutInfo fieldInfo) override
    {
        // Skip zero-size fields
        if(fieldInfo.size == 0)
            return ioStructInfo->size;

        ioStructInfo->alignment = std::max(ioStructInfo->alignment, fieldInfo.alignment);
        ioStructInfo->size = RoundToAlignment(ioStructInfo->size, fieldInfo.alignment);

        LayoutSize fieldOffset = ioStructInfo->size;
        LayoutSize fieldSize = fieldInfo.size;

        // Would this field cross a 16-byte boundary?
        auto registerSize = 16;
        auto startRegister = fieldOffset / registerSize;
        auto endRegister = (fieldOffset + fieldSize - 1) / registerSize;
        if (startRegister != endRegister)
        {
            ioStructInfo->size = RoundToAlignment(ioStructInfo->size, size_t(registerSize));
            fieldOffset = ioStructInfo->size;
        }

        ioStructInfo->size += fieldInfo.size;
        return fieldOffset;
    }
};

struct HLSLStructuredBufferLayoutRulesImpl : DefaultLayoutRulesImpl
{
    // TODO: customize these to be correct...
};

// The `std430` rules don't include the array/structure alignment padding that
// gets applied to constant buffers, but they do include the padding of 3-vectors
// to be aligned as 4-vectors.
struct Std430LayoutRulesImpl : DefaultLayoutRulesImpl
{
    SimpleLayoutInfo GetVectorLayout(SimpleLayoutInfo elementInfo, size_t elementCount) override
    {
        return getGLSLVectorLayout(elementInfo, elementCount);
    }
};

struct DefaultVaryingLayoutRulesImpl : DefaultLayoutRulesImpl
{
    LayoutResourceKind kind;

    DefaultVaryingLayoutRulesImpl(LayoutResourceKind kind)
        : kind(kind)
    {}


    // hook to allow differentiating for input/output
    virtual LayoutResourceKind getKind()
    {
        return kind;
    }

    SimpleLayoutInfo GetScalarLayout(BaseType) override
    {
        // Assume that all scalars take up one "slot"
        return SimpleLayoutInfo(
            getKind(),
            1);
    }

    SimpleLayoutInfo GetVectorLayout(SimpleLayoutInfo, size_t) override
    {
        // Vectors take up one slot by default
        //
        // TODO: some platforms may decide that vectors of `double` need
        // special handling
        return SimpleLayoutInfo(
            getKind(),
            1);
    }
};

struct GLSLVaryingLayoutRulesImpl : DefaultVaryingLayoutRulesImpl
{
    GLSLVaryingLayoutRulesImpl(LayoutResourceKind kind)
        : DefaultVaryingLayoutRulesImpl(kind)
    {}
};

struct HLSLVaryingLayoutRulesImpl : DefaultVaryingLayoutRulesImpl
{
    HLSLVaryingLayoutRulesImpl(LayoutResourceKind kind)
        : DefaultVaryingLayoutRulesImpl(kind)
    {}
};

//

struct GLSLSpecializationConstantLayoutRulesImpl : DefaultLayoutRulesImpl
{
    LayoutResourceKind getKind()
    {
        return LayoutResourceKind::SpecializationConstant;
    }

    SimpleLayoutInfo GetScalarLayout(BaseType) override
    {
        // Assume that all scalars take up one "slot"
        return SimpleLayoutInfo(
            getKind(),
            1);
    }

    SimpleLayoutInfo GetVectorLayout(SimpleLayoutInfo, size_t elementCount) override
    {
        // GLSL doesn't support vectors of specialization constants,
        // but we will assume that, if supported, they would use one slot per element.
        return SimpleLayoutInfo(
            getKind(),
            elementCount);
    }
};

GLSLSpecializationConstantLayoutRulesImpl kGLSLSpecializationConstantLayoutRulesImpl;

//

struct GLSLObjectLayoutRulesImpl : ObjectLayoutRulesImpl
{
    virtual SimpleLayoutInfo GetObjectLayout(ShaderParameterKind) override
    {
        // In Vulkan GLSL, pretty much every object is just a descriptor-table slot.
        // We can refine this method once we support a case where this isn't true.
        return SimpleLayoutInfo(LayoutResourceKind::DescriptorTableSlot, 1);
    }
};
GLSLObjectLayoutRulesImpl kGLSLObjectLayoutRulesImpl;

struct GLSLPushConstantBufferObjectLayoutRulesImpl : GLSLObjectLayoutRulesImpl
{
    virtual SimpleLayoutInfo GetObjectLayout(ShaderParameterKind /*kind*/) override
    {
        // Special-case the layout for a constant-buffer, because we don't
        // want it to allocate a descriptor-table slot
        return SimpleLayoutInfo(LayoutResourceKind::PushConstantBuffer, 1);
    }
};
GLSLPushConstantBufferObjectLayoutRulesImpl kGLSLPushConstantBufferObjectLayoutRulesImpl_;

struct GLSLShaderRecordConstantBufferObjectLayoutRulesImpl : GLSLObjectLayoutRulesImpl
{
    virtual SimpleLayoutInfo GetObjectLayout(ShaderParameterKind /*kind*/) override
    {
        // Special-case the layout for a constant-buffer, because we don't
        // want it to allocate a descriptor-table slot
        return SimpleLayoutInfo(LayoutResourceKind::ShaderRecord, 1);
    }
};
GLSLShaderRecordConstantBufferObjectLayoutRulesImpl kGLSLShaderRecordConstantBufferObjectLayoutRulesImpl_;

struct HLSLObjectLayoutRulesImpl : ObjectLayoutRulesImpl
{
    virtual SimpleLayoutInfo GetObjectLayout(ShaderParameterKind kind) override
    {
        switch( kind )
        {
        case ShaderParameterKind::ConstantBuffer:
            return SimpleLayoutInfo(LayoutResourceKind::ConstantBuffer, 1);

        case ShaderParameterKind::TextureUniformBuffer:
        case ShaderParameterKind::StructuredBuffer:
        case ShaderParameterKind::RawBuffer:
        case ShaderParameterKind::Buffer:
        case ShaderParameterKind::Texture:
            return SimpleLayoutInfo(LayoutResourceKind::ShaderResource, 1);

        case ShaderParameterKind::MutableStructuredBuffer:
        case ShaderParameterKind::MutableRawBuffer:
        case ShaderParameterKind::MutableBuffer:
        case ShaderParameterKind::MutableTexture:
            return SimpleLayoutInfo(LayoutResourceKind::UnorderedAccess, 1);

        case ShaderParameterKind::SamplerState:
            return SimpleLayoutInfo(LayoutResourceKind::SamplerState, 1);

        case ShaderParameterKind::TextureSampler:
        case ShaderParameterKind::MutableTextureSampler:
        case ShaderParameterKind::InputRenderTarget:
            // TODO: how to handle these?
        default:
            SLANG_UNEXPECTED("unhandled shader parameter kind");
            UNREACHABLE_RETURN(SimpleLayoutInfo());
        }
    }
};
HLSLObjectLayoutRulesImpl kHLSLObjectLayoutRulesImpl;

// HACK: Treating ray-tracing input/output as if it was another
// case of varying input/output when it really needs to be
// based on byte storage/layout.
//
struct GLSLRayTracingLayoutRulesImpl : DefaultVaryingLayoutRulesImpl
{
    GLSLRayTracingLayoutRulesImpl(LayoutResourceKind kind)
        : DefaultVaryingLayoutRulesImpl(kind)
    {}
};
struct HLSLRayTracingLayoutRulesImpl : DefaultVaryingLayoutRulesImpl
{
    HLSLRayTracingLayoutRulesImpl(LayoutResourceKind kind)
        : DefaultVaryingLayoutRulesImpl(kind)
    {}
};

Std140LayoutRulesImpl kStd140LayoutRulesImpl;
Std430LayoutRulesImpl kStd430LayoutRulesImpl;
HLSLConstantBufferLayoutRulesImpl kHLSLConstantBufferLayoutRulesImpl;
HLSLStructuredBufferLayoutRulesImpl kHLSLStructuredBufferLayoutRulesImpl;

GLSLVaryingLayoutRulesImpl kGLSLVaryingInputLayoutRulesImpl(LayoutResourceKind::VertexInput);
GLSLVaryingLayoutRulesImpl kGLSLVaryingOutputLayoutRulesImpl(LayoutResourceKind::FragmentOutput);

GLSLRayTracingLayoutRulesImpl kGLSLRayPayloadParameterLayoutRulesImpl(LayoutResourceKind::RayPayload);
GLSLRayTracingLayoutRulesImpl kGLSLCallablePayloadParameterLayoutRulesImpl(LayoutResourceKind::CallablePayload);
GLSLRayTracingLayoutRulesImpl kGLSLHitAttributesParameterLayoutRulesImpl(LayoutResourceKind::HitAttributes);

HLSLVaryingLayoutRulesImpl kHLSLVaryingInputLayoutRulesImpl(LayoutResourceKind::VertexInput);
HLSLVaryingLayoutRulesImpl kHLSLVaryingOutputLayoutRulesImpl(LayoutResourceKind::FragmentOutput);

HLSLRayTracingLayoutRulesImpl kHLSLRayPayloadParameterLayoutRulesImpl(LayoutResourceKind::RayPayload);
HLSLRayTracingLayoutRulesImpl kHLSLCallablePayloadParameterLayoutRulesImpl(LayoutResourceKind::CallablePayload);
HLSLRayTracingLayoutRulesImpl kHLSLHitAttributesParameterLayoutRulesImpl(LayoutResourceKind::HitAttributes);

//

struct GLSLLayoutRulesFamilyImpl : LayoutRulesFamilyImpl
{
    virtual LayoutRulesImpl* getConstantBufferRules() override;
    virtual LayoutRulesImpl* getPushConstantBufferRules() override;
    virtual LayoutRulesImpl* getTextureBufferRules() override;
    virtual LayoutRulesImpl* getVaryingInputRules() override;
    virtual LayoutRulesImpl* getVaryingOutputRules() override;
    virtual LayoutRulesImpl* getSpecializationConstantRules() override;
    virtual LayoutRulesImpl* getShaderStorageBufferRules() override;
    virtual LayoutRulesImpl* getParameterBlockRules() override;

    LayoutRulesImpl* getRayPayloadParameterRules()      override;
    LayoutRulesImpl* getCallablePayloadParameterRules() override;
    LayoutRulesImpl* getHitAttributesParameterRules()   override;

    LayoutRulesImpl* getShaderRecordConstantBufferRules() override;
};

struct HLSLLayoutRulesFamilyImpl : LayoutRulesFamilyImpl
{
    virtual LayoutRulesImpl* getConstantBufferRules() override;
    virtual LayoutRulesImpl* getPushConstantBufferRules() override;
    virtual LayoutRulesImpl* getTextureBufferRules() override;
    virtual LayoutRulesImpl* getVaryingInputRules() override;
    virtual LayoutRulesImpl* getVaryingOutputRules() override;
    virtual LayoutRulesImpl* getSpecializationConstantRules() override;
    virtual LayoutRulesImpl* getShaderStorageBufferRules() override;
    virtual LayoutRulesImpl* getParameterBlockRules() override;

    LayoutRulesImpl* getRayPayloadParameterRules()      override;
    LayoutRulesImpl* getCallablePayloadParameterRules() override;
    LayoutRulesImpl* getHitAttributesParameterRules()   override;

    LayoutRulesImpl* getShaderRecordConstantBufferRules() override;
};

GLSLLayoutRulesFamilyImpl kGLSLLayoutRulesFamilyImpl;
HLSLLayoutRulesFamilyImpl kHLSLLayoutRulesFamilyImpl;


// GLSL cases

LayoutRulesImpl kStd140LayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kStd140LayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kStd430LayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kStd430LayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kGLSLPushConstantLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kStd430LayoutRulesImpl, &kGLSLPushConstantBufferObjectLayoutRulesImpl_,
};

LayoutRulesImpl kGLSLShaderRecordLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kStd430LayoutRulesImpl, &kGLSLShaderRecordConstantBufferObjectLayoutRulesImpl_,
};

LayoutRulesImpl kGLSLVaryingInputLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLVaryingInputLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kGLSLVaryingOutputLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLVaryingOutputLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kGLSLSpecializationConstantLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLSpecializationConstantLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kGLSLRayPayloadParameterLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLRayPayloadParameterLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kGLSLCallablePayloadParameterLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLCallablePayloadParameterLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kGLSLHitAttributesParameterLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLHitAttributesParameterLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

// HLSL cases

LayoutRulesImpl kHLSLConstantBufferLayoutRulesImpl_ = {
    &kHLSLLayoutRulesFamilyImpl, &kHLSLConstantBufferLayoutRulesImpl, &kHLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kHLSLStructuredBufferLayoutRulesImpl_ = {
    &kHLSLLayoutRulesFamilyImpl, &kHLSLStructuredBufferLayoutRulesImpl, &kHLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kHLSLVaryingInputLayoutRulesImpl_ = {
    &kHLSLLayoutRulesFamilyImpl, &kHLSLVaryingInputLayoutRulesImpl, &kHLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kHLSLVaryingOutputLayoutRulesImpl_ = {
    &kHLSLLayoutRulesFamilyImpl, &kHLSLVaryingOutputLayoutRulesImpl, &kHLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kHLSLRayPayloadParameterLayoutRulesImpl_ = {
    &kHLSLLayoutRulesFamilyImpl, &kHLSLRayPayloadParameterLayoutRulesImpl, &kHLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kHLSLCallablePayloadParameterLayoutRulesImpl_ = {
    &kHLSLLayoutRulesFamilyImpl, &kHLSLCallablePayloadParameterLayoutRulesImpl, &kHLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kHLSLHitAttributesParameterLayoutRulesImpl_ = {
    &kHLSLLayoutRulesFamilyImpl, &kHLSLHitAttributesParameterLayoutRulesImpl, &kHLSLObjectLayoutRulesImpl,
};

//

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getConstantBufferRules()
{
    return &kStd140LayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getParameterBlockRules()
{
    // TODO: actually pick something appropriate
    return &kStd140LayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getPushConstantBufferRules()
{
    return &kGLSLPushConstantLayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getShaderRecordConstantBufferRules()
{
    return &kGLSLShaderRecordLayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getTextureBufferRules()
{
    return nullptr;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getVaryingInputRules()
{
    return &kGLSLVaryingInputLayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getVaryingOutputRules()
{
    return &kGLSLVaryingOutputLayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getSpecializationConstantRules()
{
    return &kGLSLSpecializationConstantLayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getShaderStorageBufferRules()
{
    return &kStd430LayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getRayPayloadParameterRules()
{
    return &kGLSLRayPayloadParameterLayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getCallablePayloadParameterRules()
{
    return &kGLSLCallablePayloadParameterLayoutRulesImpl_;
}

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getHitAttributesParameterRules()
{
    return &kGLSLHitAttributesParameterLayoutRulesImpl_;
}

//

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getConstantBufferRules()
{
    return &kHLSLConstantBufferLayoutRulesImpl_;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getParameterBlockRules()
{
    // TODO: actually pick something appropriate...
    return &kHLSLConstantBufferLayoutRulesImpl_;
}


LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getPushConstantBufferRules()
{
    return &kHLSLConstantBufferLayoutRulesImpl_;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getShaderRecordConstantBufferRules()
{
    return &kHLSLConstantBufferLayoutRulesImpl_;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getTextureBufferRules()
{
    return nullptr;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getVaryingInputRules()
{
    return &kHLSLVaryingInputLayoutRulesImpl_;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getVaryingOutputRules()
{
    return &kHLSLVaryingOutputLayoutRulesImpl_;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getSpecializationConstantRules()
{
    return nullptr;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getShaderStorageBufferRules()
{
    return nullptr;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getRayPayloadParameterRules()
{
    return &kHLSLRayPayloadParameterLayoutRulesImpl_;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getCallablePayloadParameterRules()
{
    return &kHLSLCallablePayloadParameterLayoutRulesImpl_;
}

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getHitAttributesParameterRules()
{
    return &kHLSLHitAttributesParameterLayoutRulesImpl_;
}



//

LayoutRulesImpl* GetLayoutRulesImpl(LayoutRule rule)
{
    switch (rule)
    {
    case LayoutRule::Std140:                return &kStd140LayoutRulesImpl_;
    case LayoutRule::Std430:                return &kStd430LayoutRulesImpl_;
    case LayoutRule::HLSLConstantBuffer:    return &kHLSLConstantBufferLayoutRulesImpl_;
    case LayoutRule::HLSLStructuredBuffer:  return &kHLSLStructuredBufferLayoutRulesImpl_;
    default:
        return nullptr;
    }
}

LayoutRulesFamilyImpl* getDefaultLayoutRulesFamilyForTarget(TargetRequest* targetReq)
{
    switch (targetReq->target)
    {
    case CodeGenTarget::HLSL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXILAssembly:
        return &kHLSLLayoutRulesFamilyImpl;

    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        return &kGLSLLayoutRulesFamilyImpl;

    default:
        return nullptr;
    }
}

TypeLayoutContext getInitialLayoutContextForTarget(TargetRequest* targetReq)
{
    LayoutRulesFamilyImpl* rulesFamily = getDefaultLayoutRulesFamilyForTarget(targetReq);

    TypeLayoutContext context;
    context.targetReq = targetReq;
    context.rules = nullptr;
    context.matrixLayoutMode = targetReq->getDefaultMatrixLayoutMode();

    if( rulesFamily )
    {
        context.rules = rulesFamily->getConstantBufferRules();
    }

    return context;
}


static LayoutSize GetElementCount(RefPtr<IntVal> val)
{
    // Lack of a size indicates an unbounded array.
    if(!val)
        return LayoutSize::infinite();

    if (auto constantVal = val.As<ConstantIntVal>())
    {
        return LayoutSize(LayoutSize::RawValue(constantVal->value));
    }
    else if( auto varRefVal = val.As<GenericParamIntVal>() )
    {
        // TODO: We want to treat the case where the number of
        // elements in an array depends on a generic parameter
        // much like the case where the number of elements is
        // unbounded, *but* we can't just blindly do that because
        // an API might disallow unbounded arrays in various
        // cases where a generic bound might work (because
        // any concrete specialization will have a finite bound...)
        //
        return 0;
    }
    SLANG_UNEXPECTED("unhandled integer literal kind");
    UNREACHABLE_RETURN(LayoutSize(0));
}

bool IsResourceKind(LayoutResourceKind kind)
{
    switch (kind)
    {
    case LayoutResourceKind::None:
    case LayoutResourceKind::Uniform:
        return false;

    default:
        return true;
    }

}

SimpleLayoutInfo GetSimpleLayoutImpl(
    SimpleLayoutInfo        info,
    RefPtr<Type>            type,
    LayoutRulesImpl*        rules,
    RefPtr<TypeLayout>*     outTypeLayout)
{
    if (outTypeLayout)
    {
        RefPtr<TypeLayout> typeLayout = new TypeLayout();
        *outTypeLayout = typeLayout;

        typeLayout->type = type;
        typeLayout->rules = rules;

        typeLayout->uniformAlignment = info.alignment;

        typeLayout->addResourceUsage(info.kind, info.size);
    }

    return info;
}

static SimpleLayoutInfo getParameterGroupLayoutInfo(
    RefPtr<ParameterGroupType>  type,
    LayoutRulesImpl*            rules)
{
    if( type->As<ConstantBufferType>() )
    {
        return rules->GetObjectLayout(ShaderParameterKind::ConstantBuffer);
    }
    else if( type->As<TextureBufferType>() )
    {
        return rules->GetObjectLayout(ShaderParameterKind::TextureUniformBuffer);
    }
    else if( type->As<GLSLShaderStorageBufferType>() )
    {
        return rules->GetObjectLayout(ShaderParameterKind::ShaderStorageBuffer);
    }
    else if (type->As<ParameterBlockType>())
    {
        // Note: we default to consuming zero register spces here, because
        // a parameter block might not contain anything (or all it contains
        // is other blocks), and so it won't get a space allocated.
        //
        // This choice *also* means that in the case where we don't actually
        // want to allocate register spaces to blocks at all, we haven't
        // committed to that choice here.
        //
        // TODO: wouldn't it be any different to just allocate this
        // as an empty `SimpleLayoutInfo` of any other kind?
        return SimpleLayoutInfo(LayoutResourceKind::RegisterSpace, 0);
    }

    // TODO: the vertex-input and fragment-output cases should
    // only actually apply when we are at the appropriate stage in
    // the pipeline...
    else if( type->As<GLSLInputParameterGroupType>() )
    {
        return SimpleLayoutInfo(LayoutResourceKind::VertexInput, 0);
    }
    else if( type->As<GLSLOutputParameterGroupType>() )
    {
        return SimpleLayoutInfo(LayoutResourceKind::FragmentOutput, 0);
    }
    else
    {
        SLANG_UNEXPECTED("unhandled parameter block type");
        UNREACHABLE_RETURN(SimpleLayoutInfo());
    }
}

RefPtr<TypeLayout> createTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type);

static bool isOpenGLTarget(TargetRequest*)
{
    // We aren't officially supporting OpenGL right now
    return false;
}

bool isD3DTarget(TargetRequest* targetReq)
{
    switch( targetReq->target )
    {
    case CodeGenTarget::HLSL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXILAssembly:
        return true;

    default:
        return false;
    }
}

bool isKhronosTarget(TargetRequest* targetReq)
{
    switch( targetReq->target )
    {
    default:
        return false;

    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        return true;
    }
}

static bool isD3D11Target(TargetRequest*)
{
    // We aren't officially supporting D3D11 right now
    return false;
}

static bool isD3D12Target(TargetRequest* targetReq)
{
    // We are currently only officially supporting D3D12
    return isD3DTarget(targetReq);
}


static bool isSM5OrEarlier(TargetRequest* targetReq)
{
    if(!isD3DTarget(targetReq))
        return false;

    auto profile = targetReq->targetProfile;

    if(profile.getFamily() == ProfileFamily::DX)
    {
        if(profile.GetVersion() <= ProfileVersion::DX_5_0)
            return true;
    }

    return false;
}

static bool isSM5_1OrLater(TargetRequest* targetReq)
{
    if(!isD3DTarget(targetReq))
        return false;

    auto profile = targetReq->targetProfile;

    if(profile.getFamily() == ProfileFamily::DX)
    {
        if(profile.GetVersion() >= ProfileVersion::DX_5_1)
            return true;
    }

    return false;
}

static bool isVulkanTarget(TargetRequest* targetReq)
{
    // For right now, any Khronos-related target is assumed
    // to be a Vulkan target.
    return isKhronosTarget(targetReq);
}

static bool shouldAllocateRegisterSpaceForParameterBlock(
    TypeLayoutContext const&  context)
{
    auto targetReq = context.targetReq;

    // We *never* want to use register spaces/sets under
    // OpenGL, D3D11, or for Shader Model 5.0 or earlier.
    if(isOpenGLTarget(targetReq) || isD3D11Target(targetReq) || isSM5OrEarlier(targetReq))
        return false;

    // If we know that we are targetting Vulkan, then
    // the only way to effectively use parameter blocks
    // is by using descriptor sets.
    if(isVulkanTarget(targetReq))
        return true;

    // If none of the above passed, then it seems like we
    // are generating code for D3D12, and using SM5.1 or later.
    // We will use a register space for parameter blocks *if*
    // the target options tell us to:
    if( isD3D12Target(targetReq) && isSM5_1OrLater(targetReq) )
    {
        return true;
    }

    return false;
}

// Given an existing type layout `oldTypeLayout`, apply offsets
// to any contained fields based on the resource infos in `offsetTypeLayout`
// *and* the usage implied by `offsetLayout`
RefPtr<TypeLayout> applyOffsetToTypeLayout(
    RefPtr<TypeLayout>  oldTypeLayout,
    RefPtr<TypeLayout>  offsetTypeLayout)
{
    // There is no need to apply offsets if the old type and the offset
    // don't share any resource infos in common.
    bool anyHit = false;
    for (auto oldResInfo : oldTypeLayout->resourceInfos)
    {
        if (auto offsetResInfo = offsetTypeLayout->FindResourceInfo(oldResInfo.kind))
        {
            anyHit = true;
            break;
        }
    }

    if (!anyHit)
        return oldTypeLayout;

    RefPtr<TypeLayout> newTypeLayout;
    if (auto oldStructTypeLayout = oldTypeLayout.As<StructTypeLayout>())
    {
        RefPtr<StructTypeLayout> newStructTypeLayout = new StructTypeLayout();
        newStructTypeLayout->type = oldStructTypeLayout->type;
        newStructTypeLayout->uniformAlignment = oldStructTypeLayout->uniformAlignment;

        Dictionary<VarLayout*, VarLayout*> mapOldFieldToNew;

        for (auto oldField : oldStructTypeLayout->fields)
        {
            RefPtr<VarLayout> newField = new VarLayout();
            newField->varDecl = oldField->varDecl;
            newField->typeLayout = oldField->typeLayout;
            newField->flags = oldField->flags;
            newField->semanticIndex = oldField->semanticIndex;
            newField->semanticName = oldField->semanticName;
            newField->stage = oldField->stage;
            newField->systemValueSemantic = oldField->systemValueSemantic;
            newField->systemValueSemanticIndex = oldField->systemValueSemanticIndex;


            for (auto oldResInfo : oldField->resourceInfos)
            {
                auto newResInfo = newField->findOrAddResourceInfo(oldResInfo.kind);
                newResInfo->index = oldResInfo.index;
                newResInfo->space = oldResInfo.space;
                if (auto offsetResInfo = offsetTypeLayout->FindResourceInfo(oldResInfo.kind))
                {
                    // We should not be trying to offset things by an infinite amount,
                    // since that would leave all the indices undefined.
                    SLANG_RELEASE_ASSERT(offsetResInfo->count.isFinite());
                    newResInfo->index += offsetResInfo->count.getFiniteValue();
                }
            }

            newStructTypeLayout->fields.Add(newField);

            mapOldFieldToNew.Add(oldField.Ptr(), newField.Ptr());
        }

        for (auto entry : oldStructTypeLayout->mapVarToLayout)
        {
            VarLayout* newFieldLayout = nullptr;
            if (mapOldFieldToNew.TryGetValue(entry.Value.Ptr(), newFieldLayout))
            {
                newStructTypeLayout->mapVarToLayout.Add(entry.Key, newFieldLayout);
            }
        }

        newTypeLayout = newStructTypeLayout;
    }
    else
    {
        // TODO: need to handle other cases here
        return oldTypeLayout;
    }

    // No matter what replacement we plug in for the element type, we need to copy
    // over its resource usage:
    for (auto oldResInfo : oldTypeLayout->resourceInfos)
    {
        auto newResInfo = newTypeLayout->findOrAddResourceInfo(oldResInfo.kind);
        newResInfo->count = oldResInfo.count;
    }

    return newTypeLayout;
}

RefPtr<ParameterGroupTypeLayout>
createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType,
    SimpleLayoutInfo            parameterGroupInfo,
    RefPtr<TypeLayout>          rawElementTypeLayout)
{
    auto parameterGroupRules = context.rules;

    RefPtr<ParameterGroupTypeLayout> typeLayout = new ParameterGroupTypeLayout();
    typeLayout->type = parameterGroupType;
    typeLayout->rules = parameterGroupRules;

    RefPtr<TypeLayout> containerTypeLayout = new TypeLayout();
    containerTypeLayout->type = parameterGroupType;
    containerTypeLayout->rules = parameterGroupRules;

    // The layout of the constant buffer if it gets stored
    // in another constant buffer is just what we computed
    // originally (which should be a single binding "slot"
    // and hence no uniform data).
    // 
    SLANG_RELEASE_ASSERT(parameterGroupInfo.kind != LayoutResourceKind::Uniform);
    typeLayout->uniformAlignment = 1;
    containerTypeLayout->uniformAlignment = 1;

    // TODO(tfoley): There is a subtle question here of whether
    // a constant buffer declaration that then contains zero
    // bytes of uniform data should actually allocate a CB
    // binding slot. For now I'm going to try to ignore it,
    // but handling this robustly could let other code
    // simply handle the "global scope" as a giant outer
    // CB declaration...

    // Make sure that we allocate resource usage for the
    // parameter block itself.
    if( parameterGroupInfo.size != 0 )
    {
        containerTypeLayout->addResourceUsage(
            parameterGroupInfo.kind,
            parameterGroupInfo.size);
    }

    // There are several different cases that need to be handled here,
    // depending on whether we have a `ParameterBlock`, a `ConstantBuffer`,
    // or some other kind of parameter group. Furthermore, in the
    // `ParameterBlock` case, we need to deal with different layout
    // rules depending on whether a block should map to a register `space`
    // in HLSL or not.

    // Check if we are working with a parameter block...
    auto parameterBlockType = parameterGroupType ? parameterGroupType->As<ParameterBlockType>() : nullptr;


    // Check if we have a parameter block *and* it should be
    // allocated into its own register space(s)
    bool ownRegisterSpace = false;
    if (parameterBlockType)
    {
        // Should we allocate this block its own regsiter space?
        if( shouldAllocateRegisterSpaceForParameterBlock(context) )
        {
            ownRegisterSpace = true;
        }

        // If we need a register space, then maybe allocate one.
        if( ownRegisterSpace )
        {
            // The basic logic here is that if the parameter block only
            // contains other parameter blocks (which themselves have
            // their own register spaces), then we don't need to
            // allocate *yet another* register space for the block.

            bool needsARegisterSpace = false;
            for( auto elementResourceInfo : rawElementTypeLayout->resourceInfos )
            {
                if(elementResourceInfo.kind != LayoutResourceKind::RegisterSpace)
                {
                    needsARegisterSpace = true;
                    break;
                }
            }

            // If we determine that a register space is needed, then add one here.
            if( needsARegisterSpace )
            {
                typeLayout->addResourceUsage(LayoutResourceKind::RegisterSpace, 1);
            }
        }

        // Next, we check if the parameter block has any uniform data, since
        // that means we need to allocate a constant-buffer binding for it.
        bool anyUniformData = false;
        if(auto elementUniformInfo = rawElementTypeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
        {
            if( elementUniformInfo->count != 0 )
            {
                // We have a non-zero number of bytes of uniform data here.
                anyUniformData = true;
            }
        }
        if( anyUniformData )
        {
            // We need to ensure that the block itself consumes at least one "register" for its
            // constant buffer part.
            auto cbUsage = parameterGroupRules->GetObjectLayout(ShaderParameterKind::ConstantBuffer);
            containerTypeLayout->addResourceUsage(cbUsage.kind, cbUsage.size);
        }
    }

    // The layout for the element type was computed without any knowledge
    // of what resources the parent type was going to consume; we now
    // need to go through and offset that any starting locations (e.g.,
    // in nested `StructTypeLayout`s) based on what we allocated to
    // the parent.
    //
    // Note: at the moment, constant buffers apply their own offsetting
    // logic elsewhere, so we need to only do this logic for parameter blocks
    RefPtr<TypeLayout> offsetTypeLayout = applyOffsetToTypeLayout(rawElementTypeLayout, containerTypeLayout);
    typeLayout->offsetElementTypeLayout = offsetTypeLayout;


    RefPtr<VarLayout> containerVarLayout = new VarLayout();
    containerVarLayout->typeLayout = containerTypeLayout;
    for( auto typeResInfo : containerTypeLayout->resourceInfos )
    {
        containerVarLayout->findOrAddResourceInfo(typeResInfo.kind);
    }
    typeLayout->containerVarLayout = containerVarLayout;

    // We will construct a dummy variable layout to represent the offsettting
    // that needs to be applied to the element type to put it after the
    // container.
    RefPtr<VarLayout> elementVarLayout = new VarLayout();
    elementVarLayout->typeLayout = rawElementTypeLayout;
    for( auto elementTypeResInfo : rawElementTypeLayout->resourceInfos )
    {
        auto kind = elementTypeResInfo.kind;
        auto elementVarResInfo = elementVarLayout->findOrAddResourceInfo(kind);
        if( auto containerTypeResInfo = containerTypeLayout->FindResourceInfo(kind) )
        {
            SLANG_RELEASE_ASSERT(containerTypeResInfo->count.isFinite());
            elementVarResInfo->index += containerTypeResInfo->count.getFiniteValue();
        }
    }
    typeLayout->elementVarLayout = elementVarLayout;

    if (ownRegisterSpace)
    {
        // A parameter block type that gets its own register space will only
        // include resource usage from the element type when it itself consumes
        // while register spaces.
        if (auto elementResInfo = rawElementTypeLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace))
        {
            typeLayout->addResourceUsage(*elementResInfo);
        }
    }
    else
    {
        // If the parameter block is *not* getting its own regsiter space, then
        // it needs to include the resource usage from the "container" type, plus
        // any relevant resource usage for the element type.

        // We start by accumulating any resource usage from the container.
        for (auto containerResourceInfo : containerTypeLayout->resourceInfos)
        {
            typeLayout->addResourceUsage(containerResourceInfo);
        }

        // Now we will (possibly) accumulate the resources used by the element
        // type into the resources used by the parameter group. The reason
        // this is "possibly" is because, e.g., a `ConstantBuffer<Foo>` should
        // not report itself as consuming `sizeof(Foo)` bytes of uniform data,
        // or else it would mess up layout for any type that contains the
        // constant buffer.
        for( auto elementResourceInfo : rawElementTypeLayout->resourceInfos )
        {
            switch( elementResourceInfo.kind )
            {
            case LayoutResourceKind::Uniform:
                // Uniform resource usages will always be hidden.
                break;

            default:
                // All other register types will not be hidden,
                // since we aren't in the case where the parameter group
                // gets its own register space.
                typeLayout->addResourceUsage(elementResourceInfo);
                break;
            }
        }
    }

    return typeLayout;
}

RefPtr<ParameterGroupTypeLayout>
createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType,
    LayoutRulesImpl*            parameterGroupRules,
    SimpleLayoutInfo            parameterGroupInfo,
    RefPtr<TypeLayout>          elementTypeLayout)
{
    return createParameterGroupTypeLayout(
        context.with(parameterGroupRules).with(context.targetReq->getDefaultMatrixLayoutMode()),
        parameterGroupType,
        parameterGroupInfo,
        elementTypeLayout);
}

RefPtr<ParameterGroupTypeLayout>
createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType,
    RefPtr<Type>                elementType,
    LayoutRulesImpl*            elementTypeRules)
{
    auto parameterGroupRules = context.rules;

    // First compute resource usage of the block itself.
    // For now we assume that the layout of the block can
    // always be described in a `SimpleLayoutInfo` (only
    // a single resource kind consumed).
    SimpleLayoutInfo info;
    if (parameterGroupType)
    {
        info = getParameterGroupLayoutInfo(
            parameterGroupType,
            parameterGroupRules);
    }
    else
    {
        // If there is no concrete type, then it seems like we are
        // being asked to compute layout for the global scope
        info = parameterGroupRules->GetObjectLayout(ShaderParameterKind::ConstantBuffer);
    }

    // Now compute a layout for the elements of the parameter block.
    // Note that we need to be careful and deal with the case where
    // the elements of the block use the same resource kind consumed
    // by the block itself.

    auto elementTypeLayout = createTypeLayout(
        context.with(elementTypeRules),
        elementType);

    return createParameterGroupTypeLayout(
        context,
        parameterGroupType,
        info,
        elementTypeLayout);
}

LayoutRulesImpl* getParameterBufferElementTypeLayoutRules(
    RefPtr<ParameterGroupType>  parameterGroupType,
    LayoutRulesImpl*            rules)
{
    if( parameterGroupType->As<ConstantBufferType>() )
    {
        return rules->getLayoutRulesFamily()->getConstantBufferRules();
    }
    else if( parameterGroupType->As<TextureBufferType>() )
    {
        return rules->getLayoutRulesFamily()->getTextureBufferRules();
    }
    else if( parameterGroupType->As<GLSLInputParameterGroupType>() )
    {
        return rules->getLayoutRulesFamily()->getVaryingInputRules();
    }
    else if( parameterGroupType->As<GLSLOutputParameterGroupType>() )
    {
        return rules->getLayoutRulesFamily()->getVaryingOutputRules();
    }
    else if( parameterGroupType->As<GLSLShaderStorageBufferType>() )
    {
        return rules->getLayoutRulesFamily()->getShaderStorageBufferRules();
    }
    else if (parameterGroupType->As<ParameterBlockType>())
    {
        return rules->getLayoutRulesFamily()->getParameterBlockRules();
    }
    else
    {
        SLANG_UNEXPECTED("uhandled parameter block type");
        return nullptr;
    }
}

RefPtr<ParameterGroupTypeLayout>
createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType)
{
    auto parameterGroupRules = context.rules;

    // Determine the layout rules to use for the contents of the block
    auto elementTypeRules = getParameterBufferElementTypeLayoutRules(
        parameterGroupType,
        parameterGroupRules);

    auto elementType = parameterGroupType->elementType;

    return createParameterGroupTypeLayout(
        context,
        parameterGroupType,
        elementType,
        elementTypeRules);
}

// Create a type layout for a structured buffer type.
RefPtr<StructuredBufferTypeLayout>
createStructuredBufferTypeLayout(
    TypeLayoutContext const&    context,
    ShaderParameterKind         kind,
    RefPtr<Type>                structuredBufferType,
    RefPtr<TypeLayout>          elementTypeLayout)
{
    auto rules = context.rules;
    auto info = rules->GetObjectLayout(kind);

    auto typeLayout = new StructuredBufferTypeLayout();

    typeLayout->type = structuredBufferType;
    typeLayout->rules = rules;

    typeLayout->elementTypeLayout = elementTypeLayout;

    typeLayout->uniformAlignment = info.alignment;
    SLANG_RELEASE_ASSERT(!typeLayout->FindResourceInfo(LayoutResourceKind::Uniform));
    SLANG_RELEASE_ASSERT(typeLayout->uniformAlignment == 1);

    if( info.size != 0 )
    {
        typeLayout->addResourceUsage(info.kind, info.size);
    }

    // Note: for now we don't deal with the case of a structured
    // buffer that might contain anything other than "uniform" data,
    // because there really isn't a way to implement that.

    return typeLayout;
}

// Create a type layout for a structured buffer type.
RefPtr<StructuredBufferTypeLayout>
createStructuredBufferTypeLayout(
    TypeLayoutContext const&    context,
    ShaderParameterKind         kind,
    RefPtr<Type>                structuredBufferType,
    RefPtr<Type>                elementType)
{
    // TODO(tfoley): we should be looking up the appropriate rules
    // via the `LayoutRulesFamily` in use here...
    auto structuredBufferLayoutRules = GetLayoutRulesImpl(
        LayoutRule::HLSLStructuredBuffer);

    // Create and save type layout for the buffer contents.
    auto elementTypeLayout = CreateTypeLayout(
        context.with(structuredBufferLayoutRules),
        elementType.Ptr());

    return createStructuredBufferTypeLayout(
        context,
        kind,
        structuredBufferType,
        elementTypeLayout);

}

SimpleLayoutInfo GetLayoutImpl(
    TypeLayoutContext const&    context,
    Type*                       type,
    RefPtr<TypeLayout>*         outTypeLayout);

SimpleLayoutInfo GetLayoutImpl(
    TypeLayoutContext const&    context,
    Type*                       type,
    RefPtr<TypeLayout>*         outTypeLayout,
    Decl*                       declForModifiers)
{
    TypeLayoutContext subContext = context;

    if (declForModifiers)
    {
        if (declForModifiers->HasModifier<RowMajorLayoutModifier>())
            subContext.matrixLayoutMode = kMatrixLayoutMode_RowMajor;

        if (declForModifiers->HasModifier<ColumnMajorLayoutModifier>())
            subContext.matrixLayoutMode = kMatrixLayoutMode_ColumnMajor;

        // TODO: really need to look for other modifiers that affect
        // layout, such as GLSL `std140`.
    }

    return GetLayoutImpl(subContext, type, outTypeLayout);
}

int findGenericParam(List<RefPtr<GenericParamLayout>> & genericParameters, GlobalGenericParamDecl * decl)
{
    return (int)genericParameters.FindFirst([=](RefPtr<GenericParamLayout> & x) {return x->decl.Ptr() == decl; });
}

// When constructing a new var layout from an existing one,
// copy fields to the new var from the old.
void copyVarLayoutFields(
    VarLayout* dstVarLayout,
    VarLayout* srcVarLayout)
{
    dstVarLayout->varDecl = srcVarLayout->varDecl;
    dstVarLayout->typeLayout = srcVarLayout->typeLayout;
    dstVarLayout->flags = srcVarLayout->flags;
    dstVarLayout->systemValueSemantic = srcVarLayout->systemValueSemantic;
    dstVarLayout->systemValueSemanticIndex = srcVarLayout->systemValueSemanticIndex;
    dstVarLayout->semanticName = srcVarLayout->semanticName;
    dstVarLayout->semanticIndex = srcVarLayout->semanticIndex;
    dstVarLayout->stage = srcVarLayout->stage;
    dstVarLayout->resourceInfos = srcVarLayout->resourceInfos;
}

// When constructing a new type layout from an existing one,
// copy fields to the new type from the old.
void copyTypeLayoutFields(
    TypeLayout* dstTypeLayout,
    TypeLayout* srcTypeLayout)
{
    dstTypeLayout->type = srcTypeLayout->type;
    dstTypeLayout->rules = srcTypeLayout->rules;
    dstTypeLayout->uniformAlignment = srcTypeLayout->uniformAlignment;
    dstTypeLayout->resourceInfos = srcTypeLayout->resourceInfos;
}

// Does this layout resource kind require adjustment when used in
// an array-of-structs fashion?
bool doesResourceRequireAdjustmentForArrayOfStructs(LayoutResourceKind kind)
{
    switch( kind )
    {
    case LayoutResourceKind::ConstantBuffer:
    case LayoutResourceKind::ShaderResource:
    case LayoutResourceKind::UnorderedAccess:
    case LayoutResourceKind::SamplerState:
        return true;

    default:
        return false;
    }
}

// Given the type layout for an element of an array, apply any adjustments required
// based on the element count of the array.
//
// The particular case where this matters is when we have an array of an aggregate
// type that contains resources, since each resource field might need to be at
// a different offset than we would otherwise expect.
//
// For example, given:
//
//      struct Foo { Texture2D a; Texture2D b; }
//
// if we just write:
//
//      Foo foo;
//
// it gets split into:
//
//      Texture2D foo_a;
//      Texture2D foo_b;
//
// we expect `foo_a` to get `register(t0)` and
// `foo_b` to get `register(t1)`. However, if we instead have an array:
//
//      Foo foo[10];
//
// then we expect it to be split into:
//
//      Texture2D foo_a[8];
//      Texture2D foo_b[8];
//
// and then we expect `foo_b` to get `register(t8)`, rather
// than `register(t1)`.
//
static RefPtr<TypeLayout> maybeAdjustLayoutForArrayElementType(
    RefPtr<TypeLayout>  originalTypeLayout,
    LayoutSize          elementCount,
    UInt&               ioAdditionalSpacesNeeded)
{
    // We will start by looking for cases that we can reject out
    // of hand.

    // If the original element type layout doesn't use any
    // resource registers, then we are fine.
    bool anyResource = false;
    for( auto resInfo : originalTypeLayout->resourceInfos )
    {
        if( doesResourceRequireAdjustmentForArrayOfStructs(resInfo.kind) )
        {
            anyResource = true;
            break;
        }
    }
    if(!anyResource)
        return originalTypeLayout;

    // Let's look at the type layout we have, and see if there is anything
    // that we need to do with it.
    //
    if( auto originalArrayTypeLayout = originalTypeLayout.As<ArrayTypeLayout>() )
    {
        // The element type is itself an array, so we'll need to adjust
        // *its* element type accordingly.
        //
        // We adjust the already-adjusted element type of the inner
        // array type, so that we pick up adjustments already made:
        auto originalInnerElementTypeLayout = originalArrayTypeLayout->elementTypeLayout;
        auto adjustedInnerElementTypeLayout = maybeAdjustLayoutForArrayElementType(
            originalInnerElementTypeLayout,
            elementCount,
            ioAdditionalSpacesNeeded);

        // If nothing needed to be changed on the inner element type,
        // then we are done.
        if(adjustedInnerElementTypeLayout == originalInnerElementTypeLayout)
            return originalTypeLayout;

        // Otherwise, we need to construct a new array type layout
        RefPtr<ArrayTypeLayout> adjustedArrayTypeLayout = new ArrayTypeLayout();
        adjustedArrayTypeLayout->originalElementTypeLayout = originalInnerElementTypeLayout;
        adjustedArrayTypeLayout->elementTypeLayout = adjustedInnerElementTypeLayout;
        adjustedArrayTypeLayout->uniformStride = originalArrayTypeLayout->uniformStride;

        copyTypeLayoutFields(adjustedArrayTypeLayout, originalArrayTypeLayout);

        return adjustedArrayTypeLayout;
    }
    else if(auto originalParameterGroupTypeLayout = originalTypeLayout.As<ParameterGroupTypeLayout>() )
    {
        auto originalInnerElementTypeLayout = originalParameterGroupTypeLayout->elementVarLayout->typeLayout;
        auto adjustedInnerElementTypeLayout = maybeAdjustLayoutForArrayElementType(
            originalInnerElementTypeLayout,
            elementCount,
            ioAdditionalSpacesNeeded);

        // If nothing needed to be changed on the inner element type,
        // then we are done.
        if(adjustedInnerElementTypeLayout == originalInnerElementTypeLayout)
            return originalTypeLayout;

        // TODO: actually adjust the element type, and create all the required bits and
        // pieces of layout.

        SLANG_UNIMPLEMENTED_X("array of parameter group");
        UNREACHABLE_RETURN(originalTypeLayout);
    }
    else if(auto originalStructTypeLayout = originalTypeLayout.As<StructTypeLayout>() )
    {
        UInt fieldCount = originalStructTypeLayout->fields.Count();

        // Empty struct? Bail out.
        if(fieldCount == 0)
            return originalTypeLayout;

        RefPtr<StructTypeLayout> adjustedStructTypeLayout = new StructTypeLayout();
        copyTypeLayoutFields(adjustedStructTypeLayout, originalStructTypeLayout);

        // If the array type adjustment forces us to give a whole space to
        // one or more fields, then we'll need to carefully compute the space
        // index for each field as we go.
        //
        LayoutSize nextSpaceIndex = 0;

        Dictionary<RefPtr<VarLayout>, RefPtr<VarLayout>> mapOriginalFieldToAdjusted;
        for( auto originalField : originalStructTypeLayout->fields )
        {
            auto originalFieldTypeLayout = originalField->typeLayout;

            LayoutSize originalFieldSpaceCount = 0;
            if(auto resInfo = originalFieldTypeLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace))
                originalFieldSpaceCount = resInfo->count;

            // Compute the adjusted type for the field
            UInt fieldAdditionalSpaces = 0;
            auto adjustedFieldTypeLayout = maybeAdjustLayoutForArrayElementType(
                originalFieldTypeLayout,
                elementCount,
                fieldAdditionalSpaces);

            LayoutSize adjustedFieldSpaceCount = originalFieldSpaceCount + fieldAdditionalSpaces;

            LayoutSize spaceOffsetForField = nextSpaceIndex;
            nextSpaceIndex += adjustedFieldSpaceCount;

            ioAdditionalSpacesNeeded += fieldAdditionalSpaces;

            // Create an adjusted field variable, that is mostly
            // a clone of the original field (just with our
            // adjusted type in place).
            RefPtr<VarLayout> adjustedField = new VarLayout();
            copyVarLayoutFields(adjustedField, originalField);
            adjustedField->typeLayout = adjustedFieldTypeLayout;

            // We will now walk through the resource usage for
            // the adjusted field, and try to figure out what
            // to do with it all.
            //
            for(auto& resInfo : adjustedField->resourceInfos )
            {
                if( doesResourceRequireAdjustmentForArrayOfStructs(resInfo.kind) )
                {
                    if(elementCount.isFinite())
                    {
                        // If the array size is finite, then the field's index/offset
                        // is just going to be strided by the array size since we
                        // are effectively doing AoS to SoA conversion.
                        //
                        resInfo.index *= elementCount.getFiniteValue();
                    }
                    else
                    {
                        // If we are making an unbounded array, then a `struct`
                        // field with resource type will turn into its own space,
                        // and it will start at regsiter zero in that space.
                        //
                        resInfo.index = 0;
                        resInfo.space = spaceOffsetForField.getFiniteValue();
                    }
                }
            }

            adjustedStructTypeLayout->fields.Add(adjustedField);

            mapOriginalFieldToAdjusted.Add(originalField, adjustedField);
        }

        for( auto p : originalStructTypeLayout->mapVarToLayout )
        {
            Decl* key = p.Key;
            RefPtr<VarLayout> originalVal = p.Value;
            RefPtr<VarLayout> adjustedVal;
            if( mapOriginalFieldToAdjusted.TryGetValue(originalVal, adjustedVal) )
            {
                adjustedStructTypeLayout->mapVarToLayout.Add(key, adjustedVal);
            }
        }

        return adjustedStructTypeLayout;
    }
    else
    {
        // In the leaf case, we must have a field that used up some resource
        // that requires adjustment. Because there is no sub-structure to work
        // with, we can just return the type layout as-is, but we also want
        // to make a note that this value should consume an additional register
        // space *if* the element count is unbounded.
        if( elementCount.isInfinite() )
        {
            ioAdditionalSpacesNeeded++;
        }

        return originalTypeLayout;
    }
}

SimpleLayoutInfo GetLayoutImpl(
    TypeLayoutContext const&    context,
    Type*                       type,
    RefPtr<TypeLayout>*         outTypeLayout)
{
    auto rules = context.rules;

    if (auto parameterGroupType = type->As<ParameterGroupType>())
    {
        // If the user is just interested in uniform layout info,
        // then this is easy: a `ConstantBuffer<T>` is really no
        // different from a `Texture2D<U>` in terms of how it
        // should be handled as a member of a container.
        //
        auto info = getParameterGroupLayoutInfo(parameterGroupType, rules);

        // The more interesting case, though, is when the user
        // is requesting us to actually create a `TypeLayout`,
        // since in that case we need to:
        //
        // 1. Compute a layout for the data inside the constant
        //    buffer, including offsets, etc.
        //
        // 2. Compute information about any object types inside
        //    the constant buffer, which need to be surfaces out
        //    to the top level.
        //
        if (outTypeLayout)
        {
            *outTypeLayout = createParameterGroupTypeLayout(
                context,
                parameterGroupType);
        }

        return info;
    }
    else if (auto samplerStateType = type->As<SamplerStateType>())
    {
        return GetSimpleLayoutImpl(
            rules->GetObjectLayout(ShaderParameterKind::SamplerState),
            type,
            rules,
            outTypeLayout);
    }
    else if (auto textureType = type->As<TextureType>())
    {
        // TODO: the logic here should really be defined by the rules,
        // and not at this top level...
        ShaderParameterKind kind;
        switch( textureType->getAccess() )
        {
        default:
            kind = ShaderParameterKind::MutableTexture;
            break;

        case SLANG_RESOURCE_ACCESS_READ:
            kind = ShaderParameterKind::Texture;
            break;
        }

        return GetSimpleLayoutImpl(
            rules->GetObjectLayout(kind),
            type,
            rules,
            outTypeLayout);
    }
    else if (auto imageType = type->As<GLSLImageType>())
    {
        // TODO: the logic here should really be defined by the rules,
        // and not at this top level...
        ShaderParameterKind kind;
        switch( imageType->getAccess() )
        {
        default:
            kind = ShaderParameterKind::MutableImage;
            break;

        case SLANG_RESOURCE_ACCESS_READ:
            kind = ShaderParameterKind::Image;
            break;
        }

        return GetSimpleLayoutImpl(
            rules->GetObjectLayout(kind),
            type,
            rules,
            outTypeLayout);
    }
    else if (auto textureSamplerType = type->As<TextureSamplerType>())
    {
        // TODO: the logic here should really be defined by the rules,
        // and not at this top level...
        ShaderParameterKind kind;
        switch( textureSamplerType->getAccess() )
        {
        default:
            kind = ShaderParameterKind::MutableTextureSampler;
            break;

        case SLANG_RESOURCE_ACCESS_READ:
            kind = ShaderParameterKind::TextureSampler;
            break;
        }

        return GetSimpleLayoutImpl(
            rules->GetObjectLayout(kind),
            type,
            rules,
            outTypeLayout);
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, KIND)                                                \
    else if(auto type_##TYPE = type->As<TYPE>()) do {                   \
        auto info = rules->GetObjectLayout(ShaderParameterKind::KIND);  \
        if (outTypeLayout)                                              \
        {                                                               \
            *outTypeLayout = createStructuredBufferTypeLayout(          \
                context,                                                \
                ShaderParameterKind::KIND,                              \
                type_##TYPE,                                            \
                type_##TYPE->elementType.Ptr());                        \
        }                                                               \
        return info;                                                    \
    } while(0)

    CASE(HLSLStructuredBufferType,                  StructuredBuffer);
    CASE(HLSLRWStructuredBufferType,                MutableStructuredBuffer);
    CASE(HLSLRasterizerOrderedStructuredBufferType, MutableStructuredBuffer);
    CASE(HLSLAppendStructuredBufferType,            MutableStructuredBuffer);
    CASE(HLSLConsumeStructuredBufferType,           MutableStructuredBuffer);

#undef CASE


    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, KIND)                                        \
    else if(type->As<TYPE>()) do {                              \
        return GetSimpleLayoutImpl(                             \
            rules->GetObjectLayout(ShaderParameterKind::KIND),  \
            type, rules, outTypeLayout);                        \
    } while(0)

    CASE(HLSLByteAddressBufferType,                     RawBuffer);
    CASE(HLSLRWByteAddressBufferType,                   MutableRawBuffer);
    CASE(HLSLRasterizerOrderedByteAddressBufferType,    MutableRawBuffer);

    CASE(GLSLInputAttachmentType,           InputRenderTarget);

    // This case is mostly to allow users to add new resource types...
    CASE(UntypedBufferResourceType,         RawBuffer);

#undef CASE

    //
    // TODO(tfoley): Need to recognize any UAV types here
    //
    else if(auto basicType = type->As<BasicExpressionType>())
    {
        return GetSimpleLayoutImpl(
            rules->GetScalarLayout(basicType->baseType),
            type,
            rules,
            outTypeLayout);
    }
    else if(auto vecType = type->As<VectorExpressionType>())
    {
        return GetSimpleLayoutImpl(
            rules->GetVectorLayout(
                GetLayout(context, vecType->elementType.Ptr()),
                (size_t) GetIntVal(vecType->elementCount)),
            type,
            rules,
            outTypeLayout);
    }
    else if(auto matType = type->As<MatrixExpressionType>())
    {
        // The `GetMatrixLayout` implementation in the layout rules
        // currently defaults to assuming column-major layout,
        // so if we want row-major layout we achieve it here by
        // transposing the row/column counts.
        //
        // TODO: If it is really a universal convention that matrices
        // are laid out just like arrays of vectors, when we can
        // probably eliminate the `virtual` `GetLayout` method entirely,
        // and have the code here be responsible for the layout choice.
        //
        size_t rowCount = (size_t) GetIntVal(matType->getRowCount());
        size_t colCount = (size_t) GetIntVal(matType->getColumnCount());
        if (context.matrixLayoutMode == kMatrixLayoutMode_ColumnMajor)
        {
            size_t tmp = rowCount;
            rowCount = colCount;
            colCount = tmp;
        }

        auto info = rules->GetMatrixLayout(
            GetLayout(context, matType->getElementType()),
            rowCount,
            colCount);

        if (outTypeLayout)
        {
            RefPtr<MatrixTypeLayout> typeLayout = new MatrixTypeLayout();
            *outTypeLayout = typeLayout;

            typeLayout->type = type;
            typeLayout->rules = rules;
            typeLayout->uniformAlignment = info.alignment;
            typeLayout->mode = context.matrixLayoutMode;

            typeLayout->addResourceUsage(info.kind, info.size);
        }

        return info;
    }
    else if (auto arrayType = type->As<ArrayExpressionType>())
    {
        RefPtr<TypeLayout> elementTypeLayout;
        auto elementInfo = GetLayoutImpl(
            context,
            arrayType->baseType.Ptr(),
            outTypeLayout ? &elementTypeLayout : nullptr);

        // To a first approximation, an array will usually be laid out
        // by taking the element's type layout and laying out `elementCount`
        // copies of it. There are of course many details that make
        // this simplistic version of things not quite work.
        //
        // An important complication to deal with is the possibility of
        // having "unbounded" arrays, which don't specify a size.'
        // The layout rules for these vary heavily by resource kind and API.
        //

        auto elementCount = GetElementCount(arrayType->ArrayLength);

        //
        // We can compute the uniform storage layout of an array using
        // the rules for the target API.
        //
        // TODO: ensure that this does something reasonable with the unbounded
        // case, or else issue an error message that the target doesn't
        // support unbounded types.
        //
        
        auto arrayUniformInfo = rules->GetArrayLayout(
            elementInfo,
            elementCount).getUniformLayout();

        if (outTypeLayout)
        {
            RefPtr<ArrayTypeLayout> typeLayout = new ArrayTypeLayout();
            *outTypeLayout = typeLayout;

            // Some parts of the array type layout object are easy to fill in:
            typeLayout->type = type;
            typeLayout->rules = rules;
            typeLayout->originalElementTypeLayout = elementTypeLayout;
            typeLayout->uniformAlignment = arrayUniformInfo.alignment;
            typeLayout->uniformStride = arrayUniformInfo.elementStride;

            typeLayout->addResourceUsage(LayoutResourceKind::Uniform, arrayUniformInfo.size);

            //
            // The tricky part in constructing an array type layout comes when
            // the element type is (or nests) a structure with resource-type
            // fields, because in that case we need to perform AoS-to-SoA
            // conversion as part of computing the final type layout, and
            // we also need to pre-compute an "adjusted" element type
            // layout that accounts for the striding that happens with
            // resource-type contents.
            //
            // This complication is only made worse when we have to deal with
            // unbounded-size arrays over such element types, since those
            // resource-type fields will each end up consuming a full space
            // in the resulting layout.
            //
            // The `maybeAdjustLayoutForArrayElementType` computes an "adjusted"
            // type layout for the element type which takes the array stride into
            // acount. If it returns the same type layout that was passed in,
            // then that means no adjustement took place.
            //
            // The `additionalSpacesNeededForAdjustedElementType` variable counts
            // the number of additional register spaces that were consumed,
            // in the case of an unbounded array.
            //
            UInt additionalSpacesNeededForAdjustedElementType = 0;
            RefPtr<TypeLayout> adjustedElementTypeLayout = maybeAdjustLayoutForArrayElementType(
                elementTypeLayout,
                elementCount,
                additionalSpacesNeededForAdjustedElementType);

            typeLayout->elementTypeLayout = adjustedElementTypeLayout;

            // We will now iterate over the resources consumed by the element
            // type to compute how they contribute to the resource usage
            // of the overall array type.
            //
            for( auto elementResourceInfo : elementTypeLayout->resourceInfos )
            {
                // The uniform case was already handled above
                if( elementResourceInfo.kind == LayoutResourceKind::Uniform )
                    continue;

                LayoutSize arrayResourceCount = 0;

                // In almost all cases, the resources consumed by an array
                // will be its element count times the resources consumed
                // by its element type.
                //
                // The first exception to this is arrays of resources when
                // compiling to GLSL for Vulkan, where an entire array
                // only consumes a single descriptor-table slot.
                //
                if (elementResourceInfo.kind == LayoutResourceKind::DescriptorTableSlot)
                {
                    arrayResourceCount = elementResourceInfo.count;
                }
                //
                // The next big exception is when we are forming an unbounded-size
                // array and the element type got "adjusted," because that means
                // the array type will need to allocate full spaces for any resource-type
                // fields in the element type.
                //
                // Note: we carefully carve things out so that the case of a simple
                // array of resources does *not* lead to the element type being adjusted,
                // so that this logic doesn't trigger and we instead handle it with
                // the default logic below.
                //
                else if(
                    elementCount.isInfinite()
                    && adjustedElementTypeLayout != elementTypeLayout
                    && doesResourceRequireAdjustmentForArrayOfStructs(elementResourceInfo.kind) )
                {
                    // We want to ignore resource types consumed by the element type
                    // that need adjustement if the array size is infinite, since
                    // we will be allocating whole spaces for that part of the
                    // element's resource usage.
                }
                else
                {
                    arrayResourceCount = elementResourceInfo.count * elementCount;
                }

                // Now that we've computed how the resource usage of the element type
                // should contribute to the resource usage of the array, we can
                // add in that resource usage.
                //
                typeLayout->addResourceUsage(
                    elementResourceInfo.kind,
                    arrayResourceCount);
            }

            // The loop above to compute the resource usage of the array from its
            // element type ignored any resource-type fields in an unbounded-size
            // array if they would have been allocated as full register spaces.
            // Those same fields were counted in `additionalSpacesNeededForAdjustedElementType`,
            // and need to be added into the total resource usage for the array
            // if we skipped them as part of the loop (which happens when
            // we detect that the element type layout had been "adjusted").
            //
            if( adjustedElementTypeLayout != elementTypeLayout )
            {
                typeLayout->addResourceUsage(LayoutResourceKind::RegisterSpace, additionalSpacesNeededForAdjustedElementType);
            }
        }
        return arrayUniformInfo;
    }
    else if (auto declRefType = type->As<DeclRefType>())
    {
        auto declRef = declRefType->declRef;

        if (auto structDeclRef = declRef.As<StructDecl>())
        {
            RefPtr<StructTypeLayout> typeLayout;
            if (outTypeLayout)
            {
                typeLayout = new StructTypeLayout();
                typeLayout->type = type;
                typeLayout->rules = rules;
                *outTypeLayout = typeLayout;
            }

            // The layout of a `struct` type is computed in the somewhat
            // obvious fashion by keeping a running counter of the resource
            // usage for each kind of resource, and then for a field that
            // uses a given resource, assigning it the current offset and
            // then bumping the offset by the field size. In the case of
            // uniform data we also need to deal with alignment and other
            // detailed layout rules.

            UniformLayoutInfo info = rules->BeginStructLayout();

            for (auto field : GetFields(structDeclRef))
            {
                RefPtr<TypeLayout> fieldTypeLayout;
                UniformLayoutInfo fieldInfo = GetLayoutImpl(
                    context,
                    GetType(field).Ptr(),
                    outTypeLayout ? &fieldTypeLayout : nullptr,
                    field.getDecl()).getUniformLayout();

                // Note: we don't add any zero-size fields
                // when computing structure layout, just
                // to avoid having a resource type impact
                // the final layout.
                //
                // This means that the code to generate final
                // declarations needs to *also* eliminate zero-size
                // fields to be safe...
                LayoutSize uniformOffset = info.size;
                if(fieldInfo.size != 0)
                {
                    uniformOffset = rules->AddStructField(&info, fieldInfo);
                }

                if (outTypeLayout)
                {
                    // If we are computing a complete layout,
                    // then we need to create variable layouts
                    // for each field of the structure.
                    RefPtr<VarLayout> fieldLayout = new VarLayout();
                    fieldLayout->varDecl = field;
                    fieldLayout->typeLayout = fieldTypeLayout;
                    typeLayout->fields.Add(fieldLayout);
                    typeLayout->mapVarToLayout.Add(field.getDecl(), fieldLayout);

                    // Set up uniform offset information, if there is any uniform data in the field
                    if( fieldTypeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
                    {
                        fieldLayout->AddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset.getFiniteValue();
                    }

                    // Add offset information for any other resource kinds
                    for( auto fieldTypeResourceInfo : fieldTypeLayout->resourceInfos )
                    {
                        // Uniforms were dealt with above
                        if(fieldTypeResourceInfo.kind == LayoutResourceKind::Uniform)
                            continue;

                        // We should not have already processed this resource type
                        SLANG_RELEASE_ASSERT(!fieldLayout->FindResourceInfo(fieldTypeResourceInfo.kind));

                        // The field will need offset information for this kind
                        auto fieldResourceInfo = fieldLayout->AddResourceInfo(fieldTypeResourceInfo.kind);

                        // It is possible for a `struct` field to use an unbounded array
                        // type, and in the D3D case that would consume an unbounded number
                        // of registers. What is more, a single `struct` could have multiple
                        // such fields, or ordinary resource fields after an unbounded field.
                        //
                        // We handle this case by allocating a distinct register space for
                        // any field that consumes an unbounded amount of registers.
                        //
                        if( fieldTypeResourceInfo.count.isInfinite() )
                        {
                            // We need to add one register space to own the storage for this field.
                            //
                            auto structTypeSpaceResourceInfo = typeLayout->findOrAddResourceInfo(LayoutResourceKind::RegisterSpace);
                            auto spaceOffset = structTypeSpaceResourceInfo->count;
                            structTypeSpaceResourceInfo->count += 1;

                            // The field itself will record itself as having a zero offset into
                            // the chosen space.
                            //
                            fieldResourceInfo->space = spaceOffset.getFiniteValue();
                            fieldResourceInfo->index = 0;
                        }
                        else
                        {
                            // In the case where the field consumes a finite number of slots, we
                            // can simply set its offset/index to the number of such slots consumed
                            // so far, and then increment the number of slots consumed by the
                            // `struct` type itself.
                            //
                            auto structTypeResourceInfo = typeLayout->findOrAddResourceInfo(fieldTypeResourceInfo.kind);
                            fieldResourceInfo->index = structTypeResourceInfo->count.getFiniteValue();
                            structTypeResourceInfo->count += fieldTypeResourceInfo.count;
                        }
                    }
                }
            }

            rules->EndStructLayout(&info);
            if (outTypeLayout)
            {
                typeLayout->uniformAlignment = info.alignment;
                typeLayout->addResourceUsage(LayoutResourceKind::Uniform, info.size);
            }

            return info;
        }
        else if (auto globalGenParam = declRef.As<GlobalGenericParamDecl>())
        {
            SimpleLayoutInfo info;
            info.alignment = 0;
            info.size = 0;
            info.kind = LayoutResourceKind::GenericResource;
            if (outTypeLayout)
            {
                auto genParamTypeLayout = new GenericParamTypeLayout();
                // we should have already populated ProgramLayout::genericEntryPointParams list at this point,
                // so we can find the index of this generic param decl in the list
                genParamTypeLayout->type = type;
                genParamTypeLayout->paramIndex = findGenericParam(context.targetReq->layout->globalGenericParams, genParamTypeLayout->getGlobalGenericParamDecl());
                genParamTypeLayout->rules = rules;
                genParamTypeLayout->findOrAddResourceInfo(LayoutResourceKind::GenericResource)->count += 1;
                *outTypeLayout = genParamTypeLayout;
            }
            return info;
        }
    }
    else if (auto errorType = type->As<ErrorType>())
    {
        // An error type means that we encountered something we don't understand.
        //
        // We should probalby inform the user with an error message here.

        SimpleLayoutInfo info;
        return GetSimpleLayoutImpl(
            info,
            type,
            rules,
            outTypeLayout);
    }

    // catch-all case in case nothing matched
    SLANG_ASSERT(!"unimplemented");
    SimpleLayoutInfo info;
    return GetSimpleLayoutImpl(
        info,
        type,
        rules,
        outTypeLayout);
}

SimpleLayoutInfo GetLayout(
    TypeLayoutContext const&    context,
    Type*                       inType)
{
    return GetLayoutImpl(context, inType, nullptr);
}

RefPtr<TypeLayout> createTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type)
{
    RefPtr<TypeLayout> typeLayout;
    GetLayoutImpl(context, type, &typeLayout);
    return typeLayout;
}

RefPtr<TypeLayout> CreateTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type)
{
    RefPtr<TypeLayout> typeLayout;
    GetLayoutImpl(context, type, &typeLayout);
    return typeLayout;
}

RefPtr<TypeLayout> TypeLayout::unwrapArray()
{
    TypeLayout* typeLayout = this;

    while(auto arrayTypeLayout = dynamic_cast<ArrayTypeLayout*>(typeLayout))
        typeLayout = arrayTypeLayout->elementTypeLayout;

    return typeLayout;
}


RefPtr<GlobalGenericParamDecl> GenericParamTypeLayout::getGlobalGenericParamDecl()
{
    auto declRefType = type->AsDeclRefType();
    SLANG_ASSERT(declRefType);
    auto rsDeclRef = declRefType->declRef.As<GlobalGenericParamDecl>();
    return rsDeclRef.getDecl();
}

} // namespace Slang
