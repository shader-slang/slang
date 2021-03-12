// render-cpu.cpp
#include "render-cpu.h"

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"
#include "core/slang-basic.h"
#include "core/slang-blob.h"

#include "../command-writer.h"
#include "../renderer-shared.h"
#include "../slang-context.h"

#define SLANG_PRELUDE_NAMESPACE slang_prelude
#include "prelude/slang-cpp-types.h"

namespace gfx
{
using namespace Slang;

class CPUBufferResource : public BufferResource
{
public:
    CPUBufferResource(const Desc& _desc)
        : BufferResource(_desc)
    {}

    ~CPUBufferResource()
    {
        if (m_data)
        {
            free(m_data);
        }
    }

    SlangResult init()
    {
        m_data = malloc(m_desc.sizeInBytes);
        if(!m_data) return SLANG_E_OUT_OF_MEMORY;
        return SLANG_OK;
    }

    SlangResult setData(size_t offset, size_t size, void const* data)
    {
        memcpy((char*)m_data + offset, data, size);
        return SLANG_OK;
    }

    void* m_data = nullptr;
};

struct CPUTextureBaseShapeInfo
{
    int32_t rank;
    int32_t baseCoordCount;
    int32_t implicitArrayElementCount;
};

static const CPUTextureBaseShapeInfo kCPUTextureBaseShapeInfos[(int)ITextureResource::Type::CountOf] =
{
    /* Unknown */       { 0, 0, 0 },
    /* Buffer */        { 1, 1, 1 },
    /* Texture1D */     { 1, 1, 1 },
    /* Texture2D */     { 2, 2, 1 },
    /* Texture3D */     { 3, 3, 1 },
    /* TextureCube */   { 2, 3, 6 },
};

static CPUTextureBaseShapeInfo const* _getBaseShapeInfo(ITextureResource::Type baseShape)
{
    return &kCPUTextureBaseShapeInfos[(int)baseShape];
}

typedef void (*CPUTextureUnpackFunc)(void const* texelData, void* outData, size_t outSize);

struct CPUTextureFormatInfo
{
    CPUTextureUnpackFunc unpackFunc;
};

template<int N>
void _unpackFloatTexel(void const* texelData, void* outData, size_t outSize)
{
    auto input = (float const*) texelData;

    float temp[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for(int i = 0; i < N; ++i)
        temp[i] = input[i];

    memcpy(outData, temp, outSize);
}

static inline float _unpackUnorm8Value(uint8_t value)
{
    return value / 255.0f;
}

template<int N>
void _unpackUnorm8Texel(void const* texelData, void* outData, size_t outSize)
{
    auto input = (uint8_t const*) texelData;

    float temp[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for(int i = 0; i < N; ++i)
        temp[i] = _unpackUnorm8Value(input[i]);

    memcpy(outData, temp, outSize);
}

void _unpackUnormBGRA8Texel(void const* texelData, void* outData, size_t outSize)
{
    auto input = (uint8_t const*) texelData;

    float temp[4];
    temp[0] = _unpackUnorm8Value(input[2]);
    temp[1] = _unpackUnorm8Value(input[1]);
    temp[2] = _unpackUnorm8Value(input[0]);
    temp[3] = _unpackUnorm8Value(input[3]);

    memcpy(outData, temp, outSize);
}

template<int N>
void _unpackUInt16Texel(void const* texelData, void* outData, size_t outSize)
{
    auto input = (uint16_t const*) texelData;

    uint32_t temp[4] = { 0, 0, 0, 0 };
    for(int i = 0; i < N; ++i)
        temp[i] = input[i];

    memcpy(outData, temp, outSize);
}

template<int N>
void _unpackUInt32Texel(void const* texelData, void* outData, size_t outSize)
{
    auto input = (uint32_t const*) texelData;

    uint32_t temp[4] = { 0, 0, 0, 0 };
    for(int i = 0; i < N; ++i)
        temp[i] = input[i];

    memcpy(outData, temp, outSize);
}

#define TEXTURE_FORMAT_INFO(FORMAT) static const CPUTextureFormatInfo kCPUTextureFormatInfo_##FORMAT

TEXTURE_FORMAT_INFO(RGBA_Float32)      = { &_unpackFloatTexel<4> };
TEXTURE_FORMAT_INFO(RGB_Float32)       = { &_unpackFloatTexel<3> };
TEXTURE_FORMAT_INFO(RG_Float32)        = { &_unpackFloatTexel<2> };
TEXTURE_FORMAT_INFO(R_Float32)         = { &_unpackFloatTexel<1> };
TEXTURE_FORMAT_INFO(RGBA_Unorm_UInt8)  = { &_unpackUnorm8Texel<4> };
TEXTURE_FORMAT_INFO(BGRA_Unorm_UInt8)  = { &_unpackUnormBGRA8Texel };
TEXTURE_FORMAT_INFO(R_UInt16)          = { &_unpackUInt16Texel<1> };
TEXTURE_FORMAT_INFO(R_UInt32)          = { &_unpackUInt32Texel<1> };
TEXTURE_FORMAT_INFO(D_Float32)         = { &_unpackFloatTexel<1> };

#undef TEXTURE_FORMAT_INFO

static CPUTextureFormatInfo const* _getFormatInfo(Format format)
{
    switch(format)
    {
    case Format::D_Unorm24_S8:
    default:
        return nullptr;


#define CASE(FORMAT) case Format::FORMAT: return &kCPUTextureFormatInfo_##FORMAT;
    CASE(RGBA_Float32)
    CASE(RGB_Float32)
    CASE(RG_Float32)
    CASE(R_Float32)
    CASE(RGBA_Unorm_UInt8)
    CASE(BGRA_Unorm_UInt8)
    CASE(R_UInt16)
    CASE(R_UInt32)
    CASE(D_Float32)

#undef CASE
    }
}

class CPUTextureResource : public TextureResource
{
    enum { kMaxRank = 3 };

public:
    CPUTextureResource(const TextureResource::Desc& desc)
        : TextureResource(desc)
    {}
    ~CPUTextureResource()
    {
    }

    Result init(ITextureResource::SubresourceData const* initData)
    {
        auto desc = m_desc;

        // The format of the texture will determine the
        // size of the texels we allocate.
        //
        // TODO: Compressed formats usually work in terms
        // of a fixed block size, so that we cannot actually
        // compute a simple `texelSize` like this. Instead
        // we should be computing a `blockSize` and then
        // a `blockExtents` value that gives the extent
        // in texels of each block. For uncompressed formats
        // the block extents would be 1 along each axis.
        //
        auto format = desc.format;
        auto texelSize = gfxGetFormatSize(format);
        m_texelSize = (int32_t) texelSize;

        int32_t formatBlockSize[kMaxRank] = { 1, 1, 1 };

        auto baseShapeInfo = _getBaseShapeInfo(desc.type);
        m_baseShape = baseShapeInfo;
        if(!baseShapeInfo)
            return SLANG_FAIL;

        auto formatInfo = _getFormatInfo(desc.format);
        m_formatInfo = formatInfo;
        if(!formatInfo)
            return SLANG_FAIL;

        int32_t rank = baseShapeInfo->rank;
        int32_t effectiveArrayElementCount = desc.arraySize ? desc.arraySize : 1;
        effectiveArrayElementCount *= baseShapeInfo->implicitArrayElementCount;
        m_effectiveArrayElementCount = effectiveArrayElementCount;

        int32_t extents[kMaxRank];
        extents[0] = desc.size.width;
        extents[1] = desc.size.height;
        extents[2] = desc.size.depth;

        for(int32_t axis = rank; axis < kMaxRank; ++axis)
            extents[axis] = 1;

        int32_t levelCount = desc.numMipLevels;

        m_mipLevels.setCount(levelCount);

        int64_t totalDataSize = 0;
        for( int32_t levelIndex = 0; levelIndex < levelCount; ++levelIndex )
        {
            auto& level = m_mipLevels[levelIndex];

            for( int32_t axis = 0; axis < kMaxRank; ++axis )
            {
                int32_t extent = extents[axis] >> levelIndex;
                if(extent < 1) extent = 1;
                level.extents[axis] = extent;
            }

            level.strides[0] = texelSize;
            for( int32_t axis = 1; axis < kMaxRank+1; ++axis)
            {
                level.strides[axis] = level.strides[axis-1]*level.extents[axis-1];
            }

            int64_t levelDataSize = texelSize;
            levelDataSize *= effectiveArrayElementCount;
            for( int32_t axis = 0; axis < rank; ++axis)
                levelDataSize *= int64_t(level.extents[axis]);

            level.offset = totalDataSize;
            totalDataSize += levelDataSize;
        }

        void* textureData = malloc(totalDataSize);
        m_data = textureData;

        if( initData )
        {
            int32_t subResourceCounter = 0;
            for(int32_t arrayElementIndex = 0; arrayElementIndex < effectiveArrayElementCount; ++arrayElementIndex)
            {
                for(int32_t mipLevel = 0; mipLevel < m_desc.numMipLevels; ++mipLevel)
                {
                    int32_t subResourceIndex = subResourceCounter++;

                    auto dstRowStride = m_mipLevels[mipLevel].strides[1];
                    auto dstLayerStride = m_mipLevels[mipLevel].strides[2];
                    auto dstArrayStride = m_mipLevels[mipLevel].strides[3];

                    auto textureRowSize = m_mipLevels[mipLevel].extents[0]*texelSize;

                    auto rowCount = m_mipLevels[mipLevel].extents[1];
                    auto depthLayerCount = m_mipLevels[mipLevel].extents[2];

                    auto& srcImage = initData[subResourceIndex];
                    ptrdiff_t srcRowStride = ptrdiff_t(srcImage.strideY);
                    ptrdiff_t srcLayerStride = ptrdiff_t(srcImage.strideZ);

                    char* dstLevel = (char*)textureData + m_mipLevels[mipLevel].offset;
                    char* dstImage = dstLevel + dstArrayStride*arrayElementIndex;

                    const char* srcLayer = (const char*) srcImage.data;
                    char* dstLayer = dstImage;

                    for(int32_t depthLayer = 0; depthLayer < depthLayerCount; ++depthLayer)
                    {
                        const char* srcRow = srcLayer;
                        char* dstRow = dstLayer;

                        for(int32_t row = 0; row < rowCount; ++row)
                        {
                            memcpy(dstRow, srcRow, textureRowSize);

                            srcRow += srcRowStride;
                            dstRow += dstRowStride;
                        }

                        srcLayer += srcLayerStride;
                        dstLayer += dstLayerStride;
                    }
                }
            }
        }

        return SLANG_OK;
    }

    Desc const& _getDesc() { return m_desc; }
    Format getFormat() { return m_desc.format; }
    int32_t getRank() { return m_baseShape->rank; }

    CPUTextureBaseShapeInfo const* m_baseShape;
    CPUTextureFormatInfo const* m_formatInfo;
    int32_t m_effectiveArrayElementCount = 0;
    int32_t m_texelSize = 0;

    struct MipLevel
    {
        int32_t extents[kMaxRank];
        int64_t strides[kMaxRank+1];
        int64_t offset;
    };
    List<MipLevel>  m_mipLevels;
    void*           m_data = nullptr;
};

class CPUResourceView : public IResourceView, public RefObject
{
public:
    enum class Kind
    {
        Buffer,
        Texture,
    };

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IResourceView* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResourceView)
            return static_cast<IResourceView*>(this);
        return nullptr;
    }

    Kind getViewKind() const { return m_kind; }
    Desc const& getDesc() const { return m_desc; }

protected:
    CPUResourceView(Kind kind, Desc const& desc)
        : m_kind(kind)
        , m_desc(desc)
    {}

private:
    Kind m_kind;
    Desc m_desc;
};

class CPUBufferView : public CPUResourceView
{
public:
    CPUBufferView(Desc const& desc, CPUBufferResource* buffer)
        : CPUResourceView(Kind::Buffer, desc)
        , m_buffer(buffer)
    {}

