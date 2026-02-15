// wave-fallthrough-gh6441.slang
//
// Reproduction of the original issue from GitHub #6441:
// https://github.com/shader-slang/slang/issues/6441
//
// This tests wave operations with non-trivial fallthrough, which is
// the primary motivating case for switch reconvergence lowering.

// GPU backends that support reconvergence lowering (SPIR-V, Metal, DX12)
// CUDA has reconvergence issues but the lowering doesn't work for it yet.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj

// Expected behavior with reconvergence lowering:
// Thread 0 (case 0): WaveActiveSum(1)=1 (only thread 0 in stage 0), then +40 = 41 (0x29)
// Threads 1,2,3 (default): WaveActiveSum(10)=40 (all 4 threads in stage 1) = 40 (0x28)
//
// Note: Our two-switch transformation provides reconvergence at the switch level,
// but stage guards still create divergence within the fallthrough group.
// Each stage's wave op only sees threads active in that stage.

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
