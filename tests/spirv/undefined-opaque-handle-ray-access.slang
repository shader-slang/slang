// Check that undefined acceleration structures remove their annotated initialization or trace,
// while a defined acceleration structure retains the corresponding operation and payload state.
//TEST:SIMPLE(filecheck=QUERY):-target spirv -entry reinitializeQuery -stage compute
//TEST:SIMPLE(filecheck=QUERY):-target spirv -entry reinitializeQuery -stage compute -O0
//TEST:SIMPLE(filecheck=TRACE):-target spirv -entry traceRay -stage raygeneration
//TEST:SIMPLE(filecheck=TRACE):-target spirv -entry traceRay -stage raygeneration -O0
//TEST:SIMPLE(filecheck=MOTION):-target spirv -entry traceMotionRay -stage raygeneration
//TEST:SIMPLE(filecheck=MOTION):-target spirv -entry traceMotionRay -stage raygeneration -O0

// QUERY: OpRayQueryInitializeKHR
// QUERY-NOT: OpRayQueryInitializeKHR
// QUERY: %[[TMIN:[A-Za-z0-9_]+]] = OpRayQueryGetRayTMinKHR
// QUERY: OpStore {{.*}} %[[TMIN]]
// TRACE: OpTraceRayKHR
// TRACE-NOT: OpTraceRayKHR
// TRACE: %[[PAYLOAD:[A-Za-z0-9_]+]] = OpLoad %Payload
// TRACE: %[[VALUE:[A-Za-z0-9_]+]] = OpCompositeExtract %float %[[PAYLOAD]] 0
// TRACE: OpStore {{.*}} %[[VALUE]]
// MOTION: OpTraceRayMotionNV
// MOTION-NOT: OpTraceRayMotionNV
// MOTION: %[[PAYLOAD:[A-Za-z0-9_]+]] = OpLoad %Payload
// MOTION: %[[VALUE:[A-Za-z0-9_]+]] = OpCompositeExtract %float %[[PAYLOAD]] 0
// MOTION: OpStore {{.*}} %[[VALUE]]

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float> outputBuffer;
cbuffer Settings
{
    bool useUndefined;
};

RayDesc makeRay()
{
    RayDesc ray;
    ray.Origin = float3(0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    return ray;
}

[numthreads(1, 1, 1)]
void reinitializeQuery()
{
    RaytracingAccelerationStructure missing;
    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, makeRay());
    if (useUndefined)
        query.TraceRayInline(missing, RAY_FLAG_NONE, 0xff, makeRay());
    outputBuffer[0] = query.RayTMin();
}

struct Payload
{
    float value;
};

[shader("raygeneration")]
void traceRay()
{
    RaytracingAccelerationStructure missing;
    Payload payload = { 0 };
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, makeRay(), payload);
    if (useUndefined)
    {
        Payload undefinedPayload = { 0 };
        TraceRay(missing, RAY_FLAG_NONE, 0xff, 0, 1, 0, makeRay(), undefinedPayload);
    }
    outputBuffer[0] = payload.value;
}

[shader("raygeneration")]
void traceMotionRay()
{
    RaytracingAccelerationStructure missing;
    Payload payload = { 0 };
    TraceMotionRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, makeRay(), 0.5, payload);
    if (useUndefined)
    {
        Payload undefinedPayload = { 0 };
        TraceMotionRay(missing, RAY_FLAG_NONE, 0xff, 0, 1, 0, makeRay(), 0.5, undefinedPayload);
    }
    outputBuffer[0] = payload.value;
}
