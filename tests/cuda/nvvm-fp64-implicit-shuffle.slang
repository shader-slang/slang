//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Distinct finite high/low halves, both zero signs, and quiet/signaling NaN payloads must
// survive transport unchanged. Comparing integer bits also detects accidental FP32 conversion.
bool checkBits(double2x2 value, uint source)
{
    bool ok = bit_cast<uint64_t>(value[0][0]) ==
        ((uint64_t(0x40000000u + source) << 32) | uint64_t(0x12345678u + source));
    ok &= bit_cast<uint64_t>(value[0][1]) == (uint64_t(source & 1) << 63);
    ok &= bit_cast<uint64_t>(value[1][0]) ==
        ((uint64_t(0x7FF80000u + source) << 32) | uint64_t(0xCAFE1234u + source));
    ok &= bit_cast<uint64_t>(value[1][1]) ==
        ((uint64_t(0xFFF00000u) << 32) | uint64_t(0x54321u + source));
    return ok;
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    double2x2 value = double2x2(
        bit_cast<double>((uint64_t(0x40000000u + lane) << 32) | uint64_t(0x12345678u + lane)),
        bit_cast<double>(uint64_t(lane & 1) << 63),
        bit_cast<double>((uint64_t(0x7FF80000u + lane) << 32) | uint64_t(0xCAFE1234u + lane)),
        bit_cast<double>((uint64_t(0xFFF00000u) << 32) | uint64_t(0x54321u + lane)));
    // A whole warp follows straight-line code to this first indexed shuffle, before any
    // divergence. The reverse-lane oracle is derived from the source lane's integer payloads.
    double2x2 reversed = WaveReadLaneAt(value, int(31 - lane));
    bool ok = checkBits(reversed, 31 - lane);
    // Once control flow diverges, hardware cohorts can be smaller than the source branch.
    // Every caller is always in its own snapshot; self reads remain defined for any schedule.
    if (lane < 16)
        ok &= checkBits(WaveReadLaneAt(value, int(lane)), lane);
    if ((lane & 3) == 1)
        ok &= checkBits(WaveReadLaneAt(value, int(lane)), lane);
    if (lane == 31)
        ok &= checkBits(WaveReadLaneAt(value, int(lane)), lane);
    outputBuffer[lane] = ok ? 6000 + lane : 0;
}

// CHECK: 6000
// CHECK: 6001
// CHECK: 6002
// CHECK: 6003
// CHECK: 6004
// CHECK: 6005
// CHECK: 6006
// CHECK: 6007
// CHECK: 6008
// CHECK: 6009
// CHECK: 6010
// CHECK: 6011
// CHECK: 6012
// CHECK: 6013
// CHECK: 6014
// CHECK: 6015
// CHECK: 6016
// CHECK: 6017
// CHECK: 6018
// CHECK: 6019
// CHECK: 6020
// CHECK: 6021
// CHECK: 6022
// CHECK: 6023
// CHECK: 6024
// CHECK: 6025
// CHECK: 6026
// CHECK: 6027
// CHECK: 6028
// CHECK: 6029
// CHECK: 6030
// CHECK: 6031
