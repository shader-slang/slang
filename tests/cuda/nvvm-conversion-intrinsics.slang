// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Live IEEE encodings exercise signed zero, ties, sticky subnormals, exponent carries,
// infinities and NaNs. Expected encodings come from binary format rules, not another cast API.
// The 0x10000 Half and 0xffffffff float expectations request NaN classification only.
//TEST_INPUT: ubuffer(data=[0 2147483648 1065357312 1065365504 3212840960 855638016 855638017 1199566848 947904512 2139095040 4286578688 2139095041 2143363909 4294967295 1 2147483649], stride=4):name=inputBits
StructuredBuffer<uint> inputBits;

//TEST_INPUT: ubuffer(data=[0 32768 15360 15362 48128 0 1 31744 1024 31744 64512 65536 65536 65536 0 32768], stride=4):name=expectedHalf
StructuredBuffer<uint> expectedHalf;

//TEST_INPUT: ubuffer(data=[0 32768 1 32769 1023 1024 15360 48128 15362 31743 64511 31744 64512 32341 31745 65109], stride=4):name=halfBits
StructuredBuffer<uint> halfBits;

//TEST_INPUT: ubuffer(data=[0 2147483648 864026624 3011510272 947896320 947912704 1065353216 3212836864 1065369600 1199562752 3347046400 2139095040 4286578688 4294967295 4294967295 4294967295], stride=4):name=expectedFloat
StructuredBuffer<uint> expectedFloat;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Numeric conversions promise a NaN, not a particular NaN payload. Bit reinterpretation
// below has a stronger contract and checks every bit, including signaling NaN payloads.
bool matchesHalf(half value, uint expected)
{
    uint bits = uint(bit_cast<uint16_t>(value));
    return expected == 0x10000u
        ? (bits & 0x7c00u) == 0x7c00u && (bits & 0x3ffu) != 0
        : bits == expected;
}

bool matchesFloat(float value, uint expected)
{
    uint bits = bit_cast<uint>(value);
    return expected == 0xffffffffu
        ? (bits & 0x7f800000u) == 0x7f800000u && (bits & 0x7fffffu) != 0
        : bits == expected;
}

[numthreads(16, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    uint bits = inputBits[i];
    int signedBits = bit_cast<int>(bits);
    float value = bit_cast<float>(bits);
    uint errors = 0;
    errors |= bit_cast<uint>(asfloat(signedBits)) != bits ? 1u : 0u;
    errors |= bit_cast<uint>(asfloat(bits)) != bits ? 2u : 0u;
    errors |= bit_cast<uint>(asint(value)) != bits ? 4u : 0u;
    errors |= bit_cast<uint>(asint(bits)) != bits ? 8u : 0u;
    errors |= asuint(value) != bits ? 16u : 0u;
    errors |= asuint(signedBits) != bits ? 32u : 0u;
    errors |= !matchesHalf(f32tof16_(value), expectedHalf[i]) ? 64u : 0u;
    half halfValue = bit_cast<half>(uint16_t(halfBits[i]));
    errors |= !matchesFloat(f16tof32(halfValue), expectedFloat[i]) ? 128u : 0u;

    // Each vector uses different live lanes. Check each direction independently so an
    // inverse conversion cannot conceal a lane swap or a narrowing error.
    uint j = (i + 1) % 16;
    uint k = (i + 2) % 16;
    uint l = (i + 3) % 16;
    float4 floats = float4(value, bit_cast<float>(inputBits[j]),
        bit_cast<float>(inputBits[k]), bit_cast<float>(inputBits[l]));
    half4 halves = half4(halfValue, bit_cast<half>(uint16_t(halfBits[j])),
        bit_cast<half>(uint16_t(halfBits[k])), bit_cast<half>(uint16_t(halfBits[l])));
    half2 narrowed2 = f32tof16_(floats.xy);
    float2 widened2 = f16tof32(halves.xy);
    errors |= !matchesHalf(narrowed2.x, expectedHalf[i]) ? 256u : 0u;
    errors |= !matchesFloat(widened2.x, expectedFloat[i]) ? 512u : 0u;
    errors |= !matchesHalf(narrowed2.y, expectedHalf[j]) ? 256u : 0u;
    errors |= !matchesFloat(widened2.y, expectedFloat[j]) ? 512u : 0u;
    half3 narrowed3 = f32tof16_(floats.xyz);
    float3 widened3 = f16tof32(halves.xyz);
    errors |= !matchesHalf(narrowed3.x, expectedHalf[i]) ? 1024u : 0u;
    errors |= !matchesFloat(widened3.x, expectedFloat[i]) ? 2048u : 0u;
    errors |= !matchesHalf(narrowed3.y, expectedHalf[j]) ? 1024u : 0u;
    errors |= !matchesFloat(widened3.y, expectedFloat[j]) ? 2048u : 0u;
    errors |= !matchesHalf(narrowed3.z, expectedHalf[k]) ? 1024u : 0u;
    errors |= !matchesFloat(widened3.z, expectedFloat[k]) ? 2048u : 0u;
    half4 narrowed4 = f32tof16_(floats.xyzw);
    float4 widened4 = f16tof32(halves.xyzw);
    errors |= !matchesHalf(narrowed4.x, expectedHalf[i]) ? 4096u : 0u;
    errors |= !matchesFloat(widened4.x, expectedFloat[i]) ? 8192u : 0u;
    errors |= !matchesHalf(narrowed4.y, expectedHalf[j]) ? 4096u : 0u;
    errors |= !matchesFloat(widened4.y, expectedFloat[j]) ? 8192u : 0u;
    errors |= !matchesHalf(narrowed4.z, expectedHalf[k]) ? 4096u : 0u;
    errors |= !matchesFloat(widened4.z, expectedFloat[k]) ? 8192u : 0u;
    errors |= !matchesHalf(narrowed4.w, expectedHalf[l]) ? 4096u : 0u;
    errors |= !matchesFloat(widened4.w, expectedFloat[l]) ? 8192u : 0u;
    outputBuffer[i] = errors;
    if (i == 0)
    {
        // These API calls also cover constant folding newly exposed by canonical casts.
        outputBuffer[16] = uint(bit_cast<uint16_t>(f32tof16_(1.00048828125f)));
        outputBuffer[17] = uint(bit_cast<uint16_t>(f32tof16_(1.00146484375f)));
        outputBuffer[18] = uint(bit_cast<uint16_t>(f32tof16_(2.98023223876953125e-8f)));
        outputBuffer[19] = uint(bit_cast<uint16_t>(f32tof16_(2.980232594040899e-8f)));
        outputBuffer[20] = uint(bit_cast<uint16_t>(f32tof16_(-1.00048828125f)));
        outputBuffer[21] = uint(bit_cast<uint16_t>(f32tof16_(65520.0f)));
        outputBuffer[22] = uint(bit_cast<uint16_t>(f32tof16_(0.0000610053539276123046875f)));
        outputBuffer[23] = matchesHalf(f32tof16_(bit_cast<float>(0x7f800001u)), 0x10000u) ? 1u : 0u;
    }
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 15362{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 48128{{$}}
// CHECK-NEXT: 31744{{$}}
// CHECK-NEXT: 1024{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 3735928559{{$}}
