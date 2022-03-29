#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "gfx-test-texture-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

#if SLANG_WINDOWS_FAMILY
#include <d3d12.h>
#endif

using namespace Slang;
using namespace gfx;

namespace gfx_test
{
    struct BaseTextureViewTest
    {
        IDevice* device;
        UnitTestContext* context;

        IResourceView::Type viewType = IResourceView::Type::UnorderedAccess;
        size_t alignedRowStride;

        RefPtr<TextureInfo> textureInfo;
        RefPtr<ValidationTextureFormatBase> validationFormat;

        ComPtr<ITextureResource> texture;
        ComPtr<IResourceView> textureView;
        ComPtr<IBufferResource> resultsBuffer;
        ComPtr<IResourceView> bufferView;

        ComPtr<ISamplerState> sampler;

        const void* expectedTextureData;

        void init(
            IDevice* device,
            UnitTestContext* context,
            Format format,
            RefPtr<ValidationTextureFormatBase> validationFormat,
            ITextureResource::Type type)
        {
            this->device = device;
            this->context = context;
            this->validationFormat = validationFormat;

            this->textureInfo = new TextureInfo();
            this->textureInfo->format = format;
            this->textureInfo->textureType = type;
        }

        ResourceState getDefaultResourceStateForViewType(IResourceView::Type type)
        {
            switch (type)
            {
            case IResourceView::Type::RenderTarget:
                return ResourceState::RenderTarget;
            case IResourceView::Type::DepthStencil:
                return ResourceState::DepthWrite;
            case IResourceView::Type::ShaderResource:
                return ResourceState::ShaderResource;
            case IResourceView::Type::UnorderedAccess:
                return ResourceState::UnorderedAccess;
            case IResourceView::Type::AccelerationStructure:
                return ResourceState::AccelerationStructure;
            default:
                return ResourceState::Undefined;
            }
        }

        void createRequiredResources()
        {
            ITextureResource::Desc textureDesc = {};
            textureDesc.type = textureInfo->textureType;
            textureDesc.numMipLevels = textureInfo->mipLevelCount;
            textureDesc.arraySize = textureInfo->arrayLayerCount;
            textureDesc.size = textureInfo->extents;
            textureDesc.defaultState = getDefaultResourceStateForViewType(viewType);
            textureDesc.allowedStates = ResourceStateSet(
                ResourceState::UnorderedAccess,
                ResourceState::CopySource,
                ResourceState::CopyDestination);
            textureDesc.format = textureInfo->format;

            GFX_CHECK_CALL_ABORT(device->createTextureResource(
                textureDesc,
                textureInfo->subresourceDatas.getBuffer(),
                texture.writeRef()));

            IResourceView::Desc textureViewDesc = {};
            textureViewDesc.type = viewType;
            textureViewDesc.format = textureDesc.format; // TODO: Handle typeless formats - gfxIsTypelessFormat(format) ? convertTypelessFormat(format) : format;
            GFX_CHECK_CALL_ABORT(device->createTextureView(texture, textureViewDesc, textureView.writeRef()));

            auto texelSize = getTexelSize(textureInfo->format);
            size_t alignment;
            device->getTextureRowAlignment(&alignment);
            alignedRowStride = (textureInfo->extents.width * texelSize + alignment - 1) & ~(alignment - 1);
            IBufferResource::Desc bufferDesc = {};
            // All of the values read back from the shader will be uint32_t
            bufferDesc.sizeInBytes = textureDesc.size.width * textureDesc.size.height * textureDesc.size.depth * texelSize * sizeof(uint32_t);
            bufferDesc.format = Format::Unknown;
            bufferDesc.elementSize = sizeof(uint32_t);
            bufferDesc.allowedStates = ResourceStateSet(
                ResourceState::UnorderedAccess,
                ResourceState::CopyDestination,
                ResourceState::CopySource);
            bufferDesc.defaultState = ResourceState::UnorderedAccess;
            bufferDesc.memoryType = MemoryType::DeviceLocal;

            GFX_CHECK_CALL_ABORT(device->createBufferResource(bufferDesc, nullptr, resultsBuffer.writeRef()));

            IResourceView::Desc bufferViewDesc = {};
            bufferViewDesc.type = IResourceView::Type::UnorderedAccess;
            bufferViewDesc.format = Format::Unknown;
            GFX_CHECK_CALL_ABORT(device->createBufferView(resultsBuffer, nullptr, bufferViewDesc, bufferView.writeRef()));
        }

        void submitShaderWork(const char* entryPoint)
        {
            Slang::ComPtr<ITransientResourceHeap> transientHeap;
            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, "trivial-copy-textures", entryPoint, slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            ComPtr<gfx::IPipelineState> pipelineState;
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));

