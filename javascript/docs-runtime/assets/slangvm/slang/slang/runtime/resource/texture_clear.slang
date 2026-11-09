// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

__exported import slang.runtime.resource.texture_format;

/// Selects the aspects to clear. Selection is static and determines the clear value's type.
public enum TextureAspect
{
    All = 0,
    DepthOnly = 1,
    StencilOnly = 2,
}

/// Selects whole subresources. A count of 0xffffffffu means all remaining mips or layers.
/// Empty ranges do nothing; ranges beyond the allocated texture are rejected.
public struct SubresourceRange
{
    public uint mip = 0;
    public uint mipCount = 0xffffffffu;
    public uint layer = 0;
    public uint layerCount = 0xffffffffu;
}

/// A host clear value for both depth and stencil, independent of the shader's depth texel type.
public struct DepthStencilValue
{
    public float depth;
    public uint stencil;

    public __init(float depth, uint stencil)
    {
        this.depth = depth;
        this.stencil = stencil;
    }
}

public extension<T : ITexelElement, let Format : int> RWTexture2D<T, 0, Format>
{
    /// Clears whole selected subresources independently of raster draws, viewports, and scissors.
    [host]
    public void clear<let Aspect : TextureAspect = TextureAspect.All>(
        T value,
        SubresourceRange range = {})
    {
        static_assert(Aspect == TextureAspect.All, "Color clears select all aspects.");
        static_assert(
            Format != TextureFormat.d32Float && Format != TextureFormat.d16Unorm &&
                Format != TextureFormat.d32FloatS8Uint,
            "Depth/stencil storage uses Texture2D.");
        hgbc_asm
        {
            OpClearTexture $$This $this $uint(Aspect)
                $range.mip $range.mipCount $range.layer $range.layerCount $value;
        };
    }
}

public extension<T : ITexelElement, let Format : int> Texture2D<T, 0, Format>
{
    /// Shader read-only access does not prevent host clears or raster writes.
    [host]
    public void clear<let Aspect : TextureAspect = TextureAspect.All>(
        T value,
        SubresourceRange range = {})
    {
        static_assert(
            ((Format == TextureFormat.d32Float || Format == TextureFormat.d16Unorm) &&
             (Aspect == TextureAspect.All || Aspect == TextureAspect.DepthOnly)) ||
                (Format == TextureFormat.d32FloatS8Uint && Aspect == TextureAspect.DepthOnly) ||
                (Format != TextureFormat.d32Float && Format != TextureFormat.d16Unorm &&
                 Format != TextureFormat.d32FloatS8Uint && Aspect == TextureAspect.All),
            "Clear value must match the selected texture aspects.");
        hgbc_asm
        {
            OpClearTexture $$This $this $uint(Aspect)
                $range.mip $range.mipCount $range.layer $range.layerCount $value;
        };
    }
}

public extension<let Format : int> Texture2D<float, 0, Format>
{
    [host]
    public void clear<let Aspect : TextureAspect = TextureAspect.All>(
        uint value,
        SubresourceRange range = {})
    {
        static_assert(
            Format == TextureFormat.d32FloatS8Uint && Aspect == TextureAspect.StencilOnly,
            "A uint clear value selects stencil on a depth/stencil texture.");
        hgbc_asm
        {
            OpClearTexture $$This $this $uint(Aspect)
                $range.mip $range.mipCount $range.layer $range.layerCount $value;
        };
    }

    [host]
    public void clear<let Aspect : TextureAspect = TextureAspect.All>(
        DepthStencilValue value,
        SubresourceRange range = {})
    {
        static_assert(
            Format == TextureFormat.d32FloatS8Uint && Aspect == TextureAspect.All,
            "DepthStencilValue clears both aspects of a depth/stencil texture.");
        hgbc_asm
        {
            OpClearTexture $$This $this $uint(Aspect) $range.mip $range.mipCount $range.layer $range
                .layerCount $value.depth $value.stencil;
        };
    }
}
