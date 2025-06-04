//TEST:SIMPLE(filecheck=SPIRV): -target spirv -profile cs_6_2 -entry computeMain -line-directive-mode none -fvk-use-dx-layout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -dx12 -use-dxil -Xslang... -Xdxc -fvk-use-dx-layout -Xdxc -enable-16bit-types -X. -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -vk -Xslang... -fvk-use-dx-layout -X. -output-using-type

// dxc: -T cs_6_2 -E computeMain -spirv -fvk-use-dx-layout -enable-16bit-types

//TEST_INPUT:ubuffer(stride=4, count=10):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:set Constants.v0=1.0
//TEST_INPUT:set Constants.v1={2.0,3.0,4.0}
//TEST_INPUT:set Constants.v2={5.0,6.0,7.0}
//TEST_INPUT:set Constants.v6={13.0,14.0,15.0}

// HLSL allows 3-element vectors to occur at non-power-of-two offsets
// in cbuffers, as long as they don't cross a 16-byte boundary.
// GLSL only allows 3-element vectors to occur at non-power-of-two offsets
// so long as the struct is layout(scalar).
//
// Checks cases where the offsets of 3-element vectors within a cbuffer
// will not be located at a power-of-two offset within a 16-byte chunk.
//
// FIXME: float16_t-based types not included in the compute test due to
//        issues on the DX12 target.

cbuffer Constants
{
    // float3 v1 should be packed in with float v0.
    float v0;
    float3 v1;

    // float3 v2 should appear after a 16-byte gap, plus an additional 4 byte gap.
    float3 v2 : packoffset(c2.y);

    // float16_t v3 should appear after v2.
    float16_t v3;

    // vector<float16_t, 3> v4 should be packed in with v3.
    vector<float16_t, 3> v4;

    // There should be a 2-byte gap between v5 and v6.
    float16_t v5 : packoffset(c4);
    float3 v6;
};

// SPIRV: OpMemberDecorate {{.*}} 0 Offset 0
// SPIRV: OpMemberDecorate {{.*}} 1 Offset 4
// SPIRV: OpMemberDecorate {{.*}} 2 Offset 36
// SPIRV: OpMemberDecorate {{.*}} 3 Offset 48
// SPIRV: OpMemberDecorate {{.*}} 4 Offset 50
// SPIRV: OpMemberDecorate {{.*}} 5 Offset 64
// SPIRV: OpMemberDecorate {{.*}} 6 Offset 68


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = 0;
    outputBuffer[i++] = v0;
    outputBuffer[i++] = v1.x;
    outputBuffer[i++] = v1.y;
    outputBuffer[i++] = v1.z;
    outputBuffer[i++] = v2.x;
    outputBuffer[i++] = v2.y;
    outputBuffer[i++] = v2.z;
    outputBuffer[i++] = v6.x;
    outputBuffer[i++] = v6.y;
    outputBuffer[i++] = v6.z;

    // BUFFER: 1
    // BUFFER-NEXT: 2
    // BUFFER-NEXT: 3
    // BUFFER-NEXT: 4
    // BUFFER-NEXT: 5
    // BUFFER-NEXT: 6
    // BUFFER-NEXT: 7
    // BUFFER-NEXT: 13
    // BUFFER-NEXT: 14
    // BUFFER-NEXT: 15
}
