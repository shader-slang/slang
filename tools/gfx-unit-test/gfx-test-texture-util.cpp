#include "gfx-test-texture-util.h"

#include <slang-com-ptr.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace gfx_test
{

TextureInfo::~TextureInfo()
{
    for (SubresourceData subresourceData : subresourceDatas)
    {
        ::free((void*)subresourceData.data);
    }
}

Size getTexelSize(Format format)
{
    const FormatInfo& info = getFormatInfo(format);
    return info.blockSizeInBytes / info.pixelsPerBlock;
}

RefPtr<ValidationTextureFormatBase> getValidationTextureFormat(Format format)
{
    switch (format)
    {
    case Format::RGBA32Float:
        return new ValidationTextureFormat<float>(4);
    case Format::RGB32Float:
        return new ValidationTextureFormat<float>(3);
    case Format::RG32Float:
        return new ValidationTextureFormat<float>(2);
    case Format::R32Float:
        return new ValidationTextureFormat<float>(1);

    case Format::RGBA16Float:
        return new ValidationTextureFormat<uint16_t>(4);
    case Format::RG16Float:
        return new ValidationTextureFormat<uint16_t>(2);
    case Format::R16Float:
        return new ValidationTextureFormat<uint16_t>(1);

    case Format::R64Uint:
        return new ValidationTextureFormat<uint64_t>(1);

    case Format::RGBA32Uint:
        return new ValidationTextureFormat<uint32_t>(4);
    case Format::RGB32Uint:
        return new ValidationTextureFormat<uint32_t>(3);
    case Format::RG32Uint:
        return new ValidationTextureFormat<uint32_t>(2);
    case Format::R32Uint:
        return new ValidationTextureFormat<uint32_t>(1);

    case Format::RGBA16Uint:
        return new ValidationTextureFormat<uint16_t>(4);
    case Format::RG16Uint:
        return new ValidationTextureFormat<uint16_t>(2);
    case Format::R16Uint:
        return new ValidationTextureFormat<uint16_t>(1);

    case Format::RGBA8Uint:
        return new ValidationTextureFormat<uint8_t>(4);
    case Format::RG8Uint:
        return new ValidationTextureFormat<uint8_t>(2);
    case Format::R8Uint:
        return new ValidationTextureFormat<uint8_t>(1);

    case Format::R64Sint:
        return new ValidationTextureFormat<int64_t>(1);

    case Format::RGBA32Sint:
        return new ValidationTextureFormat<int32_t>(4);
    case Format::RGB32Sint:
        return new ValidationTextureFormat<int32_t>(3);
    case Format::RG32Sint:
        return new ValidationTextureFormat<int32_t>(2);
    case Format::R32Sint:
        return new ValidationTextureFormat<int32_t>(1);

    case Format::RGBA16Sint:
        return new ValidationTextureFormat<int16_t>(4);
    case Format::RG16Sint:
        return new ValidationTextureFormat<int16_t>(2);
    case Format::R16Sint:
        return new ValidationTextureFormat<int16_t>(1);

    case Format::RGBA8Sint:
        return new ValidationTextureFormat<int8_t>(4);
    case Format::RG8Sint:
        return new ValidationTextureFormat<int8_t>(2);
    case Format::R8Sint:
        return new ValidationTextureFormat<int8_t>(1);

    case Format::RGBA16Unorm:
        return new ValidationTextureFormat<uint16_t>(4);
    case Format::RG16Unorm:
        return new ValidationTextureFormat<uint16_t>(2);
    case Format::R16Unorm:
        return new ValidationTextureFormat<uint16_t>(1);

    case Format::RGBA8Unorm:
        return new ValidationTextureFormat<uint8_t>(4);
    case Format::RGBA8UnormSrgb:
        return new ValidationTextureFormat<uint8_t>(4);
    case Format::RG8Unorm:
        return new ValidationTextureFormat<uint8_t>(2);
    case Format::R8Unorm:
        return new ValidationTextureFormat<uint8_t>(1);
    case Format::BGRA8Unorm:
        return new ValidationTextureFormat<uint8_t>(4);
    case Format::BGRA8UnormSrgb:
        return new ValidationTextureFormat<uint8_t>(4);
    case Format::BGRX8Unorm:
        return new ValidationTextureFormat<uint8_t>(3);
    case Format::BGRX8UnormSrgb:
        return new ValidationTextureFormat<uint8_t>(3);

    case Format::RGBA16Snorm:
        return new ValidationTextureFormat<int16_t>(4);
    case Format::RG16Snorm:
        return new ValidationTextureFormat<int16_t>(2);
    case Format::R16Snorm:
        return new ValidationTextureFormat<int16_t>(1);

    case Format::RGBA8Snorm:
        return new ValidationTextureFormat<int8_t>(4);
    case Format::RG8Snorm:
        return new ValidationTextureFormat<int8_t>(2);
    case Format::R8Snorm:
        return new ValidationTextureFormat<int8_t>(1);

    case Format::D32Float:
        return new ValidationTextureFormat<float>(1);
    case Format::D16Unorm:
        return new ValidationTextureFormat<uint16_t>(1);

    case Format::BGRA4Unorm:
        return new PackedValidationTextureFormat<uint16_t>(4, 4, 4, 4);
    case Format::B5G6R5Unorm:
        return new PackedValidationTextureFormat<uint16_t>(5, 6, 5, 0);
    case Format::BGR5A1Unorm:
        return new PackedValidationTextureFormat<uint16_t>(5, 5, 5, 1);

    case Format::RGB9E5Ufloat:
        return new ValidationTextureFormat<uint32_t>(1);
    case Format::RGB10A2Unorm:
        return new PackedValidationTextureFormat<uint32_t>(10, 10, 10, 2);
    case Format::RGB10A2Uint:
        return new PackedValidationTextureFormat<uint32_t>(10, 10, 10, 2);
    case Format::R11G11B10Float:
        return new PackedValidationTextureFormat<uint32_t>(11, 11, 10, 0);

        // TODO: Add testing support for BC formats
        //                     BC1Unorm,
        //                     BC1UnormSrgb,
        //                     BC2Unorm,
        //                     BC2UnormSrgb,
        //                     BC3Unorm,
        //                     BC3UnormSrgb,
        //                     BC4Unorm,
        //                     BC4Snorm,
        //                     BC5Unorm,
        //                     BC5Snorm,
        //                     BC6HUfloat,
        //                     BC6HSfloat,
        //                     BC7Unorm,
        //                     BC7UnormSrgb,
    default:
        return nullptr;
    }
}

void generateTextureData(RefPtr<TextureInfo> texture, ValidationTextureFormatBase* validationFormat)
{
    Extent3D extent = texture->extent;
    uint32_t layerCount = texture->arrayLength;
    if (texture->textureType == TextureType::TextureCube)
        layerCount *= 6;
    uint32_t mipLevels = texture->mipCount;
    Size texelSize = getTexelSize(texture->format);

    for (uint32_t layer = 0; layer < layerCount; ++layer)
    {
        for (uint32_t mip = 0; mip < mipLevels; ++mip)
        {
            RefPtr<ValidationTextureData> subresource = new ValidationTextureData();

            uint32_t mipWidth = std::max(extent.width >> mip, 1u);
            uint32_t mipHeight = std::max(extent.height >> mip, 1u);
            uint32_t mipDepth = std::max(extent.depth >> mip, 1u);
            uint32_t mipSize = mipWidth * mipHeight * mipDepth * texelSize;
            subresource->textureData = ::malloc(mipSize);
            assert(subresource->textureData != nullptr);

            subresource->extent.width = mipWidth;
            subresource->extent.height = mipHeight;
            subresource->extent.depth = mipDepth;
            subresource->pitches.x = texelSize;
            subresource->pitches.y = mipWidth * texelSize;
            subresource->pitches.z = mipHeight * subresource->pitches.y;
            texture->subresourceObjects.push_back(subresource);

            for (int z = 0; z < mipDepth; ++z)
            {
                for (int y = 0; y < mipHeight; ++y)
                {
                    for (int x = 0; x < mipWidth; ++x)
                    {
                        auto texel = subresource->getBlockAt(x, y, z);
                        validationFormat->initializeTexel(texel, x, y, z, mip, layer);
                    }
                }
            }

            SubresourceData subData = {};
            subData.data = subresource->textureData;
            subData.rowPitch = subresource->pitches.y;
            subData.slicePitch = subresource->pitches.z;
            texture->subresourceDatas.push_back(subData);
        }
    }
}

std::vector<uint8_t> removePadding(
    ISlangBlob* pixels,
    uint32_t width,
    uint32_t height,
    Size rowPitch,
    Size pixelSize)
{
    std::vector<uint8_t> buffer;
    buffer.resize(height * rowPitch);
    for (uint32_t i = 0; i < height; ++i)
    {
        Offset srcOffset = i * rowPitch;
        Offset dstOffset = i * width * pixelSize;
        memcpy(
            buffer.data() + dstOffset,
            (char*)pixels->getBufferPointer() + srcOffset,
            width * pixelSize);
    }

    return buffer;
}

Result writeImage(const char* filename, ISlangBlob* pixels, uint32_t width, uint32_t height)
{
    int stbResult = stbi_write_hdr(filename, width, height, 4, (float*)pixels->getBufferPointer());

    return stbResult ? SLANG_OK : SLANG_FAIL;
}

Result writeImage(
    const char* filename,
    ISlangBlob* pixels,
    uint32_t width,
    uint32_t height,
    uint32_t rowPitch,
    uint32_t pixelSize)
{
    if (rowPitch == width * pixelSize)
        return writeImage(filename, pixels, width, height);

    std::vector<uint8_t> buffer = removePadding(pixels, width, height, rowPitch, pixelSize);

    int stbResult = stbi_write_hdr(filename, width, height, 4, (float*)buffer.data());

    return stbResult ? SLANG_OK : SLANG_FAIL;
}

} // namespace gfx_test
