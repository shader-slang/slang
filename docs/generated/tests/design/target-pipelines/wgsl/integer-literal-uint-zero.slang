//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=602dd96dd9a5139f2ccd4e3f857107fe47c9705a3ba52eae7e88a90893f9d664
//META: purpose=The uint literal 0u emits as u32(0) at the lower edge of the unsigned integer range
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower edge of the uint range; the WGSL emitter uses the
// constructor-style spelling u32(0).

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 0u;
}

// CHECK: u32(0)
