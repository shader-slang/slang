//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// Regression coverage for https://github.com/shader-slang/slang/issues/11160.
// A differentiated out/inout value can itself be a DifferentialPair. Both fields of its
// gradient must survive the forward load and reverse load/store transformations.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
void square(float x, out float y)
{
    y = x * x;
}

[Differentiable]
void squareInout(float x, inout float y)
{
    y = y * x * x;
}

[Differentiable]
float objective(float x, float seed, no_diff bool primal)
{
    DifferentialPair<float> y;
    fwd_diff(square)(diffPair(x, seed), y);
    return primal ? y.p : y.d;
}

[Differentiable]
float objectiveInout(float x, float seed)
{
    DifferentialPair<float> y = diffPair(5.0, 7.0);
    fwd_diff(squareInout)(diffPair(x, seed), y);
    return y.p + y.d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // The tangent is 2*x*seed. At x=3 and seed=2, its gradient is (4, 6).
    var x = diffPair(3.0);
    var seed = diffPair(2.0);
    bwd_diff(objective)(x, seed, false, 1.0);
    // CHECK: 4.0
    outputBuffer[0] = x.d;
    // CHECK: 6.0
    outputBuffer[1] = seed.d;

    // The primal is x*x, whose gradient is (6, 0).
    x = diffPair(3.0);
    seed = diffPair(2.0);
    bwd_diff(objective)(x, seed, true, 1.0);
    // CHECK: 6.0
    outputBuffer[2] = x.d;
    // CHECK: 0.0
    outputBuffer[3] = seed.d;

    // y.p+y.d = 12*x*x+10*x*seed; its gradient is (92, 30).
    x = diffPair(3.0);
    seed = diffPair(2.0);
    bwd_diff(objectiveInout)(x, seed, 1.0);
    // CHECK: 92.0
    outputBuffer[4] = x.d;
    // CHECK: 30.0
    outputBuffer[5] = seed.d;
}
