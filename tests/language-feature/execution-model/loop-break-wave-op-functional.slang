//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int result = 0;
    for (int i = 0; i < 4; ++i)
    {
        int activeCount = int(WaveActiveCountBits(true));
        if (i == idx)
        {
            result = activeCount;
            break;
        }
    }
    outputBuffer[idx] = result;
}

// CHECK: 4
// CHECK: 3
// CHECK: 2
// CHECK: 1
