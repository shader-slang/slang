//TEST:SIMPLE(filecheck=TRACE): -target cuda -entry rayGen -stage raygeneration
//TEST:SIMPLE(filecheck=LARGE): -target cuda -entry rayGenLarge -stage raygeneration
//TEST:SIMPLE(filecheck=EMPTY): -target cuda -entry rayGenEmpty -stage raygeneration
//TEST:SIMPLE(filecheck=TIME): -target cuda -entry closestHit -stage closesthit
//TEST:SIMPLE(filecheck=TRACE_PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry rayGen -stage raygeneration
//TEST:SIMPLE(filecheck=LARGE_PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry rayGenLarge -stage raygeneration
//TEST:SIMPLE(filecheck=EMPTY_PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry rayGenEmpty -stage raygeneration
//TEST:SIMPLE(filecheck=TIME_PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry closestHit -stage closesthit

// Regression test for https://github.com/shader-slang/slang/issues/13028.
//
// OptiX carries motion time into a traced ray and exposes that value to its hit and miss programs.
// Keep both directions in this test so the source intrinsics cannot drift apart: TraceMotionRay
// must forward the caller's value, and RayCurrentTime must read the native OptiX ray time. The
// ray-generation entries cover register-packed, pointer-backed, and legalized-away empty payloads.

// TRACE: optixTrace(
// TRACE: 0.625

// LARGE: optixTrace(
// LARGE: 0.75

// EMPTY: optixTrace(
// EMPTY: 0.25

// TIME: optixGetRayTime()

// TRACE_PTX: mov.f32 {{.*}}, 0f3F200000;
// TRACE_PTX: _optix_trace_typed

// LARGE_PTX: mov.f32 {{.*}}, 0f3F400000;
// LARGE_PTX: _optix_trace_typed

// EMPTY_PTX: mov.f32 {{.*}}, 0f3E800000;
// EMPTY_PTX: _optix_trace_typed

// TIME_PTX: _optix_get_ray_time

RaytracingAccelerationStructure scene;

struct Payload
{
    float value;
};

struct EmptyPayload
{};

struct LargePayload
{
    float values[33];
};

[shader("raygeneration")]
void rayGen()
{
    RayDesc ray = {float3(0.0), 0.0, float3(0.0, 0.0, 1.0), 100.0};
    Payload payload = {0.0};
    TraceMotionRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, 0.625, payload);
}

[shader("raygeneration")]
void rayGenLarge()
{
    RayDesc ray = {float3(0.0), 0.0, float3(0.0, 0.0, 1.0), 100.0};
    LargePayload payload = {};
    TraceMotionRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, 0.75, payload);
}

[shader("raygeneration")]
void rayGenEmpty()
{
    RayDesc ray = {float3(0.0), 0.0, float3(0.0, 0.0, 1.0), 100.0};
    EmptyPayload payload;
    TraceMotionRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, 0.25, payload);
}

[shader("closesthit")]
void closestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload.value = RayCurrentTime();
}
