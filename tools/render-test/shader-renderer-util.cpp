// shader-renderer-util.cpp

#include "shader-renderer-util.h"

#include "tools/gfx/resource-desc-utils.h"

namespace renderer_test {

using namespace Slang;
using Slang::Result;

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
