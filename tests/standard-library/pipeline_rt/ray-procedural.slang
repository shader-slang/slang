// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm,filecheck=KIND): -api vulkan -entry mismatchedKind
// KIND: result code = {{-?[1-9][0-9]*}}
// KIND: HGVM hit-group geometry kind does not match the captured scene

import slang.runtime.compute.grid;
import slang.pipeline_rt;

typealias RTTypes = HWRaytracingContext<>;
import slang.runtime.core.simple_assert;

struct ProbeRay : IRayType
{
    uint value;
}
struct ShadowRay : IRayType
{
    uint value;
}
struct ProbeAttributes
{
    float4 data;
    uint tag;
}

[host]
RaytracingAccelerationStructure makeProceduralScene()
{
    let data = RuntimeArray<RTTypes.AABB>::alloc(3);
    data[0] = RTTypes.AABB(100, 100, 100, 101, 101, 101);
    data[1] = RTTypes.AABB(-1, -1, -1, 1, 1, 1);
    data[2] = RTTypes.AABB(-1, -1, -1, 1, 1, 1);
    let bounds = StructuredBuffer<RTTypes.AABB>::alloc(3);
    copy(bounds, 0, data, 0, 3);
    let geometries = RuntimeArray<RTTypes.AABBGeometry>::alloc(1);
    geometries[0] = RTTypes.AABBGeometry(bounds, 1, 1, 0);
    let blas = RTTypes.BottomLevelAccelerationStructure::alloc();
    blas.build(geometries);
    let instances = RuntimeArray<RTTypes.Instance>::alloc(4);
    for (uint i = 0; i < 3; ++i)
    {
        instances[i] = RTTypes.Instance(blas);
        instances[i].transform = float3x4(1, 0, 0, float(i) * 4, 0, 1, 0, 0, 0, 0, 0.5, 0);
        instances[i].instanceID = 70 + i;
    }
    // One TLAS may contain both triangle and procedural BLASes. Kind follows the geometry
    // snapshot and determines which native hit-group form is valid for each flattened slot.
    let triangleData = RuntimeArray<float3>::alloc(3);
    triangleData[0] = float3(-1, -1, 0);
    triangleData[1] = float3(1, -1, 0);
    triangleData[2] = float3(0, 1, 0);
    let vertices = StructuredBuffer<float3>::alloc(3);
    copy(vertices, 0, triangleData, 0, 3);
    let indexData = RuntimeArray<uint>::alloc(3);
    indexData[0] = 0;
    indexData[1] = 1;
    indexData[2] = 2;
    let indices = StructuredBuffer<uint>::alloc(3);
    copy(indices, 0, indexData, 0, 3);
    let triangleGeometries = RuntimeArray<RTTypes.TriangleGeometry>::alloc(1);
    var triangle = RTTypes.TriangleGeometry(vertices, indices);
    triangle.vertexCount = 3;
    triangle.indexCount = 3;
    triangleGeometries[0] = triangle;
    let triangleBLAS = RTTypes.BottomLevelAccelerationStructure::alloc();
    triangleBLAS.build(triangleGeometries);
    instances[3] = RTTypes.Instance(triangleBLAS);
    instances[3].transform = float3x4(1, 0, 0, 12, 0, 1, 0, 0, 0, 0, 1, 0);
    let scene = RaytracingAccelerationStructure::alloc();
    scene.build(instances);
    return scene;
}

