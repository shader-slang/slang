// Test compute shader system values across multiple dispatch groups.
// Uses [numthreads(2,2,2)] with a 2x2x2 group dispatch, producing a
// 4x4x4 thread grid. This covers non-zero SV_GroupID values in every
// dimension and verifies:
//   SV_DispatchThreadID == SV_GroupID * numthreads + SV_GroupThreadID
//   SV_GroupIndex == x + y*Nx + z*Nx*Ny == x + y*2 + z*4

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -compute-dispatch 2,2,2
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -compute-dispatch 2,2,2
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -compute-dispatch 2,2,2
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 2, 2)]
void computeMain(
    uint3 dispatchId : SV_DispatchThreadID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uint groupIndex : SV_GroupIndex)
{
    uint3 expectedDispatchId = groupId * uint3(2, 2, 2) + groupThreadId;
    uint expectedGroupIndex = groupThreadId.x + groupThreadId.y * 2 + groupThreadId.z * 4;
    uint linearDispatchIndex = dispatchId.x + dispatchId.y * 4 + dispatchId.z * 16;

    bool idsMatch = all(dispatchId == expectedDispatchId);
    bool groupIndexMatches = groupIndex == expectedGroupIndex;
    outputBuffer[linearDispatchIndex] = (idsMatch && groupIndexMatches) ? 1 : 0;
}

// All 64 dispatched threads should validate their dispatch ID,
// group-thread ID, group ID, and group-index relationship.
// CHECK-COUNT-64: 1
