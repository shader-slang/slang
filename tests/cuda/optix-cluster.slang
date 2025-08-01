//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry closestHitShaderA
//CHECK: __global__ void __closesthit__closestHitShaderA
//CHECK: optixGetClusterId
struct RayPayload
{
    float4 color;
};

[shader("closesthit")]
void closestHitShaderA(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    //CHECK-PTX:_optix_get_cluster_id 
    int clusterId = GetClusterID();
    float4 color = float4(0, 0, 0, 1);
    if (clusterId >= 0)
        color[clusterId] = 1;
    payload.color = color;
}