[host]
RWStructuredBuffer<Ray> registerProcedural(
    HWRaytracingContext<ProbeRay, ShadowRay> context,
    uint bias)
{
    context.onMiss<ProbeRay>((ProbeRay payload) => ProbeRay(999));
    context.onMiss<ShadowRay>((ShadowRay payload) => ShadowRay(555));
    let tags = RuntimeArray<uint>::alloc(2);
    tags[0] = 7;
    tags[1] = 13;
    let buffer = StructuredBuffer<uint>::alloc(2);
    copy(buffer, 0, tags, 0, 2);
    let tagHandle = buffer.getDescriptorHandle();
    let raySamples = RWStructuredBuffer<Ray>::alloc(3);
    let raySamplesHandle = raySamples.getDescriptorHandle();
    for (uint slot = 0; slot < 3; ++slot)
    {
        uint scale = bias + slot + 1;
        float4 captured = float4(2, 3, 4, 5);
        uint threshold = 10;
        let intersection = (Ray ray, IntersectionReporter<ProbeAttributes> reporter) =>
        {
            // A near root rejected by any-hit must let the callback report the far root.
            // Each stage has independent captures, including a managed descriptor handle.
            raySamplesHandle[slot] = ray;
            // Intersect the object's z faces using the supplied ray. The instances shrink z,
            // so normalizing direction here would change the native ray's t parameterization.
            float nearT = (-1.0 - ray.origin.z) / ray.direction.z;
            float farT = (1.0 - ray.origin.z) / ray.direction.z;
            ProbeAttributes nearHit = { captured, tagHandle[0] };
            bool accepted = false;
            if (nearT >= ray.tMin && nearT <= ray.tMax)
                accepted = reporter.report(nearT, nearHit);
            if (!accepted && farT >= ray.tMin && farT <= ray.tMax)
                reporter.report(farT, ProbeAttributes(captured, tagHandle[1]));
        };
        let closestHit = (ProbeRay payload, ProbeAttributes attributes) =>
            ProbeRay(payload.value + attributes.tag * scale + uint(attributes.data.w));
        let anyHit = (ProbeRay payload, ProbeAttributes attributes) =>
        {
            payload.value += 10;
            return AnyHitResult<ProbeRay>(
                payload,
                attributes.tag < threshold ? AnyHitAction.Ignore : AnyHitAction.AcceptAndEndSearch);
        };
        if (slot == 0)
            context.onHit<ProbeRay>(
                slot,
                makeProceduralHitGroup<ProbeRay, ProbeAttributes>(
                    intersection,
                    closestHit,
                    anyHit));
        else if (slot == 1)
            context.onHit<ProbeRay>(
                slot,
                makeProceduralHitGroup<ProbeRay, ProbeAttributes>(intersection, closestHit));
        else
            context.onHit<ProbeRay>(
                slot,
                makeProceduralHitGroup<ProbeRay, ProbeAttributes>(
                    intersection,
                    NoClosestHit<ProbeRay, ProbeAttributes>(),
                    anyHit));

        // No closest-hit or any-hit export is needed to terminate a visibility ray.
        context.onHit<ShadowRay>(
            slot,
            makeProceduralHitGroup<ShadowRay, ProbeAttributes>(intersection));
    }
    context.onHit<ProbeRay>(
        3,
        (ProbeRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
            ProbeRay(313 + bias));
    context.onHit<ShadowRay>(
        3,
        (ShadowRay payload, BuiltInTriangleIntersectionAttributes attributes) => ShadowRay(31));
    return raySamples;
}

[host]
RWStructuredBuffer<uint> traceProcedural(
    HWRaytracingContext<ProbeRay, ShadowRay> context,
    RaytracingAccelerationStructure scene)
{
    let output = RWStructuredBuffer<uint>::alloc(10);
    let outputHandle = output.getDescriptorHandle();
    let sceneHandle = scene.getDescriptorHandle();
    Grid(uint(5)).dispatchRays(
        (uint index) =>
        {
            RayDesc ray = { float3(float(index) * 4, 0, -1), 0.125, float3(0, 0, 1), 10 };
            ProbeRay primary = { 0 };
            context.traceRay<ProbeRay>(*sceneHandle, ray, primary);
            outputHandle[index * 2] = primary.value;
            ShadowRay shadow = { 123 };
            context.traceRay<ShadowRay>(
                *sceneHandle,
                ray,
                shadow,
                RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH);
            outputHandle[index * 2 + 1] = shadow.value;
        });
    return output;
}

[host]
void main()
{
    let scene = makeProceduralScene();
    HWRaytracingContext<ProbeRay, ShadowRay> context = {};
    let firstRays = registerProcedural(context, 1);
    let first = traceProcedural(context, scene);
    let secondRays = registerProcedural(context, 4);
    let second = traceProcedural(context, scene);
    for (uint i = 0; i < 3; ++i)
    {
        uint firstExpected = i == 0 ? 25 + 13 * (i + 2) : i == 1 ? 5 + 7 * (i + 2) : 20;
        uint secondExpected = i == 0 ? 25 + 13 * (i + 5) : i == 1 ? 5 + 7 * (i + 5) : 20;
        assert(first.readBack(i * 2) == firstExpected);
        assert(second.readBack(i * 2) == secondExpected);
        assert(first.readBack(i * 2 + 1) == 123);
        assert(second.readBack(i * 2 + 1) == 123);
        // Validate the callback's public inputs against known transformed rays, including
        // the non-unit direction and both interval bounds, for both registration snapshots.
        Ray firstRay = firstRays.readBack(i);
        Ray secondRay = secondRays.readBack(i);
        assert(firstRay.origin.x == 0 && firstRay.origin.y == 0 && firstRay.origin.z == -2);
        assert(firstRay.direction.x == 0 && firstRay.direction.y == 0 && firstRay.direction.z == 2);
        assert(firstRay.tMin == 0.125 && firstRay.tMax == 10.0);
        assert(secondRay.origin.x == 0 && secondRay.origin.y == 0 && secondRay.origin.z == -2);
        assert(
            secondRay.direction.x == 0 && secondRay.direction.y == 0 && secondRay.direction.z == 2);
        assert(secondRay.tMin == firstRay.tMin && secondRay.tMax == firstRay.tMax);
    }
    assert(first.readBack(6) == 314);
    assert(second.readBack(6) == 317);
    assert(first.readBack(7) == 31);
    assert(first.readBack(8) == 999);
    assert(first.readBack(9) == 555);
}

[host]
void mismatchedKind()
{
    let scene = makeProceduralScene();
    HWRaytracingContext<ProbeRay, ShadowRay> context = {};
    registerProcedural(context, 0);
    context.onHit<ProbeRay>(
        0,
        (ProbeRay payload, BuiltInTriangleIntersectionAttributes attributes) => payload);
    let output = traceProcedural(context, scene);
    print(output.readBack(0));
}
