// The sphere / linear-swept-spheres HitObject accessors are NVAPI-only (no native dx::HitObject
// form), reachable on HLSL only via the nvapiHitObjects opt-in; on native they are a clean
// capability error (41011: the method's __target_switch has no compatible arm), never native codegen.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_5 -entry main -stage raygeneration -capability nvapiHitObjects
//DIAGNOSTIC_TEST:SIMPLE(diag=NATIVE_ERR,non-exhaustive): -target hlsl -profile sm_6_9 -entry main -stage raygeneration

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

struct MyPayload { float4 color; };

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload = { float4(0) };
    HitObject ht = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    // CHECK: NvHitObject
    // CHECK-DAG: .GetSphereObjectPositionAndRadius
    // CHECK-DAG: .GetLssObjectPositionsAndRadii
    // CHECK-DAG: .IsSphereHit
    // CHECK-DAG: .IsLssHit
    // CHECK-NOT: dx::HitObject
    // NATIVE_ERR: 41011
    float4 sp = ht.GetSpherePositionAndRadius();
    float2x4 lss = ht.GetLssPositionsAndRadii();
    outputBuffer[0] = (ht.IsSphereHit() ? sp.w : 0.0f) + (ht.IsLssHit() ? lss[0][3] : 0.0f);
}
