//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=5fafdc04391fe1499bf74bfe01420955a8364571f1b55e362cc671d3444538c9
//META: purpose=Verifies a float +inf (1.0/0.0) lowers to the prelude macro SLANG_INFINITY on CUDA emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: float +infinity. CUDA emit substitutes the prelude
// macro `SLANG_INFINITY` for an infinity-valued literal, and the
// value is gated by tid.x so constant-folding cannot remove it.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float inf = (tid.x == 0U) ? (1.0f / 0.0f) : float(tid.x);
    outBuf[tid.x] = inf;
}

// CHECK: SLANG_INFINITY
