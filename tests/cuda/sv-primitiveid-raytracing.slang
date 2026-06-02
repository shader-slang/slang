// Regression test for #11197: SV_PrimitiveID hit-stage parameters lower to
// OptiX primitive-index access on CUDA instead of being emitted as parameters.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitShader -stage closesthit
//TEST:SIMPLE(filecheck=INT_CHECK): -target cuda -entry closestHitShaderInt -stage closesthit

// CHECK: __global__ void __closesthit__closestHitShader
// CHECK-NOT: primitiveID
// CHECK: optixGetPrimitiveIndex()

// INT_CHECK: __global__ void __closesthit__closestHitShaderInt
// INT_CHECK-NOT: primitiveID
// INT_CHECK: uint [[PRIMITIVE_ID:[A-Za-z_][A-Za-z_0-9]*]] = optixGetPrimitiveIndex()
// INT_CHECK: {{getPrimitiveIDAsFloat[^ ]*}}(int([[PRIMITIVE_ID]]))

struct Payload
{
    float value;
};

float getPrimitiveIDAsFloat(in int value)
{
    return float(value);
}

[shader("closesthit")]
void closestHitShader(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    uint primitiveID : SV_PrimitiveID)
{
    payload.value = float(primitiveID) + attribs.barycentrics.x + attribs.barycentrics.y;
}

[shader("closesthit")]
void closestHitShaderInt(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    int primitiveID : SV_PrimitiveID)
{
    payload.value =
        getPrimitiveIDAsFloat(primitiveID) + attribs.barycentrics.x + attribs.barycentrics.y;
}
