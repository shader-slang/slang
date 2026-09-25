//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Records contain raw BF16 bits, Float32 bits, and independent narrowing expectations.
// They include both signs of zero, subnormals, boundary ties, overflow, infinities and NaNs.
// Expected 65535 denotes NaN classification, not a universal payload promise.
//TEST_INPUT:ubuffer(data=[0 0 0 32768 2147483648 32768 1 65536 1 127 8355840 128 128 8388608 128 16256 1065385984 16256 16257 1065451520 16258 49024 3212869633 49025 32639 2139062272 32640 65408 4286578688 65408 32640 2139095040 32640 32705 2143363909 65535 32641 2139095041 65535 32769 2147516416 32768 16258 1065451519 16257 16384 1065385985 16257], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
BFloat16 roundTrip(BFloat16 value, bool chooseInput)
{
    BFloat16 result = BFloat16(3.0f);
    if (chooseInput)
        result = value;
    return result;
}

[noinline]
void replaceValue(inout BFloat16 destination, BFloat16 value)
{
    destination = value;
}

[numthreads(16, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    uint raw = inputs[3 * i];
    BFloat16 value = bit_cast<BFloat16>(uint16_t(raw));
    BFloat16 stored = BFloat16(1.0f);
    replaceValue(stored, roundTrip(value, true));
    uint narrowed = uint(bit_cast<uint16_t>(BFloat16(asfloat(inputs[3 * i + 1]))));
    uint expected = inputs[3 * i + 2];
    bool narrowOK = expected == 65535 ?
        ((narrowed & 0x7f80) == 0x7f80 && (narrowed & 0x7f) != 0) :
        narrowed == expected;
    bool chooseInput = (i & 1) != 0;
    uint selected = uint(bit_cast<uint16_t>(roundTrip(value, chooseInput)));
    bool ok = selected == (chooseInput ? raw : 0x4040) &&
        uint(bit_cast<uint16_t>(stored)) == raw &&
        asuint(float(stored)) == (raw << 16) && narrowOK &&
        uint(bit_cast<uint16_t>(roundTrip(value, false))) == 0x4040 &&
        sizeof(BFloat16) == 2;
    outputBuffer[i] = uint(ok);
}

//CHECK: type: uint
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
