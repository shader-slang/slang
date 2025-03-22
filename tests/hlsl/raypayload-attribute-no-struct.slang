//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile lib_6_7 -stage raygeneration -entry rayGenShaderA

// CHECK: struct [raypayload]

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
    float4 payload = float4(0, 0, 0, 0);
    TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    resultTexture[threadIdx.xy] = payload;
}
