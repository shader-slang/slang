// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.compute.grid;
import slang.pipeline_rt;

typealias RTTypes = HWRaytracingContext<>;
import "../support/ray_context_support.slang";
import slang.runtime.core.simple_assert;

[host]
RWStructuredBuffer<uint> dispatchPrimary<each Rays : IRayType>(
    HWRaytracingContext<Rays> context,
    RaytracingAccelerationStructure scene,
    float2 centers)
{
    let output = RWStructuredBuffer<uint>::alloc(2);
    let outputHandle = output.getDescriptorHandle();
    let sceneHandle = scene.getDescriptorHandle();
    Grid(uint(2)).dispatchRays(
        (uint index) =>
        {
            RayDesc ray;
            ray.Origin = float3(index == 0 ? centers.x : centers.y, 0.0, -1.0);
            ray.Direction = float3(0.0, 0.0, 1.0);
            ray.TMin = 0.001;
            ray.TMax = 100.0;
            PrimaryRay payload = { 0 };
            context.traceRay(*sceneHandle, ray, payload);
            outputHandle[index] = payload.value;
        });
    return output;
}

[host]
void registerPrimary<each Rays : IRayType>(HWRaytracingContext<Rays> context, uint bias)
{
    context.onMiss<PrimaryRay>((PrimaryRay payload) => PrimaryRay(999));
    for (uint geometry = 0; geometry < 2; ++geometry)
    {
        context.onHit<PrimaryRay>(
            geometry,
            (PrimaryRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
                PrimaryRay(bias + geometry));
    }
}

[host]
void main()
{
    // Keep both vertex sets in one allocation. Rebuilding changes the selected range, so the
    // test observes AS snapshot semantics without depending on mutable geometry buffer contents.
    let vertices = RuntimeArray<float3>::alloc(6);
    vertices[0] = float3(-0.75, -0.75, 0.0);
    vertices[1] = float3(0.75, -0.75, 0.0);
    vertices[2] = float3(0.0, 0.75, 0.0);
    vertices[3] = float3(1.25, -0.75, 0.0);
    vertices[4] = float3(2.75, -0.75, 0.0);
    vertices[5] = float3(2.0, 0.75, 0.0);
    let vertexBuffer = StructuredBuffer<float3>::alloc(6);
    copy(vertexBuffer, 0, vertices, 0, 6);
    let indices = RuntimeArray<uint>::alloc(3);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    let indexBuffer = StructuredBuffer<uint>::alloc(3);
    copy(indexBuffer, 0, indices, 0, 3);
    RTTypes.TriangleGeometry geometry = RTTypes.TriangleGeometry(vertexBuffer, indexBuffer);
    geometry.vertexCount = 3;
    geometry.indexCount = 3;
    let geometries = RuntimeArray<RTTypes.TriangleGeometry>::alloc(1);
    geometries[0] = geometry;
    let blas = RTTypes.BottomLevelAccelerationStructure::alloc();
    blas.build(geometries);

    let instances = RuntimeArray<RTTypes.Instance>::alloc(2);
    instances[0] = RTTypes.Instance(blas);
    RTTypes.Instance translated = RTTypes.Instance(blas);
    translated.transform = float3x4(1, 0, 0, 4, 0, 1, 0, 0, 0, 0, 1, 0);
    instances[1] = translated;
    let scene = RaytracingAccelerationStructure::alloc();
    scene.build(instances);

    HWRaytracingContext<PrimaryRay> narrow = {};
    registerPrimary(narrow, 10);
    DemoRayContext wide = {};
    registerPrimary(wide, 20);
    wide.onMiss<VisibilityRay>((VisibilityRay payload) => VisibilityRay(999, 0));
    for (uint slot = 0; slot < 2; ++slot)
    {
        wide.onHit<VisibilityRay>(
            slot,
            (VisibilityRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
                VisibilityRay(100 + slot, 0));
    }

    // These two dispatches require different instance contributions in the native TLAS.
    let narrowBefore = dispatchPrimary(narrow, scene, float2(0, 4));
    let wideBefore = dispatchPrimary(wide, scene, float2(0, 4));

    geometry.vertexOffset = 3;
    geometries[0] = geometry;
    blas.build(geometries);
    translated.transform = float3x4(1, 0, 0, 8, 0, 1, 0, 0, 0, 0, 1, 0);
    instances[1] = translated;
    // Neither rebuilding a BLAS nor editing the original instance array rebuilds its old TLAS.
    let retained = dispatchPrimary(wide, scene, float2(0, 4));

    scene.build(instances);
    let wideAfter = dispatchPrimary(wide, scene, float2(2, 10));
    let narrowAfter = dispatchPrimary(narrow, scene, float2(2, 10));
    // Read only after every build/dispatch has been queued, proving earlier work retains its
    // scene versions and table data through later changes to the same logical AS handles.
    for (uint instance = 0; instance < 2; ++instance)
    {
        assert(narrowBefore.readBack(instance) == 10 + instance);
        assert(wideBefore.readBack(instance) == 20 + instance);
        assert(retained.readBack(instance) == 20 + instance);
        assert(wideAfter.readBack(instance) == 20 + instance);
        assert(narrowAfter.readBack(instance) == 10 + instance);
    }
}
