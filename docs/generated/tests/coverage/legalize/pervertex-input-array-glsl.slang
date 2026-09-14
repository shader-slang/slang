//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes how GetAttributeAtVertex on a nointerpolation fragment input turns the scalar stage input into a single pervertexEXT three-element array shared by every vertex index.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `getOrCreatePerVertexInputArray` (slang-ir-glsl-legalize.cpp:3759) and
// its caller in `tryReplaceUsesOfStageInput`. In HLSL a barycentric per-vertex
// read is spelled as a function call on an ordinary `nointerpolation` input;
// in GLSL/SPIR-V the same value must be declared once as a `pervertexEXT`
// array of three elements and indexed. The legalizer therefore rewrites each
// `GetPerVertexInputArray` use into a lookup of a *memoized* array global —
// three calls on the same input must share one declaration, not create three.
// CHECK asserts exactly that: a single `pervertexEXT ... in vec3 i_col[3]`
// declaration, no second one, and three subscripts of it. The array is created
// by cloning the input's decorations while dropping the interpolation-mode
// decoration, which is why the emitted declaration has no `flat` qualifier
// even though the source says `nointerpolation`.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage fragment

struct FIn
{
    float4 pos : SV_Position;
    nointerpolation float3 col : COLOR0;
}

[shader("fragment")]
float4 main(FIn i) : SV_Target
{
    float3 a = GetAttributeAtVertex(i.col, 0);
    float3 b = GetAttributeAtVertex(i.col, 1);
    float3 c = GetAttributeAtVertex(i.col, 2);
    return float4(a + b + c, 1.0);
}

//GLSL: #extension GL_EXT_fragment_shader_barycentric : require
// Exactly one pervertexEXT array declaration is created for the input...
//GLSL: pervertexEXT
//GLSL-NEXT: layout(location = 0)
//GLSL-NEXT: in vec3  i_col_0[3];
// ...and the three per-vertex reads all index that same array.
//GLSL: i_col_0[0{{U?}}] + i_col_0[1{{U?}}] + i_col_0[2{{U?}}]
// The interpolation-mode decoration is not cloned onto the array, so no
// `flat` qualifier is emitted for it.
//GLSL-NOT: flat
