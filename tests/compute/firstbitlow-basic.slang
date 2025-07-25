//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test various firstbitlow patterns
    outputBuffer[0] = firstbitlow(1u);      // Should be 0
    outputBuffer[1] = firstbitlow(2u);      // Should be 1  
    outputBuffer[2] = firstbitlow(4u);      // Should be 2
    outputBuffer[3] = firstbitlow(8u);      // Should be 3
    outputBuffer[4] = firstbitlow(0u);      // Should be 0xFFFFFFFF (-1)
    outputBuffer[5] = firstbitlow(6u);      // 110 binary, should be 1
    outputBuffer[6] = firstbitlow(12u);     // 1100 binary, should be 2
    outputBuffer[7] = firstbitlow(0xFFFFFFFFu); // All bits set, should be 0
    outputBuffer[8] = firstbitlow(0x80000000u); // MSB set, should be 31
}

//CHECK: 0
//CHECK: 1
//CHECK: 2
//CHECK: 3
//CHECK: 4294967295
//CHECK: 1
//CHECK: 2
//CHECK: 0
//CHECK: 31