// Test 3D thread group linearization with [numthreads(2,2,2)].
// Each thread writes its SV_GroupIndex to outputBuffer[groupIndex].
// Verifies linearization formula: groupIndex = x + y*Nx + z*Nx*Ny
// where Nx=2, Ny=2, giving groupIndex = x + y*2 + z*4.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 2, 2)]
void computeMain(uint groupIndex : SV_GroupIndex)
{
    outputBuffer[groupIndex] = int(groupIndex);
}

// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 5
// CHECK: 6
// CHECK: 7
