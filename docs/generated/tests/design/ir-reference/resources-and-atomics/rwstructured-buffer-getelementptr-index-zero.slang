//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#rwstructuredbuffergetelementptr
//META: doc_section_digest=79a3f90a07e564c3a6781f28bec380af53c172ad701f8d621623d248c4edee79
//META: purpose=Writing `rwbuf[0] = v` (lower-edge index lvalue) still surfaces the documented `rwstructuredBufferGetElementPtr(%base, 0 : Int)` opcode shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: literal `0` index on the lvalue path.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    rwbuf[0] = (int)tid.x;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%rwbuf, 0 : Int)
