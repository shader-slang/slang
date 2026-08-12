//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=4311a2734c94c89f3951c5e93e111cd25e64443cd00a56d067df242df4d2fb08
//META: purpose=A write through a subscript into an RWStructuredBuffer lowers to `rwstructuredBufferGetElementPtr` plus a `store`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `rwstructuredBufferGetElementPtr` memory row: a
// subscript-assignment into an `RWStructuredBuffer` forms an element
// pointer via `rwstructuredBufferGetElementPtr(base, index)` and then
// `store`s the value through it. A non-constant `uniform` index keeps
// the pointer arithmetic in the IR.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int i;
uniform int v;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[i] = v;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%{{.*}}, %{{.*}})
// IR: store(%{{.*}}, %{{.*}})
