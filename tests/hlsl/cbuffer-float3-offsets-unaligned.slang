//TEST:SIMPLE(filecheck=SPIRV): -target spirv -profile cs_6_2 -entry computeMain -line-directive-mode none -fvk-use-dx-layout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -dx12 -use-dxil -Xslang... -Xdxc -fvk-use-dx-layout -Xdxc -enable-16bit-types -X. -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -vk -Xslang... -fvk-use-dx-layout -X. -output-using-type
//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry computeMain -target spirv -profile cs_6_2 -no-codegen -line-directive-mode none -fvk-use-dx-layout

// dxc: -T cs_6_2 -E computeMain -spirv -fvk-use-dx-layout -enable-16bit-types

//TEST_INPUT:ubuffer(stride=4, count=24):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:set Constants.v0=1.0
//TEST_INPUT:set Constants.v1={2.0,3.0,4.0}
//TEST_INPUT:set Constants.v2=0x4500
//TEST_INPUT:set Constants.v3={0x4600,0x4700,0x4800}
//TEST_INPUT:set Constants.v4=0x4880
//TEST_INPUT:set Constants.v5={0x4900,0x4980,0x4a00}
//TEST_INPUT:set Constants.v6=0x4a80
//TEST_INPUT:set Constants.v7={0x4b00,0x4b80,0x4c00,0x4c40}
//TEST_INPUT:set Constants.v8={0x4c80,0x4cc0,0x4d00}
//TEST_INPUT:set Constants.v9=0x4d40
//TEST_INPUT:set Constants.v10={22.0,23.0,24.0}


// Checks cbuffer packing rule cases involving 3-element vectors.
// HLSL aligns at the element size, while GLSL std140 and std430
// align to the size of a 4-element vector of the element type.
// Checks cases that are not compatible with std140 and std430.

cbuffer Constants
{
    // float3 v1 should be packed in with float v0.
    float v0;
    float3 v1;

    // v2,v3,v4,v5 should all be packed together
    float16_t v2;
    vector<float16_t, 3> v3;
    float16_t v4;
    vector<float16_t, 3> v5;

    float16_t v6;
    vector<float16_t, 4> v7;
    vector<float16_t, 3> v8;

    // There should be a 2-byte gap between v4 and v5.
    float16_t v9;
    float3 v10;
};

// SPIRV: OpMemberDecorate {{.*}} 0 Offset 0
// SPIRV: OpMemberDecorate {{.*}} 1 Offset 4
// SPIRV: OpMemberDecorate {{.*}} 2 Offset 16
// SPIRV: OpMemberDecorate {{.*}} 3 Offset 18
// SPIRV: OpMemberDecorate {{.*}} 4 Offset 24
// SPIRV: OpMemberDecorate {{.*}} 5 Offset 26
// SPIRV: OpMemberDecorate {{.*}} 6 Offset 32
// SPIRV: OpMemberDecorate {{.*}} 7 Offset 34
// SPIRV: OpMemberDecorate {{.*}} 8 Offset 42
// SPIRV: OpMemberDecorate {{.*}} 9 Offset 48
// SPIRV: OpMemberDecorate {{.*}} 10 Offset 52

// REFLECT: "name": "v0",
// REFLECT: "offset": 0
// REFLECT: "name": "v1",
// REFLECT: "offset": 4
// REFLECT: "name": "v2",
// REFLECT: "offset": 16
// REFLECT: "name": "v3",
// REFLECT: "offset": 18
// REFLECT: "name": "v4",
// REFLECT: "offset": 24
// REFLECT: "name": "v5",
// REFLECT: "offset": 26
// REFLECT: "name": "v6",
// REFLECT: "offset": 32
// REFLECT: "name": "v7",
// REFLECT: "offset": 34
// REFLECT: "name": "v8",
// REFLECT: "offset": 42
// REFLECT: "name": "v9",
// REFLECT: "offset": 48
// REFLECT: "name": "v10",
// REFLECT: "offset": 52

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = 0;
    outputBuffer[i++] = v0;
    outputBuffer[i++] = v1.x;
    outputBuffer[i++] = v1.y;
    outputBuffer[i++] = v1.z;
    outputBuffer[i++] = v2;
    outputBuffer[i++] = v3.x;
    outputBuffer[i++] = v3.y;
    outputBuffer[i++] = v3.z;
    outputBuffer[i++] = v4;
    outputBuffer[i++] = v5.x;
    outputBuffer[i++] = v5.y;
    outputBuffer[i++] = v5.z;
    outputBuffer[i++] = v6;
    outputBuffer[i++] = v7.x;
    outputBuffer[i++] = v7.y;
    outputBuffer[i++] = v7.z;
    outputBuffer[i++] = v7.w;
    outputBuffer[i++] = v8.x;
    outputBuffer[i++] = v8.y;
    outputBuffer[i++] = v8.z;
    outputBuffer[i++] = v9;
    outputBuffer[i++] = v10.x;
    outputBuffer[i++] = v10.y;
    outputBuffer[i++] = v10.z;

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
}
