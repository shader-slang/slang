// Regression test for https://github.com/shader-slang/slang/issues/10066
// Compiler hangs on recursive [BackwardDifferentiable]. Uses TEST_IGNORE_FILE until fixed.
// Run manually: timeout 5 slangc recursive-backward-diff-hang.slang -target spirv -entry computeMain -stage compute

//TEST_IGNORE_FILE:

[BackwardDifferentiable]
float recursiveSum(float x, int n)
{
    if (n <= 0)
        return 0.0;
    return x + recursiveSum(x * 0.5, n - 1);
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    DifferentialPair<float> dp = diffPair(1.0, 0.0);
    __bwd_diff(recursiveSum)(dp, 3, 1.0);
}
