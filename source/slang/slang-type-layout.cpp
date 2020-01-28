// slang-type-layout.cpp
#include "slang-type-layout.h"

#include "slang-syntax.h"
#include "slang-ir-insts.h"

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
        auto elementSize = elementInfo.size.getFiniteValue();
        auto elementAlignment = elementInfo.alignment;
        auto elementStride = RoundToAlignment(elementSize, elementAlignment);

        // An array with no elements will have zero size.
        //
        LayoutSize arraySize = 0;
        //
        // Any array with a non-zero number of elements will need
        // to have space for N elements of size `elementSize`, with
        // the constraints that there must be `elementStride` bytes
        // between consecutive elements.
        //
        if( elementCount > 0 )
        {
            // We can think of this as either allocating (N-1)
            // chunks of size `elementStride` (for most of the elements)
            // and then one final chunk of size `elementSize`  for
            // the last element, or equivalently as allocating
            // N chunks of size `elementStride` and then "giving back"
            // the final `elementStride - elementSize` bytes.
            //
            arraySize = (elementStride * (elementCount-1)) + elementSize;
        }

        SimpleArrayLayoutInfo arrayInfo;
        arrayInfo.kind = elementInfo.kind;
        arrayInfo.size = arraySize;
        arrayInfo.alignment = elementAlignment;
        arrayInfo.elementStride = elementStride;
        return arrayInfo;
    }

    SimpleLayoutInfo GetVectorLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t elementCount) override
    {
        SLANG_UNUSED(elementType);
        SimpleLayoutInfo vectorInfo;
        vectorInfo.kind = elementInfo.kind;
        vectorInfo.size = elementInfo.size * elementCount;
        vectorInfo.alignment = elementInfo.alignment;
        return vectorInfo;
    }

    SimpleArrayLayoutInfo GetMatrixLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t rowCount, size_t columnCount) override
    {
        // The default behavior here is to lay out a matrix
        // as an array of row vectors (that is row-major).
        //
        // In practice, the code that calls `GetMatrixLayout` will
        // potentially transpose the row/column counts in order
        // to get layouts with a different convention.
        //
        return GetArrayLayout(
            GetVectorLayout(elementType, elementInfo, columnCount),
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

        // A struct type must be at least as aligned as its most-aligned field.
        ioStructInfo->alignment = std::max(ioStructInfo->alignment, fieldInfo.alignment);

        // The new field will be added to the end of the struct.
        auto fieldBaseOffset = ioStructInfo->size;

        // We need to ensure that the offset for the field will respect its alignment
        auto fieldOffset = RoundToAlignment(fieldBaseOffset, fieldInfo.alignment);

        // The size of the struct must be adjusted to cover the bytes consumed
        // by this field.
        ioStructInfo->size = fieldOffset + fieldInfo.size;

        return fieldOffset;
    }


    void EndStructLayout(UniformLayoutInfo* ioStructInfo) override
    {
        SLANG_UNUSED(ioStructInfo);

        // Note: A traditional C layout algorithm would adjust the size
        // of a struct type so that it is a multiple of the alignment.
        // This is a parsimonious design choice because it means that
        // `sizeof(T)` can both be used when copying/allocating a single
        // value of type `T` or an array of N values, without having to
        // consider more details.
        //
        // Of course the choice also has down-sides in that wrapping things
        // into a `struct` can affect layout in ways that waste space. E.g.,
        // the following two cases don't lay out the same:
        //
        //      struct S0 { double d; float f; float g; };
        //
        //      struct X  { double d; float f; }
        //      struct S1 { X x;               float g; }
        //
        // Even though `S0::g` and `S1::g` have the same amount of useful
        // data in front of them, they will not land at the same offset,
        // and the resulting struct sizes will differ (`sizeof(S0)` will be
        // 16 while `sizeof(S1)` will be 24).
        //
        // Slang doesn't get to be opinionated about this stuff because
        // there is already precedent in both HLSL and GLSL for types
        // that have a size that is not rounded up to their alignment.
        //
        // Our default layout rules won't implement the C-like policy,
        // and instead it will be injected in the concrete implementations
        // that require it.
    }
};

    /// Common behavior for GLSL-family layout.
struct GLSLBaseLayoutRulesImpl : DefaultLayoutRulesImpl
{
    typedef DefaultLayoutRulesImpl Super;

    SimpleLayoutInfo GetVectorLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t elementCount) override
    {
        SLANG_UNUSED(elementType);
        // The `std140` and `std430` rules require vectors to be aligned to the next power of
        // two up from their size (so a `float2` is 8-byte aligned, and a `float3` is
        // 16-byte aligned).
        //
        // Note that in this case we have a type layout where the size is *not* a multiple
        // of the alignment, so it should be possible to pack a scalar after a `float3`.
        //
        SLANG_RELEASE_ASSERT(elementInfo.kind == LayoutResourceKind::Uniform);
        SLANG_RELEASE_ASSERT(elementInfo.size.isFinite());

        auto size = elementInfo.size.getFiniteValue() * elementCount;
        SimpleLayoutInfo vectorInfo(
            LayoutResourceKind::Uniform,
            size,
            RoundUpToPowerOfTwo(size));
        return vectorInfo;
    }

    SimpleArrayLayoutInfo GetArrayLayout(SimpleLayoutInfo elementInfo, LayoutSize elementCount) override
    {
        // The size of an array must be rounded up to be a multiple of its alignment.
        //
        auto info = Super::GetArrayLayout(elementInfo, elementCount);
        info.size = RoundToAlignment(info.size, info.alignment);
        return info;
    }

    void EndStructLayout(UniformLayoutInfo* ioStructInfo) override
    {
        // The size of a `struct` must be rounded up to be a multiple of its alignment.
        //
        ioStructInfo->size = RoundToAlignment(ioStructInfo->size, ioStructInfo->alignment);
    }
};

    /// The GLSL `std430` layout rules.
struct Std430LayoutRulesImpl : GLSLBaseLayoutRulesImpl
{
    // These rules don't actually need any differences from our
    // base/common GLSL layout rules.
};

    /// The GLSL `std430` layout rules.
struct Std140LayoutRulesImpl : GLSLBaseLayoutRulesImpl
{
    typedef GLSLBaseLayoutRulesImpl Super;

    SimpleArrayLayoutInfo GetArrayLayout(SimpleLayoutInfo elementInfo, LayoutSize elementCount) override
    {
        // The `std140` rules require that array elements
        // be aligned on 16-byte boundaries.
        //
        if(elementInfo.kind == LayoutResourceKind::Uniform)
        {
            if (elementInfo.alignment < 16)
                elementInfo.alignment = 16;
        }
        return Super::GetArrayLayout(elementInfo, elementCount);
    }

    UniformLayoutInfo BeginStructLayout() override
    {
        // The `std140` rules require that a `struct` type
        // be at least 16-byte aligned.
        //
        return UniformLayoutInfo(0, 16);
    }
};

struct HLSLConstantBufferLayoutRulesImpl : DefaultLayoutRulesImpl
{
    typedef DefaultLayoutRulesImpl Super;

    // Similar to GLSL `std140` rules, an HLSL constant buffer requires that
    // `struct` and array types have 16-byte alignement.
    //
    // Unlike GLSL `std140`, the overall size of an array or `struct` type
    // is *not* rounded up to the alignment, so it is possible for later
    // fields to sneak into the "tail space" left behind by a preceding
    // structure or array. E.g., in this example:
    //
    //     struct S { float3 a[2]; float b; };
    //
    // The stride of the array `a` is 16 bytes per element, but the size
    // of `a` will only be 28 bytes (not 32), so that `b` can fit into
    // the space after the last array element and the overall structure
    // will have a size of 32 bytes.

    SimpleArrayLayoutInfo GetArrayLayout(SimpleLayoutInfo elementInfo, LayoutSize elementCount) override
    {
        if(elementInfo.kind == LayoutResourceKind::Uniform)
        {
            if (elementInfo.alignment < 16)
                elementInfo.alignment = 16;
        }
        return Super::GetArrayLayout(elementInfo, elementCount);
    }

    UniformLayoutInfo BeginStructLayout() override
    {
        return UniformLayoutInfo(0, 16);
    }

    // HLSL layout rules do *not* impose additional alignment
    // constraints on vectors (e.g., all of `float`, `float2`,
    // `float3`, and `float4` have 4-byte alignment), but instead
    // they impose a rule that any `struct` field must not
    // "straddle" a 16-byte boundary.
    //
    // This has the effect of making it *look* like `float4`
    // values have 16-byte alignment in practice, but the
    // effects on `float2` and `float3` are more nuanched and
    // lead to different result than the GLSL rules.
    //
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

struct CPULayoutRulesImpl : DefaultLayoutRulesImpl
{
    typedef DefaultLayoutRulesImpl Super;

    SimpleLayoutInfo GetScalarLayout(BaseType baseType) override
    {
        switch (baseType)
        {
            case BaseType::Bool:
            {
                // TODO(JS): Much like ptr this is a problem - in knowing how to return this value. In the past it's been a word
                // on some compilers for example.
                // On checking though current compilers (clang, g++, visual studio) it is a single byte
                return SimpleLayoutInfo( LayoutResourceKind::Uniform, 1, 1 );
            }

            default: return Super::GetScalarLayout(baseType);
        }
    }

    SimpleArrayLayoutInfo GetArrayLayout( SimpleLayoutInfo elementInfo, LayoutSize elementCount) override
    {
        if (elementCount.isInfinite())
        {
            // This is an unsized array, get information for element
            auto info = Super::GetArrayLayout(elementInfo, LayoutSize(1));

            // So it is actually a Array<T> on CPU which is a pointer and a size
            info.size = sizeof(void*) * 2;
            info.alignment = SLANG_ALIGN_OF(void*);

            return info;
        }
        else
        {
            return Super::GetArrayLayout(elementInfo, elementCount);
        }
    }

    UniformLayoutInfo BeginStructLayout() override
    {
        return Super::BeginStructLayout();
    }

    void EndStructLayout(UniformLayoutInfo* ioStructInfo) override
    {
        // Conform to C/C++ size is adjusted to the largest alignment
        ioStructInfo->size = RoundToAlignment(ioStructInfo->size, ioStructInfo->alignment);
    }
};

struct CUDALayoutRulesImpl : DefaultLayoutRulesImpl
{
    typedef DefaultLayoutRulesImpl Super;

    SimpleLayoutInfo GetScalarLayout(BaseType baseType) override
    {
        switch (baseType)
        {
            case BaseType::Bool:
            {
                // In memory a bool is a byte. BUT when in a vector or matrix it will actually be a int32_t
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(uint8_t), SLANG_ALIGN_OF(uint8_t));
            }

            default: return Super::GetScalarLayout(baseType);
        }
    }

