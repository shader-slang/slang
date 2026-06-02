//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=A constructor-style float-to-int cast lowers to a conversion opcode (`castFloatToInt`) within the documented conversion family — distinct from same-domain `intCast` / `floatCast`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `bitCast`, `intCast`, and `floatCast` as
// representative conversion opcodes. Numeric float-to-int conversion
// is a distinct opcode (`castFloatToInt`) that the doc references
// implicitly by listing the conversion family as "..."; this test
// pins down the user surface (`int(floatExpr)`).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(a);
}

// IR-LABEL: func %main
// IR: castFloatToInt(%a)
