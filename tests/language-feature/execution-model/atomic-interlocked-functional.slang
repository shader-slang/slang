// Atomic operations under concurrent thread access.
//
// From the docs:
//   "atomic memory accesses to the same memory location by multiple
//    threads can often be coalesced within the wave and then performed
//    by a single thread. This can significantly reduce the number of
//    atomic memory accesses, and thus, increase performance."
//
// Whether or not the implementation coalesces, the user-visible
// semantics must be that all threads' contributions are atomically
// applied in some order — the final value is the sum of all
// contributions. This pins that invariant.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;

    // Slot 0: every thread atomically adds 1. Final value = 32.
    uint old0;
    InterlockedAdd(outputBuffer[0], 1u, old0);

    // Slot 1: every thread atomically adds its index. Final value =
    // 0 + 1 + 2 + ... + 31 = 496.
    uint old1;
    InterlockedAdd(outputBuffer[1], idx, old1);

    // Slot 2: only odd threads atomically add their index. Final
    // value = 1 + 3 + 5 + ... + 31 = 256.
    if ((idx & 1u) == 1u)
    {
        uint old2;
        InterlockedAdd(outputBuffer[2], idx, old2);
    }

    // Slot 3: only the last thread (uniform predicate inside the
    // wave only when laneCount >= 32) writes. Final value = 99.
    if (idx == 31)
    {
        outputBuffer[3] = 99u;
    }
}

// CHECK: 32
// CHECK: 496
// CHECK: 256
// CHECK: 99
