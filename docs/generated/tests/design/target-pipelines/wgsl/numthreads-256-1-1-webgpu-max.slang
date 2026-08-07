//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=602dd96dd9a5139f2ccd4e3f857107fe47c9705a3ba52eae7e88a90893f9d664
//META: purpose=[numthreads(256,1,1)] at the WebGPU compute upper edge emits @workgroup_size(256, 1, 1) verbatim
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Upper edge of WebGPU's per-axis compute workgroup_size. Slang
// passes the literal through unchanged; Tint enforces the limit
// downstream.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(256,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tid.x;
}

// CHECK: @compute
// CHECK: @workgroup_size(256, 1, 1)
