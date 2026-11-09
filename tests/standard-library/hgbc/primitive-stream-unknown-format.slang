// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.runtime.raster.primitive_stream;
import slang.runtime.raster.render_targets;

struct Varyings
{
    float4 position : SV_Position;
}

[host]
void main(RWTexture2D<float4> target)
{
    let stream = Primitives<Topology.TriangleList>(VertexRange(3))
                     .vertex(
                         (VertexInvocation v) =>
                         {
                             Varyings output = { float4(0, 0, 0, 1) };
                             return output;
                         });
    stream.draw(RenderTargets(target), (Varyings v) => float4(1, 0, 0, 1));
    // CHECK: error[E56006]: HGBC target does not support color targets with a statically known
    // texture format yet
}
