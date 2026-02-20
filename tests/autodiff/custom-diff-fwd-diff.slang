//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
DifferentialPair<float> mySqr_fwd(DifferentialPair<float> x)
{
    return DifferentialPair<float>(x.p * x.p, 3.0 * x.p * x.d);
}

[ForwardDerivative(mySqr_fwd)]
float mySqr(float x)
{
    return x * x;
}

[ForwardDifferentiable]
float f(float x)
{
    return mySqr(x * x);
}

[ForwardDifferentiable]
float df(float x)
{
    return fwd_diff(f)(DifferentialPair<float>(x, 1.0)).d;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Given f(x) = (x^2) * (x^2), and 
    // f'(x) = (3x * x^2) + (x^2 * 3x) 
    // Note: intentionally wrong to catch errors where the custom derivative is not used
    //
    // f''(x) = 2 * ((6x^2) + (3x^2))
    // f''(x) = 18 * x^2
    //
    // Expect f''(4) = 288
    //
    outputBuffer[0] = fwd_diff(df)(DifferentialPair<float>(4.0, 1.0)).d; // CHECK: 288
}
