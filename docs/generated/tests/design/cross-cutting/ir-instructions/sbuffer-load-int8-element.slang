//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=`StructuredBuffer<int8_t>.Load(idx)` lowers to the documented `structuredBufferLoad` resource opcode with an `Int8` result type — narrow-integer probe of the read-only resource family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `structuredBufferLoad` as the read-only sibling of
// `rwstructuredBufferStore`. An `int8_t` element type pins the
// narrow-integer edge of the element-type axis on the read-only side
// of the resource family.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform StructuredBuffer<int8_t> in_buf;
uniform RWStructuredBuffer<int8_t> buf;
uniform int idx;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = in_buf.Load(idx);
}

// IR-LABEL: func %main
// IR: structuredBufferLoad(%inx5Fbuf, %idx)
