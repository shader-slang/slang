//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=9d22512c322523764ce371d9c4007f35d2bbc6c883c193616d4814d1ae0d08eb
//META: purpose=Verifies the float positive-zero literal 0.0f survives CUDA emit as 0.0f when guarded behind a thread-id branch (constant-folding cannot remove it).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: float +0.0. Constant folding eliminates a pure
// 0.0 + 0.0 expression, so the value is gated by tid.x so the
// literal must reach emit.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float pz = (tid.x == 0U) ? 0.0f : float(tid.x);
    outBuf[tid.x] = pz;
}

// CHECK: 0.0f
