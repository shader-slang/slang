// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Width 64: 16 boundary patterns followed by every single-bit position in a 64-bit word.
// Each input is masked to the tested width. Independent host bit loops produce the
// count, reversed bits, unsigned/signed high index and low index expectations.
// Signed8 public countbits first sign-extends to uint32; other widths count their own bits.
// Live loads and independently checked vector lanes prevent folding or a roundtrip oracle.

//TEST_INPUT: ubuffer(data=[0 0 1 0 4294967295 4294967295 4294967294 4294967295 4294967293 4294967295 0 2147483648 4294967295 2147483647 1 2147483648 4294967294 2147483647 1431655765 1431655765 2863311530 2863311530 4294967295 0 0 4294967295 1 1 0 2147483647 2147483649 2147483648 1 0 2 0 4 0 8 0 16 0 32 0 64 0 128 0 256 0 512 0 1024 0 2048 0 4096 0 8192 0 16384 0 32768 0 65536 0 131072 0 262144 0 524288 0 1048576 0 2097152 0 4194304 0 8388608 0 16777216 0 33554432 0 67108864 0 134217728 0 268435456 0 536870912 0 1073741824 0 2147483648 0 0 1 0 2 0 4 0 8 0 16 0 32 0 64 0 128 0 256 0 512 0 1024 0 2048 0 4096 0 8192 0 16384 0 32768 0 65536 0 131072 0 262144 0 524288 0 1048576 0 2097152 0 4194304 0 8388608 0 16777216 0 33554432 0 67108864 0 134217728 0 268435456 0 536870912 0 1073741824 0 2147483648], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 4294967295 4294967295 4294967295 1 1 0 2147483648 0 0 0 64 64 4294967295 4294967295 63 4294967295 0 63 63 4294967295 2147483647 63 0 1 63 63 4294967295 3221225471 63 1 0 1 1 1 0 63 62 63 63 63 4294967294 4294967295 62 62 0 2 2 1 2147483648 63 62 0 62 62 4294967294 2147483647 62 62 1 32 32 2863311530 2863311530 62 62 0 32 32 1431655765 1431655765 63 62 1 32 32 0 4294967295 31 31 0 32 32 4294967295 0 63 31 32 2 2 2147483648 2147483648 32 32 0 31 31 4294967294 0 62 62 32 3 3 1 2147483649 63 62 0 1 1 0 2147483648 0 0 0 1 1 0 1073741824 1 1 1 1 1 0 536870912 2 2 2 1 1 0 268435456 3 3 3 1 1 0 134217728 4 4 4 1 1 0 67108864 5 5 5 1 1 0 33554432 6 6 6 1 1 0 16777216 7 7 7 1 1 0 8388608 8 8 8 1 1 0 4194304 9 9 9 1 1 0 2097152 10 10 10 1 1 0 1048576 11 11 11 1 1 0 524288 12 12 12 1 1 0 262144 13 13 13 1 1 0 131072 14 14 14 1 1 0 65536 15 15 15 1 1 0 32768 16 16 16 1 1 0 16384 17 17 17 1 1 0 8192 18 18 18 1 1 0 4096 19 19 19 1 1 0 2048 20 20 20 1 1 0 1024 21 21 21 1 1 0 512 22 22 22 1 1 0 256 23 23 23 1 1 0 128 24 24 24 1 1 0 64 25 25 25 1 1 0 32 26 26 26 1 1 0 16 27 27 27 1 1 0 8 28 28 28 1 1 0 4 29 29 29 1 1 0 2 30 30 30 1 1 0 1 31 31 31 1 1 2147483648 0 32 32 32 1 1 1073741824 0 33 33 33 1 1 536870912 0 34 34 34 1 1 268435456 0 35 35 35 1 1 134217728 0 36 36 36 1 1 67108864 0 37 37 37 1 1 33554432 0 38 38 38 1 1 16777216 0 39 39 39 1 1 8388608 0 40 40 40 1 1 4194304 0 41 41 41 1 1 2097152 0 42 42 42 1 1 1048576 0 43 43 43 1 1 524288 0 44 44 44 1 1 262144 0 45 45 45 1 1 131072 0 46 46 46 1 1 65536 0 47 47 47 1 1 32768 0 48 48 48 1 1 16384 0 49 49 49 1 1 8192 0 50 50 50 1 1 4096 0 51 51 51 1 1 2048 0 52 52 52 1 1 1024 0 53 53 53 1 1 512 0 54 54 54 1 1 256 0 55 55 55 1 1 128 0 56 56 56 1 1 64 0 57 57 57 1 1 32 0 58 58 58 1 1 16 0 59 59 59 1 1 8 0 60 60 60 1 1 4 0 61 61 61 1 1 2 0 62 62 62 1 1 1 0 63 62 63], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

uint64_t loadValue(uint lane)
{
    uint base = 2 * lane;
    return uint64_t(uint64_t(inputWords[base]) | (uint64_t(inputWords[base + 1]) << 32));
}

uint64_t loadReverse(uint lane)
{
    uint base = 7 * lane;
    return uint64_t(uint64_t(expectedWords[base + 2]) | (uint64_t(expectedWords[base + 3]) << 32));
}

