// Value-correctness companion to wave-count-bits-shared-ballot.slang. That test asserts the SPIR-V
// *shape* (one shared OpGroupNonUniformBallot); this one proves the reroute of WavePrefixCountBits
// through WaveActiveBallot plus the same-block ballot merge still compute the correct counts on real
// hardware. WavePrefixCountBits(p) and WaveActiveCountBits(p) share a predicate in one basic block,
// which is exactly the case the merge collapses.
//
// With 4 lanes and p = "even lane", the ballot has bits 0 and 2 set (lanes 0 and 2). The exclusive
// prefix count (set bits in strictly-lower lanes) is [0, 1, 1, 2] and the active count is 2 for
// every lane. This needs a GPU, so it is left to CI (no CPU/interpreter path: those run wave size 1
// and the merge only runs in legalizeIRForSPIRV).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    bool isEven = (idx & 1) == 0;

    uint prefix = WavePrefixCountBits(isEven);
    uint active = WaveActiveCountBits(isEven);

    outputBuffer[idx * 2 + 0] = int(prefix);
    outputBuffer[idx * 2 + 1] = int(active);
}

// Interleaved (prefix, active) per lane: lane0 (0,2), lane1 (1,2), lane2 (1,2), lane3 (2,2).
// CHECK: 0
// CHECK: 2
// CHECK: 1
// CHECK: 2
// CHECK: 1
// CHECK: 2
// CHECK: 2
// CHECK: 2