    SimpleArrayLayoutInfo GetArrayLayout(SimpleLayoutInfo elementInfo, LayoutSize elementCount) override
    {
        SLANG_RELEASE_ASSERT(elementInfo.size.isFinite());
        auto elementSize = elementInfo.size.getFiniteValue();
        auto elementAlignment = elementInfo.alignment;
        auto elementStride = RoundToAlignment(elementSize, elementAlignment);

        if (elementCount.isInfinite())
        {
            // This is an unsized array, get information for element
            auto info = Super::GetArrayLayout(elementInfo, LayoutSize(1));

            // So it is actually a Array<T> on CUDA which is a pointer and a size
            info.size = sizeof(void*) * 2;
            info.alignment = SLANG_ALIGN_OF(void*);
            return info;
        }

        // An array with no elements will have zero size.
        //
        LayoutSize arraySize = 0;
        //
        // Any array with a non-zero number of elements will need
        // to have space for N elements of size `elementSize`, with
        // the constraints that there must be `elementStride` bytes
        // between consecutive elements.
        //
        if (elementCount > 0)
        {
            // We can think of this as either allocating (N-1)
            // chunks of size `elementStride` (for most of the elements)
            // and then one final chunk of size `elementSize`  for
            // the last element, or equivalently as allocating
            // N chunks of size `elementStride` and then "giving back"
            // the final `elementStride - elementSize` bytes.
            //
            arraySize = (elementStride * (elementCount - 1)) + elementSize;
        }

        SimpleArrayLayoutInfo arrayInfo;
        arrayInfo.kind = elementInfo.kind;
        arrayInfo.size = arraySize;
        arrayInfo.alignment = elementAlignment;
        arrayInfo.elementStride = elementStride;
        return arrayInfo;
    }

    SimpleLayoutInfo GetVectorLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t elementCount) override
    {
        // Special case bool
        if (elementType == BaseType::Bool)
        {
            SimpleLayoutInfo fixInfo(elementInfo);
            fixInfo.size = sizeof(int32_t);
            fixInfo.alignment = SLANG_ALIGN_OF(int32_t);
            return GetVectorLayout(BaseType::Int, fixInfo, elementCount);
        }

        SimpleLayoutInfo vectorInfo;
        vectorInfo.kind = elementInfo.kind;
        vectorInfo.size = elementInfo.size * elementCount;
        vectorInfo.alignment = elementInfo.alignment;
    
        return vectorInfo;
    }

    SimpleArrayLayoutInfo GetMatrixLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t rowCount, size_t columnCount) override
    {
        // Special case bool
        if (elementType == BaseType::Bool)
        {
            SimpleLayoutInfo fixInfo(elementInfo);
            fixInfo.size = sizeof(int32_t);
            fixInfo.alignment = SLANG_ALIGN_OF(int32_t);
            return GetMatrixLayout(BaseType::Int, fixInfo, rowCount, columnCount);
        }

        return Super::GetMatrixLayout(elementType, elementInfo, rowCount, columnCount);
    }

    UniformLayoutInfo BeginStructLayout() override
    {
        return Super::BeginStructLayout();
    }

    void EndStructLayout(UniformLayoutInfo* ioStructInfo) override
    {
        // Conform to CUDA/C/C++ size is adjusted to the largest alignment
        ioStructInfo->size = RoundToAlignment(ioStructInfo->size, ioStructInfo->alignment);
    }
};

struct HLSLStructuredBufferLayoutRulesImpl : DefaultLayoutRulesImpl
{
    // HLSL structured buffers drop the restrictions added for constant buffers,
    // but retain the rules around not adjusting the size of an array or
    // structure to its alignment. In this way they should match our
    // default layout rules.
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

    SimpleLayoutInfo GetVectorLayout(BaseType elementType, SimpleLayoutInfo, size_t) override
    {
        SLANG_UNUSED(elementType);
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

    SimpleLayoutInfo GetVectorLayout(BaseType elementType, SimpleLayoutInfo, size_t elementCount) override
    {
        SLANG_UNUSED(elementType);
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

    LayoutRulesImpl* getStructuredBufferRules() override;
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
    
    LayoutRulesImpl* getStructuredBufferRules() override;
};

struct CPULayoutRulesFamilyImpl : LayoutRulesFamilyImpl
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
    LayoutRulesImpl* getStructuredBufferRules() override;
};

struct CUDALayoutRulesFamilyImpl : LayoutRulesFamilyImpl
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
    LayoutRulesImpl* getStructuredBufferRules() override;
};

GLSLLayoutRulesFamilyImpl kGLSLLayoutRulesFamilyImpl;
HLSLLayoutRulesFamilyImpl kHLSLLayoutRulesFamilyImpl;
CPULayoutRulesFamilyImpl kCPULayoutRulesFamilyImpl;
CUDALayoutRulesFamilyImpl kCUDALayoutRulesFamilyImpl;

// CPU case

struct CPUObjectLayoutRulesImpl : ObjectLayoutRulesImpl
{
    virtual SimpleLayoutInfo GetObjectLayout(ShaderParameterKind kind) override
    {
        switch (kind)
        {
            case ShaderParameterKind::ConstantBuffer:
                // It's a pointer to the actual uniform data
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*), SLANG_ALIGN_OF(void*));

            case ShaderParameterKind::MutableTexture:
            case ShaderParameterKind::TextureUniformBuffer:
            case ShaderParameterKind::Texture:
                // It's a pointer to a texture interface 
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*), SLANG_ALIGN_OF(void*));
                
            case ShaderParameterKind::StructuredBuffer:            
            case ShaderParameterKind::MutableStructuredBuffer:
                // It's a ptr and a size of the amount of elements
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*) * 2, SLANG_ALIGN_OF(void*));

            case ShaderParameterKind::RawBuffer:
            case ShaderParameterKind::Buffer:
            case ShaderParameterKind::MutableRawBuffer:
            case ShaderParameterKind::MutableBuffer:
                // It's a pointer and a size in bytes
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*) * 2, SLANG_ALIGN_OF(void*));

            case ShaderParameterKind::SamplerState:
                // It's a pointer
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*), SLANG_ALIGN_OF(void*));
 
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


// TODO(JS): Most likely wrong! Assumes largely CPU layout which is probably not right
struct CUDAObjectLayoutRulesImpl : CPUObjectLayoutRulesImpl
{
    typedef CPUObjectLayoutRulesImpl Super;

    // cuda.h defines a variety of handle types. We don't want to have to include cuda.h though - as it may not be available
    // on a build target. So for we define this handle type, that matches cuda.h and is used for types that use this kind
    // of opaque handle (as opposed to a pointer) such as CUsurfObject, CUtexObject
    typedef unsigned long long ObjectHandle;

    virtual SimpleLayoutInfo GetObjectLayout(ShaderParameterKind kind) override
    {
        switch (kind)
        {
            case ShaderParameterKind::ConstantBuffer:
                // It's a pointer to the actual uniform data
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*), SLANG_ALIGN_OF(void*));

            case ShaderParameterKind::MutableTexture:
            case ShaderParameterKind::TextureUniformBuffer:
            case ShaderParameterKind::Texture:
                // It's a pointer to a texture interface 
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(ObjectHandle), SLANG_ALIGN_OF(ObjectHandle));

            case ShaderParameterKind::StructuredBuffer:
            case ShaderParameterKind::MutableStructuredBuffer:
                // It's a pointer and a size
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*) * 2, SLANG_ALIGN_OF(void*));

            case ShaderParameterKind::RawBuffer:
            case ShaderParameterKind::Buffer:
            case ShaderParameterKind::MutableRawBuffer:
            case ShaderParameterKind::MutableBuffer:
                // It's a pointer and a size in bytes
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*) * 2, SLANG_ALIGN_OF(void*));

            case ShaderParameterKind::SamplerState:
                // In CUDA it seems that sampler states are combined into texture objects.
                // So it's a binding issue to combine a sampler with a texture - and sampler are ignored
                // For simplicity here though - we do create a variable and that variable takes up
                // uniform binding space.
                // TODO(JS): If we wanted to remove these variables we'd want to do it as a pass. The pass
                // would presumably have to remove use of variables of this kind throughout IR. 
                return SimpleLayoutInfo(LayoutResourceKind::Uniform, sizeof(void*), SLANG_ALIGN_OF(void*));

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

static CPUObjectLayoutRulesImpl kCPUObjectLayoutRulesImpl;
static CPULayoutRulesImpl kCPULayoutRulesImpl;

LayoutRulesImpl kCPULayoutRulesImpl_ = {
    &kCPULayoutRulesFamilyImpl, &kCPULayoutRulesImpl, &kCPUObjectLayoutRulesImpl,
};

// CUDA

static CUDAObjectLayoutRulesImpl kCUDAObjectLayoutRulesImpl;
static CUDALayoutRulesImpl kCUDALayoutRulesImpl;

LayoutRulesImpl kCUDALayoutRulesImpl_ = {
    &kCUDALayoutRulesFamilyImpl, &kCUDALayoutRulesImpl, &kCUDAObjectLayoutRulesImpl,
};


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

