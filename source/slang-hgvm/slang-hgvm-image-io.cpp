// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "slang-hgvm-image-io.h"

#include "core/slang-math.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_NO_STDIO
#include "stb_image.h"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYEXR_IMPLEMENTATION
#define TINYEXR_USE_THREAD 0
#define TINYEXR_USE_OPENMP 0
#include "tinyexr.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstring>
#include <limits>

namespace Slang::HGVM
{
namespace
{

/// Adds exact-field reads and position accounting to a potentially short sequential transport.
///
/// Consider a four-byte EXR integer delivered by a socket as one byte followed by three bytes:
/// `readBytes` performs both transport reads and advances `m_offset` only by bytes actually
/// received. No codec operation can seek this adapter.
class StreamByteReader
{
public:
    explicit StreamByteReader(SequentialByteReader& reader)
        : m_reader(reader)
    {
    }

    uint64_t getOffset() const { return m_offset; }
    const std::vector<uint8_t>& getBytes() const { return m_bytes; }

    bool readBytes(void* destination, size_t size)
    {
        uint8_t* output = static_cast<uint8_t*>(destination);
        size_t remaining = size;
        while (remaining)
        {
            size_t readCount = 0;
            if (SLANG_FAILED(m_reader.read(output, remaining, readCount)) || !readCount ||
                readCount > remaining)
            {
                return false;
            }
            m_bytes.insert(m_bytes.end(), output, output + readCount);
            output += readCount;
            remaining -= readCount;
            m_offset += readCount;
        }
        return true;
    }

    bool retainBytes(size_t size)
    {
        uint8_t scratch[4096];
        while (size)
        {
            size_t count = std::min(size, sizeof(scratch));
            if (!readBytes(scratch, count))
                return false;
            size -= count;
        }
        return true;
    }

    bool readUInt8(uint8_t& outValue) { return readBytes(&outValue, sizeof(outValue)); }

    bool readUInt16BE(uint16_t& outValue)
    {
        uint8_t bytes[2];
        if (!readBytes(bytes, sizeof(bytes)))
            return false;
        outValue = (uint16_t(bytes[0]) << 8) | bytes[1];
        return true;
    }

    bool readUInt32BE(uint32_t& outValue)
    {
        uint16_t high, low;
        if (!readUInt16BE(high) || !readUInt16BE(low))
            return false;
        outValue = (uint32_t(high) << 16) | low;
        return true;
    }

    bool readUInt32(uint32_t& outValue)
    {
        uint8_t bytes[4];
        if (!readBytes(bytes, sizeof(bytes)))
            return false;
        outValue = uint32_t(bytes[0]) | (uint32_t(bytes[1]) << 8) | (uint32_t(bytes[2]) << 16) |
                   (uint32_t(bytes[3]) << 24);
        return true;
    }

    bool readInt32(int32_t& outValue)
    {
        uint32_t bits = 0;
        if (!readUInt32(bits))
            return false;
        memcpy(&outValue, &bits, sizeof(outValue));
        return true;
    }

    bool readUInt64(uint64_t& outValue)
    {
        uint32_t low = 0;
        uint32_t high = 0;
        if (!readUInt32(low) || !readUInt32(high))
            return false;
        outValue = uint64_t(low) | (uint64_t(high) << 32);
        return true;
    }

    bool readString(std::string& outValue)
    {
        outValue.clear();
        for (;;)
        {
            uint8_t byte = 0;
            if (!readUInt8(byte))
                return false;
            if (!byte)
                return true;
            outValue.push_back(char(byte));
        }
    }

private:
    SequentialByteReader& m_reader;
    uint64_t m_offset = 0;
    std::vector<uint8_t> m_bytes;
};

/// Adapts an immutable byte span to the same short-read contract used by files and sockets.
class SpanSequentialReader : public SequentialByteReader
{
public:
    explicit SpanSequentialReader(std::span<const uint8_t> bytes)
        : m_bytes(bytes)
    {
    }

    SlangResult read(void* destination, size_t capacity, size_t& outByteCount) override
    {
        outByteCount = std::min(capacity, m_bytes.size() - m_offset);
        if (outByteCount)
            memcpy(destination, m_bytes.data() + m_offset, outByteCount);
        m_offset += outByteCount;
        return SLANG_OK;
    }

private:
    std::span<const uint8_t> m_bytes;
    size_t m_offset = 0;
};

/// Adapts a growable vector to a forward writer for the compatibility convenience overloads.
class VectorSequentialWriter : public SequentialByteWriter
{
public:
    explicit VectorSequentialWriter(std::vector<uint8_t>& bytes)
        : m_bytes(bytes)
    {
    }

