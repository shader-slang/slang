// ser-test-query-state.slang
// Test HitObject Query State Methods: IsMiss, IsHit, IsNop

// Test with DXR 1.3:  slangc -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration
// Test with NVAPI:    slangc -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration

RWStructuredBuffer<uint> outputBuffer;
RaytracingAccelerationStructure accelStruct;

struct MyPayload { float3 color; };

[shader("raygeneration")]
void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload;

    HitObject hitObj = HitObject::TraceRay(
        accelStruct, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

    HitObject missObj = HitObject::MakeMiss(0, ray);
    HitObject nopObj = HitObject::MakeNop();

    uint result = 0;

    // IsMiss - returns true if the hit object represents a miss
    if (hitObj.IsMiss())
        result |= 1;
    if (missObj.IsMiss())
        result |= 2;

    // IsHit - returns true if the hit object represents a hit
    if (hitObj.IsHit())
        result |= 4;
    if (missObj.IsHit())
        result |= 8;

    // IsNop - returns true if the hit object represents a NOP
    if (nopObj.IsNop())
        result |= 16;
    if (hitObj.IsNop())
        result |= 32;

    outputBuffer[idx] = result;
}
