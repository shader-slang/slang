//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
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
