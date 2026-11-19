// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

// `Type::alloc` allocates every 2D texture type: an unspecified format stores the element type's
// natural format, an explicit format is kept, and a depth format yields a depth target.

import slang.runtime;

struct Varyings
{
    float4 position : SV_Position;
}

typealias DepthState =
    RasterState<RasterizerState<>, DepthStencilState<true, true, ComparisonFunc.Less>>;

/// Writes `value` into every texel and returns what one texel then reads back.
[host]
float storeAndLoad<let Format : int>(RWTexture2D<float4, 0, Format> texture, float value)
{
    Grid(uint2(4, 4)).dispatch((uint2 p) => { texture[p] = float4(value); });
    let result = RWStructuredBuffer<float>::alloc(1);
    Grid(uint2(1, 1)).dispatch((uint2 p) => { result[0] = texture[uint2(2, 1)].x; });
    return result.readBack(0);
}

/// Draws a near and then a farther full-screen triangle; with a working depth target only the
/// near one is visible.
[host]
void checkDepthTarget<let Format : int>(Texture2D<float, 0, Format> depth)
{
    // Draw color targets need a statically known format.
    let color = RWTexture2D<float, 0, TextureFormat.r32Float>::alloc(4, 4);
    let targets = RenderTargets(color, depth);
    targets.clear(0.0, 1.0);
    for (uint pass = 0; pass < 2; ++pass)
    {
        float z = pass == 0 ? 0.25 : 0.75;
        float value = pass == 0 ? 1.0 : 2.0;
        Primitives<Topology.TriangleList>(VertexRange(3))
            .vertex(
                (VertexInvocation v) => Varyings(
                    float4(v.vertexID == 1 ? 3.0 : -1.0, v.vertexID == 2 ? 3.0 : -1.0, z, 1)))
            .draw<DepthState>(targets, (Varyings v) => value);
    }
    let result = RWStructuredBuffer<float>::alloc(1);
    Grid(uint2(1, 1)).dispatch((uint2 p) => { result[0] = color[uint2(1, 2)]; });
    assert(result.readBack(0) == 1.0);
}

[host]
void main()
{
    // The natural format of float4 is rgba32Float, which stores the float value unchanged.
    let natural = RWTexture2D<float4>::alloc(4, 4);
    assert(natural.getDimensions().x == 4 && natural.getDimensions().y == 4);
    assert(storeAndLoad(natural, 0.3) == 0.3);

    // An explicit rgba8Unorm format quantizes 0.25 to 64 / 255.
    let quantized = RWTexture2D<float4, 0, TextureFormat.rgba8Unorm>::alloc(4, 4);
    float stored = storeAndLoad(quantized, 0.25);
    assert(stored > 63.5 / 255.0 && stored < 64.5 / 255.0 && stored != 0.25);

    let readOnly = Texture2D<float4, 0, TextureFormat.rgba8Unorm>::alloc(3, 5);
    assert(readOnly.getDimensions().x == 3 && readOnly.getDimensions().y == 5);

    checkDepthTarget(DepthTarget2D::alloc(4, 4));
    checkDepthTarget(Texture2D<float, 0, TextureFormat.d16Unorm>::alloc(4, 4));
}
