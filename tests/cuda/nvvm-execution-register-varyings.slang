//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -compute-dispatch 2,3,2 -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -compute-dispatch 2,3,2 -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -compute-dispatch 2,3,2 -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Exercises both producers of execution-register reads: core module calls and varying-parameter
// legalization. The independent host contract is 144 invocations, no mismatch, and sum(0..143).
// The existing native execution-register test independently checks every core-read tuple at a
// different nontrivial three-dimensional launch shape.
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(3, 2, 2)]
void computeMain(
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupID : SV_GroupID,
    uint3 threadID : SV_GroupThreadID,
    uint groupIndex : SV_GroupIndex)
{
    uint3 expectedDispatchID = groupID * uint3(3, 2, 2) + threadID;
    bool matches = all(cudaThreadIdx() == threadID) && all(cudaBlockIdx() == groupID) &&
        cudaBlockDim().x == 3 && cudaBlockDim().y == 2 && cudaBlockDim().z == 2 &&
        cudaGridDim().x == 2 && cudaGridDim().y == 3 && cudaGridDim().z == 2 &&
        all(dispatchID == expectedDispatchID) &&
        groupIndex == threadID.x + 3 * threadID.y + 6 * threadID.z;
    InterlockedAdd(outputBuffer[0], 1);
    InterlockedAdd(outputBuffer[1], matches ? 0 : 1);
    InterlockedAdd(outputBuffer[2], dispatchID.x + 6 * dispatchID.y + 36 * dispatchID.z);
}

// CHECK: 144
// CHECK-NEXT: 0
// CHECK-NEXT: 10296
