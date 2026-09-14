//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=61b9f1fa9e25e0a1a331e30835332c142e930cea0b3de2b952f901beda9edd64
//META: purpose=RWByteAddressBuffer.Store<uint>(off, v) lowers to an OpAccessChain through the underlying runtime array followed by OpStore — the write counterpart to the byte-address Load case.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: legalizeByteAddressBufferOps runs with
// translateToStructuredBufferOps=true on the SPIR-V arm. The bundle
// pins the load form; this test pins the store form — Store<uint>
// becomes OpAccessChain into the StorageBuffer pointer followed by
// OpStore.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWByteAddressBuffer outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf.Store<uint>(tid.x * 4, 42u);
}

// CHECK-DAG: OpAccessChain %_ptr_StorageBuffer_uint
// CHECK-DAG: OpStore
