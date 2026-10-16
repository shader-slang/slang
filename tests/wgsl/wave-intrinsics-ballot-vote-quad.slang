//TEST:SIMPLE(filecheck=COUNT): -target wgsl -entry countBitsMain -stage compute
//TEST:SIMPLE(filecheck=PREFIX): -target wgsl -entry prefixCountBitsMain -stage compute
//TEST:SIMPLE(filecheck=EQUAL): -target wgsl -entry allEqualMain -stage compute
//TEST:SIMPLE(filecheck=QUAD): -target wgsl -entry quadReadLaneAtMain -stage compute
//TEST:SIMPLE(filecheck=QUADMAT): -target wgsl -entry quadReadLaneAtMatrixMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry countBitsMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry prefixCountBitsMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry allEqualMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry quadReadLaneAtMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry quadReadLaneAtMatrixMain -stage compute

// WGSL lowering of WaveActiveCountBits, WavePrefixCountBits, WaveActiveAllEqual and
// QuadReadLaneAt through WGSL subgroup and quad built-ins (#9972).

// The bundled Tint compiles each entry point's WGSL to SPIR-V.
// TINT: OpCapability GroupNonUniform

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

// WGSL's quadBroadcast needs a const-expression lane, so all four lanes are read and the
// requested one (the lane index modulo 4) is selected. Bools are broadcast as 0/1.
// QUAD: enable subgroups;
// QUAD: fn QuadReadLaneAt_{{[0-9]+}}( sourceValue_{{[0-9]+}} : u32,  quadLaneID_{{[0-9]+}} : u32) -> u32
// QUAD: var [[LANE:lane_[0-9]+]] : u32 = (quadLaneID_{{[0-9]+}} & (u32(3)));
// QUAD: var [[Q0:q0_[0-9]+]] : u32 = (quadBroadcast((sourceValue_{{[0-9]+}}), (u32(0))));
// QUAD: var [[Q1:q1_[0-9]+]] : u32 = (quadBroadcast((sourceValue_{{[0-9]+}}), (u32(1))));
// QUAD: var [[Q2:q2_[0-9]+]] : u32 = (quadBroadcast((sourceValue_{{[0-9]+}}), (u32(2))));
// QUAD: var [[Q3:q3_[0-9]+]] : u32 = (quadBroadcast((sourceValue_{{[0-9]+}}), (u32(3))));
// QUAD: if([[LANE]] == u32(0))
// QUAD-NEXT: {
// QUAD-NEXT: = [[Q0]];
// QUAD: if([[LANE]] == u32(1))
// QUAD-NEXT: {
// QUAD-NEXT: = [[Q1]];
// QUAD: if([[LANE]] == u32(2))
// QUAD-NEXT: {
// QUAD-NEXT: = [[Q2]];
// QUAD: else
// QUAD-NEXT: {
// QUAD-NEXT: = [[Q3]];
// QUAD-NOT: quadBroadcast({{.*}}bool
// QUAD-DAG: = (select(u32(0u), u32(1u), (sourceValue_{{[0-9]+}})));
// QUAD-DAG: = (select(vec2<u32>(0u), vec2<u32>(1u), (sourceValue_{{[0-9]+}})));
// QUAD-DAG: = (bool((
// QUAD-DAG: = (vec2<bool>((
[shader("compute")]
[numthreads(64, 1, 1)]
void quadReadLaneAtMain(uint3 tid: SV_DispatchThreadID)
{
    const uint v = outputBuffer[tid.x];
    const bool b = QuadReadLaneAt(v > 3, v);
    const bool2 b2 = QuadReadLaneAt(bool2(v > 1, v > 2), 1);
    outputBuffer[tid.x] = QuadReadLaneAt(v, v) + (b ? 8u : 0u) + (b2.x ? 16u : 0u) + (b2.y ? 32u : 0u);
    floatBuffer[tid.x] = QuadReadLaneAt(floatBuffer[tid.x], 2);
}

// A matrix is read row by row through the vector overload.
// QUADMAT: fn [[VEC:QuadReadLaneAt_[0-9]+]]( sourceValue_{{[0-9]+}} : vec2<f32>,  quadLaneID_{{[0-9]+}} : u32) -> vec2<f32>
// QUADMAT: quadBroadcast(
// QUADMAT: fn QuadReadLaneAt_{{[0-9]+}}( sourceValue_{{[0-9]+}} : mat2x2<f32>,  quadLaneID_{{[0-9]+}} : u32) -> mat2x2<f32>
// QUADMAT: [[VEC]](sourceValue_{{[0-9]+}}[i32(0)],
// QUADMAT: [[VEC]](sourceValue_{{[0-9]+}}[i32(1)],
[shader("compute")]
[numthreads(64, 1, 1)]
void quadReadLaneAtMatrixMain(uint3 tid: SV_DispatchThreadID)
{
    const float2x2 m = float2x2(floatBuffer[tid.x]);
    const float2x2 r = QuadReadLaneAt(m, outputBuffer[tid.x]);
    floatBuffer[tid.x] = float4(r[0], r[1]);
}
