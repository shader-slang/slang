//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float2> dpfloat2;

[BackwardDifferentiable]
float diffLog2(float x)
{
    return log2(x);
}

[BackwardDifferentiable]
float2 diffLog2(float2 x)
{
    return log2(x);
}

[BackwardDifferentiable]
float diffExp2(float x)
{
    return exp2(x);
}

[BackwardDifferentiable]
float2 diffExp2(float2 x)
{
    return exp2(x);
}

[BackwardDifferentiable]
float diffLog10(float x)
{
    return log10(x);
}

[BackwardDifferentiable]
float2 diffLog10(float2 x)
{
    return log10(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // Test log2 derivative: d/dx(log2(x)) = 1/(x * ln(2))
    // For x=8, derivative = 1/(8 * 0.693147) = 0.180337
    // With dOut=2.0: 2.0 * 0.180337 = 0.360674
    {
        dpfloat dpa = dpfloat(8.0, 0.0);
        bwd_diff(diffLog2)(dpa, 2.0);
        outputBuffer[0] = dpa.d; // CHECK: 0.360674
    }

    // Test log2 with vector
    // For x=2, derivative = 1/(2 * 0.693147) = 0.721348
    // For x=4, derivative = 1/(4 * 0.693147) = 0.360674
    {
        dpfloat2 dpx = dpfloat2(float2(2.0, 4.0), float2(0.0, 0.0));
        bwd_diff(diffLog2)(dpx, float2(1.0, 1.0));
        outputBuffer[1] = dpx.d[0]; // CHECK: 0.721348
        outputBuffer[2] = dpx.d[1]; // CHECK: 0.360674
    }

    // Test exp2 derivative: d/dx(2^x) = 2^x * ln(2)
    // For x=3, derivative = 8 * 0.693147 = 5.545177
    // With dOut=1.5: 1.5 * 5.545177 = 8.317766
    {
        dpfloat dpa = dpfloat(3.0, 0.0);
        bwd_diff(diffExp2)(dpa, 1.5);
        outputBuffer[3] = dpa.d; // CHECK: 8.317766
    }

    // Test exp2 with vector
    // For x=2, derivative = 4 * 0.693147 = 2.772589
    // For x=1, derivative = 2 * 0.693147 = 1.386294
    {
        dpfloat2 dpx = dpfloat2(float2(2.0, 1.0), float2(0.0, 0.0));
        bwd_diff(diffExp2)(dpx, float2(1.0, 1.0));
        outputBuffer[4] = dpx.d[0]; // CHECK: 2.772589
        outputBuffer[5] = dpx.d[1]; // CHECK: 1.386294
    }

    // Test forward mode for log2
    // d/dx(log2(4)) = 1/(4 * ln(2)) = 0.360674
    {
        dpfloat dpx = dpfloat(4.0, 1.0);
        dpfloat result = fwd_diff(diffLog2)(dpx);
        outputBuffer[6] = result.d; // CHECK: 0.360674
    }

    // Test forward mode for exp2
    // d/dx(2^2) = 4 * ln(2) = 2.772589
    {
        dpfloat dpx = dpfloat(2.0, 1.0);
        dpfloat result = fwd_diff(diffExp2)(dpx);
        outputBuffer[7] = result.d; // CHECK: 2.772589
    }

    // Test log10 derivative: d/dx(log10(x)) = 1/(x * ln(10))
    // For x=100, derivative = 1/(100 * 2.302585) = 0.004343
    // With dOut=2.0: 2.0 * 0.004343 = 0.008686
    {
        dpfloat dpa = dpfloat(100.0, 0.0);
        bwd_diff(diffLog10)(dpa, 2.0);
        outputBuffer[8] = dpa.d; // CHECK: 0.008686
    }

    // Test log10 with vector
    // For x=10, derivative = 1/(10 * 2.302585) = 0.043429
    // For x=1000, derivative = 1/(1000 * 2.302585) = 0.000434
    {
        dpfloat2 dpx = dpfloat2(float2(10.0, 1000.0), float2(0.0, 0.0));
        bwd_diff(diffLog10)(dpx, float2(1.0, 1.0));
        outputBuffer[9] = dpx.d[0]; // CHECK: 0.043429
        outputBuffer[10] = dpx.d[1]; // CHECK: 0.000434
    }

    // Test forward mode for log10
    // d/dx(log10(100)) = 1/(100 * ln(10)) = 0.004343
    {
        dpfloat dpx = dpfloat(100.0, 1.0);
        dpfloat result = fwd_diff(diffLog10)(dpx);
        outputBuffer[11] = result.d; // CHECK: 0.004343
    }
}
