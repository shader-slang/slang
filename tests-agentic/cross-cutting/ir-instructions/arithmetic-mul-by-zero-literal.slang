//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Boundary probe of the `mul` opcode with a literal `0` operand: a `uniform * 0` still emits `mul(%a, 0 : Float)` because constant folding does not strength-reduce a uniform operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probe the documented `mul` arithmetic opcode at the zero edge.
// Because `a` is a `uniform`, the compiler does not fold `a * 0` away;
// the `mul` opcode appears literally in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a * 0.0;
}

// IR-LABEL: func %main
// IR: mul(%a, 0 : Float)
