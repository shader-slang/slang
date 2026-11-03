// Run subgroupBallotBitExtract and subgroupInverseBallot on the GPU and check the results.
// CUDA and HLSL have no native operation for these functions, so their results come from
// bit math in the core module; Vulkan uses the native SPIR-V instructions.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -dx12 -profile cs_6_0 -shaderobj -render-feature hardware-device -output-using-type

// DXC must accept the generated HLSL on every platform, including where D3D12 is not available.
//TEST:SIMPLE(filecheck=DXIL):-target dxil-assembly -entry computeMain -profile cs_6_0
// DXIL: define void @computeMain()

import glsl;

// The expected values assume a subgroup of at least 32 lanes; an index at or above the subgroup
// size has an undefined result.
// [0..3]:  ballot value A for subgroupBallotBitExtract; bits 0, 3, 16, 18, 19, 20, 22, 25, 27 and 31 of A.x are set.
// [4..7]:  ballot value B for subgroupInverseBallot.
//          A.yzw and B.yzw are all ones, so reading the wrong component gives a wrong result.
// [8..39]: the bit index that each lane gives to subgroupBallotBitExtract, a permutation of 0..31.
//TEST_INPUT:ubuffer(data=[0x8A5D0009 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0x3C96A5C3 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0 1 2 3 4 7 8 15 16 17 24 29 30 31 5 6 9 10 11 12 13 14 18 19 20 21 22 23 25 26 27 28], stride=4):name inputBuffer
StructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint lane = dispatchThreadID.x;

    uint4 a = uint4(inputBuffer[0], inputBuffer[1], inputBuffer[2], inputBuffer[3]);
    uint4 b = uint4(inputBuffer[4], inputBuffer[5], inputBuffer[6], inputBuffer[7]);
    uint index = inputBuffer[8 + lane];

    outputBuffer[lane] = subgroupInverseBallot(b) ? 1u : 0u;
    outputBuffer[32 + lane] = subgroupBallotBitExtract(a, index) ? 1u : 0u;
}

// subgroupInverseBallot: bit <lane> of B.x.
// BUF: 1
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0

// subgroupBallotBitExtract(A, index) for the indices in the input, in lane order.
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
