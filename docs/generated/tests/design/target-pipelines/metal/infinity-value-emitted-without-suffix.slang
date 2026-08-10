//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=Boundary: a non-finite float value is emitted without the `f` suffix, as the bare division `(1.0 / 0.0)`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the documented carve-out on the literal-suffix rule: the suffix
// is appended only to finite values, and infinity is rendered as a
// division that evaluates to it at runtime. This is the known remaining
// gap the emitter notes, so pinning it makes any future change visible.
// CHECK asserts the `(1.0 / 0.0)` form with unsuffixed operands appears
// while the neighbouring finite literal on the next line still gets `f`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = 1.0f / 0.0f;
    outBuf[1] = float(tid.x) * 2.25f;
}

// CHECK: (1.0 / 0.0)
// CHECK: 2.25f
