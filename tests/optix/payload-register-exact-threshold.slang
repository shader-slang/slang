//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage closesthit -entry maxRegisterPayloadShader

// Test payload at exactly the 128-byte threshold (32 registers)
// This should still use direct register access (not fall back to pointer)

// Exactly 128 bytes = 32 float values = 32 registers
struct MaxRegisterPayload
{
    float data[32]; // Exactly 32 * 4 bytes = 128 bytes
};

[shader("closesthit")]
void maxRegisterPayloadShader(inout MaxRegisterPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.data[0] = RayTCurrent();
    payload.data[31] = 1.0;
}

// Should use direct register access (all 32 registers)
// CHECK-DAG: optixGetPayload_0
// CHECK-DAG: optixGetPayload_31
// CHECK-DAG: optixSetPayload_0
// CHECK-DAG: optixSetPayload_31
// CHECK-NOT: getOptiXRayPayloadPtr
