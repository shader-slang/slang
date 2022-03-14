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
    struct ValidationTextureFormatBase : RefObject
    {
        virtual void validateBlocksEqual(const void* actual, const void* expected) = 0;

        virtual void initializeTexel(void* texel, int x, int y, int z, int mipLevel, int arrayLayer) = 0;
    };

    template <typename T>
    struct ValidationTextureFormat : ValidationTextureFormatBase
    {
        int componentCount;

        ValidationTextureFormat(int componentCount) : componentCount(componentCount) {};

        virtual void validateBlocksEqual(const void* actual, const void* expected) override
        {
            auto a = (const T*)actual;
            auto e = (const T*)expected;

            for (Int i = 0; i < componentCount; ++i)
            {
                SLANG_CHECK(a[i] == e[i]);
            }
        }

        virtual void initializeTexel(void* texel, int x, int y, int z, int mipLevel, int arrayLayer) override
        {
            auto temp = (T*)texel;

            switch (componentCount)
            {
            case 1:
                temp[0] = T(x + y + z + mipLevel + arrayLayer);
                break;
            case 2:
                temp[0] = T(x + z + arrayLayer);
                temp[1] = T(y + mipLevel);
                break;
            case 3:
                temp[0] = T(x + mipLevel);
                temp[1] = T(y + arrayLayer);
                temp[2] = T(z);
                break;
            case 4:
                temp[0] = T(x + arrayLayer);
                temp[1] = (T)y;
                temp[2] = (T)z;
                temp[3] = (T)mipLevel;
                break;
            default:
                assert(!"component count should be no greater than 4");
                SLANG_CHECK_ABORT(false);
            }
        }
    };

    template <typename T>
    struct PackedValidationTextureFormat : ValidationTextureFormatBase
    {
        int rBits;
        int gBits;
        int bBits;
        int aBits;

        PackedValidationTextureFormat(int rBits, int gBits, int bBits, int aBits)
            : rBits(rBits), gBits(gBits), bBits(bBits), aBits(aBits) {};

        virtual void validateBlocksEqual(const void* actual, const void* expected) override
        {
            T a[4];
            T e[4];
            unpackTexel(*(const T*)actual, a);
            unpackTexel(*(const T*)expected, e);

            for (Int i = 0; i < 4; ++i)
            {
                SLANG_CHECK(a[i] == e[i]);
            }
        }

        virtual void initializeTexel(void* texel, int x, int y, int z, int mipLevel, int arrayLayer) override
        {
            T temp = 0;

            // The only formats which currently use this have either 3 or 4 channels. TODO: BC formats?
            if (aBits == 0)
            {
                temp |= z;
                temp <<= gBits;
                temp |= (y + arrayLayer);
                temp <<= rBits;
                temp |= (x + mipLevel);
            }
            else
            {
                temp |= mipLevel;
                temp <<= bBits;
                temp |= z;
                temp <<= gBits;
                temp |= y;
                temp <<= rBits;
                temp |= (x + arrayLayer);
            }

            *(T*)texel = temp;
        }

        void unpackTexel(T texel, T* outComponents)
        {
            outComponents[0] = texel & ((1 << rBits) - 1);
            texel >>= rBits;

            outComponents[1] = texel & ((1 << gBits) - 1);
            texel >>= gBits;

            outComponents[2] = texel & ((1 << bBits) - 1);
            texel >>= bBits;

            outComponents[3] = texel & ((1 << aBits) - 1);
            texel >>= aBits;

            //for ()
        }
    };

    struct ValidationTextureData : RefObject
    {
        const void* textureData;
        ITextureResource::Size extents;
        ITextureResource::Offset3D strides;

        RefPtr<ValidationTextureFormatBase> format;

        void* getBlockAt(Int x, Int y, Int z)
        {
            assert(x >= 0 && x < extents.width);
            assert(y >= 0 && y < extents.height);
            assert(z >= 0 && z < extents.depth);

            char* layerData = (char*)textureData + z * strides.z;
            char* rowData = layerData + y * strides.y;
            return rowData + x * strides.x;
        }
    };

    struct TextureStuff : RefObject
    {
        List<RefPtr<ValidationTextureData>> subresourceObjects;
        List<ITextureResource::SubresourceData> subresourceDatas;
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

        RefPtr<TextureStuff> generateTextureData(int width, int height, uint32_t mipLevels, uint32_t arrayLayers, Format format)
        {
            FormatInfo formatInfo;
            gfxGetFormatInfo(format, &formatInfo);

            RefPtr<TextureStuff> texture = new TextureStuff();
            for (uint32_t layer = 0; layer < arrayLayers; ++layer)
            {
                for (uint32_t mip = 0; mip < mipLevels; ++mip)
                {
                    RefPtr<ValidationTextureData> subresource = new ValidationTextureData();

                    auto mipWidth = Math::Max(width >> mip, 1);
                    auto mipHeight = Math::Max(height >> mip, 1);
                    auto texelSize = formatInfo.blockSizeInBytes / formatInfo.pixelsPerBlock;
                    auto mipSize = mipWidth * mipHeight * texelSize;
                    subresource->textureData = malloc(mipSize);
                    SLANG_CHECK_ABORT(subresource->textureData);

                    subresource->extents.width = width;
                    subresource->extents.height = height;
                    subresource->extents.depth = 1; // This should be passed in for Texture3Ds
                    subresource->strides.x = texelSize;
                    subresource->strides.y = mipWidth * texelSize;
                    subresource->strides.z = mipHeight * subresource->strides.y;
                    subresource->format = getValidationTextureFormat(format);
                    texture->subresourceObjects.add(subresource);

                    for (int h = 0; h < mipHeight; ++h)
                    {
                        for (int w = 0; w < mipWidth; ++w)
                        {
                            auto texel = subresource->getBlockAt(w, h, 0);
                            subresource->format->initializeTexel(texel, w, h, 0, mip, layer);
                        }
                    }

                    ITextureResource::SubresourceData subData = {};
                    subData.data = subresource->textureData;
                    subData.strideY = subresource->strides.y;
                    subData.strideZ = subresource->strides.z;
                    texture->subresourceDatas.add(subData);
                }
            }
            return texture;
        }

        void createRequiredResources(TextureInfo srcTextureInfo, TextureInfo dstTextureInfo, ITextureResource::Size bufferCopyExtents, Format format)
        {
            ITextureResource::Desc srcTexDesc = {};
            srcTexDesc.type = IResource::Type::Texture2D;
            srcTexDesc.numMipLevels = srcTextureInfo.numMipLevels;
            srcTexDesc.arraySize = srcTextureInfo.arraySize;
            srcTexDesc.size = srcTextureInfo.extent;
            srcTexDesc.defaultState = ResourceState::ShaderResource;
            srcTexDesc.allowedStates = ResourceStateSet(
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
            alignedRowStride = (bufferCopyExtents.width * formatInfo.blockSizeInBytes + alignment - 1) & ~(alignment - 1);
            IBufferResource::Desc bufferDesc = {};
            bufferDesc.sizeInBytes = bufferCopyExtents.height * alignedRowStride;
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
            ITextureResource::Size texCopyExtent,
            ITextureResource::Size bufferCopyExtent,
            size_t minBufferSize)
        {
            ITextureResource::Offset3D bufferCopyOffset; // This is the offset into the texture being copied into the buffer. TODO: Add a separate test for this?

            Slang::ComPtr<ITransientResourceHeap> transientHeap;
            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeResourceCommands();

            encoder->textureSubresourceBarrier(srcTexture, srcSubresource, ResourceState::ShaderResource, ResourceState::CopySource);
            encoder->copyTexture(dstTexture, ResourceState::CopyDestination, dstSubresource, dstOffset, srcTexture, ResourceState::CopySource, srcSubresource, srcOffset, texCopyExtent);
            encoder->textureSubresourceBarrier(dstTexture, dstSubresource, ResourceState::CopyDestination, ResourceState::CopySource);
            encoder->copyTextureToBuffer(resultsBuffer, 0, minBufferSize, alignedRowStride, dstTexture, ResourceState::CopySource, dstSubresource, bufferCopyOffset, bufferCopyExtent);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        // should return refptr to avoid leaking memory
        RefPtr<ValidationTextureFormatBase> getValidationTextureFormat(Format format)
        {
            switch (format)
            {
            case Format::R32G32B32A32_TYPELESS:         return new ValidationTextureFormat<uint32_t>(4);
            case Format::R32G32B32_TYPELESS:            return new ValidationTextureFormat<uint32_t>(3);
            case Format::R32G32_TYPELESS:               return new ValidationTextureFormat<uint32_t>(2);
            case Format::R32_TYPELESS:                  return new ValidationTextureFormat<uint32_t>(1);

            case Format::R16G16B16A16_TYPELESS:         return new ValidationTextureFormat<uint16_t>(4);
            case Format::R16G16_TYPELESS:               return new ValidationTextureFormat<uint16_t>(2);
            case Format::R16_TYPELESS:                  return new ValidationTextureFormat<uint16_t>(1);

            case Format::R8G8B8A8_TYPELESS:             return new ValidationTextureFormat<uint8_t>(4);
            case Format::R8G8_TYPELESS:                 return new ValidationTextureFormat<uint8_t>(2);
            case Format::R8_TYPELESS:                   return new ValidationTextureFormat<uint8_t>(1);
            case Format::B8G8R8A8_TYPELESS:             return new ValidationTextureFormat<uint8_t>(4);

            case Format::R32G32B32A32_FLOAT:            return new ValidationTextureFormat<float>(4);
            case Format::R32G32B32_FLOAT:               return new ValidationTextureFormat<float>(3);
            case Format::R32G32_FLOAT:                  return new ValidationTextureFormat<float>(2);
            case Format::R32_FLOAT:                     return new ValidationTextureFormat<float>(1);

//                     R16G16B16A16_FLOAT,
//                     R16G16_FLOAT,
//                     R16_FLOAT,
 
            case Format::R32G32B32A32_UINT:             return new ValidationTextureFormat<uint32_t>(4);
            case Format::R32G32B32_UINT:                return new ValidationTextureFormat<uint32_t>(3);
            case Format::R32G32_UINT:                   return new ValidationTextureFormat<uint32_t>(2);
            case Format::R32_UINT:                      return new ValidationTextureFormat<uint32_t>(1);

            case Format::R16G16B16A16_UINT:             return new ValidationTextureFormat<uint16_t>(4);
            case Format::R16G16_UINT:                   return new ValidationTextureFormat<uint16_t>(2);
            case Format::R16_UINT:                      return new ValidationTextureFormat<uint16_t>(1);

            case Format::R8G8B8A8_UINT:                 return new ValidationTextureFormat<uint8_t>(4);
            case Format::R8G8_UINT:                     return new ValidationTextureFormat<uint8_t>(2);
            case Format::R8_UINT:                       return new ValidationTextureFormat<uint8_t>(1);

            case Format::R32G32B32A32_SINT:             return new ValidationTextureFormat<int32_t>(4);
            case Format::R32G32B32_SINT:                return new ValidationTextureFormat<int32_t>(3);
            case Format::R32G32_SINT:                   return new ValidationTextureFormat<int32_t>(2);
            case Format::R32_SINT:                      return new ValidationTextureFormat<int32_t>(1);

            case Format::R16G16B16A16_SINT:             return new ValidationTextureFormat<int16_t>(4);
            case Format::R16G16_SINT:                   return new ValidationTextureFormat<int16_t>(2);
            case Format::R16_SINT:                      return new ValidationTextureFormat<int16_t>(1);

            case Format::R8G8B8A8_SINT:                 return new ValidationTextureFormat<int8_t>(4);
            case Format::R8G8_SINT:                     return new ValidationTextureFormat<int8_t>(2);
            case Format::R8_SINT:                       return new ValidationTextureFormat<int8_t>(1);

            case Format::R16G16B16A16_UNORM:            return new ValidationTextureFormat<uint16_t>(4);
            case Format::R16G16_UNORM:                  return new ValidationTextureFormat<uint16_t>(2);
            case Format::R16_UNORM:                     return new ValidationTextureFormat<uint16_t>(1);

            case Format::R8G8B8A8_UNORM:                return new ValidationTextureFormat<uint8_t>(4);
            case Format::R8G8B8A8_UNORM_SRGB:           return new ValidationTextureFormat<uint8_t>(4);
            case Format::R8G8_UNORM:                    return new ValidationTextureFormat<uint8_t>(2);
            case Format::R8_UNORM:                      return new ValidationTextureFormat<uint8_t>(1);
            case Format::B8G8R8A8_UNORM:                return new ValidationTextureFormat<uint8_t>(4);
            case Format::B8G8R8A8_UNORM_SRGB:           return new ValidationTextureFormat<uint8_t>(4);
            case Format::B8G8R8X8_UNORM:                return new ValidationTextureFormat<uint8_t>(3); // 3 or 4 channels?
            case Format::B8G8R8X8_UNORM_SRGB:           return new ValidationTextureFormat<uint8_t>(3);

            case Format::R16G16B16A16_SNORM:            return new ValidationTextureFormat<int16_t>(4);
            case Format::R16G16_SNORM:                  return new ValidationTextureFormat<int16_t>(2);
            case Format::R16_SNORM:                     return new ValidationTextureFormat<int16_t>(1);

            case Format::R8G8B8A8_SNORM:                return new ValidationTextureFormat<int8_t>(4);
            case Format::R8G8_SNORM:                    return new ValidationTextureFormat<int8_t>(2);
            case Format::R8_SNORM:                      return new ValidationTextureFormat<int8_t>(1);

            case Format::D32_FLOAT:                     return new ValidationTextureFormat<float>(1); // broken in VK
            case Format::D16_UNORM:                     return new ValidationTextureFormat<uint16_t>(1); // broken in VK

            case Format::B4G4R4A4_UNORM:                return new PackedValidationTextureFormat<uint16_t>(4, 4, 4, 4);
            case Format::B5G6R5_UNORM:                  return new PackedValidationTextureFormat<uint16_t>(5, 6, 5, 0);
            case Format::B5G5R5A1_UNORM:                return new PackedValidationTextureFormat<uint16_t>(5, 5, 5, 1);

//                     R9G9B9E5_SHAREDEXP,
            case Format::R10G10B10A2_TYPELESS:          return new PackedValidationTextureFormat<uint32_t>(10, 10, 10, 2);
            case Format::R10G10B10A2_UNORM:             return new PackedValidationTextureFormat<uint32_t>(10, 10, 10, 2);
            case Format::R10G10B10A2_UINT:              return new PackedValidationTextureFormat<uint32_t>(10, 10, 10, 2);
            case Format::R11G11B10_FLOAT:               return new PackedValidationTextureFormat<uint32_t>(11, 11, 10, 0);

//                     BC1_UNORM,
//                     BC1_UNORM_SRGB,
//                     BC2_UNORM,
//                     BC2_UNORM_SRGB,
//                     BC3_UNORM,
//                     BC3_UNORM_SRGB,
//                     BC4_UNORM,
//                     BC4_SNORM,
//                     BC5_UNORM,
//                     BC5_SNORM,
//                     BC6H_UF16,
//                     BC6H_SF16,
//                     BC7_UNORM,
//                     BC7_UNORM_SRGB,
            default:
                SLANG_CHECK_ABORT(false);
                return nullptr;
            }
        }

        bool isWithinCopyBounds(int x, int y, int z, ITextureResource::Offset3D copyOffset, ITextureResource::Size copyExtents)
        {
            auto xLowerBound = copyOffset.x;
            auto xUpperBound = copyOffset.x + copyExtents.width;
            auto yLowerBound = copyOffset.y;
            auto yUpperBound = copyOffset.y + copyExtents.height;
            auto zLowerBound = copyOffset.z;
            auto zUpperBound = copyOffset.z + copyExtents.depth;

            if (x < xLowerBound || x >= xUpperBound || y < yLowerBound || y >= yUpperBound || z < zLowerBound || z >= zUpperBound) return false;
            else return true;
        }

        void validateTestResults(
            ValidationTextureData actual,
            ValidationTextureData expectedCopied,
            ValidationTextureData expectedOriginal,
            ITextureResource::Size copyExtent,
            ITextureResource::Offset3D srcTexOffset,
            ITextureResource::Offset3D dstTexOffset)
        {
            auto actualExtents = actual.extents;
            auto format = actual.format;

            for (Int x = 0; x < actualExtents.width; ++x)
            {
                for (Int y = 0; y < actualExtents.height; ++y)
                {
                    for (Int z = 0; z < actualExtents.depth; ++z)
                    {
                        auto actualBlock = actual.getBlockAt(x, y, z);
                        if (isWithinCopyBounds(x, y, z, dstTexOffset, copyExtent))
                        {
                            // Block is located within the bounds of the source texture
                            auto xSource = x + srcTexOffset.x - dstTexOffset.x;
                            auto ySource = y + srcTexOffset.y - dstTexOffset.y;
                            auto zSource = z + srcTexOffset.z - dstTexOffset.z;
                            auto expectedBlock = expectedCopied.getBlockAt(xSource, ySource, zSource);
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
            ITextureResource::Offset3D srcTexOffset,
            ITextureResource::Offset3D dstTexOffset,
            Format format,
            const void* expectedCopiedData,
            const void* expectedOriginalData)
        {
            ComPtr<ISlangBlob> resultBlob;
            auto resultsSize = dstExtent.depth * dstExtent.height * alignedRowStride;
            GFX_CHECK_CALL_ABORT(device->readBufferResource(resultsBuffer, 0, resultsSize, resultBlob.writeRef()));
//             size_t outRowPitch;
//             size_t outPixelSize;
//             GFX_CHECK_CALL_ABORT(device->readTextureResource(srcTexture, ResourceState::CopySource, resultBlob.writeRef(), &outRowPitch, &outPixelSize));
            auto results = resultBlob->getBufferPointer();

            auto validationFormat = getValidationTextureFormat(format);

            FormatInfo formatInfo;
            gfxGetFormatInfo(format, &formatInfo);
            auto texelSize = formatInfo.blockSizeInBytes / formatInfo.pixelsPerBlock;

            ValidationTextureData actual;
            actual.extents = dstExtent;
            actual.format = validationFormat;
            actual.textureData = results;
            actual.strides.x = texelSize;
            actual.strides.y = (uint32_t)alignedRowStride;
            actual.strides.z = dstExtent.height * actual.strides.y;

            ValidationTextureData expectedCopied;
            expectedCopied.extents = srcExtent;
            expectedCopied.format = validationFormat;
            expectedCopied.textureData = expectedCopiedData;
            expectedCopied.strides.x = texelSize;
            expectedCopied.strides.y = srcExtent.width * expectedCopied.strides.x;
            expectedCopied.strides.z = srcExtent.height * expectedCopied.strides.y;

            ValidationTextureData expectedOriginal;
            if (expectedOriginalData)
            {
                expectedOriginal.extents = dstExtent;
                expectedOriginal.format = validationFormat;
                expectedOriginal.textureData = expectedOriginalData;
                expectedOriginal.strides.x = texelSize;
                expectedOriginal.strides.y = dstExtent.width * expectedOriginal.strides.x;
                expectedOriginal.strides.z = dstExtent.height * expectedOriginal.strides.y;
            }

            validateTestResults(actual, expectedCopied, expectedOriginal, copyExtent, srcTexOffset, dstTexOffset);
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
        void run()
        {
            // Skip Format::Unknown
            for (uint32_t i = 1; i < (uint32_t)Format::CountOf; ++i)
            {
                if (i == 16 || i == 17 || i == 18 || i == 61 || i >= 66)
                {
                    continue;
                }

                //if (i == 56 || i == 57) continue;

                printf("%d", i);
                auto format = (Format)i;

                ITextureResource::Size extent = {};
                extent.width = 4;
                extent.height = 4;
                extent.depth = 1;
                auto mipLevelCount = 1;
                auto arrayLayerCount = 1;

                auto srcTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount, format);

                TextureInfo srcTextureInfo = { extent, mipLevelCount, arrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };
                TextureInfo dstTextureInfo = { extent, mipLevelCount, arrayLayerCount, nullptr };

                createRequiredResources(srcTextureInfo, dstTextureInfo, extent, format);

//                 ComPtr<ISlangBlob> resultBlob;
//                 size_t outRowPitch;
//                 size_t outPixelSize;
//                 GFX_CHECK_CALL_ABORT(device->readTextureResource(srcTexture, ResourceState::CopySource, resultBlob.writeRef(), &outRowPitch, &outPixelSize));
//                 auto results = resultBlob->getBufferPointer();

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

                submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, extent, extent, 16);

                auto expectedData = srcTextureStuff->subresourceDatas[0];
                checkTestResults(extent, extent, extent, srcOffset, dstOffset, format, expectedData.data, nullptr);
            }
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

            auto srcTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount, format);
            
            TextureInfo srcTextureInfo = { extent, mipLevelCount, arrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };
            TextureInfo dstTextureInfo = { extent, mipLevelCount, arrayLayerCount, nullptr };

            createRequiredResources(srcTextureInfo, dstTextureInfo, extent, format);

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

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, extent, extent, extent.height * alignedRowStride);

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[2];
            checkTestResults(extent, extent, extent, srcOffset, dstOffset, format, expectedData.data, nullptr);
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

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount, format);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 4;
            dstExtent.height = 4;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

            createRequiredResources(srcTextureInfo, dstTextureInfo, dstExtent, format);

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

            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, dstExtent, dstExtent, dstExtent.height * alignedRowStride);

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, dstExtent, dstExtent, srcOffset, dstOffset, format, expectedData.data, nullptr);
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

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount, format);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 8;
            dstExtent.height = 8;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount, format);
            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

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

            ITextureResource::Size texCopyExtent = srcExtent;
            ITextureResource::Size bufferCopyExtent = dstExtent;

            createRequiredResources(srcTextureInfo, dstTextureInfo, bufferCopyExtent, format);
            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, texCopyExtent, bufferCopyExtent, dstExtent.height * alignedRowStride);

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, bufferCopyExtent, texCopyExtent, srcOffset, dstOffset, format, expectedCopiedData.data, expectedOriginalData.data);
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

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount, format);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 16;
            dstExtent.height = 16;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 4;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount, format);
            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

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
            ITextureResource::Size texCopyExtent;
            texCopyExtent.width = 4;
            texCopyExtent.height = 4;
            texCopyExtent.depth = 1;

            // These are the extents of the mip layer being copied to (and which will be copied into the results buffer).
            ITextureResource::Size bufferCopyExtent;
            bufferCopyExtent.width = 8;
            bufferCopyExtent.height = 8;
            bufferCopyExtent.depth = 1;

            createRequiredResources(srcTextureInfo, dstTextureInfo, bufferCopyExtent, format);
            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, texCopyExtent, bufferCopyExtent, bufferCopyExtent.height * alignedRowStride);

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[2];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[1];
            checkTestResults(texCopyExtent, bufferCopyExtent, texCopyExtent, srcOffset, dstOffset, format, expectedCopiedData.data, expectedOriginalData.data);
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

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount, format);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 4;
            dstExtent.height = 4;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 2;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount, format);
            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

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

            createRequiredResources(srcTextureInfo, dstTextureInfo, copyExtent, format);
            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, copyExtent, copyExtent, copyExtent.height * alignedRowStride);

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[1];
            checkTestResults(srcExtent, copyExtent, copyExtent, srcOffset, dstOffset, format, expectedCopiedData.data, expectedOriginalData.data);
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

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount, format);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 16;
            dstExtent.height = 16;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount, format);
            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = TextureAspect::Color;
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            ITextureResource::Offset3D srcOffset;
            srcOffset.x = 2;
            srcOffset.y = 2;
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

            ITextureResource::Size texCopyExtent;
            texCopyExtent.width = 4;
            texCopyExtent.height = 4;
            texCopyExtent.depth = 1;

            ITextureResource::Size bufferCopyExtent;
            bufferCopyExtent.width = 16;
            bufferCopyExtent.height = 16;
            bufferCopyExtent.depth = 1;

            createRequiredResources(srcTextureInfo, dstTextureInfo, dstExtent, format);
            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, texCopyExtent, bufferCopyExtent, bufferCopyExtent.height * alignedRowStride);

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, bufferCopyExtent, texCopyExtent, srcOffset, dstOffset, format, expectedCopiedData.data, expectedOriginalData.data);
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

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount, format);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 8;
            dstExtent.height = 8;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount, format);
            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

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

            createRequiredResources(srcTextureInfo, dstTextureInfo, dstExtent, format);
            submitGPUWork(srcSubresource, dstSubresource, srcOffset, dstOffset, copyExtent, dstExtent, dstExtent.height * alignedRowStride);

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, dstExtent, copyExtent, srcOffset, dstOffset, format, expectedCopiedData.data, expectedOriginalData.data);
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

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount, format);
            TextureInfo srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 4;
            dstExtent.height = 4;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount, format);
            TextureInfo dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

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

            ITextureResource::Offset3D dstOffset;

            createRequiredResources(srcTextureInfo, dstTextureInfo, dstExtent, format);

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

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, dstExtent, copyExtent, srcOffset, dstOffset, format, expectedCopiedData.data, expectedOriginalData.data);
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
        //runTestImpl(copyTextureTestImpl<SimpleCopyTexture>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<SimpleCopyTexture>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopyTextureSection>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyTextureSection>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<SmallSrcToLargeDst>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<SmallSrcToLargeDst>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopyBetweenMips>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyBetweenMips>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopyBetweenLayers>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyBetweenLayers>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopyWithOffsets>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyWithOffsets>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopySectionWithSetExtent>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopySectionWithSetExtent>, unitTestContext, Slang::RenderApiFlag::Vulkan);

//         runTestImpl(copyTextureTestImpl<CopyToBufferWithOffset>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyToBufferWithOffset>, unitTestContext, Slang::RenderApiFlag::Vulkan);
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
