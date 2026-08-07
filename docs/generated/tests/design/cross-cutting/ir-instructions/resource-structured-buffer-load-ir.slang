//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=9e5354e38bb09fc16bac90544687941aec8ef947b56cd9f9b65ce3ddb041f315
//META: purpose=A read from a read-only `StructuredBuffer<T>` lowers to the documented `structuredBufferLoad(base, index)` opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `structuredBufferLoad`/`rwstructuredBufferStore`
// resource row. Indexing a read-only `StructuredBuffer<int>` lowers to
// the `structuredBufferLoad(base, index)` opcode in the IR dump (the
// read-only sibling that returns a value rather than a pointer).

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