LayoutRulesImpl kGLSLStructuredBufferLayoutRulesImpl_ = {
    &kGLSLLayoutRulesFamilyImpl, &kStd430LayoutRulesImpl, &kGLSLObjectLayoutRulesImpl,
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

// GLSL Family

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

LayoutRulesImpl* GLSLLayoutRulesFamilyImpl::getStructuredBufferRules()
{
    return &kGLSLStructuredBufferLayoutRulesImpl_;
}

// HLSL Family

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

LayoutRulesImpl* HLSLLayoutRulesFamilyImpl::getStructuredBufferRules()
{
    return &kHLSLStructuredBufferLayoutRulesImpl_;
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

// CPU Family

LayoutRulesImpl* CPULayoutRulesFamilyImpl::getConstantBufferRules()
{
    return &kCPULayoutRulesImpl_;
}

LayoutRulesImpl* CPULayoutRulesFamilyImpl::getPushConstantBufferRules()
{
    return &kCPULayoutRulesImpl_;
}

LayoutRulesImpl* CPULayoutRulesFamilyImpl::getTextureBufferRules()
{
    return nullptr;
}

LayoutRulesImpl* CPULayoutRulesFamilyImpl::getVaryingInputRules()
{
    return nullptr;
}
LayoutRulesImpl* CPULayoutRulesFamilyImpl::getVaryingOutputRules()
{
    return nullptr;
}
LayoutRulesImpl* CPULayoutRulesFamilyImpl::getSpecializationConstantRules()
{
    return nullptr;
}
LayoutRulesImpl* CPULayoutRulesFamilyImpl::getShaderStorageBufferRules()
{
    return nullptr;
}
LayoutRulesImpl* CPULayoutRulesFamilyImpl::getParameterBlockRules()
{
    // Not clear - just use similar to CPU 
    return &kCPULayoutRulesImpl_;
}
LayoutRulesImpl* CPULayoutRulesFamilyImpl::getRayPayloadParameterRules()
{
    return nullptr;
}
LayoutRulesImpl* CPULayoutRulesFamilyImpl::getCallablePayloadParameterRules()
{
    return nullptr;
}
LayoutRulesImpl* CPULayoutRulesFamilyImpl::getHitAttributesParameterRules()
{
    return nullptr;
}
LayoutRulesImpl* CPULayoutRulesFamilyImpl::getShaderRecordConstantBufferRules()
{
    // Just following HLSLs lead for the moment
    return &kCPULayoutRulesImpl_;
}

LayoutRulesImpl* CPULayoutRulesFamilyImpl::getStructuredBufferRules()
{
    return &kCPULayoutRulesImpl_;
}

// CUDA Family

LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getConstantBufferRules()
{
    return &kCUDALayoutRulesImpl_;
}

LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getPushConstantBufferRules()
{
    return &kCUDALayoutRulesImpl_;
}

LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getTextureBufferRules()
{
    return nullptr;
}

LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getVaryingInputRules()
{
    return nullptr;
}
LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getVaryingOutputRules()
{
    return nullptr;
}
LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getSpecializationConstantRules()
{
    return nullptr;
}
LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getShaderStorageBufferRules()
{
    return nullptr;
}
LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getParameterBlockRules()
{
    // Not clear - just use similar to CPU 
    return &kCUDALayoutRulesImpl_;
}
LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getRayPayloadParameterRules()
{
    return nullptr;
}
LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getCallablePayloadParameterRules()
{
    return nullptr;
}
LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getHitAttributesParameterRules()
{
    return nullptr;
}
LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getShaderRecordConstantBufferRules()
{
    // Just following HLSLs lead for the moment
    return &kCUDALayoutRulesImpl_;
}

LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getStructuredBufferRules()
{
    return &kCUDALayoutRulesImpl_;
}



LayoutRulesFamilyImpl* getDefaultLayoutRulesFamilyForTarget(TargetRequest* targetReq)
{
    switch (targetReq->getTarget())
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

    case CodeGenTarget::HostCallable:
    case CodeGenTarget::Executable:
    case CodeGenTarget::SharedLibrary:
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CSource:
    {
        // For now lets use some fairly simple CPU binding rules

        // We just need to decide here what style of layout is appropriate, in terms of memory
        // and binding. That in terms of the actual binding that will be injected into functions
        // in the form of a BindContext. For now we'll go with HLSL layout -
        // that we may want to rethink that with the use of arrays and binding VK style binding might be
        // more appropriate in some ways.

        return &kCPULayoutRulesFamilyImpl;
    }

    case CodeGenTarget::PTX:
    case CodeGenTarget::CUDASource:
    {
        return &kCUDALayoutRulesFamilyImpl;
    }


    default:
        return nullptr;
    }
}

TypeLayoutContext getInitialLayoutContextForTarget(TargetRequest* targetReq, ProgramLayout* programLayout)
{
    LayoutRulesFamilyImpl* rulesFamily = getDefaultLayoutRulesFamilyForTarget(targetReq);

    TypeLayoutContext context;
    context.targetReq = targetReq;
    context.programLayout = programLayout;
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

    if (auto constantVal = as<ConstantIntVal>(val))
    {
        return LayoutSize(LayoutSize::RawValue(constantVal->value));
    }
    else if( auto varRefVal = as<GenericParamIntVal>(val) )
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

    /// Create a type layout for a type that has simple layout needs.
    ///
    /// This handles any type that can express its layout in `SimpleLayoutInfo`,
    /// and that only needs a `TypeLayout` and not a refined subclass.
    ///
static TypeLayoutResult createSimpleTypeLayout(
    SimpleLayoutInfo        info,
    RefPtr<Type>            type,
    LayoutRulesImpl*        rules)
{
    RefPtr<TypeLayout> typeLayout = new TypeLayout();

    typeLayout->type = type;
    typeLayout->rules = rules;

    typeLayout->uniformAlignment = info.alignment;

    typeLayout->addResourceUsage(info.kind, info.size);

    return TypeLayoutResult(typeLayout, info);
}

static SimpleLayoutInfo getParameterGroupLayoutInfo(
    RefPtr<ParameterGroupType>  type,
    LayoutRulesImpl*            rules)
{
    if( as<ConstantBufferType>(type) )
    {
        return rules->GetObjectLayout(ShaderParameterKind::ConstantBuffer);
    }
    else if( as<TextureBufferType>(type) )
    {
        return rules->GetObjectLayout(ShaderParameterKind::TextureUniformBuffer);
    }
    else if( as<GLSLShaderStorageBufferType>(type) )
    {
        return rules->GetObjectLayout(ShaderParameterKind::ShaderStorageBuffer);
    }
    else if (as<ParameterBlockType>(type))
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
    else if( as<GLSLInputParameterGroupType>(type) )
    {
        return SimpleLayoutInfo(LayoutResourceKind::VertexInput, 0);
    }
    else if( as<GLSLOutputParameterGroupType>(type) )
    {
        return SimpleLayoutInfo(LayoutResourceKind::FragmentOutput, 0);
    }
    else
    {
        SLANG_UNEXPECTED("unhandled parameter block type");
        UNREACHABLE_RETURN(SimpleLayoutInfo());
    }
}

static bool isOpenGLTarget(TargetRequest*)
{
    // We aren't officially supporting OpenGL right now
    return false;
}

bool isD3DTarget(TargetRequest* targetReq)
{
    switch( targetReq->getTarget() )
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
    switch( targetReq->getTarget() )
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

    auto profile = targetReq->getTargetProfile();

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

    auto profile = targetReq->getTargetProfile();

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
// to any contained fields based on the resource infos in `offsetVarLayout`.
RefPtr<TypeLayout> applyOffsetToTypeLayout(
    RefPtr<TypeLayout>  oldTypeLayout,
    RefPtr<VarLayout>   offsetVarLayout)
{
    // There is no need to apply offsets if the old type and the offset
    // don't share any resource infos in common.
    bool anyHit = false;
    for (auto oldResInfo : oldTypeLayout->resourceInfos)
    {
        if (auto offsetResInfo = offsetVarLayout->FindResourceInfo(oldResInfo.kind))
        {
            anyHit = true;
            break;
        }
    }
    if( auto oldPendingTypeLayout = oldTypeLayout->pendingDataTypeLayout )
    {
        if( auto pendingOffsetVarLayout = offsetVarLayout->pendingVarLayout )
        {
            for (auto oldResInfo : oldPendingTypeLayout->resourceInfos)
            {
                if (auto offsetResInfo = pendingOffsetVarLayout->FindResourceInfo(oldResInfo.kind))
                {
                    anyHit = true;
                    break;
                }
            }
        }
    }

    if (!anyHit)
        return oldTypeLayout;

    RefPtr<TypeLayout> newTypeLayout;
    if (auto oldStructTypeLayout = oldTypeLayout.as<StructTypeLayout>())
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
                if (auto offsetResInfo = offsetVarLayout->FindResourceInfo(oldResInfo.kind))
                {
                    newResInfo->index += offsetResInfo->index;
                }
            }

            if( auto oldPendingField = oldField->pendingVarLayout )
            {
                RefPtr<VarLayout> newPendingField = new VarLayout();
                newPendingField->varDecl                   = oldPendingField->varDecl;
                newPendingField->typeLayout                = oldPendingField->typeLayout;
                newPendingField->flags                     = oldPendingField->flags;
                newPendingField->semanticIndex             = oldPendingField->semanticIndex;
                newPendingField->semanticName              = oldPendingField->semanticName;
                newPendingField->stage                     = oldPendingField->stage;
                newPendingField->systemValueSemantic       = oldPendingField->systemValueSemantic;
                newPendingField->systemValueSemanticIndex  = oldPendingField->systemValueSemanticIndex;

                newField->pendingVarLayout = newPendingField;

                for (auto oldResInfo : oldPendingField->resourceInfos)
                {
                    auto newResInfo = newPendingField->findOrAddResourceInfo(oldResInfo.kind);
                    newResInfo->index = oldResInfo.index;
                    newResInfo->space = oldResInfo.space;
                    if( auto pendingOffsetVarLayout = offsetVarLayout->pendingVarLayout )
                    {
                        if (auto offsetResInfo = pendingOffsetVarLayout->FindResourceInfo(oldResInfo.kind))
                        {
                            newResInfo->index += offsetResInfo->index;
                        }
                    }
                }
            }

            newStructTypeLayout->fields.add(newField);

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

    if( auto oldPendingTypeLayout = oldTypeLayout->pendingDataTypeLayout )
    {
        if( auto pendingOffsetVarLayout = offsetVarLayout->pendingVarLayout )
        {
            newTypeLayout->pendingDataTypeLayout = applyOffsetToTypeLayout(oldPendingTypeLayout, pendingOffsetVarLayout);
        }
    }

    return newTypeLayout;
}

IRTypeLayout* applyOffsetToTypeLayout(
    IRBuilder*      irBuilder,
    IRTypeLayout*   oldTypeLayout,
    IRVarLayout*    offsetVarLayout)
{
    // The body of this function is derived from the AST case defined above.
    //
    // TODO: We shouldn't need this function at all because "offset" type
    // layouts were only introduced as a legacy workaround for some bad choices
    // in the reflection API.
    //

    // There is no need to apply offsets if the old type and the offset
    // don't share any resource infos in common.
    bool anyHit = false;
    for (auto oldResInfo : oldTypeLayout->getSizeAttrs())
    {
        if (auto offsetResInfo = offsetVarLayout->findOffsetAttr(oldResInfo->getResourceKind()))
        {
            anyHit = true;
            break;
        }
    }

    if (!anyHit)
        return oldTypeLayout;

    if (auto oldStructTypeLayout = as<IRStructTypeLayout>(oldTypeLayout))
    {
        IRStructTypeLayout::Builder newStructTypeLayoutBuilder(irBuilder);
        newStructTypeLayoutBuilder.addResourceUsageFrom(oldTypeLayout);

        for (auto oldFieldAttr : oldStructTypeLayout->getFieldLayoutAttrs())
        {
            auto fieldKey = oldFieldAttr->getFieldKey();
            auto oldFieldLayout = oldFieldAttr->getLayout();

            IRVarLayout::Builder newFieldBuilder(irBuilder, oldFieldLayout->getTypeLayout());
            newFieldBuilder.cloneEverythingButOffsetsFrom(oldFieldLayout);

            for (auto oldResInfo : oldFieldLayout->getOffsetAttrs())
            {
                auto kind = oldResInfo->getResourceKind();
                auto newResInfo = newFieldBuilder.findOrAddResourceInfo(kind);
                newResInfo->offset = oldResInfo->getOffset();
                newResInfo->space = oldResInfo->getSpace();
                if (auto offsetResInfo = offsetVarLayout->findOffsetAttr(kind))
                {
                    newResInfo->offset += offsetResInfo->getOffset();
                }
            }

            newStructTypeLayoutBuilder.addField(fieldKey, newFieldBuilder.build());
        }

        return newStructTypeLayoutBuilder.build();
    }
    else
    {
        // We can only effectively apply this offsetting to basic struct types,
        // and so we won't even attempt it for anything else. This matches the
        // AST implementation of this function, and shouldn't matter in the long
        // run since we will remove the concept of offset type layouts from
        // the IR.
        //
        return oldTypeLayout;
    }
}

IRVarLayout* applyOffsetToVarLayout(
    IRBuilder*      irBuilder,
    IRVarLayout*    baseLayout,
    IRVarLayout*    offsetLayout)
{
    IRVarLayout::Builder adjustedLayoutBuilder(irBuilder, baseLayout->getTypeLayout());
    adjustedLayoutBuilder.cloneEverythingButOffsetsFrom(baseLayout);

    if( auto basePendingLayout = baseLayout->getPendingVarLayout() )
    {
        if( auto offsetPendingLayout = offsetLayout->getPendingVarLayout() )
        {
            adjustedLayoutBuilder.setPendingVarLayout(
                applyOffsetToVarLayout(
                    irBuilder,
                    basePendingLayout,
                    offsetPendingLayout));
        }
    }

    for( auto baseResInfo : baseLayout->getOffsetAttrs() )
    {
        auto kind = baseResInfo->getResourceKind();
        auto adjustedResInfo = adjustedLayoutBuilder.findOrAddResourceInfo(kind);
        adjustedResInfo->offset = baseResInfo->getOffset();
        adjustedResInfo->space = baseResInfo->getSpace();

        if( auto offsetResInfo = offsetLayout->findOffsetAttr(baseResInfo->getResourceKind()) )
        {
            adjustedResInfo->offset += offsetResInfo->getOffset();
            adjustedResInfo->space += offsetResInfo->getSpace();
        }
    }

    return adjustedLayoutBuilder.build();
}

static bool _usesResourceKind(RefPtr<TypeLayout> typeLayout, LayoutResourceKind kind)
{
    auto resInfo = typeLayout->FindResourceInfo(kind);
    return resInfo && resInfo->count != 0;
}

static bool _usesOrdinaryData(RefPtr<TypeLayout> typeLayout)
{
    return _usesResourceKind(typeLayout, LayoutResourceKind::Uniform);
}

    /// Add resource usage from `srcTypeLayout` to `dstTypeLayout` unless it would be "masked."
    ///
    /// This function is appropriate for applying resource usage from an element type
    /// to the resource usage of a container like a `ConstantBuffer<X>` or
    /// `ParameterBlock<X>`.
    ///
    /// TODO: letUnformBleedThrough is (hopefully temporary) a hack that was added to enable CPU targets to
    /// produce workable layout. CPU targets have all bindings/variables laid out as uniforms
static void _addUnmaskedResourceUsage(
    bool letUniformBleedThrough, 
    TypeLayout* dstTypeLayout,
    TypeLayout* srcTypeLayout,
    bool        haveFullRegisterSpaceOrSet)
{
    for( auto resInfo : srcTypeLayout->resourceInfos )
    {
        switch( resInfo.kind )
        {
        case LayoutResourceKind::Uniform:
            // Ordinary/uniform resource usage will always be masked.
            if (letUniformBleedThrough)
            {
                dstTypeLayout->addResourceUsage(resInfo);
            }
            break;

        case LayoutResourceKind::RegisterSpace:
        case LayoutResourceKind::ExistentialTypeParam:
            // A parameter group will always pay for full registers
            // spaces consumed by its element type.
            //
            // The same is true for existential type parameters,
            // since these need to be exposed up through the API.
            //
            dstTypeLayout->addResourceUsage(resInfo);
            break;

        default:
            // For all other resource kinds, a parameter group
            // will be able to mask them if and only if it
            // has a full space/set allocated to it.
            //
            // Otherwise, the resource usage of the group must
            // include the resource usage of the element.
            //
            if( !haveFullRegisterSpaceOrSet )
            {
                dstTypeLayout->addResourceUsage(resInfo);
            }
            break;
        }
    }
}

static RefPtr<TypeLayout> _createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType,
    RefPtr<TypeLayout>          rawElementTypeLayout)
{
    // We are being asked to create a layout for a parameter group,
    // which is curently either a `ParameterBlock<T>` or a `ConstantBuffer<T>`
    //
    auto parameterGroupRules = context.rules;
    RefPtr<ParameterGroupTypeLayout> typeLayout = new ParameterGroupTypeLayout();
    typeLayout->type = parameterGroupType;
    typeLayout->rules = parameterGroupRules;

    // Computing the layout is made tricky by several factors.
    //
    // A parameter group has to draw a distinction between the element type,
    // and the resources it consumes, and the "container," which main
    // consume other resources. The type of resource consumed by
    // the two can overlap.
    //
    // Consider:
    //
    //      struct MyMaterial { float2 uvScale; Texture2D albedoMap; }
    //      ParameterBlock<MyMaterial> gMaterial;
    //
    // In this example, `gMaterial` will need both a constant buffer
    // binding (to hold the data for `uvScale`) and a texture binding
    // (for `albedoMap`). On Vulkan, those two things require the *same*
    // `LayoutResourceKind` (representing a GLSL `binding`). We will
    // thus track the resource usage of the "container" type and
    // element type separately, and then combine these to form
    // the overall layout for the parameter group.

    RefPtr<TypeLayout> containerTypeLayout = new TypeLayout();
    containerTypeLayout->type = parameterGroupType;
    containerTypeLayout->rules = parameterGroupRules;

    // Because the container and element types will each be situated
    // at some offset relative to the initial register/binding for
    // the group as a whole, we allocate a `VarLayout` for both
    // the container and the element type, to store that offset
    // information (think of `TypeLayout`s as holding size information,
    // while `VarLayout`s hold offset information).

    RefPtr<VarLayout> containerVarLayout = new VarLayout();
    containerVarLayout->typeLayout = containerTypeLayout;
    typeLayout->containerVarLayout = containerVarLayout;

    RefPtr<VarLayout> elementVarLayout = new VarLayout();
    elementVarLayout->typeLayout = rawElementTypeLayout;
    typeLayout->elementVarLayout = elementVarLayout;

    // It is possible to have a `ConstantBuffer<T>` that doesn't
    // actually need a constant buffer register/binding allocated to it,
    // because the type `T` doesn't actually contain any ordinary/uniform
    // data that needs to go into the constant buffer. For example:
    //
    //      struct MyMaterial { Texture2D t; SamplerState s; };
    //      ConstantBuffer<MyMaterial> gMaterial;
    //
    // In this example, the `gMaterial` parameter doesn't actually need
    // a constant buffer allocated for it. This isn't something that
    // comes up often for `ConstantBuffer`, but can happen a lot for
    // `ParameterBlock`.
    //
    // To determine if we actually need a constant-buffer binding,
    // we will inspect the element type and see if it contains
    // any ordinary/uniform data.
    //
    bool wantConstantBuffer = _usesOrdinaryData(rawElementTypeLayout);
    if( wantConstantBuffer )
    {
        // If there is any ordinary data, then we'll need to
        // allocate a constant buffer regiser/binding into
        // the overall layout, to account for it.
        //
        auto cbUsage = parameterGroupRules->GetObjectLayout(ShaderParameterKind::ConstantBuffer);
        containerTypeLayout->addResourceUsage(cbUsage.kind, cbUsage.size);
    }

    // Similarly to how we only need a constant buffer to be allocated
    // if the contents of the group actually had ordinary/uniform data,
    // we also only want to allocate a `space` or `set` if that is really
    // required.
    //
    //
    bool canUseSpaceOrSet = false;
    //
    // We will only allocate a `space` or `set` if the type is `ParameterBlock<T>`
    // and not just `ConstantBuffer<T>`.
    //
    // Note: `parameterGroupType` is allowed to be null here, if we are allocating
    // an anonymous constant buffer for global or entry-point parameters, but that
    // is fine because the case will just return null in that case anyway.
    //
    auto parameterBlockType = as<ParameterBlockType>(parameterGroupType);
    if( parameterBlockType )
    {
        // We also can't allocate a `space` or `set` unless the compilation
        // target actually supports them.
        //
        if( shouldAllocateRegisterSpaceForParameterBlock(context) )
        {
            canUseSpaceOrSet = true;
        }
    }

    // Just knowing that we *can* use a `space` or `set` doesn't tell
    // us if we would *like* to.
    //
    // The basic rule here is that if the element type of the parameter
    // block contains anything that isn't itself consuming a full
    // register `space` or `set`, then we'll want an umbrella `space`/`set`
    // for all such data.
    //
    bool wantSpaceOrSet = false;
    if( canUseSpaceOrSet )
    {
        // Note that if we are allocating a constant buffer to hold
        // some ordinary/uniform data then we definitely want a space/set,
        // but we don't need to special-case that because the loop
        // here will also detect the `LayoutResourceKind::Uniform` usage.

        for( auto elementResourceInfo : rawElementTypeLayout->resourceInfos )
        {
            if(elementResourceInfo.kind != LayoutResourceKind::RegisterSpace)
            {
                wantSpaceOrSet = true;
                break;
            }
        }
    }

    // If after all that we determine that we want a register space/set,
    // then we allocate one as part of the overall resource usage for
    // the parameter group type.
    //
    if( wantSpaceOrSet )
    {
        containerTypeLayout->addResourceUsage(LayoutResourceKind::RegisterSpace, 1);
    }

    // Now that we've computed basic resource requirements for the container
    // part of things (i.e., does it require a constant buffer or not?),
    // let's go ahead and assign the container variable a relative offset
    // of zero for each of the kinds of resources that it consumes.
    //
    for( auto typeResInfo : containerTypeLayout->resourceInfos )
    {
        containerVarLayout->findOrAddResourceInfo(typeResInfo.kind);
    }

    // Because the container's resource allocation is logically coming
    // first in the overall group, the element needs to have a layout
    // such that it comes *after* the container in the relative order.
    //
    for( auto elementTypeResInfo : rawElementTypeLayout->resourceInfos )
    {
        auto kind = elementTypeResInfo.kind;

        // TODO: Added to make layout work correctly for CPU target 
        if(kind == LayoutResourceKind::Uniform)
        {
            continue;
        }

        auto elementVarResInfo = elementVarLayout->findOrAddResourceInfo(kind);

        // If the container part of things is using the same resource kind
        // as the element type, then the element needs to start at an offset
        // after the container.
        //
        if( auto containerTypeResInfo = containerTypeLayout->FindResourceInfo(kind) )
        {
            SLANG_RELEASE_ASSERT(containerTypeResInfo->count.isFinite());
            elementVarResInfo->index += containerTypeResInfo->count.getFiniteValue();
        }
    }

    // Next, resource usage from the container and element
    // types may need to "bleed through" to the overall
    // parameter group type.
    //
    // If the parameter group is a `ConstantBuffer<Foo>` then
    // any ordinary/uniform bytes consumed by `Foo` are masked,
    // but any other resources it consumes (e.g. `binding`s) need
    // to bleed through and be accounted for in the overall
    // layout of the type.
    //
    // If we have a `ParameterBlock<Foo>` then any ordinary/uniform
    // bytes are masked. Furthermore, *if* a whole `space`/`set`
    // was allocated to the block, then any `register`s or
    // `binding`s consumed by `Foo` (and by the "container" constant
    // buffer if we allocated one) are also masked. Any whole
    // spaces/sets consumed by `Foo` need to bleed through.
    //
    // We can start with the easier case of the container type,
    // since it will either be empty or consume a single constant
    // buffer. Its resource usage will only bleed through if we
    // didn't allocate a full `space` or `set`.
    //
    _addUnmaskedResourceUsage(true, typeLayout, containerTypeLayout, wantSpaceOrSet);

    // next we turn to the element type, where the cases are slightly
    // more involved (technically we could use this same logic for
    // the container, as it is more general, but it was simpler to
    // just special-case the container).
    //

    _addUnmaskedResourceUsage(false, typeLayout, rawElementTypeLayout, wantSpaceOrSet);

    // At this point we have handled all the complexities that
    // arise for a parameter group that doesn't include interface-type
    // fields, or that doesn't include specialization for those fields.
    //
    // The remaining complexity all arises if we have interface-type
    // data in the parameter group, and we are specializing it to
    // concrete types, that will have their own layout requirements.
    // In those cases there will be "pending data" on the element
    // type layout that need to get placed somwhere, but wasn't
    // included in the layout computed so far.
    //
    // All of this is extra work we only have to do if there is
    // "pending" data in the element type layout.
    //
    if( auto pendingElementTypeLayout = rawElementTypeLayout->pendingDataTypeLayout )
    {
        auto rules = rawElementTypeLayout->rules;

        // One really annoying complication we need to deal with here
        // its that it is possible that the original parameter group
        // declaration didn't need a constant buffer or `space`/`set`
        // to be allocated, but once we consider the "pending" data
        // we need to have a constant buffer and/or space.
        //
        // We will compute whether the pending data create a demand
        // for a constant buffer and/or a space/set, so that we know
        // if we are in the tricky case.
        //
        bool pendingDataWantsConstantBuffer = _usesOrdinaryData(pendingElementTypeLayout);
        bool pendingDataWantsSpaceOrSet = false;
        if( canUseSpaceOrSet )
        {
            for( auto resInfo : pendingElementTypeLayout->resourceInfos )
            {
                if( resInfo.kind != LayoutResourceKind::RegisterSpace )
                {
                    pendingDataWantsSpaceOrSet = true;
                    break;
                }
            }
        }

        // We will use a few different variables to track resource
        // usage for the pending data, with roles similar to the
        // umbrella type layout, container layout, and element layout
        // that already came up for the main part of the parameter group type.


        RefPtr<TypeLayout> pendingContainerTypeLayout = new TypeLayout();
        pendingContainerTypeLayout->type = parameterGroupType;
        pendingContainerTypeLayout->rules = parameterGroupRules;

        containerTypeLayout->pendingDataTypeLayout = pendingContainerTypeLayout;

        RefPtr<VarLayout> pendingContainerVarLayout = new VarLayout();
        pendingContainerVarLayout->typeLayout = pendingContainerTypeLayout;

        containerVarLayout->pendingVarLayout = pendingContainerVarLayout;


        RefPtr<VarLayout> pendingElementVarLayout = new VarLayout();
        pendingElementVarLayout->typeLayout = pendingElementTypeLayout;

        elementVarLayout->pendingVarLayout = pendingElementVarLayout;

        // If we need a space/set for the pending data, and don't already
        // have one, then we will allocate it now, as part of the
        // "full" data type.
        //
        if( pendingDataWantsSpaceOrSet && !wantSpaceOrSet )
        {
            pendingContainerTypeLayout->addResourceUsage(LayoutResourceKind::RegisterSpace, 1);

            // From here on, we know we have access to a register space,
            // and we can mask any registers/bindings appropriately.
            //
            wantSpaceOrSet = true;
        }

        // If we need a constant buffer for laying out ordinary
        // data, and didn't have one allocated before, we will create
        // one.
        //
        if( pendingDataWantsConstantBuffer && !wantConstantBuffer )
        {
            auto cbUsage = rules->GetObjectLayout(ShaderParameterKind::ConstantBuffer);
            pendingContainerTypeLayout->addResourceUsage(cbUsage.kind, cbUsage.size);

            wantConstantBuffer = true;
        }

        for( auto resInfo : pendingContainerTypeLayout->resourceInfos )
        {
            pendingContainerVarLayout->findOrAddResourceInfo(resInfo.kind);
        }

        // Now that we've added in the resource usage for any CB or set/space
        // we needed to allocate just for the pending data, we can safely
        // lay out the pending data itself.
        //
        // The ordinary/uniform part of things wil always be "masked" and
        // needs to come after any uniform data from the original element type.
        //
        // To kick things off we will initialize state for `struct` type layout,
        // so that we can lay out the pending data as if it were the second
        // field in a structure type, after the original data.
        //
        UniformLayoutInfo uniformLayout = rules->BeginStructLayout();
        if( auto resInfo = rawElementTypeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
        {
            uniformLayout.alignment = rawElementTypeLayout->uniformAlignment;
            uniformLayout.size = resInfo->count;
        }

        // Now we can scan through the resources used by the pending data.
        //
        for( auto resInfo : pendingElementTypeLayout->resourceInfos )
        {
            if( resInfo.kind == LayoutResourceKind::Uniform )
            {
                // For the ordinary/uniform resource kind, we will add the resource
                // usage as a structure field, and then write the resulting offset
                // into the variable layout for the pending data.
                //
                auto offset = rules->AddStructField(
                    &uniformLayout,
                    UniformLayoutInfo(
                        resInfo.count,
                        pendingElementTypeLayout->uniformAlignment));
                pendingElementVarLayout->findOrAddResourceInfo(resInfo.kind)->index = offset.getFiniteValue();
            }
            else
            {
                // For all other resource kinds, we will set the offset in
                // the variable layout based on the total resources of that
                // kind seen so far (including the "container" if any),
                // and then bump the count for total resource usage.
                //
                auto elementVarResInfo = pendingElementVarLayout->findOrAddResourceInfo(resInfo.kind);
                if( auto containerTypeInfo = pendingContainerTypeLayout->FindResourceInfo(resInfo.kind) )
                {
                    elementVarResInfo->index = containerTypeInfo->count.getFiniteValue();
                }
            }
        }
        rules->EndStructLayout(&uniformLayout);

        // Okay, now we have a `VarLayout` for the element data, and an overall `TypeLayout`
        // for all the data that this parameter group needs allocated for pending
        // data.
        //
        // The next major step is to compute the version of that combined resource usage
        // that will "bleed through" and thus needs to be allocated at the next level
        // up the hierarchy.
        //
        RefPtr<TypeLayout> unmaskedPendingDataTypeLayout = new TypeLayout();
        _addUnmaskedResourceUsage(true, unmaskedPendingDataTypeLayout, pendingContainerTypeLayout, wantSpaceOrSet);
        _addUnmaskedResourceUsage(false, unmaskedPendingDataTypeLayout, pendingElementTypeLayout, wantSpaceOrSet);

        // TODO: we should probably optimize for the case where there is no unmasked
        // usage that needs to be reported out, since it should be a common case.

        // Now we need to update the type layout to  what we've done.
        //
        typeLayout->pendingDataTypeLayout = unmaskedPendingDataTypeLayout;

        // TODO: we should probably adjust the size reported by the element type
        // to include any "pending" data that was allocated into the group, so
        // that it can be easier for client code to allocate their instances.
    }

    // The existing Slang reflection API was created before we really
    // understood the wrinkle that the "container" and elements parts
    // of a parameter group could collide on some resource kinds,
    // so the API doesn't currently expose the nice `VarLayout`s we've
    // just computed.
    //
    // Instead, the API allows the user to query the element type layout
    // for the group, and the user just assumes that the offsetting
    // is magically applied there. To go back to the earlier example:
    //
    //      struct MyMaterial { Texture2D t; SamplerState s; };
    //      ConstantBuffer<MyMaterial> gMaterial;
    //
    // A user of the existing reflection API expects to be able to
    // query the `binding` of `gMaterial` and get back zero, then
    // query the `binding` of the `t` field of the element type
    // and get *one*. It is clear that in the abstract, the
    // `MyMaterial::t` field should have an offset of zero (as
    // the first field in a `struct`), so to meet the user's
    // expectations, some cleverness is needed.
    //
    // We will use a subroutine `applyOffsetToTypeLayout`
    // that tries to recursively walk an existing `TypeLayout`
    // and apply an offset to its fields. This is currently
    // quite ad hoc, but that doesn't matter much as it
    // handles `struct` types which are the 99% case for
    // parameter blocks.
    //
    typeLayout->offsetElementTypeLayout = applyOffsetToTypeLayout(rawElementTypeLayout, elementVarLayout);

    return typeLayout;
}

    /// Do we need to wrap the given element type in a constant buffer layout?
static bool needsConstantBuffer(RefPtr<TypeLayout> elementTypeLayout)
{
    // We need a constant buffer if the element type has ordinary/uniform data.
    //
    if(_usesOrdinaryData(elementTypeLayout))
        return true;

    // We also need a constant buffer if there is any "pending"
    // data that need ordinary/uniform data allocated to them.
    //
    if(auto pendingDataTypeLayout = elementTypeLayout->pendingDataTypeLayout)
    {
        if(_usesOrdinaryData(pendingDataTypeLayout))
            return true;
    }

    return false;
}

RefPtr<TypeLayout> createConstantBufferTypeLayoutIfNeeded(
    TypeLayoutContext const&    context,
    RefPtr<TypeLayout>          elementTypeLayout)
{
    // First things first, we need to check whether the element type
    // we are trying to lay out even needs a constant buffer allocated
    // for it.
    //
    if(!needsConstantBuffer(elementTypeLayout))
        return elementTypeLayout;

    auto parameterGroupRules = context.getRulesFamily()->getConstantBufferRules();

    return _createParameterGroupTypeLayout(
        context
            .with(parameterGroupRules)
            .with(context.targetReq->getDefaultMatrixLayoutMode()),
        nullptr,
        elementTypeLayout);
}


static RefPtr<TypeLayout> _createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType,
    RefPtr<Type>                elementType,
    LayoutRulesImpl*            elementTypeRules)
{
    // We will first compute a layout for the element type of
    // the parameter group.
    //
    auto elementTypeLayout = createTypeLayout(
        context.with(elementTypeRules),
        elementType);

    // Now we delegate to a routine that does the meat of
    // the complicated layout logic.
    //
    return _createParameterGroupTypeLayout(
        context,
        parameterGroupType,
        elementTypeLayout);
}

LayoutRulesImpl* getParameterBufferElementTypeLayoutRules(
    RefPtr<ParameterGroupType>  parameterGroupType,
    LayoutRulesImpl*            rules)
{
    if( as<ConstantBufferType>(parameterGroupType) )
    {
        return rules->getLayoutRulesFamily()->getConstantBufferRules();
    }
    else if( as<TextureBufferType>(parameterGroupType) )
    {
        return rules->getLayoutRulesFamily()->getTextureBufferRules();
    }
    else if( as<GLSLInputParameterGroupType>(parameterGroupType) )
    {
        return rules->getLayoutRulesFamily()->getVaryingInputRules();
    }
    else if( as<GLSLOutputParameterGroupType>(parameterGroupType) )
    {
        return rules->getLayoutRulesFamily()->getVaryingOutputRules();
    }
    else if( as<GLSLShaderStorageBufferType>(parameterGroupType) )
    {
        return rules->getLayoutRulesFamily()->getShaderStorageBufferRules();
    }
    else if (as<ParameterBlockType>(parameterGroupType))
    {
        return rules->getLayoutRulesFamily()->getParameterBlockRules();
    }
    else
    {
        SLANG_UNEXPECTED("uhandled parameter block type");
        return nullptr;
    }
}

RefPtr<TypeLayout> createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType)
{
    auto parameterGroupRules = context.rules;

    // Determine the layout rules to use for the contents of the block
    auto elementTypeRules = getParameterBufferElementTypeLayoutRules(
        parameterGroupType,
        parameterGroupRules);

    auto elementType = parameterGroupType->elementType;

    return _createParameterGroupTypeLayout(
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
    // look up the appropriate rules via the `LayoutRulesFamily` 
    auto structuredBufferLayoutRules = context.getRulesFamily()->getStructuredBufferRules();

    // Create and save type layout for the buffer contents.
    auto elementTypeLayout = createTypeLayout(
        context.with(structuredBufferLayoutRules),
        elementType.Ptr());

    return createStructuredBufferTypeLayout(
        context,
        kind,
        structuredBufferType,
        elementTypeLayout);

}

    /// Create layout information for the given `type`.
    ///
    /// This internal routine returns both the constructed type
    /// layout object and the simple layout info, encapsulated
    /// together as a `TypeLayoutResult`.
    ///
static TypeLayoutResult _createTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type);

    /// Create layout information for the given `type`, obeying any layout modifiers on the given declaration.
    ///
    /// If `declForModifiers` has any matrix layout modifiers associated with it, then
    /// the resulting type layout will respect those modifiers.
    ///
