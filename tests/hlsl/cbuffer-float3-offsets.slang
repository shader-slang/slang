//TEST:SIMPLE(filecheck=GLSL): -target glsl -profile cs_6_2 -stage compute -entry computeMain -line-directive-mode none -fvk-use-dx-layout
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -profile cs_6_2 -entry computeMain -line-directive-mode none -fvk-use-dx-layout

// DXC: -T cs_6_2 -E computeMain -spirv -fvk-use-dx-layout -enable-16bit-types

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):out,name=outputBuffer
RWStructuredBuffer<float4> outputBuffer;

//TEST_INPUT:set Constants.v0={1.0,2.0,3.0,4.0}
//TEST_INPUT:set Constants.v1=8.0
//TEST_INPUT:set Constants.v2={5.0,6.0,7.0}

// HLSL allows 3-element vectors to occur at non-power-of-two offsets
// in cbuffers, as long as they don't cross a 16-byte boundary.
// GLSL only allows 3-element vectors to occur at non-power-of-two offsets
// so long as the struct is layout(scalar).
//
// This test checks cases where the offsets of 3-element vectors within a cbuffer
// will be located at a power-of-two offset within a 16-byte chunk.
// Verifies that we (a) calculate correct offsets, and (b) request the appropriate
// additional features.

cbuffer Constants
{
    // float v1 should be packed in with float3 v0
    float3 v0;
    float v1;

    // float3[2] v3 should be aligned to next 16 byte boundary,
    // and not packed in with float v2.
    float v2;
    float3 v3[2];

    // float v4 should be packed in with last float3 from v3.
    float v4;

    // float3[1] v6 should be aligned to next 16 byte boundary
    // and not packed in with float v5.
    float v5;
    float3 v6[1];

    // float v7 should be packed in with float3 from v6.
    float v7;
};

// For spirv, check that the offsets are correct.
// SPIRV: OpMemberDecorate {{.*}} 0 Offset 0
// SPIRV: OpMemberDecorate {{.*}} 1 Offset 12
// SPIRV: OpMemberDecorate {{.*}} 2 Offset 16
// SPIRV: OpMemberDecorate {{.*}} 3 Offset 32
// SPIRV: OpMemberDecorate {{.*}} 4 Offset 60
// SPIRV: OpMemberDecorate {{.*}} 5 Offset 64
// SPIRV: OpMemberDecorate {{.*}} 6 Offset 80
// SPIRV: OpMemberDecorate {{.*}} 7 Offset 92

// For glsl, check that we're using the lowest common denominator for the uniform block.
// Could also test:
// - not using layout(offset = N)
// GLSL: layout(std140) uniform block

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = float4(v0, v1);
}
