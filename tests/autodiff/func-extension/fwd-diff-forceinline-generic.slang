//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain -O0 -g3 -experimental-feature

// Test: callable decorators on generic `__func_extension` declarations are
// applied to the synthesized derivative function.

RWStructuredBuffer<float> outputBuffer;

T sqr<T : IArithmetic>(T x) { return x * x; }

[ForceInline]
__func_extension<T : IFloat> fwd_diff(sqr<T>)(DifferentialPair<T> dpx) -> DifferentialPair<T>
    where T.Differential == T
{
    return diffPair(sqr(dpx.p), T(2.0) * dpx.p * T(dpx.d));
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var result = fwd_diff(sqr<float>)(diffPair(3.0, 1.0));
    outputBuffer[0] = result.d;
}

// CHECK-COUNT-1: DebugInlinedAt
// CHECK-NOT: DebugInlinedAt