    SlangResult write(std::span<const uint8_t> bytes) override
    {
        m_bytes.insert(m_bytes.end(), bytes.begin(), bytes.end());
        return SLANG_OK;
    }

private:
    std::vector<uint8_t>& m_bytes;
};

void setDecodeError(std::string& outError, const char* detail)
{
    outError = "Failed to decode texture image: ";
    outError += detail ? detail : "the decoder did not provide an error";
}

/// Retains one PNG through IEND, so the decoder's buffering cannot consume the next image.
bool framePNG(StreamByteReader& reader)
{
    uint8_t signature[8];
    const uint8_t expected[] = {137, 80, 78, 71, 13, 10, 26, 10};
    if (!reader.readBytes(signature, sizeof(signature)) ||
        memcmp(signature, expected, sizeof(signature)) != 0)
        return false;
    for (;;)
    {
        uint32_t size, type;
        if (!reader.readUInt32BE(size) || !reader.readUInt32BE(type) ||
            reader.getOffset() + uint64_t(size) + 4 > INT_MAX ||
            !reader.retainBytes(size_t(size) + 4))
            return false;
        if (type == 0x49454e44) // IEND, including its CRC.
            return size == 0;
    }
}

/// Retains one JPEG through EOI, including all scans in progressive images.
/// Consider an APP segment containing FF D9: its declared length makes those bytes payload,
/// whereas FF D9 in entropy data ends the image. Stuffed FF 00 and restart markers stay in a scan.
bool frameJPEG(StreamByteReader& reader)
{
    uint16_t signature;
    if (!reader.readUInt16BE(signature) || signature != 0xffd8)
        return false;
    bool inScan = false;
    while (reader.getOffset() < INT_MAX - 65536)
    {
        uint8_t byte;
        if (!reader.readUInt8(byte))
            return false;
        if (byte != 0xff)
        {
            if (!inScan)
                return false;
            continue;
        }
        do
        {
            if (!reader.readUInt8(byte))
                return false;
        } while (byte == 0xff && reader.getOffset() < INT_MAX);
        if (byte == 0xd9)
            return true;
        if (byte == 0 || (byte >= 0xd0 && byte <= 0xd7))
        {
            if (!inScan)
                return false;
            continue;
        }
        if (byte == 0x01) // TEM is a standalone marker.
            continue;
        if (byte == 0xd8 || byte == 0xff)
            return false;
        uint16_t size;
        if (!reader.readUInt16BE(size) || size < 2 || !reader.retainBytes(size - 2))
            return false;
        inScan = byte == 0xda; // SOS begins entropy data.
    }
    return false;
}

/// Retains one BMP using its file-header extent, including row padding and optional metadata.
bool frameBMP(StreamByteReader& reader)
{
    uint16_t signature;
    uint32_t size;
    return reader.readUInt16BE(signature) && signature == 0x424d && reader.readUInt32(size) &&
           size >= 14 && size <= INT_MAX && reader.retainBytes(size - 6);
}

/// Owns codec allocations on both successful decodes and malformed input paths.
struct EXRDecodeStorage
{
    EXRHeader header;
    EXRImage image;
    const char* error = nullptr;

