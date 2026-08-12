//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A float LiteralExpr lowers to an IRFloatLit constant inst spelled as `<value> : Float` inline in its operand position — the float-side of the doc's "LiteralExpr -> constant inst (IRIntLit, IRFloatLit, ...)" row.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: LiteralExpr type axis — float (sibling of Int). The doc
// lists IRFloatLit as one of the constant-inst classes. The literal
// `2.5f` shows up as `2.5 : Float` as the second operand of the
// `mul` instruction.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float r = a * 2.5f;
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK: mul(%{{.*}}, 2.5 : Float)
