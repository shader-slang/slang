//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#rwstructuredbuffergetelementptr
//META: doc_section_digest=1109150b01fc6cb7960b788c7bb9ca3bbae1748af0ae3a77abc2970881ef9206
//META: purpose=`rwbuf[uint_idx] = v` (uint-typed index) still surfaces the documented `rwstructuredBufferGetElementPtr(%base, %index)` opcode shape; the operand type is uint rather than int.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: index expression typed as `uint` rather than `int`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    rwbuf[idx] = (int)idx;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%rwbuf, %idx)