static TypeLayoutResult _createTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type,
    Decl*                       declForModifiers)
{
    TypeLayoutContext subContext = context;

    if (declForModifiers)
    {
        // TODO: The approach implemented here has a row/column-major
        // layout model recursively affect any sub-fields (so that
        // the layout of a nested struct depends on the context where
        // it is nested). This is consistent with the GLSL behavior
        // for these modifiers, but it is *not* how HLSL is supposed
        // to work.
        //
        // In the trivial case where `row_major` and `column_major`
        // are only applied to leaf fields/variables of matrix type
        // the difference should be immaterial.

        if (declForModifiers->HasModifier<RowMajorLayoutModifier>())
            subContext.matrixLayoutMode = kMatrixLayoutMode_RowMajor;

        if (declForModifiers->HasModifier<ColumnMajorLayoutModifier>())
            subContext.matrixLayoutMode = kMatrixLayoutMode_ColumnMajor;

        // TODO: really need to look for other modifiers that affect
        // layout, such as GLSL `std140`.
    }

    return _createTypeLayout(subContext, type);
}

RefPtr<Type> findGlobalGenericSpecializationArg(
    TypeLayoutContext const&    context,
    GlobalGenericParamDecl*     decl)
{
    RefPtr<Val> arg;
    context.programLayout->globalGenericArgs.TryGetValue(decl, arg);
    return arg.as<Type>();
}

