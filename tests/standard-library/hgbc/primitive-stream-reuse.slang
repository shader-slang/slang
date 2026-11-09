// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.runtime.raster.primitive_stream;
import slang.runtime.raster.render_targets;
import slang.runtime.resource.texture_format;

struct Varyings
{
    float4 position : SV_Position;
}

[host]
void main(
    RWTexture2D<float4, 0, TextureFormat::rgba32Float> target,
    RWStructuredBuffer<float4> vertices,
    StructuredBuffer<uint> indices,
    float4 tint)
{
    let vertex = (VertexInvocation v) => Varyings(vertices[v.vertexID]);
    let fragment = (Varyings v) => tint;
    let targets = RenderTargets(target);
    let stream = Primitives<Topology.TriangleList>(VertexRange(3)).vertex(vertex);
    stream.instances(2, 5).draw(targets, fragment);
    stream.instances(4, 9).draw(targets, fragment);
    Primitives<Topology.TriangleList>(IndexRange(indices, 0, 3))
        .vertex(vertex)
        .instances(3, 7)
        .draw(targets, fragment);
}

// Direct/indexed assembly, instance ranges, and captured values share the same pipeline.
// CHECK: Section("FUNC")
// CHECK: OpCall [[PIPELINE:%id[0-9]+]]
// CHECK: OpCall [[PIPELINE]]
// CHECK: Section("EMBD")
// CHECK: [[PIPELINE]] {{.*}}OpMakeGraphicsPipeline
// CHECK-NOT: OpMakeGraphicsPipeline
