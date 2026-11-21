//TEST:COMPARE_COMPUTE(targets=none, filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(targets=cpu, filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// `targets=none` makes no test, so the first line never runs. If it ran on any target, the check
// would fail, because only a test of `cpu` makes the buffer that CHECK expects.
int isCpu()
{
#if defined(SLANG_TEST_CPU)
    return 1;
#else
    return 0;
#endif
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = int(dispatchThreadID.x) + isCpu() * 10;
}

// CHECK:      10
// CHECK-NEXT: 11
// CHECK-NEXT: 12
// CHECK-NEXT: 13
