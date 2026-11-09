// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include "slang-hgvm.h"

#include <span>
#include <string>
#include <vector>

namespace Slang::HGVM
{

/// Supplies encoded bytes without exposing size or random access to an image decoder.
///
/// Consider a network transport that currently has only 17 bytes available: `read` may return
/// those 17 bytes even when the caller supplied a larger buffer. Codec helpers repeat reads until
/// their current field is complete, while never seeking or asking for the complete image size.
class SequentialByteReader
{
public:
    virtual ~SequentialByteReader() = default;
    virtual SlangResult read(void* destination, size_t capacity, size_t& outByteCount) = 0;
};

/// Accepts encoded bytes in forward order without allowing an image encoder to patch its output.
///
/// For example, the EXR encoder writes its precomputed header and offset table before emitting
/// scanline chunks. A file, socket, or memory-backed stream can implement the same operation.
class SequentialByteWriter
{
public:
    virtual ~SequentialByteWriter() = default;
    virtual SlangResult write(std::span<const uint8_t> bytes) = 0;
};

/// A decoded, tightly packed, row-major RGBA32Float image.
struct DecodedTextureImage
{
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<float> rgba;
};

/// Decodes exactly one EXR, PNG, JPEG, or BMP, leaving subsequent stream records untouched.
/// LDR channels are normalized to [0, 1] without gamma conversion; missing alpha is one.
bool decodeTextureImage(
    HGBCImageCodec codec,
    SequentialByteReader& source,
    DecodedTextureImage& outImage,
    uint64_t& outBytesConsumed,
    std::string& outError);

/// Encodes one texture as EXR or eight-bit PNG/JPEG/BMP. LDR output clamps and rounds channels
/// without gamma conversion; JPEG uses quality 90 and discards alpha.
bool encodeTextureImage(
    HGBCImageCodec codec,
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    SequentialByteWriter& destination,
    uint64_t& outBytesProduced,
    std::string& outError);

/// Decodes single-part scanline RGB/RGBA EXRs with FLOAT or HALF channels into linear RGBA32Float.
/// Accepts sequential increasing-Y chunks, including ZIP and PIZ; missing alpha is one.
bool decodeTextureEXR(
    std::span<const uint8_t> source,
    DecodedTextureImage& outImage,
    std::string& outError);

/// Decodes one EXR image using only forward reads and reports its exact encoded extent.
bool decodeTextureEXR(
    SequentialByteReader& source,
    DecodedTextureImage& outImage,
    uint64_t& outBytesConsumed,
    std::string& outError);

/// Encodes an RGBA8 or RGBA32Float texture readback as an eight-bit PNG.
bool encodeTexturePNG(
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    std::vector<uint8_t>& outPNG,
    std::string& outError);

/// Encodes a texture to a forward-only destination without retaining the complete PNG.
bool encodeTexturePNG(
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    SequentialByteWriter& destination,
    uint64_t& outBytesProduced,
    std::string& outError);

/// Encodes unpacked float16/32 and normalized 8/16-bit textures as uncompressed RGBA32Float EXR.
/// RGB sRGB components are linearized; absent RGB components are zero and absent alpha is one.
bool encodeTextureEXR(
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    std::vector<uint8_t>& outEXR,
    std::string& outError);

/// Encodes a texture to a forward-only destination using at most one scanline of scratch data.
bool encodeTextureEXR(
    const TextureInfo& info,
    const rhi::SubresourceLayout& layout,
    std::span<const uint8_t> source,
    SequentialByteWriter& destination,
    uint64_t& outBytesProduced,
    std::string& outError);

} // namespace Slang::HGVM
