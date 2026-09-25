//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Dynamic low/high words include signed extrema, the high bit, both sides of 32-bit boundaries,
// and values with equal high or low halves. The oracle compares uint2 words lexicographically.
//TEST_INPUT:ubuffer(data=[0 0 1 0 2 0 4294967294 2147483647 4294967295 2147483647 0 2147483648 1 2147483648 4294967294 4294967295 4294967295 4294967295 2147483647 0 2147483648 0 4294967295 0 0 1 1 1 0 4294967295 0 2147483647 4294967295 2147483648 4294967295 1 1985229328 4275878552 2309737967 19088743], stride=4):name=inputValues
StructuredBuffer<uint> inputValues;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Reads an input payload without interpreting it as a 64-bit integer.
uint2 readWords(uint lane, uint component, uint shift)
{
    uint index = ((lane + component + shift) % 20) * 2;
    return uint2(inputValues[index], inputValues[index + 1]);
}

// Flipping the high word's sign bit orders signed payloads using unsigned word comparisons.
bool lessWords(uint2 a, uint2 b, bool signedType)
{
    uint bias = signedType ? 0x80000000u : 0;
    return (a.y ^ bias) < (b.y ^ bias) || (a.y == b.y && a.x < b.x);
}

// Checks both output words against the independent word-wise oracle.
__generic<T : __BuiltinInt64Type>
bool matchesWords(T value, uint2 expected)
{
    uint64_t raw = value.toUInt64();
    return uint(raw) == expected.x && uint(raw >> 32) == expected.y;
}

