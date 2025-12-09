//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitShader -stage closesthit

// Test small payload (4 registers = 16 bytes) - should use direct register access
// CHECK-DAG: optixGetPayload_0
// CHECK-DAG: optixGetPayload_1
// CHECK-DAG: optixGetPayload_2
// CHECK-DAG: optixGetPayload_3
// CHECK-DAG: optixSetPayload_0
// CHECK-DAG: optixSetPayload_1
// CHECK-DAG: optixSetPayload_2
// CHECK-DAG: optixSetPayload_3

struct SmallPayload
{
    float hitT;      // register 0
    float3 color;    // registers 1, 2, 3
};

struct TriangleAttributes
{
    float2 barycentrics;
};

[shader("closesthit")]
void closestHitShader(inout SmallPayload payload, in TriangleAttributes attr)
{
    payload.hitT = RayTCurrent();
    payload.color = float3(attr.barycentrics.x, attr.barycentrics.y, 1.0 - attr.barycentrics.x - attr.barycentrics.y);
}