            // We have done all the set up work, now it is time to start recording a command buffer for
            // GPU execution.
            {
                ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
                auto queue = device->createCommandQueue(queueDesc);

                auto commandBuffer = transientHeap->createCommandBuffer();
                auto encoder = commandBuffer->encodeComputeCommands();

                auto rootObject = encoder->bindPipeline(pipelineState);

                ShaderCursor entryPointCursor(
                    rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.

                auto width = textureInfo->extents.width;
                auto height = textureInfo->extents.height;
                auto depth = textureInfo->extents.depth;

                entryPointCursor["width"].setData(width);
                entryPointCursor["height"].setData(height);
                entryPointCursor["depth"].setData(depth);

                // Bind texture view to the entry point
                entryPointCursor["resourceView"].setResource(textureView); // TODO: Bind nullptr and make sure it doesn't splut - should be 0 everywhere
                entryPointCursor["testResults"].setResource(bufferView);

                if (sampler) entryPointCursor["sampler"].setSampler(sampler); // TODO: Bind nullptr and make sure it doesn't splut

                auto bufferElementCount = width * height * depth;
                encoder->dispatchCompute(bufferElementCount, 1, 1);
                encoder->endEncoding();
                commandBuffer->close();
                queue->executeCommandBuffer(commandBuffer);
                queue->waitOnHost();
            }
        }

        void validateTextureValues(ValidationTextureData actual)
        {
            for (Int x = 0; x < actual.extents.width; ++x)
            {
                for (Int y = 0; y < actual.extents.height; ++y)
                {
                    for (Int z = 0; z < actual.extents.depth; ++z)
                    {
                        auto actualBlock = (uint8_t*)actual.getBlockAt(x, y, z);
                        for (Int i = 0; i < 4; ++i)
                        {
                            SLANG_CHECK(actualBlock[i] == 1);
                        }
                    }
                }
            }
        }

        void checkTestResults()
        {
            ComPtr<ISlangBlob> textureBlob;
            size_t outRowPitch;
            size_t outPixelSize;
            GFX_CHECK_CALL_ABORT(device->readTextureResource(texture, ResourceState::CopySource, textureBlob.writeRef(), &outRowPitch, &outPixelSize));
            auto textureValues = (uint8_t*)textureBlob->getBufferPointer();

            ValidationTextureData textureResults;
            textureResults.extents = textureInfo->extents;
            textureResults.textureData = textureValues;
            textureResults.strides.x = getTexelSize(textureInfo->format);
            textureResults.strides.y = alignedRowStride;
            textureResults.strides.z = textureResults.extents.height * textureResults.strides.y;

            validateTextureValues(textureResults);

            ComPtr<ISlangBlob> bufferBlob;
            GFX_CHECK_CALL_ABORT(device->readBufferResource(resultsBuffer, 0, resultsBuffer->getDesc()->sizeInBytes, bufferBlob.writeRef()));
            auto results = (uint32_t*)bufferBlob->getBufferPointer();

            auto elementCount = textureInfo->extents.width * textureInfo->extents.height * textureInfo->extents.depth * 4;
            auto castedTextureData = (uint8_t*)expectedTextureData;
            for (Int i = 0; i < elementCount; ++i)
            {
                SLANG_CHECK(results[i] == castedTextureData[i]);
            }
        }
    };

    struct UnorderedAccessView : BaseTextureViewTest
    {
        void run()
        {
//             ISamplerState::Desc samplerDesc;
//             sampler = device->createSamplerState(samplerDesc);

            textureInfo->extents.width = 4;
            textureInfo->extents.height = 4;
            textureInfo->extents.depth = 2;
            textureInfo->mipLevelCount = 1;
            textureInfo->arrayLayerCount = 1;
            generateTextureData(textureInfo, validationFormat);

            // We need to save the pointer to the original texture data for results checking because the texture will be
            // overwritten during testing (if the texture can be written to).
            expectedTextureData = textureInfo->subresourceDatas[getSubresourceIndex(0, 1, 0)].data;

            createRequiredResources();
            submitShaderWork("resourceViewTest");

            checkTestResults();
        }
    };

    template <typename T>
    void textureTypeTestImpl(IDevice* device, UnitTestContext* context)
    {
        // TODO: Loop over all view types
        auto format = Format::R8G8B8A8_UINT;
        auto validationFormat = getValidationTextureFormat(format);
        if (!validationFormat)
            SLANG_CHECK_ABORT(false);

        T test;
        test.init(device, context, format, validationFormat, ITextureResource::Type::Texture3D);
        test.run();
    }

    SLANG_UNIT_TEST(unorderedAccessTests)
    {
        runTestImpl(textureTypeTestImpl<UnorderedAccessView>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(textureTypeTestImpl<UnorderedAccessView>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}

// 1D + array + multisample, ditto for 2D, ditto for 3D
// one test with something bound, one test with nothing bound, one test with subset of layers (set values in SubresourceRange and assign in desc)
