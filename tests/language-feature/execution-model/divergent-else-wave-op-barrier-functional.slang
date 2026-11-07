// Mirror of divergent-if-wave-op-barrier-functional.slang: only the `else`
// side of a divergent `if` runs a wave op, and a group barrier follows.
//
// On CUDA the mask for the `else` block is `entryMask & ~trueMask`, so the
// `if` condition ballot stays live only through that `bitAnd`/`bitNot`. The
// lanes that take the `then` side must still reach the barrier
// (shader-slang/slang#14016).

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -validate-ir
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
        value = 100 + tid.x;
    else
        value = WaveActiveSum(tid.x);

    shared[tid.x] = value;
    GroupMemoryBarrierWithGroupSync();

    // Each lane reads another lane's value, so the result depends on the barrier.
    outputBuffer[tid.x] = shared[7 - tid.x];
}

// The kernel keeps the initial mask and the `if` condition ballot.
// CUDA-LABEL: __global__ void computeMain(
// CUDA-COUNT-2: __ballot_sync(
// CUDA-NOT: __ballot_sync(

// Lanes 3-7 compute 3 + 4 + 5 + 6 + 7 = 25; lane i < 3 stores 100 + i.
// CHECK: 25
// CHECK-NEXT: 25
// CHECK-NEXT: 25
// CHECK-NEXT: 25
// CHECK-NEXT: 25
// CHECK-NEXT: 102
// CHECK-NEXT: 101
// CHECK-NEXT: 100
