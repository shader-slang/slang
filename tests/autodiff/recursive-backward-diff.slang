//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Regression test for https://github.com/shader-slang/slang/issues/10066
// A recursive [BackwardDifferentiable] function used to cause the compiler
// to hang forever. It should now emit a recursion-not-allowed diagnostic.
//
// The diagnostic appears twice because the autodiff pipeline synthesizes
// both a forward-mode and a backward-mode derivative, each of which
// triggers the recursion check independently.

[BackwardDifferentiable]
float recursiveSum(float x, int n)
{
    if (n <= 0)
        return 0.0;
    return x + recursiveSum(x * 0.5, n - 1);
//CHECK: E55201
//CHECK:                   ^ recursion detected in call to 'recursiveSum'
//CHECK: E55201
//CHECK:                   ^ recursion detected in call to 'recursiveSum'
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    DifferentialPair<float> dp = diffPair(2.0, 0.0);
    __bwd_diff(recursiveSum)(dp, 3, 1.0);
    outputBuffer[0] = dp.d;
}
