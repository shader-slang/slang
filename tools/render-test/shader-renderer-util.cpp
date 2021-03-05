// shader-renderer-util.cpp

#include "shader-renderer-util.h"

namespace renderer_test {

using namespace Slang;
using Slang::Result;

void BindingStateImpl::apply(ICommandEncoder* encoder, PipelineType pipelineType)
{
    switch (pipelineType)
    {
    case PipelineType::Compute:
        {
            ComPtr<IComputeCommandEncoder> computeEncoder;
            encoder->queryInterface(SLANG_UUID_IComputeCommandEncoder, (void**)computeEncoder.writeRef());
            computeEncoder->setDescriptorSet(pipelineLayout, 0, descriptorSet);
        }
        break;
    case PipelineType::Graphics:
        {
            ComPtr<IRenderCommandEncoder> renderEncoder;
            encoder->queryInterface(
                SLANG_UUID_IRenderCommandEncoder, (void**)renderEncoder.writeRef());
            renderEncoder->setDescriptorSet(pipelineLayout, 0, descriptorSet);
        }
        break;
    default:
        throw "unknown pipeline type";
    }
}

/* static */ Result ShaderRendererUtil::generateTextureResource(
    const InputTextureDesc& inputDesc,
    int bindFlags,
    IRenderer* renderer,
    ComPtr<ITextureResource>& textureOut)
{
    TextureData texData;
    generateTextureData(texData, inputDesc);
    return createTextureResource(inputDesc, texData, bindFlags, renderer, textureOut);
}

/* static */ Result ShaderRendererUtil::createTextureResource(
    const InputTextureDesc& inputDesc,
    const TextureData& texData,
    int bindFlags,
    IRenderer* renderer,
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
    ITextureResource::Data initData;

    List<ptrdiff_t> mipRowStrides;
    mipRowStrides.setCount(textureResourceDesc.numMipLevels);
    List<const void*> subResources;
    subResources.setCount(numSubResources);

    // Set up the src row strides
    for (int i = 0; i < textureResourceDesc.numMipLevels; i++)
    {
        const int mipWidth = ITextureResource::Size::calcMipSize(textureResourceDesc.size.width, i);
        mipRowStrides[i] = mipWidth * sizeof(uint32_t);
    }

    // Set up pointers the the data
    {
        int subResourceIndex = 0;
        const int numGen = int(texData.dataBuffer.getCount());
        for (int i = 0; i < numSubResources; i++)
        {
            subResources[i] = texData.dataBuffer[subResourceIndex].getBuffer();
            // Wrap around
            subResourceIndex = (subResourceIndex + 1 >= numGen) ? 0 : (subResourceIndex + 1);
        }
    }

    initData.mipRowStrides = mipRowStrides.getBuffer();
    initData.numMips = textureResourceDesc.numMipLevels;
    initData.numSubResources = numSubResources;
    initData.subResources = subResources.getBuffer();

    textureOut = renderer->createTextureResource(IResource::Usage::GenericRead, textureResourceDesc, &initData);

    return textureOut ? SLANG_OK : SLANG_FAIL;
}

/* static */ Result ShaderRendererUtil::createBufferResource(
    const InputBufferDesc& inputDesc,
    bool isOutput,
    size_t bufferSize,
    const void* initData,
    IRenderer* renderer,
    Slang::ComPtr<IBufferResource>& bufferOut)
{
    IResource::Usage initialUsage = IResource::Usage::GenericRead;

    IBufferResource::Desc srcDesc;
    srcDesc.init(bufferSize);
    srcDesc.format = inputDesc.format;

    int bindFlags = 0;
    if (inputDesc.type == InputBufferType::ConstantBuffer)
    {
        bindFlags |= IResource::BindFlag::ConstantBuffer;
        srcDesc.cpuAccessFlags |= IResource::AccessFlag::Write;
        initialUsage = IResource::Usage::ConstantBuffer;
    }
    else
    {
        bindFlags |= IResource::BindFlag::UnorderedAccess | IResource::BindFlag::PixelShaderResource | IResource::BindFlag::NonPixelShaderResource;
        srcDesc.elementSize = inputDesc.stride;
        initialUsage = IResource::Usage::UnorderedAccess;
    }

    if (isOutput)
    {
        srcDesc.cpuAccessFlags |= IResource::AccessFlag::Read;
    }

    srcDesc.bindFlags = bindFlags;

    ComPtr<IBufferResource> bufferResource = renderer->createBufferResource(initialUsage, srcDesc, initData);
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

ComPtr<ISamplerState> _createSamplerState(IRenderer* renderer,
    const InputSamplerDesc& srcDesc)
{
    return renderer->createSamplerState(_calcSamplerDesc(srcDesc));
}

/* static */ Result ShaderRendererUtil::createBindingState(
    const ShaderInputLayout& layout,
    IRenderer* renderer,
    IBufferResource* addedConstantBuffer,
    BindingStateImpl** outBindingState)
{
    auto srcEntries = layout.entries.getBuffer();
    auto numEntries = layout.entries.getCount();

    const int textureBindFlags = IResource::BindFlag::NonPixelShaderResource | IResource::BindFlag::PixelShaderResource;

    List<IDescriptorSetLayout::SlotRangeDesc> slotRangeDescs;
    List<Index> mapEntryToSlotRange;

    if(addedConstantBuffer)
    {
        IDescriptorSetLayout::SlotRangeDesc slotRangeDesc;
        slotRangeDesc.type = DescriptorSlotType::UniformBuffer;

        slotRangeDescs.add(slotRangeDesc);
    }

    for (Index i = 0; i < numEntries; i++)
    {
        const ShaderInputLayoutEntry& srcEntry = srcEntries[i];
        SLANG_ASSERT(srcEntry.onlyCPULikeBinding == false);

        mapEntryToSlotRange.add(slotRangeDescs.getCount());
        IDescriptorSetLayout::SlotRangeDesc slotRangeDesc;

        switch (srcEntry.type)
        {
            case ShaderInputType::Buffer:
                {
                    const InputBufferDesc& srcBuffer = srcEntry.bufferDesc;

                    switch (srcBuffer.type)
                    {
                    case InputBufferType::ConstantBuffer:
                        slotRangeDesc.type = DescriptorSlotType::UniformBuffer;
                        break;

                    case InputBufferType::StorageBuffer:
                        slotRangeDesc.type = DescriptorSlotType::StorageBuffer;
                        break;

                    case InputBufferType::RootConstantBuffer:
                        {
                            // A root constant buffer maps to a root constant range
                            // where the `count` of slots is equal to the number
                            // of bytes of data.
                            //
                            Slang::UInt size = srcEntry.bufferData.getCount() * sizeof(srcEntry.bufferData[0]);
                            slotRangeDesc.type = DescriptorSlotType::RootConstant;
                            slotRangeDesc.count = size;
                        }
                        break;
                    }
                }
                break;

            case ShaderInputType::CombinedTextureSampler:
                {
                    slotRangeDesc.type = DescriptorSlotType::CombinedImageSampler;
                }
                break;

            case ShaderInputType::Texture:
                {
                    if (srcEntry.textureDesc.isRWTexture)
                    {
                        slotRangeDesc.type = DescriptorSlotType::StorageImage;
                    }
                    else
                    {
                        slotRangeDesc.type = DescriptorSlotType::SampledImage;
                    }
                }
                break;

            case ShaderInputType::Sampler:
                slotRangeDesc.type = DescriptorSlotType::Sampler;
                break;

            case ShaderInputType::Object:
                // We ignore the `Object` case here, knowing that it is meant for the shader-object path.
                continue;

            default:
                assert(!"Unhandled type");
                return SLANG_FAIL;
        }
        slotRangeDescs.add(slotRangeDesc);
    }

    IDescriptorSetLayout::Desc descriptorSetLayoutDesc;
    descriptorSetLayoutDesc.slotRangeCount = slotRangeDescs.getCount();
    descriptorSetLayoutDesc.slotRanges = slotRangeDescs.getBuffer();

    auto descriptorSetLayout = renderer->createDescriptorSetLayout(descriptorSetLayoutDesc);
    if(!descriptorSetLayout) return SLANG_FAIL;

    List<IPipelineLayout::DescriptorSetDesc> pipelineDescriptorSets;
    pipelineDescriptorSets.add(IPipelineLayout::DescriptorSetDesc(descriptorSetLayout));

    IPipelineLayout::Desc pipelineLayoutDesc;
    pipelineLayoutDesc.renderTargetCount = layout.numRenderTargets;
    pipelineLayoutDesc.descriptorSetCount = pipelineDescriptorSets.getCount();
    pipelineLayoutDesc.descriptorSets = pipelineDescriptorSets.getBuffer();

    auto pipelineLayout = renderer->createPipelineLayout(pipelineLayoutDesc);
    if(!pipelineLayout) return SLANG_FAIL;

    auto descriptorSet = renderer->createDescriptorSet(descriptorSetLayout, IDescriptorSet::Flag::Transient);
    if(!descriptorSet) return SLANG_FAIL;

    List<BindingStateImpl::OutputBinding> outputBindings;

    if(addedConstantBuffer)
    {
        descriptorSet->setConstantBuffer(0, 0, addedConstantBuffer);
    }
    for (int i = 0; i < numEntries; i++)
    {
        const ShaderInputLayoutEntry& srcEntry = srcEntries[i];

        auto rangeIndex = mapEntryToSlotRange[i];

        switch (srcEntry.type)
        {
            case ShaderInputType::Buffer:
                {
                    const InputBufferDesc& srcBuffer = srcEntry.bufferDesc;
                    const size_t bufferSize = srcEntry.bufferData.getCount() * sizeof(uint32_t);

                    if( srcBuffer.type == InputBufferType::RootConstantBuffer )
                    {
                        // A root constant buffer at the HLSL/Slang level actually
                        // maps to root constant data stored directly in the descriptor
                        // set, and thus does not need/want us to allocate a buffer
                        // to hold the data.
                        //
                        // Instead, we set the data directly here and then bypass
                        // the logic that handles the buffer-backed cases below.
                        //
                        descriptorSet->setRootConstants(rangeIndex, 0, bufferSize, srcEntry.bufferData.getBuffer());
                        break;
                    }

                    ComPtr<IBufferResource> bufferResource;
                    SLANG_RETURN_ON_FAIL(createBufferResource(srcEntry.bufferDesc, srcEntry.isOutput, bufferSize, srcEntry.bufferData.getBuffer(), renderer, bufferResource));

                    switch(srcBuffer.type)
                    {
                    case InputBufferType::ConstantBuffer:
                        descriptorSet->setConstantBuffer(rangeIndex, 0, bufferResource);
                        break;

                    case InputBufferType::StorageBuffer:
                        {
                            IResourceView::Desc viewDesc;
                            viewDesc.type = IResourceView::Type::UnorderedAccess;
                            viewDesc.format = srcBuffer.format;
                            auto bufferView = renderer->createBufferView(
                                bufferResource,
                                viewDesc);
                            descriptorSet->setResource(rangeIndex, 0, bufferView);
                        }
                        break;
                    }

                    if(srcEntry.isOutput)
                    {
                        BindingStateImpl::OutputBinding binding;
                        binding.entryIndex = i;
                        binding.resource = bufferResource;
                        outputBindings.add(binding);
                    }
                }
                break;

            case ShaderInputType::CombinedTextureSampler:
                {
                    ComPtr<ITextureResource> texture;
                    SLANG_RETURN_ON_FAIL(generateTextureResource(srcEntry.textureDesc, textureBindFlags, renderer, texture));

                    auto sampler = _createSamplerState(renderer, srcEntry.samplerDesc);

                    IResourceView::Desc viewDesc;
                    viewDesc.type = IResourceView::Type::ShaderResource;
                    auto textureView = renderer->createTextureView(
                        texture,
                        viewDesc);

                    descriptorSet->setCombinedTextureSampler(rangeIndex, 0, textureView, sampler);

                    if(srcEntry.isOutput)
                    {
                        BindingStateImpl::OutputBinding binding;
                        binding.entryIndex = i;
                        binding.resource = texture;
                        outputBindings.add(binding);
                    }
                }
                break;

            case ShaderInputType::Texture:
                {
                    ComPtr<ITextureResource> texture;
                    SLANG_RETURN_ON_FAIL(generateTextureResource(srcEntry.textureDesc, textureBindFlags, renderer, texture));

                    // TODO: support UAV textures...

                    IResourceView::Desc viewDesc;
                    viewDesc.type = IResourceView::Type::ShaderResource;
                    auto textureView = renderer->createTextureView(
                        texture,
                        viewDesc);

                    if (!textureView)
                    {
                        return SLANG_FAIL;
                    }

                    descriptorSet->setResource(rangeIndex, 0, textureView);

                    if(srcEntry.isOutput)
                    {
                        BindingStateImpl::OutputBinding binding;
                        binding.entryIndex = i;
                        binding.resource = texture;
                        outputBindings.add(binding);
                    }
                }
                break;

            case ShaderInputType::Sampler:
                {
                    auto sampler = _createSamplerState(renderer, srcEntry.samplerDesc);
                    descriptorSet->setSampler(rangeIndex, 0, sampler);
                }
                break;

            case ShaderInputType::Object:
                break;

            default:
                assert(!"Unhandled type");
                return SLANG_FAIL;
        }
    }

    BindingStateImpl* bindingState = new BindingStateImpl();
    bindingState->descriptorSet = descriptorSet;
    bindingState->pipelineLayout = pipelineLayout;
    bindingState->outputBindings = outputBindings;
    bindingState->m_numRenderTargets = layout.numRenderTargets;

    *outBindingState = bindingState;
    return SLANG_OK;
}

} // renderer_test