    EXRDecodeStorage()
    {
        InitEXRHeader(&header);
        InitEXRImage(&image);
    }
    ~EXRDecodeStorage()
    {
        FreeEXRImage(&image);
        FreeEXRHeader(&header);
        if (error)
            FreeEXRErrorMessage(error);
    }
};

/// Frames exactly one scanline EXR from a forward-only transport before invoking the codec.
/// Consider a PIZ environment followed by another image in the same byte stream. Its header and
/// offset table describe 32-row chunks, so reading to EOF would consume the second image. We read
/// the declared chunks in physical offset order, then let TinyEXR decode the retained bytes.
/// This keeps compression details in the codec and preserves the stream's next-image position.
bool decodeTextureEXRImpl(
    SequentialByteReader& source,
    DecodedTextureImage& outImage,
    uint64_t& outBytesConsumed,
    std::string& outError)
{
    outImage = {};
    outBytesConsumed = 0;
    StreamByteReader reader(source);
    uint32_t magic = 0, flags = 0;
    if (!reader.readUInt32(magic) || magic != 20000630 || !reader.readUInt32(flags))
    {
        setDecodeError(outError, "the EXR header is invalid");
        return false;
    }
    EXRVersion version{};
    if (ParseEXRVersionFromMemory(&version, reader.getBytes().data(), 8) != TINYEXR_SUCCESS ||
        version.tiled || version.non_image || version.multipart)
    {
        setDecodeError(outError, "only single-part scanline EXR images are supported");
        return false;
    }
    for (;;)
    {
        std::string name, type;
        uint32_t size = 0;
        if (!reader.readString(name))
        {
            setDecodeError(outError, "the EXR attribute list is truncated");
            return false;
        }
        if (name.empty())
            break;
        if (!reader.readString(type) || !reader.readUInt32(size) || !reader.retainBytes(size))
        {
            setDecodeError(outError, "an EXR attribute is truncated");
            return false;
        }
    }
    EXRDecodeStorage decoded;
    const auto& bytes = reader.getBytes();
    if (ParseEXRHeaderFromMemory(
            &decoded.header,
            &version,
            bytes.data(),
            bytes.size(),
            &decoded.error) != TINYEXR_SUCCESS)
    {
        setDecodeError(outError, decoded.error);
        return false;
    }
    const EXRHeader& header = decoded.header;
    int64_t width = int64_t(header.data_window.max_x) - header.data_window.min_x + 1;
    int64_t height = int64_t(header.data_window.max_y) - header.data_window.min_y + 1;
    if (width <= 0 || height <= 0 || width > INT_MAX || height > INT_MAX ||
        uint64_t(width) * uint64_t(height) >
            std::numeric_limits<size_t>::max() / (4 * sizeof(float)))
    {
        setDecodeError(outError, "the EXR dimensions are invalid or too large");
        return false;
    }
    if (header.line_order != 0)
    {
        setDecodeError(outError, "only increasing-Y EXR scanlines are supported");
        return false;
    }
    int channelIndex[4] = {-1, -1, -1, -1};
    for (int channel = 0; channel < header.num_channels; ++channel)
    {
        const auto& info = header.channels[channel];
        int output = strcmp(info.name, "R") == 0   ? 0
                     : strcmp(info.name, "G") == 0 ? 1
                     : strcmp(info.name, "B") == 0 ? 2
                     : strcmp(info.name, "A") == 0 ? 3
                                                   : -1;
        if (output < 0 || channelIndex[output] >= 0 || info.x_sampling != 1 ||
            info.y_sampling != 1 ||
            (header.pixel_types[channel] != TINYEXR_PIXELTYPE_FLOAT &&
             header.pixel_types[channel] != TINYEXR_PIXELTYPE_HALF))
        {
            setDecodeError(
                outError,
                "expected full-resolution RGB/RGBA FLOAT or HALF EXR channels");
            return false;
        }
        channelIndex[output] = channel;
        decoded.header.requested_pixel_types[channel] = TINYEXR_PIXELTYPE_FLOAT;
    }
    if (channelIndex[0] < 0 || channelIndex[1] < 0 || channelIndex[2] < 0)
    {
        setDecodeError(outError, "the EXR is missing RGB channels");
        return false;
    }
    // Use the pinned codec's block-size definition rather than duplicating its compression table.
    uint32_t rowsPerChunk = uint32_t(tinyexr::NumScanlines(header.compression_type));
    size_t chunkCount = size_t((uint64_t(height) + rowsPerChunk - 1) / rowsPerChunk);
    std::vector<uint64_t> offsets(chunkCount);
    for (auto& offset : offsets)
    {
        if (!reader.readUInt64(offset))
        {
            setDecodeError(outError, "the EXR chunk offset table is truncated");
            return false;
        }
    }
    for (size_t chunk = 0; chunk < chunkCount; ++chunk)
    {
        int32_t y = 0;
        uint32_t size = 0;
        if (offsets[chunk] != reader.getOffset() || !reader.readInt32(y) ||
            !reader.readUInt32(size) ||
            int64_t(y) != int64_t(header.data_window.min_y) + int64_t(chunk * rowsPerChunk))
        {
            setDecodeError(outError, "EXR chunks are not in sequential increasing-Y order");
            return false;
        }
        if (!reader.retainBytes(size))
        {
            setDecodeError(outError, "an EXR chunk is truncated");
            return false;
        }
    }
    if (LoadEXRImageFromMemory(
            &decoded.image,
            &header,
            bytes.data(),
            bytes.size(),
            &decoded.error) != TINYEXR_SUCCESS)
    {
        setDecodeError(outError, decoded.error);
        return false;
    }
    outImage.width = uint32_t(width);
    outImage.height = uint32_t(height);
    size_t pixelCount = size_t(width * height);
    outImage.rgba.resize(pixelCount * 4);
    for (size_t pixel = 0; pixel < pixelCount; ++pixel)
    {
        for (int component = 0; component < 4; ++component)
        {
            int channel = channelIndex[component];
            outImage.rgba[pixel * 4 + component] =
                channel < 0 ? 1.0f
                            : reinterpret_cast<const float*>(decoded.image.images[channel])[pixel];
        }
    }
    outBytesConsumed = reader.getOffset();
    return true;
}

struct ImageWriterContext
{
    SequentialByteWriter* writer = nullptr;
    uint64_t byteCount = 0;
    bool failed = false;
};

void writeImageData(void* context, void* data, int size)
{
    auto& output = *static_cast<ImageWriterContext*>(context);
    if (output.failed || size < 0)
        return;
    std::span<const uint8_t> bytes(static_cast<const uint8_t*>(data), size_t(size));
    if (SLANG_FAILED(output.writer->write(bytes)))
    {
        output.failed = true;
        return;
    }
    output.byteCount += bytes.size();
}

void appendUInt32LE(std::vector<uint8_t>& output, uint32_t value)
{
    output.push_back(uint8_t(value));
    output.push_back(uint8_t(value >> 8));
    output.push_back(uint8_t(value >> 16));
    output.push_back(uint8_t(value >> 24));
}

void appendUInt64LE(std::vector<uint8_t>& output, uint64_t value)
{
    appendUInt32LE(output, uint32_t(value));
    appendUInt32LE(output, uint32_t(value >> 32));
}

void appendFloat32LE(std::vector<uint8_t>& output, float value)
{
    uint32_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    appendUInt32LE(output, bits);
}

void appendNullTerminated(std::vector<uint8_t>& output, std::string_view value)
{
    output.insert(output.end(), value.begin(), value.end());
    output.push_back(0);
}

void appendEXRAttribute(
    std::vector<uint8_t>& output,
    std::string_view name,
    std::string_view type,
    std::span<const uint8_t> value)
{
    appendNullTerminated(output, name);
    appendNullTerminated(output, type);
    appendUInt32LE(output, uint32_t(value.size()));
    output.insert(output.end(), value.begin(), value.end());
}

/// Accepts only unpacked formats whose component order and width are known here.
/// For example, RGB10A2Unorm also has four channels in four bytes, but must not be decoded as
/// RGBA8. RHI's channel count alone cannot distinguish that packed representation.
size_t getEXRComponentSize(rhi::Format format)
{
    switch (format)
    {
    case rhi::Format::R8Unorm:
    case rhi::Format::R8Snorm:
    case rhi::Format::RG8Unorm:
    case rhi::Format::RG8Snorm:
    case rhi::Format::RGBA8Unorm:
    case rhi::Format::RGBA8Snorm:
    case rhi::Format::RGBA8UnormSrgb:
    case rhi::Format::BGRA8Unorm:
    case rhi::Format::BGRA8UnormSrgb:
        return 1;
    case rhi::Format::R16Unorm:
    case rhi::Format::R16Snorm:
    case rhi::Format::R16Float:
    case rhi::Format::RG16Unorm:
    case rhi::Format::RG16Snorm:
    case rhi::Format::RG16Float:
    case rhi::Format::RGBA16Unorm:
    case rhi::Format::RGBA16Snorm:
    case rhi::Format::RGBA16Float:
        return 2;
    case rhi::Format::R32Float:
    case rhi::Format::RG32Float:
    case rhi::Format::RGB32Float:
    case rhi::Format::RGBA32Float:
        return 4;
    default:
        return 0;
    }
}

/// Converts a physical texel component to EXR's linear float representation without tone mapping.
/// Missing RGB components are zero and missing alpha is one, matching texture-load defaults.
float readEXRComponent(
    const uint8_t* pixel,
    size_t channel,
    size_t componentSize,
    const rhi::FormatInfo& format)
{
    if (channel >= format.channelCount)
        return channel == 3 ? 1.0f : 0.0f;
    if ((format.format == rhi::Format::BGRA8Unorm ||
         format.format == rhi::Format::BGRA8UnormSrgb) &&
        channel < 3)
        channel = 2 - channel;
    const uint8_t* component = pixel + channel * componentSize;
    float value;
    if (format.kind == rhi::FormatKind::Float)
    {
        if (componentSize == 4)
            memcpy(&value, component, sizeof(value));
        else
        {
            uint16_t bits;
            memcpy(&bits, component, sizeof(bits));
            value = HalfToFloat(bits);
        }
    }
    else if (componentSize == 1)
        value = format.isSigned ? std::max(-1.0f, float(int8_t(*component)) / 127.0f)
                                : float(*component) / 255.0f;
    else if (format.isSigned)
    {
        int16_t bits;
        memcpy(&bits, component, sizeof(bits));
        value = std::max(-1.0f, float(bits) / 32767.0f);
    }
    else
    {
        uint16_t bits;
        memcpy(&bits, component, sizeof(bits));
        value = float(bits) / 65535.0f;
    }
    if (format.isSrgb && channel != 3)
        value = value <= 0.04045f ? value / 12.92f : std::pow((value + 0.055f) / 1.055f, 2.4f);
    return value;
}
} // namespace

bool decodeTextureImage(
    HGBCImageCodec codec,
    SequentialByteReader& source,
    DecodedTextureImage& outImage,
    uint64_t& outBytesConsumed,
    std::string& outError)
{
    if (codec == HGBCImageCodec::EXR)
        return decodeTextureEXR(source, outImage, outBytesConsumed, outError);
    outImage = {};
    outBytesConsumed = 0;
    outError.clear();
    StreamByteReader reader(source);
    bool framed = false;
    switch (codec)
    {
    case HGBCImageCodec::PNG:
        framed = framePNG(reader);
        break;
    case HGBCImageCodec::JPEG:
        framed = frameJPEG(reader);
        break;
    case HGBCImageCodec::BMP:
        framed = frameBMP(reader);
        break;
    default:
        outError = "Unsupported image codec";
        return false;
    }
    if (!framed)
    {
        setDecodeError(outError, "invalid or truncated image for the selected codec");
        return false;
    }
    int width, height, channels;
    const auto& bytes = reader.getBytes();
    // Loading bytes rather than stbi_loadf avoids stb's implicit LDR-to-HDR gamma transform.
    stbi_uc* pixels =
        stbi_load_from_memory(bytes.data(), int(bytes.size()), &width, &height, &channels, 4);
    if (!pixels)
    {
        setDecodeError(outError, stbi_failure_reason());
        return false;
    }
    outImage.width = uint32_t(width);
    outImage.height = uint32_t(height);
    outImage.rgba.resize(size_t(width) * size_t(height) * 4);
    for (size_t i = 0; i < outImage.rgba.size(); ++i)
        outImage.rgba[i] = float(pixels[i]) / 255.0f;
    stbi_image_free(pixels);
    outBytesConsumed = reader.getOffset();
    return true;
}

bool decodeTextureEXR(
    std::span<const uint8_t> source,
    DecodedTextureImage& outImage,
    std::string& outError)
{
    SpanSequentialReader reader(source);
    uint64_t ignoredByteCount = 0;
    return decodeTextureEXRImpl(reader, outImage, ignoredByteCount, outError);
}

bool decodeTextureEXR(
    SequentialByteReader& source,
    DecodedTextureImage& outImage,
    uint64_t& outBytesConsumed,
    std::string& outError)
{
    return decodeTextureEXRImpl(source, outImage, outBytesConsumed, outError);
}

bool encodeTexturePNG(
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    std::vector<uint8_t>& outPNG,
    std::string& outError)
{
    outPNG.clear();
    VectorSequentialWriter writer(outPNG);
    uint64_t ignoredByteCount = 0;
    return encodeTexturePNG(info, layout, source, writer, ignoredByteCount, outError);
}

bool encodeTexturePNG(
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    SequentialByteWriter& destination,
    uint64_t& outBytesProduced,
    std::string& outError)
{
    return encodeTextureImage(
        HGBCImageCodec::PNG,
        info,
        layout,
        source,
        destination,
        outBytesProduced,
        outError);
}

bool encodeTextureImage(
    HGBCImageCodec codec,
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    SequentialByteWriter& destination,
    uint64_t& outBytesProduced,
    std::string& outError)
{
    outBytesProduced = 0;
    if (codec == HGBCImageCodec::EXR)
        return encodeTextureEXR(info, layout, source, destination, outBytesProduced, outError);
    if (info.type != rhi::TextureType::Texture2D || layout.size.depth != 1)
    {
        outError = "Only two-dimensional texture images are supported";
        return false;
    }
    if (!layout.size.width || !layout.size.height || layout.size.width > INT_MAX ||
        layout.size.height > INT_MAX || layout.size.width > INT_MAX / 4 ||
        layout.size.height > SIZE_MAX / (layout.size.width * 4))
    {
        outError = "The texture image dimensions are invalid";
        return false;
    }

    size_t width = layout.size.width;
    size_t height = layout.size.height;
    std::vector<uint8_t> rgba(width * height * 4);
    if (info.format == rhi::Format::RGBA8Unorm || info.format == rhi::Format::RGBA8UnormSrgb)
    {
        size_t sourceRowSize = width * 4;
        if (layout.rowPitch < sourceRowSize || height > source.size() / layout.rowPitch)
        {
            outError = "The RGBA8 texture readback layout is inconsistent";
            return false;
        }
        for (size_t row = 0; row < height; ++row)
        {
            memcpy(
                rgba.data() + row * sourceRowSize,
                source.data() + row * layout.rowPitch,
                sourceRowSize);
        }
    }
    else if (info.format == rhi::Format::RGBA32Float)
    {
        size_t sourceRowSize = width * sizeof(float) * 4;
        if (layout.rowPitch < sourceRowSize || height > source.size() / layout.rowPitch)
        {
            outError = "The RGBA32Float texture readback layout is inconsistent";
            return false;
        }
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t column = 0; column < width; ++column)
            {
                float pixel[4];
                memcpy(
                    pixel,
                    source.data() + row * layout.rowPitch + column * sizeof(pixel),
                    sizeof(pixel));
                for (size_t channel = 0; channel < 4; ++channel)
                {
                    float value = std::isfinite(pixel[channel]) ? pixel[channel] : 0.0f;
                    rgba[(row * width + column) * 4 + channel] =
                        uint8_t(std::lround(std::clamp(value, 0.0f, 1.0f) * 255.0f));
                }
            }
        }
    }
    else
    {
        outError = std::string("Texture image format ") +
                   rhi::getRHI()->getFormatInfo(info.format).name + " is not supported";
        return false;
    }

    ImageWriterContext context = {&destination};
    int succeeded = 0;
    switch (codec)
    {
    case HGBCImageCodec::PNG:
        succeeded = stbi_write_png_to_func(
            writeImageData,
            &context,
            int(width),
            int(height),
            4,
            rgba.data(),
            int(width * 4));
        break;
    case HGBCImageCodec::JPEG:
        succeeded = stbi_write_jpg_to_func(
            writeImageData,
            &context,
            int(width),
            int(height),
            4,
            rgba.data(),
            90);
        break;
    case HGBCImageCodec::BMP:
        succeeded = stbi_write_bmp_to_func(
            writeImageData,
            &context,
            int(width),
            int(height),
            4,
            rgba.data());
        break;
    default:
        outError = "Unsupported image codec";
        return false;
    }
    if (!succeeded || context.failed)
    {
        outError = "Failed to encode the texture image";
        return false;
    }
    outBytesProduced = context.byteCount;
    return true;
}

