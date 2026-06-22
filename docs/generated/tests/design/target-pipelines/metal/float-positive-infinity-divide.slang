//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Boundary: `1.0 / 0.0` is not constant-folded; the Metal emitter renders it as a literal division so the runtime can produce +infinity per IEEE 754.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: float +inf via division by literal +0.0.
// The Metal emit preserves the `(1.0 / 0.0)` expression verbatim
// — Slang does not pre-evaluate this on the float boundary edge.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float pinf = 1.0 / 0.0;
    outBuf[tid.x] = pinf + float(tid.x);
}

// CHECK: 1.0 / 0.0
