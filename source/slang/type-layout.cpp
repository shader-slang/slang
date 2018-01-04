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

static size_t RoundUpToPowerOfTwo( size_t value )
{
    // TODO(tfoley): I know this isn't a fast approach
    size_t result = 1;
    while (result < value)
        result *= 2;
    return result;
}

//

MatrixLayoutMode LayoutRulesImpl::getDefaultMatrixLayoutMode() {
    return family->getDefaultMatrixLayoutMode();
}

//

struct DefaultLayoutRulesImpl : SimpleLayoutRulesImpl
{
    // Get size and alignment for a single value of base type.
    SimpleLayoutInfo GetScalarLayout(BaseType baseType) override
    {
        switch (baseType)
        {
        case BaseType::Int:
        case BaseType::UInt:
        case BaseType::Float:
        case BaseType::Bool:
            return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4, 4 );

        case BaseType::Double:
            return SimpleLayoutInfo( LayoutResourceKind::Uniform, 8, 8 );

        default:
            SLANG_UNEXPECTED("uhandled scalar type");
            UNREACHABLE_RETURN(SimpleLayoutInfo( LayoutResourceKind::Uniform, 0, 1 ));
        }
    }

    virtual SimpleLayoutInfo GetScalarLayout(slang::TypeReflection::ScalarType scalarType)
    {
        switch( scalarType )
        {
        case slang::TypeReflection::ScalarType::Void:       return SimpleLayoutInfo();
        case slang::TypeReflection::ScalarType::None:       return SimpleLayoutInfo();

        // TODO(tfoley): At some point we don't want to lay out `bool` as 4 bytes by default...
        case slang::TypeReflection::ScalarType::Bool:       return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4,4);
        case slang::TypeReflection::ScalarType::Int32:      return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4,4);
        case slang::TypeReflection::ScalarType::UInt32:     return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4,4);
        case slang::TypeReflection::ScalarType::Int64:      return SimpleLayoutInfo( LayoutResourceKind::Uniform, 8,8);
        case slang::TypeReflection::ScalarType::UInt64:     return SimpleLayoutInfo( LayoutResourceKind::Uniform, 8,8);

        // TODO(tfoley): What actually happens if you use `half` in a constant buffer?
        case slang::TypeReflection::ScalarType::Float16:    return SimpleLayoutInfo( LayoutResourceKind::Uniform, 2,2);
        case slang::TypeReflection::ScalarType::Float32:    return SimpleLayoutInfo( LayoutResourceKind::Uniform, 4,4);
        case slang::TypeReflection::ScalarType::Float64:    return SimpleLayoutInfo( LayoutResourceKind::Uniform, 8,8);

        default:
            SLANG_UNEXPECTED("unhandled scalar type");
            UNREACHABLE_RETURN(SimpleLayoutInfo());
        }
    }

    SimpleArrayLayoutInfo GetArrayLayout( SimpleLayoutInfo elementInfo, size_t elementCount) override
    {
        size_t stride = elementInfo.size;

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

    size_t AddStructField(UniformLayoutInfo* ioStructInfo, UniformLayoutInfo fieldInfo) override
    {
        // Skip zero-size fields
        if(fieldInfo.size == 0)
            return ioStructInfo->size;

        ioStructInfo->alignment = std::max(ioStructInfo->alignment, fieldInfo.alignment);
        ioStructInfo->size = RoundToAlignment(ioStructInfo->size, fieldInfo.alignment);
        size_t fieldOffset = ioStructInfo->size;
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
    SimpleArrayLayoutInfo GetArrayLayout(SimpleLayoutInfo elementInfo, size_t elementCount) override
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
    auto size = elementInfo.size * elementCount;
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
    size_t AddStructField(UniformLayoutInfo* ioStructInfo, UniformLayoutInfo fieldInfo) override
    {
        // Skip zero-size fields
        if(fieldInfo.size == 0)
            return ioStructInfo->size;

        ioStructInfo->alignment = std::max(ioStructInfo->alignment, fieldInfo.alignment);
        ioStructInfo->size = RoundToAlignment(ioStructInfo->size, fieldInfo.alignment);

        size_t fieldOffset = ioStructInfo->size;
        size_t fieldSize = fieldInfo.size;

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

    virtual SimpleLayoutInfo GetScalarLayout(slang::TypeReflection::ScalarType)
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

    virtual SimpleLayoutInfo GetScalarLayout(slang::TypeReflection::ScalarType)
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

Std140LayoutRulesImpl kStd140LayoutRulesImpl;
Std430LayoutRulesImpl kStd430LayoutRulesImpl;
HLSLConstantBufferLayoutRulesImpl kHLSLConstantBufferLayoutRulesImpl;
HLSLStructuredBufferLayoutRulesImpl kHLSLStructuredBufferLayoutRulesImpl;

GLSLVaryingLayoutRulesImpl kGLSLVaryingInputLayoutRulesImpl(LayoutResourceKind::VertexInput);
GLSLVaryingLayoutRulesImpl kGLSLVaryingOutputLayoutRulesImpl(LayoutResourceKind::FragmentOutput);

HLSLVaryingLayoutRulesImpl kHLSLVaryingInputLayoutRulesImpl(LayoutResourceKind::VertexInput);
HLSLVaryingLayoutRulesImpl kHLSLVaryingOutputLayoutRulesImpl(LayoutResourceKind::FragmentOutput);

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

    virtual MatrixLayoutMode getDefaultMatrixLayoutMode() override
    {
        // The default matrix layout mode in GLSL is specified
        // to be "column major" but what GLSL calls a "column"
        // is actually what HLSL (and hence Slang) calls a row.
        //
        // That is, an HLSL `float3x4` has 3 rows and 4 columns,
        // and indexing yields a `float4`.
        //
        // A GLSL `mat3x4` has 3 "columns" and 4 "rows", and
        // indexing into it yields a `vec4`.
        //
        // The Slang compiler needs to be consistent about this mess,
        // and so when the GLSL spec says that "column"-major is
        // the default, we know that they actually mean what we
        // call row-major.
        return kMatrixLayoutMode_RowMajor;
    }
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

    virtual MatrixLayoutMode getDefaultMatrixLayoutMode() override
    {
        return kMatrixLayoutMode_ColumnMajor;
    }
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

LayoutRulesImpl kGLSLVaryingInputLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLVaryingInputLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kGLSLVaryingOutputLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLVaryingOutputLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
};

LayoutRulesImpl kGLSLSpecializationConstantLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kGLSLSpecializationConstantLayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
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
    context.matrixLayoutMode = MatrixLayoutMode::kMatrixLayoutMode_RowMajor;

    if( rulesFamily )
    {
        context.rules = rulesFamily->getConstantBufferRules();
        context.matrixLayoutMode = rulesFamily->getDefaultMatrixLayoutMode();
    }

    return context;
}


static int GetElementCount(RefPtr<IntVal> val)
{
    if (auto constantVal = val.As<ConstantIntVal>())
    {
        return (int) constantVal->value;
    }
    else if( auto varRefVal = val.As<GenericParamIntVal>() )
    {
        // TODO(tfoley): do something sensible in this case
        return 0;
    }
    SLANG_UNEXPECTED("unhandled integer literal kind");
    return 0;
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
    RefPtr<Type>  type,
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

static bool isD3DTarget(TargetRequest* targetReq)
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

static bool isVulkanTarget(TargetRequest* targetReq)
{
    switch( targetReq->target )
    {
    default:
        return false;

    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        break;
    }

    // For right now, any GLSL-related target is assumed
    // to be a Vulkan target.

    return true;
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
    if( isD3D12Target(targetReq) )
    {
        if(targetReq->targetFlags & SLANG_TARGET_FLAG_PARAMETER_BLOCKS_USE_REGISTER_SPACES)
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
                    newResInfo->index += offsetResInfo->count;
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
    if( parameterGroupInfo.size )
    {
        containerTypeLayout->addResourceUsage(
            parameterGroupInfo.kind,
            parameterGroupInfo.size);
    }

    // There are several different cases that need to be handled here,
    // depending on whether we have a `ParameterBlock`, a `ConstantBuffer`,
    // or some other kind of parameter group. Furthermore, in the
    // `ParameterBlock` case, we need to deal with differnet layout
    // rules depending on whether a block should map to a register `space`
    // in HLSL or not.

    // Check if we are working with a parameter block...
    auto parameterBlockType = parameterGroupType->As<ParameterBlockType>();

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
            elementVarResInfo->index += containerTypeResInfo->count;
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
        context.with(parameterGroupRules).with(parameterGroupRules->getDefaultMatrixLayoutMode()),
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
static RefPtr<TypeLayout>maybeAdjustLayoutForArrayElementType(
    RefPtr<TypeLayout>  originalTypeLayout,
    UInt                elementCount)
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
            elementCount);

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
            elementCount);

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

        // TODO: we could try to special-case a `struct` type with a single
        // field that needs no adjustment, just to avoid some extra allocation.

        RefPtr<StructTypeLayout> adjustedStructTypeLayout = new StructTypeLayout();
        copyTypeLayoutFields(adjustedStructTypeLayout, originalStructTypeLayout);

        Dictionary<RefPtr<VarLayout>, RefPtr<VarLayout>> mapOriginalFieldToAdjusted;
        for( auto originalField : originalStructTypeLayout->fields )
        {
            // Compute the adjusted type for the field
            auto originalFieldTypeLayout = originalField->typeLayout;
            auto adjustedFieldTypeLayout = maybeAdjustLayoutForArrayElementType(
                originalFieldTypeLayout,
                elementCount);

            // Create an adjusted field variable, that is mostly
            // a clone of the original field (just with our
            // adjusted type in place).
            RefPtr<VarLayout> adjustedField = new VarLayout();
            copyVarLayoutFields(adjustedField, originalField);
            adjustedField->typeLayout = adjustedFieldTypeLayout;

            // Finally we get down to the real meat of the change,
            // which is that the field offsets for any resource-type
            // fields need to be "adjusted" which amounts to just
            // multiplying them by the element count of the array.

            for( auto& resInfo : adjustedField->resourceInfos )
            {
                if( doesResourceRequireAdjustmentForArrayOfStructs(resInfo.kind) )
                {
                    resInfo.index *= elementCount;
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
        // If the leaf type layout isn't some kind of aggregate,
        // then we can just bail out here.
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

    CASE(HLSLStructuredBufferType,          StructuredBuffer);
    CASE(HLSLRWStructuredBufferType,        MutableStructuredBuffer);
    CASE(HLSLAppendStructuredBufferType,    MutableStructuredBuffer);
    CASE(HLSLConsumeStructuredBufferType,   MutableStructuredBuffer);

#undef CASE


    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, KIND)                                        \
    else if(type->As<TYPE>()) do {                              \
        return GetSimpleLayoutImpl(                             \
            rules->GetObjectLayout(ShaderParameterKind::KIND),  \
            type, rules, outTypeLayout);                        \
    } while(0)

    CASE(HLSLByteAddressBufferType,         RawBuffer);
    CASE(HLSLRWByteAddressBufferType,       MutableRawBuffer);

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

        // For layout purposes, we treat an unsized array as an array of zero elements.
        //
        // TODO: Longer term we are going to need to be careful to include some indication
        // that a type has logically "infinite" size in some resource kind. In particular
        // this affects how we would allocate space for parameter binding purposes.
        auto elementCount = arrayType->ArrayLength ? GetElementCount(arrayType->ArrayLength) : 0;
        auto arrayUniformInfo = rules->GetArrayLayout(
            elementInfo,
            elementCount).getUniformLayout();

        if (outTypeLayout)
        {
            RefPtr<ArrayTypeLayout> typeLayout = new ArrayTypeLayout();
            *outTypeLayout = typeLayout;

            // If we construct an array over an aggregate type that contains
            // resource fields, we may need to adjust the layout we create
            // for the element type to
            RefPtr<TypeLayout> adjustedElementTypeLayout = maybeAdjustLayoutForArrayElementType(
                elementTypeLayout,
                elementCount);

            typeLayout->type = type;
            typeLayout->originalElementTypeLayout = elementTypeLayout;
            typeLayout->elementTypeLayout = adjustedElementTypeLayout;
            typeLayout->rules = rules;

            typeLayout->uniformAlignment = arrayUniformInfo.alignment;
            typeLayout->uniformStride = arrayUniformInfo.elementStride;

            typeLayout->addResourceUsage(LayoutResourceKind::Uniform, arrayUniformInfo.size);

            // translate element-type resources into array-type resources
            for( auto elementResourceInfo : elementTypeLayout->resourceInfos )
            {
                // The uniform case was already handled above
                if( elementResourceInfo.kind == LayoutResourceKind::Uniform )
                    continue;

                // In almost all cases, the resources consumed by an array
                // will be its element count times the resources consumed
                // by its element type. The one exception to this is
                // arrays of resources in Vulkan GLSL, where an entire array
                // only consumes a single descriptor-table slot.
                //
                // Note: We extend this logic to arbitrary arrays-of-structs,
                // under the assumption that downstream legalization will
                // turn those into scalarized structs-of-arrays and this
                // logic will work out.
                UInt arrayResourceCount = 0;
                if (elementResourceInfo.kind == LayoutResourceKind::DescriptorTableSlot)
                {
                    arrayResourceCount = elementResourceInfo.count;
                }
                else
                {
                    arrayResourceCount = elementResourceInfo.count * elementCount;
                }
            
                typeLayout->addResourceUsage(
                    elementResourceInfo.kind,
                    arrayResourceCount);
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
                size_t uniformOffset = info.size;
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
                        fieldLayout->AddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset;
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

                        // Check how many slots of the given kind have already been added to the type
                        auto structTypeResourceInfo = typeLayout->findOrAddResourceInfo(fieldTypeResourceInfo.kind);
                        fieldResourceInfo->index = structTypeResourceInfo->count;
                        structTypeResourceInfo->count += fieldTypeResourceInfo.count;
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
                genParamTypeLayout->findOrAddResourceInfo(LayoutResourceKind::GenericResource)->count++;
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

RefPtr<GlobalGenericParamDecl> GenericParamTypeLayout::getGlobalGenericParamDecl()
{
    auto declRefType = type->AsDeclRefType();
    SLANG_ASSERT(declRefType);
    auto rsDeclRef = declRefType->declRef.As<GlobalGenericParamDecl>();
    return rsDeclRef.getDecl();
}

} // namespace Slang
