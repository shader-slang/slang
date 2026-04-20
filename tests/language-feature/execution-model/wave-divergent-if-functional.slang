//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int branchMin = 0;
    if ((idx & 1) == 0)
        branchMin = WaveActiveMin(idx);
    else
        branchMin = WaveActiveMin(idx);
    int total = WaveActiveSum(branchMin);
    outputBuffer[idx] = total;
}

// CHECK: 2
// CHECK: 2
// CHECK: 2
// CHECK: 2
