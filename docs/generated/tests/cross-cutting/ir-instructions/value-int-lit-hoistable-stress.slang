//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Stress probe of the documented `IntLit` literal value (hoistable): six textual occurrences of `7` in source still resolve to the same `7 : Int` constant operand at every `add` use-site in the IR dump.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `IntLit`/`FloatLit`/`StringLit` as literal constants
// and marks them hoistable. The visible consequence of hoistable
// dedup for an `IntLit` is that repeated literal-7 uses in source
// each appear as `7 : Int` in the IR — same canonical text — rather
// than as numbered SSA references to a separately stored literal.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 7;
    buf[1] = a + 7;
    buf[2] = a + 7;
}

// IR-LABEL: func %main
// IR: add(%a, 7 : Int)
// IR: add(%a, 7 : Int)
// IR: add(%a, 7 : Int)
