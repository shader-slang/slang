// wave-fallthrough-gh6441.slang
//
// Reproduction of the original issue from GitHub #6441:
// https://github.com/shader-slang/slang/issues/6441
//
// This tests wave operations with non-trivial fallthrough, which is
// the primary motivating case for switch-to-if lowering.

// CPU doesn't support wave ops; need GPU backend
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj

// CURRENT BEHAVIOR (before switch-to-if lowering):
// Thread 0 (case 0): WaveActiveSum(1)=1 (only thread 0 active), then +40 = 41 (0x29)
// Threads 1,2,3 (default): WaveActiveSum(10)=40 (all threads active) = 40 (0x28)
//
// This reflects that each wave op currently sees only the threads active
// in that particular case, not all threads that could reach this code path.

// CHECK: 29
// CHECK-NEXT: 28
// CHECK-NEXT: 28
// CHECK-NEXT: 28

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=value
RWStructuredBuffer<int> value;

[numthreads(4, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    uint sum = 0;

    // Each thread reads its own value to determine switch path
    // This creates divergent control flow within the wave
    switch (value[threadID.x])
    {
    case 0:
        // Wave operation in a fallthrough case
        sum += WaveActiveSum(1);
        // Intentional fallthrough!

    default:
        // Wave operation after fallthrough
        sum += WaveActiveSum(10);
        break;
    }

    value[threadID.x] = int(sum);
}

// EXPECTED behavior after switch-to-if lowering (FUTURE):
//
// With correct reconvergence, the switch-to-if transformation should
// ensure all threads reconverge before each wave op in fallthrough cases.
//
// Thread 0: WaveActiveSum(1)=4 (all threads could reach case 0/default), +40 = 44 (0x2C)
// Threads 1,2,3: WaveActiveSum(10)=40 = 40 (0x28)
//
// Once the lowering is implemented, update the CHECK lines above to:
//   2C, 28, 28, 28 (instead of current 29, 28, 28, 28)
