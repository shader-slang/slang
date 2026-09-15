//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=372c7f6b7656c8bbbab96f3db229bae75b2d8ca93faa1c4e767d89f31a2c3386
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