    CPUBufferResource* getBuffer() const { return m_buffer; }

private:
    RefPtr<CPUBufferResource> m_buffer;
};

class CPUTextureView : public CPUResourceView, public slang_prelude::IRWTexture
{
public:
    CPUTextureView(Desc const& desc, CPUTextureResource* texture)
        : CPUResourceView(Kind::Texture, desc)
        , m_texture(texture)
    {}

    CPUTextureResource* getTexture() const { return m_texture; }

    //
    // ITexture interface
    //

    slang_prelude::TextureDimensions GetDimensions(int mipLevel = -1) SLANG_OVERRIDE
    {
        slang_prelude::TextureDimensions dimensions = {};

        CPUTextureResource* texture = m_texture;
        auto& desc = texture->_getDesc();
        auto baseShape = texture->m_baseShape;

        dimensions.arrayElementCount = desc.arraySize;
        dimensions.numberOfLevels = desc.numMipLevels;
        dimensions.shape = baseShape->rank;
        dimensions.width = desc.size.width;
        dimensions.height = desc.size.height;
        dimensions.depth = desc.size.depth;

        return dimensions;
    }

    void Load(const int32_t* texelCoords, void* outData, size_t dataSize) SLANG_OVERRIDE
    {
        void* texelPtr = _getTexelPtr(texelCoords);

        m_texture->m_formatInfo->unpackFunc(texelPtr, outData, dataSize);
    }

    void Sample(slang_prelude::SamplerState samplerState, const float* coords, void* outData, size_t dataSize) SLANG_OVERRIDE
    {
        // We have no access to information from fragment quads, so we cannot
        // compute the finite-difference derivatives needed from `coords`.
        //
        // The only reasonable thing to do is to sample mip level zero.
        //
        SampleLevel(samplerState, coords, 0.0f, outData, dataSize);
    }

