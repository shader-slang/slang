//TEST:SIMPLE(filecheck=SUM): -target wgsl -entry sumMain -stage compute
//TEST:SIMPLE(filecheck=PRODUCT): -target wgsl -entry productMain -stage compute
//TEST:SIMPLE(filecheck=AND): -target wgsl -entry andMain -stage compute
//TEST:SIMPLE(filecheck=OR): -target wgsl -entry orMain -stage compute
//TEST:SIMPLE(filecheck=XOR): -target wgsl -entry xorMain -stage compute
//TEST:SIMPLE(filecheck=MATRIX): -target wgsl -entry matrixMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry sumMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry productMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry andMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry orMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry xorMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry matrixMain -stage compute

// One entry point per WGSL WaveMultiPrefix scan: each checks the scan's identity, its combine
// operator, and whether it is exclusive (only lanes below the invocation's) or inclusive (also
// the invocation's own lane), for a scalar, vector, `int`, `bool2` or matrix overload. The TINT
// runs compile each with the bundled Tint.

// TINT: OpGroupNonUniformShuffle

RWStructuredBuffer<uint> u;
RWStructuredBuffer<int> i;
RWStructuredBuffer<float4> f;

// Exclusive sum (uint): starts at 0, adds, lanes below only.
// SUM: fn sumMain
// SUM: result_{{[0-9]+}} = u32(0);
// SUM: subgroupShuffle(
// SUM-NOT: == _builtinWaveLaneIndex
// SUM: (lane_{{[0-9]+}} < _builtinWaveLaneIndex_{{[0-9]+}})
// SUM-NOT: == _builtinWaveLaneIndex
// SUM: result_{{[0-9]+}} = result_{{[0-9]+}} + other_{{[0-9]+}};
[shader("compute")]
[numthreads(64, 1, 1)]
void sumMain(uint3 t: SV_DispatchThreadID)
{
    u[t.x] = WaveMultiPrefixSum(u[t.x], WaveMatch(u[t.x] & 3));
}

// Inclusive product (float4): starts at 1, multiplies, includes the own lane.
// PRODUCT: result_{{[0-9]+}} = vec4<f32>(1.0f);
// PRODUCT: subgroupShuffle(
// PRODUCT: (lane_{{[0-9]+}} < _builtinWaveLaneIndex_{{[0-9]+}})
// PRODUCT: lane_{{[0-9]+}} == _builtinWaveLaneIndex_{{[0-9]+}}
// PRODUCT: result_{{[0-9]+}} = result_{{[0-9]+}} * other_{{[0-9]+}};
[shader("compute")]
[numthreads(64, 1, 1)]
void productMain(uint3 t: SV_DispatchThreadID)
{
    f[t.x] = WaveMultiPrefixInclusiveProduct(f[t.x], uint4(0xFFFFFFFF));
}

// And (uint, exclusive): starts at ~0, ands.
// AND: result_{{[0-9]+}} = u32(4294967295);
// AND-NOT: == _builtinWaveLaneIndex
// AND: result_{{[0-9]+}} = (result_{{[0-9]+}} & (other_{{[0-9]+}}));
[shader("compute")]
[numthreads(64, 1, 1)]
void andMain(uint3 t: SV_DispatchThreadID)
{
    u[t.x] = WaveMultiPrefixBitAnd(u[t.x], uint4(0xFFFF));
}

// Or (int, exclusive): bit-cast to u32 and back, starts at 0, ors.
// OR: bitcast<u32>(
// OR: result_{{[0-9]+}} = u32(0);
// OR-NOT: == _builtinWaveLaneIndex
// OR: result_{{[0-9]+}} = (result_{{[0-9]+}} | (other_{{[0-9]+}}));
// OR: bitcast<i32>(result_{{[0-9]+}})
[shader("compute")]
[numthreads(64, 1, 1)]
void orMain(uint3 t: SV_DispatchThreadID)
{
    i[t.x] = WaveMultiPrefixExclusiveBitOr(i[t.x], uint4(0xFFFF));
}

// Inclusive xor (bool2): passed as 0/1 u32, starts at 0, xors, includes the own lane.
// XOR: select(vec2<u32>(0u), vec2<u32>(1u),
// XOR: result_{{[0-9]+}} = vec2<u32>(u32(0));
// XOR-NOT: subgroupShuffle({{[^,]*}}bool
// XOR: lane_{{[0-9]+}} == _builtinWaveLaneIndex_{{[0-9]+}}
// XOR: result_{{[0-9]+}} = (result_{{[0-9]+}} ^ (other_{{[0-9]+}}));
[shader("compute")]
[numthreads(64, 1, 1)]
void xorMain(uint3 t: SV_DispatchThreadID)
{
    const bool2 b = WaveMultiPrefixInclusiveBitXor(bool2(u[t.x] > 1, u[t.x] > 2), uint4(3));
    u[t.x] = (b.x ? 1u : 0u) + (b.y ? 2u : 0u);
}

// Sum of a matrix: row by row through the vector overload.
// MATRIX: result_{{[0-9]+}} = vec2<f32>(0.0f);
// MATRIX: subgroupShuffle(value_{{[0-9]+}}[
// MATRIX: result_{{[0-9]+}} = result_{{[0-9]+}} + other_{{[0-9]+}};
[shader("compute")]
[numthreads(64, 1, 1)]
void matrixMain(uint3 t: SV_DispatchThreadID)
{
    const float2x2 r = WaveMultiPrefixSum(float2x2(f[t.x]), uint4(1));
    f[t.x] = float4(r[0], r[1]);
}
