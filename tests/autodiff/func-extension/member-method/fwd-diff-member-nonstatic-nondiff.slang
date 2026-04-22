//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension fwd_diff for a non-static method of a non-differentiable type.
// The this param should appear as no_diff MyData in the fwd_diff signature.

struct MyData
{
    float scale;
    float apply(float x) { return scale * x * x; }
};

__func_extension fwd_diff(MyData::apply)(no_diff MyData self, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(self.scale * x.p * x.p, self.scale * 2.0 * x.p * x.d);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    MyData d = { 3.0 };
    DifferentialPair<float> dpx = diffPair(2.0, 1.0);
    // d.apply(x) = 3 * x^2
    // d/dx = 3 * 2 * x = 12.0
    DifferentialPair<float> result = fwd_diff(MyData::apply)(d, dpx);
    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;
    // CHECK: 12.0
    // CHECK: 12.0
}
