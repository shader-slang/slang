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
            return SimpleLayoutInfo( LayoutResourceKind::Uniform, 0, 1 );
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
            return SimpleLayoutInfo();
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
    virtual SimpleLayoutInfo GetObjectLayout(ShaderParameterKind kind) override
    {
        // Special-case the layout for a constant-buffer, because we don't
        // want it to allocate a descriptor-table slot
        return SimpleLayoutInfo(LayoutResourceKind::PushConstantBuffer, 1);

        return GLSLObjectLayoutRulesImpl::GetObjectLayout(kind);
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
            return SimpleLayoutInfo();
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

LayoutRulesFamilyImpl* GetLayoutRulesFamilyImpl(LayoutRulesFamily rule)
{
    switch (rule)
    {
    case LayoutRulesFamily::HLSL:   return &kHLSLLayoutRulesFamilyImpl;
    case LayoutRulesFamily::GLSL:   return &kGLSLLayoutRulesFamilyImpl;
    default:
        return nullptr;
    }
}

LayoutRulesFamilyImpl* GetLayoutRulesFamilyImpl(CodeGenTarget target)
{
    switch (target)
    {
    case CodeGenTarget::HLSL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::SlangIR:
        return &kHLSLLayoutRulesFamilyImpl;

    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        return &kGLSLLayoutRulesFamilyImpl;

    default:
        return nullptr;
    }
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

static SimpleLayoutInfo getParameterBlockLayoutInfo(
    RefPtr<ParameterBlockType>  type,
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
    // TODO: the vertex-input and fragment-output cases should
    // only actually apply when we are at the appropriate stage in
    // the pipeline...
    else if( type->As<GLSLInputParameterBlockType>() )
    {
        return SimpleLayoutInfo(LayoutResourceKind::VertexInput, 0);
    }
    else if( type->As<GLSLOutputParameterBlockType>() )
    {
        return SimpleLayoutInfo(LayoutResourceKind::FragmentOutput, 0);
    }
    else
    {
        SLANG_UNEXPECTED("unhandled parameter block type");
        return SimpleLayoutInfo();
    }
}

struct TypeLayoutContext
{
    // The layout rules to use (e.g., we compute
    // layout differently in a `cbuffer` vs. the
    // parameter list of a fragment shader).
    LayoutRulesImpl*    rules;

    // Whether to lay out matrices column-major
    // or row-major.
    MatrixLayoutMode    matrixLayoutMode;
};

RefPtr<TypeLayout> createTypeLayout(
    TypeLayoutContext*  context,
    Type*               type,
    SimpleLayoutInfo    offset);

RefPtr<ParameterBlockTypeLayout>
createParameterBlockTypeLayout(
    TypeLayoutContext*          context,
    RefPtr<ParameterBlockType>  parameterBlockType,
    SimpleLayoutInfo            parameterBlockInfo,
    RefPtr<TypeLayout>          elementTypeLayout)
{
    auto parameterBlockRules = context->rules;

    auto typeLayout = new ParameterBlockTypeLayout();

    typeLayout->type = parameterBlockType;
    typeLayout->rules = parameterBlockRules;

    typeLayout->elementTypeLayout = elementTypeLayout;

    // The layout of the constant buffer if it gets stored
    // in another constant buffer is just what we computed
    // originally (which should be a single binding "slot"
    // and hence no uniform data).
    // 
    typeLayout->uniformAlignment = parameterBlockInfo.alignment;
    SLANG_RELEASE_ASSERT(!typeLayout->FindResourceInfo(LayoutResourceKind::Uniform));
    SLANG_RELEASE_ASSERT(typeLayout->uniformAlignment == 1);

    // TODO(tfoley): There is a subtle question here of whether
    // a constant buffer declaration that then contains zero
    // bytes of uniform data should actually allocate a CB
    // binding slot. For now I'm going to try to ignore it,
    // but handling this robustly could let other code
    // simply handle the "global scope" as a giant outer
    // CB declaration...

    // Make sure that we allocate resource usage for the
    // parameter block itself.
    if( parameterBlockInfo.size )
    {
        typeLayout->addResourceUsage(
            parameterBlockInfo.kind,
            parameterBlockInfo.size);
    }

    // Now, if the element type itself had any resources, then
    // we need to make these part of the layout for our block
    //
    // TODO: re-consider this decision, since it creates
    // complications...
    for( auto elementResourceInfo : elementTypeLayout->resourceInfos )
    {
        // Skip uniform data, since that is encapsualted behind the constant buffer
        if(elementResourceInfo.kind == LayoutResourceKind::Uniform)
            break;

        typeLayout->addResourceUsage(elementResourceInfo);
    }

    return typeLayout;
}

RefPtr<ParameterBlockTypeLayout>
createParameterBlockTypeLayout(
    RefPtr<ParameterBlockType>  parameterBlockType,
    LayoutRulesImpl*            parameterBlockRules,
    SimpleLayoutInfo            parameterBlockInfo,
    RefPtr<TypeLayout>          elementTypeLayout)
{
    TypeLayoutContext context;
    context.rules = parameterBlockRules;
    context.matrixLayoutMode = parameterBlockRules->getDefaultMatrixLayoutMode();

    return createParameterBlockTypeLayout(
        &context,
        parameterBlockType,
        parameterBlockInfo,
        elementTypeLayout);
}

RefPtr<ParameterBlockTypeLayout>
createParameterBlockTypeLayout(
    TypeLayoutContext*          context,
    RefPtr<ParameterBlockType>  parameterBlockType,
    RefPtr<Type>                elementType,
    LayoutRulesImpl*            elementTypeRules)
{
    auto parameterBlockRules = context->rules;

    // First compute resource usage of the block itself.
    // For now we assume that the layout of the block can
    // always be described in a `SimpleLayoutInfo` (only
    // a single resource kind consumed).
    SimpleLayoutInfo info;
    if (parameterBlockType)
    {
        info = getParameterBlockLayoutInfo(
            parameterBlockType,
            parameterBlockRules);
    }
    else
    {
        // If there is no concrete type, then it seems like we are
        // being asked to compute layout for the global scope
        info = parameterBlockRules->GetObjectLayout(ShaderParameterKind::ConstantBuffer);
    }

    // Now compute a layout for the elements of the parameter block.
    // Note that we need to be careful and deal with the case where
    // the elements of the block use the same resource kind consumed
    // by the block itself.

    TypeLayoutContext elementContext = *context;
    elementContext.rules = elementTypeRules;
    auto elementTypeLayout = createTypeLayout(
        &elementContext,
        elementType,
        info);

    return createParameterBlockTypeLayout(
        context,
        parameterBlockType,
        info,
        elementTypeLayout);
}

LayoutRulesImpl* getParameterBufferElementTypeLayoutRules(
    RefPtr<ParameterBlockType>  parameterBlockType,
    LayoutRulesImpl*            rules)
{
    if( parameterBlockType->As<ConstantBufferType>() )
    {
        return rules->getLayoutRulesFamily()->getConstantBufferRules();
    }
    else if( parameterBlockType->As<TextureBufferType>() )
    {
        return rules->getLayoutRulesFamily()->getTextureBufferRules();
    }
    else if( parameterBlockType->As<GLSLInputParameterBlockType>() )
    {
        return rules->getLayoutRulesFamily()->getVaryingInputRules();
    }
    else if( parameterBlockType->As<GLSLOutputParameterBlockType>() )
    {
        return rules->getLayoutRulesFamily()->getVaryingOutputRules();
    }
    else if( parameterBlockType->As<GLSLShaderStorageBufferType>() )
    {
        return rules->getLayoutRulesFamily()->getShaderStorageBufferRules();
    }
    else
    {
        SLANG_UNEXPECTED("uhandled parameter block type");
        return nullptr;
    }
}

RefPtr<ParameterBlockTypeLayout>
createParameterBlockTypeLayout(
    TypeLayoutContext*          context,
    RefPtr<ParameterBlockType>  parameterBlockType)
{
    auto parameterBlockRules = context->rules;

    // Determine the layout rules to use for the contents of the block
    auto elementTypeRules = getParameterBufferElementTypeLayoutRules(
        parameterBlockType,
        parameterBlockRules);

    auto elementType = parameterBlockType->elementType;

    return createParameterBlockTypeLayout(
        context,
        parameterBlockType,
        elementType,
        elementTypeRules);
}

// Create a type layout for a structured buffer type.
RefPtr<StructuredBufferTypeLayout>
createStructuredBufferTypeLayout(
    ShaderParameterKind     kind,
    RefPtr<Type>  structuredBufferType,
    RefPtr<TypeLayout>      elementTypeLayout,
    LayoutRulesImpl*        rules)
{
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
    ShaderParameterKind     kind,
    RefPtr<Type>  structuredBufferType,
    RefPtr<Type>  elementType,
    LayoutRulesImpl*        rules)
{
    // TODO(tfoley): need to compute the layout for the constant
    // buffer's contents...
    auto structuredBufferLayoutRules = GetLayoutRulesImpl(
        LayoutRule::HLSLStructuredBuffer);

    // Create and save type layout for the buffer contents.
    auto elementTypeLayout = CreateTypeLayout(
        elementType.Ptr(),
        structuredBufferLayoutRules);

    return createStructuredBufferTypeLayout(
        kind,
        structuredBufferType,
        elementTypeLayout,
        rules);

}

SimpleLayoutInfo GetLayoutImpl(
    TypeLayoutContext*  context,
    Type*               type,
    RefPtr<TypeLayout>* outTypeLayout,
    SimpleLayoutInfo    offset);

SimpleLayoutInfo GetLayoutImpl(
    TypeLayoutContext*  context,
    Type*               type,
    RefPtr<TypeLayout>* outTypeLayout)
{
    return GetLayoutImpl(context, type, outTypeLayout, SimpleLayoutInfo());
}

SimpleLayoutInfo GetLayoutImpl(
    TypeLayoutContext*  context,
    Type*               type,
    RefPtr<TypeLayout>* outTypeLayout,
    Decl*               declForModifiers)
{
    TypeLayoutContext subContext = *context;

    if (declForModifiers)
    {
        if (declForModifiers->HasModifier<RowMajorLayoutModifier>())
            subContext.matrixLayoutMode = kMatrixLayoutMode_RowMajor;

        if (declForModifiers->HasModifier<ColumnMajorLayoutModifier>())
            subContext.matrixLayoutMode = kMatrixLayoutMode_ColumnMajor;

        // TODO: really need to look for other modifiers that affect
        // layout, such as GLSL `std140`.
    }

    return GetLayoutImpl(&subContext, type, outTypeLayout, SimpleLayoutInfo());
}

SimpleLayoutInfo GetLayoutImpl(
    TypeLayoutContext*  context,
    Type*               type,
    RefPtr<TypeLayout>* outTypeLayout,
    SimpleLayoutInfo    offset)
{
    auto rules = context->rules;

    if (auto parameterBlockType = type->As<ParameterBlockType>())
    {
        // If the user is just interested in uniform layout info,
        // then this is easy: a `ConstantBuffer<T>` is really no
        // different from a `Texture2D<U>` in terms of how it
        // should be handled as a member of a container.
        //
        auto info = getParameterBlockLayoutInfo(parameterBlockType, rules);

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
            *outTypeLayout = createParameterBlockTypeLayout(
                context,
                parameterBlockType);
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
                ShaderParameterKind::KIND,                              \
                type_##TYPE,                                            \
                type_##TYPE->elementType.Ptr(),                         \
                rules);                                                 \
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
                GetLayout(vecType->elementType.Ptr(), rules),
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
        if (context->matrixLayoutMode == kMatrixLayoutMode_ColumnMajor)
        {
            size_t tmp = rowCount;
            rowCount = colCount;
            colCount = tmp;
        }

        auto info = rules->GetMatrixLayout(
            GetLayout(matType->getElementType(), rules),
            rowCount,
            colCount);

        if (outTypeLayout)
        {
            RefPtr<MatrixTypeLayout> typeLayout = new MatrixTypeLayout();
            *outTypeLayout = typeLayout;

            typeLayout->type = type;
            typeLayout->rules = rules;
            typeLayout->uniformAlignment = info.alignment;
            typeLayout->mode = context->matrixLayoutMode;

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

            typeLayout->type = type;
            typeLayout->elementTypeLayout = elementTypeLayout;
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

                    // If the user passed in offset info, then apply it here
                    if (offset.size)
                    {
                        if (auto fieldResInfo = fieldLayout->FindResourceInfo(offset.kind))
                        {
                            fieldResInfo->index += offset.size;
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

SimpleLayoutInfo GetLayout(Type* inType, LayoutRulesImpl* rules)
{
    TypeLayoutContext context;
    context.rules = rules;
    context.matrixLayoutMode = rules->getDefaultMatrixLayoutMode();

    return GetLayoutImpl(&context, inType, nullptr);
}

RefPtr<TypeLayout> createTypeLayout(
    TypeLayoutContext*  context,
    Type*               type,
    SimpleLayoutInfo    offset)
{
    RefPtr<TypeLayout> typeLayout;
    GetLayoutImpl(context, type, &typeLayout, offset);
    return typeLayout;
}

RefPtr<TypeLayout> createTypeLayout(
    TypeLayoutContext*  context,
    Type*               type)
{
    RefPtr<TypeLayout> typeLayout;
    GetLayoutImpl(context, type, &typeLayout, SimpleLayoutInfo());
    return typeLayout;
}

RefPtr<TypeLayout> CreateTypeLayout(
    Type* type,
    LayoutRulesImpl* rules,
    SimpleLayoutInfo offset)
{
    TypeLayoutContext context;
    context.rules = rules;
    context.matrixLayoutMode = rules->getDefaultMatrixLayoutMode();

    RefPtr<TypeLayout> typeLayout;
    GetLayoutImpl(&context, type, &typeLayout, offset);
    return typeLayout;
}

RefPtr<TypeLayout> CreateTypeLayout(Type* type, LayoutRulesImpl* rules)
{
    return CreateTypeLayout(type, rules, SimpleLayoutInfo());
}

SimpleLayoutInfo GetLayout(Type* type, LayoutRule rule)
{
    LayoutRulesImpl* rulesImpl = GetLayoutRulesImpl(rule);
    return GetLayout(type, rulesImpl);
}

} // namespace Slang
