// The 2-argument HitObject::MakeMiss(MissShaderIndex, Ray) is NVAPI-only: native DXR 1.3 has no
// dx::HitObject::MakeMiss with this signature, so its native `hlsl` disjunct is intentionally
// dropped and it requires the explicit nvapi_hit_objects capability. Under nvapi_hit_objects it
// must emit the NVAPI NvMakeMiss intrinsic (and, being NVAPI-only, it is simply unavailable on a
// plain native SM 6.9 target). See #11903 / #11997.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_5 -entry main -stage raygeneration -capability nvapi_hit_objects

// Negative: on a plain native SM 6.9 target (no nvapi_hit_objects) the 2-arg MakeMiss has no arm,
// so it is a clean capability error (no compatible __target_switch case) rather than a silently
// mis-emitted native MakeMiss.
//DIAGNOSTIC_TEST:SIMPLE(diag=NATIVE_ERR,non-exhaustive): -target hlsl -profile sm_6_9 -entry main -stage raygeneration

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    // CHECK: NvMakeMiss
    // NATIVE_ERR: 41011
    HitObject hm = HitObject::MakeMiss(0, ray);

    // CHECK-NOT: dx::HitObject::MakeMiss
    outputBuffer[0] = hm.IsMiss() ? 1u : 0u;
}
