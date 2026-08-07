//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=26d469b87d05f8d3b2e0539d080d7839ae84df294d66110dd1541682d63c62d1
//META: purpose=`RWStructuredBuffer<T>::Load(idx, out status)` on `-target hlsl` surfaces the documented `rwstructuredBufferLoadStatus` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the status-returning overload on a writable structured
// buffer, which adds a third operand for the residency output. The
// overload needs the HLSL structured-buffer capability, so the compile
// targets HLSL at `cs_6_6`; the CHECK finds the distinct opcode in the
// user body.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main -profile cs_6_6

uniform RWStructuredBuffer<int> rwbuf;
uniform RWStructuredBuffer<int> sink;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    uint status;
    int v = rwbuf.Load(idx, status);
    sink[idx] = v + (int)status;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferLoadStatus
