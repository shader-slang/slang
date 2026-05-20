//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=ae7166b4e043d6e1a4435c167ea8bdc675ca3080f5f01a62c716172c61bce000
//META: purpose=SV_GroupIndex is synthesized from `[[thread_position_in_threadgroup]]` because Metal does not provide a single builtin for the linear in-group index; Slang computes it in the kernel body.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `legalizeEntryPointVaryingParamsForMetal` accepts
// SV_GroupIndex by binding [[thread_position_in_threadgroup]] as
// a uint3 and computing the linear index inside the kernel.

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
