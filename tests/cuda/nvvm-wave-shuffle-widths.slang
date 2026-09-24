//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Full-wave and half-wave shuffles preserve exact payload bits and use the same source lane
// for both words of 64-bit values. Only the named low 16 lanes enter the half-wave shuffle.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

__generic<T : __BuiltinArithmeticType>
bool checkMasked(uint mask, uint lane, uint source, T base)
{
    return WaveMaskReadLaneAt(mask, base + T(lane), int(source)) == base + T(source);
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint source = 31 - lane;
    uint64_t wide = (uint64_t(0xF0000000u + lane) << 32) | uint64_t(0xABC00000u + lane * 7);
    uint64_t expectedWide = (uint64_t(0xF0000000u + source) << 32) | uint64_t(0xABC00000u + source * 7);
    bool ok = WaveMaskReadLaneAt(0xFFFFFFFFu, wide, int(source)) == expectedWide;
    ok &= WaveMaskReadLaneAt(0xFFFFFFFFu, int64_t(wide), int(source)) == int64_t(expectedWide);
    ok &= checkMasked<uint8_t>(0xFFFFFFFFu, lane, source, uint8_t(192));
    ok &= checkMasked<int8_t>(0xFFFFFFFFu, lane, source, int8_t(-64));
    ok &= checkMasked<uint16_t>(0xFFFFFFFFu, lane, source, uint16_t(60000));
    ok &= checkMasked<int16_t>(0xFFFFFFFFu, lane, source, int16_t(-30000));
    ok &= WaveMaskReadLaneAt(0xFFFFFFFFu, (lane & 1) != 0, int(source)) == ((source & 1) != 0);
    // Even lanes carry negative zero; odd lanes carry distinct quiet NaN payloads.
    uint floatBits = (lane & 1) == 0 ? 0x80000000u : 0x7FC00000u + lane;
    uint expectedFloatBits = (source & 1) == 0 ? 0x80000000u : 0x7FC00000u + source;
    ok &= asuint(WaveMaskReadLaneAt(0xFFFFFFFFu, asfloat(floatBits), int(source))) == expectedFloatBits;
    uint16_t halfBits = uint16_t((lane & 1) == 0 ? 0x8000u : 0x7E00u + lane);
    uint16_t expectedHalfBits = uint16_t((source & 1) == 0 ? 0x8000u : 0x7E00u + source);
    ok &= bit_cast<uint16_t>(WaveMaskReadLaneAt(0xFFFFFFFFu, bit_cast<half>(halfBits), int(source))) == expectedHalfBits;
    uint64_t doubleBits = (lane & 1) == 0 ? uint64_t(1) << 63 : (uint64_t(0x7FF80000u) << 32) | lane;
    uint64_t expectedDoubleBits = (source & 1) == 0 ? uint64_t(1) << 63 : (uint64_t(0x7FF80000u) << 32) | source;
    ok &= bit_cast<uint64_t>(WaveMaskReadLaneAt(0xFFFFFFFFu, bit_cast<double>(doubleBits), int(source))) == expectedDoubleBits;
    if (lane < 16)
    {
        uint subSource = 15 - lane;
        uint64_t subExpected = (uint64_t(0xF0000000u + subSource) << 32) | uint64_t(0xABC00000u + subSource * 7);
        ok &= WaveMaskReadLaneAt(0xFFFFu, wide, int(subSource)) == subExpected;
        ok &= checkMasked<int16_t>(0xFFFFu, lane, subSource, int16_t(-30000));
        ok &= WaveMaskReadLaneAt(0xFFFFu, (lane & 1) != 0, int(subSource)) == ((subSource & 1) != 0);
    }
    outputBuffer[lane] = ok ? 2000 + lane : 0;
}

// CHECK: 2000
// CHECK: 2001
// CHECK: 2002
// CHECK: 2003
// CHECK: 2004
// CHECK: 2005
// CHECK: 2006
// CHECK: 2007
// CHECK: 2008
// CHECK: 2009
// CHECK: 2010
// CHECK: 2011
// CHECK: 2012
// CHECK: 2013
// CHECK: 2014
// CHECK: 2015
// CHECK: 2016
// CHECK: 2017
// CHECK: 2018
// CHECK: 2019
// CHECK: 2020
// CHECK: 2021
// CHECK: 2022
// CHECK: 2023
// CHECK: 2024
// CHECK: 2025
// CHECK: 2026
// CHECK: 2027
// CHECK: 2028
// CHECK: 2029
// CHECK: 2030
// CHECK: 2031
