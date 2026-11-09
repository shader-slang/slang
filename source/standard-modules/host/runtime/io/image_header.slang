// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.io.byte_stream;
import slang.runtime.io.image;
import slang.runtime.io.exr_header;
import slang.runtime.core.simple_assert;

/// Reads an exact header field, rejecting truncated input before texture allocation.
T readImageHeaderField<T>(ByteStream stream)
{
    T value;
    if (readByteStream(stream, value) != sizeof(T))
        fail("Truncated image header");
    return value;
}

uint readImageHeaderByte(ByteStream stream)
{
    uint value = 0;
    if (readByteStreamBytes(stream, value, 0, 1) != 1)
        fail("Truncated image header");
    return value;
}

/// Reads a two- or four-byte big-endian integer used by PNG and JPEG headers.
uint readImageHeaderBE(ByteStream stream, uint byteCount)
{
    uint value = 0;
    for (uint i = 0; i < byteCount; ++i)
        value = value * 256 + readImageHeaderByte(stream);
    return value;
}

/// Skips header payload without allocating memory based on an untrusted length.
void skipImageHeaderBytes(ByteStream stream, uint byteCount)
{
    for (uint i = 0; i < byteCount; ++i)
        readImageHeaderByte(stream);
}

/// Reads PNG's leading IHDR. The native decoder validates the complete image later.
uint2 readPNGDimensions(ByteStream stream)
{
    if (readImageHeaderBE(stream, 4) != 0x89504e47 || readImageHeaderBE(stream, 4) != 0x0d0a1a0a ||
        readImageHeaderBE(stream, 4) != 13 || readImageHeaderBE(stream, 4) != 0x49484452)
        fail("Expected a PNG image");
    uint width = readImageHeaderBE(stream, 4);
    uint height = readImageHeaderBE(stream, 4);
    return uint2(width, height);
}

/// Finds the frame header before JPEG's first scan, accepting baseline and progressive frames.
uint2 readJPEGDimensions(ByteStream stream)
{
    if (readImageHeaderBE(stream, 2) != 0xffd8)
        fail("Expected a JPEG image");
    for (;;)
    {
        if (readImageHeaderByte(stream) != 0xff)
            fail("Invalid JPEG marker");
        uint marker = readImageHeaderByte(stream);
        while (marker == 0xff)
            marker = readImageHeaderByte(stream);
        if (marker == 0xda || marker == 0xd9)
            fail("JPEG has no frame header");
        if (marker == 0x01)
            continue;
        if (marker == 0 || (marker >= 0xd0 && marker <= 0xd8))
            fail("Invalid JPEG header marker");
        uint length = readImageHeaderBE(stream, 2);
        if (length < 2)
            fail("Invalid JPEG segment length");
        if (marker == 0xc0 || marker == 0xc1 || marker == 0xc2)
        {
            if (length < 8 || readImageHeaderByte(stream) != 8)
                fail("Expected an eight-bit JPEG frame");
            uint height = readImageHeaderBE(stream, 2);
            uint width = readImageHeaderBE(stream, 2);
            return uint2(width, height);
        }
        skipImageHeaderBytes(stream, length - 2);
    }
}

/// Reads BMP dimensions, including legacy core headers and top-down Windows images.
uint2 readBMPDimensions(ByteStream stream)
{
    if (readImageHeaderBE(stream, 2) != 0x424d)
        fail("Expected a BMP image");
    skipImageHeaderBytes(stream, 12);
    uint headerSize = readImageHeaderField<uint>(stream);
    if (headerSize == 12)
    {
        uint packedDimensions = readImageHeaderField<uint>(stream);
        return uint2(packedDimensions % 65536, packedDimensions / 65536);
    }
    if (headerSize != 40 && headerSize != 56 && headerSize != 108 && headerSize != 124)
        fail("Unsupported BMP header");
    int width = readImageHeaderField<int>(stream);
    int height = readImageHeaderField<int>(stream);
    if (width <= 0 || height == int(0x80000000))
        fail("Invalid BMP dimensions");
    return uint2(uint(width), uint(height < 0 ? -height : height));
}

/// Reads dimensions before allocating a texture. The file is reopened for the full decode, so
/// header inspection does not change the sequential decoder's stream position.
public uint2 readImageDimensions(String path, ImageFileFormat fileFormat)
{
    if (fileFormat == ImageFileFormat.EXR)
        return readEXRDimensions(path);
    ByteStream stream = openByteStream(path, ByteStreamMode.read);
    uint2 dimensions;
    switch (fileFormat)
    {
    case ImageFileFormat.PNG:
        dimensions = readPNGDimensions(stream);
        break;
    case ImageFileFormat.JPEG:
        dimensions = readJPEGDimensions(stream);
        break;
    case ImageFileFormat.BMP:
        dimensions = readBMPDimensions(stream);
        break;
    default:
        fail("Unsupported image file format");
        return uint2(0u); // Unreachable; fail() has a void signature.
    }
    closeByteStream(stream);
    if (any(dimensions == 0u) || any(dimensions > 0x7fffffffu))
        fail("Invalid image dimensions");
    return dimensions;
}
