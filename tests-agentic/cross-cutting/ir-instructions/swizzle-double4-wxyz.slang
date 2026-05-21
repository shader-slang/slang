//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=A `.wxyz` swizzle on a `double4` lowers to the documented `swizzle` opcode with indices `(3, 0, 1, 2)` and a `Vec(Double, 4)` result — double-precision probe of the swizzle element-type axis.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc admits `swizzle` over the element-type axis but doesn't
// enumerate which scalar widths it covers. A rotated 4-element
// double-precision swizzle exercises both the upper edge of the
// float-width axis and the non-identity index-order edge in the
// same probe.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double4 v;
uniform RWStructuredBuffer<double4> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = v.wxyz;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 3 : Int, 0 : Int, 1 : Int, 2 : Int)
