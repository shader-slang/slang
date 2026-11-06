// An `invariant` position output is marked so every pipeline computes it identically,
// whether it is a struct field or an `out` parameter.

//TEST:SIMPLE(filecheck=SPIRV_FIELD): -target spirv-asm -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=SPIRV_PARAM): -target spirv-asm -entry outputParameter -stage vertex
//TEST:SIMPLE(filecheck=SPIRV_NONE): -target spirv-asm -entry plainOutput -stage vertex
//TEST:SIMPLE(filecheck=GLSL_FIELD): -target glsl -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=GLSL_PARAM): -target glsl -entry outputParameter -stage vertex
//TEST:SIMPLE(filecheck=VIA_GLSL): -target spirv-asm -emit-spirv-via-glsl -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=METAL_FIELD): -target metal -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=METAL_PARAM): -target metal -entry outputParameter -stage vertex
//TEST:SIMPLE(filecheck=WGSL_FIELD): -target wgsl -entry outputField -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL): -target hlsl -entry outputField -stage vertex

struct VSOut
{
    invariant float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOut outputField(float3 p : POSITION)
{
    VSOut output;
    output.pos = float4(p * 2.0 + 1.0, 1.0);
    output.uv = p.xy;
    return output;
}

void outputParameter(float3 p : POSITION, out invariant float4 pos : SV_Position)
{
    pos = float4(p * 2.0 + 1.0, 1.0);
}

float4 plainOutput(float3 p : POSITION) : SV_Position
{
    return float4(p * 2.0 + 1.0, 1.0);
}

// SPIRV_FIELD: OpDecorate %gl_Position BuiltIn Position
// SPIRV_FIELD: OpDecorate %gl_Position Invariant
// SPIRV_PARAM: OpDecorate %gl_Position BuiltIn Position
// SPIRV_PARAM: OpDecorate %gl_Position Invariant
// SPIRV_NONE: OpDecorate %gl_Position BuiltIn Position
// SPIRV_NONE-NOT: Invariant

// GLSL_FIELD: invariant gl_Position;
// GLSL_PARAM: invariant gl_Position;

// VIA_GLSL: OpMemberDecorate %gl_PerVertex 0 Invariant

// METAL_FIELD: {{\[\[position, invariant\]\]}}
// METAL_PARAM: {{\[\[position, invariant\]\]}}

// WGSL_FIELD: @invariant @builtin(position)

//HLSL: warning E56116
