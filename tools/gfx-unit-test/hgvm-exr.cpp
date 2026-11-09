// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm-image-io.h"
#include "tinyexr/tinyexr.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace Slang;
using namespace Slang::HGVM;

namespace
{
// Models a forward-only transport that splits even header fields across reads.
struct ShortEXRReader : SequentialByteReader
{
    std::span<const uint8_t> bytes;
    size_t offset = 0;

    SlangResult read(void* destination, size_t capacity, size_t& count) override
    {
        count = std::min({capacity, bytes.size() - offset, size_t(7)});
        memcpy(destination, bytes.data() + offset, count);
        offset += count;
        return SLANG_OK;
    }
};
} // namespace

SLANG_UNIT_TEST(hgvmEXRCompressedScanlines)
{
    // A partial final block, nonconstant HDR/negative values, optional alpha, and HALF storage
    // exercise the framing adapter and channel conversion independently of any downloaded asset.
    for (int compression :
         {TINYEXR_COMPRESSIONTYPE_NONE,
          TINYEXR_COMPRESSIONTYPE_RLE,
          TINYEXR_COMPRESSIONTYPE_ZIPS,
          TINYEXR_COMPRESSIONTYPE_ZIP,
          TINYEXR_COMPRESSIONTYPE_PIZ})
    {
        for (int channelCount : {3, 4})
        {
            for (int storageType : {TINYEXR_PIXELTYPE_HALF, TINYEXR_PIXELTYPE_FLOAT})
            {
                constexpr int width = 19, height = 35;
                std::vector<float> planes[4];
                unsigned char* planePointers[4];
                EXRChannelInfo channels[4] = {};
                int types[4], requestedTypes[4];
                const char* names[] = {"B", "G", "R", "A"};
                for (int c = 0; c < channelCount; ++c)
                {
                    planes[c].resize(width * height);
                    for (int i = 0; i < width * height; ++i)
                        planes[c][i] = float((i % 31) - 4 + c * 32) / 16.0f;
                    planePointers[c] = reinterpret_cast<unsigned char*>(planes[c].data());
                    strcpy(channels[c].name, names[c]);
                    channels[c].x_sampling = channels[c].y_sampling = 1;
                    types[c] = TINYEXR_PIXELTYPE_FLOAT;
                    requestedTypes[c] = storageType;
                }
                EXRImage source;
                InitEXRImage(&source);
                source.width = width;
                source.height = height;
                source.num_channels = channelCount;
                source.images = planePointers;
                EXRHeader header;
                InitEXRHeader(&header);
                header.num_channels = channelCount;
                header.channels = channels;
                header.pixel_types = types;
                header.requested_pixel_types = requestedTypes;
                header.compression_type = compression;
                unsigned char* encoded = nullptr;
                const char* codecError = nullptr;
                size_t size = SaveEXRImageToMemory(&source, &header, &encoded, &codecError);
                if (codecError)
                    FreeEXRErrorMessage(codecError);
                SLANG_CHECK_ABORT(size > 0);
                std::vector<uint8_t> bytes(encoded, encoded + size);
                free(encoded);
                // The decoder must leave the following stream record untouched.
                bytes.insert(bytes.end(), {0x12, 0x34, 0x56, 0x78});
                ShortEXRReader reader;
                reader.bytes = bytes;
                DecodedTextureImage image;
                std::string error;
                uint64_t consumed = 0;
                SLANG_CHECK_ABORT(decodeTextureEXR(reader, image, consumed, error));
                SLANG_CHECK(consumed == size && reader.offset == size);
                SLANG_CHECK(image.width == width && image.height == height);
                for (int i = 0; i < width * height; ++i)
                {
                    for (int c = 0; c < 3; ++c)
                        SLANG_CHECK(image.rgba[i * 4 + c] == planes[2 - c][i]);
                    SLANG_CHECK(image.rgba[i * 4 + 3] == (channelCount == 4 ? planes[3][i] : 1.0f));
                }
                SLANG_CHECK(!decodeTextureEXR(
                    std::span<const uint8_t>(bytes.data(), size - 1),
                    image,
                    error));
                SLANG_CHECK(!error.empty());
            }
        }
    }
}

namespace
{
struct ImageTestWriter : SequentialByteWriter
{
    std::vector<uint8_t> bytes;
    bool failWrites = false;

    SlangResult write(std::span<const uint8_t> data) override
    {
        if (failWrites)
            return SLANG_FAIL;
        bytes.insert(bytes.end(), data.begin(), data.end());
        return SLANG_OK;
    }
};
} // namespace

