// ser-test-reordering-dxr.slang
// Test DXR 1.3 MaybeReorderThread functions

// Test with DXR 1.3:  slangc -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

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

    // DXR 1.3 naming: MaybeReorderThread

    // MaybeReorderThread(HitObject) - reorder based on hit object
    MaybeReorderThread(hitObj);

    // MaybeReorderThread(CoherenceHint, NumCoherenceHintBitsFromLSB)
    MaybeReorderThread(uint(idx & 0xF), 4);

    // MaybeReorderThread(HitObject, CoherenceHint, NumCoherenceHintBitsFromLSB)
    MaybeReorderThread(hitObj, uint(idx & 0x3), 2);

    outputBuffer[idx] = uint(payload.color.x);
}
