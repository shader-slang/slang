// Corner case: verify WaveActiveBallot returns correct bit pattern.
// With 4 threads, threads with even index pass `true`, odd pass `false`.
// Expected ballot: bit 0 set (thread 0), bit 2 set (thread 2) = 0x5
//
// Also verify WaveActiveCountBits matches popcount of the ballot.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    bool isEven = (idx & 1) == 0;

    // Ballot: should have bits set for threads 0 and 2
    uint4 ballot = WaveActiveBallot(isEven);

    // CountBits of the same predicate: should be 2 (threads 0 and 2)
    uint evenCount = WaveActiveCountBits(isEven);

    // Each thread writes ballot.x and the count
    outputBuffer[idx * 2 + 0] = int(ballot.x);
    outputBuffer[idx * 2 + 1] = int(evenCount);
}

// All threads see the same ballot and count
// ballot.x = 0x5 = 5 (bits 0 and 2 set)
// evenCount = 2
// CHECK: 5
// CHECK: 2
// CHECK: 5
// CHECK: 2
// CHECK: 5
// CHECK: 2
// CHECK: 5
// CHECK: 2
