//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#specializeaddressspaceforwgsl
//META: doc_section_digest=4dfc10b6dcfc7de02c4480f005a0e2eb0f14d6a66ef8d5fb6ce781d9a448550f
//META: purpose=A 256-element groupshared array emits as var<workgroup> ... : array<u32, i32(256)>
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Upper edge of the groupshared array axis (matching the 256
// workgroup max). The element count appears literally inside
// array<T, N> in the WGSL emit.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;
groupshared uint sharedArr[256];

[shader("compute")]
[numthreads(256,1,1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 ltid : SV_GroupThreadID)
{
    sharedArr[ltid.x] = tid.x;
    outBuf[tid.x] = sharedArr[ltid.x];
}

// CHECK: var<workgroup>
// CHECK-SAME: sharedArr_{{[0-9]+}}
// CHECK-SAME: array<u32
// CHECK-SAME: 256
