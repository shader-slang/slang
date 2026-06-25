//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Boundary probe of the `sub` opcode at the `int` MIN edge: subtracting `INT_MIN` from a uniform produces `sub(%a, -2147483648 : Int)` in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probe the documented `sub` arithmetic opcode at the lower edge of
// signed 32-bit (`-2147483648`). The literal is written as
// `-2147483647 - 1` to avoid the front-end parsing it as a positive
// 2147483648 first. The IR retains the literal as `-2147483648 : Int`
// in the surviving `sub` instruction's right operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a - (-2147483647 - 1);
}

// IR-LABEL: func %main
// IR: sub(%a, -2147483648 : Int)
