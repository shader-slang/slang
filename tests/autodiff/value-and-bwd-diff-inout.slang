// Q3: the surfaced primal y reflects post-call mutation of the wrapped function's
// inout differentiable parameter. withInout mutates p inside its body; the value
// returned from value_and_bwd_diff is computed from the post-mutation value.
// (CPU compute path — the bytecode interpreter does not support backward-diff wrappers.)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[BackwardDifferentiable]
float withInout(inout float p, float k)
{
    // Mutate the inout primal before computing the returned value.
    p = p + 1.0;
    return p * k;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DifferentialPair<float> dpP = DifferentialPair<float>(2.0, 0.0);
    DifferentialPair<float> dpK = DifferentialPair<float>(4.0, 0.0);

    // After mutation p == 3.0; surfaced y == 3 * 4 == 12.0
    float y = value_and_bwd_diff(withInout)(dpP, dpK, 1.0);

    outputBuffer[0] = y; // CHECK: 12.0
}
