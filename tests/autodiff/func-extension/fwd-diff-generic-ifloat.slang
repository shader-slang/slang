//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension fwd_diff with generic IFloat constraint

T sqr<T : IArithmetic>(T x) { return x * x; }

// Forward derivative for any IFloat specialization where T.Differential == T
__func_extension<T : IFloat> fwd_diff(sqr<T>)(DifferentialPair<T> dpx) -> DifferentialPair<T>
    where T.Differential == T
{
    return diffPair(sqr(dpx.p), T(2.0) * dpx.p * T(dpx.d));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DifferentialPair<float> dpx = diffPair(3.0, 1.0);
    DifferentialPair<float> result = fwd_diff(sqr<float>)(dpx);
    // sqr(3.0) = 9.0, derivative = 2 * 3.0 * 1.0 = 6.0
    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;
    // CHECK: 9.0
    // CHECK: 6.0
}
