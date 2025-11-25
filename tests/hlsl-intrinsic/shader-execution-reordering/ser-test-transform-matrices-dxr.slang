// ser-test-transform-matrices-dxr.slang
// Test DXR 1.3-only Transform Matrix Methods

// Test with DXR 1.3:  slangc -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

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
        // DXR 1.3-only matrix accessors:

        // GetObjectToWorld3x4 - returns 3x4 object-to-world transform
        float3x4 o2w_3x4 = hitObj.GetObjectToWorld3x4();
        result += o2w_3x4[0][0];

        // GetObjectToWorld4x3 - returns 4x3 object-to-world transform (transposed)
        float4x3 o2w_4x3 = hitObj.GetObjectToWorld4x3();
        result += o2w_4x3[0][0];

        // GetWorldToObject3x4 - returns 3x4 world-to-object transform
        float3x4 w2o_3x4 = hitObj.GetWorldToObject3x4();
        result += w2o_3x4[0][0];

        // GetWorldToObject4x3 - returns 4x3 world-to-object transform (transposed)
        float4x3 w2o_4x3 = hitObj.GetWorldToObject4x3();
        result += w2o_4x3[0][0];
    }

    outputBuffer[idx] = result;
}
