//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj
//TEST:SIMPLE(filecheck=CHK):-target glsl -stage compute -entry computeMain -report-checkpoint-intermediates

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;

[BackwardDifferentiable]
float f(int p, float x)
{
    float y = 1.0;
    if (p == 0)
        // Test that generic intrinsics get recomputed
        y = rcp(x);

    return y * y;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    dpfloat dpa = dpfloat(2.0, 0.0);

    bwd_diff(f)(0, dpa, 1.0f);
    outputBuffer[0] = dpa.d; // CHECK: -0.25
}

// CHK: note: no checkpoint contexts to report