// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.io.byte_stream;
__exported import slang.runtime.io.image;
import slang.runtime.io.image_header;
import slang.runtime.core.simple_assert;
import slang.runtime.resource.texture_format;
__exported import slang.runtime.resource.texture_clear;

/// Allocates a graph-owned writable texture whose declared and physical formats agree.
RWTexture2D<T, 0, format> allocateTexture2D<
    T : ITexelElement,
    let format : int,
    let MipCount : uint = 1>(uint width, uint height)
{
    uint one = 1;
    return hgbc_asm
    {
    result:
        $$RWTexture2D<T, 0, format> = OpAllocateResource Texture Texture2D $format DeviceLocal
            UnorderedAccess $$T $width $height $one $MipCount 1 0 ShaderResource UnorderedAccess
                CopySource CopyDestination RenderTarget;
    };
}

// Allocates a graph-owned writable 2D texture using the element type's natural storage format.
RWTexture2D<T> allocateTexture2D<T : ITexelElement>(uint width, uint height)
{
    uint one = 1;
    return hgbc_asm
    {
    result:
        $$RWTexture2D<T> = OpAllocateResource Texture Texture2D $__getHGBCNaturalTextureFormat<T>()
            DeviceLocal UnorderedAccess $$T $width $height $one 1 1 0 ShaderResource UnorderedAccess
                CopySource CopyDestination RenderTarget;
    };
}

/// Allocates a graph-owned read-only texture. CopyDestination permits host upload without exposing
/// writable shader access.
Texture2D<T, 0, format> allocateReadOnlyTexture2D<
    T : ITexelElement,
    let format : int,
    let MipCount : uint = 1>(uint width, uint height)
{
    uint one = 1;
    return hgbc_asm
    {
    result:
        $$Texture2D<T, 0, format> =
            OpAllocateResource Texture Texture2D $format DeviceLocal ShaderResource $$T $width
                $height $one $MipCount 1 0 ShaderResource CopySource CopyDestination RenderTarget;
    };
}

Texture2D<T> allocateReadOnlyTexture2D<T : ITexelElement>(uint width, uint height)
{
    uint one = 1;
    return hgbc_asm
    {
    result:
        $$Texture2D<T> = OpAllocateResource Texture Texture2D $__getHGBCNaturalTextureFormat<T>()
            DeviceLocal ShaderResource $$T $width $height $one 1 1 0 ShaderResource CopySource
                CopyDestination RenderTarget;
    };
}

extension<T : ITexelElement> RWTexture2D<T>
{
    static This alloc(uint width, uint height)
    {
        return allocateTexture2D<T>(width, height);
    }
}

extension<T : ITexelElement, let format : int> RWTexture2D<T, 0, format>
{
    /// Returns the dimensions of an already allocated graph-owned texture.
    uint2 getDimensions()
    {
        uint width;
        uint height;
        this.GetDimensions(width, height);
        return uint2(width, height);
    }
}

extension<T : ITexelElement> Texture2D<T>
{
    static This alloc(uint width, uint height)
    {
        return allocateReadOnlyTexture2D<T>(width, height);
    }
}

extension<T : ITexelElement, let format : int> Texture2D<T, 0, format>
{
    uint2 getDimensions()
    {
        uint width;
        uint height;
        this.GetDimensions(width, height);
        return uint2(width, height);
    }
}

/// Reads an image into a caller-allocated RGBA32Float texture with matching dimensions.
void loadTexture2D<let Format : int>(
    String path,
    RWTexture2D<float4, 0, Format> destination,
    ImageFileFormat fileFormat = ImageFileFormat.EXR)
{
    ByteStream stream = openByteStream(path, ByteStreamMode::read);
    uint bytesRead = decodeImage(stream, destination, fileFormat);
    closeByteStream(stream);
    if (bytesRead == 0)
        fail("Image reader did not consume an image");
}

/// Allocates and loads an image whose expected dimensions are known by the caller.
RWTexture2D<float4> loadTexture2D(
    String path,
    uint2 dimensions,
    ImageFileFormat fileFormat = ImageFileFormat.EXR)
{
    RWTexture2D<float4> result = allocateTexture2D<float4>(dimensions.x, dimensions.y);
    loadTexture2D(path, result, fileFormat);
    return result;
}

/// Allocates a float texture using the image header and decodes its pixels.
RWTexture2D<float4> loadTexture2D(String path, ImageFileFormat fileFormat = ImageFileFormat.EXR)
{
    return loadTexture2D(path, readImageDimensions(path, fileFormat), fileFormat);
}

/// Writes a texture using the selected file encoding, regardless of the path extension.
/// EXR preserves HDR values; PNG/JPEG/BMP clamp to eight bits without gamma conversion.
void storeTexture2D<let Format : int>(
    String path,
    RWTexture2D<float4, 0, Format> texture,
    ImageFileFormat fileFormat = ImageFileFormat.EXR)
{
    ByteStream stream = openByteStream(path, ByteStreamMode::write);
    encodeImage(texture, stream, fileFormat);
    closeByteStream(stream);
}

/// Allocates an ordinary depth texture, readable by shaders and writable by clears and raster
/// draws.
Texture2D<float, 0, Format> allocateDepthTarget2D<
    let Format : int = TextureFormat.d32Float,
    let MipCount : uint = 1>(uint width, uint height)
{
    static_assert(
        Format == TextureFormat.d32Float || Format == TextureFormat.d16Unorm ||
            Format == TextureFormat.d32FloatS8Uint,
        "Expected a depth/stencil texture format.");
    uint one = 1;
    return hgbc_asm
    {
    result:
        $$Texture2D<float, 0, Format> =
            OpAllocateResource Texture Texture2D $Format DeviceLocal DepthWrite $$float $width
                $height $one $MipCount 1 0 ShaderResource DepthStencil CopySource CopyDestination;
    };
}

extension DepthTarget2D
{
    static This alloc(uint width, uint height)
    {
        return allocateDepthTarget2D(width, height);
    }
}

// Blits the complete source texture into the destination, scaling and converting formats as needed.
void blitTexture2D<
    TDestination : ITexelElement,
    let destinationFormat : int,
    TSource : ITexelElement,
    let sourceFormat : int>(
    RWTexture2D<TDestination, 0, destinationFormat> destination,
    RWTexture2D<TSource, 0, sourceFormat> source)
{
    hgbc_asm
    {
        OpBlitTexture2D $$RWTexture2D<TDestination, 0, destinationFormat>
            $$RWTexture2D<TSource, 0, sourceFormat> $destination $source;
    };
}

/// Blits into a presentation target while preserving its explicit physical storage format.
///
/// Consider an RGBA8 sRGB surface and an RGBA32Float render texture: the staged operation knows
/// both endpoint formats, while the fragment shader still reads and writes ordinary typed values.
void blitTexture2D<
    TDestination : ITexelElement,
    let destinationFormat : int,
    TSource : ITexelElement,
    let sourceFormat : int>(
    RenderTarget2D<TDestination, destinationFormat> destination,
    RWTexture2D<TSource, 0, sourceFormat> source)
{
    hgbc_asm
    {
        OpBlitTexture2D $$RenderTarget2D<TDestination, destinationFormat>
            $$RWTexture2D<TSource, 0, sourceFormat> $destination $source;
    };
}
