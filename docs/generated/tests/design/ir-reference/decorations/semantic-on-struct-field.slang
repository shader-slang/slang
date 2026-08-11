//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=9dcc3971ffe320daffa3155aa97909cc164f7f1f53867e82746329293f4841f5
//META: purpose=A struct field declared with a TEXCOORD semantic emits a semantic decoration on the field's key with the semantic name and default -1 index.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that struct fields are one of the surfaces that produce
// `semantic`, not just entry-point parameters. The CHECK pins the
// `("TEXCOORD0", -1 : Int)` operand pair on a field of a plain struct used
// as a local value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    float pos : SV_Position;
    float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    V v;
    v.pos = 0.0;
    v.u = float(tid.x);
    buf[0] = v.u;
}

// CHECK: [semantic("TEXCOORD0", -1 : Int)]
