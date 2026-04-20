// Test [numthreads(2,3,1)] dispatch with 6 threads.
// Each thread writes tid.x*10+tid.y to outputBuffer[groupIndex].
// Verifies correct thread ID assignment and group index linearization.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 3, 1)]
void computeMain(uint3 tid : SV_GroupThreadID, uint groupIndex : SV_GroupIndex)
{
    outputBuffer[groupIndex] = int(tid.x) * 10 + int(tid.y);
}

// CHECK: 0
// CHECK: 10
// CHECK: 1
// CHECK: 11
// CHECK: 2
// CHECK: 12
