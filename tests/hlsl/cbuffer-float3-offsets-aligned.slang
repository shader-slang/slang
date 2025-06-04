//TEST:SIMPLE(filecheck=GLSL): -target glsl -profile cs_6_2 -stage compute -entry computeMain -line-directive-mode none -fvk-use-dx-layout
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -profile cs_6_2 -entry computeMain -line-directive-mode none -fvk-use-dx-layout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -dx12 -use-dxil -profile cs_6_2 -Xslang... -Xdxc -fvk-use-dx-layout -Xdxc -enable-16bit-types -X. -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -vk -profile cs_6_2 -Xslang... -fvk-use-dx-layout -X. -output-using-type

// dxc: -T cs_6_2 -E computeMain -spirv -fvk-use-dx-layout -enable-16bit-types

//TEST_INPUT:ubuffer(stride=4, count=25):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:set Constants.v0={1.0,2.0,3.0}
//TEST_INPUT:set Constants.v1=4.0
//TEST_INPUT:set Constants.v2=5.0
//TEST_INPUT:set Constants.v3[0]={6.0,7.0,8.0}
//TEST_INPUT:set Constants.v3[1]={9.0,10.0,11.0}
//TEST_INPUT:set Constants.v4=12.0
//TEST_INPUT:set Constants.v5=13.0
//TEST_INPUT:set Constants.v6[0]={14.0,15.0,16.0}
//TEST_INPUT:set Constants.v7=17.0
//TEST_INPUT:set Constants.v8[0]={18.0,19.0,20.0}
//TEST_INPUT:set Constants.v8[1]={21.0,22.0,23.0}
//TEST_INPUT:set Constants.v9={24.0,25.0}

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

    // float3[2] v8 should be aligned to offset 128.
    float3 v8[2] : packoffset(c8);

    // float2 v9 should be aligned to next 16 byte boundary
    float2 v9;
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
// SPIRV: OpMemberDecorate {{.*}} 8 Offset 128
// SPIRV: OpMemberDecorate {{.*}} 9 Offset 160

// For glsl, check that we're using the lowest common denominator for the uniform block:
// GLSL: layout(std140) uniform block
// Also check for the offset related to the packoffset usage:
// GLSL: layout(offset = 128)

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = 0;
    outputBuffer[i++] = v0.x;
    outputBuffer[i++] = v0.y;
    outputBuffer[i++] = v0.z;
    outputBuffer[i++] = v1;
    outputBuffer[i++] = v2;
    outputBuffer[i++] = v3[0].x;
    outputBuffer[i++] = v3[0].y;
    outputBuffer[i++] = v3[0].z;
    outputBuffer[i++] = v3[1].x;
    outputBuffer[i++] = v3[1].y;
    outputBuffer[i++] = v3[1].z;
    outputBuffer[i++] = v4;
    outputBuffer[i++] = v5;
    outputBuffer[i++] = v6[0].x;
    outputBuffer[i++] = v6[0].y;
    outputBuffer[i++] = v6[0].z;
    outputBuffer[i++] = v7;
    outputBuffer[i++] = v8[0].x;
    outputBuffer[i++] = v8[0].y;
    outputBuffer[i++] = v8[0].z;
    outputBuffer[i++] = v8[1].x;
    outputBuffer[i++] = v8[1].y;
    outputBuffer[i++] = v8[1].z;
    outputBuffer[i++] = v9.x;
    outputBuffer[i++] = v9.y;

    // BUFFER: 1
    // BUFFER-NEXT: 2
    // BUFFER-NEXT: 3
    // BUFFER-NEXT: 4
    // BUFFER-NEXT: 5
    // BUFFER-NEXT: 6
    // BUFFER-NEXT: 7
    // BUFFER-NEXT: 8
    // BUFFER-NEXT: 9
    // BUFFER-NEXT: 10
    // BUFFER-NEXT: 11
    // BUFFER-NEXT: 12
    // BUFFER-NEXT: 13
    // BUFFER-NEXT: 14
    // BUFFER-NEXT: 15
    // BUFFER-NEXT: 16
    // BUFFER-NEXT: 17
    // BUFFER-NEXT: 18
    // BUFFER-NEXT: 19
    // BUFFER-NEXT: 20
    // BUFFER-NEXT: 21
    // BUFFER-NEXT: 22
    // BUFFER-NEXT: 23
    // BUFFER-NEXT: 24
    // BUFFER-NEXT: 25
}