    void SampleLevel(slang_prelude::SamplerState samplerState, const float* coords, float level, void* outData, size_t dataSize) SLANG_OVERRIDE
    {
        CPUTextureResource* texture = m_texture;
        auto baseShape = texture->m_baseShape;
        auto& desc = texture->_getDesc();
        int32_t rank = baseShape->rank;
        int32_t baseCoordCount = baseShape->baseCoordCount;

        int32_t integerMipLevel = int32_t(level + 0.5f);
        if(integerMipLevel >= desc.numMipLevels) integerMipLevel = desc.numMipLevels-1;
        if(integerMipLevel < 0) integerMipLevel = 0;

        auto& mipLevelInfo = texture->m_mipLevels[integerMipLevel];

        bool isArray = (desc.arraySize != 0) || (desc.type == ITextureResource::Type::TextureCube);
        int32_t effectiveArrayElementCount = texture->m_effectiveArrayElementCount;
        int32_t coordIndex = baseCoordCount;
        int32_t elementIndex = 0;
        if( isArray )
        {
            elementIndex = int32_t(coords[coordIndex++] + 0.5f);
        }
        if(elementIndex >= effectiveArrayElementCount) elementIndex = effectiveArrayElementCount-1;
        if(elementIndex < 0) elementIndex = 0;

        // Note: for now we are just going to do nearest-neighbor sampling
        //
        int64_t texelOffset = mipLevelInfo.offset;
        texelOffset += elementIndex * mipLevelInfo.strides[3];
        for(int32_t axis = 0; axis < rank; ++axis)
        {
            int32_t extent = mipLevelInfo.extents[axis];

            float coord = coords[axis];

            // TODO: deal with wrap/clamp/repeat if `coord < 0` or `coord > 1`

            int32_t integerCoord = int32_t(coord*(extent-1) + 0.5f);

            if(integerCoord >= extent) integerCoord = extent-1;
            if(integerCoord < 0) integerCoord = 0;

            texelOffset += integerCoord * mipLevelInfo.strides[axis];
        }

        auto texelPtr = (char const*)texture->m_data + texelOffset;

        m_texture->m_formatInfo->unpackFunc(texelPtr, outData, dataSize);
    }

    //
    // IRWTexture interface
    //

    void* refAt(const uint32_t* texelCoords) SLANG_OVERRIDE
    {
        return _getTexelPtr((int32_t const*)texelCoords);
    }

private:
    RefPtr<CPUTextureResource> m_texture;

    void* _getTexelPtr(int32_t const* texelCoords)
    {
        CPUTextureResource* texture = m_texture;
        auto baseShape = texture->m_baseShape;
        auto& desc = texture->_getDesc();

        int32_t rank = baseShape->rank;
        int32_t baseCoordCount = baseShape->baseCoordCount;

        bool isArray = (desc.arraySize != 0) || (desc.type == ITextureResource::Type::TextureCube);
        bool isMultisample = desc.sampleDesc.numSamples > 1;
        bool isBuffer = desc.type == ITextureResource::Type::Buffer;
        bool hasMipLevels = !(isMultisample || isBuffer);

        int32_t effectiveArrayElementCount = texture->m_effectiveArrayElementCount;

        int32_t coordIndex = baseCoordCount;
        int32_t elementIndex = 0;
        if( isArray )
        {
            elementIndex = texelCoords[coordIndex++];
        }
        if(elementIndex >= effectiveArrayElementCount) elementIndex = effectiveArrayElementCount-1;
        if(elementIndex < 0) elementIndex = 0;

        int32_t mipLevel = 0;
        if(!hasMipLevels)
        {
            mipLevel = texelCoords[coordIndex++];
        }
        if(mipLevel >= desc.numMipLevels) mipLevel = desc.numMipLevels-1;
        if(mipLevel < 0) mipLevel = 0;

        auto& mipLevelInfo = texture->m_mipLevels[mipLevel];

        int64_t texelOffset = mipLevelInfo.offset;
        texelOffset += elementIndex * mipLevelInfo.strides[3];
        for(int32_t axis = 0; axis < rank; ++axis)
        {
            int32_t coord = texelCoords[axis];
            if(coord >= mipLevelInfo.extents[axis]) coord = mipLevelInfo.extents[axis]-1;
            if(coord < 0) coord = 0;

            texelOffset += texelCoords[axis] * mipLevelInfo.strides[axis];
        }

        return (char*)texture->m_data + texelOffset;
    }
};

class CPUShaderObjectLayout : public ShaderObjectLayoutBase
{
public:

    // TODO: Once memory lifetime stuff is handled, there is
    // no specific need to even track binding or sub-object
    // ranges for CPU.

    struct BindingRangeInfo
    {
        slang::BindingType bindingType;
        Index count;
        Index baseIndex; // Flat index for sub-ojects

        // TODO: The `uniformOffset` field should be removed,
        // since it cannot be supported by the Slang reflection
        // API once we fix some design issues.
        //
        // It is only being used today for pre-allocation of sub-objects
        // for constant buffers and parameter blocks (which should be
        // deprecated/removed anyway).
        //
        // Note: We would need to bring this field back, plus
        // a lot of other complexity, if we ever want to support
        // setting of resources/buffers directly by a binding
        // range index and array index.
        //
        Index uniformOffset; // Uniform offset for a resource typed field.
    };

    struct SubObjectRangeInfo
    {
        RefPtr<CPUShaderObjectLayout> layout;
        Index bindingRangeIndex;
    };

    size_t m_size = 0;
    List<SubObjectRangeInfo> subObjectRanges;
    List<BindingRangeInfo> m_bindingRanges;

    Index m_subObjectCount = 0;
    Index m_resourceCount = 0;

