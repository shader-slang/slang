//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Boundary: half-precision values survive end-to-end: `half` typed buffer and uniform produce `half device*` and `half v_0` fields in the emitted Metal text.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: half-precision float edge. Metal's `half` is a
// distinct type from `float`; Slang must preserve it through
// buffer-element legalization and GlobalParams packing.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<half> outBuf;
uniform half v;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = v + half(tid.x);
}

// CHECK: half v_0
// CHECK: half device*
