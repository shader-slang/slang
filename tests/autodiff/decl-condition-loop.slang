//TEST(compute):COMPARE_COMPUTE_EX:-cpu -source-language hlsl -compute -shaderobj -output-using-type -xslang -Wno-30081
//TEST(compute):COMPARE_COMPUTE_EX:-vk -source-language hlsl -compute -shaderobj -output-using-type -xslang -Wno-30081

// A declaration condition in a differentiable loop lowers the declaration into the
// loop's condition slot, which the differentiable-loop machinery (here `[ForceUnroll]`,
// the same attribute any differentiable loop needs) treats like any other loop-local
// variable. The tests use a non-`bool` condition type, whose conversion
// to `bool` is exactly the feature under test, so the 30081 "implicit conversion not
// recommended" warning is disabled rather than heeded.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float sumOfScale(float x)
{
    float sum = 0.0;
    int n = 3;
    [ForceUnroll]
    while (int i = n--)
        sum += x * float(i);
    return sum;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // f(2) = 2 * (3 + 2 + 1) = 12; df/dx = 6.
    outputBuffer[0] = sumOfScale(2.0);
    outputBuffer[1] = __fwd_diff(sumOfScale)(DifferentialPair<float>(2.0, 1.0)).d;
}
