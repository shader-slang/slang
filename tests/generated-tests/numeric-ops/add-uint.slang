//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Test for type: uint
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint a = 10;
    uint b = 5;
    uint result = a + b;
    outputBuffer[0] = result;
}

// CHECK: 15
