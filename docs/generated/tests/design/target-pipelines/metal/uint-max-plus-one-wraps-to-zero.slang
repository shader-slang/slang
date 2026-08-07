//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=Boundary: `0xFFFFFFFFu + 1u` folds to `0U` in the emitted Metal text while the same constant added to a runtime value survives as `4294967295U`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the unsigned MAX edge on the integer-literal side of Metal emit.
// The first store is fully constant so the folder must produce the wrapped
// value `0U`; the second adds the same constant to a dispatch-derived
// value, so the literal has to survive to emit unfolded. Checking both in
// one shader pins that the wrap happened in the folder rather than in the
// literal printer.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = 0xFFFFFFFFu + 1u;
    outBuf[1] = 0xFFFFFFFFu + tid.x;
}

// CHECK-DAG: = 0U;
// CHECK-DAG: 4294967295U
