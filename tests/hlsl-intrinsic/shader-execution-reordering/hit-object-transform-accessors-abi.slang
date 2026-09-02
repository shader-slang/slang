// Coverage for the re-gated NVAPI-only HitObject accessors GetRayDesc / GetWorldToObject /
// GetObjectToWorld (#11903 / #11997). These have no native dx::HitObject arm (DXR 1.3 uses the
// suffixed GetWorldToObject3x4/4x3 forms), so on HLSL they are reachable only through the explicit
// nvapiHitObjects opt-in and emit their `.`-method form on an NvHitObject. On a plain native SM 6.9
// target they must be a clean capability error, never native codegen — the same contract the sphere
// / LSS / cluster accessors follow.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_5 -entry main -stage raygeneration -capability nvapiHitObjects

// Negative: on a plain native SM 6.9 target (no nvapiHitObjects) none of these accessors have an
// arm, so each is a clean capability error. The code is 41011 (a method with no compatible arm for
// the target), not the type-emitter's 55215.
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
    // CHECK-DAG: .GetRayDesc
    // CHECK-DAG: .GetWorldToObject
    // CHECK-DAG: .GetObjectToWorld
    // CHECK-NOT: dx::HitObject
    // NATIVE_ERR: 41011
    RayDesc rd = ht.GetRayDesc();
    float4x3 w2o = ht.GetWorldToObject();
    float4x3 o2w = ht.GetObjectToWorld();
    outputBuffer[0] = rd.TMin + w2o[0][0] + o2w[0][0];
}
