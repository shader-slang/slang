//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=A single-element extract `.w` on a `double4` lowers to the documented `swizzle` opcode with a `Double` result type and a single index `3 : Int` — double-precision probe at the length-1 swizzle edge.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion to `swizzle-double4-wxyz.slang` at the length-1 edge: a
// `double` extract uses the same `swizzle` opcode with one index
// operand. Pins both the double-precision element type and the
// length-1 swizzle boundary.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double4 v;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = v.w;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 3 : Int)
