//TEST:COMPARE_COMPUTE(targets=all-vk-cuda, filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Returns 1 for a test of a target that `targets=all-vk-cuda` removes. No such test must run.
int isExcludedTarget()
{
#if defined(SLANG_TEST_VK) || defined(SLANG_TEST_CUDA)
    return 1;
#else
    return 0;
#endif
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = int(dispatchThreadID.x) * 3 + isExcludedTarget();
}

// CHECK:      0
// CHECK-NEXT: 3
// CHECK-NEXT: 6
// CHECK-NEXT: 9
