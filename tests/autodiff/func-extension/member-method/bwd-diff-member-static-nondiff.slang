//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension bwd_diff for a static method of a non-differentiable type.

struct MyUtils
{
    static float cube(float x) { return x * x * x; }
};

__func_extension bwd_diff(MyUtils::cube)(inout DifferentialPair<float> x, float dOut) -> void
{
    x = diffPair(x.p, dOut * 3.0 * x.p * x.p);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var dpx = diffPair(2.0, 0.0);
    bwd_diff(MyUtils::cube)(dpx, 1.0);
    // d/dx(x^3) at x=2 = 12
    outputBuffer[0] = dpx.d;
    // CHECK: 12.0
}
