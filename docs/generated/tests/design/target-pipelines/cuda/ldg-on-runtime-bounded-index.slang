//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=f99045fe923d667d28ba075f821f8ecea38aaffb44e4d1ea1d4c3d94fce11929
//META: purpose=Verifies lowerImmutableBufferLoadForCUDA still emits __ldg when the index is a runtime-bounded value sourced from another buffer.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: scalar immutable-buffer load with a runtime
// index (read from another buffer). The `__ldg` lowering does not
// depend on the index being static.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

StructuredBuffer<float> roBuf;
RWStructuredBuffer<uint> idxBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = idxBuf[tid.x];
    outBuf[tid.x] = roBuf[idx];
}

// CHECK: __ldg
