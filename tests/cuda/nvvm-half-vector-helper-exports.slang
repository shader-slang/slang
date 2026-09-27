// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Runtime raw Half encodings for (1, -2, 4, -8); every check observes lane bits.
//TEST_INPUT:ubuffer(data=[15360 49152 17408 51200],stride=16):name=inputs
StructuredBuffer<uint4> inputs;
//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=results
RWStructuredBuffer<uint> results;

[CudaDeviceExport]
[noinline]
half2 exportedHalf2(half2 value)
{
    return half2(value.y, value.x);
}

[CudaDeviceExport]
[noinline]
half3 exportedHalf3(half3 value)
{
    return half3(value.z, value.x, value.y);
}

[CudaDeviceExport]
[noinline]
half4 exportedHalf4(half4 value)
{
    return half4(value.w, value.z, value.y, value.x);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint4 bits = inputs[0];
    half4 source = half4(
        bit_cast<half>(uint16_t(bits.x)), bit_cast<half>(uint16_t(bits.y)),
        bit_cast<half>(uint16_t(bits.z)), bit_cast<half>(uint16_t(bits.w)));
    half2 a = exportedHalf2(source.xy);
    half3 b = exportedHalf3(source.xyz);
    half4 c = exportedHalf4(source);
    results[0] = uint(bit_cast<uint16_t>(a.x));
    results[1] = uint(bit_cast<uint16_t>(a.y));
    results[2] = uint(bit_cast<uint16_t>(b.x));
    results[3] = uint(bit_cast<uint16_t>(b.y));
    results[4] = uint(bit_cast<uint16_t>(b.z));
    results[5] = uint(bit_cast<uint16_t>(c.x));
    results[6] = uint(bit_cast<uint16_t>(c.y));
    results[7] = uint(bit_cast<uint16_t>(c.z));
    results[8] = uint(bit_cast<uint16_t>(c.w));
    results[9] = 3;
    // Word 10 remains an untouched guard. ABI padding is never observed.
}

// CHECK: type: uint
// CHECK-NEXT: 49152{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 17408{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 49152{{$}}
// CHECK-NEXT: 51200{{$}}
// CHECK-NEXT: 17408{{$}}
// CHECK-NEXT: 49152{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 3{{$}}
// CHECK-NEXT: 3735928559{{$}}
