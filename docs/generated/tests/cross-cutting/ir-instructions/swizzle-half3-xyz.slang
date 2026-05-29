//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=A `.xyz` swizzle on a `half4` lowers to the documented `swizzle` opcode collecting indices `(0, 1, 2)` and producing a `Vec(Half, 3)` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion to `swizzle-half3-single-element.slang`: a 3-element
// swizzle of a half4 produces a `Vec(Half, 3)` result via the same
// `swizzle` opcode, with three index operands. Pins both the
// narrow-float element-type and the mid-length swizzle axis.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half4 v;
uniform RWStructuredBuffer<half3> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = v.xyz;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int, 1 : Int, 2 : Int)
