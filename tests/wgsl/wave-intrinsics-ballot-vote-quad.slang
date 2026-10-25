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
//TEST:SIMPLE(filecheck=QUADMATSPV): -target spirv-asm -entry quadReadLaneAtMatrixMain -stage compute

// WGSL lowering of WaveActiveCountBits, WavePrefixCountBits, WaveActiveAllEqual and
// QuadReadLaneAt through WGSL subgroup and quad built-ins (#9972).

// The bundled Tint compiles each entry point's WGSL to SPIR-V.
// TINT: OpCapability GroupNonUniform

RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<float4> floatBuffer;

// All four words of the ballot are counted.
// COUNT: enable subgroups;
// COUNT: fn WaveActiveCountBits_{{[0-9]+}}( [[V:value_[0-9]+]] : bool
// COUNT: var [[B:ballot_[0-9]+]] : vec4<u32> = subgroupBallot([[V]]);
// COUNT: return countOneBits([[B]].x) + countOneBits([[B]].y) + countOneBits([[B]].z) + countOneBits([[B]].w);
[shader("compute")]
[numthreads(64, 1, 1)]
void countBitsMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = WaveActiveCountBits(outputBuffer[tid.x] > 3);
}

// The ballot is masked with WaveGetLaneLtMask (the lanes below the invocation's own, so the count
// is exclusive), and all four words are counted.
// PREFIX: enable subgroups;
// PREFIX: fn WavePrefixCountBits_{{[0-9]+}}( [[V:value_[0-9]+]] : bool
// PREFIX: = subgroupBallot([[V]]);
// PREFIX: >> (u32(5))
// PREFIX: = ((u32(1) << ((({{.*}} & (u32(31))))))) - u32(1);
// PREFIX: var [[BELOW:below_[0-9]+]] : vec4<u32> =
// PREFIX: return countOneBits([[BELOW]].x) + countOneBits([[BELOW]].y) + countOneBits([[BELOW]].z) + countOneBits([[BELOW]].w);
// PREFIX: @builtin(subgroup_invocation_id)
[shader("compute")]
[numthreads(64, 1, 1)]
void prefixCountBitsMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = WavePrefixCountBits(outputBuffer[tid.x] > 3);
}

// `subgroupBroadcastFirst` takes no bool, so bools are compared as 0/1.
// EQUAL: enable subgroups;
// EQUAL: fn WaveActiveAllEqual_{{[0-9]+}}( [[BV:value_[0-9]+]] : vec2<bool>) -> bool
// EQUAL-NEXT: {
// EQUAL-NEXT: var [[UV:asUint_[0-9]+]] : vec2<u32> = (select(vec2<u32>(0u), vec2<u32>(1u), ([[BV]])));
// EQUAL-NEXT: = subgroupBroadcastFirst([[UV]]);
// EQUAL: fn WaveActiveAllEqual_{{[0-9]+}}( [[B:value_[0-9]+]] : bool) -> bool
// EQUAL-NEXT: {
// EQUAL-NEXT: var [[U:asUint_[0-9]+]] : u32 = (select(u32(0u), u32(1u), ([[B]])));
// EQUAL-NEXT: = subgroupBroadcastFirst([[U]]);
// EQUAL: subgroupBroadcastFirst(value
// EQUAL: subgroupAll(
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
// QUAD: fn QuadReadLaneAt_{{[0-9]+}}( [[BV:sourceValue_[0-9]+]] : vec2<bool>,  [[LV:quadLaneID_[0-9]+]] : u32) -> vec2<bool>
// QUAD: = (select(vec2<u32>(0u), vec2<u32>(1u), ([[BV]])));
// QUAD-NEXT: = QuadReadLaneAt_{{[0-9]+}}({{.*}}, [[LV]]);
// QUAD-NEXT: = (vec2<bool>((
// QUAD: fn QuadReadLaneAt_{{[0-9]+}}( [[B:sourceValue_[0-9]+]] : bool,  [[L:quadLaneID_[0-9]+]] : u32) -> bool
// QUAD: = (select(u32(0u), u32(1u), ([[B]])));
// QUAD-NEXT: = QuadReadLaneAt_{{[0-9]+}}({{.*}}, [[L]]);
// QUAD-NEXT: = (bool((
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

// A matrix is read row by row through the vector overload (`MATRIX_MAP_BINARY`), on WGSL and the
// other targets. Every row reads the same lane.
// QUADMATSPV: OpGroupNonUniformQuadBroadcast %v2float
// QUADMAT: fn [[VEC:QuadReadLaneAt_[0-9]+]]( sourceValue_{{[0-9]+}} : vec2<f32>,  quadLaneID_{{[0-9]+}} : u32) -> vec2<f32>
// QUADMAT: quadBroadcast(
// QUADMAT: fn QuadReadLaneAt_{{[0-9]+}}( sourceValue_{{[0-9]+}} : mat2x2<f32>,  [[LANE:quadLaneID_[0-9]+]] : u32) -> mat2x2<f32>
// QUADMAT: [[VEC]](sourceValue_{{[0-9]+}}[i_{{[0-9]+}}], [[LANE]])
[shader("compute")]
[numthreads(64, 1, 1)]
void quadReadLaneAtMatrixMain(uint3 tid: SV_DispatchThreadID)
{
    const float2x2 m = float2x2(floatBuffer[tid.x]);
    const float2x2 r = QuadReadLaneAt(m, outputBuffer[tid.x]);
    floatBuffer[tid.x] = float4(r[0], r[1]);
}
