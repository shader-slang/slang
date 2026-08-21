//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=928ba5f8c333b9ec8b6e55397dc87060a4d672e8f7a0cc913acb7f02d7fea184
//META: purpose=A [vk::location(N)] modifier lowers to a glslLocation decoration carrying the integer location operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `glslLocation` row's single `IRIntLit` operand. The
// attribute is written on a varying field and the CHECK pins
// `[glslLocation(7 : Int)]`, so the location value must survive lowering
// unchanged and as an integer literal rather than, say, a folded layout
// reference.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    float pos : SV_Position;
    [vk::location(7)] linear float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    V v;
    v.pos = 0.0;
    v.u = float(tid.x);
    buf[0] = v.u;
}

// CHECK: [glslLocation(7 : Int)]