[numthreads(80, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint64_t u = loadValue(lane);
    int64_t s = int64_t(u);
    uint errors = 0;
    uint base = 7 * lane;
    errors |= countbits(u) != expectedWords[base] ? (1u << 0) : 0u;
    errors |= countbits(s) != expectedWords[base + 1] ? (1u << 1) : 0u;
    errors |= reversebits(u) != loadReverse(lane) ? (1u << 2) : 0u;
    errors |= reversebits(s) != int64_t(loadReverse(lane)) ? (1u << 3) : 0u;
    errors |= firstbithigh(u) != expectedWords[base + 4] ? (1u << 4) : 0u;
    errors |= firstbithigh(s) != expectedWords[base + 5] ? (1u << 5) : 0u;
    errors |= firstbitlow(u) != expectedWords[base + 6] ? (1u << 6) : 0u;
    errors |= firstbitlow(s) != expectedWords[base + 6] ? (1u << 7) : 0u;
    {
        vector<uint64_t, 2> uv;
        vector<int64_t, 2> sv;
        vector<uint64_t, 2> reversed;
        vector<uint, 2> counts, signedCounts, highs, signedHighs, lows;
        for (uint component = 0; component < 2; ++component)
        {
            uint row = (lane + component) % 80;
            uint rowBase = 7 * row;
            uv[component] = loadValue(row);
            sv[component] = int64_t(uv[component]);
            reversed[component] = loadReverse(row);
            counts[component] = expectedWords[rowBase];
            signedCounts[component] = expectedWords[rowBase + 1];
            highs[component] = expectedWords[rowBase + 4];
            signedHighs[component] = expectedWords[rowBase + 5];
            lows[component] = expectedWords[rowBase + 6];
        }
        errors |= !all(countbits(uv) == counts) ? (1u << 8) : 0u;
        errors |= !all(countbits(sv) == signedCounts) ? (1u << 9) : 0u;
        errors |= !all(reversebits(uv) == reversed) ? (1u << 10) : 0u;
        errors |= !all(reversebits(sv) == vector<int64_t, 2>(reversed)) ? (1u << 11) : 0u;
        errors |= !all(firstbithigh(uv) == highs) ? (1u << 12) : 0u;
        errors |= !all(firstbithigh(sv) == signedHighs) ? (1u << 13) : 0u;
        errors |= !all(firstbitlow(uv) == lows) ? (1u << 14) : 0u;
        errors |= !all(firstbitlow(sv) == lows) ? (1u << 15) : 0u;
    }
    {
        vector<uint64_t, 3> uv;
        vector<int64_t, 3> sv;
        vector<uint64_t, 3> reversed;
        vector<uint, 3> counts, signedCounts, highs, signedHighs, lows;
        for (uint component = 0; component < 3; ++component)
        {
            uint row = (lane + component) % 80;
            uint rowBase = 7 * row;
            uv[component] = loadValue(row);
            sv[component] = int64_t(uv[component]);
            reversed[component] = loadReverse(row);
            counts[component] = expectedWords[rowBase];
            signedCounts[component] = expectedWords[rowBase + 1];
            highs[component] = expectedWords[rowBase + 4];
            signedHighs[component] = expectedWords[rowBase + 5];
            lows[component] = expectedWords[rowBase + 6];
        }
        errors |= !all(countbits(uv) == counts) ? (1u << 16) : 0u;
        errors |= !all(countbits(sv) == signedCounts) ? (1u << 17) : 0u;
        errors |= !all(reversebits(uv) == reversed) ? (1u << 18) : 0u;
        errors |= !all(reversebits(sv) == vector<int64_t, 3>(reversed)) ? (1u << 19) : 0u;
        errors |= !all(firstbithigh(uv) == highs) ? (1u << 20) : 0u;
        errors |= !all(firstbithigh(sv) == signedHighs) ? (1u << 21) : 0u;
        errors |= !all(firstbitlow(uv) == lows) ? (1u << 22) : 0u;
        errors |= !all(firstbitlow(sv) == lows) ? (1u << 23) : 0u;
    }
    {
        vector<uint64_t, 4> uv;
        vector<int64_t, 4> sv;
        vector<uint64_t, 4> reversed;
        vector<uint, 4> counts, signedCounts, highs, signedHighs, lows;
        for (uint component = 0; component < 4; ++component)
        {
            uint row = (lane + component) % 80;
            uint rowBase = 7 * row;
            uv[component] = loadValue(row);
            sv[component] = int64_t(uv[component]);
            reversed[component] = loadReverse(row);
            counts[component] = expectedWords[rowBase];
            signedCounts[component] = expectedWords[rowBase + 1];
            highs[component] = expectedWords[rowBase + 4];
            signedHighs[component] = expectedWords[rowBase + 5];
            lows[component] = expectedWords[rowBase + 6];
        }
        errors |= !all(countbits(uv) == counts) ? (1u << 24) : 0u;
        errors |= !all(countbits(sv) == signedCounts) ? (1u << 25) : 0u;
        errors |= !all(reversebits(uv) == reversed) ? (1u << 26) : 0u;
        errors |= !all(reversebits(sv) == vector<int64_t, 4>(reversed)) ? (1u << 27) : 0u;
        errors |= !all(firstbithigh(uv) == highs) ? (1u << 28) : 0u;
        errors |= !all(firstbithigh(sv) == signedHighs) ? (1u << 29) : 0u;
        errors |= !all(firstbitlow(uv) == lows) ? (1u << 30) : 0u;
        errors |= !all(firstbitlow(sv) == lows) ? (1u << 31) : 0u;
    }
    outputBuffer[2 * lane] = errors;
    outputBuffer[2 * lane + 1] = 7000u + lane;
    if (lane == 0)
    {
        outputBuffer[160] = countbits(int64_t(-1));
        outputBuffer[161] = firstbithigh(int64_t(-1));
        outputBuffer[162] = firstbitlow(uint64_t(0));
    }
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7061{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7062{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7063{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7064{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7065{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7066{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7067{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7068{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7069{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7070{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7071{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7072{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7073{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7074{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7075{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7076{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7077{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7078{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 7079{{$}}
// CHECK-NEXT: 64{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 3735928559{{$}}
