//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=9dcc3971ffe320daffa3155aa97909cc164f7f1f53867e82746329293f4841f5
//META: purpose=Stress - a struct whose fields each carry interpolationMode, glslLocation and semantic decorations produces all per-field decorations without merging across fields.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the shader-IO decorations are per-field and mutually
// independent. Five adjacent fields use five different interpolation
// modifiers, five different explicit locations and five different
// semantics; the CHECK-DAG lines assert every mode integer and every
// location appears, so a lowering that hoisted one field's decoration onto
// the struct, or reused one inst for all fields, would fail.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct ManyAttrs
{
    [vk::location(0)] linear           float a : TEXCOORD0;
    [vk::location(1)] noperspective    float b : TEXCOORD1;
    [vk::location(2)] nointerpolation  float c : TEXCOORD2;
    [vk::location(3)] centroid         float d : TEXCOORD3;
    [vk::location(4)] sample           float e : TEXCOORD4;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, ManyAttrs v)
{
    buf[0] = v.a + v.b + v.c + v.d + v.e + float(tid.x);
}

// CHECK-DAG: [interpolationMode(0 : Int)]
// CHECK-DAG: [interpolationMode(1 : Int)]
// CHECK-DAG: [interpolationMode(2 : Int)]
// CHECK-DAG: [interpolationMode(3 : Int)]
// CHECK-DAG: [interpolationMode(4 : Int)]
// CHECK-DAG: [glslLocation(0 : Int)]
// CHECK-DAG: [glslLocation(1 : Int)]
// CHECK-DAG: [glslLocation(2 : Int)]
// CHECK-DAG: [glslLocation(3 : Int)]
// CHECK-DAG: [glslLocation(4 : Int)]
// CHECK-DAG: [semantic("TEXCOORD0", -1 : Int)]
// CHECK-DAG: [semantic("TEXCOORD4", -1 : Int)]
