//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Keep the BF16 literal alive as a helper argument so packing cannot fold it to a uint literal.
// Check both signs of 1.25, zero, minimum/maximum subnormal, minimum normal and maximum finite.
// These values are exactly representable, so expected encodings need no rounding policy.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
uint pack(BFloat16 value)
{
    return uint(bit_cast<uint16_t>(value));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = pack(BFloat16(-1.25f)); // 0xbfa0
    outputBuffer[1] = pack(BFloat16(1.25f)); // 0x3fa0
    outputBuffer[2] = pack(BFloat16(-0.0f)); // 0x8000
    outputBuffer[3] = pack(BFloat16(0.0f)); // 0x0000
    outputBuffer[4] = pack(BFloat16(-9.183549615799121e-41f)); // 0x8001
    outputBuffer[5] = pack(BFloat16(9.183549615799121e-41f)); // 0x0001
    outputBuffer[6] = pack(BFloat16(-1.1663108012064884e-38f)); // 0x807f
    outputBuffer[7] = pack(BFloat16(1.1663108012064884e-38f)); // 0x007f
    outputBuffer[8] = pack(BFloat16(-1.1754943508222875e-38f)); // 0x8080
    outputBuffer[9] = pack(BFloat16(1.1754943508222875e-38f)); // 0x0080
    outputBuffer[10] = pack(BFloat16(-3.3895313892515355e+38f)); // 0xff7f
    outputBuffer[11] = pack(BFloat16(3.3895313892515355e+38f)); // 0x7f7f
}

//CHECK: type: uint
//CHECK-NEXT: 49056
//CHECK-NEXT: 16288
//CHECK-NEXT: 32768
//CHECK-NEXT: 0
//CHECK-NEXT: 32769
//CHECK-NEXT: 1
//CHECK-NEXT: 32895
//CHECK-NEXT: 127
//CHECK-NEXT: 32896
//CHECK-NEXT: 128
//CHECK-NEXT: 65407
//CHECK-NEXT: 32639