__generic<T : __BuiltinInt64Type>
bool checkMinMax(uint lane, uint mask, bool signedType, uint shift)
{
    vector<T, 4> value;
    uint2 imin[4], imax[4], emin[4], emax[4], rmin[4], rmax[4];
    uint2 lo = signedType ? uint2(0, 0x80000000u) : uint2(0, 0);
    uint2 hi = signedType ? uint2(0xFFFFFFFFu, 0x7FFFFFFFu) : uint2(0xFFFFFFFFu, 0xFFFFFFFFu);
    for (uint c = 0; c < 4; ++c)
    {
        uint2 words = readWords(lane, c, shift);
        value[c] = T(int64_t(uint64_t(words.x) | (uint64_t(words.y) << 32)));
        imin[c] = emin[c] = rmin[c] = hi;
        imax[c] = emax[c] = rmax[c] = lo;
        for (uint j = 0; j < 32; ++j)
        {
            if ((mask & (1u << j)) == 0)
                continue;
            uint2 x = readWords(j, c, shift);
            rmin[c] = lessWords(x, rmin[c], signedType) ? x : rmin[c];
            rmax[c] = lessWords(rmax[c], x, signedType) ? x : rmax[c];
            if (j <= lane)
            {
                imin[c] = lessWords(x, imin[c], signedType) ? x : imin[c];
                imax[c] = lessWords(imax[c], x, signedType) ? x : imax[c];
            }
            if (j < lane)
            {
                emin[c] = lessWords(x, emin[c], signedType) ? x : emin[c];
                emax[c] = lessWords(emax[c], x, signedType) ? x : emax[c];
            }
        }
    }
    uint4 members = uint4(mask, 0, 0, 0);
    bool ok = true;
    T resultPrefixInclusiveMin1 = WaveMultiPrefixInclusiveMin(value.x, members);
    ok &= matchesWords(resultPrefixInclusiveMin1, imin[0]);
    vector<T, 2> resultPrefixInclusiveMin2 = WaveMultiPrefixInclusiveMin(value.xy, members);
    ok &= matchesWords(resultPrefixInclusiveMin2[0], imin[0]);
    ok &= matchesWords(resultPrefixInclusiveMin2[1], imin[1]);
    vector<T, 4> resultPrefixInclusiveMin4 = WaveMultiPrefixInclusiveMin(value, members);
    ok &= matchesWords(resultPrefixInclusiveMin4[0], imin[0]);
    ok &= matchesWords(resultPrefixInclusiveMin4[1], imin[1]);
    ok &= matchesWords(resultPrefixInclusiveMin4[2], imin[2]);
    ok &= matchesWords(resultPrefixInclusiveMin4[3], imin[3]);
    T resultPrefixInclusiveMax1 = WaveMultiPrefixInclusiveMax(value.x, members);
    ok &= matchesWords(resultPrefixInclusiveMax1, imax[0]);
    vector<T, 2> resultPrefixInclusiveMax2 = WaveMultiPrefixInclusiveMax(value.xy, members);
    ok &= matchesWords(resultPrefixInclusiveMax2[0], imax[0]);
    ok &= matchesWords(resultPrefixInclusiveMax2[1], imax[1]);
    vector<T, 4> resultPrefixInclusiveMax4 = WaveMultiPrefixInclusiveMax(value, members);
    ok &= matchesWords(resultPrefixInclusiveMax4[0], imax[0]);
    ok &= matchesWords(resultPrefixInclusiveMax4[1], imax[1]);
    ok &= matchesWords(resultPrefixInclusiveMax4[2], imax[2]);
    ok &= matchesWords(resultPrefixInclusiveMax4[3], imax[3]);
    T resultPrefixExclusiveMin1 = WaveMultiPrefixExclusiveMin(value.x, members);
    ok &= matchesWords(resultPrefixExclusiveMin1, emin[0]);
    vector<T, 2> resultPrefixExclusiveMin2 = WaveMultiPrefixExclusiveMin(value.xy, members);
    ok &= matchesWords(resultPrefixExclusiveMin2[0], emin[0]);
    ok &= matchesWords(resultPrefixExclusiveMin2[1], emin[1]);
    vector<T, 4> resultPrefixExclusiveMin4 = WaveMultiPrefixExclusiveMin(value, members);
    ok &= matchesWords(resultPrefixExclusiveMin4[0], emin[0]);
    ok &= matchesWords(resultPrefixExclusiveMin4[1], emin[1]);
    ok &= matchesWords(resultPrefixExclusiveMin4[2], emin[2]);
    ok &= matchesWords(resultPrefixExclusiveMin4[3], emin[3]);
    T resultPrefixExclusiveMax1 = WaveMultiPrefixExclusiveMax(value.x, members);
    ok &= matchesWords(resultPrefixExclusiveMax1, emax[0]);
    vector<T, 2> resultPrefixExclusiveMax2 = WaveMultiPrefixExclusiveMax(value.xy, members);
    ok &= matchesWords(resultPrefixExclusiveMax2[0], emax[0]);
    ok &= matchesWords(resultPrefixExclusiveMax2[1], emax[1]);
    vector<T, 4> resultPrefixExclusiveMax4 = WaveMultiPrefixExclusiveMax(value, members);
    ok &= matchesWords(resultPrefixExclusiveMax4[0], emax[0]);
    ok &= matchesWords(resultPrefixExclusiveMax4[1], emax[1]);
    ok &= matchesWords(resultPrefixExclusiveMax4[2], emax[2]);
    ok &= matchesWords(resultPrefixExclusiveMax4[3], emax[3]);
    T resultMin1 = WaveMultiMin(value.x, members);
    ok &= matchesWords(resultMin1, rmin[0]);
    vector<T, 2> resultMin2 = WaveMultiMin(value.xy, members);
    ok &= matchesWords(resultMin2[0], rmin[0]);
    ok &= matchesWords(resultMin2[1], rmin[1]);
    vector<T, 4> resultMin4 = WaveMultiMin(value, members);
    ok &= matchesWords(resultMin4[0], rmin[0]);
    ok &= matchesWords(resultMin4[1], rmin[1]);
    ok &= matchesWords(resultMin4[2], rmin[2]);
    ok &= matchesWords(resultMin4[3], rmin[3]);
    T resultMax1 = WaveMultiMax(value.x, members);
    ok &= matchesWords(resultMax1, rmax[0]);
    vector<T, 2> resultMax2 = WaveMultiMax(value.xy, members);
    ok &= matchesWords(resultMax2[0], rmax[0]);
    ok &= matchesWords(resultMax2[1], rmax[1]);
    vector<T, 4> resultMax4 = WaveMultiMax(value, members);
    ok &= matchesWords(resultMax4[0], rmax[0]);
    ok &= matchesWords(resultMax4[1], rmax[1]);
    ok &= matchesWords(resultMax4[2], rmax[2]);
    ok &= matchesWords(resultMax4[3], rmax[3]);
    // Matrix reductions already use the same homogeneous leaf recipe as vectors.
    matrix<T, 2, 2> matrixValue = matrix<T, 2, 2>(value.x, value.y, value.z, value.w);
    matrix<T, 2, 2> matrixMin = WaveMultiMin(matrixValue, members);
    matrix<T, 2, 2> matrixMax = WaveMultiMax(matrixValue, members);
    for (uint c = 0; c < 4; ++c)
    {
        ok &= matchesWords(matrixMin[c / 2][c % 2], rmin[c]);
        ok &= matchesWords(matrixMax[c / 2][c % 2], rmax[c]);
    }
    return ok;
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    bool ok = true;
    for (uint partition = 0; partition < 5; ++partition)
    {
        uint mask = 0xFFFFFFFFu;
        if (partition == 1)
            mask = lane < 16 ? 0xFFFFu : 0xFFFF0000u;
        if (partition == 2)
            mask = lane < 15 ? 0x7FFFu : 0xFFFF8000u;
        if (partition == 3)
            mask = (lane & 1) != 0 ? 0xAAAAAAAAu : 0x55555555u;
        if (partition == 4)
            mask = 1u << lane;
        for (uint shift = 0; shift < 20; ++shift)
        {
            ok &= checkMinMax<int64_t>(lane, mask, true, shift);
            ok &= checkMinMax<uint64_t>(lane, mask, false, shift);
        }
    }
    outputBuffer[lane] = ok ? 7100 + lane : 0;
}

// CHECK: 7100
// CHECK: 7101
// CHECK: 7102
// CHECK: 7103
// CHECK: 7104
// CHECK: 7105
// CHECK: 7106
// CHECK: 7107
// CHECK: 7108
// CHECK: 7109
// CHECK: 7110
// CHECK: 7111
// CHECK: 7112
// CHECK: 7113
// CHECK: 7114
// CHECK: 7115
// CHECK: 7116
// CHECK: 7117
// CHECK: 7118
// CHECK: 7119
// CHECK: 7120
// CHECK: 7121
// CHECK: 7122
// CHECK: 7123
// CHECK: 7124
// CHECK: 7125
// CHECK: 7126
// CHECK: 7127
// CHECK: 7128
// CHECK: 7129
// CHECK: 7130
// CHECK: 7131
