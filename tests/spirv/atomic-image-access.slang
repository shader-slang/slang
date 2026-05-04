// atomic-image-access.slang
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -emit-spirv-directly

// The executable test is disabled because it somehow doesn't work. Might be a gfx issue.
//TEST_DISABLED(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -output-using-type

// Test direct SPIR-V emit on image atomics.

//TEST_INPUT:set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> resultBuffer;

// TEST_INPUT: set tex = RWTexture2D(format=R32Uint, size=4, content=zero, mipMaps = 1)
[vk::image_format("r32ui")]
RWTexture2D<uint> tex;

// CHECK-DAG: %[[UINT:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// CHECK-DAG: %[[IMAGE_PTR:[A-Za-z0-9_]+]] = OpTypePointer Image %[[UINT]]
// CHECK-DAG: %[[ONE:[A-Za-z0-9_]+]] = OpConstant %[[UINT]] 1
// CHECK-DAG: %[[TWO:[A-Za-z0-9_]+]] = OpConstant %[[UINT]] 2

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: %[[ADD_PTR_0:[A-Za-z0-9_]+]] = OpImageTexelPointer %[[IMAGE_PTR]]
    // CHECK: OpAtomicIAdd %[[UINT]] %[[ADD_PTR_0]] {{.*}} {{.*}} %[[ONE]]
    InterlockedAdd(tex[uint2(0, 0)], 1);

    uint oldVal;
    // CHECK: %[[ADD_PTR_1:[A-Za-z0-9_]+]] = OpImageTexelPointer %[[IMAGE_PTR]]
    // CHECK: OpAtomicIAdd %[[UINT]] %[[ADD_PTR_1]] {{.*}} {{.*}} %[[ONE]]
    InterlockedAdd(tex[uint2(1, 0)], 1, oldVal);

    uint compareOldVal;
    // CHECK: %[[COMPARE_PTR:[A-Za-z0-9_]+]] = OpImageTexelPointer %[[IMAGE_PTR]]
    // CHECK: OpAtomicCompareExchange %[[UINT]] %[[COMPARE_PTR]] {{.*}} {{.*}} {{.*}} %[[TWO]] %[[ONE]]
    InterlockedCompareExchange(tex[uint2(0, 0)], 1, 2, compareOldVal);

    resultBuffer[0] = oldVal;
    resultBuffer[1] = compareOldVal;
}
