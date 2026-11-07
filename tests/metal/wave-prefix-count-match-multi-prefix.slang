// Metal lowering of the wave intrinsics that MSL has no direct SIMD-group function for:
// WavePrefixCountBits, WaveActiveAllEqual, WaveMatch, WaveMultiPrefixCountBits and the
// WaveMultiPrefix arithmetic and bitwise scans.
//
// - WavePrefixCountBits / WaveMultiPrefixCountBits count the bits of a `simd_ballot` below the
//   thread's lane (and inside the mask). The ballot is masked to the SIMD-group width, because
//   `simd_vote` leaves the bits above it undefined.
// - WaveActiveAllEqual compares each value with `simd_broadcast_first`.
// - WaveMatch and the scans read every lane with `simd_shuffle` in a loop bounded by the
//   SIMD-group width.
// Bools are passed to the SIMD-group functions as 0/1 `uint`, since MSL's permute functions take
// no bool.

//TEST:SIMPLE(filecheck=BALLOT): -target metal -stage compute -entry ballotMain
//TEST:SIMPLE(filecheck=MATCH): -target metal -stage compute -entry matchMain
//TEST:SIMPLE(filecheck=SCAN): -target metal -stage compute -entry scanMain
//TEST:SIMPLE(filecheck=NO_HLSL): -target metal -stage compute -entry ballotMain
//TEST:SIMPLE(filecheck=NO_HLSL): -target metal -stage compute -entry matchMain
//TEST:SIMPLE(filecheck=NO_HLSL): -target metal -stage compute -entry scanMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry ballotMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry matchMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry scanMain

RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<float4> floatBuffer;

[numthreads(32, 1, 1)]
void ballotMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint index = dispatchThreadID.x;
    uint result = WavePrefixCountBits(index > 3);
    result += WaveMultiPrefixCountBits(index > 5, uint4(0x0F0F0F0F, 0, 0, 0)) << 8;
    result += WaveActiveAllEqual(index / 64) ? 1u << 16 : 0u;
    result += WaveActiveAllEqual(index > 100) ? 1u << 17 : 0u;
    result += WaveActiveAllEqual(uint2(index & 1, 7)) ? 1u << 18 : 0u;
    result += WaveActiveAllEqual(bool2(index > 100, true)) ? 1u << 19 : 0u;
    result += WaveActiveAllEqual(float2x2(index & 1, 0, 1, 1)) ? 1u << 20 : 0u;
    outputBuffer[index] = result;
}

// BALLOT-DAG: simd_ballot
// BALLOT-DAG: popcount
// BALLOT-DAG: simd_broadcast_first
// BALLOT-DAG: simd_all

[numthreads(32, 1, 1)]
void matchMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint index = dispatchThreadID.x;
    uint4 result = WaveMatch(index & 3);
    result += WaveMatch(index > 7);
    result += WaveMatch(float2(index & 1, 1.0));
    result += WaveMatch(float2x2(index & 1, 0, 1, 1));
    outputBuffer[index] = result.x + result.y;
}

// MATCH-DAG: simd_shuffle
// MATCH-DAG: simd_ballot

[numthreads(32, 1, 1)]
void scanMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint index = dispatchThreadID.x;
    const uint4 partition = uint4(0x55555555, 0, 0, 0);
    uint result = WaveMultiPrefixSum(index, partition);
    result += WaveMultiPrefixProduct(index | 1, partition);
    result += WaveMultiPrefixInclusiveSum(index, partition);
    result += WaveMultiPrefixExclusiveProduct(index | 1, partition);
    result += WaveMultiPrefixBitAnd(index, partition);
    result += WaveMultiPrefixInclusiveBitOr(index, partition);
    result += WaveMultiPrefixExclusiveBitXor(int(index), partition);
    result += WaveMultiPrefixBitOr(uint16_t(index), partition);
    outputBuffer[index] = result;

    float4 value = float4(index, 1, 2, 3);
    floatBuffer[index] = WaveMultiPrefixSum(value, partition) +
                         WaveMultiPrefixInclusiveProduct(value, partition) +
                         WaveMultiPrefixSum(float2x4(value, value), partition)[1];
}

// SCAN: simd_shuffle

// No call is left to an HLSL intrinsic that MSL lacks, anywhere in the output. The emitted helpers
// are named `Name_N`, which is why the pattern ends in `\(`: it matches only an HLSL name called
// directly.
// NO_HLSL: result code = 0
// NO_HLSL-NOT: {{(WavePrefixCountBits|WaveActiveAllEqual|WaveMatch|WaveMultiPrefix[A-Za-z]*)\(}}

// The metallib runs are reported as ignored where the Metal compiler is missing.
// METALLIB: result code = 0
