// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.io.byte_stream;
import slang.runtime.core.simple_assert;

/// Selects the file encoding independently of the texture's physical format and path extension.
public enum ImageFileFormat
{
    EXR,
    PNG,
    JPEG,
    JPG = JPEG,
    BMP,
}

/// Decodes exactly one image into an RGBA32Float texture with matching dimensions.
/// PNG/JPEG/BMP channels are normalized without gamma conversion; absent alpha becomes one.
public uint decodeImage<let Format : int>(
    ByteStream source,
    RWTexture2D<float4, 0, Format> destination,
    ImageFileFormat fileFormat)
{
    switch (fileFormat)
    {
    case ImageFileFormat.EXR:
        return decodeImageEXR(source, destination);
    case ImageFileFormat.PNG:
        return hgbc_asm
        {
        result:
            $$uint = OpImageDecode PNG ByteStream DeviceTexture2D $source $destination;
        };
    case ImageFileFormat.JPEG:
        return hgbc_asm
        {
        result:
            $$uint = OpImageDecode JPEG ByteStream DeviceTexture2D $source $destination;
        };
    case ImageFileFormat.BMP:
        return hgbc_asm
        {
        result:
            $$uint = OpImageDecode BMP ByteStream DeviceTexture2D $source $destination;
        };
    default:
        fail("Unsupported image file format");
        return 0; // Unreachable; fail() has a void signature rather than a non-returning type.
    }
}

/// Encodes one image, preserving HDR values in EXR and clamping other formats to eight bits.
/// No gamma transform is applied. JPEG uses quality 90 and discards alpha.
public uint encodeImage<let Format : int>(
    RWTexture2D<float4, 0, Format> source,
    ByteStream destination,
    ImageFileFormat fileFormat)
{
    switch (fileFormat)
    {
    case ImageFileFormat.EXR:
        return encodeImageEXR(source, destination);
    case ImageFileFormat.PNG:
        return encodeImagePNG(source, destination);
    case ImageFileFormat.JPEG:
        return hgbc_asm
        {
        result:
            $$uint = OpImageEncode JPEG DeviceTexture2D ByteStream $source $destination;
        };
    case ImageFileFormat.BMP:
        return hgbc_asm
        {
        result:
            $$uint = OpImageEncode BMP DeviceTexture2D ByteStream $source $destination;
        };
    default:
        fail("Unsupported image file format");
        return 0; // Unreachable; fail() has a void signature rather than a non-returning type.
    }
}

/// Decodes one sequential EXR image into an existing two-dimensional texture.
///
/// The stream remains open and positioned immediately after the encoded image. For example, two
/// concatenated images can be decoded by calling this function twice with the same stream.
public uint decodeImageEXR<let Format : int>(
    ByteStream source,
    RWTexture2D<float4, 0, Format> destination)
{
    return hgbc_asm
    {
    result:
        $$uint = OpImageDecode EXR ByteStream DeviceTexture2D $source $destination;
    };
}

/// Encodes an existing two-dimensional texture as sequential EXR bytes.
public uint encodeImageEXR<let Format : int>(
    RWTexture2D<float4, 0, Format> source,
    ByteStream destination)
{
    return hgbc_asm
    {
    result:
        $$uint = OpImageEncode EXR DeviceTexture2D ByteStream $source $destination;
    };
}

/// Encodes an existing two-dimensional texture as sequential PNG bytes.
public uint encodeImagePNG<T : ITexelElement, let Format : int>(
    RWTexture2D<T, 0, Format> source,
    ByteStream destination)
{
    return hgbc_asm
    {
    result:
        $$uint = OpImageEncode PNG DeviceTexture2D ByteStream $source $destination;
    };
}
