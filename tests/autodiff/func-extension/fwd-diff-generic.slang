//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension fwd_diff with generic specialization

T sqr<T : IArithmetic>(T x) { return x * x; }

// Forward derivative only for float specialization
__func_extension fwd_diff(sqr<float>)(DifferentialPair<float> dpx) -> DifferentialPair<float>
{
    return diffPair(sqr(dpx.p), 2.0f * dpx.p * dpx.d);
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
