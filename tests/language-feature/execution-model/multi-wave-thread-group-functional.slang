// Multi-wave thread group: verifies that wave operations are scoped
// to a single wave, not the full thread group.
//
// From the docs:
//   "Control flow uniformity is considered in the following scopes:
//    - Thread-group-uniform path: all threads in the thread group are
//      on a uniform path.
//    - Wave-uniform path: all threads in the wave are on a uniform path."
//
// With [numthreads(64, 1, 1)] and a wave size in [4, 64], the thread
// group spans one or more waves. WaveActiveSum(1) should equal the
// wave size (i.e. WaveGetLaneCount()), NOT 64. This invariant must
// hold for every thread, regardless of wave size — so the assertion
// is portable across Vulkan (subgroup size typically 32 or 64) and
// CUDA (warp size 32).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);

    // Per-wave thread count from the wave-active reduction.
    uint waveCountFromSum = WaveActiveSum(1u);

    // Reference value reported by the wave intrinsic.
    uint laneCount = WaveGetLaneCount();

    // Wave operations must operate per-wave, not per-thread-group.
    // The reduction count must equal the wave's lane count, not 64.
    outputBuffer[idx] = (waveCountFromSum == laneCount) ? 1 : 0;
}

// All 64 threads should report a match — wave reduction count equals
// wave lane count. If wave operations leaked across wave boundaries
// (e.g. summed all 64 threads), this would fail on any target where
// laneCount < 64.
// CHECK-COUNT-64: 1
