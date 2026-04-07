//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[ForwardDerivative(diff_f)]
[Differentiable]
float f(float v)
{
    return v * v;
}

DifferentialPair<float> diff_f(DifferentialPair<float> v)
{
    return diffPair(v.p * v.p, 3 * v.d * v.p); // Wrong on purpose
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var p = diffPair(3.0, 1.0);
    let rs = fwd_diff(f)(p); // The compiler should prefer the custom derivative over the auto-generated one
                             // even though "f" has both.
    outputBuffer[0] = rs.d; // CHECK: 9.0
}
