//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=26d469b87d05f8d3b2e0539d080d7839ae84df294d66110dd1541682d63c62d1
//META: purpose=`StructuredBuffer<T>::Load(idx, out status)` on `-target hlsl` (the capability-permitted target for the status overload) surfaces the documented `structuredBufferLoadStatus` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the status-returning overload on a read-only structured
// buffer. The core-module method requires the HLSL structured-buffer
// capability, so the compile targets HLSL at `cs_6_6` rather than
// SPIR-V; the CHECK pins the three-operand opcode in the user body.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main -profile cs_6_6

uniform StructuredBuffer<int> sbuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    uint status;
    int v = sbuf.Load(idx, status);
    rwbuf[idx] = v + (int)status;
}

// IR-LABEL: func %main
// IR: structuredBufferLoadStatus
