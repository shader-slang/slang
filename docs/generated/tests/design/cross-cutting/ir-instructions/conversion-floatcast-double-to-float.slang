//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=A narrowing float-to-float cast (`double` -> `float`) lowers to the documented `floatCast` conversion opcode (same-domain precision cast).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `floatCast` in its conversion-family row but the
// existing bundle covers only `intCast` (`conversion-intcast-ir.slang`)
// and `bitCast` (`conversion-bitcast-float-to-uint.slang`). A
// `float(double)` cast exercises the `floatCast` sibling on its
// natural axis (precision narrowing within the FP domain).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float(a);
}

// IR-LABEL: func %main
// IR: floatCast(%a)
