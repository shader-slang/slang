// ray-tracing-test-shader.slang

struct RayPayload
{
    float4 color;
};

uniform RWTexture2D resultTexture;
uniform RaytracingAccelerationStructure sceneBVH;

[shader("raygeneration")]
void rayGenShader()
{
    uint2 threadIdx = DispatchRaysIndex().xy;
    uint screenWidth = 2;
    uint screenHeight = 2;
    if (threadIdx.x >= (int)screenWidth) return;
    if (threadIdx.y >= (int)screenHeight) return;

    float3 rayDir = 0;
    rayDir.x = (threadIdx.x * 2) - 1;
    rayDir.y = (threadIdx.y * 2) - 1;

    // Trace the ray.
    RayDesc ray;
    ray.Origin = 0;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    resultTexture[threadIdx.xy] = payload.color;
}

[shader("miss")]
void missShader(inout RayPayload payload)
{
    payload.color = float4(0, 0, 0, 1);
}

[shader("closesthit")]
void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    let primitiveIndex = PrimitiveIndex();
    float4 color = 0;
    color[primitiveIndex] = 1;
    payload.color = color;
}
