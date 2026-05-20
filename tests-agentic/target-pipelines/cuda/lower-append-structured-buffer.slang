//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ff5496c69314ad29906a734c4d1b9e25bf4607f4a43b1f56af9601044c1031b4
//META: purpose=lowerAppendConsumeStructuredBuffers runs for CUDA (target != HLSL) so AppendStructuredBuffer is replaced with a wrapper struct holding an elements buffer and an atomic counter buffer.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// In Phase B at row 50 of the pass table, the gate
// `target != HLSL` makes CUDA run the pass. Append/Consume types
// then lower to a struct with `elements` and `counter` fields and
// the Append/Consume operations become helper functions that use
// `atomicAdd` on the counter.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

AppendStructuredBuffer<int> appendBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    appendBuf.Append(int(tid.x));
}

// CHECK: AppendStructuredBufferx3Cintx3E_{{[0-9]+}}
// CHECK: AppendStructuredBuffer_Append
// CHECK: atomicAdd
