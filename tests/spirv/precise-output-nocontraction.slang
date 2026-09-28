// A precise output must keep the arithmetic that produces it uncontracted, even
// when the arithmetic is assigned through an unqualified local first.
//TEST:SIMPLE(filecheck=PARAM): -target spirv-asm -entry outputParameter -stage vertex
//TEST:SIMPLE(filecheck=FIELD): -target spirv-asm -entry outputField -stage vertex

struct VSIn
{
    float3 pos : POSITION;
    float3 scale : SCALE;
    float3 offset : OFFSET;
};

void outputParameter(VSIn input, out precise float4 outPos : SV_Position)
{
    float3 p = input.pos * input.scale + input.offset;
    outPos = float4(p, 1.0);
}

struct VSOut
{
    precise float4 pos : SV_Position;
};

VSOut outputField(VSIn input)
{
    VSOut output;
    float3 p = input.pos * input.scale + input.offset;
    output.pos = float4(p, 1.0);
    return output;
}

// Both the multiply and the add must carry NoContraction.
//PARAM-COUNT-2: OpDecorate %{{.*}} NoContraction
//FIELD-COUNT-2: OpDecorate %{{.*}} NoContraction
