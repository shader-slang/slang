// ser-test-shader-table.slang
// Test HitObject Shader Table Methods

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

    uint result = 0;

    // GetShaderTableIndex - returns shader table index
    uint shaderTableIdx = hitObj.GetShaderTableIndex();
    result += shaderTableIdx;

    // SetShaderTableIndex - sets shader table index, returns old value
    uint oldIdx = hitObj.SetShaderTableIndex(5);
    result += oldIdx;

    // LoadLocalRootTableConstant - loads a root constant from local root table
    // RootConstantOffsetInBytes must be multiple of 4
    uint rootConstant = hitObj.LoadLocalRootTableConstant(0);
    result += rootConstant;

    outputBuffer[idx] = result;
}
