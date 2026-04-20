// Test that numthreads dimensions correctly map to thread IDs.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 3, 1)]
void computeMain(uint3 tid : SV_GroupThreadID, uint idx : SV_GroupIndex)
{
    // SV_GroupIndex should be tid.x + tid.y * 2
    outputBuffer[idx] = int(tid.x * 10 + tid.y);
}

// Thread (0,0): idx=0, value = 0*10+0 = 0
// Thread (1,0): idx=1, value = 1*10+0 = 10
// Thread (0,1): idx=2, value = 0*10+1 = 1
// Thread (1,1): idx=3, value = 1*10+1 = 11
// Thread (0,2): idx=4, value = 0*10+2 = 2
// Thread (1,2): idx=5, value = 1*10+2 = 12
// CHECK: 0
// CHECK-NEXT: 10
// CHECK-NEXT: 1
// CHECK-NEXT: 11
// CHECK-NEXT: 2
// CHECK-NEXT: 12
