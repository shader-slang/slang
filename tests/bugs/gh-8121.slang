//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj

//TEST_INPUT:ubuffer(data=[1 1 1 1], stride=4),name=inputBuffer
RWStructuredBuffer<int[2]> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void math(int data[2])
{
    // Tests that the following IR pattern of `getElement(...,load(getElementPtr(...)))` does not optimize incorrectly:
    // Expected IR: store(outputBuffer[0], getElement(Array<int,2>(5,6), load(getElementPtr(data,0))))
    outputBuffer[0] = Array<int,2>(5,6)[data[0]];
}
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(int3 id: SV_DispatchThreadID)
{
    math(inputBuffer[0]);
}

//CHECK: 6
//CHECK-NEXT: 0
