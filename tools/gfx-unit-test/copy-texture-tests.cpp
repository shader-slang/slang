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
    void copyTextureTestImpl(IDevice* device, UnitTestContext* context)
    {
        ITextureResource::Size extent = {};
        extent.width = 2;
        extent.height = 2;
        extent.depth = 1;

        uint8_t srcTexData[] = { 255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                 0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u };
        ITextureResource::SubresourceData srcSubData = { (void*)srcTexData, 8, 0 };
        
        ITextureResource::Desc srcTexDesc = {};
        srcTexDesc.type = IResource::Type::Texture2D;
        srcTexDesc.numMipLevels = 1;
        srcTexDesc.arraySize = 1;
        srcTexDesc.size = extent;
        srcTexDesc.defaultState = ResourceState::UnorderedAccess;
        srcTexDesc.allowedStates = ResourceStateSet(
            ResourceState::UnorderedAccess,
            ResourceState::ShaderResource,
            ResourceState::CopySource);
        srcTexDesc.format = Format::R8G8B8A8_UINT;

        ComPtr<ITextureResource> srcTexture;
        GFX_CHECK_CALL_ABORT(device->createTextureResource(
            srcTexDesc,
            &srcSubData,
            srcTexture.writeRef()));

        SubresourceRange srcSubresource = {};
        srcSubresource.aspectMask = TextureAspect::Color;
        srcSubresource.mipLevel = 0;
        srcSubresource.mipLevelCount = 1;
        srcSubresource.baseArrayLayer = 0;
        srcSubresource.layerCount = 1;

        ITextureResource::Offset3D srcOffset;

        uint8_t dstTexData[16] = { 0u };
        ITextureResource::SubresourceData dstSubData = { (void*)dstTexData, 8, 0 };

        ITextureResource::Desc dstTexDesc = {};
        dstTexDesc.type = IResource::Type::Texture2D;
        dstTexDesc.numMipLevels = 1;
        dstTexDesc.arraySize = 1;
        dstTexDesc.size = extent;
        dstTexDesc.defaultState = ResourceState::CopyDestination;
        dstTexDesc.allowedStates = ResourceStateSet(
            ResourceState::ShaderResource,
            ResourceState::CopyDestination,
            ResourceState::CopySource);
        dstTexDesc.format = Format::R8G8B8A8_UINT;

        ComPtr<ITextureResource> dstTexture;
        GFX_CHECK_CALL_ABORT(device->createTextureResource(
            dstTexDesc,
            &dstSubData,
            dstTexture.writeRef()));

        SubresourceRange dstSubresource = {};
        dstSubresource.aspectMask = TextureAspect::Color;
        dstSubresource.mipLevel = 0;
        dstSubresource.mipLevelCount = 1;
        dstSubresource.baseArrayLayer = 0;
        dstSubresource.layerCount = 1;

        ITextureResource::Offset3D dstOffset;

        FormatInfo formatInfo;
        gfxGetFormatInfo(srcTexDesc.format, &formatInfo);
        UInt alignment = 256; // D3D requires rows to be aligned to a multiple of 256 bytes.
        auto alignedRowPitch = (extent.width * formatInfo.blockSizeInBytes + alignment - 1) & ~(alignment - 1);
        uint8_t initialData[512] = { 0 }; // Buffer will contain 512 bytes due to alignment rules.
        IBufferResource::Desc bufferDesc = {};
        bufferDesc.sizeInBytes = extent.height * alignedRowPitch;
        bufferDesc.format = gfx::Format::Unknown;
        bufferDesc.elementSize = sizeof(uint8_t);
        bufferDesc.allowedStates = ResourceStateSet(
            ResourceState::ShaderResource,
            ResourceState::UnorderedAccess,
            ResourceState::CopyDestination,
            ResourceState::CopySource);
        bufferDesc.defaultState = ResourceState::CopyDestination;
        bufferDesc.memoryType = MemoryType::DeviceLocal;

        ComPtr<IBufferResource> resultsBuffer;
        GFX_CHECK_CALL_ABORT(device->createBufferResource(
            bufferDesc,
            (void*)initialData,
            resultsBuffer.writeRef()));

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

    // D3D12 test currently fails due to an exception inside copyTextureToResource.
    SLANG_UNIT_TEST(copyTextureD3D12)
    {
        runTestImpl(copyTextureTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(copyTextureVulkan)
    {
        runTestImpl(copyTextureTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}
