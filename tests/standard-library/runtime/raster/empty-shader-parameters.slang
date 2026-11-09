// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime;

// The fragment input has no host bindings even though it contains a float4. Reflection can
// report category None for this system-value-only struct as well as for an empty uniform closure.
struct Varyings
{
    float4 position : SV_Position;
}

[host]
void checkColor<let Format : int>(RWTexture2D<float4, 0, Format> texture, float4 expected)
{
    let result = RWStructuredBuffer<uint>::alloc(16);
    Grid(uint2(4, 4))
        .dispatch(
            (uint2 p) =>
            { result[p.x + 4 * p.y] = all(texture[p] == expected) ? 1u : 0u; });
    for (uint i = 0; i < 16; ++i)
        assert(result.readBack(i) == 1u);
}

[host]
void main()
{
    let color = allocateTexture2D<float4, TextureFormat.rgba32Float>(4, 4);
    let targets = RenderTargets(color);
    let stream = Primitives<Topology.TriangleList>(VertexRange(3))
                     .vertex(
                         (VertexInvocation v) =>
                         {
                             return Varyings(
                                 float4(v.vertexID == 1 ? 3 : -1, v.vertexID == 2 ? 3 : -1, 0, 1));
                         });

    // Both callbacks are empty, but the vertex adapter still carries draw data.
    stream.draw(targets, (Varyings v) => float4(1, 0, 0, 1));
    checkColor(color, float4(1, 0, 0, 1));

    // A changing fragment capture follows the empty vertex callback and the unbound varyings.
    for (uint i = 0; i < 2; ++i)
    {
        float4 tint = float4(0, float(i), 1, 1);
        stream.draw(targets, (Varyings v) => tint);
        checkColor(color, tint);
    }
}
