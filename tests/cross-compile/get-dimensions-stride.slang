//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=4):name buffer0
RWStructuredBuffer<int> buffer0;

//TEST_INPUT:ubuffer(data=[23 2], stride=4):name buffer1
RWStructuredBuffer<int> buffer1;

//TEST_INPUT:ubuffer(data=[-10 17 9 4 2 0], stride=4):name buffer2
RWStructuredBuffer<float> buffer2;

//TEST_INPUT:ubuffer(data=[-10 17 9 4 2 0], stride=4):name buffer3
RWStructuredBuffer<float> buffer3;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    uint count = 0;
    uint stride = 0;

    if (index == 0)
    {
        // CHECK: 4
        buffer0.GetDimensions(count, stride);
    }
    else if (index == 1)
    {
        // CHECK: 4
        buffer1.GetDimensions(count, stride);
    }
    else if (index == 2)
    {
        // CHECK: 4
        buffer2.GetDimensions(count, stride);
    }
    else if (index == 3)
    {
        // CHECK: 4
        buffer3.GetDimensions(count, stride);
    }

    outputBuffer[index] = int(stride);
}
