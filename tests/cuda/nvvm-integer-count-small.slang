// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Rows contain width8 input/u-count/i-count, then width16 input/u-count/i-count.
// Expectations are fixed width-limited host bit counts. Signed8 counts include its existing
// pre-target sign extension to uint32, including -1 -> 32 and -128 -> 25. Runtime loads,
// independent vector lanes, completion words and the final guard make skipped work observable.
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 1 1 1 1 1 1 255 8 32 65535 16 16 254 7 31 65534 15 15 253 7 31 65533 15 15 128 1 25 32768 1 1 127 7 7 32767 15 15 129 2 26 32769 2 2 126 6 6 32766 14 14 85 4 4 21845 8 8 170 4 28 43690 8 8 255 8 32 65535 16 16 0 0 0 0 0 0 1 1 1 1 1 1 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 2 1 1 2 1 1 4 1 1 4 1 1 8 1 1 8 1 1 16 1 1 16 1 1 32 1 1 32 1 1 64 1 1 64 1 1 128 1 25 128 1 1 0 0 0 256 1 1 0 0 0 512 1 1 0 0 0 1024 1 1 0 0 0 2048 1 1 0 0 0 4096 1 1 0 0 0 8192 1 1 0 0 0 16384 1 1 0 0 0 32768 1 1], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint errors = 0;
    uint base = 6 * lane;
    uint8_t u8 = uint8_t(inputWords[base + 0]);
    int8_t s8 = int8_t(u8);
    errors |= countbits(u8) != inputWords[base + 1] ? 1u : 0u;
    errors |= countbits(s8) != inputWords[base + 2] ? 2u : 0u;
    {
        vector<uint8_t, 4> u;
        vector<int8_t, 4> s;
        uint4 expectedU, expectedS;
        for (uint component = 0; component < 4; ++component)
        {
            uint row = 6 * ((lane + component) % 32) + 0;
            u[component] = uint8_t(inputWords[row]);
            s[component] = int8_t(u[component]);
            expectedU[component] = inputWords[row + 1];
            expectedS[component] = inputWords[row + 2];
        }
        errors |= !all(countbits(u) == expectedU) ? 4u : 0u;
        errors |= !all(countbits(s) == expectedS) ? 8u : 0u;
    }
    uint16_t u16 = uint16_t(inputWords[base + 3]);
    int16_t s16 = int16_t(u16);
    errors |= countbits(u16) != inputWords[base + 4] ? 1u : 0u;
    errors |= countbits(s16) != inputWords[base + 5] ? 2u : 0u;
    {
        vector<uint16_t, 4> u;
        vector<int16_t, 4> s;
        uint4 expectedU, expectedS;
        for (uint component = 0; component < 4; ++component)
        {
            uint row = 6 * ((lane + component) % 32) + 3;
            u[component] = uint16_t(inputWords[row]);
            s[component] = int16_t(u[component]);
            expectedU[component] = inputWords[row + 1];
            expectedS[component] = inputWords[row + 2];
        }
        errors |= !all(countbits(u) == expectedU) ? 4u : 0u;
        errors |= !all(countbits(s) == expectedS) ? 8u : 0u;
    }
    outputBuffer[2 * lane] = errors;
    outputBuffer[2 * lane + 1] = 7100u + lane;
    if (lane == 0)
    {
        outputBuffer[64] = countbits(int8_t(-1));
        outputBuffer[65] = countbits(int8_t(-128));
        outputBuffer[66] = countbits(int16_t(-1));
    }
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7100{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7101{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7102{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7103{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7104{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7105{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7106{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7107{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7108{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7109{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7110{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7111{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7112{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7113{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7114{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7115{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7116{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7117{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7118{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7119{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7120{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7121{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7122{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7123{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7124{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7125{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7126{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7127{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7128{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7129{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7130{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7131{{$}}
// CHECK-NEXT: 32{{$}}
// CHECK-NEXT: 25{{$}}
// CHECK-NEXT: 16{{$}}
// CHECK-NEXT: 3735928559{{$}}
