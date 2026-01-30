// Test for HitObject.SetShaderTableIndex capability restriction
// GitHub Issue: https://github.com/shader-slang/slang/issues/9509
// SetShaderTableIndex should not be available with hlsl_nvapi (sm_6_5), only with DXR native (sm_6_9).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile sm_6_9 -capability ser_dxr -entry rayGenShaderSetAndGetShaderTableIndex -stage raygeneration

// CHECK: .SetShaderTableIndex

//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR):-target hlsl -profile sm_6_5 -capability hlsl_nvapi -entry rayGenShaderSetAndGetShaderTableIndex -stage raygeneration

// ERROR: error 41011

[raypayload]
struct RayPayload
{
    int queryWasSuccess : read(caller) : write(caller, closesthit, miss);
};

uniform RaytracingAccelerationStructure sceneBVH;

[shader("raygeneration")]
void rayGenShaderSetAndGetShaderTableIndex()
{
    float3 tgt = {0.25, 0.25, 1.0};
    float3 pos = {0.0, 0.0, 0.0};
    float3 dir = tgt - pos;

    RayDesc ray;
    ray.Origin = pos;
    ray.Direction = dir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = {};
    HitObject hitObjectHit = HitObject.TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    uint shaderTableIndex = 0xF00D;
    hitObjectHit.SetShaderTableIndex(shaderTableIndex);
}
