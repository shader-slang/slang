// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Runtime inputs prevent constant folding from hiding narrow arithmetic and its consumers.
//TEST_INPUT: ubuffer(data=[32767 32768 65535 1 256 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;
//TEST_INPUT: ubuffer(data=[99 99 99 99 99 99 99 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
int16_t negateNarrow(int16_t value)
{
    return -value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int16_t maximum = int16_t(inputWords[0]);
    int16_t minimum = int16_t(inputWords[1]);
    uint16_t allBits = uint16_t(inputWords[2]);
    uint16_t one = uint16_t(inputWords[3]);
    uint16_t factor = uint16_t(inputWords[4]);
    uint16_t zero = uint16_t(inputWords[5]);
    int16_t signedOverflow = maximum + int16_t(one);
    int16_t negated = negateNarrow(minimum);
    int16_t magnitude = abs(minimum);
    uint16_t unsignedOverflow = allBits + one;
    uint16_t unsignedUnderflow = zero - one;
    uint16_t product = factor * factor;

    uint errors = 0;
    errors |= signedOverflow != int16_t(-32768) ? 1u : 0u;
    errors |= negated != int16_t(-32768) ? 2u : 0u;
    errors |= magnitude != int16_t(-32768) ? 4u : 0u;
    errors |= unsignedOverflow != uint16_t(0) ? 8u : 0u;
    errors |= unsignedUnderflow != uint16_t(65535) ? 16u : 0u;
    errors |= product != uint16_t(0) ? 32u : 0u;
    errors |= !(magnitude < int16_t(0)) || unsignedUnderflow <= uint16_t(32768) ? 64u : 0u;
    errors |= int(magnitude) != -32768 || uint(unsignedUnderflow) != 65535 ? 128u : 0u;
    errors |= float(magnitude) != -32768.0 || float(unsignedUnderflow) != 65535.0 ? 256u : 0u;
    outputBuffer[0] = errors;

    uint consumers = 0;
    consumers |= (magnitude >> 1) != int16_t(-16384) ? 1u : 0u;
    consumers |= (unsignedUnderflow >> 1) != uint16_t(32767) ? 2u : 0u;
    consumers |= magnitude / int16_t(3) != int16_t(-10922) ? 4u : 0u;
    consumers |= magnitude % int16_t(3) != int16_t(-2) ? 8u : 0u;
    consumers |= unsignedUnderflow / uint16_t(3) != uint16_t(21845) ? 16u : 0u;
    consumers |= unsignedUnderflow % uint16_t(3) != uint16_t(0) ? 32u : 0u;
    consumers |= uint(uint16_t(magnitude)) != 32768 || int(int16_t(unsignedUnderflow)) != -1 ? 64u : 0u;
    consumers |= countbits(unsignedUnderflow) != 16 || firstbithigh(unsignedUnderflow) != 15 ||
        firstbitlow(unsignedUnderflow) != 0 ? 128u : 0u;
    // Shift counts retain their source width independently of the shifted value.
    consumers |= (int(inputWords[0]) >> unsignedOverflow) != 32767 ? 256u : 0u;
    consumers |= (magnitude >> inputWords[3]) != int16_t(-16384) ? 512u : 0u;
    outputBuffer[1] = consumers;

    vector<int16_t, 2> pair = vector<int16_t, 2>(maximum, minimum);
    pair += int16_t(one);
    outputBuffer[1] |= pair[unsignedOverflow] != int16_t(-32768) ? 1024u : 0u;
    outputBuffer[2] = uint(all(pair == vector<int16_t, 2>(-32768, -32767)));
    outputBuffer[3] = uint(bit_cast<uint16_t>(magnitude));
    outputBuffer[4] = uint(bit_cast<uint16_t>(signedOverflow));
    outputBuffer[5] = uint(unsignedUnderflow);
    outputBuffer[6] = uint(product);
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 32768{{$}}
// CHECK-NEXT: 32768{{$}}
// CHECK-NEXT: 65535{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3735928559{{$}}
