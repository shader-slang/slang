//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Each record stores four BF16 operand bits for each side, then independent dot2/3/4
// expectations. Dot starts at positive zero and rounds each product and each sum in lane order.
// Expected 65535 means NaN classification; all other expectations include the exact sign bit.
// Record 0: frozen 8.5 dot.
// Record 1: lane-ordered cancellation.
// Record 2: separate product rounding.
// Record 3: second-lane contraction counterexample.
// Record 4: both signs of zero.
// Record 5: negative zero products.
// Record 6: positive subnormals.
// Record 7: negative subnormals.
// Record 8: underflow ties.
// Record 9: normal boundary.
// Record 10: positive infinity.
// Record 11: negative infinity.
// Record 12: opposing infinities.
// Record 13: infinity times zero.
// Record 14: signaling NaN.
// Record 15: overflow.
//TEST_INPUT:ubuffer(data=[16256 16384 16448 16512 16128 16128 16256 16256 16320 16528 16648 16256 15232 49024 0 16256 16256 16256 16256 16256 0 0 16257 49024 0 0 16257 16258 0 0 0 0 0 49024 16257 0 0 16258 16257 0 0 0 0 0 32768 0 32768 0 16256 16256 16256 16256 0 0 0 0 32768 0 32768 49024 16256 49024 16256 0 0 0 1 2 3 4 16256 16256 16256 16256 3 6 10 32769 32770 32771 32772 16256 16256 16256 16256 32771 32774 32778 1 3 5 7 16128 16128 16128 16128 2 4 8 127 1 1 1 16256 16256 16256 16256 128 129 130 32640 16256 0 0 16256 16256 16256 16256 32640 32640 32640 65408 16256 0 0 16256 16256 16256 16256 65408 65408 65408 32640 65408 0 0 16256 16256 16256 16256 65535 65535 65535 32640 16256 0 0 0 16256 0 0 65535 65535 65535 32641 16256 0 0 16256 16256 16256 16256 65535 65535 65535 32639 32639 0 0 16384 16256 0 0 32640 32640 32640], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

bool matches(uint actual, uint expected)
{
    return expected == 65535 ?
        ((actual & 0x7f80) == 0x7f80 && (actual & 0x7f) != 0) : actual == expected;
}

[numthreads(16, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint base = 11 * tid.x;
    BFloat16 a0 = bit_cast<BFloat16>(uint16_t(inputs[base + 0]));
    BFloat16 a1 = bit_cast<BFloat16>(uint16_t(inputs[base + 1]));
    BFloat16 a2 = bit_cast<BFloat16>(uint16_t(inputs[base + 2]));
    BFloat16 a3 = bit_cast<BFloat16>(uint16_t(inputs[base + 3]));
    BFloat16 b0 = bit_cast<BFloat16>(uint16_t(inputs[base + 4]));
    BFloat16 b1 = bit_cast<BFloat16>(uint16_t(inputs[base + 5]));
    BFloat16 b2 = bit_cast<BFloat16>(uint16_t(inputs[base + 6]));
    BFloat16 b3 = bit_cast<BFloat16>(uint16_t(inputs[base + 7]));

    uint result2 = uint(bit_cast<uint16_t>(dot(
        vector<BFloat16, 2>(a0, a1),
        vector<BFloat16, 2>(b0, b1))));
    uint result3 = uint(bit_cast<uint16_t>(dot(
        vector<BFloat16, 3>(a0, a1, a2),
        vector<BFloat16, 3>(b0, b1, b2))));
    uint result4 = uint(bit_cast<uint16_t>(dot(
        vector<BFloat16, 4>(a0, a1, a2, a3),
        vector<BFloat16, 4>(b0, b1, b2, b3))));
    outputBuffer[tid.x] = uint(matches(result2, inputs[base + 8]) &&
        matches(result3, inputs[base + 9]) && matches(result4, inputs[base + 10]));
}

//CHECK: type: uint
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
//CHECK: 1
