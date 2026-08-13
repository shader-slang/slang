//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=1b40e6975a461c5c97ead4202c2a70e579c2e6e882749ceb49d57be34bac586c
//META: purpose=`SV_GroupIndex` is synthesized in the kernel body from `[[thread_position_in_threadgroup]]` because Metal has no single builtin for the linear in-group index.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the Metal varying-param legalization does not simply
// rename this semantic: Metal exposes the 3-D in-threadgroup position but
// no flattened index, so Slang has to bind the 3-D builtin and compute the
// linear value itself. CHECK asserts the `thread_position_in_threadgroup`
// attribute appears on the kernel, which is the observable evidence that
// the synthesis path (rather than a direct builtin) was taken.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

groupshared int s_data[64];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint tid : SV_GroupIndex)
{
    s_data[tid] = int(tid);
    outBuf[tid] = s_data[tid];
}

// CHECK: thread_position_in_threadgroup
