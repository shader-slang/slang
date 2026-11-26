//TEST:COMPARE_COMPUTE(targets=all):
//TEST(smoke):COMPARE_COMPUTE(targets=cpu, filecheck-buffer=CHECK):

// CHECK:      0
// CHECK-NEXT: 3F800000
// CHECK-NEXT: 40000000
// CHECK-NEXT: 40400000

// This is a basic test for Slang compute shader.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = float(dispatchThreadID.x);
}
