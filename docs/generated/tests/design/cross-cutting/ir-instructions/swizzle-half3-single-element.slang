//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=A single-element extract `.y` on a `half3` vector lowers to the documented `swizzle` opcode with a `Half` result type — narrow-float probe of the swizzle-element-type axis.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `swizzle` under memory / aggregate accessors but does
// not constrain the element-type axis. Probing a half-precision
// vector exercises the `swizzle` opcode at the narrow-float edge: the
// result type appears as `Half` in the IR, with the same index-list
// operand shape used by the existing `float4` probes.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half3 v;
uniform RWStructuredBuffer<half> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = v.y;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 1 : Int)
