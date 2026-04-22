//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension bwd_diff for a non-static method of a non-differentiable type.
// The this param appears as no_diff MyData in the bwd_diff signature.

struct MyData
{
    float scale;
    float apply(float x) { return scale * x * x; }
};

__func_extension bwd_diff(MyData::apply)(no_diff MyData self, inout DifferentialPair<float> x, float dOut) -> void
{
    // d/dx(scale * x^2) = scale * 2 * x
    x = diffPair(x.p, dOut * self.scale * 2.0 * x.p);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    MyData d = { 3.0 };
    var dpx = diffPair(2.0, 0.0);
    bwd_diff(MyData::apply)(d, dpx, 1.0);
    // d/dx(3 * x^2) at x=2 = 3 * 2 * 2 = 12
    outputBuffer[0] = dpx.d;
    // CHECK: 12.0
}
