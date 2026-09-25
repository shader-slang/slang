//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Local mutable references transport raw BF16 bits without numeric conversion.
//TEST_INPUT:ubuffer(data=[0 32768 1 32769 127 32895 128 32896 16256 49024 32639 65407 32640 65408 32641 65535], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
vector<BFloat16, 2> replace2(inout vector<BFloat16, 2> destination, vector<BFloat16, 2> value)
{
    let previous = destination;
    destination = value;
    return previous;
}

[noinline]
void initialize2(out vector<BFloat16, 2> destination, vector<BFloat16, 2> value)
{
    destination = value;
}

[noinline]
vector<BFloat16, 3> replace3(inout vector<BFloat16, 3> destination, vector<BFloat16, 3> value)
{
    let previous = destination;
    destination = value;
    return previous;
}

[noinline]
void initialize3(out vector<BFloat16, 3> destination, vector<BFloat16, 3> value)
{
    destination = value;
}

[noinline]
vector<BFloat16, 4> replace4(inout vector<BFloat16, 4> destination, vector<BFloat16, 4> value)
{
    let previous = destination;
    destination = value;
    return previous;
}

[noinline]
void initialize4(out vector<BFloat16, 4> destination, vector<BFloat16, 4> value)
{
    destination = value;
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
    vector<BFloat16, 2> local2 = value2;
    let previous2 = replace2(local2, reverse2);
    vector<BFloat16, 2> copy2;
    initialize2(copy2, previous2);
    outputBuffer[i * 27 + 0] = uint(bit_cast<uint16_t>(previous2.x));
    outputBuffer[i * 27 + 1] = uint(bit_cast<uint16_t>(previous2.y));
    outputBuffer[i * 27 + 2] = uint(bit_cast<uint16_t>(local2.x));
    outputBuffer[i * 27 + 3] = uint(bit_cast<uint16_t>(local2.y));
    outputBuffer[i * 27 + 4] = uint(bit_cast<uint16_t>(copy2.x));
    outputBuffer[i * 27 + 5] = uint(bit_cast<uint16_t>(copy2.y));
    let value3 = vector<BFloat16, 3>(v0, v1, v2);
    let reverse3 = vector<BFloat16, 3>(v2, v1, v0);
    vector<BFloat16, 3> local3 = value3;
    let previous3 = replace3(local3, reverse3);
    vector<BFloat16, 3> copy3;
    initialize3(copy3, previous3);
    outputBuffer[i * 27 + 6] = uint(bit_cast<uint16_t>(previous3.x));
    outputBuffer[i * 27 + 7] = uint(bit_cast<uint16_t>(previous3.y));
    outputBuffer[i * 27 + 8] = uint(bit_cast<uint16_t>(previous3.z));
    outputBuffer[i * 27 + 9] = uint(bit_cast<uint16_t>(local3.x));
    outputBuffer[i * 27 + 10] = uint(bit_cast<uint16_t>(local3.y));
    outputBuffer[i * 27 + 11] = uint(bit_cast<uint16_t>(local3.z));
    outputBuffer[i * 27 + 12] = uint(bit_cast<uint16_t>(copy3.x));
    outputBuffer[i * 27 + 13] = uint(bit_cast<uint16_t>(copy3.y));
    outputBuffer[i * 27 + 14] = uint(bit_cast<uint16_t>(copy3.z));
    let value4 = vector<BFloat16, 4>(v0, v1, v2, v3);
    let reverse4 = vector<BFloat16, 4>(v3, v2, v1, v0);
    vector<BFloat16, 4> local4 = value4;
    let previous4 = replace4(local4, reverse4);
    vector<BFloat16, 4> copy4;
    initialize4(copy4, previous4);
    outputBuffer[i * 27 + 15] = uint(bit_cast<uint16_t>(previous4.x));
    outputBuffer[i * 27 + 16] = uint(bit_cast<uint16_t>(previous4.y));
    outputBuffer[i * 27 + 17] = uint(bit_cast<uint16_t>(previous4.z));
    outputBuffer[i * 27 + 18] = uint(bit_cast<uint16_t>(previous4.w));
    outputBuffer[i * 27 + 19] = uint(bit_cast<uint16_t>(local4.x));
    outputBuffer[i * 27 + 20] = uint(bit_cast<uint16_t>(local4.y));
    outputBuffer[i * 27 + 21] = uint(bit_cast<uint16_t>(local4.z));
    outputBuffer[i * 27 + 22] = uint(bit_cast<uint16_t>(local4.w));
    outputBuffer[i * 27 + 23] = uint(bit_cast<uint16_t>(copy4.x));
    outputBuffer[i * 27 + 24] = uint(bit_cast<uint16_t>(copy4.y));
    outputBuffer[i * 27 + 25] = uint(bit_cast<uint16_t>(copy4.z));
    outputBuffer[i * 27 + 26] = uint(bit_cast<uint16_t>(copy4.w));
}

//CHECK: type: uint32_t{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
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
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
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
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
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
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
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
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
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
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
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
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
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
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
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
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
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
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
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
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 32639{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
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
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 32895{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 65407{{$}}
//CHECK-NEXT: 0{{$}}
//CHECK-NEXT: 32895{{$}}
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
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 128{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32640{{$}}
//CHECK-NEXT: 32768{{$}}
//CHECK-NEXT: 128{{$}}
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
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 32896{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 65408{{$}}
//CHECK-NEXT: 1{{$}}
//CHECK-NEXT: 32896{{$}}
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
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 16256{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32641{{$}}
//CHECK-NEXT: 32769{{$}}
//CHECK-NEXT: 16256{{$}}
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
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 49024{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 65535{{$}}
//CHECK-NEXT: 127{{$}}
//CHECK-NEXT: 49024{{$}}
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
