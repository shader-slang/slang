//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer 

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test signed int versions
    outputBuffer[0] = firstbitlow(1);       // Should be 0
    outputBuffer[1] = firstbitlow(-1);      // All bits set, should be 0
    outputBuffer[2] = firstbitlow(0);       // Should be -1
    outputBuffer[3] = firstbitlow(8);       // Should be 3
    outputBuffer[4] = firstbitlow(-8);      // Two's complement: should find first set bit
    
    // Test with vectors to ensure they work too
    int2 vec = int2(2, 4);
    int2 result = firstbitlow(vec);
    outputBuffer[5] = result.x;             // Should be 1
    outputBuffer[6] = result.y;             // Should be 2
    
    // Test larger values
    outputBuffer[7] = firstbitlow(0x10000000); // Should be 28
    outputBuffer[8] = firstbitlow(0x80000000); // Should be 31 (MSB set)
    
    // Test mixed patterns  
    outputBuffer[9] = firstbitlow(0x12345678);  // Complex pattern
    outputBuffer[10] = firstbitlow(6);          // 110 binary, should be 1
    outputBuffer[11] = firstbitlow(12);         // 1100 binary, should be 2
    outputBuffer[12] = firstbitlow(0x40000000); // Should be 30
}

//CHECK: 0
//CHECK: 0
//CHECK: -1
//CHECK: 3
//CHECK: 3
//CHECK: 1
//CHECK: 2
//CHECK: 28
//CHECK: 31
//CHECK: 3
//CHECK: 1
//CHECK: 2
//CHECK: 30