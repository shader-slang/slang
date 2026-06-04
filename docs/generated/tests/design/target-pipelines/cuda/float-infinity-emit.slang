//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
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
