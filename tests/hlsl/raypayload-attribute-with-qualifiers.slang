//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage raygeneration -entry rayGenShaderA

// CHECK: struct [raypayload]
// CHECK: float4 color : read(caller) : write(closesthit, miss);
// CHECK: float2 uv : read(anyhit, closesthit) : write(caller);

struct [raypayload] RayPayload
{
    // Pure output from closesthit or miss, read by caller
    float4 color : read(caller) : write(closesthit, miss);
    
    // Pure input from caller, read by hit shaders
    float2 uv : read(anyhit, closesthit) : write(caller);
};

uniform RWTexture2D resultTexture;
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
    RayPayload payload = { float4(0, 0, 0, 0), float2(threadIdx) / float2(DispatchRaysDimensions().xy) };
    TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    resultTexture[threadIdx.xy] = payload.color;
} 