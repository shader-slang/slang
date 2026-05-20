//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#legalizebyteaddressbufferops-with-wgsl-options
//META: doc_section_digest=678e8461e323191268a43189d73cdc9e17745b10b7056df81464b4ecab6262c7
//META: purpose=legalizeByteAddressBufferOps with WGSL options lowers a ByteAddressBuffer.Load<uint> to an array<u32>[offset/4] indexing expression
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL has no native byte-address buffer; the pass lowers
// ByteAddressBuffer to array<u32> and divides the offset by 4.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

ByteAddressBuffer ba;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = ba.Load<uint>(16);
}

// CHECK: var<storage, read>
// CHECK-SAME: array<u32>
// CHECK: ba_{{[0-9]+}}[
// CHECK-SAME: /4
