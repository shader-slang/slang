//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension fwd_diff for a static method of a non-differentiable type.

struct MyUtils
{
    static float cube(float x) { return x * x * x; }
};

__func_extension fwd_diff(MyUtils::cube)(DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(MyUtils::cube(x.p), 3.0 * x.p * x.p * x.d);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DifferentialPair<float> dpx = diffPair(2.0, 1.0);
    DifferentialPair<float> result = fwd_diff(MyUtils::cube)(dpx);
    // cube(2.0) = 8.0, derivative = 3 * 4 * 1.0 = 12.0
    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;
    // CHECK: 8.0
    // CHECK: 12.0
}
