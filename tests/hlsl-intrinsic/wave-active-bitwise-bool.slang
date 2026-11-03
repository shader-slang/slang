//TEST_CATEGORY(wave, compute)
// WaveActiveBitAnd/Or/Xor on bool scalars, vectors and matrices, and WaveMaskBitAnd/Or/Xor on
// bool vectors and matrices, reduce as logical and/or/xor.

//TEST:SIMPLE(filecheck=SPV): -target spirv -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVNOT): -target spirv -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain -stage compute -profile cs_6_5
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0 -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_5 -shaderobj -render-feature hardware-device

// Every wave-op operand in this test is bool, so no OpGroupNonUniformBitwise* may appear
// after the module header. The OpEntryPoint check makes the run fail when no SPIR-V is emitted.
// SPVNOT: OpEntryPoint
// SPVNOT-NOT: OpGroupNonUniformBitwise

// SPV-DAG: OpGroupNonUniformLogicalAnd %bool %{{[^ ]+}} Reduce
// SPV-DAG: OpGroupNonUniformLogicalOr %bool %{{[^ ]+}} Reduce
// SPV-DAG: OpGroupNonUniformLogicalXor %bool %{{[^ ]+}} Reduce
// SPV-DAG: OpGroupNonUniformLogicalAnd %v4bool %{{[^ ]+}} Reduce
// SPV-DAG: OpGroupNonUniformLogicalXor %v4bool %{{[^ ]+}} PartitionedReduce

// HLSL-DAG: WaveActiveBitAnd(uint(
// HLSL-DAG: WaveActiveBitOr(uint(
// HLSL-DAG: WaveActiveBitXor(uint(
// HLSL-DAG: WaveActiveBitAnd(uint4(

// DXIL: @dx.op.waveActiveBit.i32

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
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

uint packMatrix(bool2x2 m) { return packBits(m[0]) | (packBits(m[1]) << 2); }

// Packs the And, Or and Xor reductions of `v` into hex digits 0, 1 and 2.
uint reduceAll<let N : int>(vector<bool, N> v)
{
    return packBits(WaveActiveBitAnd(v)) | (packBits(WaveActiveBitOr(v)) << 4) |
           (packBits(WaveActiveBitXor(v)) << 8);
}

[numthreads(kLaneCount, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;

    // Across the 8 lanes, the components are true in 7, 3, 0 and 8 lanes.
    bool4 v = bool4(idx > 0, idx < 3, idx > 7, true);

    bool s = v.y;
    outputBuffer[0 * kLaneCount + idx] = packBits(WaveActiveBitAnd(s)) |
                                         (packBits(WaveActiveBitOr(s)) << 4) |
                                         (packBits(WaveActiveBitXor(s)) << 8);
    outputBuffer[1 * kLaneCount + idx] = reduceAll(v.xw);
    outputBuffer[2 * kLaneCount + idx] = reduceAll(v.yzw);
    outputBuffer[3 * kLaneCount + idx] = reduceAll(v);

    WaveMask mask = WaveGetActiveMask();
    outputBuffer[4 * kLaneCount + idx] = packBits(WaveMaskBitAnd(mask, v)) |
                                         (packBits(WaveMaskBitOr(mask, v)) << 4) |
                                         (packBits(WaveMaskBitXor(mask, v)) << 8);

    // A bool2x2 holding v row by row reduces to the same bits as v.
    bool2x2 m = bool2x2(v.x, v.y, v.z, v.w);
    outputBuffer[5 * kLaneCount + idx] = packMatrix(WaveActiveBitAnd(m)) |
                                         (packMatrix(WaveActiveBitOr(m)) << 4) |
                                         (packMatrix(WaveActiveBitXor(m)) << 8);
    outputBuffer[6 * kLaneCount + idx] = packMatrix(WaveMaskBitAnd(mask, m)) |
                                         (packMatrix(WaveMaskBitOr(mask, m)) << 4) |
                                         (packMatrix(WaveMaskBitXor(mask, m)) << 8);
}

// bool (y): And=0, Or=1, Xor=1.
// CHECK: {{^}}110{{$}}
// CHECK-NEXT: {{^}}110{{$}}
// CHECK-NEXT: {{^}}110{{$}}
// CHECK-NEXT: {{^}}110{{$}}
// CHECK-NEXT: {{^}}110{{$}}
// CHECK-NEXT: {{^}}110{{$}}
// CHECK-NEXT: {{^}}110{{$}}
// CHECK-NEXT: {{^}}110{{$}}
// bool2 (x, w): And=(0,1), Or=(1,1), Xor=(1,0).
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// bool3 (y, z, w): And=(0,0,1), Or=(1,0,1), Xor=(1,0,0).
// CHECK-NEXT: {{^}}154{{$}}
// CHECK-NEXT: {{^}}154{{$}}
// CHECK-NEXT: {{^}}154{{$}}
// CHECK-NEXT: {{^}}154{{$}}
// CHECK-NEXT: {{^}}154{{$}}
// CHECK-NEXT: {{^}}154{{$}}
// CHECK-NEXT: {{^}}154{{$}}
// CHECK-NEXT: {{^}}154{{$}}
// bool4 (x, y, z, w): And=(0,0,0,1), Or=(1,1,0,1), Xor=(1,1,0,0).
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// WaveMaskBitAnd/Or/Xor over all lanes: same as bool4.
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// WaveActiveBitAnd/Or/Xor on bool2x2: same as bool4.
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// WaveMaskBitAnd/Or/Xor on bool2x2: same as bool4.
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
// CHECK-NEXT: {{^}}3B8{{$}}
