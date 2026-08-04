//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#specializeaddressspaceforwgsl
//META: doc_section_digest=4dfc10b6dcfc7de02c4480f005a0e2eb0f14d6a66ef8d5fb6ce781d9a448550f
//META: purpose=A loose-uniform global parameter emits with var<uniform> address space and a std140 wrapper struct
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge of the address-space axis: a loose uniform parameter goes
// through the GlobalParams_std140 wrapper and emits with the
// var<uniform> address space (distinct from the var<storage,...>
// and var<workgroup> shapes other tests cover).

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

uniform float scale;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = scale;
}

// CHECK: var<uniform>
// CHECK-SAME: globalParams_{{[0-9]+}}
// CHECK-SAME: GlobalParams_std140
