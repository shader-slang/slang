//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=031d306ccbb9670357e2a8af6d7e786afcb208ea0c4a357ccf9a309fc8034205
//META: purpose=Verifies lowerImmutableBufferLoadForCUDA emits __ldg for a static index 0 (lower-edge) load of a StructuredBuffer.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: scalar immutable-buffer load at the lower edge
// (index 0). The pass still fires and the read goes through `__ldg`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

StructuredBuffer<float> roBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = roBuf[0];
}

// CHECK: __ldg
// CHECK-SAME: int(0)
