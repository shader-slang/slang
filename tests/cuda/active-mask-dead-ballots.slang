// The CUDA active-mask pass must not keep ballots whose results no wave op
// uses (shader-slang/slang#14016).
//
// The inner loop has no wave op, but it can reach the `WaveActiveSum` of the
// next outer iteration through the outer back edge. That mask comes from the
// exit ballots of the outer iteration, so the inner loop and the
// `WaveIsFirstLane()` branch need no ballot. The kernel keeps five: the initial
// mask, the `true` and `false` exit ballots of the outer loop condition, and
// the `true` and `false` exit ballots of the outer iteration.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -O3 -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<uint> outputBuffer;
groupshared uint partial[4];

[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint acc = 0;
    for (uint pass = 0; pass < 4; pass++)
    {
        uint total = WaveActiveSum(tid.x + pass);
        if (WaveIsFirstLane())
            partial[pass] = total;
        GroupMemoryBarrierWithGroupSync();

        for (uint i = tid.x; i < 20; i += 8)
        {
            if ((i & 1) != 0)
                acc += i + partial[pass];
            else
                acc ^= i << pass;
        }
        GroupMemoryBarrierWithGroupSync();
    }
    outputBuffer[tid.x] = acc;
}

// CUDA-LABEL: __global__ void computeMain(
// CUDA-COUNT-4: __ballot_sync(
// CUDA-NOT: __ballot_sync(
// CUDA: __syncthreads();
// CUDA-NOT: __ballot_sync(
// CUDA: __syncthreads();
// CUDA: __ballot_sync(
// CUDA-NOT: __ballot_sync(

// CHECK: 136
// CHECK-NEXT: 588
// CHECK-NEXT: 150
// CHECK-NEXT: 612
// CHECK-NEXT: 120
// CHECK-NEXT: 392
// CHECK-NEXT: 120
// CHECK-NEXT: 408
