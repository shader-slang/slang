#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

#if SLANG_WINDOWS_FAMILY
#include <d3d12.h>
#endif

using namespace Slang;
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

        void createRequiredResources(TextureInfo srcTextureInfo, TextureInfo dstTextureInfo, Format format)
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
            UInt alignment;
            device->getTextureRowAlignment(&alignment);
            alignedRowPitch = (dstTextureInfo.extent.width * formatInfo.blockSizeInBytes + alignment - 1) & ~(alignment - 1);
            IBufferResource::Desc bufferDesc = {};
            bufferDesc.sizeInBytes = dstTextureInfo.extent.height * alignedRowPitch;
            bufferDesc.format = gfx::Format::Unknown;
            bufferDesc.elementSize = 0;
            bufferDesc.allowedStates = ResourceStateSet(
                ResourceState::ShaderResource,
                ResourceState::UnorderedAccess,
                ResourceState::CopyDestination,
                ResourceState::CopySource);
            bufferDesc.defaultState = ResourceState::CopyDestination;
            bufferDesc.memoryType = MemoryType::DeviceLocal;

            GFX_CHECK_CALL_ABORT(device->createBufferResource(
                bufferDesc,
                nullptr,
                resultsBuffer.writeRef()));
        }

        void submitGPUWork(
            SubresourceRange srcSubresource,
            SubresourceRange dstSubresource,
            ITextureResource::Offset3D srcOffset,
            ITextureResource::Offset3D dstOffset,
            ITextureResource::Size extent,
            size_t textureSize)
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
            encoder->copyTextureToBuffer(resultsBuffer, 0, textureSize, alignedRowPitch, dstTexture, ResourceState::CopySource, dstSubresource, dstOffset, extent);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }
    };

    struct SimpleCopyTexture : BaseCopyTextureTest
    {
        Format format = Format::R8G8B8A8_UINT;

        void run()
        {
            ITextureResource::Size extent = {};
            extent.width = 2;
            extent.height = 2;
            extent.depth = 1;

            uint8_t srcTexData[16] = { 255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                       0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u };
            ITextureResource::SubresourceData srcSubData = { (void*)srcTexData, 8, 0 };

            uint8_t dstTexData[16] = { 0u };
            ITextureResource::SubresourceData dstSubData = { (void*)dstTexData, 8, 0 };

            TextureInfo srcTextureInfo = { extent, 1, 1, &srcSubData };
            TextureInfo dstTextureInfo = { extent, 1, 1, &dstSubData };

            createRequiredResources(srcTextureInfo, dstTextureInfo, format);

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

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, extent, 16);

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

    struct Texel
    {
        float channels[4];
    };

    struct SubresourceStuff : RefObject
    {
        List<Texel> texels;
    };

    struct TextureStuff : RefObject
    {
        List<RefPtr<SubresourceStuff>> subresourceObjects;
        List<ITextureResource::SubresourceData> subresourceDatas;
    };

    struct CopyTextureSection : BaseCopyTextureTest
    {
        Format format = Format::R32G32B32A32_FLOAT;

        RefPtr<TextureStuff> generateTextureData(int width, int height, uint32_t mipLevels, uint32_t arrayLayers)
        {
            RefPtr<TextureStuff> texture = new TextureStuff();
            for (uint32_t layer = 0; layer < arrayLayers; ++layer)
            {
                for (uint32_t mip = 0; mip < mipLevels; ++mip)
                {
                    RefPtr<SubresourceStuff> subresource = new SubresourceStuff();
                    texture->subresourceObjects.add(subresource);

                    int mipWidth = Math::Max(width >> mip, 1);
                    int mipHeight = Math::Max(height >> mip, 1);

                    int mipTexelCount = mipWidth * mipHeight;
                    subresource->texels.setCount(mipTexelCount);
                    for (int h = 0; h < mipHeight; ++h)
                    {
                        for (int w = 0; w < mipWidth; ++w)
                        {
                            // 4 channels per pixel
                            subresource->texels[h * mipWidth + w].channels[0] = (float)w;
                            subresource->texels[h * mipWidth + w].channels[1] = (float)h;
                            subresource->texels[h * mipWidth + w].channels[2] = (float)mip;
                            subresource->texels[h * mipWidth + w].channels[3] = (float)layer;
                        }
                    }

                    ITextureResource::SubresourceData subData = {};
                    subData.data = subresource->texels.getBuffer();
                    subData.strideY = mipWidth * sizeof(Texel);
                    subData.strideZ = mipHeight * subData.strideY;
                    texture->subresourceDatas.add(subData);
                }
            }
            return texture;
        }

        // TODO: Things to test in the future
        // 1. Size of src texture(W, H, mips, layers)
        // 2. Size of dst texture(...)
        // 3. src subresource(mip, layer)
        // 4. dst subresource(mip, layer)
        // 5. copy extents(x, y)
        // 6. copy src coords(x, y)
        // 7. copy dst coords(x, y)
        // 8. Final buffer offset

        void run()
        {
            ITextureResource::Size extent = {};
            extent.width = 4;
            extent.height = 4;
            extent.depth = 1;

            auto mipLevelCount = 2;
            auto arrayLayerCount = 2;

            auto srcTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount);
            
            TextureInfo srcTextureInfo = { extent, mipLevelCount, arrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };
            TextureInfo dstTextureInfo = { extent, mipLevelCount, arrayLayerCount, nullptr };

            createRequiredResources(srcTextureInfo, dstTextureInfo, format);

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = TextureAspect::Color;
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 1;
            srcSubresource.layerCount = 1;

            ITextureResource::Offset3D srcOffset;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = TextureAspect::Color;
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            ITextureResource::Offset3D dstOffset;

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, extent, extent.height * 256);

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[2];
            if (device->getDeviceInfo().deviceType == DeviceType::DirectX12)
            {
                // D3D12 has to pad out the rows in order to adhere to alignment, so when comparing results
                // we need to make sure not to include the padding.
                size_t testOffset = 0;
                size_t dataOffset = 0;
                auto rowStride = expectedData.strideY / 4;
                for (Int i = 0; i < extent.height; ++i)
                {
                    compareComputeResult(
                        device,
                        resultsBuffer,
                        testOffset,
                        (float*) expectedData.data + rowStride * i,
                        rowStride * 4);
                    testOffset += alignedRowPitch;
                }
                dataOffset += srcTextureStuff->subresourceDatas[0].strideZ / 4;
            }
            else if (device->getDeviceInfo().deviceType == DeviceType::Vulkan)
            {
                compareComputeResult(
                    device,
                    resultsBuffer,
                    0,
                    expectedData.data,
                    64);
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

    SLANG_UNIT_TEST(copyTextureSimpleD3D12)
    {
        runTestImpl(copyTextureTestImpl<SimpleCopyTexture>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(copyTextureSectionD3D12)
    {
        runTestImpl(copyTextureTestImpl<CopyTextureSection>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(copyTextureSimpleVulkan)
    {
        runTestImpl(copyTextureTestImpl<SimpleCopyTexture>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copyTextureSectionVulkan)
    {
        runTestImpl(copyTextureTestImpl<CopyTextureSection>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}
