// Coverage for the new cuda_glsl_nvapi_hit_objects require alias (#11903 / #11997): GetClusterID is
// an NVAPI/SPIRV/CUDA extension with no native DXR 1.3 arm, so on HLSL it is reachable only through
// the explicit nvapi_hit_objects opt-in (its require is `cuda_glsl_nvapi_hit_objects`, i.e.
// cuda | glsl | nvapi_hit_objects). Confirm the HLSL path emits the NVAPI `.GetClusterID` accessor on
// an NvHitObject, and never a native dx::HitObject.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_5 -entry main -stage raygeneration -capability nvapi_hit_objects

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

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
    // CHECK: .GetClusterID
    // CHECK-NOT: dx::HitObject
    outputBuffer[0] = ht.IsHit() ? (uint)ht.GetClusterID() : 0u;
}
