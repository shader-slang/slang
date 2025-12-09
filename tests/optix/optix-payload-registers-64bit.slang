//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitShader -stage closesthit

// Test 64-bit types - double and int64 each need 2 registers (2 x 32-bit = 64-bit)
// A struct with 1 double + 1 int64 = 4 registers total (not 2!)
// This should use direct register access since 4 < 32
//
// BUG: Currently 64-bit types are incorrectly treated as 1 register.
// This test MUST FAIL until the bug is fixed.
// When fixed, the code should reconstruct 64-bit values from two 32-bit registers:
//   double = __longlong_as_double((long long)optixGetPayload_1() << 32 | optixGetPayload_0())
//   int64  = (long long)optixGetPayload_3() << 32 | optixGetPayload_2()

// Correct behavior requires 4 registers (0-3), not 2 (0-1)
// CHECK-DAG: optixGetPayload_0
// CHECK-DAG: optixGetPayload_1
// CHECK-DAG: optixGetPayload_2
// CHECK-DAG: optixGetPayload_3
// CHECK-DAG: optixSetPayload_0
// CHECK-DAG: optixSetPayload_1
// CHECK-DAG: optixSetPayload_2
// CHECK-DAG: optixSetPayload_3

struct Payload64Bit
{
    double hitDistance;  // registers 0-1 (64-bit = 2 registers)
    int64_t hitId;       // registers 2-3 (64-bit = 2 registers)
};

struct TriangleAttributes
{
    float2 barycentrics;
};

[shader("closesthit")]
void closestHitShader(inout Payload64Bit payload, in TriangleAttributes attr)
{
    payload.hitDistance = double(RayTCurrent());
    payload.hitId = int64_t(PrimitiveIndex());
}
