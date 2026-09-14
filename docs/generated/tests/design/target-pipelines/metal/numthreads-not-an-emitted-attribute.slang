//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=f18a62d46309fae1d33b5fed992f19c2eff74d722d039dfeeeccc0dfdbf5b824
//META: purpose=Negative: `[numthreads(X,Y,Z)]` does not survive into Metal text because Metal takes the threadgroup size from the dispatch call.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the HLSL threadgroup-size attribute is dropped rather than
// translated: MSL has no source-level equivalent, and the dimensions are
// supplied by the host at dispatch time. A non-trivial 8x4x2 size is used
// so a partial translation would still be visible; CHECK asserts the
// kernel carries its system-value parameter but no `numthreads(` token
// appears anywhere.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: thread_position_in_grid
// CHECK-NOT: numthreads(
