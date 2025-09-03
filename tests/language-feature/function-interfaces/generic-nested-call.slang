//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test for lowering witness information within function bodies for synthesis.

[Differentiable]
T zig<T : IFloat>(T x)
{
    return x * x;
}

[Differentiable]
T zag<T : IFloat>(T x)
{
    return x + (T)1;
}

[Differentiable]
T zap<T : IFloat>(T x)
{
    return (T)2 * zag(zig(x));
}

extension<T : IFloat> __func_as_type(zap<T>) : IDifferentiableFuncBase, IForwardDifferentiable<__func_as_type(zap<T>)> { };
extension<T : IFloat> __func_as_type(zig<T>) : IDifferentiableFuncBase, IForwardDifferentiable<__func_as_type(zig<T>)> { };
extension<T : IFloat> __func_as_type(zag<T>) : IDifferentiableFuncBase, IForwardDifferentiable<__func_as_type(zag<T>)> { };

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    outputBuffer[0] = zap<float>.fwd_diff(DifferentialPair<float>(2.f, 0)).d;
    outputBuffer[1] = zap<float2>.fwd_diff(DifferentialPair<float2>(float2(3.f, 5.f), 0)).d.y;
    outputBuffer[2] = zap<float3>.fwd_diff(DifferentialPair<float3>(float3(4.f, 5.f, 6.f), 0)).d.z;
}
