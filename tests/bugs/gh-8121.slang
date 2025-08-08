//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj

//TEST_INPUT:ubuffer(data=[1 1 1 1], stride=4),name=inputBuffer
RWStructuredBuffer<int[2]> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void math(int data[2])
{
    int4 vec;
    vec[0] = 1;
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
