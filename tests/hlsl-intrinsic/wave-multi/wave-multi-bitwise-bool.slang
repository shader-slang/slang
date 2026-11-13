//TEST_CATEGORY(wave, compute)
// Partitioned (WaveMulti*) bitwise reductions and prefixes on bool vectors.

//TEST:SIMPLE(filecheck=SPV): -target spirv -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVNOT): -target spirv -entry computeMain -stage compute -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0 -shaderobj

// Every wave-op operand in this test is bool, so no OpGroupNonUniformBitwise* may appear
// after the module header. The OpEntryPoint check makes the run fail when no SPIR-V is emitted.
// SPVNOT: OpEntryPoint
// SPVNOT-NOT: OpGroupNonUniformBitwise

// SPV-DAG: OpGroupNonUniformLogicalAnd %v4bool %{{.*}} PartitionedReduce
// SPV-DAG: OpGroupNonUniformLogicalOr %v4bool %{{.*}} PartitionedReduce
// SPV-DAG: OpGroupNonUniformLogicalXor %v4bool %{{.*}} PartitionedReduce
// SPV-DAG: OpGroupNonUniformLogicalAnd %v2bool %{{.*}} PartitionedExclusiveScan

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const uint kLaneCount = 8;

uint packBits(bool v) { return v ? 1u : 0u; }

uint packBits<let N : int>(vector<bool, N> v)
{
    uint bits = 0;
    for (int i = 0; i < N; i++)
        bits |= packBits(v[i]) << i;
    return bits;
}

[numthreads(kLaneCount, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;

    // Lanes 0-3 and lanes 4-7 form two partitions.
    uint4 mask = uint4(idx < 4 ? 0x0Fu : 0xF0u, 0, 0, 0);
    bool4 v = bool4(idx > 0, idx < 3, (idx & 1) != 0, idx == 5);

    // Reductions of v, one hex digit each: And, Or, Xor.
    outputBuffer[0 * kLaneCount + idx] = packBits(WaveMultiBitAnd(v, mask)) |
                                         (packBits(WaveMultiBitOr(v, mask)) << 4) |
                                         (packBits(WaveMultiBitXor(v, mask)) << 8);
    // Exclusive prefixes of v.xy, one hex digit each: And, Or, Xor.
    outputBuffer[1 * kLaneCount + idx] = packBits(WaveMultiPrefixBitAnd(v.xy, mask)) |
                                         (packBits(WaveMultiPrefixBitOr(v.xy, mask)) << 4) |
                                         (packBits(WaveMultiPrefixBitXor(v.xy, mask)) << 8);
    // WaveMaskPrefixBit* forwards to WaveMultiPrefixBit*, so it gives the same prefixes.
    outputBuffer[2 * kLaneCount + idx] = packBits(WaveMaskPrefixBitAnd(mask.x, v.xy)) |
                                         (packBits(WaveMaskPrefixBitOr(mask.x, v.xy)) << 4) |
                                         (packBits(WaveMaskPrefixBitXor(mask.x, v.xy)) << 8);
    // The matrix WaveMaskPrefixBitXor overload, with v stored row by row; one bit per element.
    bool2x2 m = bool2x2(v.x, v.y, v.z, v.w);
    bool2x2 r = WaveMaskPrefixBitXor(mask.x, m);
    outputBuffer[3 * kLaneCount + idx] = packBits(r[0]) | (packBits(r[1]) << 2);
}

// Reductions (lanes 0-3, then lanes 4-7)
// CHECK: {{^}}370{{$}}
// CHECK-NEXT: {{^}}370{{$}}
// CHECK-NEXT: {{^}}370{{$}}
// CHECK-NEXT: {{^}}370{{$}}
// CHECK-NEXT: {{^}}8D1{{$}}
// CHECK-NEXT: {{^}}8D1{{$}}
// CHECK-NEXT: {{^}}8D1{{$}}
// CHECK-NEXT: {{^}}8D1{{$}}
// Exclusive prefixes of v.xy
// CHECK-NEXT: {{^}}3{{$}}
// CHECK-NEXT: {{^}}222{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}232{{$}}
// CHECK-NEXT: {{^}}3{{$}}
// CHECK-NEXT: {{^}}111{{$}}
// CHECK-NEXT: {{^}}11{{$}}
// CHECK-NEXT: {{^}}111{{$}}
// WaveMaskPrefixBit* of v.xy
// CHECK-NEXT: {{^}}3{{$}}
// CHECK-NEXT: {{^}}222{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}232{{$}}
// CHECK-NEXT: {{^}}3{{$}}
// CHECK-NEXT: {{^}}111{{$}}
// CHECK-NEXT: {{^}}11{{$}}
// CHECK-NEXT: {{^}}111{{$}}
// WaveMaskPrefixBitXor of the bool2x2
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}C{{$}}
// CHECK-NEXT: {{^}}D{{$}}
