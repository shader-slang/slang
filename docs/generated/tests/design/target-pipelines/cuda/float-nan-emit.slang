//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=6125db2deed527f9850790324fd3a09c7d0cd76e0bbeee263f331f56a3527a81
//META: purpose=Verifies a float NaN (0.0/0.0) is emitted on CUDA as the (0.0 / 0.0) expression form so the value is well-defined under nvrtc.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: float NaN. CUDA emit writes the NaN-producing
// expression as `(0.0 / 0.0)` and the result is gated by tid.x to
// prevent constant folding from removing the value.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float nan = (tid.x == 0U) ? (0.0f / 0.0f) : float(tid.x);
    outBuf[tid.x] = nan;
}

// CHECK: (0.0 / 0.0)