    CPUShaderObjectLayout(RendererBase* renderer, slang::TypeLayoutReflection* layout)
    {
        initBase(renderer, layout);

        Index subObjectCount = 0;
        Index resourceCount = 0;

        m_elementTypeLayout = _unwrapParameterGroups(layout);
        m_size = m_elementTypeLayout->getSize();

        // Compute the binding ranges that are used to store
        // the logical contents of the object in memory. These will relate
        // to the descriptor ranges in the various sets, but not always
        // in a one-to-one fashion.

        SlangInt bindingRangeCount = m_elementTypeLayout->getBindingRangeCount();
        for (SlangInt r = 0; r < bindingRangeCount; ++r)
        {
            slang::BindingType slangBindingType = m_elementTypeLayout->getBindingRangeType(r);
            SlangInt count = m_elementTypeLayout->getBindingRangeBindingCount(r);
            slang::TypeLayoutReflection* slangLeafTypeLayout =
                m_elementTypeLayout->getBindingRangeLeafTypeLayout(r);

            SlangInt descriptorSetIndex = m_elementTypeLayout->getBindingRangeDescriptorSetIndex(r);
            SlangInt rangeIndexInDescriptorSet =
                m_elementTypeLayout->getBindingRangeFirstDescriptorRangeIndex(r);

            // TODO: This logic assumes that for any binding range that might consume
            // multiple kinds of resources, the descriptor range for its uniform
            // usage will be the first one in the range.
            //
            // We need to decide whether that assumption is one we intend to support
            // applications making, or whether they should be forced to perform a
            // linear search over the descriptor ranges for a specific binding range.
            //
            auto uniformOffset = m_elementTypeLayout->getDescriptorSetDescriptorRangeIndexOffset(
                descriptorSetIndex, rangeIndexInDescriptorSet);

            Index baseIndex = 0;
            switch (slangBindingType)
            {
            case slang::BindingType::ConstantBuffer:
            case slang::BindingType::ParameterBlock:
            case slang::BindingType::ExistentialValue:
                baseIndex = subObjectCount;
                subObjectCount += count;
                break;

            default:
                baseIndex = resourceCount;
                resourceCount += count;
                break;
            }

            BindingRangeInfo bindingRangeInfo;
            bindingRangeInfo.bindingType = slangBindingType;
            bindingRangeInfo.count = count;
            bindingRangeInfo.baseIndex = baseIndex;
            bindingRangeInfo.uniformOffset = uniformOffset;
            m_bindingRanges.add(bindingRangeInfo);
        }

        m_subObjectCount = subObjectCount;
        m_resourceCount = resourceCount;

        SlangInt subObjectRangeCount = m_elementTypeLayout->getSubObjectRangeCount();
        for (SlangInt r = 0; r < subObjectRangeCount; ++r)
        {
            SlangInt bindingRangeIndex = m_elementTypeLayout->getSubObjectRangeBindingRangeIndex(r);
            auto slangBindingType = m_elementTypeLayout->getBindingRangeType(bindingRangeIndex);
            slang::TypeLayoutReflection* slangLeafTypeLayout =
                m_elementTypeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

            // A sub-object range can either represent a sub-object of a known
            // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
            // (in which case we can pre-compute a layout to use, based on
            // the type `Foo`) *or* it can represent a sub-object of some
            // existential type (e.g., `IBar`) in which case we cannot
            // know the appropraite type/layout of sub-object to allocate.
            //
            RefPtr<CPUShaderObjectLayout> subObjectLayout;
            if (slangBindingType != slang::BindingType::ExistentialValue)
            {
                subObjectLayout =
                    new CPUShaderObjectLayout(renderer, slangLeafTypeLayout->getElementTypeLayout());
            }

            SubObjectRangeInfo subObjectRange;
            subObjectRange.bindingRangeIndex = bindingRangeIndex;
            subObjectRange.layout = subObjectLayout;
            subObjectRanges.add(subObjectRange);
        }
    }

    size_t getSize() { return m_size; }
    Index getResourceCount() const { return m_resourceCount; }
    Index getSubObjectCount() const { return m_subObjectCount; }
};

class CPUEntryPointLayout : public CPUShaderObjectLayout
{
private:
    slang::EntryPointLayout* m_entryPointLayout = nullptr;

public:
    CPUEntryPointLayout(
        RendererBase*               renderer,
        slang::EntryPointLayout*    entryPointLayout)
        : CPUShaderObjectLayout(renderer, entryPointLayout->getTypeLayout())
        , m_entryPointLayout(entryPointLayout)
    {}

    const char* getEntryPointName() { return m_entryPointLayout->getName(); }
};

class CPUProgramLayout : public CPUShaderObjectLayout
{
public:
    slang::ProgramLayout* m_programLayout = nullptr;
    List<RefPtr<CPUEntryPointLayout>> m_entryPointLayouts;

    CPUProgramLayout(RendererBase* renderer, slang::ProgramLayout* programLayout)
        : CPUShaderObjectLayout(renderer, programLayout->getGlobalParamsTypeLayout())
        , m_programLayout(programLayout)
    {
        for (UInt i =0; i< programLayout->getEntryPointCount(); i++)
        {
            m_entryPointLayouts.add(new CPUEntryPointLayout(
                renderer,
                programLayout->getEntryPointByIndex(i)));
        }

    }

    int getKernelIndex(UnownedStringSlice kernelName)
    {
        auto entryPointCount = (int) m_programLayout->getEntryPointCount();
        for(int i = 0; i < entryPointCount; i++)
        {
            auto entryPoint = m_programLayout->getEntryPointByIndex(i);
            if (kernelName == entryPoint->getName())
            {
                return i;
            }
        }
        return -1;
    }

    void getKernelThreadGroupSize(int kernelIndex, UInt* threadGroupSizes)
    {
        auto entryPoint = m_programLayout->getEntryPointByIndex(kernelIndex);
        entryPoint->getComputeThreadGroupSize(3, threadGroupSizes);
    }

    CPUEntryPointLayout* getEntryPoint(Index index) { return m_entryPointLayouts[index]; }
};

class CPUShaderObject : public ShaderObjectBase
{
public:
    void* m_data = nullptr;

    ~CPUShaderObject()
    {
        free(m_data);
    }

    List<RefPtr<CPUShaderObject>> m_objects;
    List<RefPtr<CPUResourceView>> m_resources;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        init(IDevice* device, CPUShaderObjectLayout* typeLayout);

