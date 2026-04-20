//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int caseValue = 0;
    switch (idx)
    {
    case 0: caseValue = 100; break;
    case 1: caseValue = 200; break;
    case 2: caseValue = 300; break;
    default: caseValue = 400; break;
    }
    outputBuffer[idx] = WaveActiveSum(caseValue);
}

// CHECK: 1000
// CHECK: 1000
// CHECK: 1000
// CHECK: 1000
