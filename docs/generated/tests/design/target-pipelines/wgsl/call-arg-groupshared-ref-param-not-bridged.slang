//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizecall-and-the-ptrfunction-t-address-space-rule
//META: doc_section_digest=be15d3c3215e5a9573f5410bea3b67209427e8137b1e2cb1df5c104f9a77fe1a
//META: purpose=A groupshared global passed to a ref/borrow parameter is not bridged so workgroupUniformLoad still aliases the workgroup-space object
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the paramHasCopyInOutSemantics carve-out in legalizeCall. A
// `ref`/borrow parameter must alias the caller's real object, so a
// module-scope groupshared variable handed to `workgroupUniformLoad` must
// NOT be copied into a function-space temporary -- doing so would both change
// the semantics and move the value out of the `workgroup` address space.
// CHECK asserts the emitted call takes the address of the workgroup global
// directly.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;
groupshared uint gShared;

[shader("compute")]
[numthreads(4,1,1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 ltid : SV_GroupThreadID)
{
    if (ltid.x == 0u)
        gShared = tid.x;
    GroupMemoryBarrierWithGroupSync();
    uint v = workgroupUniformLoad(gShared);
    outBuf[tid.x] = v;
}

// CHECK: var<workgroup> gShared_{{[0-9]+}} : u32
// CHECK: workgroupUniformLoad(&((gShared_{{[0-9]+}})))
