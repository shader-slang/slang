//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -output-using-type -shaderobj
//TEST:SIMPLE(filecheck=CHK):-target glsl -stage compute -entry computeMain -report-checkpoint-intermediates

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;

[BackwardDifferentiable]
float g(float y)
{
    float result = 2.0f * y;
    return result * result;
}

//CHK: note: checkpointing context of 4 bytes associated with function: 'f'
[BackwardDifferentiable]
float f(float x)
{
    //CHK: note: 4 bytes (float) used to checkpoint the following item:
    return 3.0f * g(2.0f * x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    dpfloat dpa = dpfloat(1.0, 0.0);

    __bwd_diff(f)(dpa, 1.0f);
    outputBuffer[0] = dpa.d; // Expect: 96.0
}

//CHK-NOT: note