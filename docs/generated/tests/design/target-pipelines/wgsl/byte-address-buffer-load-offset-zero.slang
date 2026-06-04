//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizebyteaddressbufferops-with-wgsl-options
//META: doc_section_digest=678e8461e323191268a43189d73cdc9e17745b10b7056df81464b4ecab6262c7
//META: purpose=ByteAddressBuffer.Load<uint>(0) at offset zero still emits the array<u32>[...]/4 indexing expression
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower edge of the byte-address-buffer offset axis: even with an
// explicit zero offset the pass keeps the divide-by-4 indexing
// shape instead of constant-folding it away.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

ByteAddressBuffer ba;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = ba.Load<uint>(0);
}

// CHECK: var<storage, read>
// CHECK-SAME: array<u32>
// CHECK: ba_{{[0-9]+}}[
// CHECK-SAME: /4
