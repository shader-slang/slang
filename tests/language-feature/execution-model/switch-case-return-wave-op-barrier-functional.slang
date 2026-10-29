// Corner case: in a helper function, a `case` of a `switch` inside a loop
// returns from the function, a wave op follows the `switch`, and the kernel
// runs a group barrier after the call.
//
// Lanes 2-3 return in the first iteration; the other lanes reconverge after
// the `switch` and run `WaveActiveSum`. On CUDA, the active-mask pass computes
// the mask for the wave op with a `__ballot_sync` over every lane that entered
// the `switch`, so the lanes that return must take part in that ballot on
// their way out. If they do not, the other lanes wait in the ballot for lanes
// that are waiting in the barrier, and the kernel never finishes
// (shader-slang/slang#14016).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -validate-ir

RWStructuredBuffer<uint> outputBuffer;
groupshared uint shared[8];

// We keep the helper separate so that it receives the active mask as a
// parameter instead of being inlined into the kernel.
[noinline]
uint accumulate(uint lane)
{
    uint sum = 0;
    for (uint k = 0; k < 4; k++)
    {
        switch (lane / 2)
        {
        case 0:
            sum += 1;
            break;
        case 1:
            return 200 + sum;
        default:
            sum += 2;
            break;
        }

        // Lanes 0-1 and 4-7 are active here: 0 + 1 + 4 + 5 + 6 + 7 = 23.
        sum += WaveActiveSum(lane);
    }
    return sum;
}

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    shared[tid.x] = accumulate(tid.x);
    GroupMemoryBarrierWithGroupSync();

    // Each lane reads another lane's value, so the result depends on the barrier.
    outputBuffer[tid.x] = shared[7 - tid.x];
}

// Lanes 0-1 run four iterations of (1 + 23), lanes 4-7 four iterations of
// (2 + 23), and lanes 2-3 return 200.
// CHECK: 100
// CHECK-NEXT: 100
// CHECK-NEXT: 100
// CHECK-NEXT: 100
// CHECK-NEXT: 200
// CHECK-NEXT: 200
// CHECK-NEXT: 96
// CHECK-NEXT: 96

// The lanes that leave the region on the path with no wave op take part in
// its exit ballots (`false`), and those ballots survive pruning.
// CUDA-COUNT-4: __ballot_sync({{[^,]*}}, false)
// CUDA-NOT: __ballot_sync({{[^,]*}}, false)
// CUDA-NOT: __match_any_sync(
