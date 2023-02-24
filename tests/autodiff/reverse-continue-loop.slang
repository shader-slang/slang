//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;

[BackwardDifferentiable]
float test_loop_with_continue(float y)
{
    float t = y;

    for (int i = 0; i < 3; i++)
    {
        if (t > 4.0)
            continue;

        t = t * t;
    }

    return t;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 0.0);

        __bwd_diff(test_loop_with_continue)(dpa, 1.0f);
        outputBuffer[0] = dpa.d; // Expect: 32.0
    }

    {
        dpfloat dpa = dpfloat(0.4, 0.0);

        __bwd_diff(test_loop_with_continue)(dpa, 1.0f);
        outputBuffer[1] = dpa.d; // Expect: 0.0131072
    }
}
