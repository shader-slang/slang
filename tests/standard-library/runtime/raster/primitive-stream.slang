// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan

import slang.runtime;

[host]
void main()
{
    let vertices = VertexRange(3, 7);
    let stream = Primitives<Topology.TriangleList>(vertices);
    assert(stream.source.getCount() == 3);
    assert(stream.source.firstVertex == 7);
    assert(stream.instanceCount == 1 && stream.firstInstance == 0);

    // Captures remain ordinary values. Attaching the shader must not allocate or dispatch work.
    float scale = 2.0;
    let shaded = stream.instances(5, 11).vertex(
        (VertexInvocation v) => float2(float(v.vertexID) * scale, float(v.instanceID)));
    assert(shaded.primitives.instanceCount == 5 && shaded.primitives.firstInstance == 11);
    let resized = shaded.instances(2, 19);
    assert(resized.primitives.instanceCount == 2 && resized.primitives.firstInstance == 19);
    VertexInvocation invocation = { 7, 19 };
    let result = resized.vertexShader(invocation);
    assert(result.x == 14 && result.y == 19);

    let indices = RWStructuredBuffer<uint>::alloc(12);
    let all = IndexRange(indices);
    assert(all.indexCount == 12 && all.firstIndex == 0 && all.baseVertex == 0);
    let indexed = Primitives<Topology.TriangleList>(IndexRange(indices, 2, 6, -3));
    assert(indexed.source.getCount() == 6 && indexed.source.firstIndex == 2);
    assert(indexed.source.baseVertex == -3);
    assert(IndexRange(indices, 12, 0).indexCount == 0);
    assert(Primitives<Topology.TriangleList>(VertexRange(0)).instances(0).instanceCount == 0);
}
