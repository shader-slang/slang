// Regression for #11903 / #11997: a HitObject resolves to exactly ONE shader-execution-reordering
// ABI (native DXR 1.3 vs NVAPI), consulted identically by the type emitter and every method, so the
// emitted HLSL never mixes the two on one object.
//
// Native DXR (SM 6.9) is the default. NVAPI is used ONLY when the explicit `nvapiHitObjects`
// capability is requested; merely having `hlsl_nvapi` (e.g. for atomics) does NOT switch HitObject
// to NVAPI. So none of these three configurations may produce a mixed dx::HitObject / Nv* result.

// [1] Native (SM 6.9): all dx::HitObject::*, no NVAPI intrinsics.
//TEST:SIMPLE(filecheck=NATIVE): -target hlsl -profile sm_6_9 -entry main -stage raygeneration -capability ser_dxr

// [2] Dual-capability (SM 6.9 + hlsl_nvapi for atomics, NO explicit HitObject opt-in): must stay
//     native — this is the exact mixed-ABI codegen reported in #11903.
//TEST:SIMPLE(filecheck=NATIVE): -target hlsl -profile sm_6_9 -entry main -stage raygeneration -capability hlsl_nvapi

// [3] Explicit NVAPI opt-in: all NVAPI, even alongside SM 6.9.
//TEST:SIMPLE(filecheck=NVAPI): -target hlsl -profile sm_6_9 -entry main -stage raygeneration -capability nvapiHitObjects

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct MyPayload { float4 color; };
struct MyAttribs { float2 bary; };

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload = { float4(0) };

    HitObject hn = HitObject::MakeNop();
    // NATIVE: dx::HitObject::MakeNop
    // NVAPI: NvMakeNop
    uint r = hn.IsNop() ? 1u : 0u;

    HitObject ht = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    // NATIVE: dx::HitObject::TraceRay
    // NVAPI: NvTraceRayHitObject
    if (ht.IsHit())
        r += ht.GetRayFlags();

    // ReorderThread exercises a third re-gated method (with a uniform signature across both ABIs)
    // on the same object: it must pick the same ABI as MakeNop/TraceRay above, never mixing.
    ReorderThread(ht);
    // NATIVE: dx::MaybeReorderThread
    // NVAPI: NvReorderThread

    // MakeHit is the most heavily re-gated method; it must pick the same ABI as the rest.
    MyAttribs attribs = { float2(0.5, 0.5) };
    HitObject hh = HitObject::MakeHit(scene, 0, 0, 0, 0, 0, 0, ray, attribs);
    // NATIVE: dx::HitObject::MakeHit
    // NVAPI: NvMakeHit
    r += hh.IsHit() ? 1u : 0u;

    outputBuffer[0] = r;

    // The emitted HitObject TYPE must also match the ABI, never mixed.
    // NATIVE-DAG: dx::HitObject
    // NVAPI-DAG: NvHitObject

    // The other ABI's intrinsics / type must never appear (no mixing).
    // NATIVE-NOT: NvMakeNop
    // NATIVE-NOT: NvTraceRayHitObject
    // NATIVE-NOT: NvReorderThread
    // NATIVE-NOT: NvMakeHit
    // NATIVE-NOT: NvHitObject
    // NVAPI-NOT: dx::HitObject::MakeNop
    // NVAPI-NOT: dx::HitObject::TraceRay
    // NVAPI-NOT: dx::MaybeReorderThread
    // NVAPI-NOT: dx::HitObject::MakeHit
}
