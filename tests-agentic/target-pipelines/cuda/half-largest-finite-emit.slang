//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
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
