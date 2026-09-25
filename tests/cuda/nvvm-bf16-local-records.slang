//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Direct fields in local BF16 records retain their CUDA storage layout and exact lane bits.
//TEST_INPUT:ubuffer(data=[0 32768 1 32769 127 32895 128 32896 16256 49024 32639 65407 32640 65408 32641 65535], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Record2
{
    uint16_t prefix;
    vector<BFloat16, 2> value;
    uint16_t suffix;
};

[noinline]
void initialize2(out Record2 record, vector<BFloat16, 2> value, uint guard)
{
    record.prefix = uint16_t(0x5100 + guard);
    record.value = value;
    record.suffix = uint16_t(0xa200 + guard);
}

[noinline]
vector<BFloat16, 2> replace2(inout Record2 record, vector<BFloat16, 2> value)
{
    let previous = record.value;
    record.value = value;
    return previous;
}

[noinline]
vector<BFloat16, 2> read2(inout Record2 record)
{
    return record.value;
}

[noinline]
uint guards2(inout Record2 record)
{
    return uint(record.prefix) | (uint(record.suffix) << 16);
}

struct Record3
{
    uint16_t prefix;
    vector<BFloat16, 3> value;
    uint16_t suffix;
};

[noinline]
void initialize3(out Record3 record, vector<BFloat16, 3> value, uint guard)
{
    record.prefix = uint16_t(0x5100 + guard);
    record.value = value;
    record.suffix = uint16_t(0xa200 + guard);
}

[noinline]
vector<BFloat16, 3> replace3(inout Record3 record, vector<BFloat16, 3> value)
{
    let previous = record.value;
    record.value = value;
    return previous;
}

[noinline]
vector<BFloat16, 3> read3(inout Record3 record)
{
    return record.value;
}

[noinline]
uint guards3(inout Record3 record)
{
    return uint(record.prefix) | (uint(record.suffix) << 16);
}

struct Record4
{
    uint16_t prefix;
    vector<BFloat16, 4> value;
    uint16_t suffix;
};

[noinline]
void initialize4(out Record4 record, vector<BFloat16, 4> value, uint guard)
{
    record.prefix = uint16_t(0x5100 + guard);
    record.value = value;
    record.suffix = uint16_t(0xa200 + guard);
}

[noinline]
vector<BFloat16, 4> replace4(inout Record4 record, vector<BFloat16, 4> value)
{
    let previous = record.value;
    record.value = value;
    return previous;
}

[noinline]
vector<BFloat16, 4> read4(inout Record4 record)
{
    return record.value;
}

[noinline]
uint guards4(inout Record4 record)
{
    return uint(record.prefix) | (uint(record.suffix) << 16);
}

struct MixedRecord
{
    uint8_t prefix;
    BFloat16 scalar;
    uint middle;
    uint64_t wide;
    vector<BFloat16, 3> value;
    uint16_t suffix;
};

[noinline]
void initializeMixed(out MixedRecord record, BFloat16 scalar, vector<BFloat16, 3> value)
{
    record.prefix = uint8_t(0x5a);
    record.scalar = scalar;
    record.middle = 0x12345678;
    record.wide = (uint64_t(0xfedcba98u) << 32) | uint64_t(0x76543210u);
    record.value = value;
    record.suffix = uint16_t(0xcafe);
}

[noinline]
void replaceMixed(inout MixedRecord record, BFloat16 scalar, vector<BFloat16, 3> value)
{
    record.scalar = scalar;
    record.value = value;
}

