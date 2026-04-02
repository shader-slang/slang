// raypayload-missing-access-qualifiers.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test error for field in ray payload struct missing read/write access qualifiers

[raypayload] struct RayPayload
{
    float4 color : read(caller, anyhit) : write(caller);
    float4 colorMissingQualifiers;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^^ ray payload field missing access qualifiers
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^^ field 'colorMissingQualifiers' in ray payload struct must have either 'read' OR 'write' access qualifiers
};

uniform RWTexture2D<float4> resultTexture;
uniform RaytracingAccelerationStructure sceneBVH;

[shader("raygeneration")]
void rayGenShaderA()
{
    int2 threadIdx = DispatchRaysIndex().xy;

    float3 rayDir = float3(0, 0, 1);
    float3 rayOrigin = 0;
    rayOrigin.x = (threadIdx.x * 2) - 1;
    rayOrigin.y = (threadIdx.y * 2) - 1;

    // Trace the ray.
    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) , float4(0, 0, 0, 0)};
    TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    resultTexture[threadIdx.xy] = payload.color;
}
