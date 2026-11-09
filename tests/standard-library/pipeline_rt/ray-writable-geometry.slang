// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.pipeline_rt;

typealias RTTypes = HWRaytracingContext<>;

import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

/// Builds and intersects a triangle from independently read-only or writable buffer inputs.
[host]
void checkGeometry<V : IArray<float3>, I : IArray<uint>>(V vertices, I indices)
{
    typealias Geometry = RTTypes.TriangleGeometryDesc<V, I>;
    var geometry = Geometry(vertices, indices);
    geometry.vertexCount = 3;
    geometry.indexCount = 3;
    let geometries = RuntimeArray<Geometry>::alloc(1);
    geometries[0] = geometry;
    let blas = RTTypes.BottomLevelAccelerationStructure::alloc();
    blas.build(geometries);
    let instances = RuntimeArray<RTTypes.Instance>::alloc(1);
    instances[0] = RTTypes.Instance(blas);
    let scene = RaytracingAccelerationStructure::alloc();
    scene.build(instances);
    let sceneHandle = scene.getDescriptorHandle();
    let result = RWStructuredBuffer<uint>::alloc(2);
    Grid(2u).dispatch(
        (uint index) =>
        {
            RayDesc ray;
            ray.Origin = float3(float(index) * 4.0f, 0.0f, -1.0f);
            ray.Direction = float3(0.0f, 0.0f, 1.0f);
            ray.TMin = 0.001f;
            ray.TMax = 10.0f;
            RayQuery<RAY_FLAG_FORCE_OPAQUE> query;
            query.TraceRayInline(*sceneHandle, RAY_FLAG_NONE, 0xff, ray);
            while (query.Proceed())
            {
            }
            result[index] = query.CommittedStatus() == COMMITTED_TRIANGLE_HIT ? 1u : 0u;
        });
    assert(result.readBack(0) == 1);
    assert(result.readBack(1) == 0);
}

[host]
void main()
{
    let vertices = RWStructuredBuffer<float3>::alloc(3);
    let indices = RWStructuredBuffer<uint>::alloc(3);
    // GPU writes feed the build directly, exercising the write-to-build synchronization.
    Grid(3u).dispatch(
        (uint index) =>
        {
            float3 triangle[3] = { float3(-1, -1, 0), float3(1, -1, 0), float3(0, 1, 0) };
            vertices[index] = triangle[index];
            indices[index] = index;
        });
    checkGeometry(vertices, indices);
    let readVertices = StructuredBuffer<float3>::alloc(3);
    let readIndices = StructuredBuffer<uint>::alloc(3);
    copy(readVertices, 0, vertices, 0, 3);
    copy(readIndices, 0, indices, 0, 3);
    checkGeometry(readVertices, indices);
    checkGeometry(vertices, readIndices);
    checkGeometry(readVertices, readIndices);
}
