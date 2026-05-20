//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Boundary probe of the `cmpGT` opcode at the NaN-ordering edge: comparing a `uniform float` against itself emits a `cmpGT(%a, %a)` in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The IR-level `cmpGT` opcode is target-agnostic; whether the result
// is `false` for a NaN operand is a runtime / backend concern. This
// probe confirms the IR opcode survives even when both operands are
// the same SSA value (the kind of comparison a `isnan(a) == (a != a)`
// idiom uses to detect a NaN).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = (a > a) ? 1 : 0;
}

// IR-LABEL: func %main
// IR: cmpGT(%a, %a)
