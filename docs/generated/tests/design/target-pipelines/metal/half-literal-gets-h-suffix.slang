//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=Boundary: a finite `half` literal is emitted with an explicit `h` suffix, and `half` buffer storage survives as `half device*`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the half end of the literal-suffix axis, which is the case the
// suffix exists for: without `h` a bare decimal would widen and break
// constructs like `as_type<ushort>(h)`. The literal is added to a
// dispatch-derived half so it reaches emit unfolded; CHECK asserts the
// `1.5h` spelling and that the buffer element type stayed `half` rather
// than being promoted to `float`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<half> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = half(tid.x) + half(1.5);
}

// CHECK: half device*
// CHECK: 1.5h
