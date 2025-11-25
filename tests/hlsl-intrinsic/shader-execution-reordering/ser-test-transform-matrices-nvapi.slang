// ser-test-transform-matrices-nvapi.slang
// Test NVAPI-only Transform Matrix Methods

// Test with NVAPI:    slangc -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration

RWStructuredBuffer<float> outputBuffer;
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

    float result = 0.0f;

    if (hitObj.IsHit())
    {
        // NVAPI-only matrix accessors:

        // GetObjectToWorld - returns 4x3 object-to-world transform (NVAPI naming)
        float4x3 o2w = hitObj.GetObjectToWorld();
        result += o2w[0][0];

        // GetWorldToObject - returns 4x3 world-to-object transform (NVAPI naming)
        float4x3 w2o = hitObj.GetWorldToObject();
        result += w2o[0][0];
    }

    outputBuffer[idx] = result;
}
