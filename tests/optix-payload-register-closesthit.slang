//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage closesthit -entry main

// Test that small payloads use direct register access in closest-hit shaders

struct SmallPayload
{
    float hitT;
    float3 color;
};

// CHECK-DAG: optixGetPayload_0
// CHECK-DAG: optixGetPayload_1
// CHECK-DAG: optixGetPayload_2
// CHECK-DAG: optixGetPayload_3
// CHECK-NOT: getOptiXRayPayloadPtr

[shader("closesthit")]
void main(inout SmallPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.hitT = RayTCurrent();
    payload.color = float3(1.0, 0.0, 0.0);
}
