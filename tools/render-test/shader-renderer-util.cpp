// shader-renderer-util.cpp

#include "shader-renderer-util.h"

namespace renderer_test {

using namespace Slang;

/* static */Result ShaderRendererUtil::generateTextureResource(const InputTextureDesc& inputDesc, int bindFlags, Renderer* renderer, RefPtr<TextureResource>& textureOut)
{
    TextureData texData;
    generateTextureData(texData, inputDesc);
    return createTextureResource(inputDesc, texData, bindFlags, renderer, textureOut);
}

/* static */Result ShaderRendererUtil::createTextureResource(const InputTextureDesc& inputDesc, const TextureData& texData, int bindFlags, Renderer* renderer, RefPtr<TextureResource>& textureOut)
{
    TextureResource::Desc textureResourceDesc;
    textureResourceDesc.init();

    textureResourceDesc.format = Format::RGBA_Unorm_UInt8;
    textureResourceDesc.numMipLevels = texData.mipLevels;
    textureResourceDesc.arraySize = inputDesc.arrayLength;
    textureResourceDesc.bindFlags = bindFlags;

    // It's the same size in all dimensions 
    Resource::Type type = Resource::Type::Unknown;
    switch (inputDesc.dimension)
    {
        case 1:
        {
            type = Resource::Type::Texture1D;
            textureResourceDesc.size.init(inputDesc.size);
            break;
        }
        case 2:
        {
            type = inputDesc.isCube ? Resource::Type::TextureCube : Resource::Type::Texture2D;
            textureResourceDesc.size.init(inputDesc.size, inputDesc.size);
            break;
        }
        case 3:
        {
            type = Resource::Type::Texture3D;
            textureResourceDesc.size.init(inputDesc.size, inputDesc.size, inputDesc.size);
            break;
        }
    }

    const int effectiveArraySize = textureResourceDesc.calcEffectiveArraySize(type);
    const int numSubResources = textureResourceDesc.calcNumSubResources(type);

    Resource::Usage initialUsage = Resource::Usage::GenericRead;
    TextureResource::Data initData;

    List<ptrdiff_t> mipRowStrides;
    mipRowStrides.SetSize(textureResourceDesc.numMipLevels);
    List<const void*> subResources;
    subResources.SetSize(numSubResources);

    // Set up the src row strides
    for (int i = 0; i < textureResourceDesc.numMipLevels; i++)
    {
        const int mipWidth = TextureResource::calcMipSize(textureResourceDesc.size.width, i);
        mipRowStrides[i] = mipWidth * sizeof(uint32_t);
    }

    // Set up pointers the the data
    {
        int subResourceIndex = 0;
        const int numGen = int(texData.dataBuffer.Count());
        for (int i = 0; i < numSubResources; i++)
        {
            subResources[i] = texData.dataBuffer[subResourceIndex].Buffer();
            // Wrap around
            subResourceIndex = (subResourceIndex + 1 >= numGen) ? 0 : (subResourceIndex + 1);
        }
    }

    initData.mipRowStrides = mipRowStrides.Buffer();
    initData.numMips = textureResourceDesc.numMipLevels;
    initData.numSubResources = numSubResources;
    initData.subResources = subResources.Buffer();

    textureOut = renderer->createTextureResource(type, Resource::Usage::GenericRead, textureResourceDesc, &initData);

    return textureOut ? SLANG_OK : SLANG_FAIL;
}

/* static */Result ShaderRendererUtil::createBufferResource(const InputBufferDesc& inputDesc, bool isOutput, size_t bufferSize, const void* initData, Renderer* renderer, Slang::RefPtr<BufferResource>& bufferOut)
{
    Resource::Usage initialUsage = Resource::Usage::GenericRead;

    BufferResource::Desc srcDesc;
    srcDesc.init(bufferSize);

    int bindFlags = 0;
    if (inputDesc.type == InputBufferType::ConstantBuffer)
    {
        bindFlags |= Resource::BindFlag::ConstantBuffer;
        srcDesc.cpuAccessFlags |= Resource::AccessFlag::Write;
        initialUsage = Resource::Usage::ConstantBuffer;
    }
    else
    {
        bindFlags |= Resource::BindFlag::UnorderedAccess | Resource::BindFlag::PixelShaderResource | Resource::BindFlag::NonPixelShaderResource;
        srcDesc.elementSize = inputDesc.stride;
        initialUsage = Resource::Usage::UnorderedAccess;
    }

    if (isOutput)
    {
        srcDesc.cpuAccessFlags |= Resource::AccessFlag::Read;
    }

    srcDesc.bindFlags = bindFlags;

    RefPtr<BufferResource> bufferResource = renderer->createBufferResource(initialUsage, srcDesc, initData);
    if (!bufferResource)
    {
        return SLANG_FAIL;
    }

    bufferOut = bufferResource;
    return SLANG_OK;
}
    
static BindingState::SamplerDesc _calcSamplerDesc(const InputSamplerDesc& srcDesc)
{
    BindingState::SamplerDesc dstDesc;
    dstDesc.isCompareSampler = srcDesc.isCompareSampler;
    return dstDesc;
}

/* static */Result ShaderRendererUtil::createBindingStateDesc(ShaderInputLayoutEntry* srcEntries, int numEntries, Renderer* renderer, BindingState::Desc& descOut)
{
    const int textureBindFlags = Resource::BindFlag::NonPixelShaderResource | Resource::BindFlag::PixelShaderResource;

    descOut.clear();
    for (int i = 0; i < numEntries; i++)
    {
        const ShaderInputLayoutEntry& srcEntry = srcEntries[i];

        BindingState::ShaderBindSet shaderBindSet;
        shaderBindSet.set(BindingState::ShaderStyle::Hlsl, descOut.makeCompactSlice(srcEntry.hlslBinding));
        shaderBindSet.set(BindingState::ShaderStyle::Glsl, descOut.makeCompactSlice(srcEntry.glslBinding.Buffer(), int(srcEntry.glslBinding.Count())));

        switch (srcEntry.type)
        {
            case ShaderInputType::Buffer:
            {
                const InputBufferDesc& srcBuffer = srcEntry.bufferDesc;

                const size_t bufferSize = srcEntry.bufferData.Count() * sizeof(uint32_t);

                RefPtr<BufferResource> bufferResource;
                SLANG_RETURN_ON_FAIL(createBufferResource(srcEntry.bufferDesc, srcEntry.isOutput, bufferSize, srcEntry.bufferData.Buffer(), renderer, bufferResource));
                
                descOut.addBufferResource(bufferResource, shaderBindSet);
                break;
            }
            case ShaderInputType::CombinedTextureSampler:
            {
                RefPtr<TextureResource> texture;
                SLANG_RETURN_ON_FAIL(generateTextureResource(srcEntry.textureDesc, textureBindFlags, renderer, texture));
                descOut.addCombinedTextureSampler(texture, _calcSamplerDesc(srcEntry.samplerDesc), shaderBindSet);
                break;
            }
            case ShaderInputType::Texture:
            {
                RefPtr<TextureResource> texture;
                SLANG_RETURN_ON_FAIL(generateTextureResource(srcEntry.textureDesc, textureBindFlags, renderer, texture));

                descOut.addTextureResource(texture, shaderBindSet);
                break;
            }
            case ShaderInputType::Sampler:
            {
                descOut.addSampler(_calcSamplerDesc(srcEntry.samplerDesc), shaderBindSet);
                break;
            }
            default: 
            {
                assert(!"Unhandled type");
                return SLANG_FAIL;
            }
        }
    }    

    return SLANG_OK;
}

/* static */Result ShaderRendererUtil::createBindingStateDesc(const ShaderInputLayout& layout, Renderer* renderer, BindingState::Desc& descOut)
{
    SLANG_RETURN_ON_FAIL(createBindingStateDesc(layout.entries.Buffer(), int(layout.entries.Count()), renderer, descOut));
    descOut.m_numRenderTargets = layout.numRenderTargets;
    return SLANG_OK;
}

} // renderer_test