[numthreads(16, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    BFloat16 v0 = bit_cast<BFloat16>(uint16_t(inputs[(i + 0) % 16]));
    BFloat16 v1 = bit_cast<BFloat16>(uint16_t(inputs[(i + 5) % 16]));
    BFloat16 v2 = bit_cast<BFloat16>(uint16_t(inputs[(i + 10) % 16]));
    BFloat16 v3 = bit_cast<BFloat16>(uint16_t(inputs[(i + 15) % 16]));
    let value2 = vector<BFloat16, 2>(v0, v1);
    let reverse2 = vector<BFloat16, 2>(v1, v0);
    Record2 local2;
    initialize2(local2, value2, i);
    let previous2 = replace2(local2, reverse2);
    let loaded2 = read2(local2);
    Record2 copied2;
    initialize2(copied2, previous2, i + 32);
    let copiedValue2 = read2(copied2);
    outputBuffer[i * 42 + 0] = uint(bit_cast<uint16_t>(previous2[0]));
    outputBuffer[i * 42 + 1] = uint(bit_cast<uint16_t>(previous2[1]));
    outputBuffer[i * 42 + 2] = uint(bit_cast<uint16_t>(loaded2[0]));
    outputBuffer[i * 42 + 3] = uint(bit_cast<uint16_t>(loaded2[1]));
    outputBuffer[i * 42 + 4] = uint(bit_cast<uint16_t>(copiedValue2[0]));
    outputBuffer[i * 42 + 5] = uint(bit_cast<uint16_t>(copiedValue2[1]));
    outputBuffer[i * 42 + 6] = guards2(local2);
    outputBuffer[i * 42 + 7] = guards2(copied2);
    let value3 = vector<BFloat16, 3>(v0, v1, v2);
    let reverse3 = vector<BFloat16, 3>(v2, v1, v0);
    Record3 local3;
    initialize3(local3, value3, i);
    let previous3 = replace3(local3, reverse3);
    let loaded3 = read3(local3);
    Record3 copied3;
    initialize3(copied3, previous3, i + 32);
    let copiedValue3 = read3(copied3);
    outputBuffer[i * 42 + 8] = uint(bit_cast<uint16_t>(previous3[0]));
    outputBuffer[i * 42 + 9] = uint(bit_cast<uint16_t>(previous3[1]));
    outputBuffer[i * 42 + 10] = uint(bit_cast<uint16_t>(previous3[2]));
    outputBuffer[i * 42 + 11] = uint(bit_cast<uint16_t>(loaded3[0]));
    outputBuffer[i * 42 + 12] = uint(bit_cast<uint16_t>(loaded3[1]));
    outputBuffer[i * 42 + 13] = uint(bit_cast<uint16_t>(loaded3[2]));
    outputBuffer[i * 42 + 14] = uint(bit_cast<uint16_t>(copiedValue3[0]));
    outputBuffer[i * 42 + 15] = uint(bit_cast<uint16_t>(copiedValue3[1]));
    outputBuffer[i * 42 + 16] = uint(bit_cast<uint16_t>(copiedValue3[2]));
    outputBuffer[i * 42 + 17] = guards3(local3);
    outputBuffer[i * 42 + 18] = guards3(copied3);
    let value4 = vector<BFloat16, 4>(v0, v1, v2, v3);
    let reverse4 = vector<BFloat16, 4>(v3, v2, v1, v0);
    Record4 local4;
    initialize4(local4, value4, i);
    let previous4 = replace4(local4, reverse4);
    let loaded4 = read4(local4);
    Record4 copied4;
    initialize4(copied4, previous4, i + 32);
    let copiedValue4 = read4(copied4);
    outputBuffer[i * 42 + 19] = uint(bit_cast<uint16_t>(previous4[0]));
    outputBuffer[i * 42 + 20] = uint(bit_cast<uint16_t>(previous4[1]));
    outputBuffer[i * 42 + 21] = uint(bit_cast<uint16_t>(previous4[2]));
    outputBuffer[i * 42 + 22] = uint(bit_cast<uint16_t>(previous4[3]));
    outputBuffer[i * 42 + 23] = uint(bit_cast<uint16_t>(loaded4[0]));
    outputBuffer[i * 42 + 24] = uint(bit_cast<uint16_t>(loaded4[1]));
    outputBuffer[i * 42 + 25] = uint(bit_cast<uint16_t>(loaded4[2]));
    outputBuffer[i * 42 + 26] = uint(bit_cast<uint16_t>(loaded4[3]));
    outputBuffer[i * 42 + 27] = uint(bit_cast<uint16_t>(copiedValue4[0]));
    outputBuffer[i * 42 + 28] = uint(bit_cast<uint16_t>(copiedValue4[1]));
    outputBuffer[i * 42 + 29] = uint(bit_cast<uint16_t>(copiedValue4[2]));
    outputBuffer[i * 42 + 30] = uint(bit_cast<uint16_t>(copiedValue4[3]));
    outputBuffer[i * 42 + 31] = guards4(local4);
    outputBuffer[i * 42 + 32] = guards4(copied4);
    MixedRecord mixed;
    initializeMixed(mixed, v0, vector<BFloat16, 3>(v0, v1, v2));
    replaceMixed(mixed, v1, vector<BFloat16, 3>(v3, v2, v0));
    outputBuffer[i * 42 + 33] = uint(mixed.prefix);
    outputBuffer[i * 42 + 34] = uint(bit_cast<uint16_t>(mixed.scalar));
    outputBuffer[i * 42 + 35] = mixed.middle;
    outputBuffer[i * 42 + 36] = uint(mixed.wide);
    outputBuffer[i * 42 + 37] = uint(mixed.wide >> 32);
    outputBuffer[i * 42 + 38] = uint(bit_cast<uint16_t>(mixed.value.x));
    outputBuffer[i * 42 + 39] = uint(bit_cast<uint16_t>(mixed.value.y));
    outputBuffer[i * 42 + 40] = uint(bit_cast<uint16_t>(mixed.value.z));
    outputBuffer[i * 42 + 41] = uint(mixed.suffix);
}

//CHECK: type: uint32_t{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 2717929728{{$}}
//CHECK-NEXT: 2720026912{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 2717929728{{$}}
//CHECK-NEXT: 2720026912{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 2717929728{{$}}
//CHECK-NEXT: 2720026912{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 2717995265{{$}}
//CHECK-NEXT: 2720092449{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 2717995265{{$}}
//CHECK-NEXT: 2720092449{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 2717995265{{$}}
//CHECK-NEXT: 2720092449{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 2718060802{{$}}
//CHECK-NEXT: 2720157986{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 2718060802{{$}}
//CHECK-NEXT: 2720157986{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 2718060802{{$}}
//CHECK-NEXT: 2720157986{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 2718126339{{$}}
//CHECK-NEXT: 2720223523{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 2718126339{{$}}
//CHECK-NEXT: 2720223523{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 2718126339{{$}}
//CHECK-NEXT: 2720223523{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 2718191876{{$}}
//CHECK-NEXT: 2720289060{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 2718191876{{$}}
//CHECK-NEXT: 2720289060{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 2718191876{{$}}
//CHECK-NEXT: 2720289060{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 2718257413{{$}}
//CHECK-NEXT: 2720354597{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 2718257413{{$}}
//CHECK-NEXT: 2720354597{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 2718257413{{$}}
//CHECK-NEXT: 2720354597{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 2718322950{{$}}
//CHECK-NEXT: 2720420134{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 2718322950{{$}}
//CHECK-NEXT: 2720420134{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 2718322950{{$}}
//CHECK-NEXT: 2720420134{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 2718388487{{$}}
//CHECK-NEXT: 2720485671{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 2718388487{{$}}
//CHECK-NEXT: 2720485671{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 2718388487{{$}}
//CHECK-NEXT: 2720485671{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 2718454024{{$}}
//CHECK-NEXT: 2720551208{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 2718454024{{$}}
//CHECK-NEXT: 2720551208{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 2718454024{{$}}
//CHECK-NEXT: 2720551208{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 2718519561{{$}}
//CHECK-NEXT: 2720616745{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 2718519561{{$}}
//CHECK-NEXT: 2720616745{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 2718519561{{$}}
//CHECK-NEXT: 2720616745{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 2718585098{{$}}
//CHECK-NEXT: 2720682282{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 2718585098{{$}}
//CHECK-NEXT: 2720682282{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 2718585098{{$}}
//CHECK-NEXT: 2720682282{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 2718650635{{$}}
//CHECK-NEXT: 2720747819{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 2718650635{{$}}
//CHECK-NEXT: 2720747819{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 2718650635{{$}}
//CHECK-NEXT: 2720747819{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 2718716172{{$}}
//CHECK-NEXT: 2720813356{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 2718716172{{$}}
//CHECK-NEXT: 2720813356{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 2718716172{{$}}
//CHECK-NEXT: 2720813356{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 2718781709{{$}}
//CHECK-NEXT: 2720878893{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 2718781709{{$}}
//CHECK-NEXT: 2720878893{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 2718781709{{$}}
//CHECK-NEXT: 2720878893{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 2718847246{{$}}
//CHECK-NEXT: 2720944430{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 2718847246{{$}}
//CHECK-NEXT: 2720944430{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 2718847246{{$}}
//CHECK-NEXT: 2720944430{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 51966{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 2718912783{{$}}
//CHECK-NEXT: 2721009967{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 2718912783{{$}}
//CHECK-NEXT: 2721009967{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 2718912783{{$}}
//CHECK-NEXT: 2721009967{{$}}
//CHECK-NEXT: 90{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 305419896{{$}}
//CHECK-NEXT: 1985229328{{$}}
//CHECK-NEXT: 4275878552{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 51966{{$}}
