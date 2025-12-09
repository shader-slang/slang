//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitShader -stage closesthit

// Test nested structs - recursive decomposition should work
// CHECK-DAG: optixGetPayload_0
// CHECK-DAG: optixGetPayload_1
// CHECK-DAG: optixGetPayload_2
// CHECK-DAG: optixSetPayload_0
// CHECK-DAG: optixSetPayload_1
// CHECK-DAG: optixSetPayload_2
// Verify NOT using pointer method:
// CHECK-NOT: getOptiXRayPayloadPtr

struct Inner
{
    float x;
    float y;
};

struct NestedPayload
{
    Inner inner;    // registers 0-1 (nested struct)
    float z;        // register 2
};

struct TriangleAttributes
{
    float2 barycentrics;
};

[shader("closesthit")]
void closestHitShader(inout NestedPayload payload, in TriangleAttributes attr)
{
    payload.inner.x = RayTCurrent();
    payload.inner.y = attr.barycentrics.x;
    payload.z = attr.barycentrics.y;
}
