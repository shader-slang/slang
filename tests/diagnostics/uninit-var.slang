// uninit-var.slang
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -profile cs_6_0

//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -shaderobj

struct Data
{
    float3 value;
};

RWStructuredBuffer<Data>  inputBuffer;
RWStructuredBuffer<Data> outputBuffer;

float foo(out float3 v)
{
    float uninitializedVariable;

    // CHECK: warning
    float r = v.x + uninitializedVariable;
    return r;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    Data inputData = inputBuffer[tid];


    float outputData = foo(inputData.value);

    outputBuffer[tid].value = outputData;
}