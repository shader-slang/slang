//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    // The point of this test is to just check that __alignOf isn't implemented
    // as a C macro that misinterprets templates as multiple arguments and fails
    // to compile :) That's why it doesn't care much about the actual values of
    // the alignments.
    outputBuffer[0] = __alignOf<float2>() > 0 ? 1 : 0; // CHECK: 1
    outputBuffer[1] = __alignOf<vector<int, 2>>() > 0 ? 1 : 0; // CHECK-NEXT: 1
}
