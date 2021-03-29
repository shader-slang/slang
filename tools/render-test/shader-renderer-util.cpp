// shader-renderer-util.cpp

#include "shader-renderer-util.h"

namespace renderer_test {

using namespace Slang;
using Slang::Result;

/* static */ Result ShaderRendererUtil::generateTextureResource(
    const InputTextureDesc& inputDesc,
    int bindFlags,
    IDevice* device,
    ComPtr<ITextureResource>& textureOut)
{
    TextureData texData;
    generateTextureData(texData, inputDesc);
    return createTextureResource(inputDesc, texData, bindFlags, device, textureOut);
}

/* static */ Result ShaderRendererUtil::createTextureResource(
    const InputTextureDesc& inputDesc,
    const TextureData& texData,
    int bindFlags,
    IDevice* device,
    ComPtr<ITextureResource>& textureOut)
{
    ITextureResource::Desc textureResourceDesc;
    textureResourceDesc.init(IResource::Type::Unknown);

    // Default to RGBA_Unorm_UInt8
    const Format format = (inputDesc.format == Format::Unknown) ? Format::RGBA_Unorm_UInt8 : inputDesc.format;

    textureResourceDesc.format = format;
    textureResourceDesc.numMipLevels = texData.mipLevels;
    textureResourceDesc.arraySize = inputDesc.arrayLength;
    textureResourceDesc.bindFlags = bindFlags;

    // It's the same size in all dimensions
    switch (inputDesc.dimension)
    {
        case 1:
        {
            textureResourceDesc.type = IResource::Type::Texture1D;
            textureResourceDesc.size.init(inputDesc.size);
            break;
        }
        case 2:
        {
            textureResourceDesc.type = inputDesc.isCube ? IResource::Type::TextureCube : IResource::Type::Texture2D;
            textureResourceDesc.size.init(inputDesc.size, inputDesc.size);
            break;
        }
        case 3:
        {
            textureResourceDesc.type = IResource::Type::Texture3D;
            textureResourceDesc.size.init(inputDesc.size, inputDesc.size, inputDesc.size);
            break;
        }
    }

    const int effectiveArraySize = textureResourceDesc.calcEffectiveArraySize();
    const int numSubResources = textureResourceDesc.calcNumSubResources();

    IResource::Usage initialUsage = IResource::Usage::GenericRead;

    List<ITextureResource::SubresourceData> initSubresources;
    int subResourceCounter = 0;
    for( int a = 0; a < effectiveArraySize; ++a )
    {
        for( int m = 0; m < textureResourceDesc.numMipLevels; ++m )
        {
            int subResourceIndex = subResourceCounter++;
            const int mipWidth = ITextureResource::Size::calcMipSize(textureResourceDesc.size.width, m);
            const int mipHeight = ITextureResource::Size::calcMipSize(textureResourceDesc.size.width, m);

            auto strideY = mipWidth * sizeof(uint32_t);
            auto strideZ = mipHeight * strideY;

            ITextureResource::SubresourceData subresourceData;
            subresourceData.data = texData.dataBuffer[subResourceIndex].getBuffer();
            subresourceData.strideY = strideY;
            subresourceData.strideZ = strideZ;

            initSubresources.add(subresourceData);
        }
    }

    textureOut = device->createTextureResource(IResource::Usage::GenericRead, textureResourceDesc, initSubresources.getBuffer());

    return textureOut ? SLANG_OK : SLANG_FAIL;
}

/* static */ Result ShaderRendererUtil::createBufferResource(
    const InputBufferDesc& inputDesc,
    size_t bufferSize,
    const void* initData,
    IDevice* device,
    Slang::ComPtr<IBufferResource>& bufferOut)
{
    IResource::Usage initialUsage = IResource::Usage::GenericRead;

    IBufferResource::Desc srcDesc;
    srcDesc.init(bufferSize);
    srcDesc.format = inputDesc.format;

    int bindFlags = 0;
    {
        bindFlags |= IResource::BindFlag::UnorderedAccess | IResource::BindFlag::PixelShaderResource | IResource::BindFlag::NonPixelShaderResource;
        srcDesc.elementSize = inputDesc.stride;
        initialUsage = IResource::Usage::UnorderedAccess;
    }

    srcDesc.bindFlags = bindFlags;

    ComPtr<IBufferResource> bufferResource =
        device->createBufferResource(initialUsage, srcDesc, initData);
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
