//TEST:SIMPLE(filecheck=COMPILE): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -entry motionRayGen -stage raygeneration -O0 -o nvvm-current-transform-motion-raygeneration-o0.ptx.actual
//TEST:SIMPLE(filecheck=COMPILE): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -entry motionMiss -stage miss -O0 -o nvvm-current-transform-motion-miss-o0.ptx.actual
//TEST:SIMPLE(filecheck=COMPILE): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -entry motionClosestHit -stage closesthit -O0 -o nvvm-current-transform-motion-closesthit-o0.ptx.actual
//TEST:SIMPLE(filecheck=COMPILE): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -entry motionAnyHit -stage anyhit -O0 -o nvvm-current-transform-motion-anyhit-o0.ptx.actual
//TEST:SIMPLE(filecheck=COMPILE): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -entry motionRayGen -stage raygeneration -O3 -o nvvm-current-transform-motion-raygeneration-o3.ptx.actual
//TEST:SIMPLE(filecheck=COMPILE): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -entry motionMiss -stage miss -O3 -o nvvm-current-transform-motion-miss-o3.ptx.actual
//TEST:SIMPLE(filecheck=COMPILE): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -entry motionClosestHit -stage closesthit -O3 -o nvvm-current-transform-motion-closesthit-o3.ptx.actual
//TEST:SIMPLE(filecheck=COMPILE): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -entry motionAnyHit -stage anyhit -O3 -o nvvm-current-transform-motion-anyhit-o3.ptx.actual
// COMPILE: result code = 0

// Current-ray matrices in AnyHit and ClosestHit share the independent 309-word host oracle.
// These checks compile all four stages; they do not construct or execute a motion scene.
// External-host runtime qualification and its independent guarded matrix/lifecycle oracle
// are recorded in issue-nvvm-backend/RESULTS.md. The execution gate verifies global reflection
// before the host binds its scene, output pointer and count.

// Raw external-host probe. Reflection must confirm inline global fields at offsets0/8/16.
uniform RaytracingAccelerationStructure sceneBVH;
uniform RWStructuredBuffer<uint> output;

[raypayload] struct MotionPayload
{
    uint value : read(caller, closesthit)
        : write(caller, anyhit, closesthit, miss);
    uint phase : read(caller, closesthit)
        : write(caller, anyhit, closesthit, miss);
};

[noinline] HitObject makeMotionObject(uint index, inout MotionPayload payload)
{
    RayDesc ray;
    ray.Origin = index == 0   ? float3(8, 5, 1)
                 : index == 1 ? float3(26, -4.5, 3)
                              : float3(12, -11, 7);
    ray.Direction = index == 1 ? float3(0, 0, 4) : float3(0, 0, 2);
    ray.TMin = 0.25;
    ray.TMax = 4;
    float time = float(index) * 0.5;
    return HitObject
        .TraceMotionRay(sceneBVH, RAY_FLAG_FORCE_NON_OPAQUE, 0xff, 0, 1, 0, ray, time, payload);
}

[noinline] void observeMotionObject(inout HitObject object, uint row)
{
    uint base = 1 + row * 48;
    for (uint i = 0; i < 24; ++i)
        output[base + i] = 0;
    output[base] = uint(object.IsHit());
    if (!object.IsHit())
        return;
    output[base + 1] = object.GetInstanceID();
    output[base + 2] = object.GetInstanceIndex();
    output[base + 3] = object.GetHitKind();
    output[base + 4] = object.GetPrimitiveIndex();
    output[base + 5] = asuint(object.GetCurrentTime());
    RayDesc ray = object.GetRayDesc();
    output[base + 6] = asuint(ray.TMin);
    output[base + 7] = asuint(ray.TMax);
    float3 objectOrigin = object.GetObjectRayOrigin();
    float3 objectDirection = object.GetObjectRayDirection();
    for (uint i = 0; i < 3; ++i)
    {
        output[base + 8 + i] = asuint(ray.Origin[i]);
        output[base + 11 + i] = asuint(ray.Direction[i]);
        output[base + 14 + i] = asuint(objectOrigin[i]);
        output[base + 17 + i] = asuint(objectDirection[i]);
    }
    BuiltInTriangleIntersectionAttributes attributes =
        object.GetAttributes<BuiltInTriangleIntersectionAttributes>();
    output[base + 20] = asuint(attributes.barycentrics.x);
    output[base + 21] = asuint(attributes.barycentrics.y);
    output[base + 22] = object.GetShaderTableIndex();
    output[base + 23] = object.GetGeometryIndex();

}