Index findGlobalGenericSpecializationParamIndex(
    ComponentType*          type,
    GlobalGenericParamDecl* decl)
{
    Index paramCount = type->getSpecializationParamCount();
    for( Index pp = 0; pp < paramCount; ++pp )
    {
        auto param = type->getSpecializationParam(pp);
        if(param.flavor != SpecializationParam::Flavor::GenericType)
            continue;
        if(param.object.Ptr() != decl)
            continue;

        return pp;
    }
    return -1;
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
    if( auto originalArrayTypeLayout = originalTypeLayout.as<ArrayTypeLayout>() )
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
    else if(auto originalParameterGroupTypeLayout = originalTypeLayout.as<ParameterGroupTypeLayout>() )
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
    else if(auto originalStructTypeLayout = originalTypeLayout.as<StructTypeLayout>() )
    {
        Index fieldCount = originalStructTypeLayout->fields.getCount();

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
                        // and it will start at register zero in that space.
                        //
                        resInfo.index = 0;
                        resInfo.space = spaceOffsetForField.getFiniteValue();
                    }
                }
            }

            adjustedStructTypeLayout->fields.add(adjustedField);

            mapOriginalFieldToAdjusted.Add(originalField, adjustedField);
        }

        for( auto p : originalStructTypeLayout->mapVarToLayout )
        {
            VarDeclBase* key = p.Key;
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

    /// Convert a `TypeLayout` to a `TypeLayoutResult`
    ///
    /// A `TypeLayout` holds all the data needed to make a `TypeLayoutResult` in practice,
    /// but sometimes it is more convenient to have the data split out.
    ///
TypeLayoutResult makeTypeLayoutResult(RefPtr<TypeLayout> typeLayout)
{
    TypeLayoutResult result;
    result.layout = typeLayout;

    // If the type only consumes a single kind of non-uniform resource,
    // we can fill in the `info` field directly.
    //
    if( typeLayout->resourceInfos.getCount() == 1 )
    {
        auto resInfo = typeLayout->resourceInfos[0];
        if( resInfo.kind != LayoutResourceKind::Uniform )
        {
            result.info.kind = resInfo.kind;
            result.info.size = resInfo.count;
            return result;
        }
    }

    // Otherwise, we will fill out the info based on the uniform
    // resources consumed, if any.
    //
    if( auto resInfo = typeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
    {
        result.info.kind = LayoutResourceKind::Uniform;
        result.info.alignment = typeLayout->uniformAlignment;
        result.info.size = resInfo->count;
    }

    // If there was no ordinary/uniform resource usage, then we
    // will leave the `info` field in its default state (which
    // shows no resources consumed).
    //
    // The type layout might have more detailed information, but
    // at this point it must contain either zero, or more than one
    // `ResourceInfo`, so there is nothing unambiguous we can
    // store into `info`.

    return result;
}

//
// StructTypeLayoutBuilder
//

void StructTypeLayoutBuilder::beginLayout(
    Type*               type,
    LayoutRulesImpl*    rules)
{
    m_rules = rules;

    m_typeLayout = new StructTypeLayout();
    m_typeLayout->type = type;
    m_typeLayout->rules = m_rules;

    m_info = m_rules->BeginStructLayout();
}

void StructTypeLayoutBuilder::beginLayoutIfNeeded(
    Type*               type,
    LayoutRulesImpl*    rules)
{
    if( !m_typeLayout )
    {
        beginLayout(type, rules);
    }
}

RefPtr<VarLayout> StructTypeLayoutBuilder::addField(
    DeclRef<VarDeclBase>    field,
    TypeLayoutResult        fieldResult)
{
    SLANG_ASSERT(m_typeLayout);

    RefPtr<TypeLayout> fieldTypeLayout = fieldResult.layout;
    UniformLayoutInfo fieldInfo = fieldResult.info.getUniformLayout();

    // Note: we don't add any zero-size fields
    // when computing structure layout, just
    // to avoid having a resource type impact
    // the final layout.
    //
    // This means that the code to generate final
    // declarations needs to *also* eliminate zero-size
    // fields to be safe...
    //
    LayoutSize uniformOffset = m_info.size;
    if(fieldInfo.size != 0)
    {
        uniformOffset = m_rules->AddStructField(&m_info, fieldInfo);
    }


    // We need to create variable layouts
    // for each field of the structure.
    RefPtr<VarLayout> fieldLayout = new VarLayout();
    fieldLayout->varDecl = field;
    fieldLayout->typeLayout = fieldTypeLayout;
    m_typeLayout->fields.add(fieldLayout);

    if( field )
    {
        m_typeLayout->mapVarToLayout.Add(field.getDecl(), fieldLayout);
    }

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
            auto structTypeSpaceResourceInfo = m_typeLayout->findOrAddResourceInfo(LayoutResourceKind::RegisterSpace);
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
            auto structTypeResourceInfo = m_typeLayout->findOrAddResourceInfo(fieldTypeResourceInfo.kind);
            fieldResourceInfo->index = structTypeResourceInfo->count.getFiniteValue();
            structTypeResourceInfo->count += fieldTypeResourceInfo.count;
        }
    }

    return fieldLayout;
}

