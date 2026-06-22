//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#specializeaddressspaceforwgsl
//META: doc_section_digest=55422c2a92758737eb6eb0ac0f489a44b06df5c80b1e2907ff718d43d17483b7
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
