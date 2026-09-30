// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Exact, runtime-loaded checks for the newly supported integer mad and scalar Half sincos.
// The other widths, floating mad, and aliased outputs exercise their core compositions.
//TEST_INPUT: ubuffer(data=[3 4 5 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;
//TEST_INPUT: ubuffer(data=[324508639 99 99 99 99 99 99 99 99 99 99 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = int(inputWords[0]), b = -int(inputWords[1]), c = int(inputWords[2]);
    uint errors = 0;
    errors |= mad(int8_t(a), int8_t(b), int8_t(c)) != -7 ? 1u : 0u;
    errors |= mad(int16_t(a), int16_t(b), int16_t(c)) != -7 ? 2u : 0u;
    errors |= mad(a, b, c) != -7 ? 4u : 0u;
    errors |= mad(int64_t(a), int64_t(b), int64_t(c)) != -7 ? 8u : 0u;
    errors |= mad(uint8_t(a), uint8_t(-b), uint8_t(c)) != 17 ? 16u : 0u;
    errors |= mad(uint16_t(a), uint16_t(-b), uint16_t(c)) != 17 ? 32u : 0u;
    errors |= mad(uint(a), uint(-b), uint(c)) != 17 ? 64u : 0u;
    errors |= mad(uint64_t(a), uint64_t(-b), uint64_t(c)) != 17 ? 128u : 0u;
    errors |= !all(mad(int2(a, b), int2(b, c), int2(c, a)) == int2(-7, -17)) ? 256u : 0u;
    outputBuffer[1] = errors;

    half hs, hc;
    sincos(bit_cast<half>(uint16_t(inputWords[3])), hs, hc);
    outputBuffer[2] = uint(bit_cast<uint16_t>(hs));
    outputBuffer[3] = uint(bit_cast<uint16_t>(hc));
    float fs, fc;
    sincos(asfloat(inputWords[3]), fs, fc);
    outputBuffer[4] = asuint(fs);
    outputBuffer[5] = asuint(fc);
    double ds, dc;
    sincos(asdouble(inputWords[3], inputWords[3]), ds, dc);
    outputBuffer[6] = uint(ds == 0.0 && dc == 1.0);
    float alias = asfloat(inputWords[3]);
    // Deliberate aliasing checks that the cosine assignment wins.
#pragma warning(push)
#pragma warning(disable: 30051)
    sincos(alias, alias, alias);
#pragma warning(pop)
    outputBuffer[7] = asuint(alias);
    outputBuffer[8] = uint(mad(half(a), half(-b), half(c)));
    outputBuffer[9] = uint(mad(float(a), float(-b), float(c)));
    outputBuffer[10] = uint(mad(double(a), double(-b), double(c)));
}

// CHECK: type: uint
// CHECK-NEXT: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 17{{$}}
// CHECK-NEXT: 17{{$}}
// CHECK-NEXT: 17{{$}}
// CHECK-NEXT: 3735928559{{$}}
