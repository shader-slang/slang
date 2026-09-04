// shader-slang/slang#12629: RAY_FLAG_SKIP_TRIANGLES / RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES have no
// per-ray OptixRayFlags counterpart and must be diagnosed for the CUDA/OptiX target.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry rayGenMain -stage raygeneration

struct Payload
{
    float4 color;
}

uniform RaytracingAccelerationStructure gScene;

[shader("raygeneration")]
void rayGenMain()
{
    RayDesc ray;
    ray.Origin = float3(0);
    ray.TMin = 0.0;
    ray.Direction = float3(0, 0, 1);
    ray.TMax = 1e30;

    Payload payload;
    payload.color = float4(0);

    // Supported flags incl. RAY_FLAG_FORCE_OMM_2_STATE (0x400 == OptiX bit 10) must stay clean;
    // under exhaustive matching a spurious diagnostic here would fail the test.
    TraceRay(
        gScene,
        RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_FORCE_OMM_2_STATE,
        0xff,
        0,
        1,
        0,
        ray,
        payload);

    TraceRay(gScene, RAY_FLAG_SKIP_TRIANGLES, 0xff, 0, 1, 0, ray, payload);

    TraceRay(
        gScene,
        RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES,
        0xff,
        0,
        1,
        0,
        ray,
        payload);

    // Each error emits a title row and a span row, so the two unsupported flags produce four rows.
    //CHECK: ray flag not supported by the CUDA/OptiX target
    //CHECK: RAY_FLAG_SKIP_TRIANGLES' has no OptiX equivalent
    //CHECK: ray flag not supported by the CUDA/OptiX target
    //CHECK: RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES' has no OptiX equivalent
}
