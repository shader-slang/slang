// render-cpu.cpp
#include "render-cpu.h"

#include <chrono>

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"
#include "core/slang-basic.h"
#include "core/slang-blob.h"

#include "../immediate-renderer-base.h"
#include "../slang-context.h"
#include "../mutable-shader-object.h"
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
        if (!m_data)
            return SLANG_E_OUT_OF_MEMORY;
        return SLANG_OK;
    }

    SlangResult setData(size_t offset, size_t size, void const* data)
    {
        memcpy((char*)m_data + offset, data, size);
        return SLANG_OK;
    }

    void* m_data = nullptr;

    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override
    {
        return (DeviceAddress)m_data;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        map(MemoryRange* rangeToRead, void** outPointer) override
    {
        SLANG_UNUSED(rangeToRead);
        if (outPointer)
            *outPointer = m_data;
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL unmap(MemoryRange* writtenRange) override
    {
        SLANG_UNUSED(writtenRange);
        return SLANG_OK;
    }
};

struct CPUTextureBaseShapeInfo
{
    int32_t rank;
    int32_t baseCoordCount;
    int32_t implicitArrayElementCount;
};

static const CPUTextureBaseShapeInfo kCPUTextureBaseShapeInfos[(int)ITextureResource::Type::_Count] =
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

template<int N>
void _unpackFloat16Texel(void const* texelData, void* outData, size_t outSize)
{
    auto input = (int16_t const*)texelData;

    float temp[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for (int i = 0; i < N; ++i)
        temp[i] = HalfToFloat(input[i]);

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

struct CPUFormatInfoMap
{
    CPUFormatInfoMap()
    {
        memset(m_infos, 0, sizeof(m_infos));

        set(Format::R32G32B32A32_FLOAT, &_unpackFloatTexel<4>);
        set(Format::R32G32B32_FLOAT, &_unpackFloatTexel<3>);

        set(Format::R32G32_FLOAT, &_unpackFloatTexel<2>);
        set(Format::R32_FLOAT, &_unpackFloatTexel<1>);

        set(Format::R16G16B16A16_FLOAT, &_unpackFloat16Texel<4>);
        set(Format::R16G16_FLOAT, &_unpackFloat16Texel<2>);
        set(Format::R16_FLOAT, &_unpackFloat16Texel<1>);

        set(Format::R8G8B8A8_UNORM, &_unpackUnorm8Texel<4>);
        set(Format::B8G8R8A8_UNORM, &_unpackUnormBGRA8Texel);
        set(Format::R16_UINT, &_unpackUInt16Texel<1>);
        set(Format::R32_UINT, &_unpackUInt32Texel<1>);
        set(Format::D32_FLOAT, &_unpackFloatTexel<1>);
    }

    void set(Format format, CPUTextureUnpackFunc func)
    {
        auto& info = m_infos[Index(format)];
        info.unpackFunc = func;
    }
    SLANG_FORCE_INLINE const CPUTextureFormatInfo& get(Format format) const { return m_infos[Index(format)]; }

    CPUTextureFormatInfo m_infos[Index(Format::_Count)];
};

static const CPUFormatInfoMap g_formatInfoMap;

static CPUTextureFormatInfo const* _getFormatInfo(Format format)
{
    const CPUTextureFormatInfo& info = g_formatInfoMap.get(format);
    return info.unpackFunc ? &info : nullptr;
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
        free(m_data);
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
        FormatInfo texelInfo;
        gfxGetFormatInfo(format, &texelInfo);
        uint32_t texelSize = uint32_t(texelInfo.blockSizeInBytes / texelInfo.pixelsPerBlock);
        m_texelSize = texelSize;

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

        void* textureData = malloc((size_t)totalDataSize);
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
    uint32_t m_texelSize = 0;

    struct MipLevel
    {
        int32_t extents[kMaxRank];
        int64_t strides[kMaxRank+1];
        int64_t offset;
    };
    List<MipLevel>  m_mipLevels;
    void*           m_data = nullptr;
};

class CPUResourceView : public ResourceViewBase
{
public:
    enum class Kind
    {
        Buffer,
        Texture,
    };
    Kind getViewKind() const { return m_kind; }
    Desc const& getDesc() const { return m_desc; }

protected:
    CPUResourceView(Kind kind, Desc const& desc)
        : m_kind(kind)
    {
        m_desc = desc;
    }

private:
    Kind m_kind;
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
        Index subObjectIndex;

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

        m_subObjectCount = 0;
        m_resourceCount = 0;

        m_elementTypeLayout = _unwrapParameterGroups(layout, m_containerType);
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
            Index subObjectIndex = 0;
            switch (slangBindingType)
            {
            case slang::BindingType::ConstantBuffer:
            case slang::BindingType::ParameterBlock:
            case slang::BindingType::ExistentialValue:
                baseIndex = m_subObjectCount;
                subObjectIndex = baseIndex;
                m_subObjectCount += count;
                break;
            case slang::BindingType::RawBuffer:
            case slang::BindingType::MutableRawBuffer:
                if (slangLeafTypeLayout->getType()->getElementType() != nullptr)
                {
                    // A structured buffer occupies both a resource slot and
                    // a sub-object slot.
                    subObjectIndex = m_subObjectCount;
                    m_subObjectCount += count;
                }
                baseIndex = m_resourceCount;
                m_resourceCount += count;
                break;
            default:
                baseIndex = m_resourceCount;
                m_resourceCount += count;
                break;
            }

            BindingRangeInfo bindingRangeInfo;
            bindingRangeInfo.bindingType = slangBindingType;
            bindingRangeInfo.count = count;
            bindingRangeInfo.baseIndex = baseIndex;
            bindingRangeInfo.uniformOffset = uniformOffset;
            bindingRangeInfo.subObjectIndex = subObjectIndex;
            m_bindingRanges.add(bindingRangeInfo);
        }

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
    List<SubObjectRangeInfo>& getSubObjectRanges() { return subObjectRanges; }
    BindingRangeInfo getBindingRange(Index index) { return m_bindingRanges[index]; }
    Index getBindingRangeCount() const { return m_bindingRanges.getCount(); }
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

class CPUShaderObjectData
{
public:
    Slang::List<char> m_ordinaryData;
    // Any "ordinary" / uniform data for this object
    Slang::RefPtr<CPUBufferResource> m_bufferResource;
    Slang::RefPtr<CPUBufferView> m_bufferView;

    Index getCount() { return m_ordinaryData.getCount(); }
    void setCount(Index count) { m_ordinaryData.setCount(count); }
    char* getBuffer() { return m_ordinaryData.getBuffer(); }

    ~CPUShaderObjectData()
    {
        // m_bufferResource's data is managed by m_ordinaryData so we
        // set it to null to prevent m_bufferResource from freeing it.
        if (m_bufferResource)
            m_bufferResource->m_data = nullptr;
    }

    /// Returns a StructuredBuffer resource view for GPU access into the buffer content.
    /// Creates a StructuredBuffer resource if it has not been created.
    ResourceViewBase* getResourceView(
        RendererBase* device,
        slang::TypeLayoutReflection* elementLayout,
        slang::BindingType bindingType)
    {
        SLANG_UNUSED(device);
        if (!m_bufferResource)
        {
            IBufferResource::Desc desc = {};
            desc.type = IResource::Type::Buffer;
            desc.elementSize = (int)elementLayout->getSize();
            m_bufferResource = new CPUBufferResource(desc);

            IResourceView::Desc viewDesc = {};
            viewDesc.type = IResourceView::Type::UnorderedAccess;
            viewDesc.format = Format::Unknown;
            m_bufferView = new CPUBufferView(viewDesc, m_bufferResource);
            
        }
        m_bufferResource->getDesc()->sizeInBytes = m_ordinaryData.getCount();
        m_bufferResource->m_data = m_ordinaryData.getBuffer();
        return m_bufferView.Ptr();
    }
};

class CPUShaderObject
    : public ShaderObjectBaseImpl<CPUShaderObject, CPUShaderObjectLayout, CPUShaderObjectData>
{
    typedef ShaderObjectBaseImpl<CPUShaderObject, CPUShaderObjectLayout, CPUShaderObjectData> Super;

public:
    List<RefPtr<CPUResourceView>> m_resources;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        init(IDevice* device, CPUShaderObjectLayout* typeLayout);

    virtual SLANG_NO_THROW GfxCount SLANG_MCALL getEntryPointCount() override { return 0; }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(GfxIndex index, IShaderObject** outEntryPoint) override
    {
        *outEntryPoint = nullptr;
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW const void* SLANG_MCALL getRawData() override
    {
        return m_data.getBuffer();
    }

    virtual SLANG_NO_THROW size_t SLANG_MCALL getSize() override
    {
        return (size_t)m_data.getCount();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& offset, void const* data, size_t size) override
    {
        size = Math::Min(size, size_t(m_data.getCount() - offset.uniformOffset));
        memcpy((char*)m_data.getBuffer() + offset.uniformOffset, data, size);
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
        setObject(ShaderOffset const& offset, IShaderObject* object) override
    {
        SLANG_RETURN_ON_FAIL(Super::setObject(offset, object));

        auto bindingRangeIndex = offset.bindingRangeIndex;
        auto& bindingRange = getLayout()->m_bindingRanges[bindingRangeIndex];

        CPUShaderObject* subObject = static_cast<CPUShaderObject*>(object);

        switch (bindingRange.bindingType)
        {
        default:
            {
                void* bufferPtr = subObject->m_data.getBuffer();
                SLANG_RETURN_ON_FAIL(setData(offset, &bufferPtr, sizeof(void*)));
            }
            break;
        case slang::BindingType::ExistentialValue:
        case slang::BindingType::RawBuffer:
        case slang::BindingType::MutableRawBuffer:
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

    char* getDataBuffer() { return m_data.getBuffer(); }
};

class CPUMutableShaderObject : public MutableShaderObject<CPUMutableShaderObject, CPUShaderObjectLayout>
{};

class CPUEntryPointShaderObject : public CPUShaderObject
{
public:
    CPUEntryPointLayout* getLayout() { return static_cast<CPUEntryPointLayout*>(m_layout.Ptr()); }
};

class CPURootShaderObject : public CPUShaderObject
{
public:
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }

    SlangResult init(IDevice* device, CPUProgramLayout* programLayout);

    CPUProgramLayout* getLayout() { return static_cast<CPUProgramLayout*>(m_layout.Ptr()); }

    CPUEntryPointShaderObject* getEntryPoint(Index index) { return m_entryPoints[index]; }

    List<RefPtr<CPUEntryPointShaderObject>> m_entryPoints;

    virtual SLANG_NO_THROW GfxCount SLANG_MCALL getEntryPointCount() override { return (GfxCount)m_entryPoints.getCount(); }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(GfxIndex index, IShaderObject** outEntryPoint) override
    {
        returnComPtr(outEntryPoint, m_entryPoints[index]);
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
    CPUShaderProgram* getProgram() { return static_cast<CPUShaderProgram*>(m_program.Ptr()); }

    void init(const ComputePipelineStateDesc& inDesc)
    {
        PipelineStateDesc pipelineDesc;
        pipelineDesc.type = PipelineType::Compute;
        pipelineDesc.compute = inDesc;
        initializeBase(pipelineDesc);
    }
};

class CPUQueryPool : public QueryPoolBase
{
public:
    List<uint64_t> m_queries;
    Result init(const IQueryPool::Desc& desc)
    {
        m_queries.setCount(desc.count);
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL getResult(
        GfxIndex queryIndex, GfxCount count, uint64_t* data) override
    {
        for (GfxCount i = 0; i < count; i++)
        {
            data[i] = m_queries[queryIndex + i];
        }
        return SLANG_OK;
    }
};

class CPUDevice : public ImmediateComputeDeviceBase
{
private:
    RefPtr<CPUPipelineState> m_currentPipeline = nullptr;
    RefPtr<CPURootShaderObject> m_currentRootObject = nullptr;
    DeviceInfo m_info;

    virtual void setPipelineState(IPipelineState* state) override
    {
        m_currentPipeline = static_cast<CPUPipelineState*>(state);
    }

    virtual void bindRootShaderObject(IShaderObject* object) override
    {
        m_currentRootObject = static_cast<CPURootShaderObject*>(object);
    }

    virtual void dispatchCompute(int x, int y, int z) override
    {
        int entryPointIndex = 0;
        int targetIndex = 0;

        // Specialize the compute kernel based on the shader object bindings.
        RefPtr<PipelineStateBase> newPipeline;
        maybeSpecializePipeline(m_currentPipeline, m_currentRootObject, newPipeline);
        m_currentPipeline = static_cast<CPUPipelineState*>(newPipeline.Ptr());

        auto program = m_currentPipeline->getProgram();
        auto entryPointLayout =
            m_currentRootObject->getLayout()->getEntryPoint(entryPointIndex);
        auto entryPointName = entryPointLayout->getEntryPointName();

        auto entryPointObject = m_currentRootObject->getEntryPoint(entryPointIndex);

        ComPtr<ISlangSharedLibrary> sharedLibrary;
        ComPtr<ISlangBlob> diagnostics;
        auto compileResult = program->slangGlobalScope->getEntryPointHostCallable(
            entryPointIndex, targetIndex, sharedLibrary.writeRef(), diagnostics.writeRef());
        if (diagnostics)
        {
            getDebugCallback()->handleMessage(
                compileResult == SLANG_OK ? DebugMessageType::Warning : DebugMessageType::Error,
                DebugMessageSource::Slang,
                (char*)diagnostics->getBufferPointer());
        }
        if (SLANG_FAILED(compileResult)) return;

        auto func = (slang_prelude::ComputeFunc) sharedLibrary->findSymbolAddressByName(entryPointName);

        slang_prelude::ComputeVaryingInput varyingInput;
        varyingInput.startGroupID.x = 0;
        varyingInput.startGroupID.y = 0;
        varyingInput.startGroupID.z = 0;
        varyingInput.endGroupID.x = x;
        varyingInput.endGroupID.y = y;
        varyingInput.endGroupID.z = z;

        auto globalParamsData = m_currentRootObject->getDataBuffer();
        auto entryPointParamsData = entryPointObject->getDataBuffer();
        func(&varyingInput, entryPointParamsData, globalParamsData);
    }

    virtual void copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) override
    {
        auto dstImpl = static_cast<CPUBufferResource*>(dst);
        auto srcImpl = static_cast<CPUBufferResource*>(src);
        memcpy(
            (uint8_t*)dstImpl->m_data + dstOffset,
            (uint8_t*)srcImpl->m_data + srcOffset,
            size);
    }

public:
    ~CPUDevice()
    {
        m_currentPipeline = nullptr;
        m_currentRootObject = nullptr;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override
    {
        SLANG_RETURN_ON_FAIL(slangContext.initialize(
            desc.slang,
            SLANG_SHADER_HOST_CALLABLE,
            "sm_5_1",
            makeArray(slang::PreprocessorMacroDesc{ "__CPU__", "1" }).getView()));

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
            m_info.timestampFrequency = 1000000000;
        }

        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override
    {
        TextureResource::Desc srcDesc = fixupTextureDesc(desc);

        RefPtr<CPUTextureResource> texture = new CPUTextureResource(srcDesc);

        SLANG_RETURN_ON_FAIL(texture->init(initData));

        returnComPtr(outResource, texture);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        const IBufferResource::Desc& descIn,
        const void* initData,
        IBufferResource** outResource) override
    {
        auto desc = fixupBufferDesc(descIn);
        RefPtr<CPUBufferResource> resource = new CPUBufferResource(desc);
        SLANG_RETURN_ON_FAIL(resource->init());
        if (initData)
        {
            SLANG_RETURN_ON_FAIL(resource->setData(0, desc.sizeInBytes, initData));
        }
        returnComPtr(outResource, resource);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* inTexture, IResourceView::Desc const& desc, IResourceView** outView) override
    {
        auto texture = static_cast<CPUTextureResource*>(inTexture);
        RefPtr<CPUTextureView> view = new CPUTextureView(desc, texture);
        returnComPtr(outView, view);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* inBuffer,
        IBufferResource* counterBuffer,
        IResourceView::Desc const& desc,
        IResourceView** outView) override
    {
        auto buffer = static_cast<CPUBufferResource*>(inBuffer);
        RefPtr<CPUBufferView> view = new CPUBufferView(desc, buffer);
        returnComPtr(outView, view);
        return SLANG_OK;
    }

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection*    typeLayout,
        ShaderObjectLayoutBase**        outLayout) override
    {
        RefPtr<CPUShaderObjectLayout> cpuLayout = new CPUShaderObjectLayout(this, typeLayout);
        returnRefPtrMove(outLayout, cpuLayout);

        return SLANG_OK;
    }

    virtual Result createShaderObject(
        ShaderObjectLayoutBase* layout,
        IShaderObject**         outObject) override
    {
        auto cpuLayout = static_cast<CPUShaderObjectLayout*>(layout);

        RefPtr<CPUShaderObject> result = new CPUShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, cpuLayout));
        returnComPtr(outObject, result);

        return SLANG_OK;
    }

    virtual Result createMutableShaderObject(
        ShaderObjectLayoutBase* layout,
        IShaderObject** outObject) override
    {
        auto cpuLayout = static_cast<CPUShaderObjectLayout*>(layout);

        RefPtr<CPUMutableShaderObject> result = new CPUMutableShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, cpuLayout));
        returnComPtr(outObject, result);

        return SLANG_OK;
    }

    virtual Result createRootShaderObject(IShaderProgram* program, ShaderObjectBase** outObject) override
    {
        auto cpuProgram = static_cast<CPUShaderProgram*>(program);
        auto cpuProgramLayout = cpuProgram->layout;

        RefPtr<CPURootShaderObject> result = new CPURootShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, cpuProgramLayout));
        returnRefPtrMove(outObject, result);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createProgram(
        const IShaderProgram::Desc& desc,
        IShaderProgram** outProgram,
        ISlangBlob** outDiagnosticBlob) override
    {
        RefPtr<CPUShaderProgram> cpuProgram = new CPUShaderProgram();
        cpuProgram->init(desc);
        auto slangGlobalScope = cpuProgram->linkedProgram;
        if( slangGlobalScope )
        {
            auto slangProgramLayout = slangGlobalScope->getLayout();
            if(!slangProgramLayout)
                return SLANG_FAIL;

            RefPtr<CPUProgramLayout> cpuProgramLayout = new CPUProgramLayout(this, slangProgramLayout);
            cpuProgramLayout->m_programLayout = slangProgramLayout;

            cpuProgram->layout = cpuProgramLayout;
        }

        returnComPtr(outProgram, cpuProgram);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override
    {
        RefPtr<CPUPipelineState> state = new CPUPipelineState();
        state->init(desc);
        returnComPtr(outState, state);
        return Result();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createQueryPool(
        const IQueryPool::Desc& desc, IQueryPool** outPool) override
    {
        RefPtr<CPUQueryPool> pool = new CPUQueryPool();
        pool->init(desc);
        returnComPtr(outPool, pool);
        return SLANG_OK;
    }

    virtual void writeTimestamp(IQueryPool* pool, GfxIndex index) override
    {
        static_cast<CPUQueryPool*>(pool)->m_queries[index] =
            std::chrono::high_resolution_clock::now().time_since_epoch().count();
    }

    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override
    {
        return m_info;
    }

public:
    
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) override
    {
        SLANG_UNUSED(desc);
        *outSampler = nullptr;
        return SLANG_OK;
    }

    virtual void submitGpuWork() override {}
    virtual void waitForGpu() override {}
    virtual void* map(IBufferResource* buffer, MapFlavor flavor) override
    {
        SLANG_UNUSED(flavor);
        auto bufferImpl = static_cast<CPUBufferResource*>(buffer);
        return bufferImpl->m_data;
    }
    virtual void unmap(IBufferResource* buffer, size_t offsetWritten, size_t sizeWritten) override
    {
        SLANG_UNUSED(buffer);
        SLANG_UNUSED(offsetWritten);
        SLANG_UNUSED(sizeWritten);
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
    m_data.setCount(uniformSize);

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
        auto _debugname = subObjectLayout->getElementTypeLayout()->getName();
        
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
            offset.bindingRangeIndex = (GfxIndex)subObjectRange.bindingRangeIndex;
            offset.bindingArrayIndex = (GfxIndex)i;

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
    returnComPtr(outDevice, result);
    return SLANG_OK;
}

}
