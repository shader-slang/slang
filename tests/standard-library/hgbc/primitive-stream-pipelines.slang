// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry direct
// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry indexed
// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry directDepth
// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry indexedDepth

import slang.runtime.raster.primitive_stream;
import slang.runtime.raster.raster_state;
import slang.runtime.raster.render_targets;
import slang.runtime.resource.texture_format;

typealias Target = RWTexture2D<float4, 0, TextureFormat::rgba32Float>;
typealias DepthState = RasterState<RasterizerState<>, DepthStencilState<true, true>>;

struct Varyings
{
    float4 position : SV_Position;
}

// Forwarding the source through a helper preserves its typed index buffer and draw range.
Source forwardSource<Source : IPrimitiveSource>(Source source)
{
    return source;
}

[host]
void direct(Target target, RWStructuredBuffer<float4> vertices)
{
    Primitives<Topology.TriangleList>(forwardSource(VertexRange(3)))
        .vertex((VertexInvocation v) => Varyings(vertices[v.vertexID]))
        .draw(RenderTargets(target), (Varyings v) => float4(1.0));
}

[host]
void indexed(Target target, RWStructuredBuffer<float4> vertices, StructuredBuffer<uint> indices)
{
    Primitives<Topology.TriangleList>(forwardSource(IndexRange(indices, 0, 3)))
        .vertex((VertexInvocation v) => Varyings(vertices[v.vertexID]))
        .draw(RenderTargets(target), (Varyings v) => float4(1.0));
}

[host]
void directDepth(Target target, DepthTarget2D depth, RWStructuredBuffer<float4> vertices)
{
    Primitives<Topology.TriangleList>(VertexRange(3))
        .vertex((VertexInvocation v) => Varyings(vertices[v.vertexID]))
        .draw<DepthState>(RenderTargets(target, depth), (Varyings v) => float4(1.0));
}

[host]
void indexedDepth(
    Target target,
    DepthTarget2D depth,
    RWStructuredBuffer<float4> vertices,
    StructuredBuffer<uint> indices)
{
    Primitives<Topology.TriangleList>(IndexRange(indices, 0, 3))
        .vertex((VertexInvocation v) => Varyings(vertices[v.vertexID]))
        .draw<DepthState>(RenderTargets(target, depth), (Varyings v) => float4(1.0));
}

// Draw construction occurs in host code; immutable pipeline state is emitted globally.
// CHECK: Section("FUNC")
// CHECK: OpCall [[PIPELINE:%id[0-9]+]]
// CHECK: OpMakeGraphicsDraw
// CHECK: Section("EMBD")
// CHECK: OpGraphicsPipelineState
// CHECK: [[PIPELINE]] {{.*}}OpMakeGraphicsPipeline