RefPtr<VarLayout> StructTypeLayoutBuilder::addField(
    DeclRef<VarDeclBase>    field,
    RefPtr<TypeLayout>      fieldTypeLayout)
{
    TypeLayoutResult fieldResult = makeTypeLayoutResult(fieldTypeLayout);
    return addField(field, fieldResult);
}

void StructTypeLayoutBuilder::endLayout()
{
    if(!m_typeLayout) return;

    m_rules->EndStructLayout(&m_info);

    m_typeLayout->uniformAlignment = m_info.alignment;
    m_typeLayout->addResourceUsage(LayoutResourceKind::Uniform, m_info.size);
}

RefPtr<StructTypeLayout> StructTypeLayoutBuilder::getTypeLayout()
{
    return m_typeLayout;
}

TypeLayoutResult StructTypeLayoutBuilder::getTypeLayoutResult()
{
    return TypeLayoutResult(m_typeLayout, m_info);
}

static TypeLayoutResult _createTypeLayoutForGlobalGenericTypeParam(
    TypeLayoutContext const&    context,
    Type*                       type,
    GlobalGenericParamDecl*     globalGenericParamDecl)
{
    SimpleLayoutInfo info;
    info.alignment = 0;
    info.size = 0;
    info.kind = LayoutResourceKind::GenericResource;

    RefPtr<GenericParamTypeLayout> typeLayout = new GenericParamTypeLayout();
    // we should have already populated ProgramLayout::genericEntryPointParams list at this point,
    // so we can find the index of this generic param decl in the list
    typeLayout->type = type;
    typeLayout->paramIndex = findGlobalGenericSpecializationParamIndex(
        context.programLayout->getProgram(),
        globalGenericParamDecl);
    typeLayout->rules = context.rules;
    typeLayout->findOrAddResourceInfo(LayoutResourceKind::GenericResource)->count += 1;

    return TypeLayoutResult(typeLayout, info);
}

RefPtr<TypeLayout> createTypeLayoutForGlobalGenericTypeParam(
    TypeLayoutContext const&    context,
    Type*                       type,
    GlobalGenericParamDecl*     globalGenericParamDecl)
{
    return _createTypeLayoutForGlobalGenericTypeParam(context, type, globalGenericParamDecl).layout;
}

static TypeLayoutResult _createTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type)
{
    auto rules = context.rules;

    if (auto parameterGroupType = as<ParameterGroupType>(type))
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
        auto typeLayout = createParameterGroupTypeLayout(
            context,
            parameterGroupType); 

        return TypeLayoutResult(typeLayout, info);
    }
    else if (auto samplerStateType = as<SamplerStateType>(type))
    {
        return createSimpleTypeLayout(
            rules->GetObjectLayout(ShaderParameterKind::SamplerState),
            type,
            rules);
    }
    else if (auto textureType = as<TextureType>(type))
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

        return createSimpleTypeLayout(
            rules->GetObjectLayout(kind),
            type,
            rules);
    }
    else if (auto imageType = as<GLSLImageType>(type))
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

        return createSimpleTypeLayout(
            rules->GetObjectLayout(kind),
            type,
            rules);
    }
    else if (auto textureSamplerType = as<TextureSamplerType>(type))
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

        return createSimpleTypeLayout(
            rules->GetObjectLayout(kind),
            type,
            rules);
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, KIND)                                                \
    else if(auto type_##TYPE = as<TYPE>(type)) do {                     \
        auto info = rules->GetObjectLayout(ShaderParameterKind::KIND);  \
        auto typeLayout = createStructuredBufferTypeLayout(             \
                context,                                                \
                ShaderParameterKind::KIND,                              \
                type_##TYPE,                                            \
                type_##TYPE->elementType.Ptr());                        \
        return TypeLayoutResult(typeLayout, info);                      \
    } while(0)

    CASE(HLSLStructuredBufferType,                  StructuredBuffer);
    CASE(HLSLRWStructuredBufferType,                MutableStructuredBuffer);
    CASE(HLSLRasterizerOrderedStructuredBufferType, MutableStructuredBuffer);
    CASE(HLSLAppendStructuredBufferType,            MutableStructuredBuffer);
    CASE(HLSLConsumeStructuredBufferType,           MutableStructuredBuffer);

#undef CASE


    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, KIND)                                        \
    else if(as<TYPE>(type)) do {                              \
        return createSimpleTypeLayout(                             \
            rules->GetObjectLayout(ShaderParameterKind::KIND),  \
            type, rules);                        \
    } while(0)

    CASE(HLSLByteAddressBufferType,                     RawBuffer);
    CASE(HLSLRWByteAddressBufferType,                   MutableRawBuffer);
    CASE(HLSLRasterizerOrderedByteAddressBufferType,    MutableRawBuffer);

    CASE(GLSLInputAttachmentType,           InputRenderTarget);

    // This case is mostly to allow users to add new resource types...
    CASE(UntypedBufferResourceType,         RawBuffer);

