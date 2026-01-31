//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage vertex -entry vertexMain

struct VSInput
{
    float2 data1 : TEXCOORD0;
    float2 data2 : TEXCOORD1;
};

struct VSOutput
{
    float4 pos : SV_Position;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    output.pos = float4(input.data1, input.data2);
    return output;
}

//CHECK: data1_0 : TEXCOORD0;
//CHECK: data2_0 : TEXCOORD1;
