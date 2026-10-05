//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

// A user-written forward derivative can be differentiated directly in backward mode.
// For x^3, seeding the tangent result gives 6*x*dx for x and 3*x^2 for dx.
[ForwardDerivative(cubeFwd)]
float cube(float x) { return x * x * x; }

[Differentiable]
DifferentialPair<float> cubeFwd(DifferentialPair<float> x)
{
    return diffPair(x.p * x.p * x.p, 3.0 * x.p * x.p * x.d);
}

struct Cubic
{
    [ForwardDerivative(evalFwd)]
    [NoDiffThis]
    float eval(float x) { return x * x * x; }

    [Differentiable]
    [NoDiffThis]
    DifferentialPair<float> evalFwd(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p * x.p, 3.0 * x.p * x.p * x.d);
    }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var x = diffPair(diffPair(3.0, 1.0));
    __bwd_diff(__fwd_diff(cube))(x, diffPair(0.0, 1.0));
    // CHECK: 18.0
    outputBuffer[0] = x.d.p;
    // CHECK: 27.0
    outputBuffer[1] = x.d.d;

    Cubic c;
    var y = diffPair(diffPair(2.0, 2.0));
    __bwd_diff(__fwd_diff(c.eval))(y, diffPair(0.0, 0.5));
    // CHECK: 12.0
    outputBuffer[2] = y.d.p;
    // CHECK: 6.0
    outputBuffer[3] = y.d.d;
}
