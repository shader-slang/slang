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

    struct ValidationTextureFormatBase
    {
        virtual void validateBlocksEqual(const void* actual, const void* expected) = 0;
    };

    struct ValidationTextureFormat_float4 : ValidationTextureFormatBase
    {
        virtual void validateBlocksEqual(const void* actual, const void* expected) override
        {
            auto a = (const float*)actual;
            auto e = (const float*)expected;

            for (Int i = 0; i < 4; ++i)
            {
                SLANG_CHECK(a[i] == e[i]);
            }
        }
    };

    struct ValidationTextureData
    {
        const void* textureData;
        ITextureResource::Size extents;
        ITextureResource::Offset3D strides;

        ValidationTextureFormatBase* format;

        const void* getBlockAt(Int x, Int y, Int z)
        {
            assert(x >= 0 && x < extents.width);
            assert(y >= 0 && y < extents.height);
            assert(z >= 0 && z < extents.depth);

            const char* layerData = (const char*)textureData + z * strides.z;
            const char* rowData = layerData + y * strides.y;
            return rowData + x * strides.x;
        }
    };

    struct BaseCopyTextureTest
    {
        IDevice* device;
        UnitTestContext* context;

        ComPtr<ITextureResource> srcTexture;
        ComPtr<ITextureResource> dstTexture;
        ComPtr<IBufferResource> resultsBuffer;

        size_t alignedRowStride;

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
            size_t alignment;
            device->getTextureRowAlignment(&alignment);
            alignedRowStride = (dstTextureInfo.extent.width * formatInfo.blockSizeInBytes + alignment - 1) & ~(alignment - 1);
            IBufferResource::Desc bufferDesc = {};
            bufferDesc.sizeInBytes = dstTextureInfo.extent.height * alignedRowStride;
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
            encoder->copyTextureToBuffer(resultsBuffer, 0, textureSize, alignedRowStride, dstTexture, ResourceState::CopySource, dstSubresource, dstOffset, extent);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void validateTestResults(ValidationTextureData actual, ValidationTextureData expectedCopied, ValidationTextureData expectedOriginal)
        {
            auto actualExtents = actual.extents;
            auto expectedCopiedExtents = expectedCopied.extents;
            auto format = actual.format;
            for (Int x = 0; x < actualExtents.width; ++x)
            {
                for (Int y = 0; y < actualExtents.height; ++y)
                {
                    for (Int z = 0; z < actualExtents.depth; ++z)
                    {
                        auto actualBlock = actual.getBlockAt(x, y, z);
                        if (x < expectedCopiedExtents.width && y < expectedCopiedExtents.height && z < expectedCopiedExtents.depth)
                        {
                            // Block is located within the bounds of the source texture
                            auto expectedBlock = expectedCopied.getBlockAt(x, y, z);
                            format->validateBlocksEqual(actualBlock, expectedBlock);
                        }
                        else
                        {
                            // Block is located outside the bounds of the source texture and should be compared
                            // against known expected values for the destination texture.
                            auto expectedBlock = expectedOriginal.getBlockAt(x, y, z);
                            format->validateBlocksEqual(actualBlock, expectedBlock);
                        }
                    }
                }
            }
        }

        void checkTestResults(
            ITextureResource::Size srcExtent,
            ITextureResource::Size dstExtent,
            ITextureResource::Size copyExtent,
            ITextureResource::Offset3D srcOffset,
            const void* expectedCopiedData,
            const void* expectedOriginalData)
        {
            ComPtr<ISlangBlob> resultBlob;
            auto resultsSize = dstExtent.depth * dstExtent.height * alignedRowStride;
            GFX_CHECK_CALL_ABORT(device->readBufferResource(resultsBuffer, 0, resultsSize, resultBlob.writeRef()));
            auto results = resultBlob->getBufferPointer();

            ValidationTextureFormat_float4 validationFormat;

            ValidationTextureData actual;
            actual.extents = dstExtent;
            actual.format = &validationFormat;
            actual.textureData = results;
            actual.strides.x = sizeof(Texel);
            actual.strides.y = (uint32_t)alignedRowStride;
            actual.strides.z = dstExtent.height * actual.strides.y;

            ValidationTextureData expectedCopied;
            expectedCopied.extents = srcExtent;
            expectedCopied.format = &validationFormat;
            expectedCopied.textureData = expectedCopiedData;
            expectedCopied.strides.x = sizeof(Texel);
            expectedCopied.strides.y = srcExtent.width * expectedCopied.strides.x;
            expectedCopied.strides.z = srcExtent.height * expectedCopied.strides.y;

            ValidationTextureData expectedOriginal;
            if (expectedOriginalData)
            {
                expectedOriginal.extents = dstExtent;
                expectedOriginal.format = &validationFormat;
                expectedOriginal.textureData = expectedOriginalData;
                expectedOriginal.strides.x = sizeof(Texel);
                expectedOriginal.strides.y = dstExtent.width * expectedOriginal.strides.x;
                expectedOriginal.strides.z = dstExtent.height * expectedOriginal.strides.y;
            }

            validateTestResults(actual, expectedCopied, expectedOriginal);
        }

//         template <typename T>
//         void checkTestResults(
//             ITextureResource::Size srcExtent,
//             ITextureResource::Size dstExtent,
//             ITextureResource::Size copyExtent,
//             ITextureResource::Offset3D srcOffset,
//             const void* expectedSubresourceData)
//         {
//             ComPtr<ISlangBlob> resultBlob;
//             auto resultsSize = dstExtent.width * dstExtent.height * dstExtent.depth * 4 * sizeof(T);
//             GFX_CHECK_CALL_ABORT(device->readBufferResource(resultsBuffer, 0, resultsSize, resultBlob.writeRef()));
//             auto results = (T*)resultBlob->getBufferPointer();
//             auto expecteds = (T*)expectedSubresourceData;
//             for (Int w = 0; w < dstExtent.width; ++w)
//             {
//                 for (Int h = 0; h < dstExtent.height; ++h)
//                 {
//                     if (w < srcOffset.x || h < srcOffset.y || w >= copyExtent.width + srcOffset.x || h >= copyExtent.height + srcOffset.y)
//                     {
//                         // Outside of the bounds of the copy operation
//                         SLANG_CHECK(results[4 * (h * dstExtent.width + w)] == 0);
//                         SLANG_CHECK(results[4 * (h * dstExtent.width + w) + 1] == 0);
//                         SLANG_CHECK(results[4 * (h * dstExtent.width + w) + 2] == 0);
//                         SLANG_CHECK(results[4 * (h * dstExtent.width + w) + 3] == 0);
//                     }
//                     else
//                     {
//                         // Inside the bounds of the copy operation
//                         SLANG_CHECK(results[4 * (h * dstExtent.width + w)] == expecteds[4 * (h * srcExtent.width + w)]);
//                         SLANG_CHECK(results[4 * (h * dstExtent.width + w) + 1] == expecteds[4 * (h * srcExtent.width + w) + 1]);
//                         SLANG_CHECK(results[4 * (h * dstExtent.width + w) + 2] == expecteds[4 * (h * srcExtent.width + w) + 2]);
//                         SLANG_CHECK(results[4 * (h * dstExtent.width + w) + 3] == expecteds[4 * (h * srcExtent.width + w) + 3]);
//                     }
//                 }
//             }
//         }
    };

    struct SimpleCopyTexture : BaseCopyTextureTest
    {
        Format format = Format::R32G32B32A32_FLOAT;

        void run()
        {
            ITextureResource::Size extent = {};
            extent.width = 4;
            extent.height = 4;
            extent.depth = 1;
            auto mipLevelCount = 1;
            auto arrayLayerCount = 1;
            
            auto srcTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount);

            TextureInfo srcTextureInfo = { extent, mipLevelCount, arrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };
            TextureInfo dstTextureInfo = { extent, mipLevelCount, arrayLayerCount, nullptr };

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

            auto expectedData = srcTextureStuff->subresourceDatas[0];
            checkTestResults(extent, extent, extent, srcOffset, expectedData.data, nullptr);
        }
    };

    struct CopyTextureSection : BaseCopyTextureTest
    {
        Format format = Format::R32G32B32A32_FLOAT;

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
            checkTestResults(extent, extent, extent, srcOffset, expectedData.data, nullptr);
        }
    };

    struct LargeSrcToSmallDst : BaseCopyTextureTest
    {
        void run()
        {
            Format format = Format::R32G32B32A32_FLOAT;

            ITextureResource::Size srcExtent = {};
            srcExtent.width = 16;
            srcExtent.height = 16;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 4;
            dstExtent.height = 4;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

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

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, dstExtent, dstExtent.height * 256);

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, dstExtent, dstExtent, srcOffset, expectedData.data, nullptr);
        }
    };

    struct SmallSrcToLargeDst : BaseCopyTextureTest
    {
        void run()
        {
            Format format = Format::R32G32B32A32_FLOAT;

            ITextureResource::Size srcExtent = {};
            srcExtent.width = 4;
            srcExtent.height = 4;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 16;
            dstExtent.height = 16;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount);
            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

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

            ITextureResource::Size copyExtent = srcExtent;

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, copyExtent, copyExtent.height * 256);

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, dstExtent, copyExtent, srcOffset, expectedCopiedData.data, expectedOriginalData.data);
        }
    };

    struct CopyBetweenMips : BaseCopyTextureTest
    {
        void run()
        {
            Format format = Format::R32G32B32A32_FLOAT;

            ITextureResource::Size srcExtent = {};
            srcExtent.width = 16;
            srcExtent.height = 16;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 4;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 16;
            dstExtent.height = 16;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 4;
            auto dstArrayLayerCount = 1;

            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

            createRequiredResources(srcTextureInfo, dstTextureInfo, format);

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = TextureAspect::Color;
            srcSubresource.mipLevel = 2;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            ITextureResource::Offset3D srcOffset;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = TextureAspect::Color;
            dstSubresource.mipLevel = 1;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            ITextureResource::Offset3D dstOffset;

            // These are the extents of the mip layer being copied from.
            ITextureResource::Size copyExtent;
            copyExtent.width = 4;
            copyExtent.height = 4;
            copyExtent.depth = 1;

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, copyExtent, copyExtent.height * 256);

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[0];
            if (device->getDeviceInfo().deviceType == DeviceType::DirectX12)
            {
                // D3D12 has to pad out the rows in order to adhere to alignment, so when comparing results
                // we need to make sure not to include the padding.
                size_t testOffset = 0;
                Int srcRowStride = expectedData.strideY / 4;
                Int dstRowStride = copyExtent.width * sizeof(Texel) / 4;
                for (Int i = 0; i < copyExtent.height; ++i)
                {
                    compareComputeResult(
                        device,
                        resultsBuffer,
                        testOffset,
                        (float*)expectedData.data + srcRowStride * i,
                        dstRowStride * 4);
                    testOffset += alignedRowStride;
                }
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

    struct CopyBetweenLayers : BaseCopyTextureTest
    {
        void run()
        {
            Format format = Format::R32G32B32A32_FLOAT;

            ITextureResource::Size srcExtent = {};
            srcExtent.width = 4;
            srcExtent.height = 4;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 2;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 4;
            dstExtent.height = 4;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 2;

            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

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
            dstSubresource.baseArrayLayer = 1;
            dstSubresource.layerCount = 1;

            ITextureResource::Offset3D dstOffset;

            ITextureResource::Size copyExtent = srcExtent;

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, copyExtent, copyExtent.height * 256);

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[0];
            if (device->getDeviceInfo().deviceType == DeviceType::DirectX12)
            {
                // D3D12 has to pad out the rows in order to adhere to alignment, so when comparing results
                // we need to make sure not to include the padding.
                size_t testOffset = 0;
                Int srcRowStride = expectedData.strideY / 4;
                Int dstRowStride = copyExtent.width * sizeof(Texel) / 4;
                for (Int i = 0; i < copyExtent.height; ++i)
                {
                    compareComputeResult(
                        device,
                        resultsBuffer,
                        testOffset,
                        (float*)expectedData.data + srcRowStride * i,
                        dstRowStride * 4);
                    testOffset += alignedRowStride;
                }
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

    struct CopyWithOffsets : BaseCopyTextureTest
    {
        void run()
        {
            Format format = Format::R32G32B32A32_FLOAT;

            ITextureResource::Size srcExtent = {};
            srcExtent.width = 8;
            srcExtent.height = 8;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 16;
            dstExtent.height = 16;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

            createRequiredResources(srcTextureInfo, dstTextureInfo, format);

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = TextureAspect::Color;
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            ITextureResource::Offset3D srcOffset;
            srcOffset.x = 4;
            srcOffset.y = 4;
            srcOffset.z = 0;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = TextureAspect::Color;
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            ITextureResource::Offset3D dstOffset;
            dstOffset.x = 4;
            dstOffset.y = 4;
            dstOffset.z = 0;

            ITextureResource::Size copyExtent = srcExtent;

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, copyExtent, copyExtent.height * 256);

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[0];
            if (device->getDeviceInfo().deviceType == DeviceType::DirectX12)
            {
                // D3D12 has to pad out the rows in order to adhere to alignment, so when comparing results
                // we need to make sure not to include the padding.
                size_t testOffset = 0;
                Int srcRowStride = expectedData.strideY / 4;
                Int dstRowStride = copyExtent.width * sizeof(Texel) / 4;
                for (Int i = 0; i < copyExtent.height; ++i)
                {
                    compareComputeResult(
                        device,
                        resultsBuffer,
                        testOffset,
                        (float*)expectedData.data + srcRowStride * i,
                        dstRowStride * 4);
                    testOffset += alignedRowStride;
                }
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

    struct CopySectionWithSetExtent : BaseCopyTextureTest
    {
        void run()
        {
            Format format = Format::R32G32B32A32_FLOAT;

            ITextureResource::Size srcExtent = {};
            srcExtent.width = 8;
            srcExtent.height = 8;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 8;
            dstExtent.height = 8;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

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
            dstOffset.x = 4;
            dstOffset.y = 4;
            dstOffset.z = 0;

            ITextureResource::Size copyExtent;
            copyExtent.width = 4;
            copyExtent.height = 4;
            copyExtent.depth = 1;

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, copyExtent, copyExtent.height * 256);

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[0];
            if (device->getDeviceInfo().deviceType == DeviceType::DirectX12)
            {
                // D3D12 has to pad out the rows in order to adhere to alignment, so when comparing results
                // we need to make sure not to include the padding.
                size_t testOffset = 0;
                Int srcRowStride = expectedData.strideY / 4;
                Int dstRowStride = copyExtent.width * sizeof(Texel) / 4;
                for (Int i = 0; i < copyExtent.height; ++i)
                {
                    
                    compareComputeResult(
                        device,
                        resultsBuffer,
                        testOffset,
                        (float*)expectedData.data + srcRowStride * i,
                        dstRowStride * 4);
                    testOffset += alignedRowStride;
                }
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

    struct CopyToBufferWithOffset : BaseCopyTextureTest
    {
        void run()
        {
            Format format = Format::R32G32B32A32_FLOAT;

            ITextureResource::Size srcExtent = {};
            srcExtent.width = 8;
            srcExtent.height = 8;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 4;
            dstExtent.height = 6;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

            createRequiredResources(srcTextureInfo, dstTextureInfo, format);

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = TextureAspect::Color;
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            ITextureResource::Offset3D srcOffset;
            srcOffset.x = 4;
            srcOffset.y = 4;
            srcOffset.z = 0;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = TextureAspect::Color;
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            ITextureResource::Size copyExtent;
            copyExtent.width = 4;
            copyExtent.height = 4;
            copyExtent.depth = 1;

            auto textureSize = copyExtent.width * copyExtent.height * copyExtent.depth * 16; // Size of the texture section being copied.

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
            encoder->copyTextureToBuffer(resultsBuffer, 512, textureSize, alignedRowStride, srcTexture, ResourceState::CopySource, srcSubresource, srcOffset, copyExtent);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();

            //checkTestResults<float>(srcExtent, dstExtent, copyExtent, srcOffset, srcTextureStuff->subresourceDatas[0].data);
        }
    };

    template<typename T>
    void copyTextureTestImpl(IDevice* device, UnitTestContext* context)
    {
        T test;
        test.init(device, context);
        test.run();
    }

    SLANG_UNIT_TEST(copyTextureTests)
    {
        runTestImpl(copyTextureTestImpl<SmallSrcToLargeDst>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<SmallSrcToLargeDst>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copyTextureSimple)
    {
        runTestImpl(copyTextureTestImpl<SimpleCopyTexture>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<SimpleCopyTexture>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copyTextureSection)
    {
        runTestImpl(copyTextureTestImpl<CopyTextureSection>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<CopyTextureSection>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copyLargeToSmallTexture)
    {
        runTestImpl(copyTextureTestImpl<LargeSrcToSmallDst>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<LargeSrcToSmallDst>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copySmallToLargeTexture)
    {
        runTestImpl(copyTextureTestImpl<SmallSrcToLargeDst>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<SmallSrcToLargeDst>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copyBetweenMips)
    {
        runTestImpl(copyTextureTestImpl<CopyBetweenMips>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<CopyBetweenMips>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copyBetweenLayers)
    {
        runTestImpl(copyTextureTestImpl<CopyBetweenLayers>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<CopyBetweenLayers>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copyWithOffsets)
    {
        runTestImpl(copyTextureTestImpl<CopyWithOffsets>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<CopyWithOffsets>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copySectionWithSetExtent)
    {
        runTestImpl(copyTextureTestImpl<CopySectionWithSetExtent>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<CopySectionWithSetExtent>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(copyToBufferWithOffset)
    {
        runTestImpl(copyTextureTestImpl<CopyToBufferWithOffset>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<CopyToBufferWithOffset>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}
