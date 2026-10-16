// WGSL and Metal lowering of the wave intrinsics that have no direct WGSL or MSL function:
// QuadAny, QuadAll and WaveRotate on WGSL, and the WaveClustered* reductions on WGSL and Metal.
//
// - QuadAny / QuadAll combine a 0/1 vote with quadSwapX and quadSwapY.
// - WaveRotate shuffles from lane (self + delta) mod the subgroup size.
// - The clustered reductions read each lane of the cluster with a shuffle and combine the active
//   ones, in lane order.
// On WGSL, bools are passed to the shuffles as 0/1 `uint`, since WGSL takes no bool there. Metal
// rejects bool, as for its other SIMD-group reductions.

//TEST:SIMPLE(filecheck=QUAD): -target wgsl -stage fragment -entry quadMain
//TEST:SIMPLE(filecheck=ROTATE): -target wgsl -stage compute -entry rotateMain
//TEST:SIMPLE(filecheck=CLUSTER): -target wgsl -stage compute -entry clusteredMain
//TEST:SIMPLE(filecheck=MTLCLUSTER): -target metal -stage compute -entry clusteredMain -DMETAL
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry clusteredMain -DMETAL

RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<float4> floatBuffer;

float4 quadMain(float4 position: SV_Position) : SV_Target
{
    const bool inside = position.x > 8.0;
    float4 result = 0;
    if (QuadAny(inside))
        result.x = 1;
    if (QuadAll(inside))
        result.y = 1;
    return result;
}

// QUAD: enable subgroups;
// QUAD-NOT: {{QuadAny\(|QuadAll\(|quadAny|quadAll}}
// QUAD-DAG: quadSwapX
// QUAD-DAG: quadSwapY

[numthreads(32, 1, 1)]
void rotateMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint index = dispatchThreadID.x;
    uint result = WaveRotate(index, 1);
    result += WaveRotate(uint2(index, 1), 3).x;
    result += WaveRotate(index > 7, 2) ? 1u : 0u;
    result += all(WaveRotate(bool2(index > 7, true), 2)) ? 1u : 0u;
    outputBuffer[index] = result;
}

// ROTATE: enable subgroups;
// ROTATE-NOT: {{WaveRotate\(|subgroupRotate}}
// ROTATE-DAG: subgroupShuffle(value
// The bool overloads shuffle 0/1 u32 values.
// ROTATE-DAG: subgroupShuffle(select(u32(0), u32(1),

[numthreads(32, 1, 1)]
void clusteredMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint index = dispatchThreadID.x;
    uint result = WaveClusteredSum(index, 4);
    result += WaveClusteredProduct(index | 1, 2);
    result += WaveClusteredMin(index, 8);
    result += WaveClusteredMax(int(index), 8);
    result += WaveClusteredBitAnd(index, 4);
    result += WaveClusteredBitOr(uint2(index, 1), 4).x;
    result += WaveClusteredBitXor(index, 16);
#ifndef METAL
    // Metal's SIMD-group functions reject bool, as for the other wave reductions.
    result += WaveClusteredBitAnd(index > 3, 4) ? 1u : 0u;
    result += all(WaveClusteredBitOr(bool2(index > 3, false), 4)) ? 1u : 0u;
#endif
    outputBuffer[index] = result;

    floatBuffer[index] = WaveClusteredSum(float4(index, 1, 2, 3), 8) +
                         WaveClusteredMax(float4(index, 1, 2, 3), 2);
}

// CLUSTER: enable subgroups;
// CLUSTER-NOT: {{WaveClustered[A-Za-z]*\(|subgroupClustered}}
// CLUSTER-DAG: subgroupBallot
// CLUSTER-DAG: subgroupShuffle
// CLUSTER-DAG: select(u32(0), u32(1),

// MTLCLUSTER-NOT: {{WaveClustered[A-Za-z]*\(}}
// MTLCLUSTER-DAG: simd_ballot
// MTLCLUSTER-DAG: simd_shuffle

// The Metal compiler is not available everywhere; the METALLIB run only checks that compilation
// succeeds when it is.
// METALLIB: {{.*}}
