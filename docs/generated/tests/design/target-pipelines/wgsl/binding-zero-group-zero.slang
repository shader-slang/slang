//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
//META: purpose=Explicit vk::binding(0, 0) emits @binding(0) @group(0) at the lower edge of the binding axis
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower edge of the @binding / @group axis; explicit (0, 0) maps
// directly through to the WGSL annotation.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

[vk::binding(0, 0)] RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tid.x;
}

// CHECK: @binding(0) @group(0)
// CHECK-SAME: var<storage, read_write>
// CHECK-SAME: outBuf_{{[0-9]+}}
