// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Rows: Half bits, Float32 bits, double low/high words, classification flags, widened Half bits.
// Flags are finite=1, infinity=2, NaN=4. NaN transport preserves bits; numerical conversions
// require NaN classification only. Nonzero high packed bits must be ignored on unpacking.
//TEST_INPUT: ubuffer(data=[0x8000 0x80000000 0 0x80000000 1 0x80000000 0x3c00 0x3f800000 0x12345678 0x3ff00000 1 0x3f800000 1 1 1 0 1 0x33800000 0x7bff 0x7f7fffff 0xffffffff 0x7fefffff 1 0x477fe000 0x7c00 0x7f800000 0 0x7ff00000 2 0x7f800000 0xfc00 0xff800000 0 0xfff00000 2 0xff800000 0x7e01 0x7fc12345 0x12345678 0x7ff80000 4 0 0x7c01 0xff800001 1 0xfff00000 4 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;
//TEST_INPUT: ubuffer(data=[99 0 99 0 99 0 99 0 99 0 99 0 99 0 99 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint base = 6 * lane;
    uint bits = inputWords[base];
    half h = asfloat16(uint16_t(bits));
    float f = asfloat(inputWords[base + 1]);
    double d = asdouble(inputWords[base + 2], inputWords[base + 3]);
    uint expected = inputWords[base + 4];
    uint errors = 0;
    errors |= uint(asuint16(h)) != bits ? 1u : 0u;
    errors |= uint(asuint16(asfloat16(int16_t(bits)))) != bits ? 2u : 0u;
    uint low, high;
    asuint(d, low, high);
    errors |= low != inputWords[base + 2] || high != inputWords[base + 3] ? 4u : 0u;
    uint alias;
#pragma warning(push)
#pragma warning(disable: 30051)
    asuint(d, alias, alias);
#pragma warning(pop)
    errors |= alias != inputWords[base + 3] ? 8u : 0u;

    uint hc = uint(isfinite(h)) | (uint(isinf(h)) << 1) | (uint(isnan(h)) << 2);
    uint fc = uint(isfinite(f)) | (uint(isinf(f)) << 1) | (uint(isnan(f)) << 2);
    uint dc = uint(isfinite(d)) | (uint(isinf(d)) << 1) | (uint(isnan(d)) << 2);
    errors |= hc != expected || fc != expected || dc != expected ? 16u : 0u;
    errors |= !all(isfinite(half2(h, h)) == bool2(expected == 1)) ? 32u : 0u;
    errors |= !all(isinf(float2(f, f)) == bool2(expected == 2)) ? 32u : 0u;
    errors |= !all(isnan(double2(d, d)) == bool2(expected == 4)) ? 32u : 0u;

    float unpacked = f16tof32(0xa5a50000u | bits);
    uint packed = f32tof16(unpacked);
    if (expected == 4)
    {
        errors |= !isnan(unpacked) || (packed & 0x7c00u) != 0x7c00u ||
            (packed & 0x3ffu) == 0 ? 64u : 0u;
    }
    else
    {
        errors |= asuint(unpacked) != inputWords[base + 5] || packed != bits ? 64u : 0u;
    }
    errors |= (packed & 0xffff0000u) != 0 ? 128u : 0u;
    outputBuffer[2 * lane] = errors;
    outputBuffer[2 * lane + 1] = 100 + lane;
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 100{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 101{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 102{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 103{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 104{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 105{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 106{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 107{{$}}
// CHECK-NEXT: 3735928559{{$}}
