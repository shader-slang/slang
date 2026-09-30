// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// CUDA source currently lacks the small-width reverse/scan prelude helpers. This fixture
// qualifies direct NVVM; nvvm-integer-count-small separately checks count on both routes.
// Width 8: 16 boundary patterns followed by every single-bit position in a 64-bit word.
// Each input is masked to the tested width. Independent host bit loops produce the
// count, reversed bits, unsigned/signed high index and low index expectations.
// Signed8 public countbits first sign-extends to uint32; other widths count their own bits.
// Live loads and independently checked vector lanes prevent folding or a roundtrip oracle.

//TEST_INPUT: ubuffer(data=[0 0 1 0 255 0 254 0 253 0 128 0 127 0 129 0 126 0 85 0 170 0 255 0 0 0 1 0 0 0 1 0 1 0 2 0 4 0 8 0 16 0 32 0 64 0 128 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 4294967295 4294967295 4294967295 1 1 128 0 0 0 0 8 32 255 0 7 4294967295 0 7 31 127 0 7 0 1 7 31 191 0 7 1 0 1 25 1 0 7 6 7 7 7 254 0 6 6 0 2 26 129 0 7 6 0 6 6 126 0 6 6 1 4 4 170 0 6 6 0 4 28 85 0 7 6 1 8 32 255 0 7 4294967295 0 0 0 0 0 4294967295 4294967295 4294967295 1 1 128 0 0 0 0 0 0 0 0 4294967295 4294967295 4294967295 1 1 128 0 0 0 0 1 1 128 0 0 0 0 1 1 64 0 1 1 1 1 1 32 0 2 2 2 1 1 16 0 3 3 3 1 1 8 0 4 4 4 1 1 4 0 5 5 5 1 1 2 0 6 6 6 1 25 1 0 7 6 7 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295 0 0 0 0 4294967295 4294967295 4294967295], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Public signed8 count first widens to uint32. This helper independently qualifies the
// same-width LLVM i8 primitive through libNVVM and checks the existing width8 oracle.
T countByteBits<T : __BuiltinIntegerType>(T value)
{
    __target_switch
    {
    case nvvm: __intrinsic_asm "llvm.ctpop", value;
    }
}

// The high UInt64 bit uses signed IR literal storage internally. Passing it explicitly must
// preserve its original bits rather than rebuilding a host signed integer from descriptor data.
uint64_t countHighWord()
{
    __target_switch
    {
    case nvvm: __intrinsic_asm "llvm.ctpop", uint64_t(0x8000000000000000UL);
    }
}

uint8_t loadValue(uint lane)
{
    uint base = 2 * lane;
    return uint8_t(uint64_t(inputWords[base]) | (uint64_t(inputWords[base + 1]) << 32));
}

uint8_t loadReverse(uint lane)
{
    uint base = 7 * lane;
    return uint8_t(uint64_t(expectedWords[base + 2]) | (uint64_t(expectedWords[base + 3]) << 32));
}

[numthreads(80, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint8_t u = loadValue(lane);
    int8_t s = int8_t(u);
    uint errors = 0;
    uint base = 7 * lane;
    errors |= uint(countByteBits(u)) != expectedWords[base] ? 1u : 0u;
    errors |= uint(countByteBits(s)) != expectedWords[base] ? 2u : 0u;
    errors |= countbits(u) != expectedWords[base] ? (1u << 0) : 0u;
    errors |= countbits(s) != expectedWords[base + 1] ? (1u << 1) : 0u;
    errors |= reversebits(u) != loadReverse(lane) ? (1u << 2) : 0u;
    errors |= reversebits(s) != int8_t(loadReverse(lane)) ? (1u << 3) : 0u;
    errors |= firstbithigh(u) != expectedWords[base + 4] ? (1u << 4) : 0u;
    errors |= firstbithigh(s) != expectedWords[base + 5] ? (1u << 5) : 0u;
    errors |= firstbitlow(u) != expectedWords[base + 6] ? (1u << 6) : 0u;
    errors |= firstbitlow(s) != expectedWords[base + 6] ? (1u << 7) : 0u;
    {
        vector<uint8_t, 2> uv;
        vector<int8_t, 2> sv;
        vector<uint8_t, 2> reversed;
        vector<uint, 2> counts, signedCounts, highs, signedHighs, lows;
        for (uint component = 0; component < 2; ++component)
        {
            uint row = (lane + component) % 80;
            uint rowBase = 7 * row;
            uv[component] = loadValue(row);
            sv[component] = int8_t(uv[component]);
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
        errors |= !all(reversebits(sv) == vector<int8_t, 2>(reversed)) ? (1u << 11) : 0u;
        errors |= !all(firstbithigh(uv) == highs) ? (1u << 12) : 0u;
        errors |= !all(firstbithigh(sv) == signedHighs) ? (1u << 13) : 0u;
        errors |= !all(firstbitlow(uv) == lows) ? (1u << 14) : 0u;
        errors |= !all(firstbitlow(sv) == lows) ? (1u << 15) : 0u;
    }
    {
        vector<uint8_t, 3> uv;
        vector<int8_t, 3> sv;
        vector<uint8_t, 3> reversed;
        vector<uint, 3> counts, signedCounts, highs, signedHighs, lows;
        for (uint component = 0; component < 3; ++component)
        {
            uint row = (lane + component) % 80;
            uint rowBase = 7 * row;
            uv[component] = loadValue(row);
            sv[component] = int8_t(uv[component]);
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
        errors |= !all(reversebits(sv) == vector<int8_t, 3>(reversed)) ? (1u << 19) : 0u;
        errors |= !all(firstbithigh(uv) == highs) ? (1u << 20) : 0u;
        errors |= !all(firstbithigh(sv) == signedHighs) ? (1u << 21) : 0u;
        errors |= !all(firstbitlow(uv) == lows) ? (1u << 22) : 0u;
        errors |= !all(firstbitlow(sv) == lows) ? (1u << 23) : 0u;
    }
    {
        vector<uint8_t, 4> uv;
        vector<int8_t, 4> sv;
        vector<uint8_t, 4> reversed;
        vector<uint, 4> counts, signedCounts, highs, signedHighs, lows;
        for (uint component = 0; component < 4; ++component)
        {
            uint row = (lane + component) % 80;
            uint rowBase = 7 * row;
            uv[component] = loadValue(row);
            sv[component] = int8_t(uv[component]);
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
        errors |= !all(reversebits(sv) == vector<int8_t, 4>(reversed)) ? (1u << 27) : 0u;
        errors |= !all(firstbithigh(uv) == highs) ? (1u << 28) : 0u;
        errors |= !all(firstbithigh(sv) == signedHighs) ? (1u << 29) : 0u;
        errors |= !all(firstbitlow(uv) == lows) ? (1u << 30) : 0u;
        errors |= !all(firstbitlow(sv) == lows) ? (1u << 31) : 0u;
    }
    errors |= countHighWord() != 1UL ? 1u : 0u;
    outputBuffer[2 * lane] = errors;
    outputBuffer[2 * lane + 1] = 7000u + lane;
    if (lane == 0)
    {
        outputBuffer[160] = countbits(int8_t(-1));
        outputBuffer[161] = firstbithigh(int8_t(-1));
        outputBuffer[162] = firstbitlow(uint8_t(0));
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
// CHECK-NEXT: 32{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 3735928559{{$}}
