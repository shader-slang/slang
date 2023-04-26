//TEST_IGNORE_FILE:
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[ForwardDerivative(fwd_f)]
float f(float v)
{
    return v * v;
}

[BackwardDerivative(bwd_fwd_f)]
DifferentialPair<float> fwd_f(DifferentialPair<float> v)
{
    return diffPair(v.p * v.p, v.p * v.d * 2.0);
}

void bwd_fwd_f(
    inout DifferentialPair<DifferentialPair<float>> v,
    DifferentialPair<float>.Differential d_out)
{
    return diffPair(v.p * v.p, v.p * v.d * 2.0);
}

[ForwardDifferentiable]
float test(float v)
{
    return f(v);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var p = diffPair(3.0, 1.0);
    let rs = __fwd_diff(test)(p);
    outputBuffer[0] = rs.d;
}
