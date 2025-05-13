//TEST:SIMPLE(filecheck=CHECK): -target cuda
//CHECK: __global__ void __closesthit__closestHitShaderA
//CHECK: optixGetClusterId
struct RayPayload
{
    float4 color;
};

[shader("closesthit")]
void closestHitShaderA(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    int clusterId = GetClusterID();
    float4 color = float4(0, 0, 0, 1);
    if (clusterId >= 0)
        color[clusterId] = 1;
    payload.color = color;
}
