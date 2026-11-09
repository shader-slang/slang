// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.resource.texture_clear;

/// Preserves the texture type and its shader value type when grouping raster color outputs.
public interface IRenderColorTarget
{
    associatedtype ClearValue;
    void clear<let Aspect : TextureAspect = TextureAspect.All>(
        ClearValue value,
        SubresourceRange range = {});
}

public extension<T : ITexelElement, let Format : int> RWTexture2D<T, 0, Format> : IRenderColorTarget
{
    public typealias ClearValue = T;
}

public struct NoDepthTarget
{
}

/// Groups ordered color textures and an optional depth texture without copying format metadata.
public struct RenderTargetSet<Colors, Depth>
{
    public Colors colors;
    public Depth depth;
}

public extension<Color : IRenderColorTarget> RenderTargetSet<Tuple<Color>, NoDepthTarget>
{
    [host]
    public void clear(Color.ClearValue color)
    {
        colors._0.clear(color);
    }
}

public extension<Color : IRenderColorTarget, let Format : int>
    RenderTargetSet<Tuple<Color>, Texture2D<float, 0, Format>>
{
    [host]
    public void clear(Color.ClearValue color, float depthValue)
    {
        colors._0.clear(color);
        depth.clear<TextureAspect.DepthOnly>(depthValue);
    }
}

public RenderTargetSet<Tuple<Color>, NoDepthTarget> RenderTargets<Color : IRenderColorTarget>(
    Color color)
{
    return { makeTuple(color), {} };
}

public RenderTargetSet<Tuple<Color>, Texture2D<float, 0, Format>> RenderTargets<
    Color : IRenderColorTarget,
    let Format : int>(Color color, Texture2D<float, 0, Format> depth)
{
    return { makeTuple(color), depth };
}
