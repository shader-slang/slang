//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu

// CHECK: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

uint testFunc(uint n)
{
    const uint CONST_A = 0;
    const uint CONST_B = 1;

    switch (n)
    {
    case CONST_A:
        return 0;
    case CONST_B:
        return 1;
    }

    return 2;
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = testFunc(0);
    outputBuffer[1] = testFunc(1);
    outputBuffer[2] = testFunc(2);
}
