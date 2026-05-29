//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
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
