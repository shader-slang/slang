// shader-renderer-util.cpp

#include "shader-renderer-util.h"

namespace renderer_test {

using namespace Slang;
using Slang::Result;

inline int calcMipSize(int size, int level)
{
    size = size >> level;
    return size > 0 ? size : 1;
}

inline ITextureResource::Extents calcMipSize(ITextureResource::Extents size, int mipLevel)
{
    ITextureResource::Extents rs;
    rs.width = calcMipSize(size.width, mipLevel);
    rs.height = calcMipSize(size.height, mipLevel);
    rs.depth = calcMipSize(size.depth, mipLevel);
    return rs;
}

/// Calculate the effective array size - in essence the amount if mip map sets needed.
/// In practice takes into account if the arraySize is 0 (it's not an array, but it will still have
/// at least one mip set) and if the type is a cubemap (multiplies the amount of mip sets by 6)
inline int calcEffectiveArraySize(const ITextureResource::Desc& desc)
{
    const int arrSize = (desc.arraySize > 0) ? desc.arraySize : 1;

    switch (desc.type)
    {
    case IResource::Type::Texture1D: // fallthru
    case IResource::Type::Texture2D:
        {
            return arrSize;
        }
    case IResource::Type::TextureCube:
        return arrSize * 6;
    case IResource::Type::Texture3D:
        return 1;
    default:
        return 0;
    }
}

/// Given the type works out the maximum dimension size
inline int calcMaxDimension(ITextureResource::Extents size, IResource::Type type)
{
    switch (type)
    {
    case IResource::Type::Texture1D:
        return size.width;
    case IResource::Type::Texture3D:
        return Math::Max(Math::Max(size.width, size.height), size.depth);
    case IResource::Type::TextureCube: // fallthru
    case IResource::Type::Texture2D:
        {
            return Math::Max(size.width, size.height);
        }
    default:
        return 0;
    }
}

/// Given the type, calculates the number of mip maps. 0 on error
inline int calcNumMipLevels(IResource::Type type, ITextureResource::Extents size)
{
    const int maxDimensionSize = calcMaxDimension(size, type);
    return (maxDimensionSize > 0) ? (Math::Log2Floor(maxDimensionSize) + 1) : 0;
}

/// Calculate the total number of sub resources. 0 on error.
inline int calcNumSubResources(const ITextureResource::Desc& desc)
{
    const int numMipMaps =
        (desc.numMipLevels > 0) ? desc.numMipLevels : calcNumMipLevels(desc.type, desc.size);
    const int arrSize = (desc.arraySize > 0) ? desc.arraySize : 1;

    switch (desc.type)
    {
    case IResource::Type::Texture1D:
    case IResource::Type::Texture2D:
    case IResource::Type::Texture3D:
        {
            return numMipMaps * arrSize;
        }
    case IResource::Type::TextureCube:
        {
            // There are 6 faces to a cubemap
            return numMipMaps * arrSize * 6;
        }
    default:
        return 0;
    }
}

/* static */ Result ShaderRendererUtil::generateTextureResource(
    const InputTextureDesc& inputDesc,
    ResourceState defaultState,
    IDevice* device,
    ComPtr<ITextureResource>& textureOut)
{
    TextureData texData;
    generateTextureData(texData, inputDesc);
    return createTextureResource(inputDesc, texData, defaultState, device, textureOut);
}

/* static */ Result ShaderRendererUtil::createTextureResource(
    const InputTextureDesc& inputDesc,
    const TextureData& texData,
    ResourceState defaultState,
    IDevice* device,
    ComPtr<ITextureResource>& textureOut)
{
    ITextureResource::Desc textureResourceDesc = {};

    // Default to R8G8B8A8_UNORM
    const Format format = (inputDesc.format == Format::Unknown) ? Format::R8G8B8A8_UNORM : inputDesc.format;

    textureResourceDesc.sampleDesc = ITextureResource::SampleDesc{inputDesc.sampleCount, 0};
    textureResourceDesc.format = format;
    textureResourceDesc.numMipLevels = texData.m_mipLevels;
    textureResourceDesc.arraySize = inputDesc.arrayLength;
    textureResourceDesc.allowedStates =
        ResourceStateSet(defaultState, ResourceState::CopyDestination, ResourceState::CopySource);
    textureResourceDesc.defaultState = defaultState;

    // It's the same size in all dimensions
    switch (inputDesc.dimension)
    {
        case 1:
        {
            textureResourceDesc.type = IResource::Type::Texture1D;
            textureResourceDesc.size.width = inputDesc.size;
            textureResourceDesc.size.height = 1;
            textureResourceDesc.size.depth = 1;

            break;
        }
        case 2:
        {
            textureResourceDesc.type = inputDesc.isCube ? IResource::Type::TextureCube : IResource::Type::Texture2D;
            textureResourceDesc.size.width = inputDesc.size;
            textureResourceDesc.size.height = inputDesc.size;
            textureResourceDesc.size.depth = 1;
            break;
        }
        case 3:
        {
            textureResourceDesc.type = IResource::Type::Texture3D;
            textureResourceDesc.size.width = inputDesc.size;
            textureResourceDesc.size.height = inputDesc.size;
            textureResourceDesc.size.depth = inputDesc.size;
            break;
        }
    }

    const int effectiveArraySize = calcEffectiveArraySize(textureResourceDesc);
    const int numSubResources = calcNumSubResources(textureResourceDesc);

    List<ITextureResource::SubresourceData> initSubresources;
    int subResourceCounter = 0;
    for( int a = 0; a < effectiveArraySize; ++a )
    {
        for( int m = 0; m < textureResourceDesc.numMipLevels; ++m )
        {
            int subResourceIndex = subResourceCounter++;
            const int mipWidth = calcMipSize(textureResourceDesc.size.width, m);
            const int mipHeight = calcMipSize(textureResourceDesc.size.height, m);

            auto strideY = mipWidth * sizeof(uint32_t);
            auto strideZ = mipHeight * strideY;

            ITextureResource::SubresourceData subresourceData;
            subresourceData.data = texData.m_slices[subResourceIndex].values;
            subresourceData.strideY = strideY;
            subresourceData.strideZ = strideZ;

            initSubresources.add(subresourceData);
        }
    }

    textureOut = device->createTextureResource(textureResourceDesc, initSubresources.getBuffer());

    return textureOut ? SLANG_OK : SLANG_FAIL;
}

/* static */ Result ShaderRendererUtil::createBufferResource(
    const InputBufferDesc& inputDesc,
    size_t bufferSize,
    const void* initData,
    IDevice* device,
    Slang::ComPtr<IBufferResource>& bufferOut)
{
    IBufferResource::Desc srcDesc;
    srcDesc.type = IResource::Type::Buffer;
    srcDesc.sizeInBytes = bufferSize;
    srcDesc.format = inputDesc.format;
    srcDesc.elementSize = inputDesc.stride;
    srcDesc.defaultState = ResourceState::UnorderedAccess;
    srcDesc.allowedStates = ResourceStateSet(
        ResourceState::CopyDestination,
        ResourceState::CopySource,
        ResourceState::UnorderedAccess,
        ResourceState::ShaderResource);

    ComPtr<IBufferResource> bufferResource = device->createBufferResource(srcDesc, initData);
    if (!bufferResource)
    {
        return SLANG_FAIL;
    }

    bufferOut = bufferResource;
    return SLANG_OK;
}

static ISamplerState::Desc _calcSamplerDesc(const InputSamplerDesc& srcDesc)
{
    ISamplerState::Desc dstDesc;
    if (srcDesc.isCompareSampler)
    {
        dstDesc.reductionOp = TextureReductionOp::Comparison;
        dstDesc.comparisonFunc = ComparisonFunc::Less;
    }
    return dstDesc;
}

ComPtr<ISamplerState> _createSamplerState(IDevice* device,
    const InputSamplerDesc& srcDesc)
{
    return device->createSamplerState(_calcSamplerDesc(srcDesc));
}

} // renderer_test
