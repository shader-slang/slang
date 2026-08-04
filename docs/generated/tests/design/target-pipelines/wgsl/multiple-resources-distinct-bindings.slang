//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=602dd96dd9a5139f2ccd4e3f857107fe47c9705a3ba52eae7e88a90893f9d664
//META: purpose=Multiple resources at module scope receive distinct sequential @binding indices in @group(0)
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Three resources should receive three distinct @binding indices
// sharing the same @group.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;
StructuredBuffer<float> a;
StructuredBuffer<float> b;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = a[tid.x] + b[tid.x];
}

// CHECK: @binding(0) @group(0)
// CHECK: @binding(1) @group(0)
// CHECK: @binding(2) @group(0)
