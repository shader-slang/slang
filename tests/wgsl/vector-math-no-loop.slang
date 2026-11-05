//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOLOOP): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// On WGSL, the vector overloads of `rsqrt`, `fmod`, `f32tof16`, `f16tof32`, `isnan`, `isinf`
// and `isfinite` are emitted without a per-component loop, and give the same result as the
// scalar overload on each component (#13765).

// NOLOOP-NOT: loop
// NOLOOP-NOT: for{{[ \t]*}}(
// NOLOOP-NOT: while{{[ \t]*}}(

// WGSL-DAG: unpack2x16float(
// WGSL-DAG: pack2x16float(
// WGSL-DAG: bitcast<vec4<u32>>((x
// WGSL-DAG: inverseSqrt(r
// WGSL-DAG: )) % ((

// Bit patterns of the special values, read through a buffer so that nothing is folded.
//TEST_INPUT:ubuffer(data=[0x7fc00000 0xffc00000 0x7f800001 0x7fffffff 0x7f800000 0xff800000 0x00000001 0x80000001 0x7f7fffff 0x00800000 0x00000000 0x80000000], stride=4):name=specialBits
RWStructuredBuffer<uint> specialBits;

//TEST_INPUT:ubuffer(data=[1.0 4.0 0.25 16.0  4.0 -4.0 4.0 -4.0  3.0 3.0 -3.0 -3.0  5.5 -5.5 0.75 -0.0  2.0 2.0 0.5 1.0  1.0 -2.0 65504.0 0.5], stride=4):name=floats
RWStructuredBuffer<float> floats;

//TEST_INPUT:ubuffer(data=[0x3c00 0xc000 0x7bff 0x3800 0x0001 0x8001 0x7c00 0x0400], stride=4):name=halfBits
RWStructuredBuffer<uint> halfBits;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

float4 loadFloat4(int i)
{
    return float4(floats[i], floats[i + 1], floats[i + 2], floats[i + 3]);
}

float4 loadSpecial4(int i)
{
    return asfloat(uint4(specialBits[i], specialBits[i + 1], specialBits[i + 2], specialBits[i + 3]));
}

uint4 loadHalfBits4(int i)
{
    return uint4(halfBits[i], halfBits[i + 1], halfBits[i + 2], halfBits[i + 3]);
}

// The vector form must agree with the scalar form on every component.
bool sameAsScalarIsNan(float4 v)
{
    return all(isnan(v) == bool4(isnan(v.x), isnan(v.y), isnan(v.z), isnan(v.w)));
}

bool sameAsScalarIsInf(float4 v)
{
    return all(isinf(v) == bool4(isinf(v.x), isinf(v.y), isinf(v.z), isinf(v.w)));
}

bool sameAsScalarIsFinite(float4 v)
{
    return all(isfinite(v) == bool4(isfinite(v.x), isfinite(v.y), isfinite(v.z), isfinite(v.w)));
}

int classify(bool4 b)
{
    return (b.x ? 1 : 0) | (b.y ? 2 : 0) | (b.z ? 4 : 0) | (b.w ? 8 : 0);
}

bool close<let N : int>(vector<float, N> a, vector<float, N> b)
{
    return all(abs(a - b) <= 1e-6 * abs(b));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Special values, four at a time:
    //   [0] qNaN, -qNaN, sNaN, NaN with all mantissa bits set
    //   [4] +inf, -inf, smallest denormal, smallest negative denormal
    //   [8] FLT_MAX, smallest normal, +0, -0
    float4 s0 = loadSpecial4(0);
    float4 s1 = loadSpecial4(4);
    float4 s2 = loadSpecial4(8);

    // BUF: 15
    outputBuffer[0] = classify(isnan(s0));
    // BUF-NEXT: 0
    outputBuffer[1] = classify(isnan(s1)) | classify(isnan(s2));
    // BUF-NEXT: 3
    outputBuffer[2] = classify(isinf(s1));
    // BUF-NEXT: 0
    outputBuffer[3] = classify(isinf(s0)) | classify(isinf(s2));
    // BUF-NEXT: 12
    outputBuffer[4] = classify(isfinite(s1));
    // BUF-NEXT: 15
    outputBuffer[5] = classify(isfinite(s2));
    // BUF-NEXT: 0
    outputBuffer[6] = classify(isfinite(s0));
    // BUF-NEXT: 1
    outputBuffer[7] = int(
        sameAsScalarIsNan(s0) && sameAsScalarIsNan(s1) && sameAsScalarIsNan(s2) &&
        sameAsScalarIsInf(s0) && sameAsScalarIsInf(s1) && sameAsScalarIsInf(s2) &&
        sameAsScalarIsFinite(s0) && sameAsScalarIsFinite(s1) && sameAsScalarIsFinite(s2));

    // rsqrt on positive values (WGSL `inverseSqrt` is only specified for x > 0).
    float4 r = loadFloat4(0);
    // BUF-NEXT: 1
    outputBuffer[8] = int(close(rsqrt(r), float4(1.0, 0.5, 2.0, 0.25)));
    // BUF-NEXT: 1
    outputBuffer[9] = int(close(rsqrt(r.xyz), float3(rsqrt(r.x), rsqrt(r.y), rsqrt(r.z))));

    // fmod is the truncated remainder: the result has the sign of x.
    float4 x = loadFloat4(4);
    float4 y = loadFloat4(8);
    float4 x2 = loadFloat4(12);
    float4 y2 = loadFloat4(16);
    // BUF-NEXT: 1
    outputBuffer[10] = int(all(fmod(x, y) == float4(1.0, -1.0, 1.0, -1.0)));
    // BUF-NEXT: 1
    outputBuffer[11] = int(all(fmod(x2, y2) == float4(1.5, -1.5, 0.25, 0.0)));
    // BUF-NEXT: 1
    outputBuffer[12] = int(all(
        fmod(x2.xyz, y2.xyz) == float3(fmod(x2.x, y2.x), fmod(x2.y, y2.y), fmod(x2.z, y2.z))));

    // f32tof16 / f16tof32 on values that are exact in binary16.
    float4 h = loadFloat4(20);
    uint4 hb = loadHalfBits4(0);
    // BUF-NEXT: 1
    outputBuffer[13] = int(all(f32tof16(h) == hb));
    // BUF-NEXT: 1
    outputBuffer[14] = int(all(f16tof32(hb) == h));

    // Denormal, -denormal, inf and smallest normal binary16 values: vector == scalar.
    uint4 hs = loadHalfBits4(4);
    float4 hf = f16tof32(hs);
    // BUF-NEXT: 1
    outputBuffer[15] = int(
        all(hf == float4(f16tof32(hs.x), f16tof32(hs.y), f16tof32(hs.z), f16tof32(hs.w))) &&
        all(f32tof16(hf.xyw) == uint3(f32tof16(hf.x), f32tof16(hf.y), f32tof16(hf.w))) &&
        all(f32tof16(h.xy) == uint2(f32tof16(h.x), f32tof16(h.y))) &&
        all(f16tof32(hb.zw) == float2(f16tof32(hb.z), f16tof32(hb.w))));
}
