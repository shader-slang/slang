//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -dx11 -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -dx12 -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cpu -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -wgsl -compute
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -mtl -compute
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute

// Slang allows the type of system value semantics to differ from their
// "canonical" type - e.g. `uint8_t tid : SV_DispatchThreadID` is technically
// valid and refers to the first element in the underlying uint3 vector.

// TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 4, 1)]
void computeMain(
    uint ind : SV_GroupIndex,
    vector<uint, 2> tid: SV_DispatchThreadID,
    int localTid : SV_GroupThreadID
){
    // CHECK: 0
    // CHECK-NEXT: 5
    // CHECK-NEXT: 8
    // CHECK-NEXT: D
    // CHECK-NEXT: 10
    // CHECK-NEXT: 15
    // CHECK-NEXT: 18
    // CHECK-NEXT: 1D
    outputBuffer[ind] = 4 * (tid.x + 2 * tid.y) + localTid;
}
