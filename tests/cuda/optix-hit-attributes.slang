//TEST:SIMPLE(filecheck=CHECK): -target cuda
//CHECK: __global__ void __closesthit__closestHitShaderA
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/"
struct RayPayload
{
    float4 color;
};

[shader("closesthit")]
void closestHitShaderA(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    //CHECK-PTX: _optix_read_primitive_idx
    uint primitiveIndex = PrimitiveIndex();
    float4 color = float4(0, 0, 0, 1);
    color[primitiveIndex] = 1;
    payload.color = color;
}
