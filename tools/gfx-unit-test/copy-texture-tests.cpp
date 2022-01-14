#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

#if SLANG_WINDOWS_FAMILY
#include <d3d12.h>
#endif

using namespace gfx;

namespace gfx_test
{
    struct BaseCopyTextureTest
    {
        IDevice* device;
        UnitTestContext* context;

        ComPtr<ITextureResource> srcTexture;
        ComPtr<ITextureResource> dstTexture;
        ComPtr<IBufferResource> resultsBuffer;
        Format format = Format::R8G8B8A8_UINT;

        size_t alignedRowPitch;

        struct TextureInfo
        {
            ITextureResource::Size extent;
            int numMipLevels;
            int arraySize;
            ITextureResource::SubresourceData const* initData;
        };

        void init(IDevice* device, UnitTestContext* context)
        {
            this->device = device;
            this->context = context;
        }

        void createRequiredResources(TextureInfo srcTextureInfo, TextureInfo dstTextureInfo)
        {
            ITextureResource::Desc srcTexDesc = {};
            srcTexDesc.type = IResource::Type::Texture2D;
            srcTexDesc.numMipLevels = srcTextureInfo.numMipLevels;
            srcTexDesc.arraySize = srcTextureInfo.arraySize;
            srcTexDesc.size = srcTextureInfo.extent;
            srcTexDesc.defaultState = ResourceState::UnorderedAccess;
            srcTexDesc.allowedStates = ResourceStateSet(
                ResourceState::UnorderedAccess,
                ResourceState::ShaderResource,
                ResourceState::CopySource);
            srcTexDesc.format = format;

            GFX_CHECK_CALL_ABORT(device->createTextureResource(
                srcTexDesc,
                srcTextureInfo.initData,
                srcTexture.writeRef()));

            ITextureResource::Desc dstTexDesc = {};
            dstTexDesc.type = IResource::Type::Texture2D;
            dstTexDesc.numMipLevels = dstTextureInfo.numMipLevels;
            dstTexDesc.arraySize = dstTextureInfo.arraySize;
            dstTexDesc.size = dstTextureInfo.extent;
            dstTexDesc.defaultState = ResourceState::CopyDestination;
            dstTexDesc.allowedStates = ResourceStateSet(
                ResourceState::ShaderResource,
                ResourceState::CopyDestination,
                ResourceState::CopySource);
            dstTexDesc.format = format;

            GFX_CHECK_CALL_ABORT(device->createTextureResource(
                dstTexDesc,
                dstTextureInfo.initData,
                dstTexture.writeRef()));

            FormatInfo formatInfo;
            gfxGetFormatInfo(format, &formatInfo);
            UInt alignment = 256; // D3D requires rows to be aligned to a multiple of 256 bytes.
            alignedRowPitch = (dstTextureInfo.extent.width * formatInfo.blockSizeInBytes + alignment - 1) & ~(alignment - 1);
            uint8_t initialData[512] = { 0 }; // Buffer will contain 512 bytes due to alignment rules.
            IBufferResource::Desc bufferDesc = {};
            bufferDesc.sizeInBytes = dstTextureInfo.extent.height * alignedRowPitch;
            bufferDesc.format = gfx::Format::Unknown;
            bufferDesc.elementSize = sizeof(uint8_t);
            bufferDesc.allowedStates = ResourceStateSet(
                ResourceState::ShaderResource,
                ResourceState::UnorderedAccess,
                ResourceState::CopyDestination,
                ResourceState::CopySource);
            bufferDesc.defaultState = ResourceState::CopyDestination;
            bufferDesc.memoryType = MemoryType::DeviceLocal;

            GFX_CHECK_CALL_ABORT(device->createBufferResource(
                bufferDesc,
                (void*)initialData,
                resultsBuffer.writeRef()));
        }

        void submitGPUWork(
            SubresourceRange srcSubresource,
            SubresourceRange dstSubresource,
            ITextureResource::Offset3D srcOffset,
            ITextureResource::Offset3D dstOffset,
            ITextureResource::Size extent)
        {
            Slang::ComPtr<ITransientResourceHeap> transientHeap;
            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeResourceCommands();

            encoder->textureSubresourceBarrier(srcTexture, srcSubresource, ResourceState::UnorderedAccess, ResourceState::CopySource);
            encoder->copyTexture(dstTexture, ResourceState::CopyDestination, dstSubresource, dstOffset, srcTexture, ResourceState::CopySource, srcSubresource, srcOffset, extent);
            encoder->textureSubresourceBarrier(dstTexture, dstSubresource, ResourceState::CopyDestination, ResourceState::CopySource);
            encoder->copyTextureToBuffer(resultsBuffer, 0, 16, dstTexture, ResourceState::CopySource, dstSubresource, dstOffset, extent);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void run()
        {
            ITextureResource::Size extent = {};
            extent.width = 2;
            extent.height = 2;
            extent.depth = 1;

            uint8_t srcTexData[] = { 255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                     0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u };
            ITextureResource::SubresourceData srcSubData = { (void*)srcTexData, 8, 0 };

            uint8_t dstTexData[16] = { 0u };
            ITextureResource::SubresourceData dstSubData = { (void*)dstTexData, 8, 0 };

            TextureInfo srcTextureInfo = { extent, 1, 1, &srcSubData };
            TextureInfo dstTextureInfo = { extent, 1, 1, &dstSubData };

            createRequiredResources(srcTextureInfo, dstTextureInfo);

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = TextureAspect::Color;
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            ITextureResource::Offset3D srcOffset;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = TextureAspect::Color;
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            ITextureResource::Offset3D dstOffset;

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, extent);

            if (device->getDeviceInfo().deviceType == DeviceType::DirectX12)
            {
                // D3D12 has to pad out the rows in order to adhere to alignment, so when comparing results
                // we need to make sure not to include the padding.
                size_t testOffset = 0;
                for (Int i = 0; i < extent.height; ++i)
                {
                    compareComputeResult(
                        device,
                        resultsBuffer,
                        testOffset,
                        srcTexData + 8 * i,
                        8);
                    testOffset += alignedRowPitch;
                }
            }
            else if (device->getDeviceInfo().deviceType == DeviceType::Vulkan)
            {
                compareComputeResult(
                    device,
                    resultsBuffer,
                    0,
                    srcTexData,
                    16);
            }
        }
    };

    template<typename T>
    void copyTextureTestImpl(IDevice* device, UnitTestContext* context)
    {
        T test;
        test.init(device, context);
        test.run();
    }

    // D3D12 test currently fails due to an exception inside copyTextureToResource.
    SLANG_UNIT_TEST(copyTextureD3D12)
    {
        runTestImpl(copyTextureTestImpl<BaseCopyTextureTest>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(copyTextureVulkan)
    {
        runTestImpl(copyTextureTestImpl<BaseCopyTextureTest>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}
