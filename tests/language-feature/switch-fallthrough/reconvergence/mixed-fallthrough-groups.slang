// mixed-fallthrough-groups.slang
//
// Test a switch with two fallthrough groups:
// - Group 1 (cases 0,1): Contains wave operations, requires reconvergence lowering
// - Group 2 (cases 2,3): Contains only safe operations (math), no reconvergence needed
//
// The pass should only transform Group 1 to the second switch, while Group 2
// can remain as regular fallthrough in the first switch.

// GPU backends that support wave ops
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj

// Expected values:
// tid=0 (case 0): WaveActiveSum(1) + WaveActiveSum(10) = 1 + 20 = 21 (0x15)
//   - Thread 0 alone contributes 1 to first sum
//   - Threads 0,1 contribute 10 each to second sum (20 total)
//   - Threads 2,3 are NOT in this reconvergence group, so they don't participate
// tid=1 (case 1): WaveActiveSum(10) = 20 (0x14)
//   - Threads 0,1 contribute 10 each (20 total)
// tid=2 (case 2): 100 + 200 = 300 (0x12C)
//   - Simple math fallthrough, no wave ops, NOT lowered
// tid=3 (case 3): 200 (0xC8)
//   - Just the second part of the safe fallthrough group

// CHECK: 15
// CHECK-NEXT: 14
// CHECK-NEXT: 12C
// CHECK-NEXT: C8

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    int result = 0;
    int selector = int(threadID.x);

    switch (selector)
    {
    // === Fallthrough Group 1: Requires reconvergence (has wave ops) ===
    case 0:
        result += WaveActiveSum(1);  // Wave op - triggers reconvergence
        // fallthrough to case 1

    case 1:
        result += WaveActiveSum(10);  // Wave op
        break;

    // === Fallthrough Group 2: Safe (only math operations) ===
    case 2:
        result += 100;  // Simple math - no reconvergence needed
        // fallthrough to case 3

    case 3:
        result += 200;  // Simple math
        break;

    default:
        result = -1;
        break;
    }

    outputBuffer[threadID.x] = result;
}