#undef CASE

    else if(auto basicType = as<BasicExpressionType>(type))
    {
        return createSimpleTypeLayout(
            rules->GetScalarLayout(basicType->baseType),
            type,
            rules);
    }
    else if(auto vecType = as<VectorExpressionType>(type))
    {
        auto elementType = vecType->elementType;
        size_t elementCount = (size_t) GetIntVal(vecType->elementCount);

        auto element = _createTypeLayout(
            context,
            elementType);

        BaseType elementBaseType = BaseType::Void;
        if (auto elementBasicType = as<BasicExpressionType>(elementType))
        {
            elementBaseType = elementBasicType->baseType;
        }

        auto info = rules->GetVectorLayout(elementBaseType, element.info, elementCount);

        RefPtr<VectorTypeLayout> typeLayout = new VectorTypeLayout();
        typeLayout->type = type;
        typeLayout->rules = rules;
        typeLayout->uniformAlignment = info.alignment;

        typeLayout->elementTypeLayout = element.layout;
        typeLayout->uniformStride = element.info.getUniformLayout().size.getFiniteValue();

        typeLayout->addResourceUsage(info.kind, info.size);

        return TypeLayoutResult(typeLayout, info);
    }
    else if(auto matType = as<MatrixExpressionType>(type))
    {
        size_t rowCount = (size_t) GetIntVal(matType->getRowCount());
        size_t colCount = (size_t) GetIntVal(matType->getColumnCount());

        auto elementType = matType->getElementType();
        auto elementResult = _createTypeLayout(
            context,
            elementType);
        auto elementTypeLayout = elementResult.layout;
        auto elementInfo = elementResult.info;

        BaseType elementBaseType = BaseType::Void;
        if (auto elementBasicType = as<BasicExpressionType>(elementType))
        {
            elementBaseType = elementBasicType->baseType;
        }

        // The `GetMatrixLayout` implementation in the layout rules
        // currently defaults to assuming row-major layout,
        // so if we want column-major layout we achieve it here by
        // transposing the major/minor axes counts.
        //
        size_t layoutMajorCount = rowCount;
        size_t layoutMinorCount = colCount;
        if (context.matrixLayoutMode == kMatrixLayoutMode_ColumnMajor)
        {
            size_t tmp = layoutMajorCount;
            layoutMajorCount = layoutMinorCount;
            layoutMinorCount = tmp;
        }
        auto info = rules->GetMatrixLayout(
            elementBaseType,
            elementInfo,
            layoutMajorCount,
            layoutMinorCount);

        auto rowType = matType->getRowType();
        RefPtr<VectorTypeLayout> rowTypeLayout = new VectorTypeLayout();

        auto rowInfo = rules->GetVectorLayout(
            elementBaseType,
            elementInfo,
            colCount);

        size_t majorStride = info.elementStride;
        size_t minorStride = elementInfo.getUniformLayout().size.getFiniteValue();

        size_t rowStride = 0;
        size_t colStride = 0;
        if(context.matrixLayoutMode == kMatrixLayoutMode_ColumnMajor)
        {
            colStride = majorStride;
            rowStride = minorStride;
        }
        else
        {
            rowStride = majorStride;
            colStride = minorStride;
        }

        rowTypeLayout->type = type;
        rowTypeLayout->rules = rules;
        rowTypeLayout->uniformAlignment = elementInfo.getUniformLayout().alignment;

        rowTypeLayout->uniformStride = colStride;
        rowTypeLayout->elementTypeLayout = elementTypeLayout;
        rowTypeLayout->addResourceUsage(rowInfo.kind, rowInfo.size);

        RefPtr<MatrixTypeLayout> typeLayout = new MatrixTypeLayout();

        typeLayout->type = type;
        typeLayout->rules = rules;
        typeLayout->uniformAlignment = info.alignment;

        typeLayout->elementTypeLayout = rowTypeLayout;
        typeLayout->uniformStride = rowStride;
        typeLayout->mode = context.matrixLayoutMode;

        typeLayout->addResourceUsage(info.kind, info.size);

        return TypeLayoutResult(typeLayout, info);
    }
    else if (auto arrayType = as<ArrayExpressionType>(type))
    {
        auto elementResult = _createTypeLayout(
            context,
            arrayType->baseType.Ptr());
        auto elementInfo = elementResult.info;
        auto elementTypeLayout = elementResult.layout;

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

        RefPtr<ArrayTypeLayout> typeLayout = new ArrayTypeLayout();

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
        // account. If it returns the same type layout that was passed in,
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

        return TypeLayoutResult(typeLayout, arrayUniformInfo);
    }
    else if (auto declRefType = as<DeclRefType>(type))
    {
        auto declRef = declRefType->declRef;

        if (auto structDeclRef = declRef.as<StructDecl>())
        {
            StructTypeLayoutBuilder typeLayoutBuilder;
            StructTypeLayoutBuilder pendingDataTypeLayoutBuilder;

            typeLayoutBuilder.beginLayout(type, rules);
            auto typeLayout = typeLayoutBuilder.getTypeLayout();
            for (auto field : GetFields(structDeclRef))
            {
                // Static fields shouldn't take part in layout.
                if(field.getDecl()->HasModifier<HLSLStaticModifier>())
                    continue;

                // The fields of a `struct` type may include existential (interface)
                // types (including as nested sub-fields), and any types present
                // in those fields will need to be specialized based on the
                // input arguments being passed to `_createTypeLayout`.
                //
                // We won't know how many type slots each field consumes until
                // we process it, but we can figure out the starting index for
                // the slots its will consume by looking at the layout we've
                // computed so far.
                //
                Int baseExistentialSlotIndex = 0;
                if(auto resInfo = typeLayout->FindResourceInfo(LayoutResourceKind::ExistentialTypeParam))
                    baseExistentialSlotIndex = Int(resInfo->count.getFiniteValue());
                //
                // When computing the layout for the field, we will give it access
                // to all the incoming specialized type slots that haven't already
                // been consumed/claimed by preceding fields.
                //
                auto fieldLayoutContext = context.withSpecializationArgsOffsetBy(baseExistentialSlotIndex);

                TypeLayoutResult fieldResult = _createTypeLayout(
                    fieldLayoutContext,
                    GetType(field).Ptr(),
                    field.getDecl());
                auto fieldTypeLayout = fieldResult.layout;

                auto fieldVarLayout = typeLayoutBuilder.addField(field, fieldResult);

                // If any of the fields of the `struct` type had existential/interface
                // type, then we need to compute a second `StructTypeLayout` that
                // represents the layout and resource using for the "pending data"
                // that this type needs to have stored somewhere, but which can't
                // be laid out in the layout of the type itself.
                //
                if(auto fieldPendingDataTypeLayout = fieldTypeLayout->pendingDataTypeLayout)
                {
                    // We only create this secondary layout on-demand, so that
                    // we don't end up with a bunch of empty structure type layouts
                    // created for no reason.
                    //
                    pendingDataTypeLayoutBuilder.beginLayoutIfNeeded(type, rules);
                    auto fieldPendingVarLayout = pendingDataTypeLayoutBuilder.addField(field, fieldPendingDataTypeLayout);
                    fieldVarLayout->pendingVarLayout = fieldPendingVarLayout;
                }
            }

            typeLayoutBuilder.endLayout();
            pendingDataTypeLayoutBuilder.endLayout();

            if( auto pendingDataTypeLayout = pendingDataTypeLayoutBuilder.getTypeLayout() )
            {
                typeLayout->pendingDataTypeLayout = pendingDataTypeLayout;
            }

            return typeLayoutBuilder.getTypeLayoutResult();
        }
        else if (auto globalGenericParamDecl = declRef.as<GlobalGenericParamDecl>())
        {
            if( auto concreteType = findGlobalGenericSpecializationArg(
                context,
                globalGenericParamDecl) )
            {
                // If we know what concrete type has been used to specialize
                // the global generic type parameter, then we should use
                // the concrete type instead.
                //
                return _createTypeLayout(context, concreteType);
            }
            else
            {
                // Otherwise we must create a type layout that represents
                // the generic type parameter itself.
                //
                return _createTypeLayoutForGlobalGenericTypeParam(context, type, globalGenericParamDecl);
            }
        }
        else if (auto assocTypeParam = declRef.as<AssocTypeDecl>())
        {
            return createSimpleTypeLayout(
                SimpleLayoutInfo(),
                type,
                rules);
        }
        else if( auto simpleGenericParam = declRef.as<GenericTypeParamDecl>() )
        {
            // A bare generic type parameter can come up during layout
            // of a generic entry point (or an entry point nested in
            // a generic type). For now we will just pretend like
            // the fields of generic parameter type take no space,
            // since there is no reasonable way to account for them
            // in the resulting layout.
            //
            // TODO: It might be better to completely ignore generic
            // entry points during initial layout, but doing so would
            // mean that users couldn't get layout information on
            // any parameters, even those that don't depend on
            // generics.
            //
            return createSimpleTypeLayout(
                SimpleLayoutInfo(),
                type,
                rules);
        }
        else if( auto interfaceDeclRef = declRef.as<InterfaceDecl>() )
        {
            // When laying out a type that includes interface-type fields,
            // we cannot know how much space the concrete type that
            // gets stored into the field consumes.
            //
            // If we were doing layout for a typical CPU target, then
            // we could just say that each interface-type field consumes
            // some fixed number of pointers (e.g., a data pointer plus a witness
            // table pointer).
            //
            // We will borrow the intuition from that and invent a new
            // resource kind for "existential slots" which conceptually
            // represents the indirections needed to reference the
            // data to be referenced by this field.
            //

            RefPtr<TypeLayout> typeLayout = new TypeLayout();
            typeLayout->type = type;
            typeLayout->rules = rules;

            typeLayout->addResourceUsage(LayoutResourceKind::ExistentialTypeParam, 1);
            typeLayout->addResourceUsage(LayoutResourceKind::ExistentialObjectParam, 1);

            // If there are any concrete types available, the first one will be
            // the value that should be plugged into the slot we just introduced.
            //
            if( context.specializationArgCount )
            {
                auto& specializationArg = context.specializationArgs[0];
                RefPtr<Type> concreteType = specializationArg.val.as<Type>();
                SLANG_ASSERT(concreteType);

                RefPtr<TypeLayout> concreteTypeLayout = createTypeLayout(context, concreteType);

                // Layout for this specialized interface type then results
                // in a type layout that tracks both the resource usage of the
                // interface type itself (just the type + value slots introduced
                // above), plus a "pending data" type that represents the value
                // conceptually pointed to by the interface-type field/variable at runtime.
                //
                typeLayout->pendingDataTypeLayout = concreteTypeLayout;
            }

            return TypeLayoutResult(typeLayout, SimpleLayoutInfo());
        }
    }
    else if (auto errorType = as<ErrorType>(type))
    {
        // An error type means that we encountered something we don't understand.
        //
        // We should probably inform the user with an error message here.

        return createSimpleTypeLayout(
            SimpleLayoutInfo(),
            type,
            rules);
    }
    else if( auto taggedUnionType = as<TaggedUnionType>(type) )
    {
        // A tagged union type needs to be laid out as the maximum
        // size of any constituent type.
        //
        // In practice, only a tagged union of uniform data will
        // work, but for now we will compute the maximum usage
        // for each resource kind for generality.
        //
        // For the uniform data we will start with a size
        // of zero and an alignment of one for our base case
        // (this is what a tagged union of no cases would consume).
        //
        UniformLayoutInfo info(0, 1);

        RefPtr<TaggedUnionTypeLayout> taggedUnionLayout = new TaggedUnionTypeLayout();
        taggedUnionLayout->type = type;
        taggedUnionLayout->rules = rules;

        // Now we iterate over the case types and see if they
        // change our computed maximum size/alignement.
        //
        for( auto caseType : taggedUnionType->caseTypes )
        {
            // Note: A tagged union type is not expected to have any existential/interface type
            // slots; the case types that are provided must be fully specialized before the union is
            // formed. Thus we don't need to mess around with existential type slots here the
            // way we do for the `struct` case.

            auto caseTypeResult = _createTypeLayout(context, caseType);
            RefPtr<TypeLayout> caseTypeLayout = caseTypeResult.layout;
            UniformLayoutInfo caseTypeInfo = caseTypeResult.info.getUniformLayout();

            info.size      = maximum(info.size, caseTypeInfo.size);
            info.alignment = std::max(info.alignment, caseTypeInfo.alignment);

            // We need to remember the layout of the case type
            // on the final `TaggedUnionTypeLayout`.
            //
            taggedUnionLayout->caseTypeLayouts.add(caseTypeLayout);

            // We also need to consider contributions for other
            // resource kinds beyond uniform data.
            //
            for( auto caseResInfo : caseTypeLayout->resourceInfos )
            {
                auto unionResInfo = taggedUnionLayout->findOrAddResourceInfo(caseResInfo.kind);
                unionResInfo->count = maximum(unionResInfo->count, caseResInfo.count);
            }
        }

        // After we've computed the size required to hold all the
        // case types, we will allocate space for the tag field.
        //
        // TODO: This assumes the tag will always be allocated out
        // of uniform storage, which means we can't support a tagged
        // union as part of a varying input/output signature. That is
        // probably a valid limitation, but it should get enforced
        // somewhere along the way.
        //
        {
            // The tag is always a `uint` for now.
            //
            auto tagInfo = context.rules->GetScalarLayout(BaseType::UInt);
            info.size = RoundToAlignment(info.size, tagInfo.alignment);

            taggedUnionLayout->tagOffset = info.size;

            info.size += tagInfo.size;
            info.alignment = std::max(info.alignment, tagInfo.alignment);
        }

        // As a final step, if we are computing a full `TypeLayout`
        // we will make sure that its information on uniform layout
        // matches what we've computed in the `UniformLayoutInfo` we return.
        //
        taggedUnionLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->count = info.size;
        taggedUnionLayout->uniformAlignment = info.alignment;

        return TypeLayoutResult(taggedUnionLayout, info);
    }
    else if( auto existentialSpecializedType = as<ExistentialSpecializedType>(type) )
    {
        TypeLayoutContext subContext = context.withSpecializationArgs(
            existentialSpecializedType->args.getBuffer(),
            existentialSpecializedType->args.getCount());

        auto baseTypeLayoutResult = _createTypeLayout(
            subContext,
            existentialSpecializedType->baseType);

        UniformLayoutInfo info = rules->BeginStructLayout();
        rules->AddStructField(&info, baseTypeLayoutResult.info.getUniformLayout());

        RefPtr<ExistentialSpecializedTypeLayout> typeLayout = new ExistentialSpecializedTypeLayout();
        typeLayout->type = type;
        typeLayout->rules = rules;

        RefPtr<VarLayout> pendingDataVarLayout = new VarLayout();
        if(auto pendingDataTypeLayout = baseTypeLayoutResult.layout->pendingDataTypeLayout)
        {
            pendingDataVarLayout->typeLayout = pendingDataTypeLayout;
            for( auto pendingResInfo : pendingDataTypeLayout->resourceInfos )
            {
                auto kind = pendingResInfo.kind;
                UInt index = 0;
                if( kind == LayoutResourceKind::Uniform )
                {
                    LayoutSize uniformOffset = rules->AddStructField(
                        &info,
                        makeTypeLayoutResult(pendingDataTypeLayout).info.getUniformLayout());

                    index = uniformOffset.getFiniteValue();
                }
                else
                {
                    if(auto primaryResInfo = baseTypeLayoutResult.layout->FindResourceInfo(kind))
                        index = primaryResInfo->count.getFiniteValue();
                }
                pendingDataVarLayout->AddResourceInfo(kind)->index = index;
            }
        }

        typeLayout->baseTypeLayout = baseTypeLayoutResult.layout;
        typeLayout->pendingDataVarLayout = pendingDataVarLayout;

        return makeTypeLayoutResult(typeLayout);
    }

    // catch-all case in case nothing matched
    SLANG_ASSERT(!"unimplemented case in type layout");
    return createSimpleTypeLayout(
        SimpleLayoutInfo(),
        type,
        rules);
}

