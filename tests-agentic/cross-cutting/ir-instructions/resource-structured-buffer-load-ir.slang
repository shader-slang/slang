//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=A read from a `StructuredBuffer<T>` lowers to the documented `structuredBufferLoad(base, index)` opcode (the read-only sibling of `rwstructuredBufferStore`).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `structuredBufferLoad` next to `rwstructuredBufferStore`
// with operand shape `(base, index, val?)`. The bundle already covers
// the RW-store side via `rwstructuredBufferGetElementPtr`; here we
// cover the read-only `StructuredBuffer` load form which uses the
// distinct `structuredBufferLoad` opcode rather than a pointer.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform StructuredBuffer<int> in_buf;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = in_buf[0];
}

// IR-LABEL: func %main
// IR: structuredBufferLoad(%inx5Fbuf, 0 : Int)
