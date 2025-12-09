//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitShader -stage closesthit

// Test 64-bit types - double and int64 each need 2 registers (2 x 32-bit = 64-bit)
// A struct with 1 double + 1 int64 = 4 registers total (not 2!)
// This uses direct register access since 4 < 32
// 64-bit values are reconstructed from two consecutive 32-bit registers:
//   double = __longlong_as_double((long long)high32 << 32 | low32)
//   int64  = (long long)high32 << 32 | low32
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
