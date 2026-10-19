//TEST:SIMPLE(filecheck=MATCH): -target wgsl -entry matchMain -stage compute
//TEST:SIMPLE(filecheck=SCAN): -target wgsl -entry scanMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry matchMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry scanMain -stage compute

// WGSL has no partitioned subgroup operations. WaveMatch and the WaveMulti* scans read each lane
// in turn with subgroupShuffle, in a loop bounded by the subgroup size. The subgroup size reaches
// the loop as a value (not a module-scope var<private>), so the subgroup operations stay in
// uniform control flow and Tint accepts the module.

RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<float4> floatBuffer;

// The bool overload compares 0/1 u32 values, so the shuffle never takes a bool. In the uint
// overload, each lane ballots whether its value equals the shuffled value of `lane`, an
// invocation keeps the ballot of its own lane, and its own bit is ORed in.
// MATCH-NOT: var<private>
// MATCH: fn WaveMatch_{{[0-9]+}}( [[B:value_[0-9]+]] : bool,
// MATCH: var [[BU:_S[0-9]+]] : u32 = (select(u32(0u), u32(1u), ([[B]])));
// MATCH: subgroupShuffle([[BU]], i32(lane_{{[0-9]+}}))
// MATCH: fn WaveMatch_{{[0-9]+}}( [[V:value_[0-9]+]] : u32,  [[SELF:_builtinWaveLaneIndex_[0-9]+]] : u32,
// MATCH: var [[OTHER:_S[0-9]+]] : u32 = subgroupShuffle([[V]], i32([[LANE:lane_[0-9]+]]));
// MATCH-NEXT: var [[M:matching_[0-9]+]] : vec4<u32> = subgroupBallot([[V]] == [[OTHER]]);
// MATCH-NEXT: if([[LANE]] == [[SELF]])
// MATCH: (u32(1) << (bit_{{[0-9]+}}))
// MATCH: result_{{[0-9]+}} = ([[M]] | ({{_S[0-9]+}}));
// MATCH: fn matchMain({{.*}}@builtin(subgroup_size)
// MATCH-NOT: var<private>
[shader("compute")]
[numthreads(64, 1, 1)]
void matchMain(uint3 tid: SV_DispatchThreadID)
{
    const uint v = outputBuffer[tid.x];
    const float4 f = floatBuffer[tid.x];
    outputBuffer[tid.x] = WaveMatch(v).x + WaveMatch(f.xyz).y + WaveMatch(v > 3).z +
                          WaveMatch(bool2(v > 1, v > 2)).w + WaveMatch(float2x2(f)).x;
}

// Exclusive scans start from the identity (0 for sums, 1 for products, ~0 for and, 0 for
// or/xor); inclusive scans also take the invocation's own lane.
// SCAN-NOT: var<private>
// SCAN-DAG: result_{{[0-9]+}} = u32(0);
// SCAN-DAG: result_{{[0-9]+}} = vec4<f32>(1.0f);
// SCAN-DAG: u32(4294967295)
// SCAN-DAG: < [[LANE:_builtinWaveLaneIndex_[0-9]+]]
// SCAN-DAG: == [[LANE]]
// SCAN-DAG: countOneBits(
// SCAN-NOT: var<private>
[shader("compute")]
[numthreads(64, 1, 1)]
void scanMain(uint3 tid: SV_DispatchThreadID)
{
    const uint v = outputBuffer[tid.x];
    const float4 f = floatBuffer[tid.x];
    const uint4 partition = WaveMatch(v & 3);
    uint r = WaveMultiPrefixSum(v, partition);
    r += WaveMultiPrefixInclusiveSum(v, partition);
    r += WaveMultiPrefixBitAnd(v, partition);
    r += WaveMultiPrefixBitOr(v, partition);
    r += WaveMultiPrefixInclusiveBitXor(v, partition);
    r += WaveMultiPrefixBitXor(v > 2, partition) ? 1u : 0u;
    r += WaveMultiPrefixCountBits(v > 1, partition);
    outputBuffer[tid.x] = r;
    floatBuffer[tid.x] = WaveMultiPrefixProduct(f, partition);
}

// TINT: OpGroupNonUniformShuffle
