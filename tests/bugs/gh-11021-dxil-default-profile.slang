//TEST:SIMPLE(filecheck=CS,SM60): -target dxil -entry computeMain
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil-assembly -entry computeMain
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil -entry computeMain -profile cs_5_1
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil-assembly -entry computeMain -profile cs_5_1
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil -entry computeMain -profile cs_5_0
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil-assembly -entry computeMain -profile cs_5_0
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil-assembly -entry computeMain -profile sm_5_0
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil-assembly -entry computeMain -profile sm_5_1
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil-assembly -entry computeMain -profile cs_6_0
//TEST:SIMPLE(filecheck=CS,SM65): -target dxil-assembly -entry computeMain -profile cs_6_5
//TEST:SIMPLE(filecheck=VS,SM60_VS): -target dxil-assembly -entry vsMain -profile vs_5_1
//TEST:SIMPLE(filecheck=PS,SM60_PS): -target dxil-assembly -entry psMain -profile ps_5_0
//TEST:SIMPLE(filecheck=LIB): -target dxil-assembly -entry computeMain -profile cs_5_1 -whole-program
//TEST:SIMPLE(filecheck=LIB): -target dxil-assembly -entry computeMain -profile cs_6_3 -whole-program
//TEST:SIMPLE(filecheck=LIB): -target dxil-assembly -whole-program
//TEST:SIMPLE(filecheck=LIB): -target dxil-assembly -profile lib_6_2 -whole-program
//TEST:SIMPLE(filecheck=LIB68): -target dxil-assembly -profile lib_6_8 -whole-program
//TEST(windows):COMPILE: tests/bugs/gh-11021-dxil-default-profile.slang -target dxil -o tests/bugs/gh-11021-dxil-default-profile.dxil
//TEST(windows):COMPILE: tests/bugs/gh-11021-dxil-default-profile.slang -target dxil -entry computeMain -profile cs_5_1 -whole-program -o tests/bugs/gh-11021-dxil-whole-program-profile.dxil

// CS: define void @computeMain()
// VS: define void @vsMain()
// PS: define void @psMain()

// SM60: !dx.shaderModel = !{![[SM:[0-9]+]]}
// SM60: ![[SM]] = !{!"cs", i32 6, i32 0}

// SM65: !dx.shaderModel = !{![[SM:[0-9]+]]}
// SM65: ![[SM]] = !{!"cs", i32 6, i32 5}

// SM60_VS: !dx.shaderModel = !{![[SM:[0-9]+]]}
// SM60_VS: ![[SM]] = !{!"vs", i32 6, i32 0}

// SM60_PS: !dx.shaderModel = !{![[SM:[0-9]+]]}
// SM60_PS: ![[SM]] = !{!"ps", i32 6, i32 0}

// LIB: !dx.shaderModel = !{![[SM:[0-9]+]]}
// LIB: ![[SM]] = !{!"lib", i32 6, i32 3}

// LIB68: !dx.shaderModel = !{![[SM:[0-9]+]]}
// LIB68: ![[SM]] = !{!"lib", i32 6, i32 8}

StructuredBuffer<float> buffer0;
StructuredBuffer<float> buffer1;
RWStructuredBuffer<float> result;

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

[shader("vertex")]
VertexOutput vsMain(float3 position : POSITION)
{
    VertexOutput output;
    output.position = float4(position, 1.0);
    output.color = float4(position, 1.0);
    return output;
}

[shader("fragment")]
float4 psMain(VertexOutput input) : SV_Target
{
    return input.color;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[index] = buffer0[index] + buffer1[index];
}
