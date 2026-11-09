// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.runtime.raster.primitive_stream;
import slang.runtime.raster.render_targets;
import slang.runtime.raster.raster_state;
import slang.runtime.resource.texture_format;

struct Varyings
{
    float4 position : SV_Position;
}
typealias DepthState = RasterState<RasterizerState<>, DepthStencilState<true, true>>;

void drawEmpty<
    let Format : int,
    Vertex : IFunc<Varyings, VertexInvocation>,
    Fragment : IFunc<float4, Varyings>>(
    RWTexture2D<float4, 0, Format> target,
    DepthTarget2D depth,
    StructuredBuffer<uint> indices,
    Vertex vertex,
    Fragment fragment)
{
    Primitives<Topology.TriangleList>(IndexRange(indices, 0, 3))
        .vertex(vertex)
        .draw<DepthState>(RenderTargets(target, depth), fragment);
}

[host]
void main(
    RWTexture2D<float4, 0, TextureFormat::rgba32Float> target,
    DepthTarget2D depth,
    StructuredBuffer<uint> indices)
{
    drawEmpty(
        target,
        depth,
        indices,
        (VertexInvocation v) => Varyings(float4(float(v.vertexID), 0, 0, 1)),
        (Varyings v) => float4(1.0));
}

// Both callbacks are empty. Legalization removes their physical closure fields while preserving
// the vertex adapter's draw-ID fields and matching the graphics callable to its ordinary call.
// CHECK: Section("FUNC")
// CHECK: OpCall [[PIPELINE:%id[0-9]+]]
// CHECK: Section("EMBD")
// CHECK: [[PIPELINE]] {{.*}}OpMakeGraphicsPipeline
