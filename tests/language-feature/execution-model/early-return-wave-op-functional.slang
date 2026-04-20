//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    if (idx == 0)
    {
        outputBuffer[idx] = -1;
        return;
    }
    int minVal = WaveActiveMin(idx);
    outputBuffer[idx] = minVal;
}

// CHECK: -1
// CHECK: 1
// CHECK: 1
// CHECK: 1