    CPUShaderObjectLayout* getLayout()
    {
        return static_cast<CPUShaderObjectLayout*>(m_layout.Ptr());
    }

    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() override
    {
        return getLayout()->getElementTypeLayout();
    }

    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() override { return 0; }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** outEntryPoint) override
    {
        *outEntryPoint = nullptr;
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& offset, void const* data, size_t size) override
    {
        size = Math::Min(size, getLayout()->getSize() - offset.uniformOffset);
        memcpy((char*)m_data + offset.uniformOffset, data, size);
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL getObject(
        ShaderOffset const& offset,
        IShaderObject**     outObject) override
    {
        auto layout = getLayout();

        auto bindingRangeIndex = offset.bindingRangeIndex;
        SLANG_ASSERT(bindingRangeIndex >= 0);
        SLANG_ASSERT(bindingRangeIndex < layout->m_bindingRanges.getCount());

        auto& bindingRange = layout->m_bindingRanges[bindingRangeIndex];
        auto subObjectIndex = bindingRange.baseIndex + offset.bindingArrayIndex;
        CPUShaderObject* subObject = m_objects[subObjectIndex];

        *outObject = ComPtr<IShaderObject>(subObject).detach();

        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL setObject(
        ShaderOffset const& offset,
        IShaderObject*      object) override
    {
        auto layout = getLayout();

        auto bindingRangeIndex = offset.bindingRangeIndex;
        SLANG_ASSERT(bindingRangeIndex >= 0);
        SLANG_ASSERT(bindingRangeIndex < layout->m_bindingRanges.getCount());

        auto& bindingRange = layout->m_bindingRanges[bindingRangeIndex];
        auto subObjectIndex = bindingRange.baseIndex + offset.bindingArrayIndex;

        CPUShaderObject* subObject = static_cast<CPUShaderObject*>(object);
        m_objects[subObjectIndex] = subObject;

        switch( bindingRange.bindingType )
        {
        default:
            SLANG_RETURN_ON_FAIL(setData(offset, &subObject->m_data, sizeof(void*)));
            break;

        // If the range being assigned into represents an interface/existential-type leaf field,
        // then we need to consider how the `object` being assigned here affects specialization.
        // We may also need to assign some data from the sub-object into the ordinary data
        // buffer for the parent object.
        //
        case slang::BindingType::ExistentialValue:
            {
                auto renderer = getRenderer();

                ComPtr<slang::ISession> slangSession;
                SLANG_RETURN_ON_FAIL(renderer->getSlangSession(slangSession.writeRef()));

                // A leaf field of interface type is laid out inside of the parent object
                // as a tuple of `(RTTI, WitnessTable, Payload)`. The layout of these fields
                // is a contract between the compiler and any runtime system, so we will
                // need to rely on details of the binary layout.

                // We start by querying the layout/type of the concrete value that the application
                // is trying to store into the field, and also the layout/type of the leaf
                // existential-type field itself.
                //
                auto concreteTypeLayout = subObject->getElementTypeLayout();
                auto concreteType = concreteTypeLayout->getType();
                //
                auto existentialTypeLayout = layout->getElementTypeLayout()->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                auto existentialType = existentialTypeLayout->getType();

                // The first field of the tuple (offset zero) is the run-time type information (RTTI)
                // ID for the concrete type being stored into the field.
                //
                // TODO: We need to be able to gather the RTTI type ID from `object` and then
                // use `setData(offset, &TypeID, sizeof(TypeID))`.

                // The second field of the tuple (offset 8) is the ID of the "witness" for the
                // conformance of the concrete type to the interface used by this field.
                //
                auto witnessTableOffset = offset;
                witnessTableOffset.uniformOffset += 8;
                //
                // Conformances of a type to an interface are computed and then stored by the
                // Slang runtime, so we can look up the ID for this particular conformance (which
                // will create it on demand).
                //
                // Note: If the type doesn't actually conform to the required interface for
                // this sub-object range, then this is the point where we will detect that
                // fact and error out.
                //
                uint32_t conformanceID = 0xFFFFFFFF;
                SLANG_RETURN_ON_FAIL(slangSession->getTypeConformanceWitnessSequentialID(
                    concreteType, existentialType, &conformanceID));
                //
                // Once we have the conformance ID, then we can write it into the object
                // at the required offset.
                //
                SLANG_RETURN_ON_FAIL(setData(witnessTableOffset, &conformanceID, sizeof(conformanceID)));

                // The third field of the tuple (offset 16) is the "payload" that is supposed to
                // hold the data for a value of the given concrete type.
                //
                auto payloadOffset = offset;
                payloadOffset.uniformOffset += 16;

                // There are two cases we need to consider here for how the payload might be used:
                //
                // * If the concrete type of the value being bound is one that can "fit" into the
                //   available payload space,  then it should be stored in the payload.
                //
                // * If the concrete type of the value cannot fit in the payload space, then it
                //   will need to be stored somewhere else.
                //
                if(_doesValueFitInExistentialPayload(concreteTypeLayout, existentialTypeLayout))
                {
                    // If the value can fit in the payload area, then we will go ahead and copy
                    // its bytes into that area.
                    //
                    auto valueSize = concreteTypeLayout->getSize();
                    SLANG_RETURN_ON_FAIL(setData(payloadOffset, subObject->m_data, valueSize));
                }
                else
                {
                    // If the value cannot fit in the payload area, then we will pass a pointer
                    // to the sub-object instead.
                    //
                    // Note: The Slang compiler does not currently emit code that handles the
                    // pointer case, but that is the expected implementation for values
                    // that do not fit into the fixed-size payload.
                    //
                    SLANG_RETURN_ON_FAIL(setData(payloadOffset, &subObject->m_data, sizeof(void*)));
                }
            }
            break;
        }

        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* inView) override
    {
        auto layout = getLayout();

        auto bindingRangeIndex = offset.bindingRangeIndex;
        SLANG_ASSERT(bindingRangeIndex >= 0);
        SLANG_ASSERT(bindingRangeIndex < layout->m_bindingRanges.getCount());

        auto& bindingRange = layout->m_bindingRanges[bindingRangeIndex];
        auto viewIndex = bindingRange.baseIndex + offset.bindingArrayIndex;


        auto view = static_cast<CPUResourceView*>(inView);
        m_resources[viewIndex] = view;

        switch( view->getViewKind() )
        {
        case CPUResourceView::Kind::Texture:
            {
                auto textureView = static_cast<CPUTextureView*>(view);

                slang_prelude::IRWTexture* textureObj = textureView;
                SLANG_RETURN_ON_FAIL(setData(offset, &textureObj, sizeof(textureObj)));
            }
            break;

        case CPUResourceView::Kind::Buffer:
            {
                auto bufferView = static_cast<CPUBufferView*>(view);
                auto buffer = bufferView->getBuffer();
                auto desc = *buffer->getDesc();

                void* dataPtr = buffer->m_data;
                size_t size = desc.sizeInBytes;
                if (desc.elementSize > 1)
                    size /= desc.elementSize;

                auto ptrOffset = offset;
                SLANG_RETURN_ON_FAIL(setData(ptrOffset, &dataPtr, sizeof(dataPtr)));

                auto sizeOffset = offset;
                sizeOffset.uniformOffset += sizeof(dataPtr);
                SLANG_RETURN_ON_FAIL(setData(sizeOffset, &size, sizeof(size)));
            }
            break;
        }

        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setSampler(ShaderOffset const& offset, ISamplerState* sampler) override
    {
        SLANG_UNUSED(sampler);
        SLANG_UNUSED(offset);
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) override
    {
        SLANG_UNUSED(sampler);
        setResource(offset, textureView);
        return SLANG_OK;
    }

    // Appends all types that are used to specialize the element type of this shader object in `args` list.
    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
    {
        // TODO: the logic here is a copy-paste of `GraphicsCommonShaderObject::collectSpecializationArgs`,
        // consider moving the implementation to `ShaderObjectBase` and share the logic among different implementations.

        auto& subObjectRanges = getLayout()->subObjectRanges;
        // The following logic is built on the assumption that all fields that involve existential types (and
        // therefore require specialization) will results in a sub-object range in the type layout.
        // This allows us to simply scan the sub-object ranges to find out all specialization arguments.
        for (Index subObjIndex = 0; subObjIndex < subObjectRanges.getCount(); subObjIndex++)
        {
            // Retrieve the corresponding binding range of the sub object.
            auto bindingRange = getLayout()->m_bindingRanges[subObjectRanges[subObjIndex].bindingRangeIndex];
            switch (bindingRange.bindingType)
            {
            case slang::BindingType::ExistentialValue:
            {
                // A binding type of `ExistentialValue` means the sub-object represents a interface-typed field.
                // In this case the specialization argument for this field is the actual specialized type of the bound
                // shader object. If the shader object's type is an ordinary type without existential fields, then the
                // type argument will simply be the ordinary type. But if the sub object's type is itself a specialized
                // type, we need to make sure to use that type as the specialization argument.

                // TODO: need to implement the case where the field is an array of existential values.
                SLANG_ASSERT(bindingRange.count == 1);
                ExtendedShaderObjectType specializedSubObjType;
                SLANG_RETURN_ON_FAIL(m_objects[subObjIndex]->getSpecializedShaderObjectType(&specializedSubObjType));
                args.add(specializedSubObjType);
                break;
            }
            case slang::BindingType::ParameterBlock:
            case slang::BindingType::ConstantBuffer:
                // Currently we only handle the case where the field's type is
                // `ParameterBlock<SomeStruct>` or `ConstantBuffer<SomeStruct>`, where `SomeStruct` is a struct type
                // (not directly an interface type). In this case, we just recursively collect the specialization arguments
                // from the bound sub object.
                SLANG_RETURN_ON_FAIL(m_objects[subObjIndex]->collectSpecializationArgs(args));
                // TODO: we need to handle the case where the field is of the form `ParameterBlock<IFoo>`. We should treat
                // this case the same way as the `ExistentialValue` case here, but currently we lack a mechanism to distinguish
                // the two scenarios.
                break;
            }
            // TODO: need to handle another case where specialization happens on resources fields e.g. `StructuredBuffer<IFoo>`.
        }
        return SLANG_OK;
    }
};

class CPUEntryPointShaderObject : public CPUShaderObject
{
public:
    CPUEntryPointLayout* getLayout() { return static_cast<CPUEntryPointLayout*>(m_layout.Ptr()); }
};

class CPURootShaderObject : public CPUShaderObject
{
public:
    SlangResult init(IDevice* device, CPUProgramLayout* programLayout);

    CPUProgramLayout* getLayout() { return static_cast<CPUProgramLayout*>(m_layout.Ptr()); }

    CPUEntryPointShaderObject* getEntryPoint(Index index) { return m_entryPoints[index]; }

    List<RefPtr<CPUEntryPointShaderObject>> m_entryPoints;

    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() override { return m_entryPoints.getCount(); }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** outEntryPoint) override
    {
        *outEntryPoint = ComPtr<IShaderObject>(m_entryPoints[index]).detach();
        return SLANG_OK;
    }
    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
    {
        SLANG_RETURN_ON_FAIL(CPUShaderObject::collectSpecializationArgs(args));
        for (auto& entryPoint : m_entryPoints)
        {
            SLANG_RETURN_ON_FAIL(entryPoint->collectSpecializationArgs(args));
        }
        return SLANG_OK;
    }
};

class CPUShaderProgram : public ShaderProgramBase
{
public:
    RefPtr<CPUProgramLayout> layout;

    ~CPUShaderProgram()
    {
    }
};

class CPUPipelineState : public PipelineStateBase
{
public:
    CPUShaderProgram* getProgram() { return static_cast<CPUShaderProgram*>(m_program.get()); }

    void init(const ComputePipelineStateDesc& inDesc)
    {
        PipelineStateDesc pipelineDesc;
        pipelineDesc.type = PipelineType::Compute;
        pipelineDesc.compute = inDesc;
        initializeBase(pipelineDesc);
    }
};

class CPUDevice : public RendererBase
{
private:
    RefPtr<CPUPipelineState> m_currentPipeline = nullptr;
    RefPtr<CPURootShaderObject> m_currentRootObject = nullptr;
    DeviceInfo m_info;

    class CommandQueueImpl;

    class CommandBufferImpl
        : public ICommandBuffer
        , public CommandWriter
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ICommandBuffer* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
                return static_cast<ICommandBuffer*>(this);
            return nullptr;
        }
    public:
        virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
            IRenderPassLayout* renderPass,
            IFramebuffer* framebuffer,
            IRenderCommandEncoder** outEncoder) override
        {
            SLANG_UNUSED(renderPass);
            SLANG_UNUSED(framebuffer);
            *outEncoder = nullptr;
        }

