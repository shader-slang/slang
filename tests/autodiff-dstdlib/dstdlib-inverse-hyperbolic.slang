//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -use-dxil -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

[BackwardDifferentiable]
float diffAsinh(float x)
{
    return asinh(x);
}

[BackwardDifferentiable]
float diffAcosh(float x)
{
    return acosh(x);
}

[BackwardDifferentiable]
float diffAtanh(float x)
{
    return atanh(x);
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    var index = 0U;

    let sinhValue = 2;
    {
        // Expected: 1 / sqrt(x^2 + 1) = 1 / sqrt(4 + 1) = 0.447214
        // CHECK: 0.447214
        dpfloat dpx = dpfloat(sinhValue, 1.0);
        dpfloat res = __fwd_diff(diffAsinh)(dpx);
        outputBuffer[index++] = res.d;
    }
    {
        // Check backward mode agrees with forward
        // CHECK: 0.447214
        dpfloat dpx = diffPair(sinhValue);
        __bwd_diff(diffAsinh)(dpx, 1.0);
        outputBuffer[index++] = dpx.d;
    }

    let coshValue = 4;
    {
        // Expected: 1 / sqrt(x^2 + 1) = 1 / sqrt(16 - 1) = 0.258199
        // CHECK: 0.258199
        dpfloat dpx = dpfloat(coshValue, 1.0);
        dpfloat res = __fwd_diff(diffAcosh)(dpx);
        outputBuffer[index++] = res.d;
    }
    {
        // Check backward mode agrees with forward
        // CHECK: 0.258199
        dpfloat dpx = diffPair(coshValue);
        __bwd_diff(diffAcosh)(dpx, 1.0);
        outputBuffer[index++] = dpx.d;
    }


    let tanhValue = 0.5;
    {
        // Expected: 1 / (1 - x^2) = 1 / (1 - 0.25) = 1.333...
        // CHECK: 1.3333
        dpfloat dpx = dpfloat(tanhValue, 1.0);
        dpfloat res = __fwd_diff(diffAtanh)(dpx);
        outputBuffer[index++] = res.d;
    }
    {
        // Check backward mode agrees with forward
        // CHECK: 1.3333
        dpfloat dpx = diffPair(tanhValue);
        __bwd_diff(diffAtanh)(dpx, 1.0);
        outputBuffer[index++] = dpx.d;
    }
}


