// Corner case: a wave op in only one arm of an `if`, followed by a group barrier.
//
// Lanes 0-2 run `WaveActiveSum` and lanes 3-7 take the other arm. On CUDA, the
// active-mask pass reconverges the arms with exit ballots (`__ballot_sync`),
// and each lane waits in its ballot for all of the lanes that entered the `if`.
// Either every lane leaving the `if` executes one, or none does. If only lanes
// 0-2 executed one, they would wait for lanes 3-7, which are waiting in the
// barrier, and the kernel would never finish (shader-slang/slang#14016).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<uint> outputBuffer;
groupshared uint shared[8];

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value;
    if (tid.x < 3)
        value = WaveActiveSum(tid.x + 1);
    else
        value = 100 + tid.x;

    shared[tid.x] = value;
    GroupMemoryBarrierWithGroupSync();

    // Each lane reads another lane's value, so the result depends on the barrier.
    outputBuffer[tid.x] = shared[7 - tid.x];
}

// Lanes 0-2 compute 1 + 2 + 3 = 6; lane i >= 3 stores 100 + i.
// CHECK: 107
// CHECK-NEXT: 106
// CHECK-NEXT: 105
// CHECK-NEXT: 104
// CHECK-NEXT: 103
// CHECK-NEXT: 6
// CHECK-NEXT: 6
// CHECK-NEXT: 6
