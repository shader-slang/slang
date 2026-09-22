//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX:-cuda -compute -capability cuda_sm_8_0 -Xslang -O3
//TEST(nvvm):COMPARE_COMPUTE_EX:-cuda -compute -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(nvvm):COMPARE_COMPUTE_EX:-cuda -compute -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint lane = dispatchThreadID.x;
    uint failures = 0;
    failures |= uint(WavePrefixCountBits(true) != lane);
    failures |= uint(WavePrefixCountBits(false) != 0) << 1;
    failures |= uint(WavePrefixCountBits((lane & 1) == 0) != (lane + 1) / 2) << 2;

    // Every named odd lane participates, including lane 31. Even lanes do not call the ballot.
    if ((lane & 1) != 0)
        failures |= uint(WaveMaskPrefixCountBits(0xAAAAAAAAu, true) != lane / 2) << 3;

    // Public calls must use the synthesized active mask after divergence, not a full warp mask.
    if ((lane & 3) != 1)
        failures |= uint(WavePrefixCountBits(true) != lane - (lane + 2) / 4) << 4;

    outputBuffer[lane] = failures;
}
