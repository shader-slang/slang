//TEST_CATEGORY(wave, compute)
// Lane-read, broadcast, shuffle, quad, AllEqual and prefix-bitwise wave intrinsics on bool and
// bool vectors. On CUDA these use the prelude _wave*Multiple helpers through ElementTypeTrait.

//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain -stage compute -profile cs_6_6
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0 -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_6 -shaderobj -render-feature hardware-device

// DXIL: define void @computeMain()

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
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

void store(uint slot, uint idx, uint value) { outputBuffer[slot * kLaneCount + idx] = value; }

[numthreads(kLaneCount, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;

    // packBits(v) per lane is 2, 7, 3, 5, 1, D, 1, 5.
    bool4 v = bool4(idx > 0, idx < 3, (idx & 1) != 0, idx == 5);

    store(0, idx, packBits(WaveReadLaneAt(v, 5)));
    store(1, idx, packBits(WaveReadLaneFirst(v)));
    store(2, idx, packBits(WaveBroadcastLaneAt(v, 1)) | (packBits(WaveBroadcastLaneAt(v.xyz, 1)) << 4));
    store(3, idx, packBits(WaveShuffle(v, idx ^ 1)) | (packBits(WaveShuffle(v.xy, idx ^ 2)) << 4));

    // Quad reads of the bool4, one hex digit each: across X, across Y, across diagonal, lane 2.
    store(4, idx, packBits(QuadReadAcrossX(v)) | (packBits(QuadReadAcrossY(v)) << 4) |
                      (packBits(QuadReadAcrossDiagonal(v)) << 8) |
                      (packBits(QuadReadLaneAt(v, 2)) << 12));
    // Quad reads of a scalar bool (v.z), one bit each: across X, across Y, across diagonal, lane 2.
    store(5, idx, packBits(QuadReadAcrossX(v.z)) | (packBits(QuadReadAcrossY(v.z)) << 1) |
                      (packBits(QuadReadAcrossDiagonal(v.z)) << 2) |
                      (packBits(QuadReadLaneAt(v.z, 2)) << 3));

    // AllEqual, one bit each: varying bool4 (false), uniform bool2 (true), uniform bool3 (true).
    store(6, idx, packBits(WaveActiveAllEqual(v)) |
                      (packBits(WaveActiveAllEqual(bool2(idx < 8, idx > 8))) << 1) |
                      (packBits(WaveActiveAllEqual(bool3(true, false, idx < 8))) << 2));

    // Exclusive prefixes of v.xy, one hex digit each: And, Or, Xor.
    store(7, idx, packBits(WavePrefixBitAnd(v.xy)) | (packBits(WavePrefixBitOr(v.xy)) << 4) |
                      (packBits(WavePrefixBitXor(v.xy)) << 8));
}

// WaveReadLaneAt(v, 5)
// CHECK: {{^}}D{{$}}
// CHECK-NEXT: {{^}}D{{$}}
// CHECK-NEXT: {{^}}D{{$}}
// CHECK-NEXT: {{^}}D{{$}}
// CHECK-NEXT: {{^}}D{{$}}
// CHECK-NEXT: {{^}}D{{$}}
// CHECK-NEXT: {{^}}D{{$}}
// CHECK-NEXT: {{^}}D{{$}}
// WaveReadLaneFirst(v)
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// WaveBroadcastLaneAt(v, 1), WaveBroadcastLaneAt(v.xyz, 1)
// CHECK-NEXT: {{^}}77{{$}}
// CHECK-NEXT: {{^}}77{{$}}
// CHECK-NEXT: {{^}}77{{$}}
// CHECK-NEXT: {{^}}77{{$}}
// CHECK-NEXT: {{^}}77{{$}}
// CHECK-NEXT: {{^}}77{{$}}
// CHECK-NEXT: {{^}}77{{$}}
// CHECK-NEXT: {{^}}77{{$}}
// WaveShuffle(v, idx ^ 1), WaveShuffle(v.xy, idx ^ 2)
// CHECK-NEXT: {{^}}37{{$}}
// CHECK-NEXT: {{^}}12{{$}}
// CHECK-NEXT: {{^}}25{{$}}
// CHECK-NEXT: {{^}}33{{$}}
// CHECK-NEXT: {{^}}1D{{$}}
// CHECK-NEXT: {{^}}11{{$}}
// CHECK-NEXT: {{^}}15{{$}}
// CHECK-NEXT: {{^}}11{{$}}
// Quad reads of the bool4
// CHECK-NEXT: {{^}}3537{{$}}
// CHECK-NEXT: {{^}}3352{{$}}
// CHECK-NEXT: {{^}}3725{{$}}
// CHECK-NEXT: {{^}}3273{{$}}
// CHECK-NEXT: {{^}}151D{{$}}
// CHECK-NEXT: {{^}}1151{{$}}
// CHECK-NEXT: {{^}}1D15{{$}}
// CHECK-NEXT: {{^}}11D1{{$}}
// Quad reads of the scalar bool (v.z is true on odd lanes)
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// WaveActiveAllEqual
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// Prefix And, Or, Xor of v.xy
// CHECK-NEXT: {{^}}3{{$}}
// CHECK-NEXT: {{^}}222{{$}}
// CHECK-NEXT: {{^}}132{{$}}
// CHECK-NEXT: {{^}}232{{$}}
// CHECK-NEXT: {{^}}330{{$}}
// CHECK-NEXT: {{^}}230{{$}}
// CHECK-NEXT: {{^}}330{{$}}
// CHECK-NEXT: {{^}}230{{$}}
