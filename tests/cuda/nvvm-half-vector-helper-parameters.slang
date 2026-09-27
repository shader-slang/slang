// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Runtime seed keeps the exhaustive bit families dependent on uploaded data.
//TEST_INPUT:ubuffer(data=[9321],stride=4):name=inputs
StructuredBuffer<uint> inputs;
// Six mismatch masks, completed iterations, and an untouched guard all start as sentinels.
//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=results
RWStructuredBuffer<uint> results;

// Only helper parameters are Half vectors. Results use existing scalar uint64 transport.
[noinline]
uint64_t pack2(half2 value)
{
    return (uint64_t(bit_cast<uint16_t>(value.x)) << 0) |
        (uint64_t(bit_cast<uint16_t>(value.y)) << 16);
}

[noinline]
uint64_t pack3(half3 value)
{
    return (uint64_t(bit_cast<uint16_t>(value.x)) << 0) |
        (uint64_t(bit_cast<uint16_t>(value.y)) << 16) |
        (uint64_t(bit_cast<uint16_t>(value.z)) << 32);
}

[noinline]
uint64_t pack4(half4 value)
{
    return (uint64_t(bit_cast<uint16_t>(value.x)) << 0) |
        (uint64_t(bit_cast<uint16_t>(value.y)) << 16) |
        (uint64_t(bit_cast<uint16_t>(value.z)) << 32) |
        (uint64_t(bit_cast<uint16_t>(value.w)) << 48);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint seed = inputs[0] & 65535u;
    uint errorsA2 = 0;
    uint errorsA3 = 0;
    uint errorsA4 = 0;
    uint errorsB2 = 0;
    uint errorsB3 = 0;
    uint errorsB4 = 0;
    uint completed = 0;
    for (uint iteration = 0; iteration < 65536u; ++iteration)
    {
        // XOR and addition modulo 65536 are bijections. Each lane sees every Half encoding,
        // including signed zeros and NaN payloads, without performing Half arithmetic.
        uint a = iteration ^ seed;
        uint b = (a + 0x2468u) & 65535u;
        uint4 bitsA = uint4(a, a ^ 0x1357u, a ^ 0x6c39u, a ^ 0xa5a5u);
        uint4 bitsB = uint4(b, b ^ 0x1357u, b ^ 0x6c39u, b ^ 0xa5a5u);

        // Keep all A results live across later B calls and inspect both families afterward.
        uint64_t valueA2 = pack2(half2(
            bit_cast<half>(uint16_t(bitsA.x)),
            bit_cast<half>(uint16_t(bitsA.y))));
        uint64_t valueA3 = pack3(half3(
            bit_cast<half>(uint16_t(bitsA.x)),
            bit_cast<half>(uint16_t(bitsA.y)),
            bit_cast<half>(uint16_t(bitsA.z))));
        uint64_t valueA4 = pack4(half4(
            bit_cast<half>(uint16_t(bitsA.x)),
            bit_cast<half>(uint16_t(bitsA.y)),
            bit_cast<half>(uint16_t(bitsA.z)),
            bit_cast<half>(uint16_t(bitsA.w))));
        uint64_t valueB4 = pack4(half4(
            bit_cast<half>(uint16_t(bitsB.x)),
            bit_cast<half>(uint16_t(bitsB.y)),
            bit_cast<half>(uint16_t(bitsB.z)),
            bit_cast<half>(uint16_t(bitsB.w))));
        uint64_t valueB3 = pack3(half3(
            bit_cast<half>(uint16_t(bitsB.x)),
            bit_cast<half>(uint16_t(bitsB.y)),
            bit_cast<half>(uint16_t(bitsB.z))));
        uint64_t valueB2 = pack2(half2(
            bit_cast<half>(uint16_t(bitsB.x)),
            bit_cast<half>(uint16_t(bitsB.y))));

        if (uint((valueA2 >> 0) & uint64_t(65535)) != bitsA.x)
            errorsA2 |= 1u;
        if (uint((valueA2 >> 16) & uint64_t(65535)) != bitsA.y)
            errorsA2 |= 2u;
        if (uint((valueA3 >> 0) & uint64_t(65535)) != bitsA.x)
            errorsA3 |= 1u;
        if (uint((valueA3 >> 16) & uint64_t(65535)) != bitsA.y)
            errorsA3 |= 2u;
        if (uint((valueA3 >> 32) & uint64_t(65535)) != bitsA.z)
            errorsA3 |= 4u;
        if (uint((valueA4 >> 0) & uint64_t(65535)) != bitsA.x)
            errorsA4 |= 1u;
        if (uint((valueA4 >> 16) & uint64_t(65535)) != bitsA.y)
            errorsA4 |= 2u;
        if (uint((valueA4 >> 32) & uint64_t(65535)) != bitsA.z)
            errorsA4 |= 4u;
        if (uint((valueA4 >> 48) & uint64_t(65535)) != bitsA.w)
            errorsA4 |= 8u;
        if (uint((valueB2 >> 0) & uint64_t(65535)) != bitsB.x)
            errorsB2 |= 1u;
        if (uint((valueB2 >> 16) & uint64_t(65535)) != bitsB.y)
            errorsB2 |= 2u;
        if (uint((valueB3 >> 0) & uint64_t(65535)) != bitsB.x)
            errorsB3 |= 1u;
        if (uint((valueB3 >> 16) & uint64_t(65535)) != bitsB.y)
            errorsB3 |= 2u;
        if (uint((valueB3 >> 32) & uint64_t(65535)) != bitsB.z)
            errorsB3 |= 4u;
        if (uint((valueB4 >> 0) & uint64_t(65535)) != bitsB.x)
            errorsB4 |= 1u;
        if (uint((valueB4 >> 16) & uint64_t(65535)) != bitsB.y)
            errorsB4 |= 2u;
        if (uint((valueB4 >> 32) & uint64_t(65535)) != bitsB.z)
            errorsB4 |= 4u;
        if (uint((valueB4 >> 48) & uint64_t(65535)) != bitsB.w)
            errorsB4 |= 8u;
        ++completed;
    }
    results[0] = errorsA2;
    results[1] = errorsA3;
    results[2] = errorsA4;
    results[3] = errorsB2;
    results[4] = errorsB3;
    results[5] = errorsB4;
    results[6] = completed;
    // Word 7 remains the untouched guard.
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 65536{{$}}
// CHECK-NEXT: 3735928559{{$}}
