//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj

// Runs QuadAny, QuadAll, WaveRotate and WaveClusteredSum and checks each result in the shader, so
// the test does not depend on the subgroup size. Each invocation writes 15 when all checks pass:
// - 1: WaveRotate reads lane (self + delta) mod the subgroup size, including across the end of
//   the subgroup;
// - 2: QuadAny/QuadAll of a vote that only one lane of each quad casts (any but not all);
// - 4: QuadAny/QuadAll of a vote that every lane casts (any and all);
// - 8: WaveClusteredSum over clusters of 4 lanes.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint lane = WaveGetLaneIndex();
    const uint laneCount = WaveGetLaneCount();
    const uint delta = 3;

    uint result = 0;

    if (WaveRotate(lane * 10, delta) == ((lane + delta) % laneCount) * 10)
        result |= 1;

    // Each vote is taken before the branch that tests it. Written as `QuadAny(v) && QuadAll(v)`,
    // the `&&` would make QuadAll conditional on QuadAny's result, and WGSL rejects a quad
    // operation in control flow that depends on a value its uniformity analysis cannot prove
    // uniform.
    const bool oneVote = (lane % 4) == 1;
    const bool anyOne = QuadAny(oneVote);
    const bool allOne = QuadAll(oneVote);
    if (anyOne && !allOne)
        result |= 2;

    const bool everyVote = lane < laneCount;
    const bool anyEvery = QuadAny(everyVote);
    const bool allEvery = QuadAll(everyVote);
    if (anyEvery && allEvery)
        result |= 4;

    // Lanes 4k..4k+3 hold 1, 2, 3, 4 (lane % 4 + 1), which sum to 10 in each cluster.
    if (WaveClusteredSum((lane % 4) + 1, 4) == 10)
        result |= 8;

    outputBuffer[dispatchThreadID.x] = result;
}

// CHECK-COUNT-64: 15