        class ComputeCommandEncoderImpl
            : public IComputeCommandEncoder
        {
        public:
            virtual SLANG_NO_THROW SlangResult SLANG_MCALL
                queryInterface(SlangUUID const& uuid, void** outObject) override
            {
                if (uuid == GfxGUID::IID_ISlangUnknown ||
                    uuid == GfxGUID::IID_IComputeCommandEncoder)
                {
                    *outObject = static_cast<IComputeCommandEncoder*>(this);
                    return SLANG_OK;
                }
                *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }

        public:
            CommandWriter* m_writer;

            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override {}
            void init(CommandBufferImpl* cmdBuffer)
            {
                m_writer = cmdBuffer;
            }

            virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) override
            {
                m_writer->setPipelineState(state);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL
                bindRootShaderObject(IShaderObject* object) override
            {
                m_writer->bindRootShaderObject(PipelineType::Compute, object);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
                IPipelineLayout* layout,
                UInt index,
                IDescriptorSet* descriptorSet) override
            {
                m_writer->setDescriptorSet(PipelineType::Compute, layout, index, descriptorSet);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
            {
                m_writer->dispatchCompute(x, y, z);
            }
        };

        ComputeCommandEncoderImpl m_computeCommandEncoder;
        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeComputeCommands(IComputeCommandEncoder** outEncoder) override
        {
            m_computeCommandEncoder.init(this);
            *outEncoder = &m_computeCommandEncoder;
        }

        class ResourceCommandEncoderImpl
            : public IResourceCommandEncoder
        {
        public:
            virtual SLANG_NO_THROW SlangResult SLANG_MCALL
                queryInterface(SlangUUID const& uuid, void** outObject) override
            {
                if (uuid == GfxGUID::IID_ISlangUnknown ||
                    uuid == GfxGUID::IID_IResourceCommandEncoder)
                {
                    *outObject = static_cast<IResourceCommandEncoder*>(this);
                    return SLANG_OK;
                }
                *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }

        public:
            CommandWriter* m_writer;

            void init(CommandBufferImpl* cmdBuffer)
            {
                m_writer = cmdBuffer;
            }

            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override {}
            virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
                IBufferResource* dst,
                size_t dstOffset,
                IBufferResource* src,
                size_t srcOffset,
                size_t size) override
            {
                m_writer->copyBuffer(dst, dstOffset, src, srcOffset, size);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
            uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data) override
            {
                m_writer->uploadBufferData(dst, offset, size, data);
            }
        };

        ResourceCommandEncoderImpl m_resourceCommandEncoder;

        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeResourceCommands(IResourceCommandEncoder** outEncoder) override
        {
            m_resourceCommandEncoder.init(this);
            *outEncoder = &m_resourceCommandEncoder;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL close() override {}
    };

