// An `invariant` position output is marked so every pipeline computes it identically,
// whether it is a struct field, an `out` parameter or the return value. A struct shared with
// the fragment stage carries no qualifier on the fragment input.

//TEST:SIMPLE(filecheck=SPIRV_FIELD): -target spirv-asm -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=SPIRV_PARAM): -target spirv-asm -entry outputParameter -stage vertex
//TEST:SIMPLE(filecheck=SPIRV_RETURN): -target spirv-asm -entry returnValue -stage vertex
//TEST:SIMPLE(filecheck=SPIRV_RETURN_OUT): -target spirv-asm -entry returnWithOut -stage vertex
//TEST:SIMPLE(filecheck=SPIRV_NONE): -target spirv-asm -entry plainOutput -stage vertex
//TEST:SIMPLE(filecheck=SPIRV_FRAG): -target spirv-asm -entry fragmentInput -stage fragment
//TEST:SIMPLE(filecheck=GLSL_FIELD): -target glsl -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=GLSL_PARAM): -target glsl -entry outputParameter -stage vertex
//TEST:SIMPLE(filecheck=GLSL_RETURN): -target glsl -entry returnValue -stage vertex
//TEST:SIMPLE(filecheck=GLSL_FRAG): -target glsl -entry fragmentInput -stage fragment
//TEST:SIMPLE(filecheck=VIA_GLSL): -target spirv-asm -emit-spirv-via-glsl -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=METAL_FIELD): -target metal -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=METAL_PARAM): -target metal -entry outputParameter -stage vertex
//TEST:SIMPLE(filecheck=METAL_RETURN): -target metal -entry returnValue -stage vertex
//TEST:SIMPLE(filecheck=METAL_RETURN_OUT): -target metal -entry returnWithOut -stage vertex
//TEST:SIMPLE(filecheck=METAL_FRAG): -target metal -entry fragmentInput -stage fragment
//TEST:SIMPLE(filecheck=WGSL_FIELD): -target wgsl -entry outputField -stage vertex
//TEST:SIMPLE(filecheck=WGSL_RETURN): -target wgsl -entry returnValue -stage vertex
//TEST:SIMPLE(filecheck=WGSL_RETURN_OUT): -target wgsl -entry returnWithOut -stage vertex
//TEST:SIMPLE(filecheck=WGSL_FRAG): -target wgsl -entry fragmentInput -stage fragment
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

invariant float4 returnValue(float3 p : POSITION) : SV_Position
{
    return float4(p * 2.0 + 1.0, 1.0);
}

invariant float4 returnWithOut(float3 p : POSITION, out float2 uv : TEXCOORD0) : SV_Position
{
    uv = p.xy;
    return float4(p * 2.0 + 1.0, 1.0);
}

float4 plainOutput(float3 p : POSITION) : SV_Position
{
    return float4(p * 2.0 + 1.0, 1.0);
}

float4 fragmentInput(VSOut input) : SV_Target
{
    return float4(input.uv, input.pos.z, 1.0);
}

// SPIRV_FIELD: OpDecorate %gl_Position BuiltIn Position
// SPIRV_FIELD: OpDecorate %gl_Position Invariant
// SPIRV_PARAM: OpDecorate %gl_Position BuiltIn Position
// SPIRV_PARAM: OpDecorate %gl_Position Invariant
// SPIRV_RETURN: OpDecorate %gl_Position BuiltIn Position
// SPIRV_RETURN: OpDecorate %gl_Position Invariant
// SPIRV_RETURN_OUT: OpDecorate %gl_Position BuiltIn Position
// SPIRV_RETURN_OUT: OpDecorate %gl_Position Invariant
// SPIRV_NONE: OpDecorate %gl_Position BuiltIn Position
// SPIRV_NONE-NOT: Invariant
// SPIRV_FRAG-NOT: Invariant
// SPIRV_FRAG: OpDecorate %gl_FragCoord BuiltIn FragCoord
// SPIRV_FRAG-NOT: Invariant

// GLSL_FIELD: invariant gl_Position;
// GLSL_PARAM: invariant gl_Position;
// GLSL_RETURN: invariant gl_Position;
// GLSL_FRAG-NOT: invariant
// GLSL_FRAG: gl_FragCoord
// GLSL_FRAG-NOT: invariant

// VIA_GLSL: OpMemberDecorate %gl_PerVertex 0 Invariant

// METAL_FIELD: {{\[\[position, invariant\]\]}}
// METAL_PARAM: {{\[\[position, invariant\]\]}}
// METAL_RETURN: {{\[\[position, invariant\]\]}}
// METAL_RETURN_OUT: {{\[\[position, invariant\]\]}}
// METAL_FRAG-NOT: {{position, invariant}}
// METAL_FRAG: {{\[\[position\]\]}}
// METAL_FRAG-NOT: {{position, invariant}}

// WGSL_FIELD: @invariant @builtin(position)
// WGSL_RETURN: @invariant @builtin(position)
// WGSL_RETURN_OUT: @invariant @builtin(position)
// WGSL_FRAG-NOT: invariant
// WGSL_FRAG: @builtin(position)
// WGSL_FRAG-NOT: invariant

//HLSL: warning E56116
