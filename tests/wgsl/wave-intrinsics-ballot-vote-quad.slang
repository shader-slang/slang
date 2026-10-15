//TEST:SIMPLE(filecheck=COUNT): -target wgsl -entry countBitsMain -stage compute
//TEST:SIMPLE(filecheck=PREFIX): -target wgsl -entry prefixCountBitsMain -stage compute
//TEST:SIMPLE(filecheck=EQUAL): -target wgsl -entry allEqualMain -stage compute
//TEST:SIMPLE(filecheck=QUAD): -target wgsl -entry quadReadLaneAtMain -stage compute

// WGSL lowering of WaveActiveCountBits, WavePrefixCountBits, WaveActiveAllEqual and
// QuadReadLaneAt through WGSL subgroup and quad built-ins (#9972).

RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<float4> floatBuffer;

// COUNT: enable subgroups;
// COUNT: subgroupBallot(
// COUNT: countOneBits(
[shader("compute")]
[numthreads(64, 1, 1)]
void countBitsMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = WaveActiveCountBits(outputBuffer[tid.x] > 3);
}

// PREFIX: enable subgroups;
// PREFIX: subgroupBallot(
// PREFIX: countOneBits(
// PREFIX: @builtin(subgroup_invocation_id)
[shader("compute")]
[numthreads(64, 1, 1)]
void prefixCountBitsMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = WavePrefixCountBits(outputBuffer[tid.x] > 3);
}

// `subgroupBroadcastFirst` takes no bool, so bools are compared as 0/1.
// EQUAL: enable subgroups;
// EQUAL-DAG: subgroupBroadcastFirst(value
// EQUAL-DAG: subgroupBroadcastFirst(asUint
// EQUAL-DAG: subgroupAll(
// EQUAL-NOT: subgroupBroadcastFirst({{.*}}bool
[shader("compute")]
[numthreads(64, 1, 1)]
void allEqualMain(uint3 tid: SV_DispatchThreadID)
{
    const uint v = outputBuffer[tid.x];
    const float4 f = floatBuffer[tid.x];
    uint r = 0;
    r |= WaveActiveAllEqual(v) ? 1u : 0u;
    r |= WaveActiveAllEqual(f.xy) ? 2u : 0u;
    r |= WaveActiveAllEqual(v > 2) ? 4u : 0u;
    r |= WaveActiveAllEqual(f.xy > 0.5) ? 8u : 0u;
    outputBuffer[tid.x] = r;
}

// WGSL's quadBroadcast needs a const-expression lane, so all four lanes are read and selected.
// QUAD: enable subgroups;
// QUAD: quadBroadcast({{.*}}u32(0)
// QUAD: quadBroadcast({{.*}}u32(1)
// QUAD: quadBroadcast({{.*}}u32(2)
// QUAD: quadBroadcast({{.*}}u32(3)
[shader("compute")]
[numthreads(64, 1, 1)]
void quadReadLaneAtMain(uint3 tid: SV_DispatchThreadID)
{
    const uint v = outputBuffer[tid.x];
    outputBuffer[tid.x] = QuadReadLaneAt(v, v);
    floatBuffer[tid.x] = QuadReadLaneAt(floatBuffer[tid.x], 2);
}
