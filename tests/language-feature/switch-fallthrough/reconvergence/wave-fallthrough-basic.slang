// wave-fallthrough-basic.slang
//
// Test wave operations in a case that falls through to another case.
// This is the primary motivating case for switch-to-if lowering.
//
// With OpSwitch, the reconvergence behavior after wave ops in fallthrough
// cases is undefined. The switch-to-if lowering ensures correct behavior
// by using if-chains instead.
//
// This test uses WaveReadLaneFirst which returns the value from lane 0.
// All threads in the switch should see the same result if wave operations
// are correctly reconverged.

// GPU backends that support reconvergence lowering (SPIR-V, Metal, DX12)
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj

// All threads output 84 = 0x54
// CHECK:      54
// CHECK-NEXT: 54
// CHECK-NEXT: 54
// CHECK-NEXT: 54

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    int result = 0;

    // All threads take the same path (uniform control flow)
    // This tests that wave ops work correctly after the switch-to-if lowering
    int selector = 0;  // All threads use same selector

    switch(selector)
    {
    case 0:
        // Wave operation: get value from lane 0 (which has tid=0)
        // All threads should see the same result (42 from tid=0's contribution)
        result = WaveReadLaneFirst(tid + 42);  // = 0 + 42 = 42
        // Fall through to case 1

    case 1:
        // Multiply by 2
        result = result * 2;  // = 42 * 2 = 84
        break;

    default:
        result = -1;
        break;
    }

    // All threads should have result = 84
    // 84 = 0x54
    outputBuffer[tid] = result;
}

// Expected: All 4 threads output 84 (0x54)
