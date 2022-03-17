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

        case Format::R16G16B16A16_FLOAT:            return new ValidationTextureFormat<uint16_t>(4);
        case Format::R16G16_FLOAT:                  return new ValidationTextureFormat<uint16_t>(2);
        case Format::R16_FLOAT:                     return new ValidationTextureFormat<uint16_t>(1);

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
        case Format::B8G8R8X8_UNORM:                return new ValidationTextureFormat<uint8_t>(3);
        case Format::B8G8R8X8_UNORM_SRGB:           return new ValidationTextureFormat<uint8_t>(3);

        case Format::R16G16B16A16_SNORM:            return new ValidationTextureFormat<int16_t>(4);
        case Format::R16G16_SNORM:                  return new ValidationTextureFormat<int16_t>(2);
        case Format::R16_SNORM:                     return new ValidationTextureFormat<int16_t>(1);

        case Format::R8G8B8A8_SNORM:                return new ValidationTextureFormat<int8_t>(4);
        case Format::R8G8_SNORM:                    return new ValidationTextureFormat<int8_t>(2);
        case Format::R8_SNORM:                      return new ValidationTextureFormat<int8_t>(1);

        case Format::D32_FLOAT:                     return new ValidationTextureFormat<float>(1);
        case Format::D16_UNORM:                     return new ValidationTextureFormat<uint16_t>(1);

        case Format::B4G4R4A4_UNORM:                return new PackedValidationTextureFormat<uint16_t>(4, 4, 4, 4);
        case Format::B5G6R5_UNORM:                  return new PackedValidationTextureFormat<uint16_t>(5, 6, 5, 0);
        case Format::B5G5R5A1_UNORM:                return new PackedValidationTextureFormat<uint16_t>(5, 5, 5, 1);

        case Format::R9G9B9E5_SHAREDEXP:            return new ValidationTextureFormat<uint32_t>(1);
        case Format::R10G10B10A2_TYPELESS:          return new PackedValidationTextureFormat<uint32_t>(10, 10, 10, 2);
        case Format::R10G10B10A2_UNORM:             return new PackedValidationTextureFormat<uint32_t>(10, 10, 10, 2);
        case Format::R10G10B10A2_UINT:              return new PackedValidationTextureFormat<uint32_t>(10, 10, 10, 2);
        case Format::R11G11B10_FLOAT:               return new PackedValidationTextureFormat<uint32_t>(11, 11, 10, 0);

            // TODO: Add testing support for BC formats
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
            return nullptr;
        }
    }

    struct TextureInfo
    {
        ITextureResource::Size extent;
        int numMipLevels;
        int arraySize;
        ITextureResource::SubresourceData const* initData;
    };

    struct ValidationTextureData : RefObject
    {
        const void* textureData;
        ITextureResource::Size extents;
        ITextureResource::Offset3D strides;

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

    struct TextureToTextureCopyInfo
    {
        SubresourceRange srcSubresource;
        SubresourceRange dstSubresource;
        ITextureResource::Size extent;
        ITextureResource::Offset3D srcOffset;
        ITextureResource::Offset3D dstOffset;
    };

    struct TextureToBufferCopyInfo
    {
        SubresourceRange srcSubresource;
        ITextureResource::Size extent;
        ITextureResource::Offset3D textureOffset;
        size_t bufferOffset;
        size_t bufferSize;
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

        Format format;
        uint32_t texelSize;
        size_t alignedRowStride;

        TextureInfo srcTextureInfo;
        TextureInfo dstTextureInfo;
        TextureToTextureCopyInfo texCopyInfo;
        TextureToBufferCopyInfo bufferCopyInfo;

        ComPtr<ITextureResource> srcTexture;
        ComPtr<ITextureResource> dstTexture;
        ComPtr<IBufferResource> resultsBuffer;

        RefPtr<ValidationTextureFormatBase> validationFormat;
        
        void init(IDevice* device, UnitTestContext* context, Format format, RefPtr<ValidationTextureFormatBase> validationFormat)
        {
            this->device = device;
            this->context = context;
            this->format = format;
            this->validationFormat = validationFormat;

            FormatInfo formatInfo;
            GFX_CHECK_CALL_ABORT(gfxGetFormatInfo(format, &formatInfo));
            this->texelSize = formatInfo.blockSizeInBytes / formatInfo.pixelsPerBlock;
        }

        RefPtr<TextureStuff> generateTextureData(int width, int height, uint32_t mipLevels, uint32_t arrayLayers)
        {
            RefPtr<TextureStuff> texture = new TextureStuff();
            for (uint32_t layer = 0; layer < arrayLayers; ++layer)
            {
                for (uint32_t mip = 0; mip < mipLevels; ++mip)
                {
                    RefPtr<ValidationTextureData> subresource = new ValidationTextureData();

                    auto mipWidth = Math::Max(width >> mip, 1);
                    auto mipHeight = Math::Max(height >> mip, 1);
                    auto mipSize = mipWidth * mipHeight * texelSize;
                    subresource->textureData = malloc(mipSize);
                    SLANG_CHECK_ABORT(subresource->textureData);

                    subresource->extents.width = mipWidth;
                    subresource->extents.height = mipHeight;
                    subresource->extents.depth = 1; // This should be passed in for Texture3Ds
                    subresource->strides.x = texelSize;
                    subresource->strides.y = mipWidth * texelSize;
                    subresource->strides.z = mipHeight * subresource->strides.y;
                    texture->subresourceObjects.add(subresource);

                    for (int h = 0; h < mipHeight; ++h)
                    {
                        for (int w = 0; w < mipWidth; ++w)
                        {
                            auto texel = subresource->getBlockAt(w, h, 0);
                            validationFormat->initializeTexel(texel, w, h, 0, mip, layer);
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

        TextureAspect getTextureAspect()
        {
            switch (format)
            {
            case Format::D16_UNORM:
            case Format::D32_FLOAT:
                return TextureAspect::Depth;
            default:
                return TextureAspect::Color;
            }
        }

        void createRequiredResources()
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
            if (format == Format::D32_FLOAT || format == Format::D16_UNORM)
            {
                srcTexDesc.allowedStates.add(ResourceState::DepthWrite);
                srcTexDesc.allowedStates.add(ResourceState::DepthRead);
            }
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
            if (format == Format::D32_FLOAT || format == Format::D16_UNORM)
            {
                srcTexDesc.allowedStates.add(ResourceState::DepthWrite);
                srcTexDesc.allowedStates.add(ResourceState::DepthRead);
            }
            dstTexDesc.format = format;

            GFX_CHECK_CALL_ABORT(device->createTextureResource(
                dstTexDesc,
                dstTextureInfo.initData,
                dstTexture.writeRef()));

            auto bufferCopyExtents = bufferCopyInfo.extent;
            size_t alignment;
            device->getTextureRowAlignment(&alignment);
            alignedRowStride = (bufferCopyExtents.width * texelSize + alignment - 1) & ~(alignment - 1);
            IBufferResource::Desc bufferDesc = {};
            bufferDesc.sizeInBytes = bufferCopyExtents.height * alignedRowStride;
            bufferDesc.format = gfx::Format::Unknown;
            bufferDesc.elementSize = 0;
            bufferDesc.allowedStates = ResourceStateSet(
                ResourceState::ShaderResource,
                ResourceState::UnorderedAccess,
                ResourceState::CopyDestination,
                ResourceState::CopySource);
            if (format == Format::D32_FLOAT || format == Format::D16_UNORM)
            {
                srcTexDesc.allowedStates.add(ResourceState::DepthWrite);
                srcTexDesc.allowedStates.add(ResourceState::DepthRead);
            }
            bufferDesc.defaultState = ResourceState::CopyDestination;
            bufferDesc.memoryType = MemoryType::DeviceLocal;

            GFX_CHECK_CALL_ABORT(device->createBufferResource(
                bufferDesc,
                nullptr,
                resultsBuffer.writeRef()));

            bufferCopyInfo.bufferSize = bufferDesc.sizeInBytes;
        }

        void submitGPUWork()
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

            encoder->textureSubresourceBarrier(srcTexture, texCopyInfo.srcSubresource, ResourceState::ShaderResource, ResourceState::CopySource);
            encoder->copyTexture(
                dstTexture,
                ResourceState::CopyDestination,
                texCopyInfo.dstSubresource,
                texCopyInfo.dstOffset,
                srcTexture,
                ResourceState::CopySource,
                texCopyInfo.srcSubresource,
                texCopyInfo.srcOffset,
                texCopyInfo.extent);

            encoder->textureSubresourceBarrier(dstTexture, bufferCopyInfo.srcSubresource, ResourceState::CopyDestination, ResourceState::CopySource);
            encoder->copyTextureToBuffer(
                resultsBuffer,
                bufferCopyInfo.bufferOffset,
                bufferCopyInfo.bufferSize,
                alignedRowStride,
                dstTexture,
                ResourceState::CopySource,
                bufferCopyInfo.srcSubresource,
                bufferCopyInfo.textureOffset,
                bufferCopyInfo.extent);

            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        bool isWithinCopyBounds(int x, int y, int z)
        {
            auto copyExtents = texCopyInfo.extent;
            auto copyOffset = texCopyInfo.dstOffset;

            auto xLowerBound = copyOffset.x;
            auto xUpperBound = copyOffset.x + copyExtents.width;
            auto yLowerBound = copyOffset.y;
            auto yUpperBound = copyOffset.y + copyExtents.height;
            auto zLowerBound = copyOffset.z;
            auto zUpperBound = copyOffset.z + copyExtents.depth;

            if (x < xLowerBound || x >= xUpperBound || y < yLowerBound || y >= yUpperBound || z < zLowerBound || z >= zUpperBound)
                return false;
            else
                return true;
        }

        void validateTestResults(
            ValidationTextureData actual,
            ValidationTextureData expectedCopied,
            ValidationTextureData expectedOriginal)
        {
            auto actualExtents = actual.extents;
            auto copyExtent = texCopyInfo.extent;
            auto srcTexOffset = texCopyInfo.srcOffset;
            auto dstTexOffset = texCopyInfo.dstOffset;

            for (Int x = 0; x < actualExtents.width; ++x)
            {
                for (Int y = 0; y < actualExtents.height; ++y)
                {
                    for (Int z = 0; z < actualExtents.depth; ++z)
                    {
                        auto actualBlock = actual.getBlockAt(x, y, z);
                        if (isWithinCopyBounds(x, y, z))
                        {
                            // Block is located within the bounds of the source texture
                            auto xSource = x + srcTexOffset.x - dstTexOffset.x;
                            auto ySource = y + srcTexOffset.y - dstTexOffset.y;
                            auto zSource = z + srcTexOffset.z - dstTexOffset.z;
                            auto expectedBlock = expectedCopied.getBlockAt(xSource, ySource, zSource);
                            validationFormat->validateBlocksEqual(actualBlock, expectedBlock);
                        }
                        else
                        {
                            // Block is located outside the bounds of the source texture and should be compared
                            // against known expected values for the destination texture.
                            auto expectedBlock = expectedOriginal.getBlockAt(x, y, z);
                            validationFormat->validateBlocksEqual(actualBlock, expectedBlock);
                        }
                    }
                }
            }
        }

        void checkTestResults(ITextureResource::Size srcExtent, const void* expectedCopiedData, const void* expectedOriginalData)
        {
            ComPtr<ISlangBlob> resultBlob;
            GFX_CHECK_CALL_ABORT(device->readBufferResource(resultsBuffer, 0, bufferCopyInfo.bufferSize, resultBlob.writeRef()));
            auto results = resultBlob->getBufferPointer();

            ValidationTextureData actual;
            actual.extents = bufferCopyInfo.extent;
            actual.textureData = results;
            actual.strides.x = texelSize;
            actual.strides.y = (uint32_t)alignedRowStride;
            actual.strides.z = actual.extents.height * actual.strides.y;

            ValidationTextureData expectedCopied;
            expectedCopied.extents = srcExtent;
            expectedCopied.textureData = expectedCopiedData;
            expectedCopied.strides.x = texelSize;
            expectedCopied.strides.y = expectedCopied.extents.width * expectedCopied.strides.x;
            expectedCopied.strides.z = expectedCopied.extents.height * expectedCopied.strides.y;

            ValidationTextureData expectedOriginal;
            if (expectedOriginalData)
            {
                expectedOriginal.extents = bufferCopyInfo.extent;
                expectedOriginal.textureData = expectedOriginalData;
                expectedOriginal.strides.x = texelSize;
                expectedOriginal.strides.y = expectedOriginal.extents.width * expectedOriginal.strides.x;
                expectedOriginal.strides.z = expectedOriginal.extents.height * expectedOriginal.strides.y;
            }

            validateTestResults(actual, expectedCopied, expectedOriginal);
        }
    };

    struct SimpleCopyTexture : BaseCopyTextureTest
    {
        void run()
        {
            ITextureResource::Size extent = {};
            extent.width = 4;
            extent.height = 4;
            extent.depth = 1;
            auto mipLevelCount = 1;
            auto arrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount);

            srcTextureInfo = { extent, mipLevelCount, arrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };
            dstTextureInfo = { extent, mipLevelCount, arrayLayerCount, nullptr };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = getTextureAspect();
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = getTextureAspect();
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            texCopyInfo.srcSubresource = srcSubresource;
            texCopyInfo.dstSubresource = dstSubresource;
            texCopyInfo.extent = extent;
            texCopyInfo.srcOffset = { 0, 0, 0 };
            texCopyInfo.dstOffset = { 0, 0, 0 };

            bufferCopyInfo.srcSubresource = dstSubresource;
            bufferCopyInfo.extent = extent;
            bufferCopyInfo.textureOffset = { 0, 0, 0 };
            bufferCopyInfo.bufferOffset = 0;

            createRequiredResources();
            submitGPUWork();

            auto expectedData = srcTextureStuff->subresourceDatas[0];
            checkTestResults(extent, expectedData.data, nullptr);
        }
    };

    struct CopyTextureSection : BaseCopyTextureTest
    {
        void run()
        {
            ITextureResource::Size extent = {};
            extent.width = 4;
            extent.height = 4;
            extent.depth = 1;
            auto mipLevelCount = 2;
            auto arrayLayerCount = 2;

            auto srcTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount);
            
            srcTextureInfo = { extent, mipLevelCount, arrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };
            dstTextureInfo = { extent, mipLevelCount, arrayLayerCount, nullptr };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = getTextureAspect();
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 1;
            srcSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = getTextureAspect();
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            texCopyInfo.srcSubresource = srcSubresource;
            texCopyInfo.dstSubresource = dstSubresource;
            texCopyInfo.extent = extent;
            texCopyInfo.srcOffset = { 0, 0, 0 };
            texCopyInfo.dstOffset = { 0, 0, 0 };

            bufferCopyInfo.srcSubresource = dstSubresource;
            bufferCopyInfo.extent = extent;
            bufferCopyInfo.textureOffset = { 0, 0, 0 };
            bufferCopyInfo.bufferOffset = 0;

            createRequiredResources();
            submitGPUWork();

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[2];
            checkTestResults(extent, expectedData.data, nullptr);
        }
    };

    struct LargeSrcToSmallDst : BaseCopyTextureTest
    {
        void run()
        {
            ITextureResource::Size srcExtent = {};
            srcExtent.width = 8;
            srcExtent.height = 8;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 4;
            dstExtent.height = 4;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, nullptr };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = getTextureAspect();
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = getTextureAspect();
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            texCopyInfo.srcSubresource = srcSubresource;
            texCopyInfo.dstSubresource = dstSubresource;
            texCopyInfo.extent = dstExtent;
            texCopyInfo.srcOffset = { 0, 0, 0 };
            texCopyInfo.dstOffset = { 0, 0, 0 };

            bufferCopyInfo.srcSubresource = dstSubresource;
            bufferCopyInfo.extent = dstExtent;
            bufferCopyInfo.textureOffset = { 0, 0, 0 };
            bufferCopyInfo.bufferOffset = 0;

            createRequiredResources();
            submitGPUWork();

            ITextureResource::SubresourceData expectedData = srcTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, expectedData.data, nullptr);
        }
    };

    struct SmallSrcToLargeDst : BaseCopyTextureTest
    {
        void run()
        {
            ITextureResource::Size srcExtent = {};
            srcExtent.width = 4;
            srcExtent.height = 4;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 8;
            dstExtent.height = 8;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount);
            dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = getTextureAspect();
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = getTextureAspect();
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            texCopyInfo.srcSubresource = srcSubresource;
            texCopyInfo.dstSubresource = dstSubresource;
            texCopyInfo.extent = srcExtent;
            texCopyInfo.srcOffset = { 0, 0, 0 };
            texCopyInfo.dstOffset = { 0, 0, 0 };

            bufferCopyInfo.srcSubresource = dstSubresource;
            bufferCopyInfo.extent = dstExtent;
            bufferCopyInfo.textureOffset = { 0, 0, 0 };
            bufferCopyInfo.bufferOffset = 0;

            createRequiredResources();
            submitGPUWork();

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, expectedCopiedData.data, expectedOriginalData.data);
        }
    };

    struct CopyBetweenMips : BaseCopyTextureTest
    {
        void run()
        {
            ITextureResource::Size srcExtent = {};
            srcExtent.width = 16;
            srcExtent.height = 16;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 4;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 16;
            dstExtent.height = 16;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 4;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount);
            dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = getTextureAspect();
            srcSubresource.mipLevel = 2;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = getTextureAspect();
            dstSubresource.mipLevel = 1;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            texCopyInfo.srcSubresource = srcSubresource;
            texCopyInfo.dstSubresource = dstSubresource;
            texCopyInfo.extent = { 4, 4, 1 }; // Extents of the mip layer being copied from.
            texCopyInfo.srcOffset = { 0, 0, 0 };
            texCopyInfo.dstOffset = { 0, 0, 0 };

            bufferCopyInfo.srcSubresource = dstSubresource;
            bufferCopyInfo.extent = { 8, 8, 1 }; // Extents of the mip layer being copied to (and which will be copied into the results buffer).
            bufferCopyInfo.textureOffset = { 0, 0, 0 };
            bufferCopyInfo.bufferOffset = 0;

            createRequiredResources();
            submitGPUWork();

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[2];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[1];
            auto srcMipExtent = srcTextureStuff->subresourceObjects[2]->extents;
            checkTestResults(srcMipExtent, expectedCopiedData.data, expectedOriginalData.data);
        }
    };

    struct CopyBetweenLayers : BaseCopyTextureTest
    {
        void run()
        {
            ITextureResource::Size extent = {};
            extent.width = 4;
            extent.height = 4;
            extent.depth = 1;
            auto mipLevelCount = 1;
            auto arrayLayerCount = 2;

            auto srcTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount);
            srcTextureInfo = { extent, mipLevelCount, arrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            auto dstTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount);
            dstTextureInfo = { extent, mipLevelCount, arrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = getTextureAspect();
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = getTextureAspect();
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 1;
            dstSubresource.layerCount = 1;

            texCopyInfo.srcSubresource = srcSubresource;
            texCopyInfo.dstSubresource = dstSubresource;
            texCopyInfo.extent = extent;
            texCopyInfo.srcOffset = { 0, 0, 0 };
            texCopyInfo.dstOffset = { 0, 0, 0 };

            bufferCopyInfo.srcSubresource = dstSubresource;
            bufferCopyInfo.extent = extent;
            bufferCopyInfo.textureOffset = { 0, 0, 0 };
            bufferCopyInfo.bufferOffset = 0;

            createRequiredResources();
            submitGPUWork();

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[1];
            checkTestResults(extent, expectedCopiedData.data, expectedOriginalData.data);
        }
    };

    struct CopyWithOffsets : BaseCopyTextureTest
    {
        void run()
        {
            ITextureResource::Size srcExtent = {};
            srcExtent.width = 8;
            srcExtent.height = 8;
            srcExtent.depth = 1;
            auto srcMipLevelCount = 1;
            auto srcArrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(srcExtent.width, srcExtent.height, srcMipLevelCount, srcArrayLayerCount);
            srcTextureInfo = { srcExtent, srcMipLevelCount, srcArrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            ITextureResource::Size dstExtent = {};
            dstExtent.width = 16;
            dstExtent.height = 16;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount);
            dstTextureInfo = { dstExtent, dstMipLevelCount, dstArrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = getTextureAspect();
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = getTextureAspect();
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            texCopyInfo.srcSubresource = srcSubresource;
            texCopyInfo.dstSubresource = dstSubresource;
            texCopyInfo.extent = { 4, 4, 1 };
            texCopyInfo.srcOffset = { 2, 2, 0 };
            texCopyInfo.dstOffset = { 4, 4, 0 };

            bufferCopyInfo.srcSubresource = dstSubresource;
            bufferCopyInfo.extent = dstExtent;
            bufferCopyInfo.textureOffset = { 0, 0, 0 };
            bufferCopyInfo.bufferOffset = 0;

            createRequiredResources();
            submitGPUWork();

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[0];
            checkTestResults(srcExtent, expectedCopiedData.data, expectedOriginalData.data);
        }
    };

    struct CopySectionWithSetExtent : BaseCopyTextureTest
    {
        void run()
        {
            ITextureResource::Size extent = {};
            extent.width = 8;
            extent.height = 8;
            extent.depth = 1;
            auto mipLevelCount = 1;
            auto arrayLayerCount = 1;

            auto srcTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount);
            srcTextureInfo = { extent, mipLevelCount, arrayLayerCount, srcTextureStuff->subresourceDatas.getBuffer() };

            auto dstTextureStuff = generateTextureData(extent.width, extent.height, mipLevelCount, arrayLayerCount);
            dstTextureInfo = { extent, mipLevelCount, arrayLayerCount, dstTextureStuff->subresourceDatas.getBuffer() };

            SubresourceRange srcSubresource = {};
            srcSubresource.aspectMask = getTextureAspect();
            srcSubresource.mipLevel = 0;
            srcSubresource.mipLevelCount = 1;
            srcSubresource.baseArrayLayer = 0;
            srcSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = getTextureAspect();
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            texCopyInfo.srcSubresource = srcSubresource;
            texCopyInfo.dstSubresource = dstSubresource;
            texCopyInfo.extent = { 4, 4, 1 };
            texCopyInfo.srcOffset = { 0, 0, 0 };
            texCopyInfo.dstOffset = { 4, 4, 0 };

            bufferCopyInfo.srcSubresource = dstSubresource;
            bufferCopyInfo.extent = extent;
            bufferCopyInfo.textureOffset = { 0, 0, 0 };
            bufferCopyInfo.bufferOffset = 0;

            createRequiredResources();
            submitGPUWork();

            auto expectedCopiedData = srcTextureStuff->subresourceDatas[0];
            auto expectedOriginalData = dstTextureStuff->subresourceDatas[0];
            checkTestResults(extent, expectedCopiedData.data, expectedOriginalData.data);
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
            dstExtent.height = 4;
            dstExtent.depth = 1;
            auto dstMipLevelCount = 1;
            auto dstArrayLayerCount = 1;

            auto dstTextureStuff = generateTextureData(dstExtent.width, dstExtent.height, dstMipLevelCount, dstArrayLayerCount);
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

            createRequiredResources();

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
            checkTestResults(srcExtent, expectedCopiedData.data, expectedOriginalData.data);
        }
    };

    template<typename T>
    void copyTextureTestImpl(IDevice* device, UnitTestContext* context)
    {
        for (uint32_t i = 1; i < (uint32_t)Format::CountOf; ++i)
        {
            auto format = (Format)i;
            auto validationFormat = getValidationTextureFormat(format);
            if (!validationFormat)
                continue;

            T test;
            test.init(device, context, format, validationFormat);
            test.run();
        }
    }

    SLANG_UNIT_TEST(copyTextureTests)
    {
//         runTestImpl(copyTextureTestImpl<SimpleCopyTexture>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<SimpleCopyTexture>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopyTextureSection>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyTextureSection>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<LargeSrcToSmallDst>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<LargeSrcToSmallDst>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<SmallSrcToLargeDst>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<SmallSrcToLargeDst>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopyBetweenMips>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyBetweenMips>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopyBetweenLayers>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyBetweenLayers>, unitTestContext, Slang::RenderApiFlag::Vulkan);
//         runTestImpl(copyTextureTestImpl<CopyWithOffsets>, unitTestContext, Slang::RenderApiFlag::D3D12);
//         runTestImpl(copyTextureTestImpl<CopyWithOffsets>, unitTestContext, Slang::RenderApiFlag::Vulkan);
        runTestImpl(copyTextureTestImpl<CopySectionWithSetExtent>, unitTestContext, Slang::RenderApiFlag::D3D12);
        runTestImpl(copyTextureTestImpl<CopySectionWithSetExtent>, unitTestContext, Slang::RenderApiFlag::Vulkan);

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
