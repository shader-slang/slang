// Corner case: lanes `break` out of a loop before a wave op, and a group
// barrier follows the loop.
//
// Lanes 4-7 break in the first iteration; lanes 0-3 stay and run
// `WaveActiveSum` four times. On CUDA, the lanes that stay compute the mask for
// the wave op with a `__ballot_sync` over every lane that entered the
// iteration, so the lanes that break must take part in that ballot on their
// way out. If they do not, lanes 0-3 wait in the ballot for lanes 4-7, which
// are waiting in the barrier, and the kernel never finishes
// (shader-slang/slang#14016).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -validate-ir

RWStructuredBuffer<uint> outputBuffer;
groupshared uint shared[8];

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint sum = 0;
    for (uint k = 0; k < 4; k++)
    {
        if (tid.x < 4)
            sum++;
        else
            break;

        // Only lanes 0-3 are active here: 0 + 1 + 2 + 3 = 6.
        sum += WaveActiveSum(tid.x);
    }

    shared[tid.x] = sum;
    GroupMemoryBarrierWithGroupSync();

    // Each lane reads another lane's value, so the result depends on the barrier.
    outputBuffer[tid.x] = shared[7 - tid.x];
}

// Lanes 0-3 run four iterations of (1 + 6); lanes 4-7 break with 0.
// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 28
// CHECK-NEXT: 28
// CHECK-NEXT: 28
// CHECK-NEXT: 28

// The lanes that leave the region on the path with no wave op take part in
// its exit ballots (`false`), and those ballots survive pruning.
// CUDA-COUNT-4: __ballot_sync({{[^,]*}}, false)
// CUDA-NOT: __ballot_sync({{[^,]*}}, false)
