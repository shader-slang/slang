//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=A single-element extract `.x` on a `vector<uint8_t, 4>` lowers to the documented `swizzle` opcode with a `UInt8` result type — narrow-integer probe of the swizzle element-type axis at the length-1 edge.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc admits `swizzle` over the element-type axis without
// enumerating widths. A length-1 extract on a `uint8_t` vector pins
// both the narrow-integer (8-bit unsigned) element edge and the
// length-1 swizzle boundary.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform vector<uint8_t, 4> v;
uniform RWStructuredBuffer<uint8_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = v.x;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int)
