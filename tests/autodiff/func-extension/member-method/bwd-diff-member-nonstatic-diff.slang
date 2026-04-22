//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension bwd_diff for a non-static method of a differentiable type.
// The this param appears as inout DifferentialPair<MyVec> in the bwd_diff signature.

struct MyVec : IDifferentiable
{
    typedef MyVec Differential;
    float x;
    float y;

    float lengthSquared() { return x * x + y * y; }
};

__func_extension bwd_diff(MyVec::lengthSquared)(inout DifferentialPair<MyVec> self, float dOut) -> void
{
    // d/d(self) of x^2 + y^2 = (2x, 2y)
    self = diffPair(self.p, MyVec(dOut * 2.0 * self.p.x, dOut * 2.0 * self.p.y));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var dpv = diffPair(MyVec(3.0, 4.0), MyVec(0.0, 0.0));
    bwd_diff(MyVec::lengthSquared)(dpv, 1.0);
    // d/dx = 2*3 = 6, d/dy = 2*4 = 8
    outputBuffer[0] = dpv.d.x;
    outputBuffer[1] = dpv.d.y;
    // CHECK: 6.0
    // CHECK: 8.0
}