[noinline] void invokeMotionObject(inout HitObject object, uint index, inout MotionPayload payload)
{
    HitObject.Invoke(sceneBVH, object, payload);
    output[289 + index * 2] = payload.value;
    output[290 + index * 2] = payload.phase;
}

[shader("raygeneration")] void motionRayGen()
{
    MotionPayload pa = {7, 0}, pb = {7, 0}, pc = {7, 0};
    HitObject a = makeMotionObject(0, pa);
    HitObject b = makeMotionObject(1, pb);
    HitObject c = makeMotionObject(2, pc);
    observeMotionObject(a, 0);
    observeMotionObject(b, 1);
    observeMotionObject(c, 2);
    MotionPayload invoked = {50, 0};
    invokeMotionObject(a, 0, invoked);
    invokeMotionObject(b, 1, invoked);
    invokeMotionObject(c, 2, invoked);
    invokeMotionObject(c, 3, invoked);
    invokeMotionObject(b, 4, invoked);
    invokeMotionObject(a, 5, invoked);
    observeMotionObject(c, 3);
    observeMotionObject(b, 4);
    observeMotionObject(a, 5);
    output[301] = pa.value;
    output[302] = pa.phase;
    output[303] = pb.value;
    output[304] = pb.phase;
    output[305] = pc.value;
    output[306] = pc.phase;
    output[307] = 0xc001c0de;
}

[shader("anyhit")] void motionAnyHit(
    inout MotionPayload payload,
    in BuiltInTriangleIntersectionAttributes attributes)
{
    // The host oracle expects time 0, 0.5 and 1 in the first three records.
    // Each trace has a distinct world origin, independent of transform results.
    uint index = WorldRayOrigin().x == 8 ? 0 : WorldRayOrigin().x == 26 ? 1 : 2;
    float3x4 forward = ObjectToWorld();
    float3x4 inverse = WorldToObject();
    for (uint r = 0; r < 3; ++r)
        for (uint c = 0; c < 4; ++c)
        {
            output[1 + index * 48 + 24 + r * 4 + c] = asuint(forward[r][c]);
            output[1 + index * 48 + 36 + r * 4 + c] = asuint(inverse[r][c]);
        }
    payload.value = 1000 + InstanceIndex();
    payload.phase = 64;
}

[shader("closesthit")] void motionClosestHit(
    inout MotionPayload payload,
    in BuiltInTriangleIntersectionAttributes attributes)
{
    // The last three records contain times 1, 0.5 and 0. Repeated invocation
    // checks that each callback reads its incoming list, not outgoing HitObject state.
    uint index = WorldRayOrigin().x == 8 ? 5 : WorldRayOrigin().x == 26 ? 4 : 3;
    float4x3 forward = ObjectToWorld4x3();
    float4x3 inverse = WorldToObject4x3();
    for (uint r = 0; r < 3; ++r)
        for (uint c = 0; c < 4; ++c)
        {
            output[1 + index * 48 + 24 + r * 4 + c] = asuint(forward[c][r]);
            output[1 + index * 48 + 36 + r * 4 + c] = asuint(inverse[c][r]);
        }
    payload.value += 100 + InstanceIndex();
    payload.phase += 1;
}

[shader("miss")] void motionMiss(inout MotionPayload payload)
{
    payload.value = 0xbad;
    payload.phase = 0xbad;
}
