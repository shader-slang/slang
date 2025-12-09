//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitShader -stage closesthit

// Test mixed int/float payload - should use __int_as_float only for float fields
// CHECK-DAG: optixGetPayload_0()
// CHECK-DAG: __int_as_float(optixGetPayload_1())
// CHECK-DAG: optixGetPayload_2()
// CHECK-DAG: __int_as_float(optixGetPayload_3())
// CHECK-DAG: optixSetPayload_0(
// CHECK-DAG: optixSetPayload_1(__float_as_int(
// CHECK-DAG: optixSetPayload_2(
// CHECK-DAG: optixSetPayload_3(__float_as_int(

struct MixedPayload
{
    int intValue0;     // register 0 - int (no conversion)
    float floatValue0; // register 1 - float (needs __int_as_float)
    uint uintValue0;   // register 2 - uint (no conversion)
    float floatValue1; // register 3 - float (needs __int_as_float)
};

struct TriangleAttributes
{
    float2 barycentrics;
};

[shader("closesthit")]
void closestHitShader(inout MixedPayload payload, in TriangleAttributes attr)
{
    payload.intValue0 = PrimitiveIndex();
    payload.floatValue0 = RayTCurrent();
    payload.uintValue0 = InstanceIndex();
    payload.floatValue1 = attr.barycentrics.x;
}
