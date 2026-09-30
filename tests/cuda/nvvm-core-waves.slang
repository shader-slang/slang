// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0x7fc12345 0x12345678 0xabcdef01], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;
//TEST_INPUT: ubuffer(data=[99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    // Exited lanes remain named in the original mask. Equality uses actual participants.
    if (lane >= 16)
        return;
    const uint mask = 0xffffffffu;
    uint errors = 0;
    float sameNaN = asfloat(inputWords[0]);
    bool same = WaveMaskAllEqual(mask, sameNaN);
    bool differentNaN = WaveMaskAllEqual(mask, asfloat(inputWords[0] ^ (lane & 1)));
    bool signedZero = WaveMaskAllEqual(mask, asfloat((lane & 1) << 31));
    errors |= !same ? 1u : 0u;
    errors |= differentNaN ? 2u : 0u;
    errors |= signedZero ? 4u : 0u;
    uint64_t wide = uint64_t(inputWords[1]) | (uint64_t(inputWords[2]) << 32);
    bool sameWide = WaveMaskAllEqual(mask, wide);
    bool differentWide = WaveMaskAllEqual(mask, wide ^ (uint64_t(lane & 1) << 32));
    errors |= !sameWide || differentWide ? 8u : 0u;
    // The first component differs; subsequent collectives must still execute for every lane.
    bool aggregate = WaveMaskAllEqual(mask, float2(float(lane & 1), sameNaN));
    bool equalAggregate = WaveMaskAllEqual(mask, float2(sameNaN, sameNaN));
    errors |= aggregate || !equalAggregate ? 16u : 0u;
    // Exited lanes cannot rejoin the snapshot; active participants may be a smaller subset.
    uint snapshot = WaveGetConvergedMask();
    errors |= (snapshot & 0xffff0000u) != 0 ? 64u : 0u;
    outputBuffer[lane] = errors | ((snapshot & (1u << lane)) == 0 ? 128u : 0u);
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
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
// CHECK-NEXT: 99{{$}}
