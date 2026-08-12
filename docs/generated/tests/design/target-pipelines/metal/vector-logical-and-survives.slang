//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=2a4c97aeaacd2a9728a83c1944ef99c7efe30fe3fe60ef075d8d47044f03eefc
//META: purpose=The Metal arm of `legalizeLogicalAndOr` keeps `&&` over vector operands as a single element-wise expression between two `bool3(...)` values.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the Metal arm of `legalizeLogicalAndOr` rewrites only the
// short-circuiting scalar form and leaves element-wise vector `&&` intact,
// which MSL supports natively. One operand is derived from the dispatch id
// so the expression cannot be constant-folded away; CHECK asserts the
// emitted text still contains a single `&&` sitting between two `bool3(`
// constructions rather than a lowered select or a per-component expansion.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool3 a = bool3(tid.x > 0, tid.y > 0, tid.z > 0);
    bool3 b = bool3(true, false, true);
    bool3 c = a && b;
    outBuf[tid.x] = c.x ? 1 : 0;
}

// CHECK: bool3(
// CHECK-SAME: &&
// CHECK-SAME: bool3(
