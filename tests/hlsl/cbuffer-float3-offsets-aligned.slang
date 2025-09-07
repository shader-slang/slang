//TEST:SIMPLE(filecheck=SPIRV): -target spirv -emit-spirv-directly -profile cs_6_2 -entry computeMain -line-directive-mode none -fvk-use-dx-layout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -dx12 -profile cs_6_2 -Xslang... -Xdxc -fvk-use-dx-layout -Xdxc -enable-16bit-types -X. -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -vk -profile cs_6_2 -emit-spirv-directly -Xslang... -fvk-use-dx-layout -X. -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -output-using-type
//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry computeMain -target spirv -profile cs_6_2 -no-codegen -line-directive-mode none -fvk-use-dx-layout

//TEST_INPUT:ubuffer(stride=4, count=17):out,name=outputBuffer
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

// Checks cbuffer packing rule cases involving 3-element vectors.
// HLSL aligns at the element size, while GLSL std140 and std430
// align to the size of a 4-element vector of the element type.
// Checks cases that are compatible with std140 and std430.

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

// REFLECT: "name": "v0",
// REFLECT: "offset": 0
// REFLECT: "name": "v1",
// REFLECT: "offset": 12
// REFLECT: "name": "v2",
// REFLECT: "offset": 16
// REFLECT: "name": "v3",
// REFLECT: "offset": 32
// REFLECT: "name": "v4",
// REFLECT: "offset": 60
// REFLECT: "name": "v5",
// REFLECT: "offset": 64
// REFLECT: "name": "v6",
// REFLECT: "offset": 80
// REFLECT: "name": "v7",
// REFLECT: "offset": 92

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

}

