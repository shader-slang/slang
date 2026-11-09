// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.raster.render_targets;
import slang.runtime.compute.grid;
import slang.runtime.resource.simple_texture;
import slang.runtime.resource.texture_format;
import slang.runtime.core.simple_assert;

[host]
void checkTexture<T : ITexelElement & IComparable, let Format : int>(
    RWTexture2D<T, 0, Format> texture,
    T expected)
{
    let result = RWStructuredBuffer<uint>::alloc(35);
    Grid(uint2(7, 5))
        .dispatch(
            (uint2 p) =>
            { result[p.x + p.y * 7] = texture[p].equals(expected) ? 1 : 0; });
    for (uint i = 0; i < 35; ++i)
        assert(readBack(result, i) == 1);
}

[host]
void main()
{
    let color = allocateTexture2D<float4, TextureFormat.rgba32Float>(7, 5);
    let depth = allocateDepthTarget2D(7, 5);
    let targets = RenderTargets(color, depth);
    targets.clear(float4(0.25, 0.5, 0.75, 1), 0.5);
    // A compute read forces the clear to be visible without any raster draw.
    checkTexture(color, float4(0.25, 0.5, 0.75, 1));
    color.clear(float4(1, 0, 0, 1));
    checkTexture(color, float4(1, 0, 0, 1));
    depth.clear(1.0);

    // Values above float's exact integer range and negative values preserve all 32 bits.
    let integers = allocateTexture2D<uint4, TextureFormat.rgba32Uint>(7, 5);
    RenderTargets(integers).clear(uint4(0xffffffffu, 0x12345678u, 17, 0));
    checkTexture(integers, uint4(0xffffffffu, 0x12345678u, 17, 0));
    let signedIntegers = allocateTexture2D<int4, TextureFormat.rgba32Sint>(7, 5);
    RenderTargets(signedIntegers).clear(int4(-3, -123456789, 37, 0));
    checkTexture(signedIntegers, int4(-3, -123456789, 37, 0));

    // Clearing is independent of raster output arity.
    let scalar = allocateTexture2D<uint, TextureFormat.r32Uint>(7, 5);
    scalar.clear(0xffffffffu);
    checkTexture(scalar, 0xffffffffu);
    let pair = allocateTexture2D<float2, TextureFormat.rg32Float>(7, 5);
    pair.clear(float2(0.25, 0.75));
    checkTexture(pair, float2(0.25, 0.75));
}
