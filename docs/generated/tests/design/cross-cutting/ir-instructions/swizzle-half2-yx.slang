//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=A `.yx` swap-swizzle on a `half2` lowers to the documented `swizzle` opcode with indices `(1, 0)` and a `Vec(Half, 2)` result — narrow-float length-2 reverse-order probe.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two-element swizzles with reversed order pin the smallest-vector
// non-identity index pattern. Combining that with a half-precision
// element type covers the narrow-float / short-vector corner of the
// `swizzle` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half2 v;
uniform RWStructuredBuffer<half2> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = v.yx;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 1 : Int, 0 : Int)
