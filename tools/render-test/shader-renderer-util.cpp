// shader-renderer-util.cpp

#include "shader-renderer-util.h"

namespace renderer_test
{

using namespace Slang;
using Slang::Result;

inline int calcMipSize(int size, int level)
{
    size = size >> level;
    return size > 0 ? size : 1;
}

inline Extents calcMipSize(Extents size, int mipLevel)
{
    Extents rs;
    rs.width = calcMipSize(size.width, mipLevel);
    rs.height = calcMipSize(size.height, mipLevel);
    rs.depth = calcMipSize(size.depth, mipLevel);
    return rs;
}

/// Given the type works out the maximum dimension size
inline int calcMaxDimension(Extents size, TextureType type)
{
    switch (type)
    {
    case TextureType::Texture1D:
        return size.width;
    case TextureType::Texture3D:
        return Math::Max(Math::Max(size.width, size.height), size.depth);
    case TextureType::TextureCube: // fallthru
    case TextureType::Texture2D:
        {
            return Math::Max(size.width, size.height);
        }
    default:
        return 0;
    }
}

/// Given the type, calculates the number of mip maps. 0 on error
inline int calcNumMipLevels(TextureType type, Extents size)
{
    const int maxDimensionSize = calcMaxDimension(size, type);
    return (maxDimensionSize > 0) ? (Math::Log2Floor(maxDimensionSize) + 1) : 0;
}

/* static */ Result ShaderRendererUtil::generateTexture(
    const InputTextureDesc& inputDesc,
    ResourceState defaultState,
    IDevice* device,
    ComPtr<ITexture>& textureOut)
{
    TextureData texData;
    generateTextureData(texData, inputDesc);
    return createTexture(inputDesc, texData, defaultState, device, textureOut);
}

/* static */ Result ShaderRendererUtil::createTexture(
    const InputTextureDesc& inputDesc,
    const TextureData& texData,
    ResourceState defaultState,
    IDevice* device,
    ComPtr<ITexture>& textureOut)
{
    TextureDesc textureDesc = {};

    // Default to R8G8B8A8_UNORM
    const Format format =
        (inputDesc.format == Format::Unknown) ? Format::R8G8B8A8_UNORM : inputDesc.format;

    textureDesc.sampleCount = inputDesc.sampleCount;
    textureDesc.format = format;
    textureDesc.mipLevelCount = texData.m_mipLevels;
    textureDesc.arrayLength = inputDesc.arrayLength > 0 ? inputDesc.arrayLength : 1;
    textureDesc.usage = TextureUsage::CopyDestination | TextureUsage::CopySource;
    switch (defaultState)
    {
    case ResourceState::ShaderResource:
        textureDesc.usage |= TextureUsage::ShaderResource;
        break;
    case ResourceState::UnorderedAccess:
        textureDesc.usage |= TextureUsage::UnorderedAccess;
        break;
    default:
        return SLANG_FAIL;
    }
    textureDesc.defaultState = defaultState;

    // It's the same size in all dimensions
    switch (inputDesc.dimension)
    {
    case 1:
        {
            textureDesc.type = TextureType::Texture1D;
            textureDesc.size.width = inputDesc.size;
            textureDesc.size.height = 1;
            textureDesc.size.depth = 1;

            break;
        }
    case 2:
        {
            textureDesc.type = inputDesc.isCube ? TextureType::TextureCube : TextureType::Texture2D;
            textureDesc.size.width = inputDesc.size;
            textureDesc.size.height = inputDesc.size;
            textureDesc.size.depth = 1;
            break;
        }
    case 3:
        {
            textureDesc.type = TextureType::Texture3D;
            textureDesc.size.width = inputDesc.size;
            textureDesc.size.height = inputDesc.size;
            textureDesc.size.depth = inputDesc.size;
            break;
        }
    }

    if (textureDesc.mipLevelCount == 0)
    {
        textureDesc.mipLevelCount = calcNumMipLevels(textureDesc.type, textureDesc.size);
    }

    List<SubresourceData> initSubresources;
    int arrayLayerCount =
        textureDesc.arrayLength * (textureDesc.type == TextureType::TextureCube ? 6 : 1);
    int subResourceCounter = 0;
    for (int a = 0; a < arrayLayerCount; ++a)
    {
        for (int m = 0; m < textureDesc.mipLevelCount; ++m)
        {
            int subResourceIndex = subResourceCounter++;
            const int mipWidth = calcMipSize(textureDesc.size.width, m);
            const int mipHeight = calcMipSize(textureDesc.size.height, m);

            auto strideY = mipWidth * sizeof(uint32_t);
            auto strideZ = mipHeight * strideY;

            SubresourceData subresourceData;
            subresourceData.data = texData.m_slices[subResourceIndex].values;
            subresourceData.strideY = strideY;
            subresourceData.strideZ = strideZ;

            initSubresources.add(subresourceData);
        }
    }

    textureOut = device->createTexture(textureDesc, initSubresources.getBuffer());

    return textureOut ? SLANG_OK : SLANG_FAIL;
}

/* static */ Result ShaderRendererUtil::createBuffer(
    const InputBufferDesc& inputDesc,
    size_t bufferSize,
    const void* initData,
    IDevice* device,
    Slang::ComPtr<IBuffer>& bufferOut)
{
    BufferDesc bufferDesc;
    bufferDesc.size = bufferSize;
    bufferDesc.format = inputDesc.format;
    bufferDesc.elementSize = inputDesc.stride;
    bufferDesc.usage = BufferUsage::CopyDestination | BufferUsage::CopySource |
                       BufferUsage::ShaderResource | BufferUsage::UnorderedAccess;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;

    ComPtr<IBuffer> bufferResource = device->createBuffer(bufferDesc, initData);
    if (!bufferResource)
    {
        return SLANG_FAIL;
    }

    bufferOut = bufferResource;
    return SLANG_OK;
}

static SamplerDesc _calcSamplerDesc(const InputSamplerDesc& srcDesc)
{
    SamplerDesc samplerDesc;
    if (srcDesc.isCompareSampler)
    {
        samplerDesc.reductionOp = TextureReductionOp::Comparison;
        samplerDesc.comparisonFunc = ComparisonFunc::Less;
    }
    return samplerDesc;
}

ComPtr<ISampler> _createSampler(IDevice* device, const InputSamplerDesc& srcDesc)
{
    return device->createSampler(_calcSamplerDesc(srcDesc));
}

} // namespace renderer_test
