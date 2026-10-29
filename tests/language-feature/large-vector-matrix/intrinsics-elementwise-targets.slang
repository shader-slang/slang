// Every target compiles intrinsics with a native vector or matrix implementation for vectors and
// matrices larger than 4, element by element, instead of reporting E38208. Metal-only intrinsics
// are compiled for Metal, 16-bit casts for the targets that support them (Metal has only the
// signed vector casts), and each `clip` overload in a fragment shader of its own.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -DHALF_CASTS
//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry computeMain -stage compute -DHALF_CASTS
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -DHALF_CASTS
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -DHALF_CASTS
//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute -DHALF_CASTS
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute -DMETAL_ONLY
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute -DSIGNED_HALF_CASTS
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLIP): -target hlsl -entry clipVector -stage fragment
//TEST:SIMPLE(filecheck=CLIP): -target spirv-asm -entry clipVector -stage fragment
//TEST:SIMPLE(filecheck=CLIP): -target hlsl -entry clipMatrix -stage fragment
//TEST:SIMPLE(filecheck=CLIP): -target spirv-asm -entry clipMatrix -stage fragment

// CHECK-NOT: {{error\[E[0-9]+\]}}
// CHECK: outputBuffer
// CLIP-NOT: {{error\[E[0-9]+\]}}
// CLIP: {{discard|OpKill|OpTerminateInvocation|OpDemoteToHelperInvocation}}

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<uint> outputBits;

typealias Vec8 = vector<float, 8>;
typealias Mat6 = matrix<float, 6, 6>;

[numthreads(1, 1, 1)]
void computeMain()
{
    Vec8 a = Vec8(outputBuffer[0]);
    Vec8 b = Vec8(outputBuffer[1]);
    Vec8 c = Vec8(outputBuffer[2]);
    Mat6 m = Mat6(outputBuffer[3]);
    float r = 0.0;

    r += ldexp(a, b)[1] + ldexp(a, vector<int, 8>(int(outputBuffer[4])))[2];
    r += radians(a)[3] + radians(m)[5][5];
    r += faceforward(a, b, c)[4] + reflect(a, b)[5] + refract(a, b, 0.5)[6];
    Vec8 s;
    Vec8 co;
    sincos(a, s, co);
    Mat6 ms;
    Mat6 mco;
    sincos(m, ms, mco);
    r += s[7] + co[0] + ms[5][5] + mco[5][4];
    r += frac(matrix<float, 12, 12>(outputBuffer[5]))[11][3];
    r += fabs(a)[1] + copysign(a, b)[2];
    r += cospi(a)[3] + sinpi(a)[4] + tanpi(a)[5] + divide(a, b)[6] + exp10(a)[7] + powr(a, b)[0];
    r += (all(m > 0.0) ? 1.0 : 0.0) + (any(m > 1.0) ? 1.0 : 0.0);
#ifdef METAL_ONLY
    r += fdim(a, b)[1] + max3(a, b, c)[2] + min3(a, b, c)[3] + median3(a, b, c)[4];
#endif
    outputBuffer[6] = r;

#ifdef HALF_CASTS
    vector<half, 8> h = vector<half, 8>(half(outputBuffer[7]));
    vector<int16_t, 8> i16 = vector<int16_t, 8>(int16_t(outputBits[0]));
    vector<uint16_t, 8> u16 = vector<uint16_t, 8>(uint16_t(outputBits[1]));
    matrix<half, 6, 6> mh = matrix<half, 6, 6>(h[0]);
    matrix<int16_t, 6, 6> mi16 = matrix<int16_t, 6, 6>(i16[0]);
    matrix<uint16_t, 6, 6> mu16 = matrix<uint16_t, 6, 6>(u16[0]);
    uint bits = 0;
    bits += uint(asint16(h)[1]) + uint(asuint16(h)[2]);
    bits += uint(asfloat16(i16)[3]) + uint(asfloat16(u16)[4]);
    bits += uint(asint16(mh)[5][5]) + uint(asuint16(mh)[5][4]);
    bits += uint(asfloat16(mi16)[5][3]) + uint(asfloat16(mu16)[5][2]);
    outputBits[2] = bits;
#endif

#ifdef SIGNED_HALF_CASTS
    // The 4-wide casts check that the large-size path does not hide the native Metal cast.
    vector<half, 8> h8 = vector<half, 8>(half(outputBuffer[7]));
    vector<int16_t, 8> i8 = vector<int16_t, 8>(int16_t(outputBits[0]));
    vector<half, 4> h4 = vector<half, 4>(half(outputBuffer[8]));
    vector<int16_t, 4> i4 = vector<int16_t, 4>(int16_t(outputBits[1]));
    uint signedBits = uint(asint16(h8)[7]) + uint(asfloat16(i8)[6]);
    signedBits += uint(asint16(h4)[3]) + uint(asfloat16(i4)[2]);
    outputBits[3] = signedBits;
#endif
}

[shader("fragment")]
float4 clipVector(float4 pos : SV_Position) : SV_Target
{
    clip(vector<float, 8>(pos.x - 1.0));
    return pos;
}

[shader("fragment")]
float4 clipMatrix(float4 pos : SV_Position) : SV_Target
{
    clip(matrix<float, 6, 6>(pos.y - 1.0));
    return pos;
}
