//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for issue #11029 (mixed differentiable + constexpr params):
// Multiple constexpr params interspersed with differentiable params exercise
// the parameter-iteration loops in propagateParamTypes, splitCall, and
// transposeCallInst to catch any off-by-one errors.

[BackwardDerivative(scale2_bwd)]
float scale2(float a, constexpr int m1, float b, constexpr int m2)
{
    return a * float(m1) + b * float(m2);
}

void scale2_bwd(
    inout DifferentialPair<float> a,
    constexpr int m1,
    inout DifferentialPair<float> b,
    constexpr int m2,
    float dOut)
{
    a = DifferentialPair<float>(a.p, dOut * float(m1));
    b = DifferentialPair<float>(b.p, dOut * float(m2));
}

[Differentiable]
float run<let M1 : int, let M2 : int>(float a, float b)
{
    return scale2(a, M1, b, M2);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // d(2*a + 3*b)/da = 2, d(2*a + 3*b)/db = 3
    var pa = diffPair(1.0f);
    var pb = diffPair(1.0f);
    __bwd_diff(run<2, 3>)(pa, pb, 1.0f);
    outputBuffer[0] = pa.d; // gradient w.r.t. a: 2
    //CHECK: 2
    outputBuffer[1] = pb.d; // gradient w.r.t. b: 3
    //CHECK: 3

    // Different specialization: m1=5, m2=7
    var qa = diffPair(1.0f);
    var qb = diffPair(1.0f);
    __bwd_diff(run<5, 7>)(qa, qb, 1.0f);
    outputBuffer[2] = qa.d; // gradient w.r.t. a: 5
    //CHECK: 5
    outputBuffer[3] = qb.d; // gradient w.r.t. b: 7
    //CHECK: 7
}
