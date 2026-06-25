//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=Indexing into an `RWStructuredBuffer` lowers to a resource-family IR opcode that produces a pointer to the element.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Structured-buffer access" entry covers
// `rwstructuredBufferStore` / `structuredBufferLoad`. In practice
// the front end lowers `buf[i] = ...` through
// `rwstructuredBufferGetElementPtr` (a member of the same resource
// family) followed by a `store`. Both opcodes are visible in the IR
// dump for an `RWStructuredBuffer` write.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%buf, 0 : Int)
// IR: store(%{{.*}}, %{{.*}})
