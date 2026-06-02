//TEST:SIMPLE(filecheck=CS,SM60): -target dxil -entry computeMain
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil-assembly -entry computeMain
//TEST:SIMPLE(filecheck=CS,SM60): -target dxil-assembly -entry computeMain -profile cs_6_0
//TEST:SIMPLE(filecheck=CS,SM65): -target dxil-assembly -entry computeMain -profile cs_6_5
//TEST:SIMPLE(filecheck=LIB): -target dxil-assembly -entry computeMain -profile cs_6_3 -whole-program
//TEST:SIMPLE(filecheck=LIB): -target dxil-assembly -whole-program
//TEST:SIMPLE(filecheck=LIB68): -target dxil-assembly -profile lib_6_8 -whole-program
//TEST(windows):COMPILE: tests/bugs/gh-11021-dxil-default-profile.slang -target dxil -whole-program -o tests/bugs/gh-11021-dxil-default-profile-whole-program.dxil
//TEST(windows):COMPILE: tests/bugs/gh-11021-dxil-default-profile.slang -target dxil -o tests/bugs/gh-11021-dxil-default-profile.dxil

// CS: define void @computeMain()

// SM60: !dx.shaderModel = !{![[SM:[0-9]+]]}
// SM60: ![[SM]] = !{!"cs", i32 6, i32 0}

// SM65: !dx.shaderModel = !{![[SM:[0-9]+]]}
// SM65: ![[SM]] = !{!"cs", i32 6, i32 5}

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
