// Regression test for #11197: SV_PrimitiveID hit-stage parameters lower to
// OptiX primitive-index access on CUDA instead of being emitted as parameters.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitShader -stage closesthit

// CHECK: __global__ void __closesthit__closestHitShader
// CHECK-NOT: primitiveID
// CHECK: optixGetPrimitiveIndex()

struct Payload
{
    float value;
};

[shader("closesthit")]
void closestHitShader(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    uint primitiveID : SV_PrimitiveID)
{
    payload.value = float(primitiveID) + attribs.barycentrics.x + attribs.barycentrics.y;
}