SLANG_UNIT_TEST(hgvmLDRImageCodecs)
{
    // Odd dimensions and padded rows catch both codec orientation and RHI layout mistakes.
    constexpr uint32_t width = 19, height = 9;
    for (auto format : {rhi::Format::RGBA32Float, rhi::Format::RGBA8Unorm})
    {
        size_t pixelSize = format == rhi::Format::RGBA32Float ? 16 : 4;
        rhi::SubresourceLayout layout = {};
        layout.size = {width, height, 1};
        layout.rowPitch = width * pixelSize + 16;
        std::vector<uint8_t> pixels(layout.rowPitch * height, 0xcc);
        std::vector<float> expected(width * height * 4);
        for (uint32_t y = 0; y < height; ++y)
        {
            for (uint32_t x = 0; x < width; ++x)
            {
                // Negative and HDR values must clamp for LDR output; alpha must not get gamma.
                float rgba[] =
                    {float(x) / (width - 1) * 1.5f - 0.25f, float(y) / (height - 1), 0.25f, 0.5f};
                for (size_t c = 0; c < 4; ++c)
                {
                    uint8_t value = uint8_t(std::lround(std::clamp(rgba[c], 0.0f, 1.0f) * 255));
                    expected[(y * width + x) * 4 + c] = float(value) / 255.0f;
                    if (format == rhi::Format::RGBA32Float)
                        memcpy(
                            pixels.data() + y * layout.rowPitch + x * pixelSize + c * 4,
                            &rgba[c],
                            4);
                    else
                        pixels[y * layout.rowPitch + x * pixelSize + c] = value;
                }
            }
        }
        TextureInfo info;
        info.type = rhi::TextureType::Texture2D;
        info.format = format;
        info.width = width;
        info.height = height;
        for (auto codec : {HGBCImageCodec::PNG, HGBCImageCodec::JPEG, HGBCImageCodec::BMP})
        {
            ImageTestWriter writer;
            uint64_t produced = 0;
            std::string error;
            SLANG_CHECK_ABORT(
                encodeTextureImage(codec, info, layout, pixels, writer, produced, error));
            SLANG_CHECK(produced == writer.bytes.size() && produced > 0);
            if (codec == HGBCImageCodec::JPEG)
            {
                // EOI-looking bytes in metadata must not terminate the sequential image frame.
                const uint8_t app[] = {0xff, 0xe1, 0, 6, 0xff, 0xd9, 0xff, 0xd8};
                writer.bytes.insert(writer.bytes.begin() + 2, std::begin(app), std::end(app));
            }
            auto encoded = writer.bytes;
            writer.bytes.insert(writer.bytes.end(), encoded.begin(), encoded.end());
            writer.bytes.push_back(0x42);
            ShortEXRReader reader;
            reader.bytes = writer.bytes;
            for (int imageIndex = 0; imageIndex < 2; ++imageIndex)
            {
                DecodedTextureImage decoded;
                uint64_t consumed = 0;
                SLANG_CHECK_ABORT(decodeTextureImage(codec, reader, decoded, consumed, error));
                SLANG_CHECK(consumed == encoded.size());
                SLANG_CHECK(reader.offset == encoded.size() * (imageIndex + 1));
                SLANG_CHECK(decoded.width == width && decoded.height == height);
                for (size_t i = 0; i < expected.size(); ++i)
                {
                    if (codec == HGBCImageCodec::JPEG)
                    {
                        if (i % 4 == 3)
                        {
                            SLANG_CHECK(decoded.rgba[i] == 1.0f);
                        }
                        else
                        {
                            SLANG_CHECK(std::abs(decoded.rgba[i] - expected[i]) < 0.09f);
                        }
                    }
                    else
                    {
                        SLANG_CHECK(decoded.rgba[i] == expected[i]);
                    }
                }
            }
            // Reject truncated data and a mismatched explicit format instead of accepting a
            // partial image or silently switching codecs based on its signature.
            for (size_t length : {size_t(0), size_t(1), encoded.size() / 2, encoded.size() - 1})
            {
                ShortEXRReader truncated;
                truncated.bytes = std::span<const uint8_t>(encoded.data(), length);
                DecodedTextureImage decoded;
                uint64_t consumed = 0;
                SLANG_CHECK(!decodeTextureImage(codec, truncated, decoded, consumed, error));
                SLANG_CHECK(!error.empty());
            }
            ShortEXRReader mismatched;
            mismatched.bytes = encoded;
            DecodedTextureImage decoded;
            uint64_t consumed = 0;
            SLANG_CHECK(!decodeTextureImage(
                codec == HGBCImageCodec::PNG ? HGBCImageCodec::JPEG : HGBCImageCodec::PNG,
                mismatched,
                decoded,
                consumed,
                error));
            ImageTestWriter failingWriter;
            failingWriter.failWrites = true;
            SLANG_CHECK(
                !encodeTextureImage(codec, info, layout, pixels, failingWriter, produced, error));
        }
    }
}