    class CommandQueueImpl
        : public ICommandQueue
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ICommandQueue* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandQueue)
                return static_cast<ICommandQueue*>(this);
            return nullptr;
        }

    public:
        RefPtr<CPUPipelineState> currentPipeline;
        RefPtr<CPURootShaderObject> currentRootObject;
        RefPtr<CPUDevice> renderer;
        Desc m_desc;
    public:
        void init(CPUDevice* inRenderer)
        {
            renderer = inRenderer;
            m_desc.type = ICommandQueue::QueueType::Graphics;
        }
        ~CommandQueueImpl()
        {
            currentPipeline = nullptr;
            currentRootObject = nullptr;
        }

    public:
        virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override
        {
            return m_desc;
        }
        virtual SLANG_NO_THROW Result SLANG_MCALL
            createCommandBuffer(ICommandBuffer** outCommandBuffer) override
        {
            RefPtr<CommandBufferImpl> result = new CommandBufferImpl();
            *outCommandBuffer = result.detach();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            executeCommandBuffers(uint32_t count, ICommandBuffer* const* commandBuffers) override
        {
            for (uint32_t i = 0; i < count; i++)
            {
                execute(static_cast<CommandBufferImpl*>(commandBuffers[i]));
            }
        }

        virtual SLANG_NO_THROW void SLANG_MCALL wait() override
        {}

    public:
        void setPipelineState(IPipelineState* state)
        {
            currentPipeline = static_cast<CPUPipelineState*>(state);
        }

        Result bindRootShaderObject(PipelineType pipelineType, IShaderObject* object)
        {
            currentRootObject = static_cast<CPURootShaderObject*>(object);
            if (currentRootObject)
                return SLANG_OK;
            return SLANG_E_INVALID_ARG;
        }

        void dispatchCompute(int x, int y, int z)
        {
            int entryPointIndex = 0;
            int targetIndex = 0;

            // Specialize the compute kernel based on the shader object bindings.
            RefPtr<PipelineStateBase> newPipeline;
            renderer->maybeSpecializePipeline(currentPipeline, currentRootObject, newPipeline);
            currentPipeline = static_cast<CPUPipelineState*>(newPipeline.Ptr());

            auto program = currentPipeline->getProgram();
            auto entryPointLayout = currentRootObject->getLayout()->getEntryPoint(entryPointIndex);
            auto entryPointName = entryPointLayout->getEntryPointName();

            auto entryPointObject = currentRootObject->getEntryPoint(entryPointIndex);

            ComPtr<ISlangSharedLibrary> sharedLibrary;
            program->slangProgram->getEntryPointHostCallable(entryPointIndex, targetIndex, sharedLibrary.writeRef());

            auto func = (slang_prelude::ComputeFunc) sharedLibrary->findSymbolAddressByName(entryPointName);

            slang_prelude::ComputeVaryingInput varyingInput;
            varyingInput.startGroupID.x = 0;
            varyingInput.startGroupID.y = 0;
            varyingInput.startGroupID.z = 0;
            varyingInput.endGroupID.x = x;
            varyingInput.endGroupID.y = y;
            varyingInput.endGroupID.z = z;

            auto globalParamsData = currentRootObject->m_data;
            auto entryPointParamsData = entryPointObject->m_data;
            func(&varyingInput, entryPointParamsData, globalParamsData);
        }

        void copyBuffer(
            IBufferResource* dst,
            size_t dstOffset,
            IBufferResource* src,
            size_t srcOffset,
            size_t size)
        {
            auto dstImpl = static_cast<CPUBufferResource*>(dst);
            auto srcImpl = static_cast<CPUBufferResource*>(src);
            memcpy(
                (uint8_t*)dstImpl->m_data + dstOffset,
                (uint8_t*)srcImpl->m_data + srcOffset,
                size);
        }

        void uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data)
        {
            auto dstImpl = static_cast<CPUBufferResource*>(dst);
            memcpy((uint8_t*)dstImpl->m_data + offset, data, size);
        }

        void execute(CommandBufferImpl* commandBuffer)
        {
            for (auto& cmd : commandBuffer->m_commands)
            {
                switch (cmd.name)
                {
                case CommandName::SetPipelineState:
                    setPipelineState(commandBuffer->getObject<IPipelineState>(cmd.operands[0]));
                    break;
                case CommandName::BindRootShaderObject:
                    bindRootShaderObject(
                        (PipelineType)cmd.operands[0],
                        commandBuffer->getObject<IShaderObject>(cmd.operands[1]));
                    break;
                case CommandName::DispatchCompute:
                    dispatchCompute(
                        int(cmd.operands[0]), int(cmd.operands[1]), int(cmd.operands[2]));
                    break;
                case CommandName::CopyBuffer:
                    copyBuffer(
                        commandBuffer->getObject<IBufferResource>(cmd.operands[0]),
                        cmd.operands[1],
                        commandBuffer->getObject<IBufferResource>(cmd.operands[2]),
                        cmd.operands[3],
                        cmd.operands[4]);
                    break;
                case CommandName::UploadBufferData:
                    uploadBufferData(
                        commandBuffer->getObject<IBufferResource>(cmd.operands[0]),
                        cmd.operands[1],
                        cmd.operands[2],
                        commandBuffer->getData<uint8_t>(cmd.operands[3]));
                    break;
                }
            }
        }
    };

