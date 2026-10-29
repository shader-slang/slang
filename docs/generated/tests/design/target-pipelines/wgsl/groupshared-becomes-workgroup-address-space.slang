//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#specializeaddressspaceforwgsl
//META: doc_section_digest=7430da3e9233ea2bf55c1bea479c7ce668cf06b5f720da136de2a6800f764369
//META: purpose=groupshared declarations are tagged var<workgroup> by specializeAddressSpaceForWGSL
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A 'groupshared' array maps to WGSL's workgroup address space.
// This is decided by specializeAddressSpaceForWGSL.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;
groupshared int sharedArr[64];

[shader("compute")]
[numthreads(64,1,1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 ltid : SV_GroupThreadID)
{
    sharedArr[ltid.x] = int(tid.x);
    outBuf[tid.x] = sharedArr[ltid.x];
}

// CHECK: var<workgroup>
// CHECK-SAME: sharedArr_{{[0-9]+}}
// CHECK-SAME: array<i32
