// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.runtime.raster.primitive_stream;
import slang.runtime.raster.render_targets;
import slang.runtime.resource.texture_format;

struct Varyings
{
    float4 position : SV_Position;
}
struct InvalidColor
{
    float4 value;
}

[host]
void main(RWTexture2D<float4, 0, TextureFormat::rgba32Float> target)
{
    Primitives<Topology.TriangleList>(VertexRange(3))
        .vertex((VertexInvocation v) => Varyings(float4(0)))
        .draw(RenderTargets(target), (Varyings v) => InvalidColor(float4(1)));
}

// CHECK: no overload for 'draw' applicable to arguments of type
// CHECK: .draw(RenderTargets(target), (Varyings v) => InvalidColor(float4(1)))
