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
        dstTexDesc.defaultState = ResourceState::ShaderResource;
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

        const int numberCount = 16;
        uint8_t initialData[] = { 0 };
        IBufferResource::Desc bufferDesc = {};
        bufferDesc.sizeInBytes = numberCount * sizeof(uint8_t);
        bufferDesc.format = gfx::Format::Unknown;
        bufferDesc.elementSize = sizeof(uint8_t);
        bufferDesc.allowedStates = ResourceStateSet(
            ResourceState::ShaderResource,
            ResourceState::UnorderedAccess,
            ResourceState::CopyDestination,
            ResourceState::CopySource);
        bufferDesc.defaultState = ResourceState::ShaderResource;
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

        encoder->textureSubresourceBarrier(srcTexture, srcSubresource, ResourceState::UnorderedAccess, ResourceState::ShaderResource);
        encoder->copyTexture(dstTexture, ResourceState::ShaderResource, dstSubresource, dstOffset, srcTexture, ResourceState::ShaderResource, srcSubresource, srcOffset, extent);
        encoder->copyTextureToBuffer(resultsBuffer, 0, 16, dstTexture, ResourceState::ShaderResource, srcSubresource, srcOffset, extent);
        encoder->endEncoding();
        commandBuffer->close();
        queue->executeCommandBuffer(commandBuffer);
        queue->waitOnHost();

        compareComputeResult(
            device,
            resultsBuffer,
            srcTexData,
            16);
    }

    // D3D12 test currently fails due to an exception inside copyTextureToResource.
//     SLANG_UNIT_TEST(copyTextureD3D12)
//     {
//         runTestImpl(copyTextureTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
//     }

    SLANG_UNIT_TEST(copyTextureVulkan)
    {
        runTestImpl(copyTextureTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}
