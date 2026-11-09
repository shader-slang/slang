// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.pipeline_rt;

typealias RTTypes = HWRaytracingContext<>;

import slang.runtime;

public struct PrimaryRay : IRayType
{
    public uint value;
}

public struct VisibilityRay : IRayType
{
    public uint value;
    public uint instance;
}


public typealias DemoRayContext = HWRaytracingContext<PrimaryRay, VisibilityRay>;

/// Builds two triangle geometries and instances them twice, producing geometry slots 0..3.
[host]
public RaytracingAccelerationStructure makeRayTypeScene()
{
    let vertices = RuntimeArray<float3>::alloc(6);
    vertices[0] = float3(-1.75, -0.75, 0.0);
    vertices[1] = float3(-0.25, -0.75, 0.0);
    vertices[2] = float3(-1.0, 0.75, 0.0);
    vertices[3] = float3(0.25, -0.75, 0.0);
    vertices[4] = float3(1.75, -0.75, 0.0);
    vertices[5] = float3(1.0, 0.75, 0.0);
    let vertexBuffer = StructuredBuffer<float3>::alloc(6);
    copy(vertexBuffer, 0, vertices, 0, 6);

    let indices = RuntimeArray<uint>::alloc(3);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    let indexBuffer = StructuredBuffer<uint>::alloc(3);
    copy(indexBuffer, 0, indices, 0, 3);

    let geometries = RuntimeArray<RTTypes.TriangleGeometry>::alloc(2);
    for (uint geometryIndex = 0; geometryIndex < 2; ++geometryIndex)
    {
        RTTypes.TriangleGeometry geometry = RTTypes.TriangleGeometry(vertexBuffer, indexBuffer);
        geometry.vertexOffset = geometryIndex * 3;
        geometry.vertexCount = 3;
        geometry.indexCount = 3;
        geometries[geometryIndex] = geometry;
    }
    let blas = RTTypes.BottomLevelAccelerationStructure::alloc();
    blas.build(geometries);

    let instances = RuntimeArray<RTTypes.Instance>::alloc(2);
    for (uint instanceIndex = 0; instanceIndex < 2; ++instanceIndex)
    {
        RTTypes.Instance instance = RTTypes.Instance(blas);
        instance.transform = float3x4(
            1.0,
            0.0,
            0.0,
            float(instanceIndex * 4),
            0.0,
            1.0,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0,
            0.0);
        instance.instanceID = instanceIndex == 0 ? 7 : 19;
        instances[instanceIndex] = instance;
    }
    let scene = RaytracingAccelerationStructure::alloc();
    scene.build(instances);
    return scene;
}

/// Reuses the same shader specializations for every geometry, with distinct record data.
[host]
public void registerRayTypeCallbacks(DemoRayContext context, uint bias)
{
    context.onMiss<PrimaryRay>((PrimaryRay payload) => PrimaryRay(payload.value + 1000 + bias));
    context.onMiss<VisibilityRay>(
        (VisibilityRay payload) => VisibilityRay(payload.value + 2000 + bias, 99));

    for (uint geometrySlot = 0; geometrySlot < 4; ++geometrySlot)
    {
        uint value = bias + geometrySlot * 10;
        context.onHit<PrimaryRay>(
            geometrySlot,
            (PrimaryRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
                PrimaryRay(payload.value + value));
        context.onHit<VisibilityRay>(
            geometrySlot,
            (VisibilityRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
                VisibilityRay(payload.value + 100 + value, InstanceID()));
    }
}

/// Returns five triples: primary result, visibility result, and application instance ID.
/// The first four rays hit different geometry slots; the final ray misses both instances.
[host]
public RWStructuredBuffer<uint> dispatchRayTypes<let MaxRecursion : uint = 1>(
    DemoRayContext context,
    RaytracingAccelerationStructure scene)
{
    let output = RWStructuredBuffer<uint>::alloc(15);
    let outputHandle = output.getDescriptorHandle();
    let sceneHandle = scene.getDescriptorHandle();
    Grid(uint(5)).dispatchRays<MaxRecursion>(
        (uint index) =>
        {
            RayDesc ray;
            ray.Origin = float3(index < 4 ? float(index * 2) - 1.0 : 20.0, 0.0, -1.0);
            ray.Direction = float3(0.0, 0.0, 1.0);
            ray.TMin = 0.001;
            ray.TMax = 100.0;
            PrimaryRay primary = { 3 };
            VisibilityRay visibility = { 5, 0 };
            context.traceRay(*sceneHandle, ray, primary);
            context.traceRay(*sceneHandle, ray, visibility);
            outputHandle[index * 3] = primary.value;
            outputHandle[index * 3 + 1] = visibility.value;
            outputHandle[index * 3 + 2] = visibility.instance;
        });
    return output;
}
