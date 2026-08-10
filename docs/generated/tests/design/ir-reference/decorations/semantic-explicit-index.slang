//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=9dcc3971ffe320daffa3155aa97909cc164f7f1f53867e82746329293f4841f5
//META: purpose=Boundary - a TEXCOORD with a trailing digit (TEXCOORD3) lowers to semantic("TEXCOORD3", -1 : Int); the digit stays fused with the name operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies which of the two `semantic` operands carries an index written in
// the source. Writing `TEXCOORD3` keeps the digit inside the name string
// and leaves the index operand at -1, so the CHECK pins both operands
// together - asserting only the name would not distinguish this encoding
// from a split `("TEXCOORD", 3)` form.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    float u : TEXCOORD3;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, V v)
{
    buf[0] = v.u + float(tid.x);
}

// CHECK: [semantic("TEXCOORD3", -1 : Int)]
