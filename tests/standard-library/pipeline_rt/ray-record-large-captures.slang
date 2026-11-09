// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api vulkan -entry largerCaptures
// TEST:INTERPRET(hgvm,filecheck=LIMIT): -api d3d12 -entry largerCaptures
// LIMIT: result code = {{-?[1-9][0-9]*}}
// LIMIT: D3D12 currently limits application data to 512 bytes (D3D12 driver blocker:
// LIMIT-NOT: DXGI_ERROR_DEVICE_REMOVED

import "../support/ray_context_support.slang";
import slang.runtime.compute.grid;
import slang.pipeline_rt;
import slang.runtime.core.simple_assert;

// The default uses 384-byte captures, leaving room for raygen's handles below D3D12's
// driver cap. The larger entry tests 1024-byte captures on Vulkan and rejection on D3D12.
struct RecordValues<let N : uint>
{
    uint4 values[N];

    uint sum()
    {
        uint result = 0;
        for (uint i = 0; i < N; ++i)
            result += values[i].x + values[i].y + values[i].z + values[i].w;
        return result;
    }
}

[host]
RecordValues<N> makeValues<let N : uint>(uint base)
{
    RecordValues<N> result;
    for (uint i = 0; i < N; ++i)
        result.values[i] = uint4(base + i, base + i + 1, base + i + 2, base + i + 3);
    return result;
}

[host]
[noinline]
void registerLargeRecords<let N : uint>(DemoRayContext context, uint bias)
{
    registerRayTypeCallbacks(context, 0);
    for (uint geometry = 0; geometry < 4; ++geometry)
    {
        let values = makeValues<N>(bias + geometry * 10);
        context.onHit<PrimaryRay>(
            geometry,
            (PrimaryRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
                PrimaryRay(payload.value + values.sum()));
    }
    let missValues = makeValues<N>(bias + 200);
    context.onMiss<PrimaryRay>(
        (PrimaryRay payload) => PrimaryRay(payload.value + missValues.sum()));
}

[host]
[noinline]
RWStructuredBuffer<uint> dispatchLargeRecords<let N : uint>(
    DemoRayContext context,
    RaytracingAccelerationStructure scene,
    uint bias)
{
    let output = RWStructuredBuffer<uint>::alloc(5);
    let outputHandle = output.getDescriptorHandle();
    let sceneHandle = scene.getDescriptorHandle();
    let values = makeValues<N>(bias + 500);
    Grid(uint(5)).dispatchRays(
        (uint index) =>
        {
            RayDesc ray;
            ray.Origin = float3(index < 4 ? float(index * 2) - 1.0 : 20.0, 0.0, -1.0);
            ray.Direction = float3(0.0, 0.0, 1.0);
            ray.TMin = 0.001;
            ray.TMax = 100.0;
            PrimaryRay payload = { 0 };
            context.traceRay(*sceneHandle, ray, payload);
            outputHandle[index] = payload.value + values.sum();
        });
    return output;
}

[host]
void checkLargeRecords<let N : uint>()
{
    let scene = makeRayTypeScene();
    DemoRayContext context = {};
    registerLargeRecords<N>(context, 0);
    let first = dispatchLargeRecords<N>(context, scene, 0);
    registerLargeRecords<N>(context, 100);
    let second = dispatchLargeRecords<N>(context, scene, 100);
    // Read both only after replacing the registrations and queuing the second dispatch.
    for (uint i = 0; i < 5; ++i)
    {
        uint recordBase = i < 4 ? i * 10 : 200;
        uint expected = 4 * N * (recordBase + 500) + 2 * (2 * N * N + 4 * N);
        assert(first.readBack(i) == expected);
        assert(second.readBack(i) == expected + 2 * 4 * N * 100);
    }
}

[host]
void main()
{
    checkLargeRecords<24>();
}

[host]
void largerCaptures()
{
    checkLargeRecords<64>();
}
