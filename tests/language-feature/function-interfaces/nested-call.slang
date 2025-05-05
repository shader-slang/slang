//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test for lowering witness information within function bodies for synthesis.

[Differentiable]
float zig(float x)
{
    return x * x;
}

[Differentiable]
float zag(float x)
{
    return x + 1;
}

[Differentiable]
float zap(float x) 
{
    return 2 * zag(zig(x));
}

extension __func_as_type(zap) : IDifferentiableFuncBase, IForwardDifferentiable<__func_as_type(zap)> { };
extension __func_as_type(zig) : IDifferentiableFuncBase, IForwardDifferentiable<__func_as_type(zig)> { };
extension __func_as_type(zag) : IDifferentiableFuncBase, IForwardDifferentiable<__func_as_type(zag)> { };

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    outputBuffer[tig] = zap.fwd_diff(DifferentialPair<float>(0, 0)).d;
}
