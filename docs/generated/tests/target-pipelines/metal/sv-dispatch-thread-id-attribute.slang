//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=ae7166b4e043d6e1a4435c167ea8bdc675ca3080f5f01a62c716172c61bce000
//META: purpose=SV_DispatchThreadID lowers to a uint3 entry-point parameter carrying the Metal [[thread_position_in_grid]] system-value attribute.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `legalizeEntryPointVaryingParamsForMetal` rewrites the
// SV_DispatchThreadID input into a `uint3` kernel parameter
// with `[[thread_position_in_grid]]`. We match the attribute
// as a bare substring because `[[...]]` would be parsed as a
// FileCheck regex variable.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: uint3 tid_{{[0-9]+}}
// CHECK-SAME: thread_position_in_grid
