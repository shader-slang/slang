// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.resource.simple_texture;
import slang.runtime.core.simple_assert;
import slang.runtime.compute.grid;

[host]
void checkMip<let Format : int>(Texture2D<float, 0, Format> texture, uint mip, float expected)
{
    let checks = RWStructuredBuffer<uint>::alloc(64);
    uint size = 8;
    for (uint level = 0; level < mip; ++level)
        size /= 2;
    Grid(uint2(size))
        .dispatch(
            (uint2 p) =>
            { checks[p.x + size * p.y] = texture.Load(int3(p, mip)) == expected ? 1 : 0; });
    for (uint i = 0; i < size * size; ++i)
        assert(readBack(checks, i) == 1);
}

[host]
void main()
{
    let texture = allocateReadOnlyTexture2D<float, TextureFormat.r32Float, 4>(8, 8);
    texture.clear(0.25);
    var range = SubresourceRange();
    range.mip = 1;
    range.mipCount = 2;
    texture.clear(0.5, range);
    checkMip(texture, 0, 0.25);
    checkMip(texture, 1, 0.5);
    checkMip(texture, 2, 0.5);
    checkMip(texture, 3, 0.25);

    // The same staged clear executes with different runtime bounds and values.
    range.mipCount = 1;
    for (uint mip = 0; mip < 4; ++mip)
    {
        range.mip = mip;
        texture.clear(float(mip), range);
    }
    for (uint mip = 0; mip < 4; ++mip)
        checkMip(texture, mip, float(mip));

    // All remaining subresources starts at the selected mip, rather than at mip zero.
    range.mip = 2;
    range.mipCount = 0xffffffffu;
    texture.clear(8.0, range);
    checkMip(texture, 1, 1.0);
    checkMip(texture, 2, 8.0);
    checkMip(texture, 3, 8.0);

    range.mipCount = 0;
    texture.clear(99.0, range);
    range.mip = 4;
    range.mipCount = 0xffffffffu;
    texture.clear(99.0, range);
    range.mip = 0;
    range.layerCount = 0;
    texture.clear(99.0, range);
    checkMip(texture, 0, 0.0);
    checkMip(texture, 3, 8.0);

    // Depth is an ordinary shader-readable float texture with an explicit storage format.
    let depth = allocateDepthTarget2D<TextureFormat.d32Float, 4>(8, 8);
    depth.clear(0.25);
    range = SubresourceRange();
    range.mip = 1;
    range.mipCount = 1;
    depth.clear<TextureAspect.DepthOnly>(0.75, range);
    checkMip(depth, 0, 0.25);
    checkMip(depth, 1, 0.75);
    checkMip(depth, 2, 0.25);

    let depth16 = allocateDepthTarget2D<TextureFormat.d16Unorm>(8, 8);
    depth16.clear(1.0);
    checkMip(depth16, 0, 1.0);
}
