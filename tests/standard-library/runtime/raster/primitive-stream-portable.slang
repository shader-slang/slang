// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api webgpu

// Raster draws restricted to operations that WebGPU also permits: the vertex stage reads only
// read-only buffers, and results are read through a single-channel float target, which WebGPU
// allows as a read-write storage texture.

import slang.runtime;

struct FlatVaryings
{
    float4 position : SV_Position;
    nointerpolation uint instance : INSTANCE;
    nointerpolation uint firstVertex : FIRSTVERTEX;
}

struct Varyings
{
    float4 position : SV_Position;
}

typealias DepthState =
    RasterState<RasterizerState<>, DepthStencilState<true, true, ComparisonFunc.Less>>;

typealias Target = RWTexture2D<float, 0, TextureFormat.r32Float>;

[host]
void checkAll(Target texture, float expected)
{
    let hits = RWStructuredBuffer<uint>::alloc(64);
    Grid(uint2(8, 8))
        .dispatch((uint2 p) => { hits[p.x + 8 * p.y] = texture[p] == expected ? 1 : 0; });
    for (uint i = 0; i < 64; ++i)
        assert(hits.readBack(i) == 1);
}

[host]
void main()
{
    let color = allocateTexture2D<float, TextureFormat.r32Float>(8, 8);
    let targets = RenderTargets(color);

    // Render-target clears must reach the texture on every backend.
    targets.clear(0.5);
    checkAll(color, 0.5);

    // The vertex callback sees effective IDs 5..7 and instance 7 exactly once, whichever offsets
    // the backend's system values include. Flat integer varyings carry them to the fragment stage.
    targets.clear(0.0);
    Primitives<Topology.TriangleList>(VertexRange(3, 5))
        .instances(1, 7)
        .vertex(
            (VertexInvocation v) =>
            {
                uint k = v.vertexID - 5;
                FlatVaryings result = {
                    float4(k == 1 ? 3.0 : -1.0, k == 2 ? 3.0 : -1.0, 0.5, 1),
                    v.instanceID,
                    v.vertexID - k
                };
                return result;
            })
        .draw(targets, (FlatVaryings v) => float(v.instance * 100 + v.firstVertex));
    checkAll(color, 705.0);

    // Indexed draws add the base vertex once, including a negative one. Positions come from a
    // read-only StructuredBuffer, which WebGPU permits in the vertex stage.
    let cpuPositions = RuntimeArray<float4>::alloc(5);
    for (uint i = 0; i < 5; ++i)
        cpuPositions[i] = float4(i == 3 ? 3.0 : -1.0, i == 4 ? 3.0 : -1.0, 0.5, 1);
    let positions = StructuredBuffer<float4>::alloc(5);
    copy(positions, 0, cpuPositions, 0, 5);
    let cpuIndices = RuntimeArray<uint>::alloc(6);
    for (uint i = 0; i < 6; ++i)
        cpuIndices[i] = i < 3 ? i : i + 2;
    let indices = StructuredBuffer<uint>::alloc(6);
    copy(indices, 0, cpuIndices, 0, 6);
    let vertexShader = (VertexInvocation v) => Varyings(positions[v.vertexID]);
    targets.clear(0.0);
    Primitives<Topology.TriangleList>(IndexRange(indices, 0, 3, 2))
        .vertex(vertexShader)
        .draw(targets, (Varyings v) => 3.0);
    checkAll(color, 3.0);
    targets.clear(0.0);
    Primitives<Topology.TriangleList>(IndexRange(indices, 3, 3, -3))
        .vertex(vertexShader)
        .draw(targets, (Varyings v) => 4.0);
    checkAll(color, 4.0);

    // Depth clears must reach the depth target: the near draw passes only against a cleared far
    // plane, and the farther draw must then fail the test.
    let depth = allocateDepthTarget2D(8, 8);
    let depthTargets = RenderTargets(color, depth);
    depthTargets.clear(0.0, 1.0);
    for (uint pass = 0; pass < 2; ++pass)
    {
        float z = pass == 0 ? 0.25 : 0.75;
        float value = pass == 0 ? 1.0 : 2.0;
        Primitives<Topology.TriangleList>(VertexRange(3))
            .vertex(
                (VertexInvocation v) => Varyings(
                    float4(v.vertexID == 1 ? 3.0 : -1.0, v.vertexID == 2 ? 3.0 : -1.0, z, 1)))
            .draw<DepthState>(depthTargets, (Varyings v) => value);
    }
    checkAll(color, 1.0);
}
