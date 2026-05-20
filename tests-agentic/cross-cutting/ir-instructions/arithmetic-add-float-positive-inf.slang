//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Boundary probe of the `add` opcode at `+inf`: a literal `1.0/0.0` folds to `inf : Float`, and `add(%a, inf : Float)` survives in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probe the documented `add` arithmetic opcode at the float `+inf`
// boundary. The compile-time `1.0/0.0` is folded to a literal `inf`
// in the IR — confirming the literal-handling path. The surrounding
// `add` is preserved because the left operand is a `uniform`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + (1.0 / 0.0);
}

// IR-LABEL: func %main
// IR: add(%a, inf : Float)
