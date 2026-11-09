// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.raster.primitive_stream;
import slang.runtime.raster.raster_state;
import slang.runtime.raster.render_targets;
import slang.runtime.raster.draw_parameters;
import slang.runtime.compute.grid;
import slang.runtime.resource.simple_texture;
import slang.runtime.resource.texture_format;
import slang.runtime.core.simple_assert;

struct Varyings
{
    float4 position : SV_Position;
}

struct InstancedVaryings
{
    float4 position : SV_Position;
    nointerpolation uint instance : INSTANCE;
}

typealias DepthState =
    RasterState<RasterizerState<>, DepthStencilState<true, true, ComparisonFunc.Less>>;
typealias ScissorState = RasterState<
    RasterizerState<FillMode.Solid, CullMode.None, FrontFaceMode.CounterClockwise, 0, true, true>>;
typealias AdditiveState = RasterState<
    RasterizerState<>,
    DepthStencilState<>,
    ColorTargetStates<ColorTargetState<
        true,
        LogicOp.NoOp,
        RenderTargetWriteMask.All,
        AspectBlendState<BlendFactor.One, BlendFactor.One>,
        AspectBlendState<BlendFactor.One, BlendFactor.One>>>>;

[host]
void checkColor<let Format : int>(RWTexture2D<float4, 0, Format> texture, float4 expected)
{
    let result = RWStructuredBuffer<uint>::alloc(64);
    Grid(uint2(8, 8))
        .dispatch(
            (uint2 p) =>
            { result[p.x + 8 * p.y] = all(texture[p] == expected) ? 1 : 0; });
    for (uint i = 0; i < 64; ++i)
        assert(readBack(result, i) == 1);
}

[host]
void main()
{
    let color = allocateTexture2D<float4, TextureFormat.rgba32Float>(8, 8);
    let targets = RenderTargets(color);
    targets.clear(float4(0, 0, 1, 1));
    let stream = Primitives<Topology.TriangleList>(VertexRange(3))
                     .vertex(
                         (VertexInvocation v) =>
                         {
                             float x = v.vertexID == 1 ? 3.0 : -1.0;
                             float y = v.vertexID == 2 ? 3.0 : -1.0;
                             Varyings result = { float4(x, y, 0.25, 1) };
                             return result;
                         });
    let red = (Varyings v) => float4(1, 0, 0, 1);
    stream.draw(targets, red);
    checkColor(color, float4(1, 0, 0, 1));

    // Zero work must preserve the target, including a zero instance count on a nonempty source.
    let empty = Primitives<Topology.TriangleList>(VertexRange(0)).vertex(stream.vertexShader);
    let green = (Varyings v) => float4(0, 1, 0, 1);
    empty.draw(targets, green);
    stream.instances(0).draw(targets, green);
    checkColor(color, float4(1, 0, 0, 1));

    // Viewport and scissor are independent draw values. Both preserve pixels outside their area.
    color.clear(float4(0, 0, 1, 1));
    let viewport = DrawParameters().withViewport(Viewport(2, 2, 4, 4));
    stream.draw(targets, red, viewport);
    let scissored = viewport.withScissor(ScissorRect(3, 3, 5, 5));
    stream.draw<ScissorState>(targets, green, scissored);
    let pixels = RWStructuredBuffer<uint>::alloc(64);
    Grid(uint2(8, 8))
        .dispatch(
            (uint2 p) =>
            {
                bool inViewport = all(p >= uint2(2)) && all(p < uint2(6));
                bool inScissor = all(p >= uint2(3)) && all(p < uint2(5));
                float4 expected = inScissor    ? float4(0, 1, 0, 1)
                                  : inViewport ? float4(1, 0, 0, 1)
                                               : float4(0, 0, 1, 1);
                pixels[p.x + 8 * p.y] = all(color[p] == expected) ? 1 : 0;
            });
    for (uint p = 0; p < 64; ++p)
        assert(readBack(pixels, p) == 1);

    // Separate submissions share depth. Changing capture values must reuse the specialization.
    let depth = allocateDepthTarget2D(8, 8);
    let depthTargets = RenderTargets(color, depth);
    depthTargets.clear(float4(0, 0, 1, 1), 1.0);
    for (uint pass = 0; pass < 3; ++pass)
    {
        float z = pass == 0 ? 0.25 : pass == 1 ? 0.75 : 0.125;
        float4 tint = pass == 0 ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1);
        let layered =
            Primitives<Topology.TriangleList>(VertexRange(3))
                .vertex(
                    (VertexInvocation v) =>
                    {
                        Varyings result = {
                            float4(v.vertexID == 1 ? 3.0 : -1.0, v.vertexID == 2 ? 3.0 : -1.0, z, 1)
                        };
                        return result;
                    });
        layered.draw<DepthState>(depthTargets, (Varyings v) => tint);
        // Readback forces a native submission/pass break between the depth-tested draws.
        checkColor(color, pass < 2 ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1));
    }

    // Vertex resources are captures, independent of the index source. Compute-written indices
    // must transition to index reads without host readback. Both ranges address vertices 2..4.
    let positions = RWStructuredBuffer<float4>::alloc(5);
    let indices = RWStructuredBuffer<uint>::alloc(5);
    let tints = RWStructuredBuffer<float4>::alloc(1);
    Grid(5).dispatch(
        (uint i) =>
        {
            positions[i] = float4(i == 3 ? 3.0 : -1.0, i == 4 ? 3.0 : -1.0, 0.25, 1);
            indices[i] = i >= 2 ? i - 2 : 0xffffffffu;
            if (i == 0)
                tints[0] = float4(1, 2, 3, 1);
        });
    let capturedVertex = (VertexInvocation v) =>
    {
        InstancedVaryings result = { positions[v.vertexID], v.instanceID };
        return result;
    };
    float scale = 2;
    let capturedFragment = (InstancedVaryings v) => tints[0] * (float(v.instance) * scale);
    color.clear(float4(0.0));
    Primitives<Topology.TriangleList>(VertexRange(3, 2))
        .instances(2, 7)
        .vertex(capturedVertex)
        .draw<AdditiveState>(targets, capturedFragment);
    checkColor(color, float4(30, 60, 90, 30));
    color.clear(float4(0.0));
    Primitives<Topology.TriangleList>(IndexRange(indices, 2, 3, 2))
        .instances(2, 7)
        .vertex(capturedVertex)
        .draw<AdditiveState>(targets, capturedFragment);
    checkColor(color, float4(30, 60, 90, 30));
}
