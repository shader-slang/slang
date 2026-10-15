//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute

// `f16tof32` returns `float`. On Metal, `as_type<half>` alone is a `half`, and
// `min(half, float)` is an ambiguous call in MSL, so the result must be widened.

// CHECK-DAG: min((float(as_type<half>((ushort)(
// CHECK-DAG: result_{{.*}} = (float(as_type<half>((ushort)(

RWStructuredBuffer<uint> inputBuffer;
RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    const float scalar = min(f16tof32(inputBuffer[tid.x]), 1.0f);
    const float2 pair = f16tof32(uint2(inputBuffer[tid.x], inputBuffer[tid.x + 1]));
    outputBuffer[tid.x] = float4(scalar, pair, 0.0);
}
