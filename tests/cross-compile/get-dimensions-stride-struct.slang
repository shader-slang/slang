//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d11 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct MS
{
    uint a;
    uint b;
}

//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=8):name buffer0
RWStructuredBuffer<MS> buffer0;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    uint count = 0;
    uint stride = 0;

    // CHECK: 8
    buffer0.GetDimensions(count, stride);

    outputBuffer[index] = int(stride);
}
