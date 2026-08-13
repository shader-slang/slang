//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
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
