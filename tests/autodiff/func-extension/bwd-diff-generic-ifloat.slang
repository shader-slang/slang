//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension bwd_diff with generic IFloat constraint

T sqr<T : IArithmetic>(T x) { return x * x; }

// Backward derivative for any IFloat specialization where T.Differential == T
__func_extension<T : IFloat> bwd_diff(sqr<T>)(inout DifferentialPair<T> dpx, T dOut) -> void
    where T.Differential == T
{
    dpx = diffPair(dpx.p, dOut * T(2.0) * dpx.p);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var dpx = diffPair(3.0, 0.0);
    bwd_diff(sqr<float>)(dpx, 1.0);
    // d/dx(x^2) at x=3.0 = 2 * 3.0 = 6.0
    outputBuffer[0] = dpx.d;
    // CHECK: 6.0
}
