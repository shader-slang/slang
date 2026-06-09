// value_and_bwd_diff smoke test: scalar primal + backward gradients in one call.
// f(x) = x*x; value_and_bwd_diff(f)(p, 1.0) returns f(x) and writes 2*x into p.d.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[BackwardDifferentiable]
float square(float x)
{
    return x * x;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DifferentialPair<float> p = DifferentialPair<float>(3.0, 0.0);

    // Surfaced primal y = 3*3 = 9.0
    float y = value_and_bwd_diff(square)(p, 1.0);

    outputBuffer[0] = y;     // CHECK: 9.0
    outputBuffer[1] = p.d;   // CHECK-NEXT: 6.0
}
