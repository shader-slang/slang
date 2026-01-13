//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// CHECK: type: int32_t
// CHECK-NEXT: 2
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Create two CoopVec<int, 4> instances
    CoopVec<int, 4> vecA = CoopVec<int, 4>(1, 2, 3, 4);
    CoopVec<int, 4> vecB = CoopVec<int, 4>(2, 1, 2, 1);

    // This should now work correctly - no ambiguous overload error
    // The float overload should not be considered as a candidate
    CoopVec<int, 4> result = max(vecA, vecB);

    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}
