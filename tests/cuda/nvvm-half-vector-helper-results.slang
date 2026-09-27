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

// Only helper results are Half vectors. Parameters are ordinary integer scalars.
[noinline]
half2 make2(uint x, uint y)
{
    return half2(bit_cast<half>(uint16_t(x)), bit_cast<half>(uint16_t(y)));
}

[noinline]
half3 make3(uint x, uint y, uint z)
{
    return half3(
        bit_cast<half>(uint16_t(x)),
        bit_cast<half>(uint16_t(y)),
        bit_cast<half>(uint16_t(z)));
}

[noinline]
half4 make4(uint x, uint y, uint z, uint w)
{
    return half4(
        bit_cast<half>(uint16_t(x)),
        bit_cast<half>(uint16_t(y)),
        bit_cast<half>(uint16_t(z)),
        bit_cast<half>(uint16_t(w)));
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
        half2 valueA2 = make2(bitsA.x, bitsA.y);
        half3 valueA3 = make3(bitsA.x, bitsA.y, bitsA.z);
        half4 valueA4 = make4(bitsA.x, bitsA.y, bitsA.z, bitsA.w);
        half4 valueB4 = make4(bitsB.x, bitsB.y, bitsB.z, bitsB.w);
        half3 valueB3 = make3(bitsB.x, bitsB.y, bitsB.z);
        half2 valueB2 = make2(bitsB.x, bitsB.y);

        if (uint(bit_cast<uint16_t>(valueA2.x)) != bitsA.x)
            errorsA2 |= 1u;
        if (uint(bit_cast<uint16_t>(valueA2.y)) != bitsA.y)
            errorsA2 |= 2u;
        if (uint(bit_cast<uint16_t>(valueA3.x)) != bitsA.x)
            errorsA3 |= 1u;
        if (uint(bit_cast<uint16_t>(valueA3.y)) != bitsA.y)
            errorsA3 |= 2u;
        if (uint(bit_cast<uint16_t>(valueA3.z)) != bitsA.z)
            errorsA3 |= 4u;
        if (uint(bit_cast<uint16_t>(valueA4.x)) != bitsA.x)
            errorsA4 |= 1u;
        if (uint(bit_cast<uint16_t>(valueA4.y)) != bitsA.y)
            errorsA4 |= 2u;
        if (uint(bit_cast<uint16_t>(valueA4.z)) != bitsA.z)
            errorsA4 |= 4u;
        if (uint(bit_cast<uint16_t>(valueA4.w)) != bitsA.w)
            errorsA4 |= 8u;
        if (uint(bit_cast<uint16_t>(valueB2.x)) != bitsB.x)
            errorsB2 |= 1u;
        if (uint(bit_cast<uint16_t>(valueB2.y)) != bitsB.y)
            errorsB2 |= 2u;
        if (uint(bit_cast<uint16_t>(valueB3.x)) != bitsB.x)
            errorsB3 |= 1u;
        if (uint(bit_cast<uint16_t>(valueB3.y)) != bitsB.y)
            errorsB3 |= 2u;
        if (uint(bit_cast<uint16_t>(valueB3.z)) != bitsB.z)
            errorsB3 |= 4u;
        if (uint(bit_cast<uint16_t>(valueB4.x)) != bitsB.x)
            errorsB4 |= 1u;
        if (uint(bit_cast<uint16_t>(valueB4.y)) != bitsB.y)
            errorsB4 |= 2u;
        if (uint(bit_cast<uint16_t>(valueB4.z)) != bitsB.z)
            errorsB4 |= 4u;
        if (uint(bit_cast<uint16_t>(valueB4.w)) != bitsB.w)
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
