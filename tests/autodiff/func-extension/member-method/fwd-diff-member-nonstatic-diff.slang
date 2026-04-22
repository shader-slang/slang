//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension fwd_diff for a non-static method of a differentiable type.
// The this param should appear as DifferentialPair<MyVec> in the fwd_diff signature.

struct MyVec : IDifferentiable
{
    typedef MyVec Differential;
    float x;
    float y;

    float lengthSquared() { return x * x + y * y; }
};

__func_extension fwd_diff(MyVec::lengthSquared)(DifferentialPair<MyVec> self) -> DifferentialPair<float>
{
    float p = self.p.lengthSquared();
    // d/d(self) of x^2 + y^2 = 2*x*dx + 2*y*dy
    float d = 2.0 * self.p.x * self.d.x + 2.0 * self.p.y * self.d.y;
    return diffPair(p, d);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    MyVec v = { 3.0, 4.0 };
    MyVec dv = { 1.0, 1.0 };
    var dpv = diffPair(v, dv);
    DifferentialPair<float> result = fwd_diff(MyVec::lengthSquared)(dpv);
    // lengthSquared = 9 + 16 = 25
    // d = 2*3*1 + 2*4*1 = 14
    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;
    // CHECK: 25.0
    // CHECK: 14.0
}
