//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//
// Test for public unscoped enum constants visibility across module boundaries
//
// This test verifies that unscoped enum constants are visible across module
// boundaries when the enum is declared as public.
//

import unscoped_enum_lib;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // These should all be visible due to the public unscoped enum
    TestEnum e1 = CASE_A; // Should work - unscoped access
    TestEnum e2 = TestEnum.CASE_B; // Should work - scoped access
    
    // CHECK: 0
    outputBuffer[0] = (uint)e1;
    // CHECK: 1  
    outputBuffer[1] = (uint)e2;
    // CHECK: 0
    outputBuffer[2] = (uint)CASE_A;
    // CHECK: 1
    outputBuffer[3] = (uint)CASE_B;
}