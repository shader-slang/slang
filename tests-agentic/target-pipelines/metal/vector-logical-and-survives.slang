//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=legalizeLogicalAndOr (Metal arm) preserves `&&` over vector operands as element-wise vector AND in the emitted MSL.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `legalizeLogicalAndOr` runs in the Metal arm. With vector
// operands, the resulting MSL keeps the `&&` operator between
// two `bool3(...)` constructions (Metal supports element-wise
// `&&` on vector booleans).

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
