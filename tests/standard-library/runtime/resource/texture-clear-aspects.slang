// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.resource.simple_texture;
import slang.runtime.core.simple_assert;
import slang.runtime.compute.grid;
import slang.runtime.raster.primitive_stream;
import slang.runtime.raster.raster_state;
import slang.runtime.raster.render_targets;
import slang.runtime.raster.draw_parameters;

struct Varyings
{
    float4 position : SV_Position;
}

typealias StencilEqual =
    DepthStencilOpState<StencilOp.Keep, StencilOp.Keep, StencilOp.Keep, ComparisonFunc.Equal>;
typealias TestBoth = RasterState<
    RasterizerState<>,
    DepthStencilState<true, false, ComparisonFunc.Less, true, 255, 0, StencilEqual, StencilEqual>>;

[host]
void checkColor<let Format : int>(RWTexture2D<float4, 0, Format> texture, float4 expected)
{
    let checks = RWStructuredBuffer<uint>::alloc(64);
    Grid(uint2(8u)).dispatch(
        (uint2 p) =>
        { checks[p.x + 8 * p.y] = all(texture[p] == expected) ? 1 : 0; });
    for (uint i = 0; i < 64; ++i)
        assert(readBack(checks, i) == 1);
}

[host]
void main()
{
    let color = allocateTexture2D<float4, TextureFormat.rgba32Float>(8, 8);
    let depth = allocateDepthTarget2D<TextureFormat.d32FloatS8Uint>(8, 8);
    let targets = RenderTargets(color, depth);
    let stream =
        Primitives<Topology.TriangleList>(VertexRange(3))
            .vertex(
                (VertexInvocation v) =>
                {
                    Varyings result = {
                        float4(v.vertexID == 1 ? 3.0 : -1.0, v.vertexID == 2 ? 3.0 : -1.0, 0.5, 1)
                    };
                    return result;
                });
    let red = (Varyings v) => float4(1, 0, 0, 1);
    let green = (Varyings v) => float4(0, 1, 0, 1);
    var parameters = DrawParameters();
    parameters.stencilRef = 7;
    color.clear(float4(0.0));
    depth.clear(DepthStencilValue(0.25, 7));
    stream.draw<TestBoth>(targets, red, parameters);
    checkColor(color, float4(0.0)); // Depth rejects the fragment.

    depth.clear<TextureAspect.DepthOnly>(0.75);
    stream.draw<TestBoth>(targets, red, parameters);
    checkColor(color, float4(1, 0, 0, 1)); // Stencil 7 survived the depth clear.

    depth.clear<TextureAspect.StencilOnly>(3u);
    stream.draw<TestBoth>(targets, green, parameters);
    checkColor(color, float4(1, 0, 0, 1)); // The old stencil value is gone.
    parameters.stencilRef = 3;
    stream.draw<TestBoth>(targets, green, parameters);
    checkColor(color, float4(0, 1, 0, 1)); // Depth 0.75 survived the stencil clear.

    depth.clear(DepthStencilValue(0.25, 9));
    parameters.stencilRef = 9;
    stream.draw<TestBoth>(targets, red, parameters);
    checkColor(color, float4(0, 1, 0, 1));
    depth.clear<TextureAspect.DepthOnly>(0.75);
    stream.draw<TestBoth>(targets, red, parameters);
    checkColor(color, float4(1, 0, 0, 1)); // The combined clear also replaced stencil.
}
