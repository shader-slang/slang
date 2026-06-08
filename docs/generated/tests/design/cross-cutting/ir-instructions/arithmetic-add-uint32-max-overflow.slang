//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Boundary probe of the `add` opcode at the `uint` MAX edge: `uint a + 0xFFFFFFFF` keeps an `add(%a, 4294967295 : UInt)` in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// One of the documented arithmetic opcodes (`add`) is observed at the
// upper edge of `uint` (`0xFFFFFFFF`). Because one operand is a
// `uniform` (not a literal), constant folding cannot collapse the sum,
// so the `add` instruction survives into the IR dump with the literal
// `4294967295 : UInt` as its second operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + uint(0xFFFFFFFF);
}

// IR-LABEL: func %main
// IR: add(%a, 4294967295 : UInt)
