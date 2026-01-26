//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage closesthit -entry largePayloadShader

// Test that payloads exceeding 128 bytes (33+ registers) fall back to pointer method

// Large payload - 136 bytes = 34 registers (should use pointer method)
struct LargePayload
{
    float4 data[34]; // 34 * 4 bytes = 136 bytes (exceeds 128-byte limit)
};

[shader("closesthit")]
void largePayloadShader(inout LargePayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.data[0] = float4(1.0, 0.0, 0.0, 1.0);
}

// CHECK: getOptiXRayPayloadPtr
// CHECK-NOT: optixGetPayload_0