bool encodeTextureEXR(
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    std::vector<uint8_t>& outEXR,
    std::string& outError)
{
    outEXR.clear();
    VectorSequentialWriter writer(outEXR);
    uint64_t ignoredByteCount = 0;
    return encodeTextureEXR(info, layout, source, writer, ignoredByteCount, outError);
}

bool encodeTextureEXR(
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    SequentialByteWriter& destination,
    uint64_t& outBytesProduced,
    std::string& outError)
{
    outBytesProduced = 0;
    if (info.type != rhi::TextureType::Texture2D || layout.size.depth != 1)
    {
        outError = "Only two-dimensional texture images are supported";
        return false;
    }
    if (!layout.size.width || !layout.size.height || layout.size.width > INT_MAX ||
        layout.size.height > INT_MAX || layout.size.width > INT_MAX / 16)
    {
        outError = "The texture image dimensions are invalid";
        return false;
    }

    size_t width = layout.size.width;
    size_t height = layout.size.height;
    const auto& format = rhi::getFormatInfo(info.format);
    size_t componentSize = getEXRComponentSize(info.format);
    size_t sourcePixelSize = format.blockSizeInBytes;
    if (!componentSize)
    {
        outError = std::string("Texture image format ") +
                   rhi::getRHI()->getFormatInfo(info.format).name + " is not supported";
        return false;
    }

    size_t sourceRowSize = width * sourcePixelSize;
    if (layout.rowPitch < sourceRowSize || height > source.size() / layout.rowPitch)
    {
        outError = "The texture readback layout is inconsistent";
        return false;
    }

    size_t scanlineSize = width * sizeof(float) * 4;
    size_t chunkSize = sizeof(uint32_t) * 2 + scanlineSize;
    if (height > (SIZE_MAX - 1024) / (sizeof(uint64_t) + chunkSize))
    {
        outError = "The EXR texture image is too large";
        return false;
    }

    // The complete header and offset table are bounded by image height and must precede the first
    // scanline in EXR. Pixel payload remains row-streamed below rather than retaining the complete
    // encoded image.
    std::vector<uint8_t> header;
    appendUInt32LE(header, 20000630);
    appendUInt32LE(header, 2);

    std::vector<uint8_t> channels;
    for (std::string_view channelName : {"A", "B", "G", "R"})
    {
        appendNullTerminated(channels, channelName);
        appendUInt32LE(channels, 2);
        channels.insert(channels.end(), {0, 0, 0, 0});
        appendUInt32LE(channels, 1);
        appendUInt32LE(channels, 1);
    }
    channels.push_back(0);
    appendEXRAttribute(header, "channels", "chlist", channels);

    const uint8_t noCompression = 0;
    appendEXRAttribute(
        header,
        "compression",
        "compression",
        std::span<const uint8_t>(&noCompression, 1));

    std::vector<uint8_t> window;
    appendUInt32LE(window, 0);
    appendUInt32LE(window, 0);
    appendUInt32LE(window, uint32_t(width - 1));
    appendUInt32LE(window, uint32_t(height - 1));
    appendEXRAttribute(header, "dataWindow", "box2i", window);
    appendEXRAttribute(header, "displayWindow", "box2i", window);

    const uint8_t increasingY = 0;
    appendEXRAttribute(header, "lineOrder", "lineOrder", std::span<const uint8_t>(&increasingY, 1));

    std::vector<uint8_t> floatAttribute;
    appendFloat32LE(floatAttribute, 1.0f);
    appendEXRAttribute(header, "pixelAspectRatio", "float", floatAttribute);

    std::vector<uint8_t> screenWindowCenter;
    appendFloat32LE(screenWindowCenter, 0.0f);
    appendFloat32LE(screenWindowCenter, 0.0f);
    appendEXRAttribute(header, "screenWindowCenter", "v2f", screenWindowCenter);
    appendEXRAttribute(header, "screenWindowWidth", "float", floatAttribute);
    header.push_back(0);

    uint64_t scanlineOffset = uint64_t(header.size()) + uint64_t(height) * sizeof(uint64_t);
    for (size_t row = 0; row < height; ++row)
    {
        appendUInt64LE(header, scanlineOffset);
        scanlineOffset += chunkSize;
    }

    if (SLANG_FAILED(destination.write(header)))
    {
        outError = "Failed to write the EXR header";
        return false;
    }
    outBytesProduced = header.size();

    static constexpr size_t channelOrder[] = {3, 2, 1, 0};
    for (size_t row = 0; row < height; ++row)
    {
        std::vector<uint8_t> scanline;
        scanline.reserve(chunkSize);
        appendUInt32LE(scanline, uint32_t(row));
        appendUInt32LE(scanline, uint32_t(scanlineSize));
        const uint8_t* sourceRow = source.data() + row * layout.rowPitch;
        for (size_t channel : channelOrder)
        {
            for (size_t column = 0; column < width; ++column)
            {
                const uint8_t* pixel = sourceRow + column * sourcePixelSize;
                appendFloat32LE(scanline, readEXRComponent(pixel, channel, componentSize, format));
            }
        }
        SLANG_ASSERT(scanline.size() == chunkSize);
        if (SLANG_FAILED(destination.write(scanline)))
        {
            outError = "Failed to write an EXR scanline";
            return false;
        }
        outBytesProduced += scanline.size();
    }
    return true;
}

} // namespace Slang::HGVM