public:
    ~CPUDevice()
    {
        m_currentPipeline = nullptr;
        m_currentRootObject = nullptr;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override
    {
        SLANG_RETURN_ON_FAIL(slangContext.initialize(desc.slang, SLANG_HOST_CALLABLE, "sm_5_1"));

        SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));

        // Initialize DeviceInfo
        {
            m_info.deviceType = DeviceType::CPU;
            m_info.bindingStyle = BindingStyle::CUDA;
            m_info.projectionStyle = ProjectionStyle::DirectX;
            m_info.apiName = "CPU";
            static const float kIdentity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
            m_info.adapterName = "CPU";
        }

        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override
    {
        RefPtr<CPUTextureResource> texture = new CPUTextureResource(desc);

        SLANG_RETURN_ON_FAIL(texture->init(initData));

        *outResource = texture.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        IResource::Usage initialUsage,
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) override
    {
        RefPtr<CPUBufferResource> resource = new CPUBufferResource(desc);
        SLANG_RETURN_ON_FAIL(resource->init());
        if (initData)
        {
            SLANG_RETURN_ON_FAIL(resource->setData(0, desc.sizeInBytes, initData));
        }
        *outResource = resource.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* inTexture, IResourceView::Desc const& desc, IResourceView** outView) override
    {
        auto texture = static_cast<CPUTextureResource*>(inTexture);
        RefPtr<CPUTextureView> view = new CPUTextureView(desc, texture);
        *outView = view.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* inBuffer, IResourceView::Desc const& desc, IResourceView** outView) override
    {
        auto buffer = static_cast<CPUBufferResource*>(inBuffer);
        RefPtr<CPUBufferView> view = new CPUBufferView(desc, buffer);
        *outView = view.detach();
        return SLANG_OK;
    }

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection*    typeLayout,
        ShaderObjectLayoutBase**        outLayout) override
    {
        RefPtr<CPUShaderObjectLayout> cpuLayout = new CPUShaderObjectLayout(this, typeLayout);
        *outLayout = cpuLayout.detach();

        return SLANG_OK;
    }

    virtual Result createShaderObject(
        ShaderObjectLayoutBase* layout,
        IShaderObject**         outObject) override
    {
        auto cpuLayout = static_cast<CPUShaderObjectLayout*>(layout);

        RefPtr<CPUShaderObject> result = new CPUShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, cpuLayout));
        *outObject = result.detach();

        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override
    {
        auto cpuProgram = static_cast<CPUShaderProgram*>(program);
        auto cpuProgramLayout = cpuProgram->layout;

        RefPtr<CPURootShaderObject> result = new CPURootShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, cpuProgramLayout));
        *outObject = result.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override
    {
        if( desc.kernelCount == 0 )
        {
            return createProgramFromSlang(this, desc, outProgram);
        }

        if (desc.kernelCount != 1)
            return SLANG_E_INVALID_ARG;

        RefPtr<CPUShaderProgram> cpuProgram = new CPUShaderProgram();

        // TODO: stuff?

        auto slangProgram = desc.slangProgram;
        if( slangProgram )
        {
            cpuProgram->slangProgram = slangProgram;

            auto slangProgramLayout = slangProgram->getLayout();
            if(!slangProgramLayout)
                return SLANG_FAIL;

            RefPtr<CPUProgramLayout> cpuProgramLayout = new CPUProgramLayout(this, slangProgramLayout);
            cpuProgramLayout->m_programLayout = slangProgramLayout;

            cpuProgram->layout = cpuProgramLayout;
        }

        *outProgram = cpuProgram.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override
    {
        RefPtr<CPUPipelineState> state = new CPUPipelineState();
        state->init(desc);
        *outState = state.detach();
        return Result();
    }

    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override
    {
        return m_info;
    }

public:
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override
    {
        RefPtr<CommandQueueImpl> queue = new CommandQueueImpl();
        queue->init(this);
        *outQueue = queue.detach();
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(window);
        SLANG_UNUSED(outSwapchain);
        return SLANG_FAIL;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebufferLayout(
        const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outLayout);
        return SLANG_FAIL;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outFramebuffer);
        return SLANG_FAIL;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc,
        IRenderPassLayout** outRenderPassLayout) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outRenderPassLayout);
        return SLANG_FAIL;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) override
    {
        SLANG_UNUSED(desc);
        *outSampler = nullptr;
        return SLANG_OK;
    }
    
    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override
    {
        SLANG_UNUSED(inputElements);
        SLANG_UNUSED(inputElementCount);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createDescriptorSet(IDescriptorSetLayout* layout, IDescriptorSet::Flag::Enum flags, IDescriptorSet** outDescriptorSet) override
    {
        SLANG_UNUSED(layout);
        SLANG_UNUSED(flags);
        SLANG_UNUSED(outDescriptorSet);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outState);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* texture,
        ResourceState state,
        ISlangBlob** outBlob,
        size_t* outRowPitch,
        size_t* outPixelSize) override
    {
        SLANG_UNUSED(texture);
        SLANG_UNUSED(outBlob);
        SLANG_UNUSED(outRowPitch);
        SLANG_UNUSED(outPixelSize);

        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL readBufferResource(
        IBufferResource* buffer,
        size_t offset,
        size_t size,
        ISlangBlob** outBlob) override
    {
        auto bufferImpl = static_cast<CPUBufferResource*>(buffer);
        RefPtr<ListBlob> blob = new ListBlob();
        blob->m_data.setCount((Index)size);
        memcpy(
            blob->m_data.getBuffer(),
            (uint8_t*)bufferImpl->m_data + offset,
            size);
        *outBlob = blob.detach();
        return SLANG_OK;
    }
};

SlangResult CPUShaderObject::init(IDevice* device, CPUShaderObjectLayout* typeLayout)
{
    m_layout = typeLayout;

    // If the layout tells us that there is any uniform data,
    // then we need to allocate a constant buffer to hold that data.
    //
    // TODO: Do we need to allocate a shadow copy for use from
    // the CPU?
    //
    // TODO: When/where do we bind this constant buffer into
    // a descriptor set for later use?
    //
    auto slangLayout = getLayout()->getElementTypeLayout();
    size_t uniformSize = slangLayout->getSize();
    if (uniformSize)
    {
        m_data = malloc(uniformSize);
    }

    // If the layout specifies that we have any resources or sub-objects,
    // then we need to size the appropriate arrays to account for them.
    //
    // Note: the counts here are the *total* number of resources/sub-objects
    // and not just the number of resource/sub-object ranges.
    //
    m_resources.setCount(typeLayout->getResourceCount());
    m_objects.setCount(typeLayout->getSubObjectCount());

    for (auto subObjectRange : getLayout()->subObjectRanges)
    {
        RefPtr<CPUShaderObjectLayout> subObjectLayout = subObjectRange.layout;

        // In the case where the sub-object range represents an
        // existential-type leaf field (e.g., an `IBar`), we
        // cannot pre-allocate the object(s) to go into that
        // range, since we can't possibly know what to allocate
        // at this point.
        //
        if (!subObjectLayout)
            continue;
        //
        // Otherwise, we will allocate a sub-object to fill
        // in each entry in this range, based on the layout
        // information we already have.

        auto& bindingRangeInfo = getLayout()->m_bindingRanges[subObjectRange.bindingRangeIndex];
        for (Index i = 0; i < bindingRangeInfo.count; ++i)
        {
            RefPtr<CPUShaderObject> subObject = new CPUShaderObject();
            SLANG_RETURN_ON_FAIL(subObject->init(device, subObjectLayout));

            ShaderOffset offset;
            offset.uniformOffset = bindingRangeInfo.uniformOffset + sizeof(void*) * i;
            offset.bindingRangeIndex = subObjectRange.bindingRangeIndex;
            offset.bindingArrayIndex = i;

            SLANG_RETURN_ON_FAIL(setObject(offset, subObject));
        }
    }
    return SLANG_OK;
}

SlangResult CPURootShaderObject::init(IDevice* device, CPUProgramLayout* programLayout)
{
    SLANG_RETURN_ON_FAIL(CPUShaderObject::init(device, programLayout));
    for (auto& entryPoint : programLayout->m_entryPointLayouts)
    {
        RefPtr<CPUEntryPointShaderObject> object = new CPUEntryPointShaderObject();
        SLANG_RETURN_ON_FAIL(object->init(device, entryPoint));
        m_entryPoints.add(object);
    }
    return SLANG_OK;
}

SlangResult SLANG_MCALL createCPUDevice(const IDevice::Desc* desc, IDevice** outDevice)
{
    RefPtr<CPUDevice> result = new CPUDevice();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    *outDevice = result.detach();
    return SLANG_OK;
}

}
