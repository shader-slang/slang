//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain -O0 -g3

// Test: callable decorators on `__func_extension` are applied to the
// synthesized derivative function.

RWStructuredBuffer<float> outputBuffer;

float cube(float x) { return x * x * x; }

[ForceInline]
__func_extension fwd_diff(cube)(DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(cube(x.p), 3.0 * x.p * x.p * x.d);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var result = fwd_diff(cube)(diffPair(2.0, 1.0));
    outputBuffer[0] = result.d;
}

// CHECK-COUNT-1: DebugInlinedAt
// CHECK-NOT: DebugInlinedAt
