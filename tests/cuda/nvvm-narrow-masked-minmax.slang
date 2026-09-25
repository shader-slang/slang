//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Runtime values cross both sign boundaries and include wide inputs whose low bits are retained.
//TEST_INPUT:ubuffer(data=[0 1 127 128 255 32767 32768 65535 4294967168 4294934528 3735928559 4294967295], stride=4):name=inputValues
StructuredBuffer<uint> inputValues;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Decodes low bits mathematically; the expected values do not use a narrow cast or another wave.
int decode(uint value, uint bits, bool signedType)
{
    uint low = value & ((1u << bits) - 1);
    return signedType && (low & (1u << (bits - 1))) != 0 ? int(low) - int(1u << bits) : int(low);
}

__generic<T : __BuiltinArithmeticType>
bool checkMinMax(uint lane, uint mask, uint bits, bool signedType, uint shift)
{
    vector<T, 4> value;
    int4 imin, imax, emin, emax, rmin, rmax;
    int lo = signedType ? -int(1u << (bits - 1)) : 0;
    int hi = signedType ? int((1u << (bits - 1)) - 1) : int((1u << bits) - 1);
    for (uint c = 0; c < 4; ++c)
    {
        value[c] = T(inputValues[(lane + c + shift) % 12]);
        imin[c] = emin[c] = rmin[c] = hi;
        imax[c] = emax[c] = rmax[c] = lo;
        for (uint j = 0; j < 32; ++j)
        {
            if ((mask & (1u << j)) == 0)
                continue;
            int x = decode(inputValues[(j + c + shift) % 12], bits, signedType);
            rmin[c] = min(rmin[c], x);
            rmax[c] = max(rmax[c], x);
            if (j <= lane)
            {
                imin[c] = min(imin[c], x);
                imax[c] = max(imax[c], x);
            }
            if (j < lane)
            {
                emin[c] = min(emin[c], x);
                emax[c] = max(emax[c], x);
            }
        }
    }
    uint4 members = uint4(mask, 0, 0, 0);
    bool ok = true;
    T resultPrefixInclusiveMin1 = WaveMultiPrefixInclusiveMin(value.x, members);
    ok &= resultPrefixInclusiveMin1 == T(imin[0]);
    vector<T, 2> resultPrefixInclusiveMin2 = WaveMultiPrefixInclusiveMin(value.xy, members);
    ok &= resultPrefixInclusiveMin2[0] == T(imin[0]);
    ok &= resultPrefixInclusiveMin2[1] == T(imin[1]);
    vector<T, 4> resultPrefixInclusiveMin4 = WaveMultiPrefixInclusiveMin(value, members);
    ok &= resultPrefixInclusiveMin4[0] == T(imin[0]);
    ok &= resultPrefixInclusiveMin4[1] == T(imin[1]);
    ok &= resultPrefixInclusiveMin4[2] == T(imin[2]);
    ok &= resultPrefixInclusiveMin4[3] == T(imin[3]);
    T resultPrefixInclusiveMax1 = WaveMultiPrefixInclusiveMax(value.x, members);
    ok &= resultPrefixInclusiveMax1 == T(imax[0]);
    vector<T, 2> resultPrefixInclusiveMax2 = WaveMultiPrefixInclusiveMax(value.xy, members);
    ok &= resultPrefixInclusiveMax2[0] == T(imax[0]);
    ok &= resultPrefixInclusiveMax2[1] == T(imax[1]);
    vector<T, 4> resultPrefixInclusiveMax4 = WaveMultiPrefixInclusiveMax(value, members);
    ok &= resultPrefixInclusiveMax4[0] == T(imax[0]);
    ok &= resultPrefixInclusiveMax4[1] == T(imax[1]);
    ok &= resultPrefixInclusiveMax4[2] == T(imax[2]);
    ok &= resultPrefixInclusiveMax4[3] == T(imax[3]);
    T resultPrefixExclusiveMin1 = WaveMultiPrefixExclusiveMin(value.x, members);
    ok &= resultPrefixExclusiveMin1 == T(emin[0]);
    vector<T, 2> resultPrefixExclusiveMin2 = WaveMultiPrefixExclusiveMin(value.xy, members);
    ok &= resultPrefixExclusiveMin2[0] == T(emin[0]);
    ok &= resultPrefixExclusiveMin2[1] == T(emin[1]);
    vector<T, 4> resultPrefixExclusiveMin4 = WaveMultiPrefixExclusiveMin(value, members);
    ok &= resultPrefixExclusiveMin4[0] == T(emin[0]);
    ok &= resultPrefixExclusiveMin4[1] == T(emin[1]);
    ok &= resultPrefixExclusiveMin4[2] == T(emin[2]);
    ok &= resultPrefixExclusiveMin4[3] == T(emin[3]);
    T resultPrefixExclusiveMax1 = WaveMultiPrefixExclusiveMax(value.x, members);
    ok &= resultPrefixExclusiveMax1 == T(emax[0]);
    vector<T, 2> resultPrefixExclusiveMax2 = WaveMultiPrefixExclusiveMax(value.xy, members);
    ok &= resultPrefixExclusiveMax2[0] == T(emax[0]);
    ok &= resultPrefixExclusiveMax2[1] == T(emax[1]);
    vector<T, 4> resultPrefixExclusiveMax4 = WaveMultiPrefixExclusiveMax(value, members);
    ok &= resultPrefixExclusiveMax4[0] == T(emax[0]);
    ok &= resultPrefixExclusiveMax4[1] == T(emax[1]);
    ok &= resultPrefixExclusiveMax4[2] == T(emax[2]);
    ok &= resultPrefixExclusiveMax4[3] == T(emax[3]);
    T resultMin1 = WaveMultiMin(value.x, members);
    ok &= resultMin1 == T(rmin[0]);
    vector<T, 2> resultMin2 = WaveMultiMin(value.xy, members);
    ok &= resultMin2[0] == T(rmin[0]);
    ok &= resultMin2[1] == T(rmin[1]);
    vector<T, 4> resultMin4 = WaveMultiMin(value, members);
    ok &= resultMin4[0] == T(rmin[0]);
    ok &= resultMin4[1] == T(rmin[1]);
    ok &= resultMin4[2] == T(rmin[2]);
    ok &= resultMin4[3] == T(rmin[3]);
    T resultMax1 = WaveMultiMax(value.x, members);
    ok &= resultMax1 == T(rmax[0]);
    vector<T, 2> resultMax2 = WaveMultiMax(value.xy, members);
    ok &= resultMax2[0] == T(rmax[0]);
    ok &= resultMax2[1] == T(rmax[1]);
    vector<T, 4> resultMax4 = WaveMultiMax(value, members);
    ok &= resultMax4[0] == T(rmax[0]);
    ok &= resultMax4[1] == T(rmax[1]);
    ok &= resultMax4[2] == T(rmax[2]);
    ok &= resultMax4[3] == T(rmax[3]);
    // Matrix reductions already use the same homogeneous leaf recipe as vectors.
    matrix<T, 2, 2> matrixValue = matrix<T, 2, 2>(value.x, value.y, value.z, value.w);
    matrix<T, 2, 2> matrixMin = WaveMultiMin(matrixValue, members);
    matrix<T, 2, 2> matrixMax = WaveMultiMax(matrixValue, members);
    for (uint c = 0; c < 4; ++c)
    {
        ok &= matrixMin[c / 2][c % 2] == T(rmin[c]);
        ok &= matrixMax[c / 2][c % 2] == T(rmax[c]);
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
        for (uint shift = 0; shift < 12; ++shift)
        {
            ok &= checkMinMax<int8_t>(lane, mask, 8, true, shift);
            ok &= checkMinMax<uint8_t>(lane, mask, 8, false, shift);
            ok &= checkMinMax<int16_t>(lane, mask, 16, true, shift);
            ok &= checkMinMax<uint16_t>(lane, mask, 16, false, shift);
        }
    }
    outputBuffer[lane] = ok ? 6900 + lane : 0;
}

// CHECK: 6900
// CHECK: 6901
// CHECK: 6902
// CHECK: 6903
// CHECK: 6904
// CHECK: 6905
// CHECK: 6906
// CHECK: 6907
// CHECK: 6908
// CHECK: 6909
// CHECK: 6910
// CHECK: 6911
// CHECK: 6912
// CHECK: 6913
// CHECK: 6914
// CHECK: 6915
// CHECK: 6916
// CHECK: 6917
// CHECK: 6918
// CHECK: 6919
// CHECK: 6920
// CHECK: 6921
// CHECK: 6922
// CHECK: 6923
// CHECK: 6924
// CHECK: 6925
// CHECK: 6926
// CHECK: 6927
// CHECK: 6928
// CHECK: 6929
// CHECK: 6930
// CHECK: 6931