RefPtr<TypeLayout> getSimpleVaryingParameterTypeLayout(
    TypeLayoutContext const&            context,
    Type*                               type,
    EntryPointParameterDirectionMask    directionMask)
{
    auto rules = context.rules;

    // TODO: This logic should ideally share as much
    // as possible with the `_createTypeLayout` function,
    // to avoid duplication, but we also have to deal
    // with the many ways in which varying parameter
    // layout differs from non-varying layout.

    // We will compute resource consumption for the type
    // as a varying input, output, or both/neither.
    // To avoid duplication, we'll build an array that
    // includes all the layout rules we need to apply.
    //
    int varyingRulesCount = 0;
    LayoutRulesImpl* varyingRules[2];

    if( directionMask & kEntryPointParameterDirection_Input )
    {
        varyingRules[varyingRulesCount++] = context.getRulesFamily()->getVaryingInputRules();
    }
    if( directionMask & kEntryPointParameterDirection_Output )
    {
        varyingRules[varyingRulesCount++] = context.getRulesFamily()->getVaryingOutputRules();
    }

    if(auto basicType = as<BasicExpressionType>(type))
    {
        auto baseType = basicType->baseType;

        RefPtr<TypeLayout> typeLayout = new TypeLayout();
        typeLayout->type = type;
        typeLayout->rules = rules;

        for( int rr = 0; rr < varyingRulesCount; ++rr )
        {
            auto info = varyingRules[rr]->GetScalarLayout(baseType);
            typeLayout->addResourceUsage(info.kind, info.size);
        }

        return typeLayout;
    }
    else if(auto vecType = as<VectorExpressionType>(type))
    {
        auto elementType = vecType->elementType;
        size_t elementCount = (size_t) GetIntVal(vecType->elementCount);

        BaseType elementBaseType = BaseType::Void;
        if( auto elementBasicType = as<BasicExpressionType>(elementType) )
        {
            elementBaseType = elementBasicType->baseType;
        }

        // Note that we do *not* add any resource usage to the type
        // layout for the element type, because we currently cannot count
        // varying parameter usage at a granularity finer than
        // individual "locations."
        //
        RefPtr<TypeLayout> elementTypeLayout = new TypeLayout();
        elementTypeLayout->type = elementType;
        elementTypeLayout->rules = rules;

        RefPtr<VectorTypeLayout> typeLayout = new VectorTypeLayout();
        typeLayout->type = vecType;
        typeLayout->rules = rules;
        typeLayout->elementTypeLayout = elementTypeLayout;

        for( int rr = 0; rr < varyingRulesCount; ++rr )
        {
            auto varyingRuleSet = varyingRules[rr];
            auto elementInfo = varyingRuleSet->GetScalarLayout(elementBaseType);
            auto info = varyingRuleSet->GetVectorLayout(elementBaseType, elementInfo, elementCount);
            typeLayout->addResourceUsage(info.kind, info.size);
        }

        return typeLayout;
    }
    else if(auto matType = as<MatrixExpressionType>(type))
    {
        size_t rowCount = (size_t) GetIntVal(matType->getRowCount());
        size_t colCount = (size_t) GetIntVal(matType->getColumnCount());
        auto elementType = matType->getElementType();

        BaseType elementBaseType = BaseType::Void;
        if( auto elementBasicType = as<BasicExpressionType>(elementType) )
        {
            elementBaseType = elementBasicType->baseType;
        }

        // Just as for `_createTypeLayout`, we need to handle row- and
        // column-major matrices differently, to ensure we get
        // the expected layout.
        //
        // A varying parameter with row-major layout is effectively
        // just an array of row vectors, while a column-major one
        // is just an array of column vectors.
        //
        size_t layoutMajorCount = rowCount;
        size_t layoutMinorCount = colCount;
        if (context.matrixLayoutMode == kMatrixLayoutMode_ColumnMajor)
        {
            size_t tmp = layoutMajorCount;
            layoutMajorCount = layoutMinorCount;
            layoutMinorCount = tmp;
        }

        RefPtr<TypeLayout> elementTypeLayout = new TypeLayout();
        elementTypeLayout->type = elementType;
        elementTypeLayout->rules = rules;

        RefPtr<VectorTypeLayout> rowTypeLayout = new VectorTypeLayout();
        rowTypeLayout->type = matType->getRowType();
        rowTypeLayout->rules = rules;
        rowTypeLayout->elementTypeLayout = elementTypeLayout;

        RefPtr<MatrixTypeLayout> typeLayout = new MatrixTypeLayout();
        typeLayout->type = type;
        typeLayout->rules = rules;
        typeLayout->elementTypeLayout = rowTypeLayout;
        typeLayout->mode = context.matrixLayoutMode;

        for( int rr = 0; rr < varyingRulesCount; ++rr )
        {
            auto varyingRuleSet = varyingRules[rr];
            auto elementInfo = varyingRuleSet->GetScalarLayout(elementBaseType);

            auto info = varyingRuleSet->GetMatrixLayout(elementBaseType, elementInfo, layoutMajorCount, layoutMinorCount);
            typeLayout->addResourceUsage(info.kind, info.size);

            if(context.matrixLayoutMode == kMatrixLayoutMode_RowMajor)
            {
                // For row-major matrices only, we can compute an effective
                // resource usage for the row type.
                auto rowInfo = varyingRuleSet->GetVectorLayout(elementBaseType, elementInfo, colCount);
                rowTypeLayout->addResourceUsage(rowInfo.kind, rowInfo.size);
            }
        }

        return typeLayout;
    }

    // catch-all case in case nothing matched
    SLANG_ASSERT(!"unimplemented case for varying parameter layout");
    return createSimpleTypeLayout(
        SimpleLayoutInfo(),
        type,
        rules).layout;
}

RefPtr<TypeLayout> createTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type)
{
    return _createTypeLayout(context, type).layout;
}

void TypeLayout::removeResourceUsage(LayoutResourceKind kind)
{
    Int infoCount = resourceInfos.getCount();
    for( Int ii = 0; ii < infoCount; ++ii )
    {
        if( resourceInfos[ii].kind == kind )
        {
            resourceInfos.removeAt(ii);
            return;
        }
    }
}


void TypeLayout::addResourceUsageFrom(TypeLayout* otherTypeLayout)
{
    for(auto resInfo : otherTypeLayout->resourceInfos)
        addResourceUsage(resInfo);
}


RefPtr<TypeLayout> TypeLayout::unwrapArray()
{
    TypeLayout* typeLayout = this;

    while(auto arrayTypeLayout = as<ArrayTypeLayout>(typeLayout))
        typeLayout = arrayTypeLayout->elementTypeLayout;

    return typeLayout;
}


RefPtr<GlobalGenericParamDecl> GenericParamTypeLayout::getGlobalGenericParamDecl()
{
    auto declRefType = as<DeclRefType>(type);
    SLANG_ASSERT(declRefType);
    auto rsDeclRef = declRefType->declRef.as<GlobalGenericParamDecl>();
    return rsDeclRef.getDecl();
}

} // namespace Slang
