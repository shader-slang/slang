//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=6125db2deed527f9850790324fd3a09c7d0cd76e0bbeee263f331f56a3527a81
//META: purpose=Verifies the half largest-finite value 65504 round-trips through CUDA emit as the __half(...) cast so the value reaches nvrtc as a CUDA half.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: half (fp16) largest finite. CUDA emit wraps half
// values in the prelude `__half(...)` constructor and the literal
// value 65504 is the documented half max.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<half> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    half h = half((tid.x == 0U) ? 65504.0f : float(tid.x));
    outBuf[tid.x] = h;
}

// CHECK: 65504.0f
// CHECK: __half(
